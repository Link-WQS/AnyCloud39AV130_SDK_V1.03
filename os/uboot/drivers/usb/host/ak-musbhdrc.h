#ifndef __AK37E_HCD_H__
#define __AK37E_HCD_H__

#include <asm/io.h>
#include <linux/compat.h>
#include <linux/bug.h>

/** system control register */
#define SYSTEM_CTRL_BASE_ADDR	(0x08000000)
#define USB_OP_MOD_REG			(SYSTEM_CTRL_BASE_ADDR + 0x58)
#define USB_MODULE_RESET_REG	(SYSTEM_CTRL_BASE_ADDR + 0x20)

/** usb hc base address */
#define USB_HC_BASE_ADDR		(0x20200000)

/** Control Registers*/
#define USB_REG_FADDR    (0x0000) // 8-bit
#define USB_REG_POWER    (0x0001) // 8-bit
#define USB_REG_INTRTX   (0x0002) // 16-bit, read clear
#define USB_REG_INTRRX   (0x0004) // 16-bit, read clear
#define USB_REG_INTRTXE  (0x0006) // 16-bit
#define USB_REG_INTRRXE  (0x0008) // 16-bit
#define USB_REG_INTRUSB  (0x000A) // 8-bit, read clear
#define USB_REG_INTRUSBE (0x000B) // 8-bit
#define USB_REG_FRAME    (0x000C) // 16-bit
#define USB_REG_INDEX    (0x000E) // 8-bit
#define USB_REG_TESEMODE (0x000F) // 8-bit
#define USB_REG_DEVCTL   (0x0060) // 8-bit

/** Endpoint Control/Status Registers */
#define USB_REG_TXMAXP     (0x0010) // 16-bit, when index == 1~4
#define USB_REG_TXCSR1     (0x0012) // 8-bit, when index == 1~4
#define USB_REG_TXCSR2     (0x0013) // 8-bit, when index == 1~4
#define USB_REG_RXMAXP     (0x0014) // 16-bit, when index == 1~4
#define USB_REG_RXCSR1     (0x0016) // 8-bit, when index == 1~4
#define USB_REG_RXCSR2     (0x0017) // 8-bit, when index == 1~4
#define USB_REG_COUNT0     (0x0018) // 16-bit, when index == 0
#define USB_REG_RXCOUNT    (0x0018) // 16-bit, when index == 1~4
#define USB_REG_TXTYPE     (0x001A) // 8-bit, when index == 1~4,    host only
#define USB_REG_NAKLIMIT0  (0x001B) // 8-bit, when index == 0,      host only
#define USB_REG_TXINTERVAL (0x001B) // 8-bit, when index == 1~4,    host only
#define USB_REG_RXTYPE     (0x001C) // 8-bit, when index == 1~4,    host only
#define USB_REG_RXINTERVAL (0x001D) // 8-bit, when index == 1~4,    host only
#define USB_REG_CONFIGDATA (0x001F) // 8-bit, when index == 0
#define USB_REG_FIFOSIZE   (0x001F) // 8-bit, when index == 1~4
#define USB_REG_FIFOSIZE_CFG  (0x0062)
#define USB_REG_FIFOADDR_CFG  (0x0064)

/**  USB DMA */
#define USB_DMA_INTR       (0x0200)
#define USB_DMA_CTRL_BASE  (0x0204)
#define USB_DMA_ADDR_BASE  (0x0208)
#define USB_DMA_COUNT_BASE (0x020c)
#define USB_DMA_CTRL(n)    (USB_DMA_CTRL_BASE + (n)*0x10)
#define USB_DMA_ADDR(n)    (USB_DMA_ADDR_BASE + (n)*0x10)
#define USB_DMA_COUNT(n)   (USB_DMA_COUNT_BASE + (n)*0x10)

#define USB_REG_REQPKTCNT_BASE (0x0304)
#define USB_REG_REQPKTCNT(ep)  (USB_REG_REQPKTCNT_BASE + (ep - 1) * 0x4)

/** FIFOs Entry */
#define USB_FIFO_EP0 (0x0020) // 8- / 16- / 32-bit
#define USB_FIFO_EP1 (0x0024) // 8- / 16- / 32-bit
#define USB_FIFO_EP2 (0x0028) // 8- / 16- / 32-bit
#define USB_FIFO_EP3 (0x002C) // 8- / 16- / 32-bit
#define USB_FIFO_EP4 (0x0030) // 8- / 16- / 32-bit
#define USB_FIFO_EP5 (0x0034) // 8- / 16- / 32-bit
#define USB_FIFO_EP6 (0x0038) // 8- / 16- / 32-bit

/*DMA controler registers.*/
#define DMA_INTR_STAT 0x0 /*DMA interrupt status.*/

#define DMA_CTRL_REG1 0x04 /*DMA channel 1 control.*/
#define DMA_CTRL_REG2 0x14 /*DMA channel 2 control.*/
#define DMA_CTRL_REG3 0x24 /*DMA channel 3 control.*/
#define DMA_CTRL_REG4 0x34 /*DMA channel 4 control.*/

#define DMA_ADDR_REG1 0x08 /*DMA channel 1 AHB memory address.*/
#define DMA_ADDR_REG2 0x18 /*DMA channel 2 AHB memory address.*/
#define DMA_ADDR_REG3 0x28 /*DMA channel 3 AHB memory address.*/
#define DMA_ADDR_REG4 0x38 /*DMA channel 4 AHB memory address.*/

#define DMA_CUNT_REG1 0x0c /*DMA channel 1 byte count.*/
#define DMA_CUNT_REG2 0x1c /*DMA channel 2 byte count.*/
#define DMA_CUNT_REG3 0x2c /*DMA channel 3 byte count.*/
#define DMA_CUNT_REG4 0x3c /*DMA channel 4 byte count.*/

//********************** bit fields defs***********************

#define USB_ENABLE_DMA      (1)
#define USB_DIRECTION_RX    (0 << 1)
#define USB_DIRECTION_TX    (1 << 1)
#define USB_DMA_MODE1       (1 << 2)
#define USB_DMA_MODE0       (0 << 2)
#define USB_DMA_INT_ENABLE  (1 << 3)
#define USB_DMA_INT_DISABLE (0 << 3)
#define USB_DMA_BUS_ERROR   (1 << 8)
#define USB_DMA_BUS_MODE0   (0 << 9)
#define USB_DMA_BUS_MODE1   (1 << 9)
#define USB_DMA_BUS_MODE2   (2 << 9)
#define USB_DMA_BUS_MODE3   (3 << 9)

// RTC_USB_CONFIG_REG
#define SESSEND       (1 << 31) // 0:above/1:below Session End threshold
#define AVALID        (1 << 30)
#define VBUSVALID     (1 << 29)
#define SESSEND_SEL   (1 << 28)
#define AVALID_SEL    (1 << 27)
#define VBUSVALID_SEL (1 << 26)

/** POWER Control register  */
#define USB_POWER_ENSUSPEND (1 << 0)
#define USB_POWER_SUSPENDM  (1 << 1) // host/client
#define USB_POWER_RESUME    (1 << 2) // host/client
#define USB_POWER_RESET     (1 << 3)
#define USB_POWER_HSMODE    (1 << 4)
#define USB_HOSG_HIGH_SPEED (1 << 5)

/** CSR01 register */
// mode-agnostic
#define USB_CSR01_RXPKTRDY (1 << 0) // r / clear
#define USB_CSR01_TXPKTRDY (1 << 1) // r / set
// Client mode
#define USB_CSR01_P_SENTSTALL   (1 << 2)
#define USB_CSR01_P_DATAEND     (1 << 3)
#define USB_CSR01_P_SETUPEND    (1 << 4)
#define USB_CSR01_P_SENDSTALL   (1 << 5)
#define USB_CSR01_P_SVDRXPKTRDY (1 << 6)
#define USB_CSR01_P_SVDSETUPEND (1 << 7)
// Host mode
#define USB_CSR01_H_RXSTALL    (1 << 2) // r / clear
#define USB_CSR01_H_SETUPPKT   (1 << 3) // r / w
#define USB_CSR01_H_ERROR      (1 << 4) // r / clear
#define USB_CSR01_H_REQPKT     (1 << 5) // r / w
#define USB_CSR01_H_STATUSPKT  (1 << 6) // r / w
#define USB_CSR01_H_NAKTIMEOUT (1 << 7) // r / clear

/** CSR02 register */
// mode-agnostic
#define USB_CSR02_FLUSHFIFO (1 << 0) // set
// Client mode (none)
// Host mode
#define USB_CSR02_H_DISPING (1 << 3) // r / w

/** TXCSR1 register */
// mode-agnostic
#define USB_TXCSR1_TXPKTRDY     (1 << 0)
#define USB_TXCSR1_FIFONOTEMPTY (1 << 1)
#define USB_TXCSR1_FLUSHFIFO    (1 << 3)
#define USB_TXCSR1_CLRDATATOG   (1 << 6)
// Client mode
#define USB_TXCSR1_P_UNDERRUN  (1 << 2)
#define USB_TXCSR1_P_SENDSTALL (1 << 4)
#define USB_TXCSR1_P_SENTSTALL (1 << 5)
#define USB_TXCSR1_P_INCOMPTX  (1 << 7)
// Host MODE
#define USB_TXCSR1_H_ERROR      (1 << 2)
#define USB_TXCSR1_H_RXSTALL    (1 << 5)
#define USB_TXCSR1_H_NAKTIMEOUT (1 << 7) // for Bulk Endpoint
#define USB_TXCSR1_H_INCOMPTX   (1 << 7) // for Interrupt Endpoint

/** TXCSR2 register */
// mode-agnostic
#define USB_TXCSR2_DMAMODE1   (1 << 2)
#define USB_TXCSR2_FRCDATATOG (1 << 3)
#define USB_TXCSR2_DMAENAB    (1 << 4)
#define USB_TXCSR2_MODE       (1 << 5)
// Host mode
#define USB_TXCSR2_MODE_TX (1 << 5)
#define USB_TXCSR2_AUTOSET (1 << 7)
// Client mode
#define USB_TXCSR2_P_ISO (1 << 6)

/** RXCSR1 register */
// mode-agnostic
#define USB_RXCSR1_RXPKTRDY   (1 << 0) // r / clear
#define USB_RXCSR1_FIFOFULL   (1 << 1) // r
#define USB_RXCSR1_DATAERR    (1 << 3) // r / clear(host), for ISO only
#define USB_RXCSR1_FLUSHFIFO  (1 << 4) // set
#define USB_RXCSR1_CLRDATATOG (1 << 7) // set
// Client mode
#define USB_RXCSR1_P_OVERRUN   (1 << 2) // r / clear, for ISO only
#define USB_RXCSR1_P_SENDSTALL (1 << 5) // r / w
#define USB_RXCSR1_P_SENTSTALL (1 << 6) // r / clear
// Host MODE
#define USB_RXCSR1_H_ERROR      (1 << 2) // r / clear
#define USB_RXCSR1_H_NAKTIMEOUT (1 << 3) // r / clear, for BULK
#define USB_RXCSR1_H_REQPKT     (1 << 5) // r / w
#define USB_RXCSR1_H_RXSTALL    (1 << 6) // r / clear

/** RXCSR2 register */
// mode-agnostic
#define USB_RXCSR2_INCOMPRX   (1 << 0) // r
#define USB_RXCSR2_DMAMODE0   (0 << 3)
#define USB_RXCSR2_DMAMODE1   (1 << 3) // r / w
#define USB_RXCSR2_DMAREQENAB (1 << 5) // r / w
#define USB_RXCSR2_AUTOCLEAR  (1 << 7) // r / w

#define USB_RXCSR2_AUTOREQ (1 << 6)
#define USB_RXCSR2_DMAENAB (1 << 5)
#define USB_RXCSR2_DISNYET (1 << 4)
#define USB_RXCSR2_DMAMODE USB_RXCSR2_DMAMODE1

// Client mode
#define USB_RXCSR2_P_DISNYET  (1 << 4) // for Bulk/Interrupt transaction
#define USB_RXCSR2_P_PIDERROR (1 << 4) // for ISO transaction
#define USB_RXCSR2_P_ISO      (1 << 6)
// Host Mode
#define USB_RXCSR2_H_PIDERROR (1 << 4) // r / w, for ISO only
#define USB_RXCSR2_H_AUTOREQ  (1 << 6) // r / w

/** IntrUSB/IntrUSBE register */
#define USB_INTR_SUSPEND    (1 << 0) // client
#define USB_INTR_RESUME     (1 << 1)
#define USB_INTR_RESET      (1 << 2) // client
#define USB_INTR_BABBLE     (1 << 2) // host
#define USB_INTR_SOF        (1 << 3)
#define USB_INTR_CONNECT    (1 << 4) // host
#define USB_INTR_DISCONNECT (1 << 5) // host/client
#define USB_INTR_SESSREQ    (1 << 6) // A device
#define USB_INTR_VBUSERROR  (1 << 7) // A device

/** IntrTX register */
/** IntrRX register */
/** IntrTXE register */
/** IntrRXE register */
#define USB_INTR_EP0 (1 << 0)
#define USB_INTR_EP1 (1 << 1)
#define USB_INTR_EP2 (1 << 2)
#define USB_INTR_EP3 (1 << 3)
#define USB_INTR_EP4 (1 << 4)
#define USB_INTR_EP5 (1 << 5)
#define USB_INTR_EPX_MASK                                                      \
    (USB_INTR_EP1 | USB_INTR_EP2 | USB_INTR_EP3 | USB_INTR_EP4 | USB_INTR_EP5)

/** IntrUSB/IntrUSBE register */
#define USB_INTR_SUSPEND    (1 << 0) // client
#define USB_INTR_RESUME     (1 << 1)
#define USB_INTR_RESET      (1 << 2) // client
#define USB_INTR_BABBLE     (1 << 2) // host
#define USB_INTR_SOF        (1 << 3)
#define USB_INTR_CONNECT    (1 << 4) // host
#define USB_INTR_DISCONNECT (1 << 5) // host/client
#define USB_INTR_SESSREQ    (1 << 6) // A device
#define USB_INTR_VBUSERROR  (1 << 7) // A device

/** RXMAXP register */
#define USB_RXMAXP_MASK (0x07FF)
#define USB_RXMAXP(cnt) ((cnt)&0x07FF)

/** TXTYPE register */
#define USB_TXTYPE_EPNUM_MASK       (0xf << 0)
#define USB_TXTYPE_EPNUM(ep)        (((ep)&0xf) << 0)
#define USB_TXTYPE_PROTOCOL_ILLEGAL (0 << 4)
#define USB_TXTYPE_PROTOCOL_ISO     (1 << 4)
#define USB_TXTYPE_PROTOCOL_BULK    (2 << 4)
#define USB_TXTYPE_PROTOCOL_INTR    (3 << 4)

/** DevCtl register */
#define USB_DEVCTL_SESSION       (1 << 0) // host/client
#define USB_DEVCTL_HOSTREQ       (1 << 1) // B device
#define USB_DEVCTL_HOSTMODE_MASK (1 << 2)

#define USB_DEVCTL_HOSTMODE_HOST (1 << 2)
#define USB_DEVCTL_VBUS_MASK     (3 << 3)

#define USB_DEVCTL_LSDEV        (1 << 5)
#define USB_DEVCTL_FSDEV        (1 << 6) // host
#define USB_DEVCTL_BDEVICE_MASK (1 << 7)

#define USB_DEVCTL_BDEVICE_B (1 << 7)

#define usb_unused __attribute__((unused))

#define usb_loop_delay(loop) \
    do{\
    	unsigned int i;\
    	unsigned int delay = 1000 * (loop);\
    	for(i = 0; i < delay; i++)\
    		asm volatile("nop");\
    }while(0)

#if 1
#if 1
#define reg_debug(fmt, arg...)  \
	usb_loop_delay(1)
#else
#define reg_debug(fmt, arg...)
#endif
#else
#define reg_debug printf
#endif
/* register access ops */
static inline unsigned char hc_readb(unsigned int reg)
{
	usb_unused unsigned char reg8;
	reg8 = __raw_readb(USB_HC_BASE_ADDR + (reg));
	reg_debug("%s reg %08x val %02x\n", __func__, USB_HC_BASE_ADDR + (reg), reg8);
	return reg8;
}

static inline void hc_writeb(unsigned char val, unsigned int reg)
{
	__raw_writeb(val, USB_HC_BASE_ADDR + (reg));
	reg_debug("%s reg %08x val %02x\n", __func__, USB_HC_BASE_ADDR + (reg), val);
}

static inline unsigned short hc_readw(unsigned int reg)
{
	usb_unused unsigned short reg16;
	reg16 = __raw_readw(USB_HC_BASE_ADDR + (reg));
	reg_debug("%s reg %08x val %04x\n", __func__, USB_HC_BASE_ADDR + (reg), reg16);
	return reg16;
}

static inline void hc_writew(unsigned short val, unsigned int reg)
{
	__raw_writew(val, USB_HC_BASE_ADDR + (reg));
	reg_debug("%s reg %08x val %04x\n", __func__, USB_HC_BASE_ADDR + (reg), val);
}

static inline unsigned long hc_readl(unsigned int reg)
{
	usb_unused unsigned short reg32;
	reg32 = __raw_readl(USB_HC_BASE_ADDR + (reg));
	reg_debug("%s reg %08x val %08x\n", __func__, USB_HC_BASE_ADDR + (reg), reg32);
	return reg32;
}
static inline void hc_writel(unsigned long val, unsigned int reg)
{
	__raw_writel(val, USB_HC_BASE_ADDR + (reg));
	reg_debug("%s reg %08x val %08x\n", __func__, USB_HC_BASE_ADDR + (reg), val);
}

#define hc_reg8(reg) (*(volatile unsigned char *)(USB_HC_BASE_ADDR + (reg)))
#define hc_reg16(reg) (*(volatile unsigned short *)(USB_HC_BASE_ADDR + (reg)))
#define hc_reg32(reg) (*(volatile unsigned long *)(USB_HC_BASE_ADDR + (reg)))

/*
 * IN 37e, we have 7 epfifo. epfifo 0 using for Control transfer.
 * epfifo1-epfifi6 usnig for Interrupt/Bulk/Isochronous tranfer.
 * so, MAX_EP_NUM is 6 (EP6)
 */
#define MAX_EP_NUM         (6)
#define USBDMA_CHANNEL_NUM (6)

#define H_MAXPACKET			512	   /* bytes in fifo */

enum { USB_SLAVE, USB_HOST, USB_ROLE_UNKNOWN };

struct usbhc_ep{
    u8 in_used;
    struct usb_device *udev;

    u16     maxpacket;
	u8 		epnum;
	u8	    epfifo;

    u16		error_count;
    u16		nak_count;
    u16	    length;	 /* of current packet */
};

struct _usbhc{
    /* ofdata to platdata */
    struct udevice *udev;
	struct clk clk;

	unsigned long hcdc_baseaddr;
	unsigned long sysctrl_baseaddr;

    /* probe */
    spinlock_t lock;

    struct usbhc_ep active_ep0;
    struct usbhc_ep	active_epx[MAX_EP_NUM];

    u16 port_status;
    u16 port_change;
	u16 frame;

    u32 configuration;
    u32 rootdev;
    u32 dev_address;
};

struct epfifo_mapping {
	int	epfifo;		
	int	used;		/* 0 - Unused, 1 - used */
	int	epnum;		/* USB Device endpoint number: 1 ~ 16 */
	int	direction;	/* 0 - In, 1 - Out */
	int fifostart;
	int fifosize;
};

struct akotghc_epfifo_mapping {
	struct epfifo_mapping mapping[MAX_EP_NUM];
};

static inline bool __is_epnum_mapped(
	struct akotghc_epfifo_mapping *akotg_mapping, int epnum, int direction)
{
	int i;
	struct epfifo_mapping *mapping;

	if(epnum == 0)
		return true;

	for (i = 0; i < MAX_EP_NUM; i++) {
		mapping = &akotg_mapping->mapping[i];
		if (mapping->used 
			&& (mapping->epnum == epnum) 
			&& (mapping->direction == direction)) {
			return true;
		}
	}

	return false;
}

static inline bool is_epnum_mapped(struct akotghc_epfifo_mapping *akotg_mapping,
	int epnum, int direction)
{
	bool ret;

	BUG_ON(akotg_mapping == NULL);

	if(epnum == 0)
		return true;

	ret = __is_epnum_mapped(akotg_mapping, epnum, direction);

	return ret;

}

// static inline bool map_epnum_to_epfifo(struct usb_device *dev, struct akotghc_epfifo_mapping *akotg_mapping,
// 	int epnum, int direction, int *epfifo)
// {
// 	bool ret;

// 	BUG_ON(akotg_mapping == NULL);

// 	if (is_epnum_mapped(akotg_mapping, epnum, direction))
// 		return false;
	
// 	ret = __map_epnum_to_epfifo(dev, akotg_mapping, epnum, direction, epfifo);

// 	return ret;
// }

static inline bool epfifo_to_epnum(struct akotghc_epfifo_mapping *akotg_mapping, int epfifo, int *epnum, int *direction)
{
	int ret;
	struct epfifo_mapping *mapping;

	ret = false;

	mapping = &akotg_mapping->mapping[epfifo];
	if (mapping->used) {
		*epnum = mapping->epnum;
		*direction = mapping->direction;
		ret = true;
	} else {
		*epnum = 0;
		*direction = 0;
		ret = false;
	}

	return ret;
}

static inline bool epnum_to_epfifo(struct akotghc_epfifo_mapping *akotg_mapping, int epnum, int direction, int *epfifo)
{
	int i;
	struct epfifo_mapping *mapping;

	if (epnum == 0) {
		*epfifo = 0;
		return true;
	}
	
	for (i = 0; i < MAX_EP_NUM; i++) {
		mapping = &akotg_mapping->mapping[i];
		if (mapping->used && (mapping->epnum == epnum) && (mapping->direction == direction)) {
			*epfifo = mapping->epfifo;
			return true;
		}
	}

	return false;
}

static inline unsigned char hc_index_readb(int epindex, int reg)
{
	unsigned long flags = 0;
	unsigned char val;

	local_irq_save(flags);

	hc_writeb(epindex, USB_REG_INDEX);
	val = hc_readb(reg);

	local_irq_restore(flags);

	return val;
}

static inline void hc_index_writeb(int epindex, unsigned char val, int reg)
{
	unsigned long flags = 0;
	
	local_irq_save(flags);

	hc_writeb(epindex, USB_REG_INDEX);
	hc_writeb(val, reg);

	local_irq_restore(flags);
}

static inline unsigned short hc_index_readw(int epindex, int reg)
{
	unsigned long flags = 0;
	unsigned short val;

	local_irq_save(flags);

	hc_writeb(epindex, USB_REG_INDEX);
	val = hc_readw(reg);

	local_irq_restore(flags);

	return val;
}

static inline void hc_index_writew(int epindex, unsigned short val, int reg)
{
	unsigned long flags = 0;

	local_irq_save(flags);

	hc_writeb(epindex, USB_REG_INDEX);
	hc_writew(val, reg);

	local_irq_restore(flags);
}

static inline unsigned long hc_index_readl(int epindex, int reg)
{
	unsigned long flags = 0;
	unsigned long val;

	local_irq_save(flags);

	hc_writeb(epindex, USB_REG_INDEX);
	val = hc_readl(reg);

	local_irq_restore(flags);

	return val;
}

static inline void hc_index_writel(int epindex, unsigned long val, int reg)
{
	unsigned long flags = 0;

	local_irq_save(flags);

	hc_writeb(epindex, USB_REG_INDEX);
	hc_writel(val, reg);

	local_irq_restore(flags);
}

static inline void enable_ep0_interrupt(void)
{
	u8 regval;
	unsigned long flags = 0;

	local_irq_save(flags);

	regval = hc_readb(USB_REG_INTRTXE);
	regval |= USB_INTR_EP0;
	hc_writeb(regval, USB_REG_INTRTXE);

	local_irq_restore(flags);
}

static inline void enable_epx_tx_interrupt(int i)
{
	u8 regval;
	unsigned long flags = 0;

	local_irq_save(flags);

	regval = hc_readb(USB_REG_INTRTXE);
	regval |= (1 << i);
	hc_writeb(regval, USB_REG_INTRTXE);

	local_irq_restore(flags);
}

static inline void enable_epx_rx_interrupt(int i)
{
	u8 regval;
	unsigned long flags = 0;

	local_irq_save(flags);

	regval = hc_readb(USB_REG_INTRRXE);
	regval |= (1 << i);
	hc_writeb(regval, USB_REG_INTRRXE);

	local_irq_restore(flags);
}

static inline void disable_ep0_interrupt(void)
{
	u8 regval;
	unsigned long flags = 0;

	local_irq_save(flags);

	regval = hc_readb(USB_REG_INTRTXE);
	regval &= ~USB_INTR_EP0;
	hc_writeb(regval, USB_REG_INTRTXE);

	local_irq_restore(flags);
}

static inline void disable_epx_tx_interrupt(int i)
{
	u8 regval;
	unsigned long flags = 0;

	local_irq_save(flags);

	regval = hc_readb(USB_REG_INTRTXE);
	regval &= ~(1 << i);
	hc_writeb(regval, USB_REG_INTRTXE);

	local_irq_restore(flags);
}

static inline void disable_epx_rx_interrupt(int i)
{
	u8 regval;
	unsigned long flags = 0;

	local_irq_save(flags);

	regval = hc_readb(USB_REG_INTRRXE);
	regval &= ~(1 << i);
	hc_writeb(regval, USB_REG_INTRRXE);

	local_irq_restore(flags);
}

static inline void disable_epx_interrupt(int i)
{
	disable_epx_tx_interrupt(i);
	disable_epx_rx_interrupt(i);
}

static inline void disable_ep_interrupt(int i)
{
	BUG_ON(i < 0 || i > MAX_EP_NUM);

	if (i == 0) {
		disable_ep0_interrupt();
	} else {
		disable_epx_interrupt(i);
	}
}
static inline void clear_all_interrupts(void)
{
	hc_writeb(0x0, USB_REG_INTRUSBE);
	hc_writew(0x0, USB_REG_INTRTXE);
	hc_writew(0x0, USB_REG_INTRRXE);
	
	hc_readb(USB_REG_INTRUSB);
	hc_readw(USB_REG_INTRTX);
	hc_readw(USB_REG_INTRRX);
}

static inline void flush_ep0_fifo(void)
{
	unsigned long flags = 0;

	local_irq_save(flags);

	hc_writeb(0, USB_REG_INDEX);
	if (hc_readb(USB_REG_TXCSR1) & (USB_CSR01_RXPKTRDY | USB_CSR01_TXPKTRDY))
		hc_writeb(USB_CSR02_FLUSHFIFO, USB_REG_TXCSR2);

	local_irq_restore(flags);
}

static inline void flush_epx_tx_fifo(int i)
{
	unsigned long flags = 0;

	local_irq_save(flags);

	hc_writeb(i, USB_REG_INDEX);
	if (hc_readb(USB_REG_TXCSR1) & (USB_CSR01_RXPKTRDY)){
		hc_writeb(USB_TXCSR1_FLUSHFIFO, USB_REG_TXCSR1);
		/* 
		 * With Index Register Value set to 1~5 in HOST mode:(Address: 0x2020,0012) 
		 * Bit[3]
		 * 
		 * flushfifo
		 * 	0: No instruction
		 *	1: To flush the latest packet from the endpoint TX FIFO. 
		 *	The FIFO pointer is reset, the TxPktRdy Bit (Bit[0]) is cleared, and an interrupt is generated.
		 *  NOTE:
		 *	1) FlushFIFO should only be used when TxPktRdy (Bit[0]) is set.
		 *	2) If the FIFO is double-buffered, FlushFIFO may need to be set twice to completely clear the FIFO.
		 *
		 *  Notice that, double-buffered flushFIFO may need to be set twice to completely clear the FIFO
		 *
		 *  We set flushFIFO twice in case of double-buffered and don't determine it is double-buffered or not.
		 */
		hc_writeb(USB_TXCSR1_FLUSHFIFO, USB_REG_TXCSR1);
	}

	local_irq_restore(flags);
}

static inline void flush_epx_rx_fifo(int i)
{
	unsigned long flags = 0;

	local_irq_save(flags);

	hc_writeb(i, USB_REG_INDEX);
	if (hc_readb(USB_REG_RXCSR1) & (USB_RXCSR1_RXPKTRDY)){
		hc_writeb(USB_RXCSR1_FLUSHFIFO, USB_REG_RXCSR1);
		/* 
		 * In HOST mode:(Address: 0x2020,0016)
		 * Bit[4]
		 * 
		 *	flushfifo
		 *	0: No instruction.
		 *	1: To flush the next packet to be read from the endpoint RX FIFO. The FIFO pointer is reset and the RxPktRdy Bit is cleared.
		 *	NOTE:
		 *	1) FlushFIFO should only be used when RxPktRdy bit (bit[0]) is set.
		 *	2) If the FIFO is double-buffered, FlushFIFO may need to be set twice to completely clear the FIFO.
		 *
		 *
		 *  Notice that, double-buffered flushFIFO may need to be set twice to completely clear the FIFO
		 *
		 *  We set flushFIFO twice in case of double-buffered and don't determine it is double-buffered or not.
		 */
		hc_writeb(USB_RXCSR1_FLUSHFIFO, USB_REG_RXCSR1);
	}

	local_irq_restore(flags);
}

static inline void flush_epx_fifo(int i)
{
	flush_epx_tx_fifo(i);
	flush_epx_rx_fifo(i);
}

static inline void flush_ep_fifo(int i)
{
	if (i == 0)
		flush_ep0_fifo();
	else
		flush_epx_fifo(i);
}

static inline void set_epx_rx_mode(int i)
{
	u8 regval;
	unsigned long flags = 0;

	local_irq_save(flags);

	hc_writeb(i, USB_REG_INDEX);
	regval = hc_readb(USB_REG_TXCSR2);
	regval &= ~USB_TXCSR2_MODE;
	hc_writeb(regval, USB_REG_TXCSR2);

	local_irq_restore(flags);
}

static inline void set_epx_tx_mode(int i)
{
	u8 regval;
	unsigned long flags = 0;

	local_irq_save(flags);

	hc_writeb(i, USB_REG_INDEX);
	regval = hc_readb(USB_REG_TXCSR2);
	regval |= USB_TXCSR2_MODE;
	hc_writeb(regval, USB_REG_TXCSR2);

	local_irq_restore(flags);
}

static inline void clear_epx_tx_data_toggle(int i)
{
	hc_index_writeb(i, USB_TXCSR1_CLRDATATOG, USB_REG_TXCSR1);
	//hc_index_writeb(i, USB_RXCSR2_DMAREQENAB, USB_REG_TXCSR2);  //???
}

static inline void clear_epx_rx_data_toggle(int i)
{
	hc_index_writeb(i, USB_RXCSR1_CLRDATATOG, USB_REG_RXCSR1);
}

/*
 * Valid types:
 *   1 - Isochronous
 *   2 - Bulk
 *   3 - Interrupt
 * Invalid type:
 *   0 - Illegal
 */
static inline void set_epx_tx_type(int i, int epnum, int type)
{
	BUG_ON(i < 0 || i > MAX_EP_NUM);
	BUG_ON(type < 0 || type > 3);

	hc_index_writeb(i, type << 4 | epnum, USB_REG_TXTYPE);
}

static inline void set_epx_rx_type(int i, int epnum, int type)
{
	BUG_ON(i < 0 || i > MAX_EP_NUM);
	BUG_ON(type < 0 || type > 3);

	hc_index_writeb(i, type << 4 | epnum, USB_REG_RXTYPE);
}

static inline void reset_endpoint(int i)
{
	BUG_ON(i < 0 || i > MAX_EP_NUM);

	disable_ep_interrupt(i);
	if (i == 0) {
		flush_ep0_fifo();
	} else {
		flush_epx_fifo(i);
		set_epx_rx_type(i, 0, 0);
		set_epx_tx_type(i, 0, 0);
	}
}

static inline void reset_endpoints(void)
{
	int i;

	for (i = 0; i < MAX_EP_NUM + 1; i++) {
		reset_endpoint(i);
	}
}

static inline void set_usb_as_host(void)
{
    unsigned long con;

    con = __raw_readl(USB_OP_MOD_REG);
    con &= ~(0xff << 6);
    con |= (0x1 << 12) | (0x1f << 6);
    __raw_writel(con, USB_OP_MOD_REG);
}

static inline int get_usb_role(void)
{
    unsigned long con;
    con = __raw_readl(USB_OP_MOD_REG);
    con >>= 12;
    con &= 0x3;

    if (0x01 == con)
        return USB_HOST;
    else if (0x11 == con)
        return USB_SLAVE;
    else
        return USB_ROLE_UNKNOWN;
}

static inline void usb_reset_phy(struct _usbhc* otghc)
{
    unsigned long con;

    con = __raw_readl(USB_OP_MOD_REG);
    con &= ~(0x7 << 0);
    con |= (1 << 0);
    __raw_writel(con, USB_OP_MOD_REG);
}

/* power up and set usb suspend */
static inline void usb_power_up(struct _usbhc* otghc)
{
    unsigned long con;

    con = __raw_readl(USB_OP_MOD_REG);
    con &= ~(0x7 << 0);
    con |= (3 << 1);
    __raw_writel(con, USB_OP_MOD_REG);
}

static inline void usb_set_address(unsigned char address)
{
    hc_writeb(0x0, USB_REG_FADDR);
}

#endif