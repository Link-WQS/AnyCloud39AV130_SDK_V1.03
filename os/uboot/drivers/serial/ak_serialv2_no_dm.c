/*
 * (C) Copyright 2002
 * Gary Jennejohn, DENX Software Engineering, <garyj@denx.de>
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#include <common.h>
#include <linux/compiler.h>
#include <asm/io.h>
#include "ak_serialv2_no_dm.h"

DECLARE_GLOBAL_DATA_PTR;

#include <asm/io.h>
#include <serial.h>

/************* h322-d *************/
#define CFG_INTERGER_SHIFT      16
#define CFG_FRACTION_SHIFT      0

// #define CFG_INTERGER_MASK       0xffff
// #define CFG_FRACTION_MASK       0xf


//
#define CONFIG_SERIAL0

#define UART0_L2_TX_BUF_BASE        0x48001000
#define UART0_L2_RX_BUF_BASE        0x48001080
#define UART1_L2_TX_BUF_BASE        0x48001100
#define UART1_L2_RX_BUF_BASE        0x48001180

#define UART_L2_RX_BUF_SIZE     128

/* Multi serial device functions */
#define DECLARE_AK_SERIAL_FUNCTIONS(port) \
    int akserial##port##_init(void) \
    { \
        return serial_init_dev(port); \
    } \
    void akserial##port##_setbrg(void) \
    { \
        serial_setbrg_dev(port); \
    } \
    int akserial##port##_getc(void) \
    { \
        return serial_getc_dev(port); \
    } \
    int akserial##port##_tstc(void) \
    { \
        return serial_tstc_dev(port); \
    } \
    void akserial##port##_putc(const char c) \
    { \
        serial_putc_dev(port, c); \
    } \
    void akserial##port##_puts(const char *s) \
    { \
        serial_puts_dev(port, s); \
    }

#define INIT_AK_SERIAL_STRUCTURE(port, __name) {    \
    .name   = __name,               \
    .start  = akserial##port##_init,        \
    .stop   = NULL,                 \
    .setbrg = akserial##port##_setbrg,      \
    .getc   = akserial##port##_getc,        \
    .tstc   = akserial##port##_tstc,        \
    .putc   = akserial##port##_putc,        \
    .puts   = akserial##port##_puts,        \
}

#ifdef CONFIG_HWFLOW
static int hwflow;
#endif

// static int l2_uartbuffer_offset = 0;

extern unsigned long get_asic_freq(void);

void _serial_setbrg(const int dev_index)
{
}

static inline void serial_setbrg_dev(unsigned int dev_index)
{
    _serial_setbrg(dev_index);
}

/*
 * get uart base address.
 */
static inline unsigned long serial_reg_base(unsigned int dev_index)
{
    return (dev_index == ak39_uart0) ? UART0_BASE_ADDR : UART1_BASE_ADDR;
}

static inline unsigned long serial_l2buffer_base(unsigned int dev_index)
{
    return (dev_index == ak39_uart0) ?
        UART0_L2_RX_BUF_BASE : UART1_L2_RX_BUF_BASE;
}

/* Initialise the serial port. The settings are always 8 data bits, no parity,
 * 1 stop bit, no start bits.
 */
static int serial_init_dev(const int dev_index)
{
    unsigned long baud_div_interger, baud_div_fraction;
    unsigned long regval;
    unsigned long asic_freq;
    unsigned long base = serial_reg_base(dev_index); // check ok

    asic_freq = get_asic_freq();

    baud_div_interger = asic_freq / 115200 / 16;
    baud_div_fraction = asic_freq / 115200 % 16;

    // enable uart0 working clock,OK
    *(volatile unsigned int*)(0x0800001c) &= ~(0x1 << 7);

#if defined(CONFIG_KM01A_CODE) || defined(CONFIG_3918AV130_CODE)
    //config share pin  
    *(volatile unsigned int*)(0x0800018C) &= (~(0x3f<<6));//(~(0x240));  
    *(volatile unsigned int*)(0x0800018C) |= ((0x1<<6)|(0x1<<9));
#endif

    regval = readl(base + UART_BAUD_DIV);
    regval &= ~(CFG_INTERGER_MASK << CFG_INTERGER_SHIFT);
    regval |= (baud_div_interger << CFG_INTERGER_SHIFT);

    regval &= ~(CFG_FRACTION_MASK << CFG_FRACTION_SHIFT);
    regval |= (baud_div_fraction << CFG_FRACTION_SHIFT);
    writel(regval, base + UART_BAUD_DIV);

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

    return 0;
}


/*
 * Read a single byte from the serial port. Returns 1 on success, 0
 * otherwise. When the function is succesfull, the character read is
 * written into its argument c.
 */
int _serial_getc(const int dev_index)
{
    unsigned long reg;
    unsigned long base = serial_reg_base(dev_index);
    //unsigned long l2_rxbuffer_base = serial_l2buffer_base(dev_index);
    
    //clear int status
    reg = readl(base + UART_STATUS);
    reg |= ((1<<STA_RX_TIMEOUT) | (1<<STA_RX_FRAME_ERR) | (1<<STA_RX_TH_INT));//clear rx timeout and frame err
    writel(reg, base + UART_STATUS);

    while(1)
    {
        if (((readl(base + UART_FIFO_STATUS) >> 16) & 0x3FF) > 0) {
            reg = readl(base + UART_TX_RX_DATA);
            return (reg & (0xff));
        }
    }
    return 0;
}

/*
 * get serial device.
 */
static inline int serial_getc_dev(unsigned int dev_index)
{
    return _serial_getc(dev_index);
}

#ifdef CONFIG_HWFLOW
/*
 * hardware flow control
 */
int hwflow_onoff(int on)
{
    switch (on) {
    case 0:
    default:
        break;      /* return current */
    case 1:
        hwflow = 1; /* turn on */
        break;
    case -1:
        hwflow = 0; /* turn off */
        break;
    }
    return hwflow;
}
#endif

#ifdef CONFIG_MODEM_SUPPORT
static int be_quiet = 0;
void disable_putc(void)
{
    be_quiet = 1;
}

void enable_putc(void)
{
    be_quiet = 0;
}
#endif

#if 0
int test_dd(void)
{
#define REG32(_register_) (*(volatile unsigned long *)(_register_))
    ulong tmp = 0;
    tmp = REG32(0X20170000);
    tmp |= 1 << 29;
    REG32(0X20170000) = tmp;


    tmp = REG32(0X2017000c);
    tmp |= 0 << 29;
    REG32(0X2017000c) = tmp;
    return 0;
}
#endif

/*
 * Output a single byte to the serial port.
 */
void _serial_putc(const char c, const int dev_index)
{
    unsigned long regval;
    unsigned long base = serial_reg_base(dev_index);

    while((readl(base + UART_STATUS) & (0x1 << STA_TX_FIFO_FULL)) != 0);
    regval = (c << TX_DAT);
    writel(regval, base + UART_TX_RX_DATA);//DMA disable
}

static inline void serial_putc_dev(unsigned int dev_index, const char c)
{
    _serial_putc(c, dev_index);
}

/*
 * Test whether a character is in the RX buffer
 */
int _serial_tstc(const int dev_index)
{
    unsigned long base = serial_reg_base(dev_index);

    if((readl(base + UART_STATUS) & (0x1 << 4))) {
        return 0;
    }

    return 1;
}

static inline int serial_tstc_dev(unsigned int dev_index)
{
    return _serial_tstc(dev_index);
}

/*
 * function: output string.
 * param: dev index, string.
 */
void _serial_puts(const char *s, const int dev_index)
{
    while (*s) {
        if(*s == '\n') {
            _serial_putc('\r', dev_index);
        }
        _serial_putc(*s++, dev_index);
    }
}

static inline void serial_puts_dev(int dev_index, const char *s)
{
    _serial_puts(s, dev_index);
}

DECLARE_AK_SERIAL_FUNCTIONS(0);
struct serial_device ak_serial0_device =
INIT_AK_SERIAL_STRUCTURE(0, "ak_serial0");
DECLARE_AK_SERIAL_FUNCTIONS(1);
struct serial_device ak_serial1_device =
INIT_AK_SERIAL_STRUCTURE(1, "ak_serial1");

__weak struct serial_device *default_serial_console(void)
{
#if defined(CONFIG_SERIAL0)
    return &ak_serial0_device;
#elif defined(CONFIG_SERIAL1)
    return &ak_serial1_device;
#else
#error "CONFIG_SERIAL? missing."
#endif
}

/*
 * function:init serial device
 * param: null
 */
void ak_serial_initialize(void)
{
    serial_register(&ak_serial0_device);
    serial_register(&ak_serial1_device);
}
