/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2023 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * This file is part of the TinyUSB stack.
 */

#include "tusb_option.h"

#if CFG_TUH_ENABLED && defined(TUP_USBIP_DWC2)

#include "host/hcd.h"
#include "dwc2_type.h"

#if TU_CHECK_MCU(OPT_MCU_AIC8800, OPT_MCU_AIC8800MC, OPT_MCU_AIC8800M40)
  #include "dwc2_aic.h"
#else
  #error "Unsupported MCUs"
#endif

// DWC2 registers
#define DWC2_REG(_port)       ((dwc2_regs_t*) _dwc2_controller[_port].reg_base)

//--------------------------------------------------------------------+
// Controller API
//--------------------------------------------------------------------+

// optional hcd configuration, called by tuh_configure()
bool hcd_configure(uint8_t rhport, uint32_t cfg_id, const void* cfg_param) {
  (void) rhport;
  (void) cfg_id;
  (void) cfg_param;

  return false;
}

TU_ATTR_ALWAYS_INLINE static inline void fifo_flush_tx(dwc2_regs_t* dwc2, uint8_t epnum) {
  // flush TX fifo and wait for it cleared
  dwc2->grstctl = GRSTCTL_TXFFLSH | (epnum << GRSTCTL_TXFNUM_Pos);
  while (dwc2->grstctl & GRSTCTL_TXFFLSH_Msk) {}
}
TU_ATTR_ALWAYS_INLINE static inline void fifo_flush_rx(dwc2_regs_t* dwc2) {
  // flush RX fifo and wait for it cleared
  dwc2->grstctl = GRSTCTL_RXFFLSH;
  while (dwc2->grstctl & GRSTCTL_RXFFLSH_Msk) {}
}

#if CFG_TUSB_DEBUG >= DWC2_DEBUG
void print_dwc2_info(dwc2_regs_t* dwc2) {
  // print guid, gsnpsid, ghwcfg1, ghwcfg2, ghwcfg3, ghwcfg4
  // use dwc2_info.py/md for bit-field value and comparison with other ports
  volatile uint32_t const* p = (volatile uint32_t const*) &dwc2->guid;
  TU_LOG(DWC2_DEBUG, "guid, gsnpsid, ghwcfg1, ghwcfg2, ghwcfg3, ghwcfg4\r\n");
  for (size_t i = 0; i < 5; i++) {
    TU_LOG(DWC2_DEBUG, "0x%08" PRIX32 ", ", p[i]);
  }
  TU_LOG(DWC2_DEBUG, "0x%08" PRIX32 "\r\n", p[5]);
}
#endif

static void reset_core(dwc2_regs_t* dwc2) {
  // reset core
  dwc2->grstctl |= GRSTCTL_CSRST;

  // wait for reset bit is cleared
  // TODO version 4.20a should wait for RESET DONE mask
  while (dwc2->grstctl & GRSTCTL_CSRST) {}

  // wait for AHB master IDLE
  while (!(dwc2->grstctl & GRSTCTL_AHBIDL)) {}

  // wait for host mode ?
}

static bool phy_hs_supported(dwc2_regs_t* dwc2) {
  (void) dwc2;

#if TU_CHECK_MCU(OPT_MCU_ESP32S2, OPT_MCU_ESP32S3)
  // note: esp32 incorrect report its hs_phy_type as utmi
  return false;
#elif !TUD_OPT_HIGH_SPEED
  return false;
#else
  return dwc2->ghwcfg2_bm.hs_phy_type != HS_PHY_TYPE_NONE;
#endif
}

static void phy_fs_init(dwc2_regs_t* dwc2) {
  TU_LOG(DWC2_DEBUG, "Fullspeed PHY init\r\n");

  // Select FS PHY
  dwc2->gusbcfg |= GUSBCFG_PHYSEL;

  // MCU specific PHY init before reset
  dwc2_phy_init(dwc2, HS_PHY_TYPE_NONE);

  // Reset core after selecting PHY
  reset_core(dwc2);

  // USB turnaround time is critical for certification where long cables and 5-Hubs are used.
  // So if you need the AHB to run at less than 30 MHz, and if USB turnaround time is not critical,
  // these bits can be programmed to a larger value. Default is 5
  dwc2->gusbcfg = (dwc2->gusbcfg & ~GUSBCFG_TRDT_Msk) | (5u << GUSBCFG_TRDT_Pos);

  // MCU specific PHY update post reset
  dwc2_phy_update(dwc2, HS_PHY_TYPE_NONE);

  // set max speed
  //dwc2->dcfg = (dwc2->dcfg & ~DCFG_DSPD_Msk) | (DCFG_DSPD_FS << DCFG_DSPD_Pos);
}

static void phy_hs_init(dwc2_regs_t* dwc2) {
  uint32_t gusbcfg = dwc2->gusbcfg;

  // De-select FS PHY
  gusbcfg &= ~GUSBCFG_PHYSEL;

  if (dwc2->ghwcfg2_bm.hs_phy_type == HS_PHY_TYPE_ULPI) {
    TU_LOG(DWC2_DEBUG, "Highspeed ULPI PHY init\r\n");

    // Select ULPI
    gusbcfg |= GUSBCFG_ULPI_UTMI_SEL;

    // ULPI 8-bit interface, single data rate
    gusbcfg &= ~(GUSBCFG_PHYIF16 | GUSBCFG_DDRSEL);

    // default internal VBUS Indicator and Drive
    gusbcfg &= ~(GUSBCFG_ULPIEVBUSD | GUSBCFG_ULPIEVBUSI);

    // Disable FS/LS ULPI
    gusbcfg &= ~(GUSBCFG_ULPIFSLS | GUSBCFG_ULPICSM);
  } else {
    TU_LOG(DWC2_DEBUG, "Highspeed UTMI+ PHY init\r\n");

    // Select UTMI+ with 8-bit interface
    gusbcfg &= ~(GUSBCFG_ULPI_UTMI_SEL | GUSBCFG_PHYIF16);

    // Set 16-bit interface if supported
    if (dwc2->ghwcfg4_bm.utmi_phy_data_width) gusbcfg |= GUSBCFG_PHYIF16;
  }

  // Apply config
  dwc2->gusbcfg = gusbcfg;

  // mcu specific phy init
  dwc2_phy_init(dwc2, dwc2->ghwcfg2_bm.hs_phy_type);

  // Reset core after selecting PHY
  reset_core(dwc2);

  // Set turn-around, must after core reset otherwise it will be clear
  // - 9 if using 8-bit PHY interface
  // - 5 if using 16-bit PHY interface
  gusbcfg &= ~GUSBCFG_TRDT_Msk;
  gusbcfg |= (dwc2->ghwcfg4_bm.utmi_phy_data_width ? 5u : 9u) << GUSBCFG_TRDT_Pos;
  dwc2->gusbcfg = gusbcfg;

  // MCU specific PHY update post reset
  dwc2_phy_update(dwc2, dwc2->ghwcfg2_bm.hs_phy_type);

  // Set max speed
  /*uint32_t dcfg = dwc2->dcfg;
  dcfg &= ~DCFG_DSPD_Msk;
  dcfg |= DCFG_DSPD_HS << DCFG_DSPD_Pos;

  // XCVRDLY: transceiver delay between xcvr_sel and txvalid during device chirp is required
  // when using with some PHYs such as USB334x (USB3341, USB3343, USB3346, USB3347)
  if (dwc2->ghwcfg2_bm.hs_phy_type == HS_PHY_TYPE_ULPI) dcfg |= DCFG_XCVRDLY;

  dwc2->dcfg = dcfg;*/

  // sof en
  dwc2->gintsts = GINTSTS_SOF;
  dwc2->gintmsk |= GINTMSK_SOFM;
}

static bool check_dwc2(dwc2_regs_t* dwc2) {
#if CFG_TUSB_DEBUG >= DWC2_DEBUG
  print_dwc2_info(dwc2);
#endif

  // For some reasons: GD32VF103 snpsid and all hwcfg register are always zero (skip it)
  (void) dwc2;
#if !TU_CHECK_MCU(OPT_MCU_GD32VF103)
  uint32_t const gsnpsid = dwc2->gsnpsid & GSNPSID_ID_MASK;
  TU_ASSERT(gsnpsid == DWC2_OTG_ID || gsnpsid == DWC2_FS_IOT_ID || gsnpsid == DWC2_HS_IOT_ID);
#endif

  return true;
}

// Initialize controller to host mode
bool hcd_init(uint8_t rhport) {
    dwc2_regs_t* dwc2 = DWC2_REG(rhport);

    // Check Synopsys ID register, failed if controller clock/power is not enabled
    if (!check_dwc2(dwc2)) return;

    // max number of endpoints & total_fifo_size are:
    // hw_cfg2->num_dev_ep, hw_cfg2->total_fifo_size

    if (phy_hs_supported(dwc2)) {
        phy_hs_init(dwc2); // Highspeed
    } else {
        phy_fs_init(dwc2); // core does not support highspeed or hs phy is not present
    }

    // Restart PHY clock
    dwc2->pcgctl &= ~(PCGCTL_STOPPCLK | PCGCTL_GATEHCLK | PCGCTL_PWRCLMP | PCGCTL_RSTPDWNMODULE);

    /* Set HS/FS Timeout Calibration to 7 (max available value).
    * The number of PHY clocks that the application programs in
    * this field is added to the high/full speed interpacket timeout
    * duration in the core to account for any additional delays
    * introduced by the PHY. This can be required, because the delay
    * introduced by the PHY in generating the linestate condition
    * can vary from one PHY to another.
    */
    dwc2->gusbcfg |= (7ul << GUSBCFG_TOCAL_Pos);

    // Force host mode
    dwc2->gusbcfg = (dwc2->gusbcfg & ~GUSBCFG_FDMOD) | GUSBCFG_FHMOD;

    // Clear A override, force B Valid
    dwc2->gotgctl = (dwc2->gotgctl & ~GOTGCTL_AVALOEN) | GOTGCTL_BVALOEN | GOTGCTL_BVALOVAL;

    fifo_flush_tx(dwc2, 0x10); // all tx fifo
    fifo_flush_rx(dwc2);

    // Host port
    dwc2->hcfg = 0;
    dwc2->hprt = HPRT_PPWR;

    // Host channel
    for (int i = 0; i < 8; i++) {
        dwc2->channel[i].hcint = ~0;
        dwc2->channel[i].hcintmsk = 0U;
    }

    // Host channel interrupt mask
    dwc2->haintmsk = 0;

    // Clear all interrupts
    uint32_t int_mask = dwc2->gintsts;
    dwc2->gintsts |= int_mask;
    int_mask = dwc2->gotgint;
    dwc2->gotgint |= int_mask;

    // Required as part of core initialization.
    dwc2->gintmsk = GINTMSK_PRTIM | GINTMSK_HCIM | GINTMSK_DISCINT;

    // Configure TX FIFO empty level for interrupt. Default is complete empty
    //dwc2->gahbcfg |= GAHBCFG_TXFELVL;

    dwc2->gahbcfg = GAHBCFG_DMAEN | (0x7UL << GAHBCFG_HBSTLEN_Pos);

    // Enable global interrupt
    dwc2->gahbcfg |= GAHBCFG_GINT;

    return false;
}

void hcd_disconnect(dwc2_regs_t* dwc2)
{
}

void hcd_port_handler(dwc2_regs_t* dwc2)
{
}

// Interrupt Handler
void hcd_int_handler(uint8_t rhport, bool in_isr) {
    (void) in_isr;
    dwc2_regs_t* dwc2 = DWC2_REG(rhport);

    uint32_t const int_mask = dwc2->gintmsk;
    uint32_t const int_status = dwc2->gintsts & int_mask;

    /* Handle Host Disconnect Interrupts */
    if (int_status & GINTSTS_DISCINT) {
        /* Cleanup HPRT */
        dwc2->hprt &= ~(HPRT_PCDET | HPRT_PENA | HPRT_PENCHNG | HPRT_POCCHNG);

        /* Handle Host Port Interrupts */
        //HAL_HCD_Disconnect_Callback(hhcd);
        hcd_disconnect(dwc2);
        // FSLSPCS = 48MHz
        dwc2->hcfg &= ~(HCFG_FSLSPCS_Msk);
        dwc2->hcfg |= ((0x1UL << HCFG_FSLSPCS_Pos) & HCFG_FSLSPCS_Msk);
        dwc2->hfir = 48000U;
        dwc2->gintsts = GINTSTS_DISCINT;
    }

    /* Handle Host Port Interrupts */
    if (int_status & GINTSTS_HPRTINT) {
        hcd_port_handler(dwc2);
        dwc2->gintsts = GINTSTS_HPRTINT;
    }

    if (int_status & GINTSTS_SOF) {
        //hcd_sof_handler(dwc2); // TBD:
        dwc2->gintsts = GINTSTS_SOF;
    }
}

// Enable USB interrupt
void hcd_int_enable (uint8_t rhport) {
  (void) rhport;
}

// Disable USB interrupt
void hcd_int_disable(uint8_t rhport) {
  (void) rhport;
}

// Get frame number (1ms)
uint32_t hcd_frame_number(uint8_t rhport) {
  (void) rhport;

  return 0;
}

//--------------------------------------------------------------------+
// Port API
//--------------------------------------------------------------------+

// Get the current connect status of roothub port
bool hcd_port_connect_status(uint8_t rhport) {
  (void) rhport;

  return false;
}

// Reset USB bus on the port. Return immediately, bus reset sequence may not be complete.
// Some port would require hcd_port_reset_end() to be invoked after 10ms to complete the reset sequence.
void hcd_port_reset(uint8_t rhport) {
  (void) rhport;
}

// Complete bus reset sequence, may be required by some controllers
void hcd_port_reset_end(uint8_t rhport) {
  (void) rhport;
}

// Get port link speed
tusb_speed_t hcd_port_speed_get(uint8_t rhport) {
  (void) rhport;

  return TUSB_SPEED_FULL;
}

// HCD closes all opened endpoints belong to this device
void hcd_device_close(uint8_t rhport, uint8_t dev_addr) {
  (void) rhport;
  (void) dev_addr;
}

//--------------------------------------------------------------------+
// Endpoints API
//--------------------------------------------------------------------+

// Open an endpoint
bool hcd_edpt_open(uint8_t rhport, uint8_t dev_addr, tusb_desc_endpoint_t const * ep_desc) {
  (void) rhport;
  (void) dev_addr;
  (void) ep_desc;

  return false;
}

// Submit a transfer, when complete hcd_event_xfer_complete() must be invoked
bool hcd_edpt_xfer(uint8_t rhport, uint8_t dev_addr, uint8_t ep_addr, uint8_t * buffer, uint16_t buflen) {
  (void) rhport;
  (void) dev_addr;
  (void) ep_addr;
  (void) buffer;
  (void) buflen;

  return false;
}

// Abort a queued transfer. Note: it can only abort transfer that has not been started
// Return true if a queued transfer is aborted, false if there is no transfer to abort
bool hcd_edpt_abort_xfer(uint8_t rhport, uint8_t dev_addr, uint8_t ep_addr) {
  (void) rhport;
  (void) dev_addr;
  (void) ep_addr;

  return false;
}

// Submit a special transfer to send 8-byte Setup Packet, when complete hcd_event_xfer_complete() must be invoked
bool hcd_setup_send(uint8_t rhport, uint8_t dev_addr, uint8_t const setup_packet[8]) {
  (void) rhport;
  (void) dev_addr;
  (void) setup_packet;

  return false;
}

// clear stall, data toggle is also reset to DATA0
bool hcd_edpt_clear_stall(uint8_t rhport, uint8_t dev_addr, uint8_t ep_addr) {
  (void) rhport;
  (void) dev_addr;
  (void) ep_addr;

  return false;
}

#endif
