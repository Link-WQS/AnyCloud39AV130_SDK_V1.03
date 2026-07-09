/**
>>>>>>> AK3.1.16
 * Copyright (C) 2020 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @date 2020-11-15
 */


#include <common.h>
#include <asm/io.h>
#include <serial.h>
#include <dm.h>
#include <clk.h>
#include <linux/io.h>
#include "ak_serial.h"

DECLARE_GLOBAL_DATA_PTR;

#define CONFIG_SERIAL0

#define CLOCK_GATE_CONTROL          0x0800001C
#define PIN_CONTROL_REG5            0x0800018C
#define CPU_CTRL_BUFFER             0x2014008C
#define FRACTION_DMA_ADDRESS        0x20140084

#define UART0_L2_TX_BUF_BASE        0x48001000
#define UART0_L2_RX_BUF_BASE        0x48001080
#define UART1_L2_TX_BUF_BASE        0x48001100
#define UART1_L2_RX_BUF_BASE        0x48001180

#define UART_L2_RX_BUF_SIZE     128

static int l2_uartbuffer_offset = 0;

struct ak_serial_priv {
    void __iomem *base;
    ulong uartclk;
};

static inline unsigned long serial_l2buffer_base(unsigned int dev_index)
{
    return (dev_index == ak39_uart0) ? UART0_L2_RX_BUF_BASE
                                     : UART1_L2_RX_BUF_BASE;
}

/* Initialise the serial port. The settings are always 8 data bits, no parity,
 * 1 stop bit, no start bits.
 */
static int ak_serial_init(void __iomem *base, ulong clk, u32 baudrate)
{
    unsigned long baud_div;
    unsigned long regval;
    unsigned long asic_freq;

    asic_freq = clk;

    baud_div = asic_freq/baudrate - 1;

    /* enable uart0 working clock,OK*/
    regval = readl(CLOCK_GATE_CONTROL);
    regval &= ~(0x1 << 7);
    writel(regval, CLOCK_GATE_CONTROL);

#ifdef CONFIG_39EV33X_CODE
    /* config share pin */
    regval = readl(0x08000074);
    regval |= (0x3 << 2);
    writel(regval, 0x08000074);

    regval = readl(0x08000080);
    regval &= ~(0x3 << 1);
    writel(regval, 0x08000080);
#endif

#ifdef CONFIG_37_E_CODE
    /* config share pin */
    regval = readl(0x08000164);
    regval &= (~(0x3f<<6));
    writel(regval, 0x08000164);

    regval = readl(0x08000164);
    regval |= ((0x1<<6)|(0x1<<9));
    writel(regval, 0x08000164);
#endif

#ifdef CONFIG_37_D_CODE
    /* config share pin */
    regval = readl(SHARE_PIN_CFG0_REG);
    regval &= (~(0xf<<0));
    writel(regval, SHARE_PIN_CFG0_REG);

    regval = readl(SHARE_PIN_CFG0_REG);
    regval  |= (0x5<<0);
    writel(regval, SHARE_PIN_CFG0_REG);

#endif

#ifdef CONFIG_3918AV100_CODE
    /* config share pin */
    regval = readl(PIN_CONTROL_REG5);
    regval &= (~(0x3f<<6));
    writel(regval, PIN_CONTROL_REG5);

    regval = readl(PIN_CONTROL_REG5);
    regval |= ((0x1<<6)|(0x1<<9));
    writel(regval, PIN_CONTROL_REG5);
#endif

#ifdef CONFIG_3918EV300L_CODE
    /* config share pin */
    regval = readl(PIN_CONTROL_REG5);
    regval &= (~(0x3f<<6));
    writel(regval, PIN_CONTROL_REG5);

    regval = readl(PIN_CONTROL_REG5);
    regval |= ((0x1<<6)|(0x1<<9));
    writel(regval, PIN_CONTROL_REG5);
#endif

#ifdef CONFIG_39EV200_CODE
    /* config share pin */
    regval = readl(0x08000074);
    regval |= (0x3 << 1);
    writel(regval, 0x08000074);

    regval = readl(0x08000080);
    regval &= ~(0x3 << 1);
    writel(regval, 0x08000080);
#endif

    /* config l2 buffer state enable ahb_flag, dma_flag */
    regval = readl(FRACTION_DMA_ADDRESS);
    regval |= (0x3<<28);
    writel(regval, FRACTION_DMA_ADDRESS);

    /* add clean l2 buffer uart buffer status */
    regval = readl(CPU_CTRL_BUFFER);
    regval |= (0x3<<16);
    writel(regval, CPU_CTRL_BUFFER);

    /* initial l2 uart offset */
    l2_uartbuffer_offset = 0;

    regval = (UART_CLEAR_RX_STAT|UART_CLEAR_TX_STAT |UART_ENABLE|baud_div);
    writel(regval, base + UART_CONF_REG);

    /* set rx_threshold=2/1B, tx_threshold=4B */
    regval = readl(base + UART_THRESHOLD_REG);
    regval &= ~(0x1f<<0);
    regval &= ~(0x1<<5);
    regval |= 0x1;
    writel(regval, base + UART_THRESHOLD_REG);

    /* set rx wait timeout=32 cycle */
    writel((0x1F << 16)|(0x1 << 0), base + UART_TIMEOUT_REG);

    /* start receiver */
    regval = readl(base + UART_THRESHOLD_REG);
    regval |= (1UL<<31);
    writel(regval, base + UART_THRESHOLD_REG);

    return 0;
}

static int ak_serial_setbrg(struct udevice *dev, int baudrate)
{
    struct ak_serial_priv *priv = dev_get_priv(dev);

    return ak_serial_init(priv->base, priv->uartclk, baudrate);
}

static int ak_serial_pending(struct udevice *dev, bool input)
{
    struct ak_serial_priv *priv = dev_get_priv(dev);
    unsigned long regval;
    unsigned long status = 0;

    regval = (readl(priv->base + UART_COUNT_REG)>>13) & 0x7f;

    /*
     * Test whether a character is in the RX buffer
     */
    if (input){
        /* 
         * bit[17]
         * uart_wmem_Nrdy
         * 0: The receive buffer is ready to receive new data.
         * 1: There is no enough buffer to store new data.
         */
        if((readl(priv->base + UART_STATUS_REG) & (0x1 << 17))){
            return 1;  //add by lhd
        }
        /* 
         * bit[1]
         * rx_fifo_full
         * 1: The receive FIFO is full.
         * NOTE: This bit is cleared by being written with 1.
         */
        else if((readl(priv->base + UART_STATUS_REG) & UART_RX_FIFO_FULL)){
            /* write 1 to clear */
            status |= (UART_RX_FIFO_FULL);
            writel(status, priv->base + UART_STATUS_REG);
            return 1;  //add by lhd
        }
        /* 
         * bit[30]
         * rx_th_int_sta
         * 0: The rx_th interrupt is not generated.
         * 1: A rx_th interrupt is generated.
         */
        else if((readl(priv->base + UART_STATUS_REG) & UART_RX_TH_INT_STA)){
            status |= (UART_RX_TH_INT_STA);
            writel(status, priv->base + UART_STATUS_REG);
            return 1;
        }
        else if((l2_uartbuffer_offset%UART_L2_RX_BUF_SIZE) != regval)
            return 1;
        else
            return 0;
    }else{
    /*
     * Test whether a character is in the TX buffer
     * UART_TX_END tx_end
     * 1: All the data in TX buffer has been sent.
     *
     * UART_TX_FIFO_EMPTY tx_fifo_empty
     * 1: The transmit buffer is empty.
     * 
     * better use UART_TX_FIFO_EMPTY
     */
     return (readl(priv->base + UART_STATUS_REG) & UART_TX_FIFO_EMPTY)? 0 : 1;  
    }
}
/*
 * Read a single byte from the serial port. Returns 1 on success, 0
 * otherwise. When the function is succesfull, the character read is
 * written into its argument c.
 */
static int ak_serial_getc(struct udevice *dev)
{
    struct ak_serial_priv *priv = dev_get_priv(dev);
    unsigned long reg;
    unsigned long l2_rxbuffer_base = serial_l2buffer_base(ak39_uart0);

    reg = readl(priv->base + UART_STATUS_REG);
    reg |= (UART_RX_TO_FLAG | UART_RX_FP_ERR);
    writel(reg, priv->base + UART_STATUS_REG);

    while(1)
    {
        if (l2_uartbuffer_offset == UART_L2_RX_BUF_SIZE) {
            l2_uartbuffer_offset = 0;
        }

        reg = (readl(priv->base + UART_COUNT_REG)>>13) & 0x7f;

        if (l2_uartbuffer_offset != reg) {
            return readb(l2_rxbuffer_base + l2_uartbuffer_offset++);
        }

        /*
         * case: there is no enough buffer to store new data
         */
        if((readl(priv->base + UART_STATUS_REG) & (0x1 << 17))) {
            /* add clean l2 buffer uart buffer status */
            reg = readl(CPU_CTRL_BUFFER);
            reg |= (0x1<<17);
            writel(reg, CPU_CTRL_BUFFER);
            
            reg = readl(priv->base + UART_CONF_REG);
            reg |= UART_CLEAR_RX_STAT;
            writel(reg, priv->base + UART_CONF_REG);
        }
    }

    return 0;
}

/*
 * Output a single byte to the serial port.
 */
static int ak_serial_putc(struct udevice *dev, const char ch)
{
    struct ak_serial_priv *priv = dev_get_priv(dev);
    unsigned long regval;

    /* Clear the uart tx buf */
    regval = readl(CPU_CTRL_BUFFER);
    regval |= (0x1<<16);
    writel(regval, CPU_CTRL_BUFFER);

    /* write data to l2_buffer*/
    writel((unsigned int)ch, 0x48001000);

    /* make l2_buffer to transfer data*/
    writel((unsigned int)'\0', (0x48001000 + 60));

    regval = readl(priv->base + UART_CONF_REG);
    regval |= (0x1 << 28);
    writel(regval, priv->base + UART_CONF_REG);

    regval = readl(priv->base + UART_STATUS_REG);
    regval |= (0x1 << 4)|(0x1 << 16);
    writel(regval, priv->base + UART_STATUS_REG);

    while((readl(priv->base + UART_STATUS_REG) & (0x1 << 19)) == 0)
        ;

    return 0;
}

static int ak_serial_probe(struct udevice *dev)
{
    struct ak_serial_priv *priv = dev_get_priv(dev);
    fdt_addr_t addr;
    fdt_size_t size;
    struct clk clk;
    int ret;

    /* get address */
    addr = fdtdec_get_addr_size(gd->fdt_blob, dev_of_offset(dev), "reg",
                    &size);
    if (addr == FDT_ADDR_T_NONE)
        return -EINVAL;

    priv->base = ioremap(addr, size);

    /* get clock rate */
    ret = clk_get_by_index(dev, 0, &clk);
    if (ret) {
        pr_err("%s: fail to get clk\n", __func__);
        return ret;
    }
    ret = clk_enable(&clk);
    if (ret) {
        pr_err("%s: fail to enable clk\n", __func__);
        return ret;
    }
    priv->uartclk = clk_get_rate(&clk);
    //pr_err("%s: uartclk %lu\n", __func__, priv->uartclk);

    /* initialize serial */
    return ak_serial_init(priv->base, priv->uartclk, CONFIG_BAUDRATE);
}

static const struct dm_serial_ops ak_serial_ops = {
    .putc = ak_serial_putc,
    .pending = ak_serial_pending,
    .getc = ak_serial_getc,
    .setbrg = ak_serial_setbrg,
};

static const struct udevice_id ak_serial_ids[] = {
    { .compatible = "anyka,ak37e-uart0" },
    { .compatible = "anyka,ak37d-uart0" },
    { .compatible = "anyka,ak39ev330-uart0" },
    { .compatible = "anyka,ak3918av100-uart0" },
    { .compatible = "anyka,ak39ev200-uart0" },
    { .compatible = "anyka,ak3918ev300l-uart0" },
    { }
};

U_BOOT_DRIVER(serial_ak) = {
    .name = "serial_ak",
    .id = UCLASS_SERIAL,
    .of_match = ak_serial_ids,
    .priv_auto_alloc_size = sizeof(struct ak_serial_priv),
    .probe = ak_serial_probe,
    .ops = &ak_serial_ops,
    .flags = DM_FLAG_PRE_RELOC,
};
