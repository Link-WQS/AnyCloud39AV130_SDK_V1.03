/** @file
 * @brief Define the register of ANYKA CPU
 *
 * Define the register address and bit map for system.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author dfl
 * @date 2022-03-01
 * @version 1.0
 * @note
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

#define PERIL_PLL_CHANNEL_CTRL_REG  (CHIP_CONF_BASE_ADDR + 0x00000014)
#define PERIL_PLL_CHANNEL_CTRL_REG2 (CHIP_CONF_BASE_ADDR + 0x00000018)

#define CLOCK_CTRL_REG                                                         \
    (CHIP_CONF_BASE_ADDR + 0x0000001C) // module clock control(switch)
#define RESET_CTRL_REG                                                         \
    (CHIP_CONF_BASE_ADDR + 0x00000020) // module software reset control register
#define IRQINT_MASK_REG                                                        \
    (CHIP_CONF_BASE_ADDR                                                       \
        + 0x00000024) // cdh:module IRQ interrupt mask register, 1: mask;
                      // 0:unmask(default);
#define FRQINT_MASK_REG                                                        \
    (CHIP_CONF_BASE_ADDR                                                       \
        + 0x00000028) // cdh:module FRQ interrupt mask register, 1: mask;
                      // 0:unmask(default);
#define INT_STATUS_REG                                                         \
    (CHIP_CONF_BASE_ADDR + 0x0000004C) // cdh:module interrupt status register
#define INT_SYS_MODULE_REG                                                     \
    (CHIP_CONF_BASE_ADDR                                                       \
        + 0x00000030) // cdh:system module interrupt status control register
/** @} */

#define SHARE_PIN_CFG0_REG (CHIP_CONF_BASE_ADDR + 0x00000074) // SHARE PIN CFG1
#define SHARE_PIN_CFG1_REG (CHIP_CONF_BASE_ADDR + 0x00000078) // SHARE PIN CFG2
#define SHARE_PIN_CFG2_REG (CHIP_CONF_BASE_ADDR + 0x0000007C) // SHARE PIN CFG3
#define SHARE_PIN_CFG3_REG (CHIP_CONF_BASE_ADDR + 0x000000DC) // SHARE PIN CFG4

#define PU_PD_ENABLE_CFG0_REG (CHIP_CONF_BASE_ADDR + 0x00000080)
#define PU_PD_ENABLE_CFG1_REG (CHIP_CONF_BASE_ADDR + 0x00000084)
#define PU_PD_ENABLE_CFG2_REG (CHIP_CONF_BASE_ADDR + 0x00000088)
#define PU_PD_ENABLE_CFG3_REG (CHIP_CONF_BASE_ADDR + 0x000000E0)

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
/** @} */

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
/** @} */

/** @{@name GPIO module register and bit map define, cdh check all
 * gpio direction
 */
#define GPIO_MODULE_BASE_ADDR 0x20170000
#define GPIO_DIR_REG1         (GPIO_MODULE_BASE_ADDR + 0x00)
#define GPIO_DIR_REG2         (GPIO_MODULE_BASE_ADDR + 0x04)
#define GPIO_DIR_REG3         (GPIO_MODULE_BASE_ADDR + 0x08)

/*
 * gpio output control
 */
#define GPIO_OUT_REG1 (GPIO_MODULE_BASE_ADDR + 0x0C)
#define GPIO_OUT_REG2 (GPIO_MODULE_BASE_ADDR + 0x10)
#define GPIO_OUT_REG3 (GPIO_MODULE_BASE_ADDR + 0x14)

/*
 * gpio input control
 */
#define GPIO_IN_REG1 (GPIO_MODULE_BASE_ADDR + 0x18)
#define GPIO_IN_REG2 (GPIO_MODULE_BASE_ADDR + 0x1c)
#define GPIO_IN_REG3 (GPIO_MODULE_BASE_ADDR + 0x20)
/** @} */

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
