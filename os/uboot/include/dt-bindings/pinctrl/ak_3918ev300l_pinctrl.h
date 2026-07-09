/*
 * This header provides constants for the 3918EV300L GPIO binding.
 */
#ifndef __AK_3918EV300L_PINCTRL_H__
#define __AK_3918EV300L_PINCTRL_H__

#define PULL_UP        (0x0 << 0)
#define PULL_DOWN      (0x1 << 0)
#define PUPD_EN        (0x1 << 4)
#define DRIVE_02MA     (0x0 << 8)
#define DRIVE_04MA     (0x1 << 8)
#define DRIVE_08MA     (0x2 << 8)
#define DRIVE_12MA     (0x3 << 8)
#define INPUT_EN       (0x1 << 16)
#define FAST_CTRL      (0x1 << 24)
#define SCHMITT_EN     (0x1 << 28)
#define GPIO_DIR_EN    (0x1 << 15)
#define GPIO_DIR_IN    (0x1 << 12)
#define GPIO_DIR_OUT   (0x0 << 12)
#define GPIO_OUT_EN    (0x1 << 23)
#define GPIO_OUT_LOW   (0x0 << 20)
#define GPIO_OUT_HIGH  (0x1 << 20)

#define XGPIO_000                      0
#define XGPIO_000_FUNC_GPIO0           0
#define XGPIO_000_FUNC_UART1_TXD       1
#define XGPIO_000_FUNC_SPI1_CLK        2
#define XGPIO_000_FUNC_OPCLK1          3

#define XGPIO_001                      1
#define XGPIO_001_FUNC_GPIO1           0
#define XGPIO_000_FUNC_UART1_RXD       1
#define XGPIO_001_FUNC_SPI1_DIN        2
#define XGPIO_001_FUNC_RMII_RXER       3

#define XGPIO_002                      2
#define XGPIO_002_FUNC_GPIO2           0
#define XGPIO_002_FUNC_PWM3            2
#define XGPIO_002_FUNC_SPI1_DOUT       3
#define XGPIO_002_FUNC_RMII_RXDV       4

#define XGPIO_003                      3
#define XGPIO_003_FUNC_GPIO3           0
#define XGPIO_003_FUNC_SPI1_CS         1
#define XGPIO_003_FUNC_RMII_MDIO       2

#define XGPIO_004                      4
#define XGPIO_004_FUNC_GPIO4           0
#define XGPIO_004_FUNC_TWI2_SCL        1
#define XGPIO_004_FUNC_SD1_D3          2
#define XGPIO_004_FUNC_SPI2_CS1        3
#define XGPIO_004_FUNC_RMII_TXEN       4

#define XGPIO_005                      5
#define XGPIO_005_FUNC_GPIO5           0
#define XGPIO_005_FUNC_TWI2_SDA        1
#define XGPIO_005_FUNC_SD1_D2          2
#define XGPIO_005_FUNC_RMII_MDC        3

#define XGPIO_006                      6
#define XGPIO_006_FUNC_GPIO6           0
#define XGPIO_006_FUNC_TWI0_SDA        1
#define XGPIO_006_FUNC_SD1_D1          2
#define XGPIO_006_FUNC_RMII_RXD1       3

#define XGPIO_007                      7
#define XGPIO_007_FUNC_GPIO7           0
#define XGPIO_007_FUNC_TWI0_SCL        1
#define XGPIO_007_FUNC_SD1_D0          2
#define XGPIO_007_FUNC_RMII_TXD0       3

#define XGPIO_008                      8
#define XGPIO_008_FUNC_GPIO8           0
#define XGPIO_008_FUNC_SD1_MCLK        3
#define XGPIO_008_FUNC_SPI2_CLK        4
#define XGPIO_008_FUNC_RMII_TXD1       5

#define XGPIO_009                      9
#define XGPIO_009_FUNC_GPIO9           0
#define XGPIO_009_FUNC_CSI_HSYNC       1
#define XGPIO_009_FUNC_SD1_CMD         2
#define XGPIO_009_FUNC_SPI2_DIN        3

#define XGPIO_010                      10
#define XGPIO_010_FUNC_GPIO10          0
#define XGPIO_010_FUNC_CSI_VSYNC       1
#define XGPIO_010_FUNC_SPI2_DOUT       2
#define XGPIO_010_FUNC_TWI1_SCL        3

#define XGPIO_011                      11
#define XGPIO_011_FUNC_GPIO11          0
#define XGPIO_011_FUNC_CSI0_SCLK       2
#define XGPIO_011_FUNC_PWM2            3
#define XGPIO_011_FUNC_SPI2_CS0        4
#define XGPIO_011_FUNC_RMII_RXD0       5
#define XGPIO_011_FUNC_TWI1_SDA        6

#define XGPIO_016                      16
#define XGPIO_016_FUNC_GPIO16          0
#define XGPIO_016_FUNC_SD1_MCLK        1
#define XGPIO_016_FUNC_PWM0            2
#define XGPIO_016_FUNC_UART2_RXD       3

#define XGPIO_017                      17
#define XGPIO_017_FUNC_GPIO17          0
#define XGPIO_017_FUNC_CSI_D0          1
#define XGPIO_017_FUNC_SPI1_CS         2
#define XGPIO_017_FUNC_SD1_CMD         3
#define XGPIO_017_FUNC_PWM3            4
#define XGPIO_017_FUNC_UART2_TXD       5

#define XGPIO_018                      18
#define XGPIO_018_FUNC_GPIO18          0
#define XGPIO_018_FUNC_TWI2_SCL        1
#define XGPIO_018_FUNC_CSI_D1          2
#define XGPIO_018_FUNC_SPI1_CLK        3

#define XGPIO_019                      19
#define XGPIO_019_FUNC_GPIO19          0
#define XGPIO_019_FUNC_TWI2_SDA        1
#define XGPIO_019_FUNC_CSI_D2          2
#define XGPIO_019_FUNC_SPI1_DOUT       3

#define XGPIO_020                      20
#define XGPIO_020_FUNC_GPIO20          0
#define XGPIO_020_FUNC_TWI0_SDA        1
#define XGPIO_020_FUNC_CSI_D3          2

#define XGPIO_021                      21
#define XGPIO_021_FUNC_GPIO21          0
#define XGPIO_021_FUNC_TWI0_SCL        1
#define XGPIO_021_FUNC_CSI_D5          2

#define CSI0_SCLK                      22
#define XGPIO_022_FUNC_CSI0_SCLK       1
#define XGPIO_022_FUNC_CSI_D6          2
#define XGPIO_022_FUNC_GPIO22          3

#define XGPIO_023                      23
#define XGPIO_023_FUNC_GPIO23          0
#define XGPIO_023_FUNC_CSI_D4          2

#define XGPIO_024                      24
#define XGPIO_024_FUNC_GPIO24          0
#define XGPIO_024_FUNC_SPI1_CS         1
#define XGPIO_024_FUNC_SD1_D1          2

#define XGPIO_025                      25
#define XGPIO_025_FUNC_GPIO25          0
#define XGPIO_025_FUNC_SPI1_CLK        1
#define XGPIO_025_FUNC_SD1_D0          2

#define XGPIO_026                      26
#define XGPIO_026_FUNC_GPIO26          0
#define XGPIO_026_FUNC_SPI1_DOUT       1
#define XGPIO_026_FUNC_SD1_CMD         2

#define XGPIO_027                      27
#define XGPIO_027_FUNC_GPIO27          0
#define XGPIO_027_FUNC_SPI1_DIN        1
#define XGPIO_027_FUNC_SD1_MCLK        2
#define XGPIO_027_FUNC_PWM1            3

#define XGPIO_031                      31
#define XGPIO_031_FUNC_GPIO31          0
#define XGPIO_031_FUNC_SD0_D2          1
#define XGPIO_031_FUNC_EMMC_D3         3
#define XGPIO_031_FUNC_SD1_D1          4

#define XGPIO_032                      32
#define XGPIO_032_FUNC_GPIO32          0
#define XGPIO_032_FUNC_SD0_D3          1
#define XGPIO_032_FUNC_EMMC_D0         3
#define XGPIO_032_FUNC_SD1_D0          4

#define XGPIO_033                      33
#define XGPIO_033_FUNC_GPIO33          0
#define XGPIO_033_FUNC_SD0_CMD         1
#define XGPIO_033_FUNC_EMMC_CMD        2
#define XGPIO_033_FUNC_SD1_CMD         3

#define XGPIO_034                      34
#define XGPIO_034_FUNC_GPIO34          0
#define XGPIO_034_FUNC_SD0_MCLK        1
#define XGPIO_034_FUNC_EMMC_MCLK       3
#define XGPIO_034_FUNC_SD1_MCLK        4

#define XGPIO_035                      35
#define XGPIO_035_FUNC_GPIO35          0
#define XGPIO_035_FUNC_SD0_D0          1
#define XGPIO_035_FUNC_EMMC_D1         3
#define XGPIO_035_FUNC_SD1_D3          4

#define XGPIO_036                      36
#define XGPIO_036_FUNC_GPIO36          0
#define XGPIO_036_FUNC_SD0_D1          1
#define XGPIO_036_FUNC_EMMC_D2         3
#define XGPIO_036_FUNC_SD1_D2          4

#define XGPIO_037                      37
#define XGPIO_037_FUNC_GPIO37          0

#define XGPIO_042                      42
#define XGPIO_042_FUNC_GPIO42          0
#define XGPIO_042_FUNC_EMMC_D3         1
#define XGPIO_042_FUNC_SPI0_WP         2

#define XGPIO_043                      43
#define XGPIO_043_FUNC_GPIO43          0
#define XGPIO_043_FUNC_EMMC_D0         1
#define XGPIO_043_FUNC_SPI0_DIN        2

#define XGPIO_044                      44
#define XGPIO_044_FUNC_GPIO44          0
#define XGPIO_044_FUNC_EMMC_D1         1
#define XGPIO_044_FUNC_SPI0_CS         2

#define XGPIO_045                      45
#define XGPIO_045_FUNC_GPIO45          0
#define XGPIO_045_FUNC_EMMC_D2         1
#define XGPIO_045_FUNC_SPI0_HOLD       2

#define XGPIO_046                      46
#define XGPIO_046_FUNC_GPIO46          0
#define XGPIO_046_FUNC_EMMC_MCLK       1
#define XGPIO_046_FUNC_SPI0_CLK        2

#define XGPIO_047                      47
#define XGPIO_047_FUNC_GPIO47          0
#define XGPIO_047_FUNC_EMMC_CMD        1
#define XGPIO_047_FUNC_SPI0_DOUT       2

#define XGPIO_048                      48
#define XGPIO_048_FUNC_GPIO48          0
#define XGPIO_048_FUNC_PWM0            1
#define XGPIO_048_FUNC_UART1_TXD       2
#define XGPIO_048_FUNC_TWI1_SCL        3

#define XGPIO_049                      49
#define XGPIO_049_FUNC_GPIO49          0
#define XGPIO_049_FUNC_PWM1            1
#define XGPIO_049_FUNC_UART1_RXD       2
#define XGPIO_049_FUNC_TWI1_SDA        3

#define XGPIO_050                      50
#define XGPIO_050_FUNC_GPIO50          0

#define XGPIO_051                      51
#define XGPIO_051_FUNC_GPIO51          0
#define XGPIO_051_FUNC_UART0_TXD       1

#define XGPIO_052                      52
#define XGPIO_052_FUNC_GPIO52          0
#define XGPIO_052_FUNC_UART0_RXD       1

#define XGPIO_055                      55
#define XGPIO_055_FUNC_GPIO55          0
#define XGPIO_055_FUNC_SPI2_CS0        1
#define XGPIO_055_FUNC_UART2_RXD       2
#define XGPIO_055_FUNC_PWM3            3

#define XGPIO_056                      56
#define XGPIO_056_FUNC_GPIO56          0
#define XGPIO_056_FUNC_SPI2_CLK        1
#define XGPIO_056_FUNC_UART2_TXD       2
#define XGPIO_056_FUNC_PWM4            3

#define XGPIO_058                      58
#define XGPIO_058_FUNC_GPIO58          0
#define XGPIO_058_FUNC_SPI2_CS1        1
#define XGPIO_058_FUNC_UART2_RTS       2
#define XGPIO_058_FUNC_TWI1_SDA        3
#define XGPIO_058_FUNC_SD1_CMD         4

#define XGPIO_065                      65
#define SPI2_CS0                       65
#define XGPIO_065_FUNC_SPI2_CS0        1
#define XGPIO_065_FUNC_GPIO65          2
#define XGPIO_065_FUNC_UART2_RXD       3
#define XGPIO_065_FUNC_I2S_MCLK        4
#define XGPIO_065_FUNC_SD1_MCLK        5

#define XGPIO_066                      66
#define SPI2_CLK                       66
#define XGPIO_066_FUNC_SPI2_CLK        1
#define XGPIO_066_FUNC_GPIO66          2
#define XGPIO_066_FUNC_UART2_TXD       3
#define XGPIO_066_FUNC_I2S_BCLK        4
#define XGPIO_066_FUNC_SD1_D2          5

#define XGPIO_067                      67
#define SPI2_DIN                       67
#define XGPIO_067_FUNC_SPI2_DIN        1
#define XGPIO_067_FUNC_GPIO67          2
#define XGPIO_067_FUNC_PWM2            3
#define XGPIO_067_FUNC_UART2_CTS       4
#define XGPIO_067_FUNC_I2S_LRCLK       5
#define XGPIO_067_FUNC_SD1_D3          6

#define XGPIO_068                      68
#define SPI2_CS1                       68
#define XGPIO_068_FUNC_SPI2_CS1        1
#define XGPIO_068_FUNC_GPIO68          2
#define XGPIO_068_FUNC_PWM3            4
#define XGPIO_068_FUNC_I2S_DOUT        5
#define XGPIO_068_FUNC_SD1_D0          6

#define XGPIO_069                      69
#define SPI2_DOUT                      69
#define XGPIO_069_FUNC_SPI2_DOUT       1
#define XGPIO_069_FUNC_GPIO69          2
#define XGPIO_069_FUNC_UART2_RTS       3
#define XGPIO_069_FUNC_I2S_DIN         4
#define XGPIO_069_FUNC_SD1_D1          5

#define XGPIO_070                      70
#define XGPIO_070_FUNC_GPIO70          0
#define XGPIO_070_FUNC_AIN1            1

#define XGPIO_071                      71
#define XGPIO_071_FUNC_GPIO71          0
#define XGPIO_071_FUNC_AIN0            1

#define SYS_RSTN_OUT                   98
#define XGPIO_098_FUNC_GPIO98          1

#define MIPI_RX_L0P                    99
#define XGPIO_099_FUNC_MIPI_RX_L0P     0
#define XGPIO_099_FUNC_CSI_D7          1
#define XGPIO_099_FUNC_GPIO99          2

#define MIPI_RX_L0N                    100
#define XGPIO_099_FUNC_MIPI_RX_L0N     0
#define XGPIO_099_FUNC_CSI_CLK         1
#define XGPIO_099_FUNC_GPIO100         2

#define MIPI_RX_L1P                    101
#define XGPIO_101_FUNC_MIPI_RX_L1P     0
#define XGPIO_101_FUNC_CSI_D9          1
#define XGPIO_101_FUNC_GPIO101         2

#define MIPI_RX_L1N                    102
#define XGPIO_102_FUNC_MIPI_RX_L1N     0
#define XGPIO_102_FUNC_CSI_D8          1
#define XGPIO_102_FUNC_GPIO102         2

#define MIPI_RX_L2P                    103
#define XGPIO_101_FUNC_MIPI_RX_L2P     0
#define XGPIO_101_FUNC_CSI_D10         1
#define XGPIO_101_FUNC_GPIO103         2

#define MIPI_RX_L2N                    104
#define XGPIO_102_FUNC_MIPI_RX_L2N     0
#define XGPIO_102_FUNC_CSI_D11         1
#define XGPIO_102_FUNC_GPIO104         2
#endif //__AK_3918EV300L_PINCTRL_H__
