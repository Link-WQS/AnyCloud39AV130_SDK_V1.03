/*
 * This header provides constants for the AK39EV200 GPIO binding.
 */
#ifndef __AK_39EV200_PINCTRL_H__
#define __AK_39EV200_PINCTRL_H__

#define XGPIO_000                      0
#define XGPIO_000_FUNC_GPIO0           0
#define XGPIO_000_FUNC_JTAG_RSTN       1

#define XGPIO_001                      1
#define XGPIO_001_FUNC_GPIO1           0
#define XGPIO_001_FUNC_UART0_RXD       1

#define XGPIO_002                      2
#define XGPIO_002_FUNC_GPIO2           0
#define XGPIO_002_FUNC_UART0_TXD       1

#define XGPIO_004                      4
#define XGPIO_004_FUNC_GPIO4           0
#define XGPIO_004_FUNC_PWM1 	       1

#define XGPIO_005                      5
#define XGPIO_005_FUNC_GPIO5           0
#define XGPIO_005_FUNC_PWM2 	       1
#define XGPIO_005_FUNC_JTAG_RTCK       2

#define XGPIO_006                      6
#define XGPIO_006_FUNC_CIS0_D0         0
#define XGPIO_006_FUNC_GPIO6           1
#define XGPIO_006_FUNC_UART1_RXD       2
#define XGPIO_006_FUNC_JTAG_TCLK       3

#define XGPIO_007                      7
#define XGPIO_007_FUNC_CIS0_D1         0
#define XGPIO_007_FUNC_GPIO7           1
#define XGPIO_007_FUNC_UART1_TXD       2
#define XGPIO_007_FUNC_JTAG_TMS        3

#define XGPIO_008                      8
#define XGPIO_008_FUNC_CIS0_D2         0
#define XGPIO_008_FUNC_GPIO8           1
#define XGPIO_008_FUNC_UART1_CTS       2
#define XGPIO_006_FUNC_JTAG_TDI        3

#define XGPIO_009                      9
#define XGPIO_009_FUNC_CIS0_D3         0
#define XGPIO_009_FUNC_GPIO9           1
#define XGPIO_009_FUNC_UART1_RTS       2
#define XGPIO_006_FUNC_JTAG_TDO        3

#define XGPIO_010                      10
#define XGPIO_010_FUNC_GPIO10          0
#define XGPIO_010_FUNC_MII_MDC         1
#define XGPIO_010_FUNC_I2S_MCLK        2
#define XGPIO_010_FUNC_PWM2            3

#define XGPIO_011                      11
#define XGPIO_011_FUNC_GPIO11          0
#define XGPIO_011_FUNC_MII_MDIO        1
#define XGPIO_011_FUNC_I2S_LRCLK       2
#define XGPIO_011_FUNC_SPI1_CS         3

#define XGPIO_012                      12
#define XGPIO_012_FUNC_GPIO12          0
#define XGPIO_012_FUNC_MII_TXER        1

#define XGPIO_013                      13
#define XGPIO_013_FUNC_GPIO13          0
#define XGPIO_013_FUNC_MII_TXEN        1
#define XGPIO_013_FUNC_I2S_BCLK        2
#define XGPIO_013_FUNC_SPI1_SCLK       3

#define XGPIO_014                      14
#define XGPIO_014_FUNC_GPIO14          0
#define XGPIO_014_FUNC_MII_TXD0        1
#define XGPIO_014_FUNC_I2S_DI          2
#define XGPIO_014_FUNC_SPI1_DI         3

#define XGPIO_015                      15
#define XGPIO_015_FUNC_GPIO15          0
#define XGPIO_015_FUNC_MII_TXD1        1
#define XGPIO_015_FUNC_I2S_DO          2
#define XGPIO_015_FUNC_SPI1_DO         3

#define XGPIO_016                      16
#define XGPIO_016_FUNC_GPIO16          0
#define XGPIO_016_FUNC_MII_TXD2        1

#define XGPIO_017                      17
#define XGPIO_017_FUNC_GPIO17          0
#define XGPIO_017_FUNC_MII_TXD3        1

#define XGPIO_018                      18
#define XGPIO_018_FUNC_GPIO18          0
#define XGPIO_018_FUNC_MII_CRS         1

#define XGPIO_019                      19
#define XGPIO_019_FUNC_GPIO19          0
#define XGPIO_019_FUNC_MII_RXD0        1
#define XGPIO_019_FUNC_SD1_D3          2
#define XGPIO_019_FUNC_SPI1_HOLD       3

#define XGPIO_020                      20
#define XGPIO_020_FUNC_GPIO20          0
#define XGPIO_020_FUNC_MII_RXD1        1
#define XGPIO_020_FUNC_SD1_D2          2
#define XGPIO_020_FUNC_SPI1_WP         3

#define XGPIO_021                      21
#define XGPIO_021_FUNC_GPIO21          0
#define XGPIO_021_FUNC_MII_RXD2        1

#define XGPIO_022                      22
#define XGPIO_022_FUNC_GPIO22          0
#define XGPIO_022_FUNC_MII_RXD3        1

#define XGPIO_023                      23
#define XGPIO_023_FUNC_GPIO23          0
#define XGPIO_023_FUNC_MII_RXER        1
#define XGPIO_023_FUNC_SD1_D1          2
#define XGPIO_023_FUNC_PWM3            3

#define XGPIO_024                      24
#define XGPIO_024_FUNC_GPIO24          0
#define XGPIO_024_FUNC_MII_RXDV        1
#define XGPIO_024_FUNC_SD1_D0          2
#define XGPIO_024_FUNC_PWM4            3

#define XGPIO_025                      25
#define XGPIO_025_FUNC_GPIO25          0
#define XGPIO_025_FUNC_SPI0_CS         1

#define XGPIO_026                      26
#define XGPIO_026_FUNC_GPIO26          0
#define XGPIO_026_FUNC_SPI0_SCLK       1

#define XGPIO_027                      27
#define XGPIO_027_FUNC_GPIO27          0
#define XGPIO_027_FUNC_TWI0_SCL        1

#define XGPIO_028                      28
#define XGPIO_028_FUNC_GPIO28          0
#define XGPIO_028_FUNC_TWI0_SDA        1

#define XGPIO_029                      29
#define XGPIO_029_FUNC_GPIO29          0
#define XGPIO_029_FUNC_IRDA_RX         1
#define XGPIO_029_FUNC_SPI1_SCLK       2

#define XGPIO_030                      30
#define XGPIO_030_FUNC_GPIO30          0
#define XGPIO_030_FUNC_PWM4            1
#define XGPIO_030_FUNC_SPI1_CS         2

#define XGPIO_031                      31
#define XGPIO_031_FUNC_GPIO31          0
#define XGPIO_031_FUNC_SD0_CMD         1

#define XGPIO_032                      32
#define XGPIO_032_FUNC_GPIO32          0
#define XGPIO_032_FUNC_SD0_CLK         1

#define XGPIO_033                      33
#define XGPIO_033_FUNC_GPIO33          0
#define XGPIO_033_FUNC_SD0_D0          1
#define XGPIO_033_FUNC_SPI0_HOLD       2

#define XGPIO_034                      34
#define XGPIO_034_FUNC_GPIO34          0
#define XGPIO_034_FUNC_SD0_D1          1
#define XGPIO_034_FUNC_SPI0_WP         2

#define XGPIO_035                      35
#define XGPIO_035_FUNC_GPIO35          0
#define XGPIO_035_FUNC_SD0_D2          1
#define XGPIO_035_FUNC_SPI0_DI         2

#define XGPIO_036                      36
#define XGPIO_036_FUNC_GPIO36          0
#define XGPIO_036_FUNC_SD0_D3          1
#define XGPIO_036_FUNC_SPI0_DO         2

#define XGPIO_037                      37
#define XGPIO_037_FUNC_GPIO37          0
#define XGPIO_037_FUNC_SD0_D4          1
#define XGPIO_037_FUNC_SPI0_DI         2

#define XGPIO_038                      38
#define XGPIO_038_FUNC_GPIO38          0
#define XGPIO_038_FUNC_SD0_D5          1
#define XGPIO_038_FUNC_SPI0_DO         2

#define XGPIO_039                      39
#define XGPIO_039_FUNC_GPIO39          0
#define XGPIO_039_FUNC_SD0_D6          1
#define XGPIO_039_FUNC_SPI0_WP         2

#define XGPIO_040                      40
#define XGPIO_040_FUNC_GPIO40          0
#define XGPIO_040_FUNC_SD0_D7          1
#define XGPIO_040_FUNC_SPI0_HOLD       2

#define XGPIO_041                      41
#define XGPIO_041_FUNC_GPIO41          0
#define XGPIO_041_FUNC_SD1_CMD         1

#define XGPIO_042                      42
#define XGPIO_042_FUNC_GPIO42          0
#define XGPIO_042_FUNC_SD1_MCK         1

#define XGPIO_043                      43
#define XGPIO_043_FUNC_GPIO43          0
#define XGPIO_043_FUNC_SD1_D0          1
#define XGPIO_043_FUNC_SPI1_HOLD       2

#define XGPIO_044                      44
#define XGPIO_044_FUNC_GPIO44          0
#define XGPIO_044_FUNC_SD1_D1          1
#define XGPIO_044_FUNC_SPI1_WP         2

#define XGPIO_045                      45
#define XGPIO_045_FUNC_GPIO45          0
#define XGPIO_045_FUNC_SD1_D2          1
#define XGPIO_045_FUNC_SPI1_DO         2

#define XGPIO_046                      46
#define XGPIO_046_FUNC_GPIO46          0
#define XGPIO_046_FUNC_SD1_D3          1
#define XGPIO_046_FUNC_SPI1_DI         2

#define XGPIO_047                      47
#define XGPIO_047_FUNC_GPIO47          0
#define XGPIO_047_FUNC_OPCLK           1
#define XGPIO_047_FUNC_PWM4            2

#define XGPIO_048                      48
#define XGPIO_048_FUNC_GPIO48          0
#define XGPIO_048_FUNC_PWM0            1

#define XGPIO_050                      50
#define XGPIO_050_FUNC_GPIO50          0
#define XGPIO_050_FUNC_PWM1            1

#define XGPIO_051                      51
#define XGPIO_051_FUNC_GPIO51          0
#define XGPIO_051_FUNC_PWM2            1

#define XGPIO_052                      52
#define XGPIO_052_FUNC_GPIO52          0
#define XGPIO_052_FUNC_I2S_DO          1

#define XGPIO_053                      53
#define XGPIO_053_FUNC_GPIO53          0
#define XGPIO_053_FUNC_I2S_MCLK        1

#define XGPIO_054                      54
#define XGPIO_054_FUNC_GPIO54          0
#define XGPIO_054_FUNC_I2S_BCLK        1

#define XGPIO_055                      55
#define XGPIO_055_FUNC_GPIO55          0
#define XGPIO_055_FUNC_I2S_LRCLK       1

#define XGPIO_057                      57
#define XGPIO_057_FUNC_GPIO57          0
#define XGPIO_057_FUNC_PWM3            1

#define XGPIO_058                      58
#define XGPIO_058_FUNC_GPIO58          0
#define XGPIO_058_FUNC_PWM4            1

#define XGPIO_059                      59
#define XGPIO_059_FUNC_GPIO59          0

#define XGPIO_060                      60
#define XGPIO_060_FUNC_GPIO60          0

#define XGPIO_061                      61
#define XGPIO_061_FUNC_GPIO61          0

#define XGPIO_062                      62
#define XGPIO_062_FUNC_GPIO62          0

#define XGPIO_063                      63
#define XGPIO_063_FUNC_GPIO63          0

#define XGPIO_064                      64
#define XGPIO_064_FUNC_CIS0_SCLK       0
#define XGPIO_064_FUNC_GPIO64          1

#define XGPIO_065                      65
#define XGPIO_065_FUNC_CIS0_PCLK       0
#define XGPIO_065_FUNC_GPIO65          1

#define XGPIO_066                      66
#define XGPIO_066_FUNC_CIS0_HSYNC      0
#define XGPIO_066_FUNC_GPIO66          1

#define XGPIO_067                      67
#define XGPIO_067_FUNC_CIS0_VSYNC      0
#define XGPIO_067_FUNC_GPIO67          1

#define XGPIO_068                      68
#define XGPIO_068_FUNC_CIS0_D4         0
#define XGPIO_068_FUNC_GPIO68          1

#define XGPIO_069                      69
#define XGPIO_069_FUNC_CIS0_D5         0
#define XGPIO_069_FUNC_GPIO69          1

#define XGPIO_070                      70
#define XGPIO_070_FUNC_CIS0_D9         0
#define XGPIO_070_FUNC_GPIO70          1

#define XGPIO_071                      71
#define XGPIO_071_FUNC_CIS0_D8         0
#define XGPIO_071_FUNC_GPIO71          1

#define XGPIO_072                      72
#define XGPIO_072_FUNC_CIS0_D10        0
#define XGPIO_072_FUNC_GPIO72          1

#define XGPIO_073                      73
#define XGPIO_073_FUNC_CIS0_D11        0
#define XGPIO_073_FUNC_GPIO73          1

#define XGPIO_074                      74
#define XGPIO_074_FUNC_CIS0_D7         0
#define XGPIO_074_FUNC_GPIO74          1

#define XGPIO_075                      75
#define XGPIO_075_FUNC_CIS0_D6         0
#define XGPIO_075_FUNC_GPIO75          1

#define XGPIO_076                      76
#define XGPIO_076_FUNC_GPIO76          0
#define XGPIO_076_FUNC_MII_TXCLK 	   1

#define XGPIO_077                      77
#define XGPIO_077_FUNC_GPIO77          0
#define XGPIO_077_FUNC_MII_RXCLK 	   1

#define XGPIO_078                      78
#define XGPIO_078_FUNC_GPIO78          0
#define XGPIO_078_FUNC_MII_COL 	       1

#endif //__AK_39EV200_PINCTRL_H__
