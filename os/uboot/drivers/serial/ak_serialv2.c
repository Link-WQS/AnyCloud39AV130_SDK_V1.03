/**
 * Copyright (C) 2023 Anyka (GuangZhou) Software Technology Co., Ltd.
 *
 * @brief Anyka UARTV2 driver
 * @date 2023-7-17
 * @author Yang JianXiong
 */

#include <common.h>
#include <asm/io.h>
#include <serial.h>
#include <dm.h>
#include <clk.h>
#include <linux/io.h>
#include "ak_serialv2.h"

#include <asm/arch-km01a/ak_cpu.h>

DECLARE_GLOBAL_DATA_PTR;


/************* h322-d *************/
#define CFG_INTERGER_SHIFT      16
#define CFG_FRACTION_SHIFT      0

#define CFG_INTERGER_MASK       0xffffUL
#define CFG_FRACTION_MASK       0xf

/************* h322-d *************/

#define CLOCK_GATE_CONTROL          0x0800001C
#define PIN_CONTROL_REG5            0x0800018C
#define PIN_CONTROL_REG0            0x08000178

//#define AK_UART_DEBUG

#if defined(AK_UART_DEBUG)
static void for_loop(unsigned int times)
{
    unsigned int i;
    for(i = 0; i < times; i++)
        asm volatile("nop");
}

static void gpio22_on_and_off(unsigned char on)
{
    /* set driver */
    *(volatile unsigned int *)(0x080001a8) |= 0x3 << 12;
    /* set pinmux */
    *(volatile unsigned int *)(0x08000180) &= ~(0x7 << 9);
    *(volatile unsigned int *)(0x08000180) |= 3 << 9;
    /* set direction output mode */
    *(volatile unsigned int *)(0x20170000) |= 0x1 << 22;
    // /* set direction input mode */
    // *(volatile unsigned int *)(0x20170000) &= ~(0x1 << 22);

    if(on){
        /* set output high level */
        *(volatile unsigned int *)(0x20170014) |= 0x1 << 22;
    }
    else{
        /* set output low level */
        *(volatile unsigned int *)(0x20170014) &= ~(0x1 << 22);
    }
}

static void gpio70_on_and_off(unsigned char on)
{
    /* set pinmux */
    *(volatile unsigned int *)(0x08000198) &= ~(0x7 << 24);
    *(volatile unsigned int *)(0x08000198) |= 0 << 24;
    /* set direction output mode */
    *(volatile unsigned int *)(0x20170008) |= 0x1 << 6;
    if(on){
        /* set output high level */
        *(volatile unsigned int *)(0x2017001c) |= 0x1 << 6;
    }
    else{
        /* set output low level */
        *(volatile unsigned int *)(0x2017001c) &= ~(0x1 << 6);
    }
}

void gpio70_blinding(unsigned times)
{
    unsigned int i;
    for(i = 0; i < times; i++){
        gpio70_on_and_off(1);
        for_loop(2*10*1000*1000);
        gpio70_on_and_off(0);
        for_loop(2*10*1000*1000);
    }
}
#endif



struct ak_serial_priv {
    void *base;
    ulong uartclk;
};

static void ak_console_init(void *base)
{
    unsigned int regval = 0;

    //default data format: data_bit(8) stop_bit(1)

    /* disable tx threshold */
    writel(0, base + UART_CFG_TX_THRESHOLD);

    /* enable rx 1 byte threshold */
    writel(RX_TH_EN, base + UART_CFG_RX_THRESHOLD);

    /* fifo ctrl 
    * rx fifo int threshold >= 1/2full (default)
    * tx fifo int threshold <= 1/2full (default)
    */
    regval = readl(base + UART_ALTER_FUNC_CTRL);
    regval &= ~(CFG_RX_INT_SEL_MASK);
    regval &= ~(CFG_TX_INT_SEL_MASK);
    regval |= (RX_FIFO_1_2_FULL | TX_FIFO_1_2_FULL);
    writel(regval, base + UART_ALTER_FUNC_CTRL);

    //mask all interrupt
    writel(0, base + UART_INT_CTRL);

    /* uart en ctrl: clear tx/rx status、enable uart */
    regval = readl(base + UART_CFG);
    //regval |= (CFG_TX_CLR | CFG_RX_CLR | CFG_TX_EN | CFG_RX_EN | CFG_UART_EN);
    regval |= ( CFG_TX_EN | CFG_RX_EN | CFG_UART_EN);
    writel(regval, base + UART_CFG);


    //gpio70_on_and_off(1);//ok

}


/* Initialise the serial port. The settings are always 8 data bits, no parity,
 * 1 stop bit, no start bits.
 */
static int ak_serial_init(void *base, ulong clk, u32 baudrate)
{
    //unsigned long baud_div;
    unsigned long regval;
    unsigned long asic_freq;
    unsigned int baud_div_interger;
    unsigned int baud_div_fraction;

    asic_freq = clk;

#if !defined(CONFIG_Z1_H322D)
    /*波特率的设置*/
    //baudrate = freq/(16*div)
    baud_div_interger = asic_freq/baudrate/16;
    baud_div_fraction = asic_freq%baudrate/16;

    regval = readl(base + UART_BAUD_DIV);
    regval &= ~(CFG_INTERGER_MASK << CFG_INTERGER_SHIFT);
    regval |= (baud_div_interger << CFG_INTERGER_SHIFT);

    regval &= ~(CFG_FRACTION_MASK << CFG_FRACTION_SHIFT);
    regval |= (baud_div_fraction << CFG_FRACTION_SHIFT);
    writel(regval, base + UART_BAUD_DIV);
#endif

#if 0
   /*直接写死寄存器，调试使用*/
#if defined(CONFIG_SERIAL0)
    /* enable uart0 working clock,OK*/
    regval = readl(CLOCK_GATE_CONTROL);
    regval &= ~(0x1 << 7);              //gclk: uart gate clock
    writel(regval, CLOCK_GATE_CONTROL);
#endif

/* enable uart2 working clock,OK*/
#if defined(CONFIG_SERIAL2)
    regval = readl(CLOCK_GATE_CONTROL);
    regval &= ~(0x1 << 18);              //gclk: uart gate clock
    writel(regval, CLOCK_GATE_CONTROL);
#endif

#endif
//gpio70_on_and_off(1);//ok

    ak_console_init(base);

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
    //unsigned long regval;
    unsigned long status = 0;
    
    status = readl(priv->base + UART_STATUS);

    /*
     * Test whether a character is in the RX buffer
     */
    if (input){
        /* 
         * bit[6]
         * uart_wmem_Nrdy
         * 0: The receive buffer is ready to receive new data.
         * 1: There is no enough buffer to store new data.
         */
        if((readl(priv->base + UART_STATUS) & (0x1 << STA_WMEM_NRDY))){
            return 1;  //add by lhd
        }
        /* 
         * bit[5]
         * rx_fifo_full
         * 1: The receive FIFO is full.
         * NOTE: This bit is cleared by being written with 1.
         */
        else if((readl(priv->base + UART_STATUS) & (1 << STA_RX_FIFO_FULL))){
            /* write 1 to clear */
            status |= (1<<STA_RX_FIFO_FULL);
            writel(status, priv->base + UART_STATUS);
            return 1;  //add by lhd
        }
        /* 
         * bit[8]
         * rx_th_int_sta
         * 0: The rx_th interrupt is not generated.
         * 1: A rx_th interrupt is generated.
         */
        else if((readl(priv->base + UART_STATUS) & (1<<STA_RX_TH_INT))){
            status |= (1<<STA_RX_TH_INT);
            writel(status, priv->base + UART_STATUS);
            return 1;
        }
        /* 
         * UART_FIFO_STATUS
         * bit[25:16]
         * rx fifo level
         * rx count
         */
        else if ((readl(priv->base + UART_FIFO_STATUS) >> 16) & 0x3FF) {
            return 1;
        }
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
    return (readl(priv->base + UART_STATUS) & (1<<STA_TX_FIFO_EMPTY))? 0 : 1;  
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

    /* Clear rx status */
    //reg = readl(priv->base + UART_CFG);
    //reg |= CFG_RX_CLR;
    //writel(reg, priv->base + UART_CFG);

    //clear int status
    reg = readl(priv->base + UART_STATUS);
    reg |= ((1<<STA_RX_TIMEOUT) | (1<<STA_RX_FRAME_ERR) | (1<<STA_RX_TH_INT));//clear rx timeout and frame err
    writel(reg, priv->base + UART_STATUS);


    while(1)
    {
        if (((readl(priv->base + UART_FIFO_STATUS) >> 16) & 0x3FF) > 0) {
            reg = readl(priv->base + UART_TX_RX_DATA);
            return (reg & (0xff));
        }

        /**
         * This position is unreachable when dma is disabled
        */

        /*
         * case: there is no enough buffer to store new data
         */
        //if((readl(priv->base + UART_STATUS) & UART_WMEM_RDY)) {
            /* add clean l2 buffer uart buffer status */
  
        //}
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

    /* Clear tx status */
    // regval = readl(priv->base + UART_CFG);
    // regval |= CFG_TX_CLR;
    // writel(regval, priv->base + UART_CFG);

    //clear int status
    // regval = readl(priv->base + UART_STATUS);
    // regval |= ((1<<STA_TX_END) | (1<<STA_TX_TH_INT));
    // writel(regval, priv->base + UART_STATUS);

    while((readl(priv->base + UART_STATUS) & (0x1 << STA_TX_FIFO_FULL)) != 0);
    /* write data to tx_fifo */
    // regval = readl(priv->base + UART_TX_RX_DATA);
    regval = (ch << TX_DAT);
    writel(regval, priv->base + UART_TX_RX_DATA);//DMA disable

    /* wait for tx finish */
    // while((readl(priv->base + UART_STATUS) & (0x1 << STA_TX_END)) == 0);

    return 0;
}

static int ak_serial_probe(struct udevice *dev)
{
    struct ak_serial_priv *priv = dev_get_priv(dev);
    fdt_addr_t addr;
    fdt_size_t size;
    struct clk clk;
    int ret;

    unsigned int regval = 0;
#if 0
    /*reset */
    regval = readl(0x08000020);
    regval &= ~(0xFFFFFFFF);
    writel(regval, 0x08000020);

#endif


#if 1
/*直接写死寄存器，调试使用*/
if(CONFIG_CONS_INDEX == 0){

/* config h322d share pin */
regval = readl(PIN_CONTROL_REG5);
regval &= (~(0x3f<<6));
writel(regval, PIN_CONTROL_REG5);

regval = readl(PIN_CONTROL_REG5);
regval |= ((0x1<<6)|(0x1<<9));
writel(regval, PIN_CONTROL_REG5);

}

if(CONFIG_CONS_INDEX == 2){
    /*板子串口2调试*/
    /* config h322d share pin */
    regval = readl(0x0800018C);
    regval &= (~(0x3f<<18));
    writel(regval, 0x0800018C);

    regval = readl(0x0800018C);
    regval |= ((0x2<<18)|(0x2<<21));
    writel(regval, 0x0800018C);
    
   #if defined(AK_UART_DEBUG)
     gpio70_on_and_off(1);//ok
   #endif  
}
#endif

    /* get address */
    addr = fdtdec_get_addr_size(gd->fdt_blob, dev_of_offset(dev), "reg",
                    &size);
    if (addr == FDT_ADDR_T_NONE)
        return -EINVAL;

    priv->base=(void *)addr;

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
    { .compatible = "anyka,ak3918av100plus-uart0" },
    { .compatible = "anyka,ak3918av100plus-uart2" },
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
