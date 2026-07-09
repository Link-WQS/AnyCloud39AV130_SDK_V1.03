// SPDX-License-Identifier: GPL-2.0+
/*
 * Faraday FTMAC100 Ethernet
 *
 * (C) Copyright 2009 Faraday Technology
 * Po-Yu Chuang <ratbert@faraday-tech.com>
 */

#include <common.h>
#include <config.h>
#include <env.h>
#include <linux/io.h>
#include <malloc.h>
#include <net.h>
#ifdef CONFIG_DM_ETH
#include <dm.h>
DECLARE_GLOBAL_DATA_PTR;
#endif
#include <asm/dma-mapping.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <clk.h>
#include <console.h>
#include <dm/pinctrl.h>
#include <linux/bug.h>
#include <linux/errno.h>
#include <linux/mii.h>
#include <linux/netdevice.h>
#include <miiphy.h>
#include <reset.h>
#include <wait_bit.h>

#ifdef CONFIG_39EV33X_CODE
#include <asm/arch-ak39ev33x/ak_cpu.h>
#include <asm/arch-ak39ev33x/ak_module_reset.h>
#endif

#ifdef CONFIG_37_D_CODE
#include <asm/arch-ak37d/ak_cpu.h>
#include <asm/arch-ak37d/ak_module_reset.h>
#endif

#ifdef CONFIG_37_E_CODE
#include <asm/arch-ak37e/ak_cpu.h>
#include <asm/arch-ak37e/ak_module_reset.h>
#endif

#if defined(CONFIG_3918AV100_CODE)
#include <asm/arch-ak3918av100/ak_cpu.h>
#include <asm/arch-ak3918av100/ak_module_reset.h>
#endif

#if defined(CONFIG_3918AV130_CODE)
#include <asm/arch-ak3918av130/ak_cpu.h>
#include <asm/arch-ak3918av130/ak_module_reset.h>
#endif

#ifdef CONFIG_KM01A_CODE
#include <asm/arch-km01a/ak_cpu.h>
#include <asm/arch-km01a/ak_module_reset.h>
#endif

#ifdef CONFIG_3918EV300L_CODE
#include <asm/arch-ak3918ev300l/ak_cpu.h>
#include <asm/arch-ak3918ev300l/ak_module_reset.h>
#endif

#ifdef CONFIG_39EV200_CODE
#include <asm/arch-ak39ev200/ak_cpu.h>
#include <asm/arch-ak39ev200/ak_module_reset.h>
#endif

#include "ak_eth.h"

#if defined(CONFIG_KM01A_CODE) || defined(CONFIG_3918AV130_CODE)
#define AK_MAC0_BASE     (0x20240000)
#else
#define AK_MAC0_BASE     (0x20300000)
#endif
#define MDIO_PRTAD_NONE  (-1)
#define MDIO_DEVAD_NONE  (-1)
#define MDIO_EMULATE_C22 4
#define ETH_ZLEN         60
#define AK_PA_SYSCTRL    (0x08000000)
#define AK_PA_GPIOCTRL   (0x20170000)

#define MAX_ADDR_LEN 32 /* Largest hardware address length */
#define MAC_ADDR_LEN 6

/* MACIF_CTRL */
#define RGMII_SPEED_1000 0x2c
#define RGMII_SPEED_100  0x2f
#define RGMII_SPEED_10   0x2d
#define MII_SPEED_100    0x0f
#define MII_SPEED_10     0x0d
#define GMAC_SPEED_1000  0x05
#define GMAC_SPEED_100   0x01
#define GMAC_SPEED_10    0x00
#define GMAC_FULL_DUPLEX BIT(4)

/*SynopGMAC can support up to 32 phys*/
enum GMACPhyBase {
    PHY0  = 0,
    PHY1  = 1,
    PHY31 = 31,
};

#define DEFAULT_PHY_BASE PHY1
#define MACBASE          0x0000
#define DMABASE          0x1000

struct ak_mac_priv {
    gmac_device* gmacdev_pt;
    DmaDesc* txfirst_desc;
    DmaDesc* rxfirst_desc;
    unsigned char dev_addr[MAC_ADDR_LEN];
    unsigned char broadcast[MAX_ADDR_LEN];
    struct eth_device* netdev_pt;
    struct net_device_stats net_dev_stats;
    void __iomem* base;
    int macif_ctrl;
    struct mii_dev* bus;
    struct phy_device* phydev;
    int phyintf; /* phy de rmii interface */
    int phyaddr; /* phy address */
    int phy_reset;
    struct udevice* dev;
    int inner_phy_mode;
};

static void ak_mac_init_tx_desc_base(gmac_device* gmacdev);
static u32 ak_mac_init_rx_desc_base(gmac_device* gmacdev);
static s32 ak_setup_tx_desc_queue(
    gmac_device* gmacdev, u32 no_of_desc, u32 desc_mode);
static s32 ak_setup_rx_desc_queue(
    gmac_device* gmacdev, u32 no_of_desc, u32 desc_mode);
static int ak_mac_reset_phy(struct ak_mac_priv* priv);
static s32 ak_mac_set_mdc_clk_div(gmac_device* gmacdev, u32 clk_div_val);
static u32 ak_mac_get_mdc_clk_div(gmac_device* gmacdev);
static void ak_gmac_clr_bits(u32* RegBase, u32 RegOffset, u32 BitPos);
static void ak_gmac_set_bits(u32* RegBase, u32 RegOffset, u32 BitPos);
static bool ak_gmac_is_last_rx_desc(gmac_device* gmacdev, DmaDesc* desc);
static void ak_gmac_rx_desc_init_ring3(
    u32 index, DmaDesc* desc, bool last_ring_desc);
static void smc_desc_set_rx_owner(DmaDesc* desc);

static void clean_invalidate_dcache_all(void)
{
#if CONFIG_IS_ENABLED(SYS_THUMB_BUILD)
    flush_dcache_all();
    // invalidate_dcache_all();
#else
    asm("MMU_Clean_Invalidate_Dcache:\n"
        "mrc  p15,0,r15,c7,c14,3\n"
        "bne MMU_Clean_Invalidate_Dcache");
#endif
}

static void ak_gmac_write_reg(u32* regbase, u32 regoffset, u32 regdata)
{
    u32 addr = (u32)regbase + regoffset;
    __raw_writel(regdata, (void*)addr);

    return;
}

static u32 ak_gmac_read_reg(u32* regbase, u32 regoffset)
{
    u32 addr = (u32)regbase + regoffset;
    u32 data = __raw_readl((void*)addr);

    return data;
}

static void ak_plat_delay(u32 delay)
{
    while (delay--)
        ;
    return;
}

static int ak_gmac_read_phy_reg(
    u32* RegBase, u32 PhyBase, u32 RegOffset, u16* data)
{
    u32 addr;
    u32 loop_variable;

    addr = ((PhyBase << GmiiDevShift) & GmiiDevMask)
        | ((RegOffset << GmiiRegShift) & GmiiRegMask);
#if defined(CONFIG_3918AV100_CODE) || defined(CONFIG_3918EV300L_CODE) \
 || defined(CONFIG_KM01A_CODE) || defined(CONFIG_3918AV130_CODE)
    addr = addr | GmiiCsrClk4 | GmiiBusy;
#else
    addr = addr | GmiiCsrClk1 | GmiiBusy;
#endif
    ak_gmac_write_reg(RegBase, GmacGmiiAddr, addr);

    for (loop_variable = 0; loop_variable < DEFAULT_LOOP_VARIABLE;
         loop_variable++) {
        if (!(ak_gmac_read_reg(RegBase, GmacGmiiAddr) & GmiiBusy)) {
            break;
        }
        ak_plat_delay(DEFAULT_DELAY_VARIABLE);
    }

    if (loop_variable < DEFAULT_LOOP_VARIABLE) {
        *data = (u16)(ak_gmac_read_reg(RegBase, GmacGmiiData) & 0xFFFF);
    } else {
        return -ESYNOPGMACPHYERR;
    }

    return -ESYNOPGMACNOERR;
}

static int ak_gmac_write_phy_reg(
    u32* RegBase, u32 PhyBase, u32 RegOffset, u16 data)
{
    u32 addr;
    u32 loop_variable;

    ak_gmac_write_reg(RegBase, GmacGmiiData, data);

    addr = ((PhyBase << GmiiDevShift) & GmiiDevMask)
        | ((RegOffset << GmiiRegShift) & GmiiRegMask) | GmiiWrite;
#if defined(CONFIG_3918AV100_CODE) || defined(CONFIG_3918EV300L_CODE) \
 || defined(CONFIG_KM01A_CODE) || defined(CONFIG_3918AV130_CODE)
    addr = addr | GmiiCsrClk4 | GmiiBusy;
#else
    addr = addr | GmiiCsrClk1 | GmiiBusy;
#endif
    ak_gmac_write_reg(RegBase, GmacGmiiAddr, addr);

    for (loop_variable = 0; loop_variable < DEFAULT_LOOP_VARIABLE;
         loop_variable++) {
        if (!(ak_gmac_read_reg(RegBase, GmacGmiiAddr) & GmiiBusy)) {
            break;
        }
        ak_plat_delay(DEFAULT_DELAY_VARIABLE);
    }

    if (loop_variable < DEFAULT_LOOP_VARIABLE) {
        return -ESYNOPGMACNOERR;
    }

    pr_err("read phy time out err!\n");
    return -ESYNOPGMACPHYERR;
}

static s32 ak_gmac_reset(gmac_device* gmacdev)
{
    u32 data      = 0;
    s32 reset_cnt = 0xFFFF;

    /* 
    *software reset , the resets all of the GMAC internal registers and logic
    */
    ak_gmac_write_reg(
        (u32*)gmacdev->DmaBase, DmaBusMode, DmaResetOn); // reset dma engine
    ak_plat_delay(DEFAULT_LOOP_VARIABLE);

    while (reset_cnt > 0) {
        data = ak_gmac_read_reg((u32*)gmacdev->DmaBase, DmaBusMode);

        /*
        *after finish become 0 
        */
        if ((data & DmaResetOn) != DmaResetOn) {
            break;
        }
        reset_cnt--;
    }

    if (reset_cnt <= 0) {
        pr_err("No find phy small board!\n");
        return -1;
    }

    return 0;
}

/*
 * Reset MAC
 */
static void ak_mac_reset(struct ak_mac_priv* priv)
{
    if (priv->macif_ctrl == AK_MAC0_IF) {
#ifndef CONFIG_3918AV100_CODE
        ak_soft_reset(AK39_SRESET_MAC);
#else
        /*
         * H322 reset MAC后，和MAC连接的DMA
         * Buffer复位不彻底，导致FIFO指针乱了，触发reset后把DDR控制器也复位了
         * reset MAC第一次释放后，就不能再reset了
         */
        REG32(MODULE_RESET_CON1) &= ~(0x1 << AK39_SRESET_MAC);
#endif
    }
#ifdef CONFIG_37_E_CODE
    if (priv->macif_ctrl == AK_MAC1_IF) {
        ak_soft_reset(AK39_SRESET_MAC1);
    }
#endif
}

static s32 ak_gmac_set_mac_addr(
    gmac_device* gmacdev, u32 MacHigh, u32 MacLow, u8* MacAddr)
{
    u32 data;

    data = (MacAddr[5] << 8) | MacAddr[4];
    ak_gmac_write_reg((u32*)gmacdev->MacBase, MacHigh, data);
    data = (MacAddr[3] << 24) | (MacAddr[2] << 16) | (MacAddr[1] << 8)
        | MacAddr[0];
    ak_gmac_write_reg((u32*)gmacdev->MacBase, MacLow, data);

    return 0;
}

static s32 ak_gmac_get_mac_addr(
    gmac_device* gmacdev, u32 MacHigh, u32 MacLow, u8* MacAddr)
{
    u32 data;

    data       = ak_gmac_read_reg((u32*)gmacdev->MacBase, MacHigh);
    MacAddr[5] = (data >> 8) & 0xff;
    MacAddr[4] = (data)&0xff;
    data       = ak_gmac_read_reg((u32*)gmacdev->MacBase, MacLow);
    MacAddr[3] = (data >> 24) & 0xff;
    MacAddr[2] = (data >> 16) & 0xff;
    MacAddr[1] = (data >> 8) & 0xff;
    MacAddr[0] = (data)&0xff;

    return 0;
}

static s32 ak_gmac_read_version(gmac_device* gmacdev)
{
    u32 data = 0;

    /*
    *available the mac ip version 
    */
    data             = ak_gmac_read_reg((u32*)gmacdev->MacBase, GmacVersion);
    gmacdev->Version = data;

    return 0;
}

/*
 * Set MAC address
 */
static void ak_mac_set_mac(struct ak_mac_priv* priv, unsigned char* mac)
{
    u8 macstr[20];

    memset(macstr, 0x0, 20);

    /*
    *program flash in the station IP's Mac address 
    */
    ak_gmac_set_mac_addr(priv->gmacdev_pt, GmacAddr0High, GmacAddr0Low, mac);

    /*
    *Lets set ipaddress in to device structure 
    */
    ak_gmac_get_mac_addr(
        priv->gmacdev_pt, GmacAddr0High, GmacAddr0Low, (u8*)macstr);

    /*
    *Lets read the version of ip 
    */
    ak_gmac_read_version(priv->gmacdev_pt);
}

/**
 * @brief Disable the transmission of frames on GMII/MII.
 *
 * GMAC transmit state machine is disabled after completion of transmission of
 * current frame.
 * @author CaoDonghua
 * @date 2016-09-26
 * @param[in] pointer to gmac_device.
 * @return returns void.
 * @retval none
 */
static void ak_gmac_tx_disable(gmac_device* gmacdev)
{
    ak_gmac_clr_bits((u32*)gmacdev->MacBase, GmacConfig, GmacTx);
    return;
}

/**
 * @brief Disable the reception of frames on GMII/MII.
 *
 * GMAC receive state machine is disabled after completion of reception of
 * current frame.
 * @author CaoDonghua
 * @date 2016-09-26
 * @param[in] pointer to gmac_device.
 * @return returns void.
 * @retval none
 */
static void ak_gmac_rx_disable(gmac_device* gmacdev)
{
    ak_gmac_clr_bits((u32*)gmacdev->MacBase, GmacConfig, GmacRx);

    return;
}

/**
 * @brief disable the DMA for transmission.
 *
 * disable the DMA for transmission.
 * @author CaoDonghua
 * @date 2016-09-26
 * @param[in] gmacdev gmac device.
 * @return void.
 * @retval none
 */
static void ak_gmac_disable_dma_tx(gmac_device* gmacdev)
{
    u32 data;

    data = ak_gmac_read_reg((u32*)gmacdev->DmaBase, DmaControl);
    data &= (~DmaTxStart);
    ak_gmac_write_reg((u32*)gmacdev->DmaBase, DmaControl, data);
}

/**
 * @brief disable the DMA for reception.
 *
 * disable the DMA controller for reception.
 * @author CaoDonghua
 * @date 2016-09-26
 * @param[in] gmacdev gmac device.
 * @return void.
 * @retval none
 */
static void ak_gmac_disable_dma_rx(gmac_device* gmacdev)
{
    u32 data;

    data = ak_gmac_read_reg((u32*)gmacdev->DmaBase, DmaControl);
    data &= (~DmaRxStart);
    ak_gmac_write_reg((u32*)gmacdev->DmaBase, DmaControl, data);
}

/**
 * @brief disable MAC.
 *
 * disable the DMA controller for reception.
 * @author CaoDonghua
 * @date 2016-09-26
 * @param[in] gmacdev gmac device.
 * @return void.
 * @retval none
 */
static void _ak_mac_halt(struct ak_mac_priv* priv)
{
    gmac_device* gmacdev;
    gmacdev = (gmac_device*)priv->gmacdev_pt;

    /*
    *disable gmac controller tx/rx 
    */
    ak_gmac_tx_disable(gmacdev);
    ak_gmac_rx_disable(gmacdev);

    /* 
    *disable gmac dma tx/rx 
    */
    ak_gmac_disable_dma_tx(gmacdev);
    ak_gmac_disable_dma_rx(gmacdev);

    /*
    *must set to 0, or when started up will cause issues 
    */
    gmacdev->TxNext = 0;
    gmacdev->RxNext = 0;

    /*
    *free rx dma desc ring buffer 
    */
    if (gmacdev->RxDescDma) {
        dma_free_coherent((void*)gmacdev->RxDescDma);
        gmacdev->RxDescDma = (dma_addr_t)NULL;
    }

    /*
    *free tx dma desc ring buffer 
    */
    if (gmacdev->TxDescDma) {
        dma_free_coherent((void*)gmacdev->TxDescDma);
        gmacdev->TxDescDma = (dma_addr_t)NULL;
    }
}

/**
 * @brief gmac clear bits.
 *
 * @author CaoDonghua
 * @date 2016-09-26
 * @param[in] gmac RegBase.
 * @return void.
 * @retval none
 */
static void ak_gmac_clr_bits(u32* RegBase, u32 RegOffset, u32 BitPos)
{
    u32 addr = (u32)RegBase + RegOffset;
    u32 data = __raw_readl((void*)addr);

    data &= (~BitPos);
    __raw_writel(data, (void*)addr);

    return;
}

/**
 * @brief The Low level function to set bits of a register in Hardware.
 *
 * @author CaoDonghua
 * @date 2016-09-26
 * @param[in] pointer to the base of register map
 * @param[in] Offset from the base
 * @param[in] Bit mask to set bits to logical 1
 * @return  void
 * @retval none
 */
static void ak_gmac_set_bits(u32* RegBase, u32 RegOffset, u32 BitPos)
{
    u32 addr = (u32)RegBase + RegOffset;
    u32 data = __raw_readl((void*)addr);

    data |= BitPos;
    __raw_writel(data, (void*)addr);

    return;
}

static void ak_gmac_clear_interrupt(gmac_device* gmacdev)
{
    u32 data;

    data = ak_gmac_read_reg((u32*)gmacdev->DmaBase, DmaStatus);
    ak_gmac_write_reg((u32*)gmacdev->DmaBase, DmaStatus, data);
}

static void ak_gmac_disable_mmc_tx_interrupt(gmac_device* gmacdev, u32 mask)
{
    ak_gmac_set_bits((u32*)gmacdev->MacBase, GmacMmcIntrMaskTx, mask);

    return;
}

static void ak_gmac_disable_mmc_rx_interrupt(gmac_device* gmacdev, u32 mask)
{
    ak_gmac_set_bits((u32*)gmacdev->MacBase, GmacMmcIntrMaskRx, mask);

    return;
}

static void ak_gmac_disable_mmc_ipc_rx_interrupt(gmac_device* gmacdev, u32 mask)
{
    ak_gmac_set_bits((u32*)gmacdev->MacBase, GmacMmcRxIpcIntrMask, mask);

    return;
}

static s32 ak_gmac_set_dma_bus_mode(gmac_device* gmacdev, u32 init_value)
{
    ak_gmac_write_reg((u32*)gmacdev->DmaBase, DmaBusMode, init_value);

    return 0;
}

static s32 ak_gmac_set_dma_control(gmac_device* gmacdev, u32 init_value)
{
    ak_gmac_write_reg((u32*)gmacdev->DmaBase, DmaControl, init_value);

    return 0;
}

static void ak_gmac_pause_control(gmac_device* gmacdev)
{
    u32 omr_reg;
    u32 mac_flow_control_reg;

    omr_reg = ak_gmac_read_reg((u32*)gmacdev->DmaBase, DmaControl);
    omr_reg |= DmaRxFlowCtrlAct4K | DmaRxFlowCtrlDeact5K | DmaEnHwFlowCtrl;
    ak_gmac_write_reg((u32*)gmacdev->DmaBase, DmaControl, omr_reg);

    mac_flow_control_reg
        = ak_gmac_read_reg((u32*)gmacdev->MacBase, GmacFlowControl);
    mac_flow_control_reg |= GmacRxFlowControl | GmacTxFlowControl | 0xFFFF0000;
    ak_gmac_write_reg(
        (u32*)gmacdev->MacBase, GmacFlowControl, mac_flow_control_reg);

    return;
}

static void ak_gmac_enable_dma_rx(gmac_device* gmacdev)
{
    u32 data;

    data = ak_gmac_read_reg((u32*)gmacdev->DmaBase, DmaControl);
    data |= DmaRxStart;
    ak_gmac_write_reg((u32*)gmacdev->DmaBase, DmaControl, data);
}

/**
 * @brief enable the DMA transmission.
 *
 * enable the DMA controller transmission..
 * @author CaoDonghua
 * @date 2016-09-26
 * @param[in] gmacdev gmac device
 * @return void.
 * @retval none
 */
static void ak_gmac_enable_dma_tx(gmac_device* gmacdev)
{
    u32 data;

    data = ak_gmac_read_reg((u32*)gmacdev->DmaBase, DmaControl);
    data |= DmaTxStart;
    ak_gmac_write_reg((u32*)gmacdev->DmaBase, DmaControl, data);
}

static void ak_gmac_mmc_counters_reset(gmac_device* gmacdev)
{
    ak_gmac_clr_bits((u32*)gmacdev->MacBase, GmacMmcCntrl, GmacMmcCounterReset);

    return;
}

static void ak_gmac_mmc_counters_disable_rollover(gmac_device* gmacdev)
{
    ak_gmac_set_bits(
        (u32*)gmacdev->MacBase, GmacMmcCntrl, GmacMmcCounterStopRollover);

    return;
}

static void ak_gmac_wd_enable(gmac_device* gmacdev)
{
    ak_gmac_clr_bits((u32*)gmacdev->MacBase, GmacConfig, GmacWatchdog);
    return;
}

static void ak_gmac_jab_enable(gmac_device* gmacdev)
{
    ak_gmac_set_bits((u32*)gmacdev->MacBase, GmacConfig, GmacJabber);
    return;
}

static void ak_gmac_frame_burst_enable(gmac_device* gmacdev)
{
    ak_gmac_set_bits((u32*)gmacdev->MacBase, GmacConfig, GmacFrameBurst);
    return;
}

static void ak_gmac_jumbo_frame_disable(gmac_device* gmacdev)
{
    ak_gmac_clr_bits((u32*)gmacdev->MacBase, GmacConfig, GmacJumboFrame);
    return;
}

static void ak_gmac_rx_own_enable(gmac_device* gmacdev)
{
    ak_gmac_clr_bits((u32*)gmacdev->MacBase, GmacConfig, GmacRxOwn);
    return;
}

static void ak_gmac_loopback_off(gmac_device* gmacdev)
{
    ak_gmac_clr_bits((u32*)gmacdev->MacBase, GmacConfig, GmacLoopback);
    return;
}

static void ak_gmac_set_full_duplex(gmac_device* gmacdev)
{
    ak_gmac_set_bits((u32*)gmacdev->MacBase, GmacConfig, GmacDuplex);
    return;
}

static void ak_gmac_retry_enable(gmac_device* gmacdev)
{
    ak_gmac_clr_bits((u32*)gmacdev->MacBase, GmacConfig, GmacRetry);
    return;
}

static void ak_gmac_pad_crc_strip_disable(gmac_device* gmacdev)
{
    ak_gmac_clr_bits((u32*)gmacdev->MacBase, GmacConfig, GmacPadCrcStrip);
    return;
}

static void ak_gmac_back_off_limit(gmac_device* gmacdev, u32 value)
{
    u32 data;

    data = ak_gmac_read_reg((u32*)gmacdev->MacBase, GmacConfig);
    data &= (~GmacBackoffLimit);
    data |= value;
    ak_gmac_write_reg((u32*)gmacdev->MacBase, GmacConfig, data);
    return;
}

static void ak_gmac_deferral_check_disable(gmac_device* gmacdev)
{
    ak_gmac_clr_bits((u32*)gmacdev->MacBase, GmacConfig, GmacDeferralCheck);
    return;
}

static void ak_gmac_tx_enable(gmac_device* gmacdev)
{
    ak_gmac_set_bits((u32*)gmacdev->MacBase, GmacConfig, GmacTx);
    return;
}

static void ak_gmac_rx_enable(gmac_device* gmacdev)
{
    ak_gmac_set_bits((u32*)gmacdev->MacBase, GmacConfig, GmacRx);
    return;
}

static void ak_gmac_select_gmii(gmac_device* gmacdev)
{
    ak_gmac_clr_bits((u32*)gmacdev->MacBase, GmacConfig, GmacMiiGmii);
    return;
}

static void ak_gmac_select_mii(gmac_device* gmacdev)
{
    ak_gmac_set_bits((u32*)gmacdev->MacBase, GmacConfig, GmacMiiGmii);
    return;
}

static void ak_gmac_frame_filter_enable(gmac_device* gmacdev)
{
    ak_gmac_clr_bits((u32*)gmacdev->MacBase, GmacFrameFilter, GmacFilter);
    return;
}

static void ak_gmac_set_pass_control(gmac_device* gmacdev, u32 passcontrol)
{
    u32 data;

    data = ak_gmac_read_reg((u32*)gmacdev->MacBase, GmacFrameFilter);
    data &= (~GmacPassControl);
    data |= passcontrol;
    ak_gmac_write_reg((u32*)gmacdev->MacBase, GmacFrameFilter, data);
    return;
}

static void ak_gmac_broadcast_enable(gmac_device* gmacdev)
{
    ak_gmac_clr_bits((u32*)gmacdev->MacBase, GmacFrameFilter, GmacBroadcast);
    return;
}

static void ak_gmac_src_addr_filter_disable(gmac_device* gmacdev)
{
    ak_gmac_clr_bits(
        (u32*)gmacdev->MacBase, GmacFrameFilter, GmacSrcAddrFilter);
    return;
}

static void ak_gmac_multicast_disable(gmac_device* gmacdev)
{
    ak_gmac_clr_bits(
        (u32*)gmacdev->MacBase, GmacFrameFilter, GmacMulticastFilter);
    return;
}

static void ak_gmac_dst_addr_filter_normal(gmac_device* gmacdev)
{
    ak_gmac_clr_bits(
        (u32*)gmacdev->MacBase, GmacFrameFilter, GmacDestAddrFilterNor);
    return;
}

static void ak_gmac_multicast_hash_filter_enable(gmac_device* gmacdev)
{
    ak_gmac_set_bits(
        (u32*)gmacdev->MacBase, GmacFrameFilter, GmacMcastHashFilter);
    return;
}

static void ak_gmac_promisc_disable(gmac_device* gmacdev)
{
    ak_gmac_clr_bits(
        (u32*)gmacdev->MacBase, GmacFrameFilter, GmacPromiscuousMode);
    return;
}

static void ak_gmac_unicast_hash_filter_disable(gmac_device* gmacdev)
{
    ak_gmac_clr_bits(
        (u32*)gmacdev->MacBase, GmacFrameFilter, GmacUcastHashFilter);
    return;
}

static void ak_gmac_unicast_pause_frame_detect_disable(gmac_device* gmacdev)
{
    ak_gmac_clr_bits(
        (u32*)gmacdev->MacBase, GmacFlowControl, GmacUnicastPauseFrame);
    return;
}

static void ak_gmac_rx_flow_control_enable(gmac_device* gmacdev)
{
    ak_gmac_set_bits(
        (u32*)gmacdev->MacBase, GmacFlowControl, GmacRxFlowControl);
    return;
}

static void ak_gmac_tx_flow_control_enable(gmac_device* gmacdev)
{
    ak_gmac_set_bits(
        (u32*)gmacdev->MacBase, GmacFlowControl, GmacTxFlowControl);
    return;
}

static void ak_gmac_set_half_duplex(gmac_device* gmacdev)
{
    ak_gmac_clr_bits((u32*)gmacdev->MacBase, GmacConfig, GmacDuplex);
    return;
}

static s32 ak_gmac_mac_init(gmac_device* gmacdev)
{
    u32 PHYreg;

    if (gmacdev->DuplexMode == FULLDUPLEX) {
        ak_gmac_wd_enable(gmacdev);
        ak_gmac_jab_enable(gmacdev);
        ak_gmac_frame_burst_enable(gmacdev);
        ak_gmac_jumbo_frame_disable(gmacdev);
        ak_gmac_rx_own_enable(gmacdev);
        ak_gmac_loopback_off(gmacdev);
        ak_gmac_set_full_duplex(gmacdev);
        ak_gmac_retry_enable(gmacdev);
        ak_gmac_pad_crc_strip_disable(gmacdev);
        ak_gmac_back_off_limit(gmacdev, GmacBackoffLimit0);
        ak_gmac_deferral_check_disable(gmacdev);
        ak_gmac_tx_enable(gmacdev);
        ak_gmac_rx_enable(gmacdev);

        if (gmacdev->Speed == SPEED1000) {
            ak_gmac_select_gmii(gmacdev);
        } else {
            ak_gmac_select_mii(gmacdev);
        }

        /*Frame Filter Configuration*/
        ak_gmac_frame_filter_enable(gmacdev);
        ak_gmac_set_pass_control(gmacdev, GmacPassControl0);
        ak_gmac_broadcast_enable(gmacdev);
        ak_gmac_src_addr_filter_disable(gmacdev);
        ak_gmac_multicast_disable(gmacdev);
        ak_gmac_dst_addr_filter_normal(gmacdev);
        ak_gmac_multicast_hash_filter_enable(gmacdev);
        ak_gmac_promisc_disable(gmacdev);
        ak_gmac_unicast_hash_filter_disable(gmacdev);

        /*Flow Control Configuration*/
        ak_gmac_unicast_pause_frame_detect_disable(gmacdev);
        ak_gmac_rx_flow_control_enable(gmacdev);
        ak_gmac_tx_flow_control_enable(gmacdev);
    } else {
        ak_gmac_wd_enable(gmacdev);
        ak_gmac_jab_enable(gmacdev);
        ak_gmac_frame_burst_enable(gmacdev);
        ak_gmac_jumbo_frame_disable(gmacdev);
        ak_gmac_rx_own_enable(gmacdev);
        ak_gmac_loopback_off(gmacdev);
        ak_gmac_set_half_duplex(gmacdev);
        ak_gmac_retry_enable(gmacdev);
        ak_gmac_pad_crc_strip_disable(gmacdev);
        ak_gmac_back_off_limit(gmacdev, GmacBackoffLimit0);
        ak_gmac_deferral_check_disable(gmacdev);
        ak_gmac_tx_enable(gmacdev);
        ak_gmac_rx_enable(gmacdev);

        if (gmacdev->Speed == SPEED1000)
            ak_gmac_select_gmii(gmacdev);
        else
            ak_gmac_select_mii(gmacdev);

        /*Frame Filter Configuration*/
        ak_gmac_frame_filter_enable(gmacdev);
        ak_gmac_set_pass_control(gmacdev, GmacPassControl0);
        ak_gmac_broadcast_enable(gmacdev);
        // ak_gmac_broadcast_disable(gmacdev);
        ak_gmac_src_addr_filter_disable(gmacdev);
        ak_gmac_multicast_disable(gmacdev);
        ak_gmac_dst_addr_filter_normal(gmacdev);
        ak_gmac_multicast_hash_filter_enable(gmacdev);
        ak_gmac_promisc_disable(gmacdev);
        ak_gmac_unicast_hash_filter_disable(gmacdev);

        /*Flow Control Configuration*/
        ak_gmac_unicast_pause_frame_detect_disable(gmacdev);
        ak_gmac_rx_flow_control_enable(gmacdev);
        ak_gmac_tx_flow_control_enable(gmacdev);

        /*
        *To set PHY register to enable CRS on Transmit
        */
        ak_gmac_write_reg(
            (u32*)gmacdev->MacBase, GmacGmiiAddr, GmiiBusy | 0x00000408);
        PHYreg = ak_gmac_read_reg((u32*)gmacdev->MacBase, GmacGmiiData);
        ak_gmac_write_reg(
            (u32*)gmacdev->MacBase, GmacGmiiData, PHYreg | 0x00000800);
        ak_gmac_write_reg(
            (u32*)gmacdev->MacBase, GmacGmiiAddr, GmiiBusy | 0x0000040a);
    }

    return 0;
}
/*end of ak_gmac_mac_init*/

/*
 * Initialize MAC
 */
static int _ak_mac_init(struct ak_mac_priv* priv, unsigned char enetaddr[6])
{
    pr_debug("%s()\n", __func__);

    ak_mac_reset(priv);

    /*
    *now platform dependent initialization 
    */
    ak_gmac_mmc_counters_reset(priv->gmacdev_pt);
    ak_gmac_mmc_counters_disable_rollover(priv->gmacdev_pt);

    /* 
    *software reset , the resets all of the GMAC internal registers and logic
     */
    if (ak_gmac_reset(priv->gmacdev_pt)) {
        pr_err("%s, no find phy device!\n", __func__);
        return -1;
    }

    /*
    *set the ethernet address 
    */
    ak_mac_set_mac(priv, enetaddr);

    ak_mac_set_mdc_clk_div(priv->gmacdev_pt, GmiiCsrClk4);

    /*
    *Initialize tx hardware queues 
    */
    ak_setup_tx_desc_queue(priv->gmacdev_pt, TRANSMIT_DESC_SIZE, RINGMODE);

    /*
    *transmit ring 
    */
    ak_mac_init_tx_desc_base(priv->gmacdev_pt);

    /*
    *Initialize rx hardware queues 
    */
    ak_setup_rx_desc_queue(priv->gmacdev_pt, RECEIVE_DESC_SIZE, RINGMODE);

    /*
    *receive ring 
    */
    ak_mac_init_rx_desc_base(priv->gmacdev_pt);

    /*
    *dma busrt=32words=128B, two dma_descriptor interval = 2Bytes  
    */
    ak_gmac_set_dma_bus_mode(
        priv->gmacdev_pt, DmaBurstLength32 | DmaDescriptorSkip2);

    /*
    *set dma transmit method 
    */
    ak_gmac_set_dma_control(priv->gmacdev_pt,
        DmaDisableFlush | DmaStoreAndForward | DmaTxSecondFrame
            | DmaRxThreshCtrl128);

    /*
    *initial phy register and mac part ip 
    */
    ak_gmac_mac_init(priv->gmacdev_pt);

    /**
     *inital dma and mac flow control
     */
    ak_gmac_pause_control(priv->gmacdev_pt);

    /**
     *clear all the interrupts
     */
    ak_gmac_clear_interrupt(priv->gmacdev_pt);

    /**
     *Disable the interrupts generated by MMC and IPC counters.
     *If these are not disabled ISR should be modified accordingly to handle
     *these interrupts.
     */
    ak_gmac_disable_mmc_tx_interrupt(priv->gmacdev_pt, 0xFFFFFFFF);
    ak_gmac_disable_mmc_rx_interrupt(priv->gmacdev_pt, 0xFFFFFFFF);
    ak_gmac_disable_mmc_ipc_rx_interrupt(priv->gmacdev_pt, 0xFFFFFFFF);

    /**
     *Enable Tx and Rx DMA
     */
    ak_gmac_enable_dma_rx(priv->gmacdev_pt);
    ak_gmac_enable_dma_tx(priv->gmacdev_pt);

    return 0;
}

static int ak_smc_desc_get_rx_frame_len(DmaDesc* desc)
{
    u32 data = desc->status;
    u32 len  = (data & DescFrameLengthMask) >> DescFrameLengthShift;

    return len;
}

static bool ak_gmac_is_last_rx_desc(gmac_device* gmacdev, DmaDesc* desc)
{
    return ((desc->length & RxDescEndOfRing) == RxDescEndOfRing);
}

static void ak_gmac_rx_desc_init_ring3(
    u32 index, DmaDesc* desc, bool last_ring_desc)
{
    desc->length |= last_ring_desc ? RxDescEndOfRing : 0;

    return;
}

static void smc_desc_set_rx_owner(DmaDesc* desc)
{
    desc->status = DescOwnByDma;
    clean_invalidate_dcache_all();
}

static void ak_gmac_resume_dma_rx(gmac_device* gmacdev)
{
    ak_gmac_write_reg((u32*)gmacdev->DmaBase, DmaRxPollDemand, 0);
}

static int smc_desc_get_owner(DmaDesc* desc)
{

    clean_invalidate_dcache_all();
    return (desc->status & DescOwnByDma);
}

/*
 * Receive a data block via Ethernet
 */
static int _ak_mac_recv(struct ak_mac_priv* priv, uchar** packetp)
{
    gmac_device* gmacdev;
    u32 rxnext;
    DmaDesc* rxdesc;
    int length = 0;

    gmacdev = priv->gmacdev_pt;
    rxnext  = gmacdev->RxNext;
    rxdesc  = gmacdev->rx_ring[rxnext];

    /*
     * make sure we see the changes made by the DMA engine
     * check if the host(cpu)  has the desc,  no data receive, must wait for
     */
    if (smc_desc_get_owner(rxdesc)) {
        /* something bad happened */
        return 0;
    }

    /*
    *crc len 4B, so must decrease 4Byte 
    */
    length = ak_smc_desc_get_rx_frame_len(rxdesc) - 4;
    if (length)
        *packetp = (uchar*)(u32)rxdesc->buffer1;

    /*
    *re-init gmac ring descriptor  
    */
    gmacdev->RxNext = ak_gmac_is_last_rx_desc(gmacdev, rxdesc) ? 0 : rxnext + 1;
    ak_gmac_rx_desc_init_ring3(
        gmacdev->RxNext, rxdesc, ak_gmac_is_last_rx_desc(gmacdev, rxdesc));

    /*
    *set descriptor back to owned by XGMAC 
    */
    smc_desc_set_rx_owner(rxdesc);

    /*
    *poll cmd start  
    */
    ak_gmac_resume_dma_rx(gmacdev);

    return length;
}

static bool gmac_is_desc_empty(DmaDesc* desc)
{
    return (((desc->length & DescSize1Mask) == 0)
        && ((desc->length & DescSize2Mask) == 0));
}

static s32 gmac_set_tx_qptr(gmac_device* gmacdev, u32 Buffer1, u32 Length1,
    u32 Data1, u32 Buffer2, u32 Length2, u32 Data2, u32 offload_needed)
{
    u32 txnext      = gmacdev->TxNext;
    DmaDesc* txdesc = gmacdev->tx_ring[txnext];

    if (!gmac_is_desc_empty(txdesc)) {
        pr_err("%s, line:%d\n", __func__, __LINE__);
        return -1;
    }

    txdesc->length |= (((Length1 << DescSize1Shift) & DescSize1Mask)
        | ((Length2 << DescSize2Shift) & DescSize2Mask));
    txdesc->length |= (DescTxFirst | DescTxLast
        | DescTxIntEnable); // Its always assumed that complete data will fit in
                            // to one descriptor
    txdesc->buffer1 = Buffer1;
    txdesc->data1   = Data1;
    txdesc->buffer2 = Buffer2;
    txdesc->data2   = Data2;
    txdesc->status  = DescOwnByDma;

    return txnext;
}

static void gmac_resume_dma_tx(gmac_device* gmacdev)
{
    ak_gmac_write_reg((u32*)gmacdev->DmaBase, DmaTxPollDemand, 0);
}

static bool gmac_is_last_tx_desc(gmac_device* gmacdev, DmaDesc* desc)
{
    return ((desc->length & TxDescEndOfRing) == TxDescEndOfRing);
}

static void gmac_tx_desc_init_ring(DmaDesc* desc, bool last_ring_desc)
{
    desc->status  = 0;
    desc->length  = last_ring_desc ? TxDescEndOfRing : 0;
    desc->buffer1 = 0;
    desc->buffer2 = 0;
    desc->data1   = 0;
    desc->data2   = 0;
    return;
}

/*
 * Send a data block via Ethernet
 */
static int _ak_mac_send(struct ak_mac_priv* priv, void* packet, int length)
{
    gmac_device* gmacdev;
    int timeout;
    u32 txnext;
    DmaDesc* txdesc;

    length = (length < ETH_ZLEN) ? ETH_ZLEN : length;

    gmacdev = priv->gmacdev_pt;

    /*
    *set tx descriptor  for ready transmit
    */
    gmac_set_tx_qptr(
        gmacdev, (u32)packet, (u32)length, (u32)packet, 0, 0, 0, 0);
    clean_invalidate_dcache_all();

    /*
    *write poll demand start transmit 
    */
    gmac_resume_dma_tx(gmacdev);

    txnext = gmacdev->TxNext;
    txdesc = gmacdev->tx_ring[txnext];

    /*
    *wait for transmit finish 
    */
    timeout = 1000000;

    while (smc_desc_get_owner(txdesc)) {
        if (timeout-- < 0) {
            pr_err("gmac: tx timeout\n");
            return -ETIMEDOUT;
        }
        udelay(1);
    }

    /*
    *update mac controller transmit descriptor ring 
    */
    gmacdev->TxNext = gmac_is_last_tx_desc(gmacdev, txdesc) ? 0 : txnext + 1;
    gmac_tx_desc_init_ring(txdesc, gmac_is_last_tx_desc(gmacdev, txdesc));

    return 0;
}

static int ak_mac_adjust_link(struct ak_mac_priv* priv)
{
    struct phy_device* phydev = priv->phydev;

    priv->gmacdev_pt->DuplexMode
        = (u32)phydev->duplex ? FULLDUPLEX : HALFDUPLEX;
    priv->gmacdev_pt->Speed = (u32)phydev->speed == 100 ? SPEED100 : SPEED10;

    return 0;
}

#ifdef CONFIG_DM_ETH
static int ak_mac_start(struct udevice* dev)
{
    struct eth_pdata* plat    = dev_get_platdata(dev);
    struct ak_mac_priv* priv  = dev_get_priv(dev);
    struct phy_device* phydev = priv->phydev;
    int ret;

#if 1//WQS
    u32 status = 0;
    u16 phy_regval;
    /*** 切换到mii寄存器下面 ***/
    ak_gmac_write_phy_reg((u32 *)priv->base, priv->phyaddr, 0x1e, 0x100);
    ak_gmac_read_phy_reg((u32 *)priv->base, priv->phyaddr, 0x1f, &phy_regval);
    phy_regval |= 0x02;
    ak_gmac_write_phy_reg((u32 *)priv->base, priv->phyaddr, 0x1e, 0x100);
    ak_gmac_write_phy_reg((u32 *)priv->base, priv->phyaddr, 0x1f, phy_regval);

    status = ak_gmac_read_phy_reg((u32 *)priv->base, priv->phyaddr, PHY_STATUS_REG, &phy_regval);
    if (!status)
    {
        if ((phy_regval & Mii_Link) == 0)
        {
            printf("phy_regval:0x%x  No Link!!!\n", phy_regval);
            // priv.LinkState = LINKDOWN;
        }
        else
        {
            // priv.LinkState = LINKUP;
            printf("phy_regval:0x%x  Link UP!!!!!!!!\n", phy_regval);
        }
    }
#endif
    ret = phy_startup(phydev);
    if (ret) {
        pr_err("%s, line:%d\n", __func__, __LINE__);
        return ret;
    }

    if (!phydev->link) {
        pr_err("%s: link down\n", phydev->dev->name);
        return -ENODEV;
    }

    ret = ak_mac_adjust_link(priv);
    if (ret) {
        return ret;
    }

    /*
    *Enable port 
    */
    ret = _ak_mac_init(priv, plat->enetaddr);

    return ret;
}

static void ak_mac_stop(struct udevice* dev)
{
    struct ak_mac_priv* priv = dev_get_priv(dev);

    /*
    *Disable port 
    */
    _ak_mac_halt(priv);
}

static int ak_mac_send(struct udevice* dev, void* packet, int length)
{
    struct ak_mac_priv* priv = dev_get_priv(dev);
    int ret;

    ret = _ak_mac_send(priv, packet, length);

    return ret ? 0 : -ETIMEDOUT;
}

static int ak_mac_recv(struct udevice* dev, int flags, uchar** packetp)
{
    struct ak_mac_priv* priv = dev_get_priv(dev);
    int len;

    len = _ak_mac_recv(priv, packetp);

    return len ? len : -EAGAIN;
}

static int ak_mac_write_hwaddr(struct udevice* dev)
{
    struct eth_pdata* pdata  = dev_get_platdata(dev);
    struct ak_mac_priv* priv = dev_get_priv(dev);
    unsigned char* mac       = pdata->enetaddr;
    gmac_device* gmacdev;
    u32 data;

    gmacdev = priv->gmacdev_pt;

    data = (mac[5] << 8) | mac[4];
    ak_gmac_write_reg((u32*)gmacdev->MacBase, GmacAddr0High, data);
    data = (mac[3] << 24) | (mac[2] << 16) | (mac[1] << 8) | mac[0];
    ak_gmac_write_reg((u32*)gmacdev->MacBase, GmacAddr0Low, data);

    return 0;
}

static const char* ak_dtbmacaddr(u32 ifno)
{
    int node, len;
    char enet[16];
    const char* mac;
    const char* path;

    if (gd->fdt_blob == NULL) {
        pr_err("%s: don't have a valid gd->fdt_blob!\n", __func__);
        return NULL;
    }

    node = fdt_path_offset(gd->fdt_blob, "/aliases");
    if (node < 0)
        return NULL;

    sprintf(enet, "ethernet%d", ifno);
    path = fdt_getprop(gd->fdt_blob, node, enet, NULL);
    if (!path) {
        pr_err("no alias for %s\n", enet);
        return NULL;
    }

    node = fdt_path_offset(gd->fdt_blob, path);
    mac  = fdt_getprop(gd->fdt_blob, node, "mac-address", &len);
    if (mac && is_valid_ethaddr((u8*)mac))
        return mac;

    return NULL;
}

static int ak_mac_ofdata_to_platdata(struct udevice* dev)
{
    struct ak_mac_priv* priv = dev_get_priv(dev);
    struct eth_pdata* pdata  = dev_get_platdata(dev);
    const char* mac;
    int phyintf = PHY_INTERFACE_MODE_NONE;
    const char* phy_mode;
    u32 macif_base;
    u32 mac_ifno;
    int ret;
    unsigned int array[3] = { 0 };

    priv->base = dev_remap_addr_index(dev, 0);
    macif_base = (u32)priv->base;
    if (!macif_base) {
        pr_err("%s, line:%d\n", __func__, __LINE__);
        return -ENODEV;
    }

    mac_ifno         = (macif_base == AK_MAC0_BASE) ? 0 : 1;
    priv->macif_ctrl = mac_ifno;

    mac = ak_dtbmacaddr(mac_ifno);
    if (mac)
        memcpy(pdata->enetaddr, mac, 6);

    phy_mode = dev_read_string(dev, "phy-mode");
    if (phy_mode) {
        phyintf = phy_get_interface_by_name(phy_mode);
    }

    if (phyintf == PHY_INTERFACE_MODE_NONE) {
        pr_err("%s, line:%d\n", __func__, __LINE__);
        return -ENODEV;
    }

    priv->phyintf = phyintf;
    priv->phyaddr = DEFAULT_PHY_BASE;
    dev_read_u32u(dev, "phy-address", (uint*)&priv->phyaddr);

    ret = fdtdec_get_int_array(
        gd->fdt_blob, dev_of_offset(dev), "phy-reset", array, 3);
    if (ret < 0) {
        pr_err("fdt err:%d, %s, line:%d\n", ret, __func__, __LINE__);
        return -EINVAL;
    }

    priv->phy_reset = array[1];

    priv->inner_phy_mode = 0;
    dev_read_u32u(dev, "inner-phy-mode", (uint*)&priv->inner_phy_mode);
    priv->inner_phy_mode = !!priv->inner_phy_mode;

    return 0;
}

static int ak_mdio_rmii_read(struct mii_dev* bus, int addr, int devad, int reg)
{
    struct ak_mac_priv* priv = bus->priv;
    u32 val;
    int ret;

    if (devad != MDIO_DEVAD_NONE)
        return -EOPNOTSUPP;

    ret = ak_gmac_read_phy_reg(
        (u32*)priv->base, (u32)addr, (u32)reg, (u16*)&val);
    if (ret < 0) {
        return ret;
    }

    return val & GENMASK(15, 0);
}

static int ak_mdio_rmii_write(
    struct mii_dev* bus, int addr, int devad, int reg, u16 value)
{
    struct ak_mac_priv* priv = bus->priv;
    int ret;

    if (devad != MDIO_DEVAD_NONE)
        return -EOPNOTSUPP;

    ret = ak_gmac_write_phy_reg((u32*)priv->base, (u32)addr, (u32)reg, value);
    if (ret < 0)
        return ret;

    return 0;
}

static void ak_mac_tx_desc_init_ring(DmaDesc* desc, bool last_ring_desc)
{
    desc->status  = 0;
    desc->length  = last_ring_desc ? TxDescEndOfRing : 0;
    desc->buffer1 = 0;
    desc->buffer2 = 0;
    desc->data1   = 0;
    desc->data2   = 0;

    return;
}

static s32 ak_setup_tx_desc_queue(
    gmac_device* gmacdev, u32 no_of_desc, u32 desc_mode)
{
    s32 i;

    DmaDesc* first_desc = NULL;
    dma_addr_t dma_addr;
    gmacdev->TxDescCount = 0;

    if (desc_mode == RINGMODE) {
        first_desc = (DmaDesc*)dma_alloc_coherent(
            sizeof(DmaDesc) * no_of_desc, (unsigned long*)&dma_addr);
        if (first_desc == NULL) {
            pr_err("Error in Tx Descriptors memory allocation\n");
            return -ESYNOPGMACNOMEM;
        }

        /*
        *ring descriptor count 
        */
        gmacdev->TxDescCount = no_of_desc;

        /*
        *ring descriptor base virtual address 
        */
        gmacdev->tx_ring[0] = first_desc;

        /*
        *ring descriptor base physic address 
        */
        gmacdev->TxDescDma = dma_addr;

        for (i = 0; i < gmacdev->TxDescCount; i++) {
            /* tx ring descriptor every one give base virtual address */
            gmacdev->tx_ring[i] = first_desc + i;

            /* why not init dma fifo start addresss??? */
            ak_mac_tx_desc_init_ring(
                gmacdev->tx_ring[i], i == gmacdev->TxDescCount - 1);
        }
    }

    gmacdev->TxNext     = 0;
    gmacdev->TxBusy     = 0;
    gmacdev->TxNextDesc = gmacdev->TxDesc;
    gmacdev->TxBusyDesc = gmacdev->TxDesc;
    gmacdev->BusyTxDesc = 0;
    flush_dcache_all();

    return -ESYNOPGMACNOERR;
}

static void ak_mac_init_tx_desc_base(gmac_device* gmacdev)
{
    /*
    *write TxDescDma consistent  physic address to mac dma ctrl reg 
    */
    ak_gmac_write_reg(
        (u32*)gmacdev->DmaBase, DmaTxBaseAddr, (u32)gmacdev->TxDescDma);

    return;
}

static void ak_mac_rx_desc_init_ring(
    u32 index, DmaDesc* desc, bool last_ring_desc)
{
    desc->status = 0;
    desc->length = last_ring_desc ? RxDescEndOfRing : 0;
    desc->length |= (((PKTSIZE_ALIGN << DescSize1Shift) & DescSize1Mask)
        | ((0 << DescSize2Shift) & DescSize2Mask));
    desc->buffer1 = (unsigned int)net_rx_packets[index];
    desc->buffer2 = 0;
    desc->data1   = 0;
    desc->data2   = 0;
    return;
}

static void ak_smc_desc_set_rx_owner(DmaDesc* desc)
{
    desc->status = DescOwnByDma;
}

static s32 ak_setup_rx_desc_queue(
    gmac_device* gmacdev, u32 no_of_desc, u32 desc_mode)
{
    s32 i;

    DmaDesc* first_desc = NULL;
    dma_addr_t dma_addr;
    gmacdev->RxDescCount = 0;

    if (desc_mode == RINGMODE) {
        first_desc = (DmaDesc*)dma_alloc_coherent(
            sizeof(DmaDesc) * no_of_desc, (unsigned long*)&dma_addr);
        if (first_desc == NULL) {
            pr_err("Error in Rx Descriptor Memory allocation\n");
            return -ESYNOPGMACNOMEM;
        }

        gmacdev->RxDescCount = no_of_desc;

        /* the first desc virtual addr */
        gmacdev->rx_ring[0] = first_desc;

        /* the first desc physic addr */
        gmacdev->RxDescDma = dma_addr;

        for (i = 0; i < gmacdev->RxDescCount; i++) {
            /* tx ring descriptor every one give base virtual address */
            gmacdev->rx_ring[i] = first_desc + i;

            ak_mac_rx_desc_init_ring(
                i, gmacdev->rx_ring[i], i == gmacdev->RxDescCount - 1);
            ak_smc_desc_set_rx_owner(gmacdev->rx_ring[i]);
        }
    }

    gmacdev->RxNext     = 0;
    gmacdev->RxBusy     = 0;
    gmacdev->RxNextDesc = gmacdev->RxDesc;
    gmacdev->RxBusyDesc = gmacdev->RxDesc;
    gmacdev->BusyRxDesc = 0;
    flush_dcache_all();

    return -ESYNOPGMACNOERR;
}

static u32 ak_mac_init_rx_desc_base(gmac_device* gmacdev)
{
    u32 rx_desc_addr;

    /* write RxDescDma consistent  physic address to mac dma ctrl reg */
    ak_gmac_write_reg(
        (u32*)gmacdev->DmaBase, DmaRxBaseAddr, (u32)gmacdev->RxDescDma);
    rx_desc_addr = ak_gmac_read_reg((u32*)gmacdev->DmaBase, DmaRxBaseAddr);

    return rx_desc_addr;
}

static void ak_mac_rmii_sharepin_cfg(struct ak_mac_priv* priv)
{

#ifdef CONFIG_37_E_CODE

    unsigned long val = 0;

    if (priv->macif_ctrl == AK_MAC0_IF) {
        /* opclk drv strength 3 level */
        val = __raw_readl(PAD_DRV_CFG0_REG);
        val &= ~((0x3 << 12));
        val |= ((0x3 << 12));
        __raw_writel(val, PAD_DRV_CFG0_REG);
    }

    if (priv->macif_ctrl == AK_MAC1_IF) {
        /*set rmii1 drv level 3 */
        val = __raw_readl(PAD_DRV_CFG3_REG);
        val &= ~((0x3 << 20) | (0x3 << 22) | (0x3 << 24) | (0x3 << 26)
            | (0x3 << 28) | (0x3UL << 30));
        val |= ((0x3 << 20) | (0x3 << 22) | (0x3 << 24) | (0x3 << 26)
            | (0x3 << 28) | (0x3UL << 30));
        __raw_writel(val, PAD_DRV_CFG3_REG);

        val = __raw_readl(PAD_DRV_CFG4_REG);
        val &= ~((0x3 << 0) | (0x3 << 2) | (0x3 << 4) | (0x3 << 6));
        val |= ((0x3 << 0) | (0x3 << 2) | (0x3 << 4) | (0x3 << 6));
        __raw_writel(val, PAD_DRV_CFG4_REG);

        /*set rmii1 disable pull up */
        val = __raw_readl(PU_PD_SEL_CFG1_REG);
        val |= ((0x1 << 26) | (0x1 << 27) | (0x1 << 28) | (0x1 << 29)
            | (0x1 << 30) | (0x1UL << 31));
        __raw_writel(val, PU_PD_SEL_CFG1_REG);

        val = __raw_readl(PU_PD_SEL_CFG2_REG);
        val |= ((0x1 << 0) | (0x1 << 1) | (0x1 << 2) | (0x1 << 3));
        __raw_writel(val, PU_PD_SEL_CFG2_REG);

        /*set rmii1 enable slew rate */
        val = __raw_readl(PAD_SL_CFG1_REG);
        val |= ((0x1 << 26) | (0x1 << 27) | (0x1 << 28) | (0x1 << 29)
            | (0x1 << 30) | (0x1UL << 31));
        __raw_writel(val, PAD_SL_CFG1_REG);

        val = __raw_readl(PAD_SL_CFG2_REG);
        val |= ((0x1 << 0) | (0x1 << 1) | (0x1 << 2) | (0x1 << 3));
        __raw_writel(val, PAD_SL_CFG2_REG);
    }
#endif
}

#if 0

static void ak_gpio_disable_pupd(
    struct ak_mac_priv *priv, T_GPIO_PD_CONFG gpio_pddis_en)
{
    u32 val = 0;

    /* set chip system ctrl reg base address, 1:disable, 0:enable */ 
    if (gpio_pddis_en){
        if (priv->macif_ctrl == AK_MAC0_IF){
            val = __raw_readl(PU_PD_ENABLE_CFG0_REG);
            val &= ~(0x1<<6);
            __raw_writel(val, PU_PD_ENABLE_CFG0_REG);
        }

        if (priv->macif_ctrl == AK_MAC1_IF){
            val = __raw_readl(PU_PD_ENABLE_CFG2_REG);
            val &= ~(0x1<<0);
            __raw_writel(val, PU_PD_ENABLE_CFG2_REG);
        }

    }else {
        if (priv->macif_ctrl == AK_MAC0_IF){
            val = __raw_readl(PU_PD_ENABLE_CFG0_REG);
            val |= (0x1<<6); 
            __raw_writel(val, PU_PD_ENABLE_CFG0_REG);
        }

        if (priv->macif_ctrl == AK_MAC1_IF){
            val = __raw_readl(PU_PD_ENABLE_CFG2_REG);
            val |= (0x1<<0);
            __raw_writel(val, PU_PD_ENABLE_CFG2_REG);
        }
    }
}
#endif

static void ak_mac_rmii_interface_50mclk(struct ak_mac_priv* priv)
{
    u32 data;

    if (priv->macif_ctrl == AK_MAC0_IF) {
#if defined(CONFIG_3918AV100_CODE) || defined(CONFIG_3918EV300L_CODE) \
 || defined(CONFIG_KM01A_CODE) || defined(CONFIG_3918AV130_CODE)
        int ret;
        struct clk clk;
        ulong rate;
        ret = clk_get_by_index(priv->dev, 0, &clk);
        if (ret) {
            printf("clk_get_by_index 0 fail:%d\r\n", ret);
            return;
        }
        rate = clk_set_rate(&clk, 50000000);
        debug("MAC_OPCLK rate:%ld\r\n", rate);

        /* first  mac_speed_cfg=1(100m) */
        data = __raw_readl(AK_PA_SYSCTRL + 0x14);
#if 0 // 100M
        data |= (0x1 << 23);
#else // 10M
        data &= ~(0x1 << 23);
#endif
        __raw_writel(data, AK_PA_SYSCTRL + 0x14);

        /* bit[8],enable generate 50m */
        data = __raw_readl(AK_PA_SYSCTRL + 0x18);
        data |= (0x1 << 8);
        __raw_writel(data, AK_PA_SYSCTRL + 0x18);

        ret = clk_get_by_index(priv->dev, 1, &clk);
        if (ret) {
            printf("clk_get_by_index 1 fail:%d\r\n", ret);
            return;
        }
        ret = clk_enable(&clk);
        if (ret) {
            printf("clk_enable fail:%d\r\n", ret);
            return;
        }
#elif defined(CONFIG_37_E_CODE)
        /* first mac interface select Rmii */
        data = __raw_readl(AK_PA_SYSCTRL + 0x14);
        data &= ~(0x1 << 22);
        __raw_writel(data, AK_PA_SYSCTRL + 0x14);

        /* first  mac_speed_cfg=1(100m) */
        data = __raw_readl(AK_PA_SYSCTRL + 0x14);
        data |= (0x1 << 23);
        __raw_writel(data, AK_PA_SYSCTRL + 0x14);

        /* mac feedback en */
        data = __raw_readl(AK_PA_SYSCTRL + 0x14);
        data |= (0x1 << 28);
        __raw_writel(data, AK_PA_SYSCTRL + 0x14);

        data = __raw_readl(AK_PA_SYSCTRL + 0x18);
        data &= ~(0x3f << 0);
        __raw_writel(data, AK_PA_SYSCTRL + 0x18);

        data = __raw_readl(AK_PA_SYSCTRL + 0x18);
        data |= (0x9 << 0);
        __raw_writel(data, AK_PA_SYSCTRL + 0x18);

        /* bit[8],enable generate 50m */
        data = __raw_readl(AK_PA_SYSCTRL + 0x18);
        data |= (0x1 << 8);
        __raw_writel(data, AK_PA_SYSCTRL + 0x18);

        /* bit[8],enable generate 50m */
        data = __raw_readl(AK_PA_SYSCTRL + 0x18);
        data |= (0x1 << 9);
        __raw_writel(data, AK_PA_SYSCTRL + 0x18);

        while (__raw_readl(AK_PA_SYSCTRL + 0x18) & (0x1 << 9))
            ;

        /* mac ctronller clk  */
        data = __raw_readl(AK_PA_SYSCTRL + 0x1c);
        data &= ~(1 << 13);
        __raw_writel(data, AK_PA_SYSCTRL + 0x1c);
#else

        /* first mac interface select Rmii */
        data = __raw_readl(AK_PA_SYSCTRL + 0x14);
        data &= ~(0x1 << 22);
        __raw_writel(data, AK_PA_SYSCTRL + 0x14);

        /* first  mac_speed_cfg=1(100m) */
        data = __raw_readl(AK_PA_SYSCTRL + 0x14);
        data |= (0x1 << 23);
        __raw_writel(data, AK_PA_SYSCTRL + 0x14);

        /* bit[21],enable generate 50m */
        data = __raw_readl(AK_PA_SYSCTRL + 0x14);
        data |= (0x1 << 21);
        __raw_writel(data, AK_PA_SYSCTRL + 0x14);

        /* set   bit[21],enable generate 50m,   bit[18], select 25m clock of mac
         * from pll div */
        data = __raw_readl(AK_PA_SYSCTRL + 0x14);
        data |= ((0x1 << 28) | (0x1 << 18));
        __raw_writel(data, AK_PA_SYSCTRL + 0x14);

        /* mac ctronller clk */
        data = __raw_readl(AK_PA_SYSCTRL + 0x1c);
        data &= ~(0x1 << 13);
        __raw_writel(data, AK_PA_SYSCTRL + 0x1c);

#endif
    }

    if (priv->macif_ctrl == AK_MAC1_IF) {
#if defined(CONFIG_3918AV100_CODE) || defined(CONFIG_3918EV300L_CODE) \
 || defined(CONFIG_KM01A_CODE) || defined(CONFIG_3918AV130_CODE)
        pr_debug("%s: No MAC1!\n", __func__);
#else
        /* first mac interface select Rmii */
        data = __raw_readl(AK_PA_SYSCTRL + 0x14);
        data &= ~(0x1 << 20);
        __raw_writel(data, AK_PA_SYSCTRL + 0x14);

        /* first  mac_speed_cfg=1(100m) */
        data = __raw_readl(AK_PA_SYSCTRL + 0x14);
        data |= (0x1 << 21);
        __raw_writel(data, AK_PA_SYSCTRL + 0x14);

        /* mac feedback en */
        data = __raw_readl(AK_PA_SYSCTRL + 0x14);
        data |= (0x1 << 28);
        __raw_writel(data, AK_PA_SYSCTRL + 0x14);

        data = __raw_readl(AK_PA_SYSCTRL + 0x18);
        data &= ~(0x3f << 20);
        __raw_writel(data, AK_PA_SYSCTRL + 0x18);

        data = __raw_readl(AK_PA_SYSCTRL + 0x18);
        data |= (0x9 << 20);
        __raw_writel(data, AK_PA_SYSCTRL + 0x18);

        /* bit[8],enable generate 50m */
        data = __raw_readl(AK_PA_SYSCTRL + 0x18);
        data |= (0x1 << 28);
        __raw_writel(data, AK_PA_SYSCTRL + 0x18);

        /* bit[8],enable generate 50m */
        data = __raw_readl(AK_PA_SYSCTRL + 0x18);
        data |= (0x1 << 29);
        __raw_writel(data, AK_PA_SYSCTRL + 0x18);

        while (__raw_readl(AK_PA_SYSCTRL + 0x18) & (0x1 << 29))
            ;

        /* mac ctronller clk */
        data = __raw_readl(AK_PA_SYSCTRL + 0x1c);
        data &= ~(1 << 16);
        __raw_writel(data, AK_PA_SYSCTRL + 0x1c);
#endif
    }
}
/*end of ak_mac_rmii_interface_50mclk*/

#if defined(CONFIG_39EV200_CODE)

static void ak_mac_disable_rmii_interface_50mclk(struct ak_mac_priv* priv)
{
    u32 data;

    data = __raw_readl(AK_PA_SYSCTRL + 0x14);
    data &= ~((0x1 << 16) | (0x1 << 21));
    __raw_writel(data, AK_PA_SYSCTRL + 0x14);
}
#endif

static int ak_mac_reset_phy(struct ak_mac_priv* priv)
{
    int ret;

    ret = gpio_request(priv->phy_reset, "phy-reset-pin");
    if (ret) {
        pr_debug("%s %d, gpio request phy-reset-pin:%d fail\n", __func__,
            __LINE__, priv->phy_reset);
        return ret;
    }

    ret |= gpio_direction_output(priv->phy_reset, 1);
    mdelay(5);
    ret |= gpio_direction_output(priv->phy_reset, 0);
    mdelay(25);
    ret |= gpio_direction_output(priv->phy_reset, 1);
    mdelay(5);

    return ret;
}
static void ak_mac_clk_sharepin_init(struct ak_mac_priv* priv)
{

    /* set rmii interface and 50m phy clock sharepin */
    ak_mac_rmii_sharepin_cfg(priv);
#if 0

    /* enable gpio49 pull up */ 
    ak_gpio_disable_pupd(priv, AK_PULLDOWN_DISABLE);
#endif
    /* open 50M phy clock */
    ak_mac_rmii_interface_50mclk(priv);

    ak_mac_reset_phy(priv);
}

static s32 ak_mac_set_mdc_clk_div(gmac_device* gmacdev, u32 clk_div_val)
{
    u32 orig_data;

    /* set MDO_CLK for MDIO transmit, note GmacGmiiAddr bit5, and 802.3
     * limit 2.5MHz */
    orig_data = ak_gmac_read_reg((u32*)gmacdev->MacBase, GmacGmiiAddr);
    orig_data &= (~GmiiCsrClkMask);
    orig_data |= clk_div_val;
    ak_gmac_write_reg((u32*)gmacdev->MacBase, GmacGmiiAddr, orig_data);

    return 0;
}

static u32 ak_mac_get_mdc_clk_div(gmac_device* gmacdev)
{
    u32 data;

    data = ak_gmac_read_reg((u32*)gmacdev->MacBase, GmacGmiiAddr);
    data &= GmiiCsrClkMask;

    return data;
}

static s32 ak_gmac_attach(
    gmac_device* gmacdev, u32 macBase, u32 dmaBase, u32 phyBase)
{

    /* make sure the device data strucure is cleared before we proceed further
     */
    memset((void*)gmacdev, 0, sizeof(gmac_device));

    /* populate the mac and dma base addresses*/
    gmacdev->MacBase = macBase;
    gmacdev->DmaBase = dmaBase;
    gmacdev->PhyBase = phyBase;

    return 0;
}

static int ak_mac_hw_init(struct ak_mac_priv* priv)
{
    /* Allocate Memory for the GMACip structure */
    priv->gmacdev_pt = (gmac_device*)malloc(sizeof(gmac_device));
    if (!priv->gmacdev_pt) {
        pr_err("error in gmac_device memory allocataion \n");
        return -ENOMEM;
    } else {
        pr_debug("allocataion gmacdev OK\n");
    }

    /* set mac ctrl address */
    ak_gmac_attach(priv->gmacdev_pt, (u32)priv->base + MACBASE,
        (u32)priv->base + DMABASE, priv->phyaddr);

    /* cfg mii share pin and open mac controller clock */
    ak_mac_clk_sharepin_init(priv);

    /* set MDO_CLK for MDIO transmit, note GmacGmiiAddr bit5, and 802.3
     * limit 2.5MHz div=42 */
#if defined(CONFIG_3918AV100_CODE) || defined(CONFIG_3918EV300L_CODE) \
 || defined(CONFIG_KM01A_CODE) || defined(CONFIG_3918AV130_CODE)
    ak_mac_set_mdc_clk_div(priv->gmacdev_pt, GmiiCsrClk4);
#else
    ak_mac_set_mdc_clk_div(priv->gmacdev_pt, GmiiCsrClk1);
#endif

    /* get MDO_CLK div */
    priv->gmacdev_pt->ClockDivMdc = ak_mac_get_mdc_clk_div(priv->gmacdev_pt);

    return 0;
}

static int ak_mac_probe(struct udevice* dev)
{
    struct ak_mac_priv* priv = dev_get_priv(dev);
    struct phy_device* phydev;
    struct mii_dev* bus;
    int ret, i;
    u16 phy_regval;
    u32 regval;

    priv->dev = dev;

    ret = ak_mac_hw_init(priv);
    if (ret)
        return ret;

    bus = mdio_alloc();
    if (!bus)
        return -ENOMEM;

    bus->read  = ak_mdio_rmii_read;
    bus->write = ak_mdio_rmii_write;
    bus->priv  = priv;
    priv->bus  = bus;

    ret = mdio_register_seq(bus, dev->seq);
    if (ret)
        return ret;

    priv->phyaddr = 0x1;
    phydev        = phy_connect(bus, priv->phyaddr, dev, priv->phyintf);
    if (!phydev) {
        pr_err("%s, line:%d\n", __func__, __LINE__);
        return -ENODEV;
    }

    phydev->supported &= PHY_DEFAULT_FEATURES;
    phydev->advertising = phydev->supported;
    priv->phydev        = phydev;

    printf("[%s][%d]Inner_phy_mode:%d,Phy_id:0x%x\n\n", __func__, __LINE__, priv->inner_phy_mode, phydev->phy_id);
    if (priv->inner_phy_mode) {
        if (phydev->phy_id == 0x118) { // 内置SZ18201
            ret = pinctrl_select_state(dev, "inner_sz18201");
            if (ret < 0) {
                pr_err("pinctrl_select_state inner_sz18201 err!\n");
                return ret;
            }

            ak_gmac_write_phy_reg(
                (u32*)priv->base, priv->phyaddr, 0x001e, 0x20e2);
            ak_gmac_read_phy_reg(
                (u32*)priv->base, priv->phyaddr, 0x001f, &phy_regval);
            phy_regval &= ~(1 << 7);
            ak_gmac_write_phy_reg(
                (u32*)priv->base, priv->phyaddr, 0x001e, 0x20e2);
            ak_gmac_write_phy_reg(
                (u32*)priv->base, priv->phyaddr, 0x001f, phy_regval);

            ak_gmac_read_phy_reg(
                (u32*)priv->base, priv->phyaddr, 0x0000, &phy_regval);
            phy_regval |= (1 << 15);
            ak_gmac_write_phy_reg(
                (u32*)priv->base, priv->phyaddr, 0x0000, phy_regval);

            for (i = 0; i < 1000; i++) {
                ak_gmac_read_phy_reg(
                    (u32*)priv->base, priv->phyaddr, 0x0000, &phy_regval);
                if (phy_regval & (1 << 15)) {
                    mdelay(1);
                    continue;
                }
                break;
            }

            if (i >= 1000) {
                pr_err("wait for PHY software reset failed!\n");
                return -ENXIO;
            } else {
                ak_gmac_write_phy_reg(
                    (u32*)priv->base, priv->phyaddr, 0x001e, 0x4000);
                ak_gmac_read_phy_reg(
                    (u32*)priv->base, priv->phyaddr, 0x001f, &phy_regval);
                phy_regval |= 3;
                ak_gmac_write_phy_reg(
                    (u32*)priv->base, priv->phyaddr, 0x001e, 0x4000);
                ak_gmac_write_phy_reg(
                    (u32*)priv->base, priv->phyaddr, 0x001f, phy_regval);

                regval = __raw_readl(AK_PA_SYSCTRL + 0x194); // funcmux_reg7
                regval &= ~(0x7 << 3); // GPIO82 -> OPCLK
                __raw_writel(regval, AK_PA_SYSCTRL + 0x194);

                ak_gmac_write_phy_reg(
                    (u32*)priv->base, priv->phyaddr, 0x001e, 0x00);
            }
        } else { // 内置JL1101 PHY
            ak_gmac_write_phy_reg(
                (u32*)priv->base, priv->phyaddr, 31, 7); // select page 7
            ak_gmac_read_phy_reg((u32*)priv->base, priv->phyaddr, 0xf0,
                &phy_regval); // read RMSR
            printf("PHY RMSR=0x%x\n", phy_regval);
            ak_gmac_write_phy_reg((u32*)priv->base, priv->phyaddr, 0xf0,
                0xdf5a); // RMII mode, RMII Clock Input
            ak_gmac_read_phy_reg((u32*)priv->base, priv->phyaddr, 0xf0,
                &phy_regval); // read RMSR
            printf("PHY RMSR=0x%x\n", phy_regval);

            regval = __raw_readl(AK_PA_SYSCTRL + 0x194); // funcmux_reg7
            regval &= ~(0x7 << 3); // GPIO82 -> OPCLK
            __raw_writel(regval, AK_PA_SYSCTRL + 0x194);

            ak_gmac_write_phy_reg(
                (u32*)priv->base, priv->phyaddr, 31, 0); // select page 0
        }
    }

    ret = phy_config(phydev);
    if ((!ret) && (env_get("ethact") == NULL)) {
        env_set("ethact", dev->name);
    }

    if (phydev->phy_id == 0x109)
    {
        /*** 切换到mii寄存器下面 ***/
        ak_gmac_write_phy_reg((u32 *)priv->base, priv->phyaddr, 0x1e, 0x100);
        ak_gmac_read_phy_reg((u32 *)priv->base, priv->phyaddr, 0x1f, &phy_regval);
        phy_regval |= 0x02;
        ak_gmac_write_phy_reg((u32 *)priv->base, priv->phyaddr, 0x1e, 0x100);
        ak_gmac_write_phy_reg((u32 *)priv->base, priv->phyaddr, 0x1f, phy_regval);

        /*** reg0x00[12]=1,打开自协商 ***/
        ak_gmac_read_phy_reg((u32 *)priv->base, priv->phyaddr, PHY_CONTROL_REG, &phy_regval);
        phy_regval |= ((0x01 << 12) | (0x01 << 9));
        ak_gmac_write_phy_reg((u32 *)priv->base, priv->phyaddr, PHY_CONTROL_REG, phy_regval);
        /*** reg0x04[8,5],打开自协商时的100/10BT的速度双工能力 ***/
        ak_gmac_read_phy_reg((u32 *)priv->base, priv->phyaddr, PHY_AN_ADV_REG, &phy_regval);
        phy_regval |= ((0x01 << 8) | (0x01 << 5));
        ak_gmac_write_phy_reg((u32 *)priv->base, priv->phyaddr, PHY_AN_ADV_REG, phy_regval);

        /*** 软复位 reg0[15] = 1  ***/
        ak_gmac_read_phy_reg((u32 *)priv->base, priv->phyaddr, PHY_CONTROL_REG, &phy_regval);
        phy_regval |= (0x01 << 15);
        ak_gmac_write_phy_reg((u32 *)priv->base, priv->phyaddr, PHY_CONTROL_REG, phy_regval);

        /***** lds LRE100/LRE10 配置 *****/

        /*** 切换到lds mii寄存器下面 ***/
        ak_gmac_write_phy_reg((u32 *)priv->base, priv->phyaddr, 0x1e, 0x100);
        ak_gmac_read_phy_reg((u32 *)priv->base, priv->phyaddr, 0x1f, &phy_regval);
        phy_regval &= 0xFFFD;
        ak_gmac_write_phy_reg((u32 *)priv->base, priv->phyaddr, 0x1e, 0x100);
        ak_gmac_write_phy_reg((u32 *)priv->base, priv->phyaddr, 0x1f, phy_regval);

        /*** lds reg0x00[12]=1,打开自协商 ***/
        ak_gmac_read_phy_reg((u32 *)priv->base, priv->phyaddr, PHY_CONTROL_REG, &phy_regval);
        phy_regval |= ((0x01 << 12) | (0x01 << 9));
        ak_gmac_write_phy_reg((u32 *)priv->base, priv->phyaddr, PHY_CONTROL_REG, phy_regval);

        /*** lds  reg0x04[5,1],打开自协商时的100/10LRE的速度双工能力 ***/
        ak_gmac_read_phy_reg((u32 *)priv->base, priv->phyaddr, PHY_AN_ADV_REG, &phy_regval);
        phy_regval |= ((0x01 << 1) | (0x01 << 5));
        phy_regval &= (~(0x01 << 5));
        printf("Mandatory LAR - 10M !\n");
        ak_gmac_write_phy_reg((u32 *)priv->base, priv->phyaddr, PHY_AN_ADV_REG, phy_regval);

        /*** 切换到mii寄存器下面 ***/
        ak_gmac_write_phy_reg((u32 *)priv->base, priv->phyaddr, 0x1e, 0x100);
        ak_gmac_read_phy_reg((u32 *)priv->base, priv->phyaddr, 0x1f, &phy_regval);
        phy_regval |= 0x02;
        ak_gmac_write_phy_reg((u32 *)priv->base, priv->phyaddr, 0x1e, 0x100);
        ak_gmac_write_phy_reg((u32 *)priv->base, priv->phyaddr, 0x1f, phy_regval);

        /* 解决10 M idle issue Bit[7]清零*/
        ak_gmac_write_phy_reg((u32 *)priv->base, priv->phyaddr, 0x1e, 0x2023);
        ak_gmac_read_phy_reg((u32 *)priv->base, priv->phyaddr, 0x1f, &phy_regval);
        phy_regval &= 0x7F;
        ak_gmac_write_phy_reg((u32 *)priv->base, priv->phyaddr, 0x1e, 0x2023);
        ak_gmac_write_phy_reg((u32 *)priv->base, priv->phyaddr, 0x1f, phy_regval);

        /*** 软复位 lds reg0[15] = 1  ***/
        ak_gmac_read_phy_reg((u32 *)priv->base, priv->phyaddr, PHY_CONTROL_REG, &phy_regval);
        phy_regval |= (0x01 << 15);
        ak_gmac_write_phy_reg((u32 *)priv->base, priv->phyaddr, PHY_CONTROL_REG, phy_regval);
    }
    return ret;
}
/*end of ak_mac_probe*/

static int ak_mac_remove(struct udevice* dev)
{
    struct ak_mac_priv* priv = dev_get_priv(dev);

#if defined(CONFIG_39EV200_CODE)
    ak_mac_disable_rmii_interface_50mclk(priv);
#endif

    mdio_unregister(priv->bus);
    mdio_free(priv->bus);

    return 0;
}

static const struct eth_ops ak_mac_ops = {
    .start        = ak_mac_start,
    .send         = ak_mac_send,
    .recv         = ak_mac_recv,
    .stop         = ak_mac_stop,
    .write_hwaddr = ak_mac_write_hwaddr,
};

static const struct udevice_id ak_mac_ids[]
    = { { .compatible = "anyka,ak37e-ethernet" },
          { .compatible = "anyka,ak37e-ethernet1" },
          { .compatible = "anyka,ak37d-ethernet" },
          { .compatible = "anyka,ak39ev330-ethernet" },
          { .compatible = "anyka,ak3918av100-ethernet" },
          { .compatible = "anyka,ak3918av130-ethernet" },
          { .compatible = "anyka,ak3918ev300l-ethernet" },
          { .compatible = "anyka,ak39ev200-ethernet" }, {} };

U_BOOT_DRIVER(ak_mac) = {
    .name                     = "ak_mac",
    .id                       = UCLASS_ETH,
    .of_match                 = ak_mac_ids,
    .ofdata_to_platdata       = ak_mac_ofdata_to_platdata,
    .probe                    = ak_mac_probe,
    .remove                   = ak_mac_remove,
    .ops                      = &ak_mac_ops,
    .priv_auto_alloc_size     = sizeof(struct ak_mac_priv),
    .platdata_auto_alloc_size = sizeof(struct eth_pdata),
#if defined(CONFIG_39EV200_CODE)
    .flags = DM_FLAG_OS_PREPARE,
#endif
};
#endif
