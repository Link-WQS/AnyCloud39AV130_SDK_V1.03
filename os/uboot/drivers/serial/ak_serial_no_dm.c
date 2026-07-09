/*
 * (C) Copyright 2002
 * Gary Jennejohn, DENX Software Engineering, <garyj@denx.de>
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#include <common.h>
#include <linux/compiler.h>
#include <asm/io.h>
#include "ak_serial.h"

DECLARE_GLOBAL_DATA_PTR;

#include <asm/io.h>
#include <serial.h>


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

static int l2_uartbuffer_offset = 0;

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
    unsigned long baudrate;
    unsigned long regval;
    unsigned long asic_freq;
    unsigned long base = serial_reg_base(dev_index); // check ok

    asic_freq = get_asic_freq();
    baudrate = asic_freq/115200 - 1;

    // enable uart0 working clock,OK
    *(volatile unsigned int*)(0x0800001c) &= ~(0x1 << 7);

#ifdef CONFIG_39EV33X_CODE
    // cfg uart0 share pin ,OK
    *(volatile unsigned int*)(0x08000074) |= (0x3 << 2);
    // enabel uart0 pin pull up , OK
    *(volatile unsigned int*)(0x08000080) &= ~(0x3 << 1);

#endif

#ifdef CONFIG_37_E_CODE
    //config share pin  
    *(volatile unsigned int*)(0x08000164) &= (~(0x3f<<6));//(~(0x240));  
    *(volatile unsigned int*)(0x08000164) |= ((0x1<<6)|(0x1<<9));
#endif

#ifdef CONFIG_37_D_CODE
    //config share pin  
    *(volatile unsigned int*)(SHARE_PIN_CFG0_REG) &= (~(0xf<<0));  
    *(volatile unsigned int*)(SHARE_PIN_CFG0_REG) |= (0x5<<0);
#endif


#ifdef CONFIG_3918AV100_CODE
    //config share pin  
    *(volatile unsigned int*)(0x0800018C) &= (~(0x3f<<6));//(~(0x240));  
    *(volatile unsigned int*)(0x0800018C) |= ((0x1<<6)|(0x1<<9));
#endif

#ifdef CONFIG_3918EV300L_CODE
    //config share pin  
    *(volatile unsigned int*)(0x0800018C) &= (~(0x3f<<6));//(~(0x240));  
    *(volatile unsigned int*)(0x0800018C) |= ((0x1<<6)|(0x1<<9));
#endif

#ifdef CONFIG_39EV200_CODE
    *(volatile unsigned int*)(0x08000074) |= (0x3<<1);
    *(volatile unsigned int*)(0x08000080) &= ~(0x3<<1);
#endif

    //config l2 buffer state enable ahb_flag, dma_flag
    *(volatile unsigned int*)(0x20140084) |= (0x3<<28);

    // add clean l2 buffer uart buffer status
    *(volatile unsigned int*)(0x2014008C) |= (0x3<<16);

    // initial l2 uart offset 
    l2_uartbuffer_offset = 0;
    
    regval = (UART_CLEAR_RX_STAT|UART_CLEAR_TX_STAT |UART_ENABLE|baudrate);
    writel(regval, base + UART_CONF_REG);

    // set rx_threshold=2/1B, tx_threshold=4B
    regval = readl(base + UART_THRESHOLD_REG);
    regval &= ~(0x1f<<0);
    regval &= ~(0x1<<5);
    regval |= 0x0;
    writel(regval, base + UART_THRESHOLD_REG);

    // set rx wait timeout=32 cycle
    writel((0x1F << 16)|(0x1 << 0), base + UART_TIMEOUT_REG);

    // start receiver
    regval = readl(base + UART_THRESHOLD_REG);
    regval |= (1UL<<31);
    writel(regval, base + UART_THRESHOLD_REG);
    
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
    unsigned long l2_rxbuffer_base = serial_l2buffer_base(dev_index);
    
    while(1)
    {
        if (l2_uartbuffer_offset == UART_L2_RX_BUF_SIZE) {
            l2_uartbuffer_offset = 0;
        }
        
        reg = (readl(base + UART_COUNT_REG)>>13) & 0x7f;
        if (l2_uartbuffer_offset != reg) {
            return readb(l2_rxbuffer_base + l2_uartbuffer_offset++);
        }
    }
    
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

    /*FIXME*/
    *(volatile unsigned int*)(0x2014008c) |= (0x1<<16); //Clear the uart tx buf
    *(volatile unsigned int*)(0x48001000) = (unsigned int)c; 
    *(volatile unsigned int*)(0x48001000 + 60) = (unsigned int)'\0'; 

    regval = readl(base + UART_CONF_REG);
    regval |= (0x1 << 28);
    writel(regval, base + UART_CONF_REG);

    regval = readl(base + UART_STATUS_REG);
    regval |= (0x1 << 4)|(0x1 << 16);
    writel(regval, base + UART_STATUS_REG);
    
    while((readl(base + UART_STATUS_REG) & (0x1 << 19)) == 0)
        ;
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

    if((readl(base + UART_STATUS_REG) & (0x1 << 30)) == 0) {
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
