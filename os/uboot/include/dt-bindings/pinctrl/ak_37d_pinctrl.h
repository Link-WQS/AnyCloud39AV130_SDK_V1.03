/*
 * This header provides constants for the AK37E GPIO binding.
 */
#ifndef __AK_37D_PINCTRL_H__
#define __AK_37D_PINCTRL_H__

#define XGPIO_000                      0
#define XGPIO_000_FUNC_GPIO0           0
#define XGPIO_000_FUNC_UART3_RXD       1

#define XGPIO_001                      1
#define XGPIO_001_FUNC_GPIO1           0
#define XGPIO_001_FUNC_UART0_RXD       1
#define XGPIO_001_FUNC_PWM4            2


#define XGPIO_002                      2
#define XGPIO_002_FUNC_GPIO2           0
#define XGPIO_002_FUNC_UART0_TXD       1
#define XGPIO_002_FUNC_PWM3            2

#define XGPIO_003                      3
#define XGPIO_003_FUNC_GPIO3           0
#define XGPIO_003_FUNC_PWM0            1
#define XGPIO_003_FUNC_SD2_D0          2

#define XGPIO_004                      4
#define XGPIO_004_FUNC_GPIO4           0
#define XGPIO_004_FUNC_PWM1            1
#define XGPIO_004_FUNC_UART3_TXD       2
#define XGPIO_004_FUNC_SD2_CMD         3

#define XGPIO_005                      5
#define XGPIO_005_FUNC_GPIO5           0
#define XGPIO_005_FUNC_PWM2            1
#define XGPIO_005_FUNC_UART3_RXD       2
#define XGPIO_005_FUNC_SD2_MCK         3

#define XGPIO_006                      6
#define XGPIO_006_FUNC_GPIO6           0
#define XGPIO_006_FUNC_UART1_RXD       1
#define XGPIO_006_FUNC_PWM1            2
#define XGPIO_006_FUNC_TWI3_SDA        3


#define XGPIO_007                      7
#define XGPIO_007_FUNC_GPIO7           0
#define XGPIO_007_FUNC_UART1_TXD       1
#define XGPIO_007_FUNC_PWM1            2
#define XGPIO_007_FUNC_TWI3_SDA        3

#define XGPIO_008                      8
#define XGPIO_008_FUNC_GPIO8           0
#define XGPIO_008_FUNC_UART1_CTS       1
#define XGPIO_008_FUNC_IRDA_RX         2


#define XGPIO_009                      9
#define XGPIO_009_FUNC_GPIO9           0
#define XGPIO_009_FUNC_UART1_RTS       1


#define XGPIO_010                      10
#define XGPIO_010_FUNC_GPIO10          0
#define XGPIO_010_FUNC_RMII_MDC        1
#define XGPIO_010_FUNC_TWI3_SCL        2
#define XGPIO_010_FUNC_PWM0            3

#define XGPIO_011                      11
#define XGPIO_011_FUNC_GPIO11          0
#define XGPIO_011_FUNC_RMII_MDIO       1
#define XGPIO_011_FUNC_TWI3_SDA        2


#define XGPIO_012                      12
#define XGPIO_012_FUNC_SPI0_CS         0
#define XGPIO_012_FUNC_GPIO12          1

#define XGPIO_013                      13
#define XGPIO_013_FUNC_GPIO13          0
#define XGPIO_013_FUNC_RMII_TXEN       1

#define XGPIO_014                      14
#define XGPIO_014_FUNC_GPIO14          0
#define XGPIO_014_FUNC_RMII_TXD0       1
#define XGPIO_014_FUNC_SD2_D3          2


#define XGPIO_015                      15
#define XGPIO_015_FUNC_GPIO15          0
#define XGPIO_015_FUNC_RMII_TXD1       1
#define XGPIO_015_FUNC_SD2_D2          2


#define XGPIO_016                      16
#define XGPIO_016_FUNC_GPIO16          0
#define XGPIO_016_FUNC_GPO0            1

#define XGPIO_017                      17
#define XGPIO_017_FUNC_GPIO17          0
#define XGPIO_017_FUNC_GPO1			   1


#define XGPIO_018                      18
#define XGPIO_018_FUNC_GPIO18          0
#define XGPIO_018_FUNC_UART3_TXD       1


#define XGPIO_019                      19
#define XGPIO_019_FUNC_GPIO19          0
#define XGPIO_019_FUNC_RMII_RXD0       1
#define XGPIO_019_FUNC_SD2_MCK         2

#define XGPIO_020                      20
#define XGPIO_020_FUNC_GPIO20          0
#define XGPIO_020_FUNC_RMII_RXD1       1
#define XGPIO_020_FUNC_SD2_CMD         2


#define XGPIO_021                      21
#define XGPIO_021_FUNC_GPIO21          0
#define XGPIO_021_FUNC_GPO2		       1


#define XGPIO_022                      22
#define XGPIO_022_FUNC_GPIO22          0
#define XGPIO_022_FUNC_GPO3            1


#define XGPIO_023                      23
#define XGPIO_023_FUNC_GPIO23          0
#define XGPIO_023_FUNC_RMII_RXER       1
#define XGPIO_023_FUNC_SD2_D1          2
#define XGPIO_023_FUNC_PWM3            3

#define XGPIO_024                      24
#define XGPIO_024_FUNC_GPIO24          0
#define XGPIO_024_FUNC_RMII_RXDV       1
#define XGPIO_024_FUNC_SD2_D0          2
#define XGPIO_024_FUNC_PWM4            3

#define XGPIO_025                      25
#define XGPIO_025_FUNC_GPIO25          0
#define XGPIO_025_FUNC_CSI_SCLK        1


#define XGPIO_026                      26
#define XGPIO_026_FUNC_GPIO26          0
#define XGPIO_026_FUNC_CSI_PCLK        1
#define XGPIO_026_FUNC_CSI_SCLK1       2


#define XGPIO_027                      27
#define XGPIO_027_FUNC_GPIO27          0
#define XGPIO_027_FUNC_CSI_HSYNC       1
#define XGPIO_027_FUNC_PWM3            2
#define XGPIO_027_FUNC_TWI1_SCL        3


#define XGPIO_028                      28
#define XGPIO_028_FUNC_GPIO28          0
#define XGPIO_028_FUNC_CSI_VSYNC       1
#define XGPIO_028_FUNC_PWM4            2
#define XGPIO_028_FUNC_TWI1_SDA        3


#define XGPIO_029                      29
#define XGPIO_029_FUNC_GPIO29          0
#define XGPIO_029_FUNC_PWM0            1


#define XGPIO_030                      30
#define XGPIO_030_FUNC_GPIO30          0
#define XGPIO_030_FUNC_PWM1            1

#define XGPIO_031                      31
#define XGPIO_031_FUNC_GPIO31          0
#define XGPIO_031_FUNC_TWI0_SCL        1

#define XGPIO_032                      32
#define XGPIO_032_FUNC_GPIO32          0
#define XGPIO_032_FUNC_TWI0_SDA        1

#define XGPIO_033                      33
#define XGPIO_033_FUNC_GPIO33          0
#define XGPIO_033_FUNC_SD0_CMD         1

#define XGPIO_034                      34
#define XGPIO_034_FUNC_GPIO34          0
#define XGPIO_034_FUNC_SD0_MCK         1
#define XGPIO_034_FUNC_UART2_TXD       2

#define XGPIO_035                      35
#define XGPIO_035_FUNC_GPIO35          0
#define XGPIO_035_FUNC_SD0_D0          1
#define XGPIO_035_FUNC_UART2_RXD       2
#define XGPIO_035_FUNC_JTAG_RSTN       3


#define XGPIO_036                      36
#define XGPIO_036_FUNC_GPIO36          0
#define XGPIO_036_FUNC_SD0_D1          1
#define XGPIO_036_FUNC_UART1_RXD       2
#define XGPIO_036_FUNC_JTAG_RTCK       3


#define XGPIO_037                      37
#define XGPIO_037_FUNC_GPIO37          0
#define XGPIO_037_FUNC_SD0_D2          1
#define XGPIO_037_FUNC_PWM0            2
#define XGPIO_037_FUNC_UART1_TXD       3

#define XGPIO_038                      38
#define XGPIO_038_FUNC_GPIO38          0
#define XGPIO_038_FUNC_SD0_D3          1
#define XGPIO_038_FUNC_PWM1            2
#define XGPIO_038_FUNC_JTAG_TCK        3


#define XGPIO_039                      39
#define XGPIO_039_FUNC_GPIO39          0
#define XGPIO_039_FUNC_SD0_D4          1
#define XGPIO_039_FUNC_PWM2            2
#define XGPIO_039_FUNC_JTAG_TMS        3

#define XGPIO_040                      40
#define XGPIO_040_FUNC_GPIO40          0
#define XGPIO_040_FUNC_SD0_D5          1
#define XGPIO_040_FUNC_PWM3            2
#define XGPIO_040_FUNC_JTAG_TDI        3

#define XGPIO_041                      41
#define XGPIO_041_FUNC_GPIO41          0
#define XGPIO_041_FUNC_SD0_D6          1
#define XGPIO_041_FUNC_PWM4            2
#define XGPIO_041_FUNC_JTAG_TDOUT      3

#define XGPIO_042                      42
#define XGPIO_042_FUNC_GPIO42          0
#define XGPIO_042_FUNC_SD0_D7          1


#define XGPIO_043                      43
#define XGPIO_043_FUNC_GPIO43          0
#define XGPIO_043_FUNC_SD1_CMD         1
#define XGPIO_043_FUNC_SPI1_SS         2

#define XGPIO_044                      44
#define XGPIO_044_FUNC_GPIO44          0
#define XGPIO_044_FUNC_SD1_MCK         1
#define XGPIO_044_FUNC_SPI1_SCLK       2

#define XGPIO_045                      45
#define XGPIO_045_FUNC_GPIO45          0
#define XGPIO_045_FUNC_SD1_D0          1
#define XGPIO_045_FUNC_SPI1_HOLD       2

#define XGPIO_046                      46
#define XGPIO_046_FUNC_GPIO46          0
#define XGPIO_046_FUNC_SD1_D1          1
#define XGPIO_046_FUNC_SPI1_WP         2


#define XGPIO_047                      47
#define XGPIO_047_FUNC_GPIO47          0
#define XGPIO_047_FUNC_SD1_D2          1
#define XGPIO_047_FUNC_SPI1_DOUT       2


#define XGPIO_048                      48
#define XGPIO_048_FUNC_GPIO48          0
#define XGPIO_048_FUNC_SD1_D3          1
#define XGPIO_048_FUNC_SPI1_DIN        2


#define XGPIO_049                      49
#define XGPIO_049_FUNC_GPIO49          0
#define XGPIO_049_FUNC_OPCLK           1
#define XGPIO_049_FUNC_SD2_D0          2
#define XGPIO_049_FUNC_PWM1            3

#define XGPIO_050                      50
#define XGPIO_050_FUNC_GPIO50          0
#define XGPIO_050_FUNC_PWM0            1
#define XGPIO_050_FUNC_UART1_TXD       2


#define XGPIO_051                      51
#define XGPIO_051_FUNC_GPIO51          0
#define XGPIO_051_FUNC_PWM1            1
#define XGPIO_051_FUNC_UART1_CTS       2


#define XGPIO_052                      52
#define XGPIO_052_FUNC_GPIO52          0
#define XGPIO_052_FUNC_PWM2            1
#define XGPIO_052_FUNC_UART1_RXD       2


#define XGPIO_053                      53
#define XGPIO_053_FUNC_GPIO53          0
#define XGPIO_053_FUNC_I2S_DOUT        1
#define XGPIO_053_FUNC_UART1_RTS       2


#define XGPIO_054                      54
#define XGPIO_054_FUNC_GPIO54          0
#define XGPIO_054_FUNC_I2S_MCLK        1


#define XGPIO_055                      55
#define XGPIO_055_FUNC_GPIO55          0
#define XGPIO_055_FUNC_I2S_BCLK        1


#define XGPIO_056                      56
#define XGPIO_056_FUNC_GPIO56          0
#define XGPIO_056_FUNC_I2S_LRCLK       1


#define XGPIO_057                      57
#define XGPIO_057_FUNC_GPIO57          0
#define XGPIO_057_FUNC_I2S_DIN         1


#define XGPIO_058                      58
#define XGPIO_058_FUNC_GPIO58          0
#define XGPIO_058_FUNC_PWM3            1
#define XGPIO_058_FUNC_TWI2_SDA        2


#define XGPIO_059                      59
#define XGPIO_059_FUNC_GPIO59          0
#define XGPIO_059_FUNC_PWM4   		   1
#define XGPIO_059_FUNC_TWI2_SCL        2


#define XGPIO_060                      60
#define XGPIO_060_FUNC_GPIO60          0
#define XGPIO_060_FUNC_PWM0            1


#define XGPIO_061                      61
#define XGPIO_061_FUNC_GPIO61          0
#define XGPIO_061_FUNC_PWM2            1


#define XGPIO_062                      62
#define XGPIO_062_FUNC_GPIO62          0
#define XGPIO_062_FUNC_PWM1            1


#define XGPIO_063                      63
#define XGPIO_063_FUNC_GPIO63          0


#define XGPIO_064                      64
#define XGPIO_064_FUNC_GPIO64          0


#define XGPIO_065                      65
#define XGPIO_065_FUNC_GPIO65          0


#define XGPIO_066                      66
#define XGPIO_066_FUNC_GPIO66          0


#define XGPIO_067                      67
#define XGPIO_067_FUNC_SPI0_DIN        0
#define XGPIO_067_FUNC_GPIO67          1


#define XGPIO_068                      68
#define XGPIO_068_FUNC_SPI0_DOUT       0
#define XGPIO_068_FUNC_GPIO68          1


#define XGPIO_069                      69
#define XGPIO_069_FUNC_SPI0_WP         0
#define XGPIO_069_FUNC_GPIO69	       1


#define XGPIO_070                      70
#define XGPIO_070_FUNC_SPI0_HOLD       0
#define XGPIO_070_FUNC_GPIO70	       1


#define XGPIO_071                      71
#define XGPIO_071_FUNC_GPIO71          0
#define XGPIO_071_FUNC_RGB_VOGATE      1


#define XGPIO_072                      72
#define XGPIO_072_FUNC_GPIO72          0
#define XGPIO_072_FUNC_RGB_VOVSYNC     1


#define XGPIO_073                      73
#define XGPIO_073_FUNC_GPIO73          0
#define XGPIO_073_FUNC_RGB_VOHSYNC     1


#define XGPIO_074                      74
#define XGPIO_074_FUNC_GPIO74          0
#define XGPIO_074_FUNC_RGB_VOPCLK      1


#define XGPIO_075                      75
#define XGPIO_075_FUNC_GPIO75          0
#define XGPIO_075_FUNC_RGB_D0          1


#define XGPIO_076                      76
#define XGPIO_076_FUNC_GPIO76          0
#define XGPIO_076_FUNC_RGB_D1          1


#define XGPIO_077                      77
#define XGPIO_077_FUNC_GPIO77          0
#define XGPIO_077_FUNC_RGB_D2          1


#define XGPIO_078                      78
#define XGPIO_078_FUNC_GPIO78          0
#define XGPIO_078_FUNC_RGB_D3          1


#define XGPIO_079                      79
#define XGPIO_079_FUNC_GPIO79          0
#define XGPIO_079_FUNC_RGB_D4          1


#define XGPIO_080                      80
#define XGPIO_080_FUNC_GPIO80          0
#define XGPIO_080_FUNC_RGB_D5          1

#define XGPIO_081                      81
#define XGPIO_081_FUNC_GPIO81          0
#define XGPIO_081_FUNC_RGB_D6          1


#define XGPIO_082                      82
#define XGPIO_082_FUNC_GPIO82          0
#define XGPIO_082_FUNC_RGB_D7          1


#define XGPIO_083                      83
#define XGPIO_083_FUNC_GPIO83          0
#define XGPIO_083_FUNC_RGB_D8          1


#define XGPIO_084                      84
#define XGPIO_084_FUNC_GPIO84          0
#define XGPIO_084_FUNC_RGB_D9          1


#define XGPIO_085                      85
#define XGPIO_085_FUNC_GPIO85          0
#define XGPIO_085_FUNC_RGB_D10         1


#define XGPIO_086                      86
#define XGPIO_086_FUNC_GPIO86          0
#define XGPIO_086_FUNC_RGB_D11         1
#define XGPIO_086_FUNC_MPU_D11         1
#define XGPIO_086_FUNC_I2S_LRCLK       2



#define XGPIO_087                      87
#define XGPIO_087_FUNC_GPIO87          0
#define XGPIO_087_FUNC_RGB_D12         1
#define XGPIO_087_FUNC_I2S_BCLK        2


#define XGPIO_088                      88
#define XGPIO_088_FUNC_GPIO88          0
#define XGPIO_088_FUNC_RGB_D13         1
#define XGPIO_088_FUNC_I2S_MCLK        2


#define XGPIO_089                      89
#define XGPIO_089_FUNC_GPIO89          0
#define XGPIO_089_FUNC_RGB_D14         1
#define XGPIO_089_FUNC_I2S_DOUT        2
#define XGPIO_089_FUNC_PWM3        	   3

#define XGPIO_090                      90
#define XGPIO_090_FUNC_GPIO90          0
#define XGPIO_090_FUNC_RGB_D15         1
#define XGPIO_090_FUNC_PWM4        	   2


#define XGPIO_091                      91
#define XGPIO_091_FUNC_GPIO91          0
#define XGPIO_091_FUNC_RGB_D16         1
#define XGPIO_091_FUNC_SD2_D2          2


#define XGPIO_092                      92
#define XGPIO_092_FUNC_GPIO92          0
#define XGPIO_092_FUNC_RGB_D17         1
#define XGPIO_092_FUNC_SD2_D3          2

#define XGPIO_093                      93
#define XGPIO_093_FUNC_GPIO93          0
#define XGPIO_093_FUNC_RGB_D18         1
#define XGPIO_093_FUNC_SPI1_SS         2
#define XGPIO_093_FUNC_SD2_CMD         3


#define XGPIO_094                      94
#define XGPIO_094_FUNC_GPIO94          0
#define XGPIO_094_FUNC_RGB_D19         1
#define XGPIO_094_FUNC_SPI1_SCLK       2
#define XGPIO_094_FUNC_SD2_MCK         3


#define XGPIO_095                      95
#define XGPIO_095_FUNC_GPIO95          0
#define XGPIO_095_FUNC_RGB_D20         1
#define XGPIO_095_FUNC_SPI1_DOUT       2
#define XGPIO_095_FUNC_SD2_D0          3

#define XGPIO_096                      96
#define XGPIO_096_FUNC_GPIO96          0
#define XGPIO_096_FUNC_RGB_D21         1
#define XGPIO_096_FUNC_SPI1_DIN        2
#define XGPIO_096_FUNC_SD2_D1          3

#define XGPIO_097                      97
#define XGPIO_097_FUNC_GPIO97          0
#define XGPIO_097_FUNC_RGB_D22         1
#define XGPIO_097_FUNC_SPI1_HOLD       2
#define XGPIO_097_FUNC_TWI2_SDA        3

#define XGPIO_098                      98
#define XGPIO_098_FUNC_GPIO98          0
#define XGPIO_098_FUNC_RGB_D23         1
#define XGPIO_098_FUNC_SPI1_WP         2
#define XGPIO_098_FUNC_TWI2_SCL        3
#define XGPIO_098_FUNC_SPI1_DIN        4



#define XGPIO_099                      99
#define XGPIO_099_FUNC_GPIO99          0


#define XGPIO_0100                     100
#define XGPIO_0100_FUNC_GPIO100        0


#define XGPIO_0101                     101
#define XGPIO_0101_FUNC_GPIO101        0


#define XGPIO_0102                     102
#define XGPIO_0102_FUNC_GPIO102        0


#define XGPIO_0103                     103
#define XGPIO_0103_FUNC_GPIO103        0


#define XGPIO_0108                     108
#define XGPIO_0108_FUNC_GPIO108        0
#define XGPIO_0108_FUNC_GPI4		   1

#define XGPIO_0107                     107
#define XGPIO_0107_FUNC_GPIO107        0
#define XGPIO_0107_FUNC_GPI3		   1

#define XGPIO_0106                     106
#define XGPIO_0106_FUNC_GPIO106        0
#define XGPIO_0106_FUNC_GPI2		   1

#define XGPIO_0105                     105
#define XGPIO_0105_FUNC_GPIO105        0
#define XGPIO_0105_FUNC_GPI1		   1

#define XGPIO_0104                     104
#define XGPIO_0104_FUNC_GPIO104        0
#define XGPIO_0104_FUNC_GPI0		   1


#endif //__AK_37E_PINCTRL_H__
