/** @file
 * @brief Define the register of ANYKA CPU
 *
 * Define the register address and bit map for system.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author xuchang
 * @date 2008-01-05
 * @version 1.0
 * @note
 * ¸ÃÎÄ¼þ¶¨ÒåËùÓÐÇý¶¯Ä£¿éµÄ¼Ä´æÆ÷£¬²»µÃÔÚÆäËûµØ·½×ö¼Ä´æÆ÷¶¨Òå!!
 * ¼Ä´æÆ÷µÄÎ»¶¨ÒåÔ­ÔòÉÏ·ÅÔÚ¸÷¸öÇý¶¯Ä£¿éµÄÍ·ÎÄ¼þÖÐ¶¨Òå£¬Èç¹û¶ÔÓ¦
 * µÄÇý¶¯Ä£¿éÌ«Ð¡Ã»ÓÐÍ·ÎÄ¼þ£¬
 * Ôò¿É·ÅÔÚ´Ë´¦¶¨Òå
 */
#ifndef _AK_CPU_H_
#define _AK_CPU_H_

/** @defgroup ANYKA_CPU
 *      @ingroup Drv_Lib
 */
/*@{*/

/** @{@name Base Address Define
 *      The base address of system memory space is define here.
 *      Include memory assignment and module base address define.
 */
/**Memory assignment*/
#define CHIP_CONF_BASE_ADDR  0x08000000 // chip configurations
#define USB_BASE_ADDR        0x70000000 // USB
#define L2_BUF_MEM_BASE_ADDR 0x48000000 // L2 Buffer start address
/** @} */

/** @{@name CPU working mode
 */
#define ANYKA_CPU_Mode_USR   0x10
#define ANYKA_CPU_Mode_FIQ   0x11
#define ANYKA_CPU_Mode_IRQ   0x12
#define ANYKA_CPU_Mode_SVC   0x13
#define ANYKA_CPU_Mode_ABT   0x17
#define ANYKA_CPU_Mode_UNDEF 0x1B
#define ANYKA_CPU_Mode_SYS   0x1F
#define ANYKA_CPU_I_Bit      0x80
#define ANYKA_CPU_F_Bit      0x40
/** @} */

/** @{@name System Control Register
 *      Define system control register here, include CLOCK/INT/RESET
 */
#define CLK_ASIC_PLL_CTRL (CHIP_CONF_BASE_ADDR + 0x00000008)

#define CORE_PLL_CHANNEL_CTRL_REG (CHIP_CONF_BASE_ADDR + 0x00000008)
#define CPU_PLL_CHANNEL_CTRL_REG  (CHIP_CONF_BASE_ADDR + 0x00000004)

#define PERIL_PLL_CHANNEL_CTRL_REG (CHIP_CONF_BASE_ADDR + 0x00000014)

#define CLOCK_CTRL_REG                                                         \
    (CHIP_CONF_BASE_ADDR + 0x0000001C) // module clock control(switch)
#define CLOCK_CTRL2_REG                                                        \
    (CHIP_CONF_BASE_ADDR + 0x000000FC) // module clock control(switch)
#define RESET_CTRL_REG                                                         \
    (CHIP_CONF_BASE_ADDR + 0x00000020) // module software reset control register
#define RESET_CTRL2_REG                                                        \
    (CHIP_CONF_BASE_ADDR + 0x00000010) // module clock control(switch)
#define IRQINT_MASK_REG                                                        \
    (CHIP_CONF_BASE_ADDR + 0x00000024) // module IRQ interrupt mask register, 1:
                                       // mask; 0:unmask(default);
#define FRQINT_MASK_REG                                                        \
    (CHIP_CONF_BASE_ADDR + 0x00000028) // module FRQ interrupt mask register, 1:
                                       // mask; 0:unmask(default);
#define INT_STATUS_REG                                                         \
    (CHIP_CONF_BASE_ADDR + 0x0000004C) // module interrupt status register
#define INT_SYS_MODULE_REG                                                     \
    (CHIP_CONF_BASE_ADDR                                                       \
        + 0x00000030) // system module interrupt status control register
#define CLOCK_DIV_REG                                                          \
    (CHIP_CONF_BASE_ADDR + 0x00000004) // clock divider register 1
#define CLOCK2X_CTRL_REG                                                       \
    (CHIP_CONF_BASE_ADDR + 0x00000004) // clock2x control register,1: 2*ASIC
                                       // clock; 0: ASIC clock
#define CLOCK3X_CTRL_REG                                                       \
    (CHIP_CONF_BASE_ADDR                                                       \
        + 0x00000064) // bit28 => 1: asic = pll1_clock /3, 0: refresh to
                      // bit[6..8] of 0x08000004 register
#define PLL_NPARAM_REG                                                         \
    (CHIP_CONF_BASE_ADDR                                                       \
        + 0x000000dc) // n configuration register,PLL_Clk = 4*M/N
/** @} */

#define SHARE_PIN_CFG0_REG                                                     \
    (CHIP_CONF_BASE_ADDR + 0x00000074) // 0x00000178)     // SHARE PIN CFG1
#define SHARE_PIN_CFG1_REG                                                     \
    (CHIP_CONF_BASE_ADDR + 0x00000078) // 0x0000017C)     // SHARE PIN CFG2
#define SHARE_PIN_CFG2_REG                                                     \
    (CHIP_CONF_BASE_ADDR + 0x0000007C) // 0x00000180)     // SHARE PIN CFG3
#define SHARE_PIN_CFG3_REG                                                     \
    (CHIP_CONF_BASE_ADDR + 0x000000DC) // 0x00000184)     // SHARE PIN CFG4

#define SHARE_PIN_CFG4_REG                                                     \
    (CHIP_CONF_BASE_ADDR + 0x00000014) // 0x00000188)     // SHARE PIN CFG5
#define SHARE_PIN_CFG5_REG                                                     \
    (CHIP_CONF_BASE_ADDR + 0x000000A4) // 0x0000018C)     // SHARE PIN CFG6

#define PU_PD_ENABLE_CFG0_REG (CHIP_CONF_BASE_ADDR + 0x00000080) // 0x00000194)
#define PU_PD_ENABLE_CFG1_REG (CHIP_CONF_BASE_ADDR + 0x00000084) // 0x00000198)
#define PU_PD_ENABLE_CFG2_REG (CHIP_CONF_BASE_ADDR + 0x00000088) // 0x0000019c)
#define PU_PD_ENABLE_CFG3_REG (CHIP_CONF_BASE_ADDR + 0x000000E0) // 0x000001F8)

#define PU_PD_SEL_CFG0_REG (CHIP_CONF_BASE_ADDR + 0x000001a0)
#define PU_PD_SEL_CFG1_REG (CHIP_CONF_BASE_ADDR + 0x000001a4)
#define PU_PD_SEL_CFG2_REG (CHIP_CONF_BASE_ADDR + 0x000001e8)
#define PU_PD_SEL_CFG3_REG (CHIP_CONF_BASE_ADDR + 0x000001F8)

#define PAD_DRV_CFG0_REG (CHIP_CONF_BASE_ADDR + 0x000001d0)
#define PAD_DRV_CFG1_REG (CHIP_CONF_BASE_ADDR + 0x000001d4)
#define PAD_DRV_CFG2_REG (CHIP_CONF_BASE_ADDR + 0x000001d8)
#define PAD_DRV_CFG3_REG (CHIP_CONF_BASE_ADDR + 0x000001dc)
#define PAD_DRV_CFG4_REG (CHIP_CONF_BASE_ADDR + 0x000001e0)
#define PAD_DRV_CFG5_REG (CHIP_CONF_BASE_ADDR + 0x000001e4)
#define PAD_DRV_CFG6_REG (CHIP_CONF_BASE_ADDR + 0x000001F8)

#define PAD_IE_CFG0_REG (CHIP_CONF_BASE_ADDR + 0x000001ec)
#define PAD_IE_CFG1_REG (CHIP_CONF_BASE_ADDR + 0x000001f0)
#define PAD_IE_CFG2_REG (CHIP_CONF_BASE_ADDR + 0x000001f4)
#define PAD_IE_CFG3_REG (CHIP_CONF_BASE_ADDR + 0x000001F8)

/** @{@name L2 memory register and bit map define
 */
#define L2_BASE_ADDR   0x20140000
#define L2_DMA_ADDR    (L2_BASE_ADDR + 0x00)
#define L2_DMA_CNT     (L2_BASE_ADDR + 0x40)
#define L2_DMA_REQ     (L2_BASE_ADDR + 0x80)
#define L2_FRAC_ADDR   (L2_BASE_ADDR + 0x84)
#define L2_COMBUF_CFG  (L2_BASE_ADDR + 0x88)
#define L2_UARTBUF_CFG (L2_BASE_ADDR + 0x8c)
#define L2_ASSIGN_REG1 (L2_BASE_ADDR + 0x90)
#define L2_ASSIGN_REG2 (L2_BASE_ADDR + 0x94)
#define L2_LDMA_CFG    (L2_BASE_ADDR + 0x98)
#define L2_INT_ENA     (L2_BASE_ADDR + 0x9c)
#define L2_STAT_REG1   (L2_BASE_ADDR + 0xa0)
#define L2_STAT_REG2   (L2_BASE_ADDR + 0xa8)

/** @{@name SPI module register and bit map define
 */
#define SPI_MODULE_BASE_ADDR 0x20120000 // SPI
#define SPI0_BASE_ADDR       (SPI_MODULE_BASE_ADDR + 0x00000000)
#define SPI1_BASE_ADDR       (SPI_MODULE_BASE_ADDR + 0x00008000)

#define ASPEN_SPI_CTRL     (0x00)
#define ASPEN_SPI_STA      (0x04)
#define ASPEN_SPI_INTENA   (0x08)
#define ASPEN_SPI_NBR      (0x0c)
#define ASPEN_SPI_TX_EXBUF (0x10)
#define ASPEN_SPI_RX_EXBUF (0x14)
#define ASPEN_SPI_TX_INBUF (0x18)
#define ASPEN_SPI_RX_INBUF (0x1c)
#define ASPEN_SPI_RTIM     (0x20)
/** @} */

/** @{@name TIMER module register and bit map define
 */
#define TIMER_MODULE_BASE_ADDR 0x08000000 // timer registers
#define TIMER0_CTRL1_REG       (TIMER_MODULE_BASE_ADDR + 0x00B4)
#define TIMER0_CTRL2_REG       (TIMER_MODULE_BASE_ADDR + 0x00B8)
#define TIMER1_CTRL1_REG       (TIMER_MODULE_BASE_ADDR + 0x00BC)
#define TIMER1_CTRL2_REG       (TIMER_MODULE_BASE_ADDR + 0x00C0)
#define TIMER2_CTRL1_REG       (TIMER_MODULE_BASE_ADDR + 0x00C4)
#define TIMER2_CTRL2_REG       (TIMER_MODULE_BASE_ADDR + 0x00C8)
#define TIMER3_CTRL1_REG       (TIMER_MODULE_BASE_ADDR + 0x00CC)
#define TIMER3_CTRL2_REG       (TIMER_MODULE_BASE_ADDR + 0x00D0)
#define TIMER4_CTRL1_REG       (TIMER_MODULE_BASE_ADDR + 0x00D4)
#define TIMER4_CTRL2_REG       (TIMER_MODULE_BASE_ADDR + 0x00D8)
#define TIMER5_CTRL1_REG       (TIMER_MODULE_BASE_ADDR + 0x00F4)
#define TIMER5_CTRL2_REG       (TIMER_MODULE_BASE_ADDR + 0x00F8)
#define TIMER6_CTRL1_REG       (TIMER_MODULE_BASE_ADDR + 0x00FC)
#define TIMER6_CTRL2_REG       (TIMER_MODULE_BASE_ADDR + 0x0100)
#define TIMER7_CTRL1_REG       (TIMER_MODULE_BASE_ADDR + 0x0104)
#define TIMER7_CTRL2_REG       (TIMER_MODULE_BASE_ADDR + 0x0108)
#define TIMER8_CTRL1_REG       (TIMER_MODULE_BASE_ADDR + 0x010C)
#define TIMER8_CTRL2_REG       (TIMER_MODULE_BASE_ADDR + 0x0110)
#define TIMER9_CTRL1_REG       (TIMER_MODULE_BASE_ADDR + 0x0114)
#define TIMER9_CTRL2_REG       (TIMER_MODULE_BASE_ADDR + 0x0118)

/** @} */

/** @{@name GPIO module register and bit map define, cdh check all
 * gpio direction
 * cdh:check ok
 */
#define GPIO_MODULE_BASE_ADDR 0x20170000
#define GPIO_DIR_REG1         (GPIO_MODULE_BASE_ADDR + 0x00)
#define GPIO_DIR_REG2         (GPIO_MODULE_BASE_ADDR + 0x04)
#define GPIO_DIR_REG3         (GPIO_MODULE_BASE_ADDR + 0x08)

/*
 * gpio output control
 * cdh:check ok
 */
#define GPIO_OUT_REG1 (GPIO_MODULE_BASE_ADDR + 0x0C)
#define GPIO_OUT_REG2 (GPIO_MODULE_BASE_ADDR + 0x10)
#define GPIO_OUT_REG3 (GPIO_MODULE_BASE_ADDR + 0x14)

/*
 * gpio input control
 * cdh:check ok
 */
#define GPIO_IN_REG1 (GPIO_MODULE_BASE_ADDR + 0x18)
#define GPIO_IN_REG2 (GPIO_MODULE_BASE_ADDR + 0x1c)
#define GPIO_IN_REG3 (GPIO_MODULE_BASE_ADDR + 0x20)

/*
 * gpio interrupt enable/disable
 * cdh:check ok
 */
#define GPIO_INT_EN1 (GPIO_MODULE_BASE_ADDR + 0x24)
#define GPIO_INT_EN2 (GPIO_MODULE_BASE_ADDR + 0x28)
#define GPIO_INT_EN3 (GPIO_MODULE_BASE_ADDR + 0x2C)

/** @{@name Register Operation Define
 *      Define the macro for read/write register and memory
 */
/* ------ Macro definition for reading/writing data from/to register ------ */
#define HAL_READ_UINT32(_register_, _value_)                                   \
    ((_value_) = *((volatile unsigned long*)(_register_)))
#define HAL_WRITE_UINT32(_register_, _value_)                                  \
    (*((volatile unsigned long*)(_register_)) = (_value_))

#define REG32(_register_) (*(volatile unsigned long*)(_register_))
#define REG16(_register_) (*(volatile unsigned short*)(_register_))
#define REG8(_register_)  (*(volatile unsigned char*)(_register_))

// read and write register
#define outb(v, p) (*(volatile unsigned char*)(p) = (v))
#define outw(v, p) (*(volatile unsigned short*)(p) = (v))
#define outl(v, p) (*(volatile unsigned long*)(p) = (v))

#define inb(p)         (*(volatile unsigned char*)(p))
#define inw(p)         (*(volatile unsigned short*)(p))
#define inl(p)         (*(volatile unsigned long*)(p))
#define WriteBuf(v, p) (*(volatile unsigned long*)(p) = (v))
#define ReadBuf(p)     (*(volatile unsigned long*)(p))

#define WriteRamb(v, p) (*(volatile unsigned char*)(p) = (v))
#define WriteRamw(v, p) (*(volatile unsigned short*)(p) = (v))
#define WriteRaml(v, p) (*(volatile unsigned long*)(p) = (v))

#define ReadRamb(p) (*(volatile unsigned char*)(p))
#define ReadRamw(p) (*(volatile unsigned short*)(p))
#define ReadRaml(p) (*(volatile unsigned long*)(p))
/*@}*/

#endif // _ANYKA_CPU_H_
