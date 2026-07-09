/**
* @FILENAME: clk_tree_drv.h
* @BRIEF:  system clock cfg driver
* @Copyright (C) 2012 Anyka (ShenZhen) Micro-Electronic Technology Co., LTD
* @AUTHOR  ZouTianxiang 
* @DATA 2012-10-24   
* @VERSION 1.0
* @REF please refer to...
*/


#ifndef __CLK_TREE_DRV_H_
#define __CLK_TREE_DRV_H_

#include "AK_types.h"
#include "AK_reg.h"
#include "console.h"


typedef struct _clk_pll_clock_para
{
    T_U32 NF;
    T_U32 NR;
    T_U32 OD;
}T_CLK_PLL_CLOCK_PARA;





typedef struct _clk_audio_pll_clock_para
{
    T_U32 NF;
    T_U32 NR;
    T_U32 OD;
}T_CLK_AUDIO_PLL_CLOCK_PARA;




typedef enum _clk_peri_type
{
    CLK_TCM_ROM = 0,                        // 0          0~31�ļĴ�����0x0800001c
    CLK_MMC_SD0,                            // 1
    CLK_MMC_SD1,                            // 2
    CLK_ADC,                                // 3
    CLK_DAC,                                // 4
    CLK_SFC,                                // 5
    CLK_SPI0,                               // 6
    CLK_UART0,                              // 7
    CLK_UART1,                              // 8
    CLK_L2_BUFFFER0,                        // 9
    CLK_I2C0,                               // 10
    FFT,                                    // 11
    CLK_GPIO,                               // 12
    CLK_MAC0,                               // 13
    CLK_ENDECRYPT,                          // 14
    CLK_USB_OTG,                            // 15
    CLK_MIPI_CSI0,                          // 16
    CLK_I2C1,                               // 17
    CLK_UART2,                              // 18
    CLK_MIPI_CSI1,                          // 19
    CLK_SPI1,                               // 20
    CLK_MMC_SD2,                            // 21
    CLK_I2C2,                               // 22
    CLK_PDM,                                // 23
    CLK_DRAM,                               // 24       //CLK_DRAM ����dclk��
    CLK_DRAM_PHY,                           // 25       //CLK_DRAM_PHY ����dphyclk��
    CLK_FEATURE0_RESERVED,                  // 26
    CLK_FEATURE1_RESERVED,                  // 27
    CLK_FEATURE2_RESERVED,                  // 28
    CLK_FEATURE3_RESERVED,                  // 29
    CLK_FEATURE4_RESERVED,                  // 30
    CLK_FEATURE5_RESERVED,                  // 31

    CLK_HASH,                               // 32
    CLK_TRNG,                               // 33
    
    CLK_ISP,                                //34        //ISP����VCLK ��
    CLK_CNN,                                //35        //CNN����VCLK ��
    CLK_VIDEO_ENCODER,                      //36        //ENCODER����VCLK ��
    CLK_PRE_PROCESSOR,                      //37              //PP����VCLK ��

    
    
}T_CLK_PERI_TYPE;





//2021.2.28����H322 ��cpu pll, jclk, dclk, dphyclk��ģʽ���ö���2��ģʽ��
//�˴����ú�CLK TREE����ĵ�
typedef enum _clk_jclk_dclk_dphyclk_mode
{
    JCLK_DDR2CLK_2X1X_MODE = 0, //jclk = dphyclk = cpu pll,  ddr2clk = dclk=hclk=cpu pll/2        //CPU2xMEM1xģʽ   Defaultģʽ
    JCLK_DDR2CLK_1X1X_MODE,     //dphyclk = cpu pll, jclk = ddr2clk = dclk=hclk=cpu pll/2        //CPU1xMEM1xģʽ
}T_CLK_JCLK_DDR2CLK_MODE;


//SKY: 2021.2.28���Ӹ�ע:
//CPU2XMEM1X ����ѹ(0.8V)Max Freq: CPU/DDR2 = 810/405MZH  ,  ��ѹ��Ŀ��Ƶ��CPU/DDR2=1GHZ/500MHz
//CPU1XMEM1X ����ѹ(0.8V)Max Freq: CPU/DDR2 = 500/500MZH  


//3��pll ��֧
typedef enum _clk_pll_type
{
    CPU_PLL = 0,
    CORE_PLL,
    AUDIO_PLL,
    USB_PHY_PLL,        //M31�Ѹ�֪����jitter���ܱ�֤������USB PHY PLL��������ģ�飬
}T_CLK_PLL_TYPE;



//��VCLK ʱ�����µ�ģ��
typedef enum _clk_module_in_vclk
{
    ISP_VCLK = 0,
    CNN_VCLK,
    ENC_VCLK,
    PP_VCLK,
}T_CLK_MODULE_IN_VCLK;





//CSI_SCLK ��ʹ�õ�ʱ������
typedef enum _clk_csi_sclk_src
{
    CSI_SCLK_USE_24MHZ = 0,             //24MHZ ����
    CSI_SCLK_USE_CSI_SRC0_CLK,          //CSI_SRC0_CLKʱ��
    CSI_SCLK_USE_CSI_SRC1_CLK,          //CSI_SRC1_CLKʱ��
}T_CLK_CSI_SCLK_SRC;




//SENSOR  SCLK
typedef enum _clk_csi_sclk
{
    CSI_SCLK0 = 0,
    CSI_SCLK1,
    CSI_SCLK2,
    CSI_SCLK3,
}T_CLK_CSI_SCLK_INDEX;




//CSI_SCLK ���ýṹ��
typedef struct _clk_sensor_sclk_cfg
{
    T_CLK_CSI_SCLK_INDEX csi_sclk_index;
    T_CLK_CSI_SCLK_SRC csi_sclk_src;
    T_CLK_PLL_TYPE pll_type;
    T_U32 div;
}T_CLK_CSI_SCLK_CFG;





#define CPU_PLL_MAX_FREQ                1000000000  // 1GHZ
#define CORE_PLL_MAX_FREQ               800000000   //
#define CPU_CORE_PLL_MIN_FREQ           31250000    // 500/(15+1) = 31.25MHZ
#define AUDIO_PLL_MAX_FREQ              800000000   
#define AUDIO_PLL_MIN_FREQ              31250000

#define CRYSTAL_FREQUENCE               24000000


/* get func */
T_U32 ak_clk_get_core_pll_freq(void);
T_U32 clk_get_core_pll_freq(void);
T_U32 clk_get_audio_pll_freq(void);
T_U32 clk_get_jclk_freq(void);
T_U32 clk_get_hclk_freq(void);
T_U32 clk_get_dclk_freq(void);
T_U32 clk_get_dphyclk_freq(void);
T_U32 clk_get_ddr2clk_freq(void);
T_U32 clk_get_vclk_freq(T_CLK_MODULE_IN_VCLK module_vclk);
T_U32 clk_get_gclk_freq(void);
T_U32 clk_get_peripheral_gclk_module_freq( T_CLK_PERI_TYPE  peri_type );
T_U32 clk_get_sar_adc_clk_freq();
T_U32 clk_get_sddac_clk_freq();
T_U32 clk_get_sddac_hsclk_freq();
T_U32 clk_get_sdadc_clk_freq();
T_U32 clk_get_sdadc_hsclk_freq();
T_U32 clk_get_mac_opclk_freq();
T_U32 clk_get_csi_sclk_freq(T_CLK_CSI_SCLK_INDEX sensor_sclk);
T_U32 clk_get_otg_phy_freq();
T_U32 clk_get_mipi_dphy_clk_freq();
T_CLK_JCLK_DDR2CLK_MODE clk_get_cpu_clk_mode(T_VOID);

/* set func */
T_BOOL clk_set_cpu_pll_freq_auto(T_U32 freq);
T_BOOL clk_set_core_pll_freq_auto(T_U32 freq);
T_U32 clk_set_audio_pll_freq_auto(T_U32 freq);
T_BOOL clk_set_cpu_pll_freq_manual(T_CLK_PLL_CLOCK_PARA *p_pll_clk_para );
T_BOOL clk_set_core_pll_freq_manual(T_CLK_PLL_CLOCK_PARA *p_pll_clk_para );
T_U32 clk_set_audio_pll_freq_manual(T_CLK_AUDIO_PLL_CLOCK_PARA *p_audio_pll_clk_para);
T_VOID clk_set_gclk_div(T_U32 div);
T_VOID clk_set_vclk_div(T_U32 div, T_CLK_PLL_TYPE src_pll_type, T_CLK_MODULE_IN_VCLK module_vclk);
T_VOID clk_set_csi_sclk_div(T_CLK_CSI_SCLK_CFG csi_sclk_cfg);
T_VOID clk_set_sfc_phyclk_div(T_U32 div);
T_VOID clk_set_mipi_dphy_clk_div(T_U32 div);
T_VOID clk_set_opclk_div(T_U32 div, T_CLK_PLL_TYPE src_pll_type);
T_VOID clk_set_cpu_clk_mode(T_CLK_JCLK_DDR2CLK_MODE  jclk_dclk_dphyclk_mode);

/* open & close & reset func */
T_VOID clk_open_csi_sclk(T_CLK_CSI_SCLK_INDEX  sensor_sclk);
T_VOID clk_close_csi_sclk(T_CLK_CSI_SCLK_INDEX  sensor_sclk);
T_VOID clk_open_opclk();
T_VOID clk_close_opclk();
T_VOID clk_open_opclk_src();
T_VOID clk_close_opclk_src();
T_VOID clk_open_csi_src(T_CLK_CSI_SCLK_SRC  csi_sclk_src);
T_VOID clk_close_csi_src(T_CLK_CSI_SCLK_SRC  csi_sclk_src);
T_VOID clk_open_peripheral_module_gate( T_CLK_PERI_TYPE  peri_type );
T_VOID clk_close_peripheral_module_gate( T_CLK_PERI_TYPE  peri_type );
T_VOID clk_reset_peripheral_module( T_CLK_PERI_TYPE  peri_type );
T_VOID clk_hold_reset_peripheral_module( T_CLK_PERI_TYPE  peri_type );
T_VOID clk_release_reset_peripheral_module( T_CLK_PERI_TYPE  peri_type );

/* clutter func */
T_VOID clk_copy_data_to_interram(T_U8 *addr, T_U32 size);

#endif /* __CONSOLE_H_ */

