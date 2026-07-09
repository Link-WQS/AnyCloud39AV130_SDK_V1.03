/** @file
 * @brief Define the register 
 *
 * Define the register address and bit map for system.
 * Copyright (C) 2021 Anyka (GuangZhou) Micro-Electronic Technology Co., Ltd.
 * @author Zou Tianxiang make reference to H322
 * @date 2024-5-11
 * @version 1.0
 * @note
  */
#ifndef _AK_REG_
#define _AK_REG_


//**Memory assignment*/
#define CHIP_CONF_BASE_ADDR             0x08000000      //chip configurations
#define L2_BUF_MEM_BASE_ADDR            0x48000000      //L2 Buffer0 start address



//CPU working mode
#define ANYKA_CPU_Mode_USR              0x10
#define ANYKA_CPU_Mode_FIQ              0x11
#define ANYKA_CPU_Mode_IRQ              0x12
#define ANYKA_CPU_Mode_SVC              0x13
#define ANYKA_CPU_Mode_ABT              0x17
#define ANYKA_CPU_Mode_UNDEF            0x1B
#define ANYKA_CPU_Mode_SYS              0x1F
#define ANYKA_CPU_I_Bit                 0x80
#define ANYKA_CPU_F_Bit                 0x40



//TOP CONFIG REG INCLUDE ALL 08000000 BASE ADDRESS
#define CHIP_ID_REG                         (CHIP_CONF_BASE_ADDR + 0x00000000)      // CHIP ID
#define CPU_PLL_CFG_REG                     (CHIP_CONF_BASE_ADDR + 0x00000004)      // CPU PLL CFG
#define CORE_PLL_CFG_REG                    (CHIP_CONF_BASE_ADDR + 0x00000008)      // CORE PLL CFG
#define SAR_ADC_CLK_REG                     (CHIP_CONF_BASE_ADDR + 0x0000000C)      // SAR_ADC_CLK CFG
#define MIPI_DPHY_CLK_REG                   (CHIP_CONF_BASE_ADDR + 0x0000000C)      // DPHY CLK CFG
#define SFC_PHYCLK_CFG_REG                  (CHIP_CONF_BASE_ADDR + 0x0000000C)      // HIGH SPEED SPI CLK & GATE REGISTER
#define AUDIO_DAC_CLK_CFG_REG               (CHIP_CONF_BASE_ADDR + 0x0000000C)      // // AUDIO DAC CLK CFG REG
#define AD_DA_HCLK_REG                      (CHIP_CONF_BASE_ADDR + 0x00000010)      // AD DA HS CLK CFG 
#define AUDIO_PLL_CFG_REG                   (CHIP_CONF_BASE_ADDR + 0x00000014)     // AUDIO PLL CFG REG
#define MAC_AND_CSI_TOP_CFG0_REG            (CHIP_CONF_BASE_ADDR + 0x00000014)      // MAC AND CIS TOP CFG1 REG
#define MAC_AND_CSI_TOP_CFG1_REG            (CHIP_CONF_BASE_ADDR + 0x00000018)      // MAC AND CIS TOP CFG2 REG


#define CLOCK_GATE_REG                      (CHIP_CONF_BASE_ADDR + 0x0000001C)      // module clock control(switch)
#define RESET_CTRL_REG                      (CHIP_CONF_BASE_ADDR + 0x00000020)      // module software reset control register
#define SENSOR_CLK_CFG_REG                  (CHIP_CONF_BASE_ADDR + 0x00000024)      // sensor clock config
#define PDM_CLK_RST_CFG_REG                 (CHIP_CONF_BASE_ADDR + 0x00000028)      // PDM clock & reset config
#define SYSINT_MASK_REG                     (CHIP_CONF_BASE_ADDR + 0x0000002C)      // module SYS interrupt status mask register, 0: mask; 1:unmask(default);
#define SYSINT_STATUS_REG                   (CHIP_CONF_BASE_ADDR + 0x00000030)      // module SYS interrupt status


#define ANA_TRIG_WAKEUP_CFG_REG             (CHIP_CONF_BASE_ADDR + 0x00000034)      // ANA TRIG CFG
#define ANA_TRIG_WAKEUP_STATUS_REG          (CHIP_CONF_BASE_ADDR + 0x00000038)      // ANA TRIG STATUS CFG
#define GPIO_WAKEUP_POLAR_REG               (CHIP_CONF_BASE_ADDR + 0x0000003C)      // GPIO POLAR WAKEUP CFG
#define GPIO_WAKEUP_CLR_REG                 (CHIP_CONF_BASE_ADDR + 0x00000040)      // GPIO CLEAR WAKEUP STATUS
#define GPIO_WAKEUP_ENA_REG                 (CHIP_CONF_BASE_ADDR + 0x00000044)      // GPIO WAKEUP ENABLE CFG
#define GPIO_WAKEUP_STATUS_REG              (CHIP_CONF_BASE_ADDR + 0x00000048)      // GPIO WAKEUP STATUS


#define RTC_CFG_REG                         (CHIP_CONF_BASE_ADDR + 0x00000050)      // RTC CFG register
#define RTC_RECE_DAT_REG                    (CHIP_CONF_BASE_ADDR + 0x00000054)      // RTC RECE DATA register
#define USB_PHY_CFG_REG                     (CHIP_CONF_BASE_ADDR + 0x00000058)     // USB PHY CFG register
#define I2S_CFG_REG                         (CHIP_CONF_BASE_ADDR + 0x00000058)     // I2S  CFG register
#define SAR_IF_CFG_REG                      (CHIP_CONF_BASE_ADDR + 0x0000005C)     // SAR ADC INTERFACE
#define SAR_TIMING_CFG_REG                  (CHIP_CONF_BASE_ADDR + 0x00000060)     // SAR ADC INTERFACE TIMING CFG
#define SAR_THRESHOLD_REG                   (CHIP_CONF_BASE_ADDR + 0x00000064)     // SAR ADC INTERFACE THRESHOLD 
#define SAR_IF_SMP_DAT_REG                  (CHIP_CONF_BASE_ADDR + 0x00000068)     // SAR ADC INTERFACE  SAMPLE DAT
#define SAR_IF_INT_STATUS_REG               (CHIP_CONF_BASE_ADDR + 0x0000006C)     // SAR ADC INTERFACE  INT STATUS
#define DAC_CFG_REG                         (CHIP_CONF_BASE_ADDR + 0x00000070)     // DAC  CONFIG


#define USB_PHY_XCFGI0_REG                  (CHIP_CONF_BASE_ADDR + 0x00000074)     // USB XCFGI0 PHY CONFIG
#define USB_PHY_XCFGI1_REG                  (CHIP_CONF_BASE_ADDR + 0x00000078)     // USB XCFGI1 PHY CONFIG
#define USB_PHY_XCFGI2_REG                  (CHIP_CONF_BASE_ADDR + 0x0000007C)     // USB XCFGI2 PHY CONFIG
#define USB_PHY_XCFGI3_REG                  (CHIP_CONF_BASE_ADDR + 0x00000080)     // USB XCFGI3 PHY CONFIG
#define USB_PHY_XCFGI4_REG                  (CHIP_CONF_BASE_ADDR + 0x00000084)     // USB XCFGI4 PHY CONFIG
#define USB_PHY_XCFGI5_REG                  (CHIP_CONF_BASE_ADDR + 0x00000088)     // USB XCFGI5 PHY CONFIG


#define EFUSE_CFG_REG                       (CHIP_CONF_BASE_ADDR + 0x0000008C)     // EFUSE CFG
#define EFUSE_RDATA0_REG                    (CHIP_CONF_BASE_ADDR + 0x00000090)     // EFUSE READ DATA 0
#define EFUSE_RDATA1_REG                    (CHIP_CONF_BASE_ADDR + 0x00000094)     // EFUSE READ DATA 1
#define EFUSE_RDATA2_REG                    (CHIP_CONF_BASE_ADDR + 0x00000098)     // EFUSE READ DATA 2


#define AUDIO_CODEC_CFG0_REG                (CHIP_CONF_BASE_ADDR + 0x0000009C)     // AUDIO CODEC CFG0
#define AUDIO_CODEC_CFG1_REG                (CHIP_CONF_BASE_ADDR + 0x000000A0)     // AUDIO CODEC CFG1
#define AUDIO_CODEC_CFG2_REG                (CHIP_CONF_BASE_ADDR + 0x000000A4)     // AUDIO CODEC CFG2


#define CRC_DATA_REG                        (CHIP_CONF_BASE_ADDR + 0x000000A8)     // CRC DAT OUT
#define CRC_CFG1_REG                        (CHIP_CONF_BASE_ADDR + 0x000000AC)     // CRC CFG1
#define CRC_CFG2_REG                        (CHIP_CONF_BASE_ADDR + 0x000000B0)     // CRC CFG2


#define EFUSE_TEMP_TRIMMING_REG             (CHIP_CONF_BASE_ADDR + 0x000000DC)     // efuse tempreture  triming bit reg
#define CPU_PLL_BWADJ_CFG_REG               (CHIP_CONF_BASE_ADDR + 0x000000DC)     // cpu pll BWADJ CFG[11:0]
#define CORE_AND_AUDIO_PLL_BWADJ_REG        (CHIP_CONF_BASE_ADDR + 0x000000E0)     // core & audio pll BWADJ CFG[11:0]


#define WATCHDOG_CFG0_REG                   (CHIP_CONF_BASE_ADDR + 0x000000E4)     // WATCHDOG CFG1
#define WATCHDOG_CFG1_REG                   (CHIP_CONF_BASE_ADDR + 0x000000E8)     // WATCHDOG CFG2

#define UART_IP_SELECT_REG                  (CHIP_CONF_BASE_ADDR + 0x000000EC)     // UART IP SELECT CFG

#define ISP_CNN_VIDEO_PP_SRC_CLK_CFG_REG    (CHIP_CONF_BASE_ADDR + 0x0000011C)     // ISP_CNN_VIDEO SRC CLK CFG



#define SHARE_PIN_CFG0_REG                  (CHIP_CONF_BASE_ADDR + 0x00000178)     // SHARE PIN CFG0
#define SHARE_PIN_CFG1_REG                  (CHIP_CONF_BASE_ADDR + 0x0000017C)     // SHARE PIN CFG1
#define SHARE_PIN_CFG2_REG                  (CHIP_CONF_BASE_ADDR + 0x00000180)     // SHARE PIN CFG2
#define SHARE_PIN_CFG3_REG                  (CHIP_CONF_BASE_ADDR + 0x00000184)     // SHARE PIN CFG3
#define SHARE_PIN_CFG4_REG                  (CHIP_CONF_BASE_ADDR + 0x00000188)     // SHARE PIN CFG4
#define SHARE_PIN_CFG5_REG                  (CHIP_CONF_BASE_ADDR + 0x0000018C)     // SHARE PIN CFG5
#define SHARE_PIN_CFG6_REG                  (CHIP_CONF_BASE_ADDR + 0x00000190)     // SHARE PIN CFG6
#define SHARE_PIN_CFG7_REG                  (CHIP_CONF_BASE_ADDR + 0x00000194)     // SHARE PIN CFG7
#define SHARE_PIN_CFG8_REG                  (CHIP_CONF_BASE_ADDR + 0x00000198)     // SHARE PIN CFG8
#define SHARE_PIN_CFG9_REG                  (CHIP_CONF_BASE_ADDR + 0x0000019C)     // SHARE PIN CFG9
#define SHARE_PIN_CFG10_REG                 (CHIP_CONF_BASE_ADDR + 0x000001A0)     // SHARE PIN CFG10
#define DVP_INPUT_DELAY_CFG_REG             (CHIP_CONF_BASE_ADDR + 0x000001A0)     // DVP_INPUT_DELAY_CFG_REG




#define PAD_DRIVE_LEVEL_CFG0_REG            (CHIP_CONF_BASE_ADDR + 0x000001A4)     //  PAD DRIVE LEVEL CFG REG0
#define PAD_DRIVE_LEVEL_CFG1_REG            (CHIP_CONF_BASE_ADDR + 0x000001A8)     //  PAD DRIVE LEVEL CFG REG1
#define PAD_DRIVE_LEVEL_CFG2_REG            (CHIP_CONF_BASE_ADDR + 0x000001AC)     //  PAD DRIVE LEVEL CFG REG2
#define PAD_DRIVE_LEVEL_CFG3_REG            (CHIP_CONF_BASE_ADDR + 0x000001B0)     //  PAD DRIVE LEVEL CFG REG3
#define PAD_DRIVE_LEVEL_CFG4_REG            (CHIP_CONF_BASE_ADDR + 0x000001B4)     //  PAD DRIVE LEVEL CFG REG4
#define PAD_DRIVE_LEVEL_CFG5_REG            (CHIP_CONF_BASE_ADDR + 0x000001B8)     //  PAD DRIVE LEVEL CFG REG5
#define PAD_DRIVE_LEVEL_CFG6_REG            (CHIP_CONF_BASE_ADDR + 0x000001BC)     //  PAD DRIVE LEVEL CFG REG6


#define PAD_IE_CFG0_REG                     (CHIP_CONF_BASE_ADDR + 0x000001C0)     // PAD IE CFG0 REGISTER
#define PAD_IE_CFG1_REG                     (CHIP_CONF_BASE_ADDR + 0x000001C4)     // PAD IE CFG1 REGISTER
#define PAD_IE_CFG2_REG                     (CHIP_CONF_BASE_ADDR + 0x000001C8)     // PAD IE CFG2 REGISTER
#define PAD_IE_CFG3_REG                     (CHIP_CONF_BASE_ADDR + 0x000001CC)     // PAD IE CFG3 REGISTER


#define PAD_SLEW_RATE_CFG0_REG              (CHIP_CONF_BASE_ADDR + 0x000001D0)     // PAD SLEW RATE CFG0
#define PAD_SLEW_RATE_CFG1_REG              (CHIP_CONF_BASE_ADDR + 0x000001D4)     // PAD SLEW RATE CFG0
#define PAD_SLEW_RATE_CFG2_REG              (CHIP_CONF_BASE_ADDR + 0x000001D8)     // PAD SLEW RATE CFG0
#define PAD_SLEW_RATE_CFG3_REG              (CHIP_CONF_BASE_ADDR + 0x000001DC)     // PAD SLEW RATE CFG0


#define TSENSOR_CFG0_REG                    (CHIP_CONF_BASE_ADDR + 0x000001E0)     // TSENSOR CFG0 REG
#define TSENSOR_CFG1_REG                    (CHIP_CONF_BASE_ADDR + 0x000001E4)     // TSENSOR CFG1 REG
#define TSENSOR_CFG2_REG                    (CHIP_CONF_BASE_ADDR + 0x000001E8)     // TSENSOR CFG2 REG
#define TSENSOR_CFG3_REG                    (CHIP_CONF_BASE_ADDR + 0x000001EC)     // TSENSOR CFG3 REG
#define TSENSOR_STATUS_REG                  (CHIP_CONF_BASE_ADDR + 0x000001F0)     // TSENSOR STATUS REG
#define TSENSOR_DUTY0_CYCLE_REG             (CHIP_CONF_BASE_ADDR + 0x000001F4)
#define TSENSOR_DUTY1_CYCLE_REG             (CHIP_CONF_BASE_ADDR + 0x000001F8)
#define TSENSOR_DUTY2_CYCLE_REG             (CHIP_CONF_BASE_ADDR + 0x000001FC)
#define TSENSOR_DUTY3_CYCLE_REG             (CHIP_CONF_BASE_ADDR + 0x00000200)
#define TSENSOR_DUTY4_CYCLE_REG             (CHIP_CONF_BASE_ADDR + 0x00000204)
#define TSENSOR_DUTY5_CYCLE_REG             (CHIP_CONF_BASE_ADDR + 0x00000208)
#define TSENSOR_DUTY6_CYCLE_REG             (CHIP_CONF_BASE_ADDR + 0x0000020C)
#define TSENSOR_DUTY7_CYCLE_REG             (CHIP_CONF_BASE_ADDR + 0x00000210)


#define GPIO_WAKEUP_POLAR_1_REG             (CHIP_CONF_BASE_ADDR + 0x0000023C)     // GPIO_WAKEUP_POLAR_1_REG
#define GPIO_WAKEUP_ENA_1_REG               (CHIP_CONF_BASE_ADDR + 0x00000240)     // GPIO_WAKEUP_CLR_1_REG
#define GPIO_WAKEUP_CLR_1_REG               (CHIP_CONF_BASE_ADDR + 0x00000244)     // GPIO_WAKEUP_ENA_1_REG
#define GPIO_WAKEUP_STATUS_1_REG            (CHIP_CONF_BASE_ADDR + 0x00000248)     // GPIO_WAKEUP_STATUS_1_REG


#define GPIO_PUPD_ENA0_REG                  (CHIP_CONF_BASE_ADDR + 0x00000264)     // GPIO PULL UP DOWN ENA0
#define GPIO_PUPD_ENA1_REG                  (CHIP_CONF_BASE_ADDR + 0x00000268)     // GPIO PULL UP DOWN ENA1
#define GPIO_PUPD_ENA2_REG                  (CHIP_CONF_BASE_ADDR + 0x0000026C)     // GPIO PULL UP DOWN ENA2
#define GPIO_PUPD_ENA3_REG                  (CHIP_CONF_BASE_ADDR + 0x00000270)     // GPIO PULL UP DOWN ENA3


#define GPIO_PUPD_SEL0_REG                  (CHIP_CONF_BASE_ADDR + 0x00000274)     // GPIO PULL UP DOWN SELECT0
#define GPIO_PUPD_SEL1_REG                  (CHIP_CONF_BASE_ADDR + 0x00000278)     // GPIO PULL UP DOWN SELECT1
#define GPIO_PUPD_SEL2_REG                  (CHIP_CONF_BASE_ADDR + 0x0000027C)     // GPIO PULL UP DOWN SELECT2
#define GPIO_PUPD_SEL3_REG                  (CHIP_CONF_BASE_ADDR + 0x00000280)     // GPIO PULL UP DOWN SELECT3


#define GPIO_PAD_ST0_REG                    (CHIP_CONF_BASE_ADDR + 0x00000284)     // GPIO_PAD_ST0_REG
#define GPIO_PAD_ST1_REG                    (CHIP_CONF_BASE_ADDR + 0x00000288)     // GPIO_PAD_ST1_REG
#define GPIO_PAD_ST2_REG                    (CHIP_CONF_BASE_ADDR + 0x0000028C)     // GPIO_PAD_ST2_REG
#define GPIO_PAD_ST3_REG                    (CHIP_CONF_BASE_ADDR + 0x00000290)     // GPIO_PAD_ST3_REG


#define PROCESS_MONITOR_CFG_REG             (CHIP_CONF_BASE_ADDR + 0x000002A0)     // PROCESS_MONITOR_CFG_REG
#define PROCESS_MONITOR_DATA_REG            (CHIP_CONF_BASE_ADDR + 0x000002A4)     // PROCESS_MONITOR_DATA_REG


#define MIPI_DPHY_CFG0_REG                  (CHIP_CONF_BASE_ADDR + 0x000002A8)
#define MIPI_DPHY_CFG1_REG                  (CHIP_CONF_BASE_ADDR + 0x000002AC)
#define MIPI_DPHY_CFG2_REG                  (CHIP_CONF_BASE_ADDR + 0x000002B0)
#define MIPI_DPHY_CFG3_REG                  (CHIP_CONF_BASE_ADDR + 0x000002B4)
#define MIPI_DPHY_CFG4_REG                  (CHIP_CONF_BASE_ADDR + 0x000002B8)
#define MIPI_DPHY_CFG5_REG                  (CHIP_CONF_BASE_ADDR + 0x000002BC)
#define MIPI_DPHY_CFG6_REG                  (CHIP_CONF_BASE_ADDR + 0x000002C0)
#define MIPI_DPHY_CFG7_REG                  (CHIP_CONF_BASE_ADDR + 0x000002C4)
#define MIPI_DPHY_CFG8_REG                  (CHIP_CONF_BASE_ADDR + 0x000002C8)
#define MIPI_DPHY_CFG9_REG                  (CHIP_CONF_BASE_ADDR + 0x000002CC)
#define MIPI_DPHY_CFG10_REG                 (CHIP_CONF_BASE_ADDR + 0x000002D0)
#define MIPI_DPHY_CFG11_REG                 (CHIP_CONF_BASE_ADDR + 0x000002D4)
#define MIPI_DPHY_CFG12_REG                 (CHIP_CONF_BASE_ADDR + 0x000002D8)
#define MIPI_DPHY_CFG13_REG                 (CHIP_CONF_BASE_ADDR + 0x000002DC)
#define MIPI_DPHY_CFG14_REG                 (CHIP_CONF_BASE_ADDR + 0x000002E0)
#define MIPI_DPHY_CFG15_REG                 (CHIP_CONF_BASE_ADDR + 0x000002E4)
#define MIPI_DPHY_CFG16_REG                 (CHIP_CONF_BASE_ADDR + 0x000002E8)
#define MIPI_DPHY_CFG17_REG                 (CHIP_CONF_BASE_ADDR + 0x000002EC)
#define MIPI_DPHY_CFG18_REG                 (CHIP_CONF_BASE_ADDR + 0x000002F0)
#define MIPI_DPHY_CFG19_REG                 (CHIP_CONF_BASE_ADDR + 0x000002F4)
#define MIPI_DPHY_CFG20_REG                 (CHIP_CONF_BASE_ADDR + 0x000002F8)
#define MIPI_DPHY_RET0_REG                  (CHIP_CONF_BASE_ADDR + 0x000002FC)
#define MIPI_DPHY_RET1_REG                  (CHIP_CONF_BASE_ADDR + 0x00000300)
#define MIPI_DPHY_RET2_REG                  (CHIP_CONF_BASE_ADDR + 0x00000304)
#define MIPI_DPHY_RET3_REG                  (CHIP_CONF_BASE_ADDR + 0x00000308)
#define MIPI_DPHY_RET4_REG                  (CHIP_CONF_BASE_ADDR + 0x0000030C)

//x = 0 ~ 4
#define PMW_CFG_HIGH_REG(x)                 (CHIP_CONF_BASE_ADDR + 0x00000310 + x*32)
#define PMW_CFG_LOW_REG(x)                  (CHIP_CONF_BASE_ADDR + 0x00000314 + x*32)
#define PMW_CFG_CTRL_REG(x)                 (CHIP_CONF_BASE_ADDR + 0x00000318 + x*32)
#define PMW_CFG_NUM_REG(x)                  (CHIP_CONF_BASE_ADDR + 0x0000031C + x*32)
#define PMW_RET_LEVEL_REG(x)                (CHIP_CONF_BASE_ADDR + 0x00000320 + x*32)
#define PMW_RET_INT_STA_REG(x)              (CHIP_CONF_BASE_ADDR + 0x00000324 + x*32)
#define PMW_CFG_INT_EN_REG(x)               (CHIP_CONF_BASE_ADDR + 0x00000328 + x*32)
#define PMW_CFG_INTCLR_REG(x)               (CHIP_CONF_BASE_ADDR + 0x0000032C + x*32)


#define TRNG_HASH_CLOCK_GATE_REG            (CHIP_CONF_BASE_ADDR + 0x000003B0)
#define TRNG_HASH_RESET_CTRL_REG            (CHIP_CONF_BASE_ADDR + 0x000003B4)





//Interrupt Register
#define INT_BASE_ADDR                       0x21100000      //Interrupt base address
#define FIQINT_MASK_REG                     (INT_BASE_ADDR + 0x00000000)            // module IRQ interrupt mask register, 0: mask; 1:unmask(default);
#define IRQINT_MASK_REG                     (INT_BASE_ADDR + 0x00000004)            // module FRQ interrupt mask register, 0: mask; 1:unmask(default);
#define INT_STATUS_REG                      (INT_BASE_ADDR + 0x00000008)            // module interrupt status register
#define LEVEL2_INT_I2C_ENA_REG              (INT_BASE_ADDR + 0x0000000C)            // level2  I2C initerrupt enable
#define LEVEL2_INT_I2C_STA_REG              (INT_BASE_ADDR + 0x00000010)            // level2  I2C initerrupt status
#define LEVEL2_INT_SECURE_ENA_REG           (INT_BASE_ADDR + 0x00000014)            // level2  Secure  engine initerrupt enable
#define LEVEL2_INT_SECURE_STA_REG           (INT_BASE_ADDR + 0x00000014)            // level2  Secure  engine initerrupt status



#if 0

//�ж�״̬λ   
//��Ӧ 0x21100000, 0x21100004   FIQ  IRQ �ж�״̬�Ĵ���
#define INT_STATUS_MEM_BIT                      (1 << 0)
#define INT_STATUS_ISP_BIT                      (1 << 1)
#define INT_STATUS_VEDIO_ENCODE_BIT             (1 << 2)
#define INT_STATUS_SYSTEM_BIT                   (1 << 3)
#define INT_STATUS_MMCSD0_BIT                   (1 << 4)
#define INT_STATUS_MMCSD1_BIT                   (1 << 5)
#define INT_STATUS_DAC_BIT                      (1 << 6)
#define INT_STATUS_ADC_BIT                      (1 << 7)
#define INT_STATUS_SFC_BIT                      (1 << 8)
#define INT_STATUS_SPI0_BIT                     (1 << 9)
#define INT_STATUS_UART0_BIT                    (1 << 10)
#define INT_STATUS_UART1_BIT                    (1 << 11)
#define INT_STATUS_L2_BUF0_BIT                  (1 << 12)
#define INT_STATUS_IIC_MUX_BIT                  (1 << 13)
#define INT_STATUS_SPI1_BIT                     (1 << 14)
#define INT_STATUS_GPIO_BIT                     (1 << 15)
#define INT_STATUS_MAC0_BIT                     (1 << 16)
#define INT_STATUS_ENCRYPT_HASH_TRNG_BIT        (1 << 17)
#define INT_STATUS_USB_BIT                      (1 << 18)
#define INT_STATUS_USBDMA_BIT                   (1 << 19)
#define INT_STATUS_MEMCPY_BIT                   (1 << 20)
#define INT_STATUS_ENCODER_BRIDGE_BIT           (1 << 21)
#define INT_STATUS_MMCSD2_BIT                   (1 << 22)
#define INT_STATUS_PDM_BIT                      (1 << 23)
#define INT_STATUS_UART2_BIT                    (1 << 24)
#define INT_STATUS_FFT_BIT                      (1 << 25) 
#define INT_STATUS_CNN_BIT                      (1 << 26)
#define INT_STATUS_TIMER0_BIT                   (1 << 27)
#define INT_STATUS_TIMER1_BIT                   (1 << 28)
#define INT_STATUS_TIMER2_BIT                   (1 << 29)
#define INT_STATUS_TIMER3_BIT                   (1 << 30)
#define INT_STATUS_TIMER4_BIT                   (1 << 31)



//level 2 interrupt status bit map
#define INT_STATUS_SARADC_BIT                   (1 << 0)
#define INT_STATUS_PWM4_BIT                     (1 << 1)
#define INT_STATUS_PWM3_BIT                     (1 << 2)
#define INT_STATUS_PWM2_BIT                     (1 << 3)
#define INT_STATUS_PWM1_BIT                     (1 << 4)
#define INT_STATUS_PWM0_BIT                     (1 << 5)
#define INT_STATUS_WAKEUP_BIT                   (1 << 6)
#define INT_STATUS_RTC_RDY_BIT                  (1 << 7)
#define INT_STATUS_RTC_ALARM_BIT                (1 << 8)
#define INT_STATUS_RTC_TIMER_BIT                (1 << 9)
#define INT_STATUS_RTC_WATCHDOG_BIT             (1 << 10)
#define INT_STATUS_TESENSOR_BIT                 (1 << 16)




//�ж�ʹ�ܼĴ���
//��Ӧ 0x21100000, 0x21100004   FIQ  IRQ �ж�ʹ������
#define INT_UNMASK_MEM_BIT                      (1 << 0)
#define INT_UNMASK_ISP_BIT                      (1 << 1)
#define INT_UNMASK_VEDIO_ENCODE_BIT             (1 << 2)
#define INT_UNMASK_SYSTEM_BIT                   (1 << 3)
#define INT_UNMASK_MMCSD0_BIT                   (1 << 4)
#define INT_UNMASK_MMCSD1_BIT                   (1 << 5)
#define INT_UNMASK_DAC_BIT                      (1 << 6)
#define INT_UNMASK_ADC_BIT                      (1 << 7)
#define INT_UNMASK_SFC_BIT                      (1 << 8)
#define INT_UNMASK_SPI0_BIT                     (1 << 9)
#define INT_UNMASK_UART0_BIT                    (1 << 10)
#define INT_UNMASK_UART1_BIT                    (1 << 11)
#define INT_UNMASK_L2_BUF0_BIT                  (1 << 12)
#define INT_UNMASK_IIC_MUX_BIT                  (1 << 13)
#define INT_UNMASK_SPI1_BIT                     (1 << 14)
#define INT_UNMASK_GPIO_BIT                     (1 << 15)
#define INT_UNMASK_MAC0_BIT                     (1 << 16)
#define INT_UNMASK_ENCRYPT_HASH_TRNG_BIT        (1 << 17)
#define INT_UNMASK_USB_BIT                      (1 << 18)
#define INT_UNMASK_USBDMA_BIT                   (1 << 19)
#define INT_UNMASK_MEMCPY_BIT                   (1 << 20)
#define INT_UNMASK_ENCODER_BRIDGE_BIT           (1 << 21)
#define INT_UNMASK_MMCSD2_BIT                   (1 << 22)
#define INT_UNMASK_PDM_BIT                      (1 << 23)
#define INT_UNMASK_UART2_BIT                    (1 << 24)
#define INT_UNMASK_FFT_BIT                      (1 << 25)
#define INT_UNMASK_CNN_BIT                      (1 << 26)
#define INT_UNMASK_TIMER0_BIT                   (1 << 27)
#define INT_UNMASK_TIMER1_BIT                   (1 << 28)
#define INT_UNMASK_TIMER2_BIT                   (1 << 29)
#define INT_UNMASK_TIMER3_BIT                   (1 << 30)
#define INT_UNMASK_TIMER4_BIT                   (1 << 31)


//level 2 interrupt mask bit map
#define INT_UNMASK_SARADC_BIT                   (1 << 0)
#define INT_UNMASK_PWM4_BIT                     (1 << 1)
#define INT_UNMASK_PWM3_BIT                     (1 << 2)
#define INT_UNMASK_PWM2_BIT                     (1 << 3)
#define INT_UNMASK_PWM1_BIT                     (1 << 4)
#define INT_UNMASK_PWM0_BIT                     (1 << 5)
#define INT_UNMASK_WAKEUP_BIT                   (1 << 6)
#define INT_UNMASK_RTC_RDY_BIT                  (1 << 7)
#define INT_UNMASK_RTC_ALARM_BIT                (1 << 8)
#define INT_UNMASK_RTC_TIMER_BIT                (1 << 9)
#define INT_UNMASK_RTC_WATCHDOG_BIT             (1 << 10)
#define INT_UNMASK_TESENSOR_BIT                 (1 << 16)
/** @} */
#endif


#endif

