#ifndef _USER_CFG_H_
#define _USER_CFG_H_

#define CONFIG_MACH_AV130

#ifdef CONFIG_MACH_KM01A
#define GPIO_KM01A_PWR_EN    PB_4
#define GPIO_VBUS_DETECT     PA_3
#define GPIO_WIFI_ST         PA_7
#define GPIO_PIR_IN_MCU_C    PB_1
#define GPIO_CHARGER_EN      (-1)
#define GPIO_LED1            PA_1
#define GPIO_LED2            PA_0
#define GPIO_PIR_OUT_MCU_C   PB_0
#define GPIO_WAKEUP_WIFI     PA_10
#define GPIO_WAKEUP_AK       PA_15
#define GPIO_WAKEUP_MCU_BUTTON PB_2
#define GPIO_ADC_CH1_MCU     PB_3
#define GPIO_BAT_CHARGE_STA  PA_2
#elif defined(CONFIG_MACH_AV130)
#define GPIO_KM01A_PWR_EN    PB_4
#define GPIO_VBUS_DETECT     (-1)
#define GPIO_WIFI_ST         (-1)
#define GPIO_PIR_IN_MCU_C    (-1)
#define GPIO_CHARGER_EN      (-1)
#define GPIO_LED1            (-1)
#define GPIO_LED2            (-1)
#define GPIO_PIR_OUT_MCU_C   (-1)
#define GPIO_WAKEUP_WIFI     PA_10
#define GPIO_WAKEUP_AK       PA_15
#define GPIO_WAKEUP_MCU_BUTTON PB_2
#define GPIO_ADC_CH1_MCU     (-1)
#define GPIO_BAT_CHARGE_STA  (-1)
#elif defined(CONFIG_MACH_KM01A_GOLDEN_SAMPLE)
#define GPIO_KM01A_PWR_EN    PA_6
#define GPIO_VBUS_DETECT     (-1)
#define GPIO_WIFI_ST         PA_7
#define GPIO_PIR_IN_MCU_C    PA_5
#define GPIO_CHARGER_EN      (-1)
#define GPIO_LED1            PA_1
#define GPIO_LED2            PA_3
#define GPIO_PIR_OUT_MCU_C   (-1)
#define GPIO_WAKEUP_WIFI     PA_10
#define GPIO_WAKEUP_AK       PA_15
#define GPIO_WAKEUP_MCU_BUTTON PB_4
#define GPIO_ADC_CH1_MCU     PB_3
#define GPIO_BAT_CHARGE_STA  (-1)
#else /* KM01W */
#define GPIO_KM01A_PWR_EN    PA_0
#define GPIO_VBUS_DETECT     PA_1
#define GPIO_WIFI_ST         PA_2
#define GPIO_PIR_IN_MCU_C    PA_3
#define GPIO_CHARGER_EN      PA_4
#define GPIO_LED1            PA_5
#define GPIO_LED2            PA_6
#define GPIO_PIR_OUT_MCU_C   PA_7
#define GPIO_WAKEUP_WIFI     PA_10
#define GPIO_WAKEUP_AK       PA_15
#define GPIO_WAKEUP_MCU_BUTTON PB_0
#define GPIO_ADC_CH1_MCU     PB_2
#define GPIO_BAT_CHARGE_STA  PB_3
#endif

#define NO_SERVER_KEEPALIVE

#endif
