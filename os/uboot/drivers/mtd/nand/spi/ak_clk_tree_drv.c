/**
* @FILENAME: clk_tree_drv.c
* @BRIEF:  system clock cfg driver
* @Copyright (C) 2020 Anyka (GuangZhou) Micro-Electronic Technology Co., LTD
* @AUTHOR  ZouTianxiang 
* @DATA 2020-3-24
* @VERSION 1.0
* @REF please refer to...
*/

#include "ak_clk_tree_drv.h"
#include "AK_reg.h"


#define NF_MAX_VAL  8192
#define NR_MAX_VAL  64



#if 0

typedef struct
{
    T_U32 jclk;                 //CPU CLK
    T_U32 hclk;                 //AHB BRIDGE CLK
    T_U32 dclk;                 //MEM CLK
    T_U32 dphyclk;              //MEM DPHY CLK
    T_U32 ddr2clk;              //MEM ddr2 clk  ����DDR2��ʵ�����ʱ��
}T_CPU_PLL_GROUP, *T_pCPU_PLL_GROUP;



#ifdef GCC_COMPILE
    //�����ڲ�RAM�ϵı�CPU PLL�Ĵ���
    static T_U8 change_cpu_pll_freq_code[1024] __attribute__((aligned(4))) = 
    {
        #include "change_cpu_pll_freq_code_out.dat"
    };

#else
    //�����ڲ�RAM�ϵı�CPU PLL�Ĵ���
    static __align(4) T_U8 change_cpu_pll_freq_code[1024] = 
    {
        #include "change_cpu_pll_freq_code_out.dat"
    };

#endif


//������������ΪSTATIC������ʹ��
static void clk_get_jclk_hclk_dclk_dphyclk_ddr2clk_freq(T_pCPU_PLL_GROUP p_cpu_pll_group);
static T_U32 clk_get_expect_ddr2clk(T_U32 expect_cpu_pll_clk);



//Pll clk parameter OD 
static const T_U32 OD_SET[] = {1, 2, 4, 6, 8, 10, 12, 14, 16};


//max frequence  
static const T_U32 MAX_PLL_FREQ_SET[2] = {CPU_PLL_MAX_FREQ, CORE_PLL_MAX_FREQ};   //cpu pll, core pll, peri pll

//divider val
static const T_U32 DIV_VAL_SET[4] = { 2, 4, 6, 8 };
static const T_U32 DIV_SEL_SET[4] = { 0, 1, 2, 3 };




/**
* @BRIEF  get cpu pll clk
* @AUTHOR Zou Tianxiang
* @DATE   2012-10 -24  
* @CHANGE ON DATE   2021-2 -28 FOR H322
* @PARAM  
* @PARAM  
* @RETURN cpu pll frequence   unit: HZ
* @NOTE: 
*/
T_U32 clk_get_cpu_pll_freq(void)
{
    T_U32 nf, nr, od;   
    T_U32 pll_cfg_val;
    T_U32 cpu_pll_freq;
    
    pll_cfg_val = inl(CPU_PLL_CFG_REG) ;
    nf = (pll_cfg_val & 0x1fff) + 1;
    nr = ((pll_cfg_val>>13) & 0x3f) + 1;
    od = ((pll_cfg_val>>19) & 0xf) + 1; 
    
    cpu_pll_freq =  ((T_U64)nf * 24 * 1000000) / (nr * od) ;
    
    return cpu_pll_freq;
}





/**
* @BRIEF  clk_get_jclk_hclk_dclk_dphyclk_ddr2clk_freq
* @AUTHOR Zou Tianxiang
* @DATE   2021-11 -14
* @PARAM  
* @return PARAM  : T_pCPU_PLL_GROUP p_cpu_pll_group 
* @RETURN 
* @NOTE: 
*/
static void clk_get_jclk_hclk_dclk_dphyclk_ddr2clk_freq(T_pCPU_PLL_GROUP p_cpu_pll_group)
{
    T_U32 cpu_pll_freq;
    T_CLK_JCLK_DDR2CLK_MODE cpu_clk_mode;

    //�õ�CPU PLLƵ��
    cpu_pll_freq = clk_get_cpu_pll_freq();

    //�õ�CPU ģʽ
    cpu_clk_mode = clk_get_cpu_clk_mode();

    if( cpu_clk_mode == JCLK_DDR2CLK_2X1X_MODE)
    {
        p_cpu_pll_group->jclk = cpu_pll_freq;  
    }
    else if( cpu_clk_mode == JCLK_DDR2CLK_1X1X_MODE)
    {
        p_cpu_pll_group->jclk = cpu_pll_freq/2;  
    }

    p_cpu_pll_group->hclk = cpu_pll_freq/2;
    p_cpu_pll_group->dclk = cpu_pll_freq/2;
    p_cpu_pll_group->dphyclk = cpu_pll_freq;
    p_cpu_pll_group->ddr2clk = cpu_pll_freq/2;
}





/**
* @BRIEF �õ�JCLK��Ƶ��
* @AUTHOR Zou Tianxiang
* @DATE   2017-10 -4
* @CHANGE DATE   2021-11 -14 FOR H322
* @PARAM  
* @PARAM  
* @RETURN jclk frequence   unit: HZ
* @NOTE: 
*/
T_U32 clk_get_jclk_freq(void)
{
    T_CPU_PLL_GROUP cpu_pll_group;
    clk_get_jclk_hclk_dclk_dphyclk_ddr2clk_freq(&cpu_pll_group);
    return cpu_pll_group.jclk;
}




/**
* @BRIEF  �õ�hclk��Ƶ��
*              hclk ��  ARM Arbiter & AHB Asynchronous Bridge
* @AUTHOR Zou Tianxiang
* @DATE   2017-10 -4
* @CHANGE DATE   2021-11 -14 FOR H322
* @PARAM  
* @PARAM  
* @RETURN hclk frequence   unit: HZ
* @NOTE: 
*/
T_U32 clk_get_hclk_freq(void)
{
    T_CPU_PLL_GROUP cpu_pll_group;
    clk_get_jclk_hclk_dclk_dphyclk_ddr2clk_freq(&cpu_pll_group);
    return cpu_pll_group.hclk;
}



/**
* @BRIEF  �õ�DCLK��Ƶ�ʣ�ע���ⲻ����ΧDDR2����������Ƶ�ʣ�
*               ����DDR2�ڲ���������Ƶ��
* @AUTHOR Zou Tianxiang
* @DATE   2017-10 -4
* @CHANGE DATE   2021-11 -14 FOR H322
* @PARAM  
* @PARAM  
* @RETURN dclk frequence   unit: HZ
* @NOTE: 
*/
T_U32 clk_get_dclk_freq(void)
{
    T_CPU_PLL_GROUP cpu_pll_group;
    clk_get_jclk_hclk_dclk_dphyclk_ddr2clk_freq(&cpu_pll_group);
    return cpu_pll_group.dclk;
}




/**
* @BRIEF  �õ�DPHYCLK��Ƶ�������ڲ���ʱ��
* @AUTHOR Zou Tianxiang
* @DATE   2019-3 -18
* @CHANGE DATE   2021-11 -14 FOR H322
* @PARAM  
* @PARAM  
* @RETURN dphyclk frequence   unit: HZ
* @NOTE: 
*/
T_U32 clk_get_dphyclk_freq(void)
{
    T_CPU_PLL_GROUP cpu_pll_group;
    clk_get_jclk_hclk_dclk_dphyclk_ddr2clk_freq(&cpu_pll_group);
    return cpu_pll_group.dphyclk;
}



/**
* @BRIEF  �õ�ddr2 clk��Ƶ��, ����DDR2�����Ľӿ�ʱ��Ƶ��
* @AUTHOR Zou Tianxiang
* @DATE   2021-11 -14 
* @PARAM  
* @PARAM  
* @RETURN ddr2clk frequence   unit: HZ
* @NOTE: 
*/
T_U32 clk_get_ddr2clk_freq(void)
{
    T_CPU_PLL_GROUP cpu_pll_group;
    clk_get_jclk_hclk_dclk_dphyclk_ddr2clk_freq(&cpu_pll_group);
    return cpu_pll_group.ddr2clk;
}



#endif

/**
* @BRIEF  get core pll clk
* @AUTHOR Zou Tianxiang
* @DATE   2012-10 -24
* @CHANGE ON DATE   2021-2 -28 FOR H322
* @PARAM  
* @PARAM  
* @RETURN core pll frequence   unit: HZ
* @NOTE: 
*/
T_U32 ak_clk_get_core_pll_freq(void)
{
    T_U32 nf, nr, od;   
    T_U32 pll_cfg_val;
    T_U32 core_pll_freq;
    
    pll_cfg_val = inl(CORE_PLL_CFG_REG) ;
    nf = (pll_cfg_val & 0x1fff) + 1;
    nr = ((pll_cfg_val>>13) & 0x3f) + 1;
    od = ((pll_cfg_val>>19) & 0xf) + 1; 
    
    core_pll_freq =  ((T_U64)nf * 24 * 1000000) / (nr * od) ;
    
    return core_pll_freq;

}


/**
* @BRIEF  open peripheral module clk gate
* @AUTHOR Zou Tianxiang
* @DATE   2013-3 -2    
* @PARAM  T_CLK_PERI_TYPE_CLKGATE  PERI_TYPE
* @PARAM  
* @RETURN 
* @NOTE: 
*/
T_VOID clk_open_peripheral_module_gate(T_CLK_PERI_TYPE  peri_type )
{
    if( peri_type <= CLK_PDM)
    {
        REG32(CLOCK_GATE_REG) &= (~(1<<peri_type));
    }
    else if( (peri_type >= CLK_ISP) && (peri_type <= CLK_PRE_PROCESSOR))
    {
        REG32(ISP_CNN_VIDEO_PP_SRC_CLK_CFG_REG) |= (1 << ((peri_type-CLK_ISP)*8+6));
    }
    else if( (peri_type >= CLK_HASH) && (peri_type <= CLK_TRNG))
    {
        REG32(TRNG_HASH_CLOCK_GATE_REG) &= (~(1<<(peri_type-CLK_HASH)));
    }
    
}


/**
* @BRIEF  hold reset state on peripheral module
* @AUTHOR Zou Tianxiang
* @DATE   2013-3 -2
* @PARAM  T_CLK_PERI_TYPE_CLKGATE  PERI_TYPE
* @PARAM  
* @RETURN
* @NOTE: 
*/
T_VOID clk_hold_reset_peripheral_module( T_CLK_PERI_TYPE  peri_type )
{
    if( peri_type <= CLK_I2C2)
    {
        REG32(RESET_CTRL_REG) |= (1<<peri_type);
    }
    else if( (peri_type >= CLK_ISP) && (peri_type <= CLK_PRE_PROCESSOR))
    {
        REG32(ISP_CNN_VIDEO_PP_SRC_CLK_CFG_REG) |= (1 << ((peri_type-CLK_ISP)*8+7));
    }
    else if( (peri_type >= CLK_HASH) && (peri_type <= CLK_TRNG))
    {
        REG32(TRNG_HASH_RESET_CTRL_REG) |= (1<<(peri_type-CLK_HASH));
    }
}

/**
 * @BRIEF  clk_set_sfc_phyclk_div
 * @AUTHOR ZouTianxiang
 * @DATE   2024.5.15
 * @PARAM  div   range: 1 ~ 256
 * @NOTE         sfc_phyclk_div_num_cfg = div - 1
 *               sfc_phyclk = core_pll_clk / (sfc_phyclk_div_num_cfg + 1)
 */
T_VOID clk_set_sfc_phyclk_div(T_U32 div)
{
    T_U32 core_pll;
    T_U32 reg_val;

    core_pll = clk_get_core_pll_freq();

    if(div < 1 || div > 256)
    {
        printf("Error: sfc_phyclk_div range is from 1 to 256.\n");
        while(1);
    }
    
    //sfc_phyclk��signoffΪ200MHZ
    if((core_pll / div) > 200000000)
    {
        printf("Error: sfc_phyclk signoff is 200MHZ.now %dhz\n", (core_pll / div) );
        while(1);
    }

    //��sfc_phyclk
    reg_val = REG32(SFC_PHYCLK_CFG_REG);
    reg_val &= ~(0x3ff<<10);
    REG32(SFC_PHYCLK_CFG_REG) = reg_val;
    
    //HOLDסsfc_phyclk��λ
    REG32(RESET_CTRL_REG) |= (0x1<<31);
        
    //���÷�Ƶ
    REG32(SFC_PHYCLK_CFG_REG) |= (((div-1)<<10) | (0x1<<19));
    
    //�ȴ���Ƶ���
    while((REG32(SFC_PHYCLK_CFG_REG) & (0x1<<19)))
    {;}

    //��sfc_phyclk
    REG32(SFC_PHYCLK_CFG_REG) |= (0x1<<18);
    
    //�ſ�sfc_phyclk��λ
    REG32(RESET_CTRL_REG) &= ~(0x1<<31);


}



/**
* @BRIEF  release reset state on peripheral module
* @AUTHOR Zou Tianxiang
* @DATE   2013-3 -2
* @PARAM  T_CLK_PERI_TYPE_CLKGATE  PERI_TYPE
* @PARAM  
* @RETURN
* @NOTE: 
*/
T_VOID clk_release_reset_peripheral_module( T_CLK_PERI_TYPE  peri_type )
{
    if( peri_type <= CLK_I2C2)
    {
        REG32(RESET_CTRL_REG) &= (~(1<<peri_type));
    }
    else if( (peri_type >= CLK_ISP) && (peri_type <= CLK_PRE_PROCESSOR))
    {
        REG32(ISP_CNN_VIDEO_PP_SRC_CLK_CFG_REG) &= ~(1 << ((peri_type-CLK_ISP)*8+7));
    }
    else if( (peri_type >= CLK_HASH) && (peri_type <= CLK_TRNG))
    {
        REG32(TRNG_HASH_RESET_CTRL_REG) &= ~(1<<(peri_type-CLK_HASH));
    }      
}



/**
* @BRIEF  close peripheral module clk gate
* @AUTHOR Zou Tianxiang
* @DATE   2013-3 -2 
* @PARAM  T_CLK_PERI_TYPE_CLKGATE  PERI_TYPE
* @PARAM  
* @RETURN
* @NOTE: 
*/
T_VOID clk_close_peripheral_module_gate( T_CLK_PERI_TYPE  peri_type )
{
    if( peri_type <= CLK_I2C2)
    {
        REG32(CLOCK_GATE_REG) |= (1<<peri_type);
    }
    else if( (peri_type >= CLK_ISP) && (peri_type <= CLK_VIDEO_ENCODER))
    {
        REG32(ISP_CNN_VIDEO_PP_SRC_CLK_CFG_REG) &= ~(1 << ((peri_type-CLK_ISP)*8+6));
    }
    else if( (peri_type >= CLK_HASH) && (peri_type <= CLK_TRNG))
    {
        REG32(TRNG_HASH_CLOCK_GATE_REG) |= (1<<(peri_type-CLK_HASH));
    }    
}


#if 0


/**
* @BRIEF  clk_get_audio_pll_freq
* @AUTHOR Zou Tianxiang
* @DATE   2019-3 -26
* @CHANGE ON DATE   2021-2 -28 FOR H322
* @PARAM  
* @PARAM  
* @RETURN audio pll frequence   unit: HZ
* @NOTE: �õ�audio pll��Ƶ��
*/
T_U32 clk_get_audio_pll_freq(void)
{
     T_U32 nf, nr, od;
     T_U32 pll_cfg_val;
     T_U32 audio_pll_freq;
     
     pll_cfg_val = inl(AUDIO_PLL_CFG_REG) ;
     nf = (pll_cfg_val & 0x1fff) + 1;
     nr = ((pll_cfg_val>>13) & 0x3f) + 1;
     od = ((pll_cfg_val>>19) & 0xf) + 1; 
     
     audio_pll_freq =  ((T_U64)nf * 24 * 1000000) / (nr * od) ;
     
     return audio_pll_freq;
}





/**
* @BRIEF  clk_get_vclk_freq
* @AUTHOR Zou Tianxiang
* @DATE   2021-2 -28
* @PARAM  T_CLK_MODULE_IN_VCLK module_vclk
* @PARAM  
* @RETURN vclk frequence   unit: HZ
* @NOTE: 
*/
T_U32 clk_get_vclk_freq(T_CLK_MODULE_IN_VCLK module_vclk)
{
    //T_U32 cpu_pll_freq, core_pll_freq, audio_pll_freq;
    T_U32 src_clk_sel_cfg, src_clk_div_num_cfg;
    T_U32 vclk_src_pll;
    T_U32 vclk_freq;


#if 0    
    //�õ�CPU, CORE, AUDIO PLL��Ƶ��
    cpu_pll_freq = clk_get_cpu_pll_freq();
    core_pll_freq = clk_get_core_pll_freq();
    audio_pll_freq = clk_get_audio_pll_freq();
#endif  


    if(module_vclk == CNN_VCLK)
    {
        //�õ�cnn_src_clk_sel_cfg��cnn_src_clk_div_num_cfg
        src_clk_sel_cfg = ((REG32(ISP_CNN_VIDEO_PP_SRC_CLK_CFG_REG) >> 11) & 0x7);
        src_clk_div_num_cfg = ((REG32(ISP_CNN_VIDEO_PP_SRC_CLK_CFG_REG) >> 8) & 0x7);
        //�ж�CNNʱ���Ƿ��
        if((REG32(ISP_CNN_VIDEO_PP_SRC_CLK_CFG_REG) & (0x1<<14)) == 0)
        {
            printf("Note: CNN vclk not enable.\n");
            return 0;
        }
    }
    else if(module_vclk == ISP_VCLK)
    {
        //�õ�isp_src_clk_sel_cfg��isp_src_clk_div_num_cfg
        src_clk_sel_cfg = ((REG32(ISP_CNN_VIDEO_PP_SRC_CLK_CFG_REG) >> 3) & 0x7);
        src_clk_div_num_cfg = ((REG32(ISP_CNN_VIDEO_PP_SRC_CLK_CFG_REG) >> 0) & 0x7);

        //�ж�ISPʱ���Ƿ��
        if((REG32(ISP_CNN_VIDEO_PP_SRC_CLK_CFG_REG) & (0x1<<6)) == 0)
        {
            printf("Note: ISP vclk not enable.\n");
            return 0;
        }
    }
    else if(module_vclk == ENC_VCLK)
    {
        //�õ�enc_src_clk_sel_cfg��enc_src_clk_div_num_cfg
        src_clk_sel_cfg = ((REG32(ISP_CNN_VIDEO_PP_SRC_CLK_CFG_REG) >> 19) & 0x7);
        src_clk_div_num_cfg = ((REG32(ISP_CNN_VIDEO_PP_SRC_CLK_CFG_REG) >> 16) & 0x7);

        //�ж�ENCODERʱ���Ƿ��
        if((REG32(ISP_CNN_VIDEO_PP_SRC_CLK_CFG_REG) & (0x1<<22)) == 0)
        {
            printf("Note: VIDEO ENC vclk not enable.\n");
            return 0;
        }
    }
    else if(module_vclk == PP_VCLK)
    {
        //�õ�pp_src_clk_sel_cfg��pp_src_clk_div_num_cfg
        src_clk_sel_cfg = ((REG32(ISP_CNN_VIDEO_PP_SRC_CLK_CFG_REG) >> 27) & 0x7);
        src_clk_div_num_cfg = ((REG32(ISP_CNN_VIDEO_PP_SRC_CLK_CFG_REG) >> 24) & 0x7);
        
        //�ж�PPʱ���Ƿ��
        if((REG32(ISP_CNN_VIDEO_PP_SRC_CLK_CFG_REG) & (0x1<<30)) == 0)
        {
            printf("Note: PP vclk not enable.\n");
            return 0;
        }
        
    }

    
    //ȡ��2λ
    vclk_src_pll = (src_clk_sel_cfg & 0x3);
    
    //���src_clk_sel_cfg ��4���ϵ���ֱ��ΪPLL��ֵ
    if((src_clk_sel_cfg & (0x1<<2)) == (0x1<<2))
    {
        switch(vclk_src_pll)
        {
            case CPU_PLL:
                vclk_freq = clk_get_cpu_pll_freq();
                break;
            case CORE_PLL:
                vclk_freq = clk_get_core_pll_freq();
                break;
                
            case AUDIO_PLL:
                vclk_freq = clk_get_audio_pll_freq();
                break;              
                
            default:
                printf("Err vclk_src_pll1 = %x\n", vclk_src_pll);
                while(1);
                break;
        }
    }
    //��� ��4���µ���ȡ��Ƶֵ
    else
    {
        switch(vclk_src_pll)
        {
            case CPU_PLL:
                vclk_freq = clk_get_cpu_pll_freq()/(src_clk_div_num_cfg+1);
                break;
            case CORE_PLL:
                vclk_freq = clk_get_core_pll_freq()/(src_clk_div_num_cfg+1);
                break;
            case AUDIO_PLL:
                vclk_freq = clk_get_audio_pll_freq()/(src_clk_div_num_cfg+1);
                break;              
                
            default:
                printf("Err cnn_vclk_src_pll2 = %x\n", vclk_src_pll);
                break;
        }
        
    }
        

    return vclk_freq;
    
}





/**
* @BRIEF clk_get_gclk_freq
* @AUTHOR Zou Tianxiang
* @DATE   2012-10 -24
* @CHANGE ON DATE   2021-2 -28 FOR H322
* @PARAM  
* @PARAM  
* @RETURN gclk frequence   unit: HZ
* @NOTE: 
*/
T_U32 clk_get_gclk_freq(void)
{
    T_U32 core_pll_freq;
    T_U32 gclk_div_num_cfg;
    T_U32 gclk_freq;

    core_pll_freq = clk_get_core_pll_freq();

    //gclk divider
    gclk_div_num_cfg = (REG32(CORE_PLL_CFG_REG) >> 23) & 0x3;

    gclk_freq = core_pll_freq/(2*(gclk_div_num_cfg+1));
    
    return gclk_freq;
    
}




/**
* @BRIEF  get peripheral gclk frequence
* @AUTHOR Zou Tianxiang
* @DATE   2012-10 -24 
* @CHANGE ON DATE   2021-2 -28 FOR H322
* @PARAM  T_CLK_PERI_TYPE_CLKGATE  PERI_TYPE
* @PARAM  
* @RETURN peripheral gclk frequence   unit: HZ
* @NOTE: 
*/
T_U32 clk_get_peripheral_gclk_module_freq( T_CLK_PERI_TYPE  peri_type )
{
    //С�ڵ���CLK_PDM  ����GCLK��
    if(peri_type <= CLK_PDM)
    {
        if( (REG32(CLOCK_GATE_REG) & (1<<peri_type)) == 0 )
        {
            return clk_get_gclk_freq();
        }
        else
        {
            return 0;
        }
    }
    

    //CLK_DRAM ����dclk��
    else if(peri_type == CLK_DRAM)
    {
        if( (REG32(CLOCK_GATE_REG) & (1 << CLK_DRAM)) == 0 )
        {
            return clk_get_dclk_freq();
        }
        else
        {
            return 0;
        }
        
    }

    //CLK_DRAM_PHY ����dphyclk��
    else if(peri_type == CLK_DRAM_PHY)
    {
        if( (REG32(CLOCK_GATE_REG) & (1 << CLK_DRAM_PHY)) == 0 )
        {
            return clk_get_dphyclk_freq();
        }
        else
        {
            return 0;
        }
        
    }


    //��CLK_HASH    ��CLK_TRNG֮�������GCLK��
    else if( (peri_type >= CLK_HASH) && (peri_type <= CLK_TRNG) )
    {
        if( (REG32(TRNG_HASH_CLOCK_GATE_REG) & (1 << (peri_type-CLK_HASH)) ) == 0 )
        {
            return clk_get_gclk_freq();
        }
        else
        {
            return 0;
        }
    }

    
    //��CLK_ISP ��CLK_PRE_PROCESSOR֮�������VCLK��
    else if( (peri_type >= CLK_ISP) && (peri_type <= CLK_PRE_PROCESSOR) )
    {
        //VCLK ���µ�ISP, CNN, ENCODER ����1��Ч
        if( (inl(ISP_CNN_VIDEO_PP_SRC_CLK_CFG_REG) & (1 << ((peri_type-CLK_ISP)*8+6)) ) == 0 )
        {
            return 0;
        }
        else
        {
            return clk_get_vclk_freq(peri_type-CLK_ISP);
        }
    }


    else
    {
        printf("err input in clk_get_peripheral_gclk_module_freq.\n");
        while(1);
    }
    
    
        
}



/**
* @BRIEF  get sar adc clk frequence
* @AUTHOR Zou Tianxiang
* @DATE   2012-10 -24   
* @CHECK ON DATE   2021-2 -28 FOR H322
* @PARAM  
* @PARAM  
* @RETURN sar adc frequence  unit: HZ
* @NOTE: 
*/
T_U32 clk_get_sar_adc_clk_freq()
{
    T_U32 sar_adc_val;
    T_U32 sar_adc_div;
    T_U32 sar_adc_freq;

    sar_adc_val = inl(SAR_ADC_CLK_REG);
    sar_adc_div = sar_adc_val & 0x7 ;

    //���û�д�SAR ADC��clk�Ļ�������0
    if( (sar_adc_val & (1<<3) ) == 0)
    {
        printf("not open sar adc clk gate.\n");
        return 0;
    }
    
    sar_adc_freq = 24000000/(sar_adc_div+1) ;
    return sar_adc_freq;
}



/**
* @BRIEF  clk_get_sddac_clk_freq  (AUDIO PLL�µ�SDDAC CLK)
* @AUTHOR Zou Tianxiang
* @DATE   2017-10 -4
* @CHANGE ON DATE   2021-2 -28 FOR H322
* @PARAM  
* @PARAM  
* @RETURN audio dac frequence  unit: HZ
* @NOTE: 
*/
T_U32 clk_get_sddac_clk_freq()
{
    T_U32 reg_val;
    T_U32 audio_dac_div;
    T_U32 audio_dac_freq;

    reg_val = inl(AUDIO_DAC_CLK_CFG_REG);
    if( (reg_val & (1<<28) )  == 0)
    {
        return 0;
    }
    
    audio_dac_div  = (reg_val >> 20) & 0xff;
    
    audio_dac_freq = (clk_get_audio_pll_freq() / (audio_dac_div+1));
        
    return audio_dac_freq;


}




/**
* @BRIEF  clk_get_sdadc_clk_freq(AUDIO PLL�µ�SDADC CLK)
* @AUTHOR Zou Tianxiang
* @DATE   2012-10 -24
* @CHANGE ON DATE   2021-2 -28 FOR H322
* @PARAM  
* @PARAM  
* @RETURN audio adc frequence  unit: HZ
* @NOTE: 
*/
T_U32 clk_get_sdadc_clk_freq()
{
    T_U32 reg_val;
    T_U32 audio_adc_div;
    T_U32 core_pll_freq;
    T_U32 audio_adc_freq;

    reg_val = inl(AD_DA_HCLK_REG);
    if( (reg_val & (1<<8) ) == 0)
    {
        return 0;
    }
    
    audio_adc_div  = reg_val  & 0xff;
    
    audio_adc_freq = (clk_get_audio_pll_freq() / (audio_adc_div+1));
        
    return audio_adc_freq;
    
}




/**
* @BRIEF  clk_get_sddac_hsclk_freq
*              dac_hsclk support  sddac filter    (AUDIO PLL�µ�SDDAC HSCLK)
* @AUTHOR Zou Tianxiang
* @DATE   2012-10 -24
* @CHECK ON DATE   2021-2 -28 FOR H322
* @PARAM  
* @PARAM  
* @RETURN audio dac hsclk frequence  unit: HZ
* @NOTE: 
*/
T_U32 clk_get_sddac_hsclk_freq()
{
    T_U32 reg_val;
    T_U32 audio_dac_hsclk_div;
    T_U32 audio_pll_freq;
    T_U32 audio_dac_hsclk_freq;

    reg_val = inl(AD_DA_HCLK_REG);
    if( (reg_val & (1<<18) )  == 0)
    {
        return 0;
    }
    
    audio_dac_hsclk_div  = (reg_val >> 10) & 0xff;
    
    audio_pll_freq = clk_get_audio_pll_freq();

    audio_dac_hsclk_freq = audio_pll_freq / (audio_dac_hsclk_div+1);
        
    return audio_dac_hsclk_freq;
}





/**
* @BRIEF  clk_get_sdadc_hsclk_freq
*              adc_hsclk support  sdadc filter    
* @AUTHOR Zou Tianxiang
* @DATE   2012-10 -24
* @CHECK ON DATE   2021-2 -28 FOR H322
* @PARAM  
* @PARAM  
* @RETURN audio adc hsclk frequence  unit: HZ
* @NOTE: 
*/
T_U32 clk_get_sdadc_hsclk_freq()
{
    T_U32 audio_adc_hsclk_val;
    T_U32 audio_adc_hsclk_div;
    T_U32 audio_pll_freq;
    T_U32 audio_adc_hsclk_freq;

    audio_adc_hsclk_val = inl(AD_DA_HCLK_REG);
    if( (audio_adc_hsclk_val & (1<<28) ) == 0)
    {
        return 0;
    }
    
    audio_adc_hsclk_div  = (audio_adc_hsclk_val>>20)  & 0xff;
    
    audio_pll_freq = clk_get_audio_pll_freq();

    audio_adc_hsclk_freq = audio_pll_freq / (audio_adc_hsclk_div+1);
        
    return audio_adc_hsclk_freq;
}





/**
* @BRIEF  clk_get_mac_opclk_freq
* @AUTHOR Zou Tianxiang
* @DATE   2024-5 -12
* @PARAM  
* @PARAM  
* @RETURN opclk frequence  unit: HZ
* @NOTE: 
*/
T_U32 clk_get_mac_opclk_freq()
{
    T_U32 reg_val;
    T_U32 opclk_div;
    T_U32 opclk_freq;
    T_U32 src_clk_sel_cfg;

    reg_val = inl(MAC_AND_CSI_TOP_CFG1_REG);
    if( (reg_val & (1<<8) ) == 0)
    {
        printf("opclk is close.\n");
        return 0;
    }

    if( (reg_val & (1<<9) ) == 0)
    {
        printf("opclk src is close.\n");
        return 0;
    }

    //�õ�opclk pllԴ
    src_clk_sel_cfg = ((reg_val >> 6) & 0x3);
    //�õ���Ƶֵ
    opclk_div  = reg_val & 0x3f;

    switch(src_clk_sel_cfg)
    {
        case CPU_PLL:
            opclk_freq =  clk_get_cpu_pll_freq() / (opclk_div+1);
            break;
            
        case CORE_PLL:
            opclk_freq =  clk_get_core_pll_freq() / (opclk_div+1);
            break;
            
        case AUDIO_PLL:
            opclk_freq =  clk_get_audio_pll_freq() / (opclk_div+1);
            break;
            
        //ʹ��USB PHY��400MHZ ʱ��
        case USB_PHY_PLL:
            //���USB�ڸ�λ״̬����ֱ�ӷ���0
            if((REG32(USB_PHY_CFG_REG) & 0x1) == 0x1)
            {
                printf("USB PHY is in reset stat.\n");
                return 0;
            }
			//���û�д�XCFGI0 BIT31, ��ֱ�ӷ���0
            if((REG32(USB_PHY_XCFGI0_REG) & (0x1<<31)) == 0)
            {
                printf("USB PHY xcfgi_usb_phy[31] = 0.\n");
                return 0;
            }
            
            opclk_freq =  400000000 / (opclk_div+1);
            break;

        default:
            printf("clk_get_mac_opclk_freq err.\n");
            while(1);
            break;
    }

    return opclk_freq;
    
}



/**
* @BRIEF  clk_get_csi_sclk_freq
* @AUTHOR Zou Tianxiang
* @DATE   2021-2 -28
* @PARAM  
* @PARAM  
* @RETURN camera sclk frequence  unit: HZ
* @NOTE: 
*/
T_U32 clk_get_csi_sclk_freq(T_CLK_CSI_SCLK_INDEX csi_sclk_index)
{
    T_U32 cpu_pll_freq, core_pll_freq, audio_pll_freq;
    T_CLK_CSI_SCLK_SRC csi_sclk_src;

    T_U32 csi_src_pll_sel;    
    T_U32 csi_src_clk_div;
    T_U32 csi_src_ena;
    T_U32 csi_sclk_freq;

    
    //ȷ��CSI_SCLK�Ƿ�OPEN , ���û�д���ֱ�ӷ���0
    if((REG32(SENSOR_CLK_CFG_REG)>>(csi_sclk_index*4 + 2)) == 0x0)
    {
        printf("CSI_SCLK is close.\n");
        return 0;
    }

    //�õ�csi_sclk_index ��Դʱ������
    csi_sclk_src = (REG32(SENSOR_CLK_CFG_REG)>>(csi_sclk_index*4)) & 0x3;

    //�Ƿ�ѡ��24MHZ
    if(csi_sclk_src == CSI_SCLK_USE_24MHZ)
    {
        return CRYSTAL_FREQUENCE;
    }

    else if(csi_sclk_src == CSI_SCLK_USE_CSI_SRC0_CLK)
    {
        //�õ�pllԴ
        csi_src_pll_sel = (REG32(MAC_AND_CSI_TOP_CFG1_REG)>>16) & 0x7;
        //�õ���Ƶ
        csi_src_clk_div = (REG32(MAC_AND_CSI_TOP_CFG1_REG)>>10) & 0x3f;
        //�õ�ʹ��
        csi_src_ena = (REG32(MAC_AND_CSI_TOP_CFG1_REG)>>19) & 0x1;
    }
    else if(csi_sclk_src == CSI_SCLK_USE_CSI_SRC1_CLK)
    {
        //�õ�pllԴ    
        csi_src_pll_sel = (REG32(MAC_AND_CSI_TOP_CFG1_REG)>>26) & 0x7;;
        //�õ���Ƶ        
        csi_src_clk_div = (REG32(MAC_AND_CSI_TOP_CFG1_REG)>>20) & 0x3f;
        //�õ�ʹ��        
        csi_src_ena = (REG32(MAC_AND_CSI_TOP_CFG1_REG)>>29) & 0x1;
    }
    else
    {
        printf("Err: in clk_get_csi_sclk_freq 1.\n");
    }


    //��ʹ��û�д򿪣���ֱ�ӷ���0
    if(csi_src_ena == 0)
    {
        printf("csi_src is close.\n");
        return 0;
    }


        //����Դ����
    switch(csi_src_pll_sel)
    {
        case CPU_PLL:
            csi_sclk_freq = clk_get_cpu_pll_freq()/(csi_src_clk_div+1);
            break;
        case CORE_PLL:
            csi_sclk_freq = clk_get_core_pll_freq()/(csi_src_clk_div+1);
            break;
        case AUDIO_PLL:
            csi_sclk_freq = clk_get_audio_pll_freq()/(csi_src_clk_div+1);
            break;
        default:
            printf("Err: in clk_get_csi_sclk_freq 2.\n");
            while(1);
            break;
    }
    
    return csi_sclk_freq;

}






/**
* @BRIEF  get otg phy frequence
* @AUTHOR Zou Tianxiang
* @DATE   2012-10 -24
* @CHECK ON DATE   2021-2 -28 FOR H322
* @PARAM  
* @PARAM  
* @RETURN otg phy frequence  unit: HZ
* @NOTE: 
*/
T_U32 clk_get_otg_phy_freq()
{
    return 12000000;
}


/**
 * @BRIEF  get mipi dphy_cfgclk frequence
 * @AUTHOR Zheng Jiansheng
 * @DATE   2021.9.22
 * @RETURN mipi dphy_cfgclk frequence(unit: Hz)
 * @NOTE   dphy_cfgclk_div_num_cfg = div - 1
 *         mipi_dphy_cfgclk = core_pll_clk / (dphy_cfgclk_div_num_cfg + 1)
 */
T_U32 clk_get_mipi_dphy_clk_freq()
{
    T_U32 reg_val;
    T_U32 mipi_dphy_cfgclk;
    T_U32 mipi_dphy_cfgclk_div;

    reg_val = REG32(MIPI_DPHY_CLK_REG);
    
    // if ture, mipi dphy_cfgclk is disabled.
    if((reg_val & (1 << 8)) == 0)
    {
        return 0;
    }

    mipi_dphy_cfgclk_div = reg_val >> 4 & 0xF;

    mipi_dphy_cfgclk = clk_get_core_pll_freq() / (mipi_dphy_cfgclk_div + 1);

    return mipi_dphy_cfgclk;
}




/**
* @BRIEF  calculate the cpu & core pll NF NR OD
* @AUTHOR Zou Tianxiang
* @DATE   2021-2 -28   
* @PARAM  freq_mhz
* @PARAM  T_CLK_PLL_CLOCK_PARA
* @PARAM  T_CLK_PLL_TYPE pll_type
* @RETURN   AK_TRUE
*                  AK_FALSE
* @COMMENT  �����մ����freq�Զ�����NF , NR , OD.
* @NOTE: 
*/
static T_BOOL clk_cpu_core_pll_calc_NF_NR_OD(T_U32 freq_mhz,  T_CLK_PLL_CLOCK_PARA *p_pll_clk_para, T_CLK_PLL_TYPE pll_type)
{
    T_U32 i, j;
    T_U32 freq_mul_od, nf_val, nf_remain;
    
    for(i = 0; i < sizeof(OD_SET)/4; i++)
    {

        freq_mul_od = (freq_mhz * OD_SET[i] );

        // result  must   500MHZ <= freq_mul_od <= 2.5GHZ
        // mean:   500MHZ <= 24*NF/NR<= 2.5GHZ
        if ( (freq_mul_od < 500) || (freq_mul_od > 2500) )
        {
            continue;
        }
        for(j = 1; j <= NR_MAX_VAL; j++)
        {
            //calculate the m value could divider by 24
            nf_remain = (freq_mul_od * j ) % 24;
            if( nf_remain == 0)
            {
                nf_val = (freq_mul_od * j ) / 24;

                if( (nf_val >= 1) && (nf_val <= NF_MAX_VAL))
                {
                    p_pll_clk_para->NF = nf_val;
                    p_pll_clk_para->NR = j; 
                    p_pll_clk_para->OD = OD_SET[i];
                    
                    return AK_TRUE;
                }
            }
        }
    }

    return AK_FALSE;
    
}










/**
* @BRIEF  calculate the audio pll NF , NR, OD
* @AUTHOR Zou Tianxiang
* @DATE   2021-2 -28   
* @PARAM  freq  ��λhz
* @PARAM  T_CLK_AUDIO_PLL_CLOCK_PARA
* @RETURN   AK_TRUE: 
*                  AK_FALSE: 
* @COMMENT  �����մ����freq�Զ�����NF , NR, OD
* @NOTE: 
*/
static T_U32 clk_audio_pll_calc_NF_NR_OD(T_U32 freq,  T_CLK_AUDIO_PLL_CLOCK_PARA *p_audio_pll_clk_para)
{
    T_U32 i, j;
    T_U64 freq_mul_od;
    T_U64 nf_val, nf_remain;
    //T_U32 *p_freq_mul_od = &freq_mul_od;

    for(i = 0; i < sizeof(OD_SET)/4; i++)
    {

        freq_mul_od = (T_U64)freq * OD_SET[i];

        // result  must   500MHZ <= freq_mul_od <= 2.5GHZ
        // mean:   500MHZ <= 24*NF/NR<= 2.5GHZ
        if ( (freq_mul_od < 500000000) || (freq_mul_od > 2500000000) )
        {
            continue;
        }
        for(j = 1; j <= NR_MAX_VAL; j++)
        {
            //calculate the m value could divider by 24
            nf_remain = (freq_mul_od * j ) % 24;
            if( nf_remain == 0)
            {
                nf_val = (freq_mul_od * j ) / 24;
                if((nf_val % 1000000) != 0)
                {
                    continue;
                }
                else
                {
                    nf_val = nf_val/1000000;
                }

                if( (nf_val >= 1) && (nf_val <= NF_MAX_VAL))
                {
                    p_audio_pll_clk_para->NF = nf_val;
                    p_audio_pll_clk_para->NR = j; 
                    p_audio_pll_clk_para->OD = OD_SET[i];
                    printf("NF = %d\n", p_audio_pll_clk_para->NF);
                    printf("NR = %d\n", p_audio_pll_clk_para->NR);
                    printf("OD = %d\n", p_audio_pll_clk_para->OD);
                    return AK_TRUE;
                }
            }
        }
    }

    return AK_FALSE;
}







/**
* @BRIEF  clk_check_pll_param_right
* @AUTHOR Zou Tianxiang
* @DATE   2021-3 -1
* @PARAM  
* @PARAM  T_CLK_PLL_CLOCK_PARA
* @RETURN   AK_TRUE   
*                  AK_FALSE  
* @NOTE:   ȷ��p_pll_clk_para �е�NF, NR , OD.  �����Ƿ��д�
*/
static T_BOOL clk_check_pll_param_right(T_CLK_PLL_CLOCK_PARA *p_pll_clk_para)
{
    T_U32 i;
    T_BOOL od_right;
    T_U64  pll_voc_freq;

    //NF�Ƿ񳬳����ֵ
    if(p_pll_clk_para->NF > NF_MAX_VAL)
    {
        printf("Err: NF bigger than NF_MAX_VAL\n");
        return AK_FALSE;
    }
    
    //NR�Ƿ񳬳����ֵ
    if(p_pll_clk_para->NR > NR_MAX_VAL) 
    {
        printf("Err: NR bigger than NR_MAX_VAL\n");
        return AK_FALSE;        
    }

    //OD ֵ�Ƿ���ȷ
    od_right = AK_FALSE;
    for(i = 0; i < sizeof(OD_SET)/4; i++)
    {
        if(p_pll_clk_para->OD == OD_SET[i])
        {
            od_right = AK_TRUE;
            break;
        }
    }
    if(od_right == AK_FALSE)
    {
        printf("Err: OD VAL = %d, Not in OD range.\n", p_pll_clk_para->OD);
        return AK_FALSE;        
    }


    //�ж�24*NF/NR �Ƿ���500~2500MHZ��Χ��
    pll_voc_freq = (T_U64)24000000 * p_pll_clk_para->NF / p_pll_clk_para->NR;
    if ( (pll_voc_freq < 500000000) || (pll_voc_freq > 2500000000) )
    {
        printf("Err: 24*NF/NR = %d, Not in range[500000000, 2500000000] .\n", pll_voc_freq);
        return AK_FALSE;
    }


    return AK_TRUE;

}





/**
* @BRIEF  set cpu & core  pll frequence
* @AUTHOR Zou Tianxiang
* @DATE   2012-10 -26 
* @CHANGE ON DATE   2021-2 -28 FOR H322
* @PARAM  freq        unit: HZ
* @PARAM  T_CLK_PLL_CLOCK_PARA
* @RETURN   AK_TRUE:   Set pll frequence success 
*                  AK_FALSE:  Set pll frequence false 
* @COMMENT  ��������freq Ϊ0����������p_pll_clk_para ֱ�ӵõ�NF, NR , OD.  
*                   ��������freq��0, �� �����մ����freq�Զ�����NF, NR , OD.  
*                   ���ʹ���Զ�����NF, NR , OD.  �������freq ��ע��MHZ����
*           
* @NOTE:  ������ֻ��� CPU, CORE PLL
*/
static T_BOOL clk_set_cpu_core_pll_freq(T_U32 freq,  T_CLK_PLL_CLOCK_PARA *p_pll_clk_para,  T_CLK_PLL_TYPE pll_type)
{
    T_U32 i;
    T_U32 pll_reg_addr;
    T_U32 freq_mhz;
    T_U32 refresh_val;
    T_U32 powerof_od = 1;
    T_U32 expect_ddr2clk;
    T_CLK_PLL_CLOCK_PARA pll_para;
    T_U32 cpu_now_clk_mode;
    
    void (*p_change_freq)();
    
    if( freq == 0 )
    {
        pll_para.NF = p_pll_clk_para->NF;
        pll_para.NR = p_pll_clk_para->NR;
        pll_para.OD = p_pll_clk_para->OD;
        //ȷ�ϴ���Ĳ����Ƿ���ȷ
        if( clk_check_pll_param_right(&pll_para) == AK_FALSE)
        {
            printf("check pll param fail in clk_set_cpu_core_pll_freq.\n");
            return AK_FALSE;
        }
        
        freq_mhz = (24 * pll_para.NF) / (pll_para.OD * pll_para.NR);
        //printf("freq_mhz = %d\n",freq_mhz);
    }
    else
    {
        if( (freq < CPU_CORE_PLL_MIN_FREQ) || (freq > MAX_PLL_FREQ_SET[pll_type]) )
        {
            printf("Err: pll_clk = %d,  must bigger than %dhz and smaller than %dhz!\n", freq, CPU_CORE_PLL_MIN_FREQ, MAX_PLL_FREQ_SET[pll_type]);
            while(1);
            return AK_FALSE;
        }
    
        if( (freq % 1000000) != 0 )
        {
            printf("This function cannot support MHZ not align!\n");
            return AK_FALSE;
        }

        freq_mhz = freq / 1000000;
        if( clk_cpu_core_pll_calc_NF_NR_OD( freq_mhz, &pll_para, pll_type) == AK_FALSE)
        {
            printf("calculate NF, NR, OD failed!\n");
            return AK_FALSE;
        }
        else
        {
            printf("NF = %d\n", pll_para.NF);
            printf("NR = %d\n", pll_para.NR);
            printf("OD = %d\n", pll_para.OD);
            freq_mhz =  (pll_para.NF * 24) / (pll_para.NR * pll_para.OD);
            printf("real destinate pll = %d\n", freq_mhz);
        }
    }
    

    switch(pll_type)
    {
        case CPU_PLL:

            //�õ���ǰCPU CLKģʽ
            cpu_now_clk_mode = clk_get_cpu_clk_mode();
            
            //�õ�DDR2���������¼��������Ƶ��
            expect_ddr2clk = clk_get_expect_ddr2clk(freq_mhz);
            //printf("expect_ddr2clk = %d mhz\n", expect_ddr2clk);
            
            
            //�õ���Ӧ��ˢ��ֵ
            refresh_val = 7600/(1000/expect_ddr2clk);
            //printf("refresh_val = %d\n", refresh_val);
                
            // wait pll adjust finish
            while( (inl(CPU_PLL_CFG_REG)  & (1<<30)) !=0)
            {;}
            
            // write NF, NR, OD
            REG32(CPU_PLL_CFG_REG) &= (~0x7fffff);
            REG32(CPU_PLL_CFG_REG) |= ((pll_para.NF-1) | ((pll_para.NR-1)<<13) | ((pll_para.OD - 1)<<19)) ;
            REG32(CPU_PLL_BWADJ_CFG_REG) &= ~(0xfff<<20);
            REG32(CPU_PLL_BWADJ_CFG_REG) |= ((pll_para.NF/2 - 1)<<20);
            if(freq_mhz != 0)   
            {
                //copy the code to interram
                clk_copy_data_to_interram(change_cpu_pll_freq_code, sizeof(change_cpu_pll_freq_code));
                //��2���������ڹ̶��ĵ�ַ: refresh_val����0x48000400��ʼ λ��
                //0x48000400 : �洢ˢ��ֵ
                //0x48000404 : �洢DRAM У׼���ڵĵ�ַλ��
                REG32(L2_BUF_MEM_BASE_ADDR + sizeof(change_cpu_pll_freq_code) + 0x00) = refresh_val;                        //ˢ������
                REG32(L2_BUF_MEM_BASE_ADDR + sizeof(change_cpu_pll_freq_code) + 0x04) = DRAM_CALIBRATION_ADDRESS;
                p_change_freq = (void(*)())(L2_BUF_MEM_BASE_ADDR);
                p_change_freq();
            }
            else
            {
                printf("freq_mhz error!\n");
                while(1);
            }
            break;
            
        case CORE_PLL:
            // wait pll adjust finish
            while( (inl(CPU_PLL_CFG_REG)  & (1<<28)) !=0)
            {;}
            
            // write NF, NR, OD
            REG32(CORE_PLL_CFG_REG) &= (~0x7fffff);
            REG32(CORE_PLL_CFG_REG) |= ((pll_para.NF-1) | ((pll_para.NR-1)<<13) | ((pll_para.OD - 1)<<19)) ;
            REG32(CORE_AND_AUDIO_PLL_BWADJ_REG) &= (~0xfff);
            REG32(CORE_AND_AUDIO_PLL_BWADJ_REG) |= (pll_para.NF/2 - 1);
            //REG32(CORE_AND_AUDIO_PLL_BWADJ_REG) |= (pll_para.NF/2/4 - 1);  //SKY��������������п��ܻ����
            REG32(CPU_PLL_CFG_REG) |= (1<<28);
            
            // wait pll adjust finish
            while( (inl(CPU_PLL_CFG_REG)  & (1<<28)) !=0)
            {;}
            break;

        default:
            break;          
    }

    return AK_TRUE; 
    
}
        




/**
* @BRIEF  set cpu pll frequence auto
* @AUTHOR Zou Tianxiang
* @DATE   2012-10 -26     2021-3-1   CHECK PASS
* @PARAM  freq        unit: HZ
* @PARAM  
* @RETURN   AK_TRUE:   Set cpu pll frequence success 
*                  AK_FALSE:  Set cpu pll frequence false 
* @COMMENT �����մ����freq�Զ�����M , N , OD.    �����������MHZ������
*           
* @NOTE: 
*/
T_BOOL clk_set_cpu_pll_freq_auto(T_U32 freq)
{
    return clk_set_cpu_core_pll_freq(freq, AK_NULL, CPU_PLL);
}   



/**
* @BRIEF  set cpu pll frequence manual
* @AUTHOR Zou Tianxiang
* @DATE   2012-10 -26       2021-3-1   CHECK PASS
* @PARAM  T_CLK_PLL_CLOCK_PARA
* @PARAM  
* @RETURN   AK_TRUE:   Set cpu pll frequence success 
*                  AK_FALSE:  Set cpu pll frequence false 
* @COMMENT    ��������û������p_pll_clk_para ֱ������Ƶ�� 
*           
* @NOTE: 
*/
T_BOOL clk_set_cpu_pll_freq_manual(T_CLK_PLL_CLOCK_PARA *p_pll_clk_para )
{
    return clk_set_cpu_core_pll_freq(0,  p_pll_clk_para, CPU_PLL);
}





/**
* @BRIEF  set core pll frequence auto 
* @AUTHOR Zou Tianxiang
* @DATE   2012-10 -26      2021-3-1   CHECK PASS
* @PARAM  freq        unit: HZ
* @PARAM  
* @RETURN   AK_TRUE:   Set core pll frequence success 
*                  AK_FALSE:  Set core pll frequence false 
* @COMMENT  �����մ����freq�Զ�����M , N , OD.    �����������MHZ������
*
* @NOTE: 
*/
T_BOOL clk_set_core_pll_freq_auto(T_U32 freq)
{
    return clk_set_cpu_core_pll_freq(freq, AK_NULL, CORE_PLL);
}




/**
* @BRIEF  set core pll frequence manual
* @AUTHOR Zou Tianxiang
* @DATE   2012-10 -26     2021-3-1   CHECK PASS
* @PARAM  T_CLK_PLL_CLOCK_PARA
* @PARAM  
* @RETURN   AK_TRUE:   Set core pll frequence success 
*                  AK_FALSE:  Set core pll frequence false 
* @COMMENT    ��������û������p_pll_clk_para ֱ������Ƶ�� 
*           
* @NOTE: 
*/
T_BOOL clk_set_core_pll_freq_manual(T_CLK_PLL_CLOCK_PARA *p_pll_clk_para )
{
    return clk_set_cpu_core_pll_freq(0,  p_pll_clk_para, CORE_PLL); 
}





/**
* @BRIEF  set pll frequence
* @AUTHOR Zou Tianxiang
* @DATE   2019-3-18 
* @CHANGE DATE   2021-3-1 FOR H322 
* @PARAM  freq        unit: HZ
* @PARAM  T_CLK_AUDIO_PLL_CLOCK_PARA
* @RETURN   AK_TRUE
*                  AK_FALSE 
* @NOTE: ��������freq Ϊ0����������p_audio_pll_clk_para ֱ�ӵõ�NF, NR, OD 
*               ��������freq��0, �� �����մ����freq�Զ�����NF, NR, OD ��
*/
static T_U32 clk_set_audio_pll_freq(T_U32 freq,  T_CLK_AUDIO_PLL_CLOCK_PARA *p_audio_pll_clk_para)
{
    T_U32 real_freq;
    T_U32 reg_val;
    T_CLK_AUDIO_PLL_CLOCK_PARA pll_para;
    

    //freq = 0,   ���ʾ���մ����p_audio_pll_clk_para����audio pll Ƶ������
    if( freq == 0 )
    {
        pll_para = *p_audio_pll_clk_para;
        
        //ȷ�ϴ���Ĳ����Ƿ���ȷ
        if( clk_check_pll_param_right(&pll_para) == AK_FALSE)
        {
            printf("check pll param fail in clk_set_audio_pll_freq.\n");
            return AK_FALSE;
        }
        
    }
    else
    {
        //�����Ҫ���õ�ֵ��AUDIO PLL�ķ�Χ����ֱ��return false
        if( (freq < AUDIO_PLL_MIN_FREQ) || (freq > AUDIO_PLL_MAX_FREQ) )
        {
            printf("Err: Audio pll clk must > %dhz and < %dhz !\n", 
                    AUDIO_PLL_MIN_FREQ,  AUDIO_PLL_MAX_FREQ);
            while(1);
            return AK_FALSE;
        }

        //���ݴ����Ƶ�ʼ���N,  Mֵ
        if( clk_audio_pll_calc_NF_NR_OD(freq, &pll_para) == AK_FALSE)
        {
            printf("Err: audio calculate NF,NR,OD failed!\n");
            return AK_FALSE;
        }
    }


    // wait pll adjust finish
    while( (inl(CPU_PLL_CFG_REG)  & (1<<29)) !=0)
    {;}
            
    // write NF, NR, OD
    REG32(AUDIO_PLL_CFG_REG) &= (~0x7fffff);
    REG32(AUDIO_PLL_CFG_REG) |= ((pll_para.NF-1) | ((pll_para.NR-1)<<13) | ((pll_para.OD - 1)<<19)) ;
    REG32(CORE_AND_AUDIO_PLL_BWADJ_REG) &= (~(0xfff<<12));
    REG32(CORE_AND_AUDIO_PLL_BWADJ_REG) |= ((pll_para.NF/2 - 1)<<12);
    REG32(CPU_PLL_CFG_REG) |= (1<<29);
    
    // wait pll adjust finish
    while( (inl(CPU_PLL_CFG_REG)  & (1<<29)) !=0)
    {;}

    //��ӡ������ʵֵ
    real_freq = clk_get_audio_pll_freq();
    printf("real audio freq = %d\n", real_freq);


    return AK_TRUE; 
    
}




/**
* @BRIEF  ���ݴ����M,Nֱֵ������AUDIO PLL
* @AUTHOR Zou Tianxiang
* @DATE   2019-3 -29 
* @CHANGE ON DATE   2021-3 -1  FOR H322
* @PARAM  T_CLK_AUDIO_PLL_CLOCK_PARA *p_audio_pll_clk_para
* @PARAM  
* @RETURN   AK_TRUE
*                  AK_FALSE
* @NOTE: ��������û������p_audio_pll_clk_para ֱ������Ƶ�� 
*/
T_U32 clk_set_audio_pll_freq_manual(T_CLK_AUDIO_PLL_CLOCK_PARA *p_audio_pll_clk_para)
{
    return clk_set_audio_pll_freq(0,  p_audio_pll_clk_para);
}





/**
* @BRIEF  ���ݴ����Ƶ���Զ�����AUDIO PLL
* @AUTHOR Zou Tianxiang
* @DATE   2019-3 -29 
* @CHANGE ON DATE   2021-3 -1  FOR H322
* @PARAM  freq        unit: HZ
* @PARAM  
* @RETURN   AK_TRUE
*                  AK_FALSE
* @NOTE:  �����մ����freq�Զ�����M , N 
*/
T_U32 clk_set_audio_pll_freq_auto(T_U32 freq)
{
    return clk_set_audio_pll_freq(freq,  AK_NULL);
}





/**
* @BRIEF  �����Ƶ��DDR2  ddr2clk ���ܴﵽ��Ƶ��(�����������⹫��)
* @AUTHOR Zou Tianxiang
* @DATE   2021-11 -14
* @PARAM  expect_cpu_pll_clk
* @PARAM  expect_cpu_clk_mode
* @RETURN    ��PLL���ddr2clk Ƶ��
* @NOTE: ���������Ҫ���ڼ����Ƶ��DDR2��ˢ����
*/
static T_U32 clk_get_expect_ddr2clk(T_U32 expect_cpu_pll_clk)
{
    return (expect_cpu_pll_clk/2);  
}


/**
 * @BRIEF  set mipi dphy_cfgclk divider
 * @AUTHOR Zheng Jiansheng
 * @DATE   2021.9.22
 * @PARAM  div   range: 1 ~ 16
 * @NOTE         dphy_cfgclk_div_num_cfg = div - 1
 *               mipi_dphy_cfgclk = core_pll_clk / (dphy_cfgclk_div_num_cfg + 1)
 */
T_VOID clk_set_mipi_dphy_clk_div(T_U32 div)
{
    T_U32 core_pll;

    core_pll = clk_get_core_pll_freq();

    if(div < 1 || div > 16)
    {
        printf("Error: The mipi_dhpy_clk divider range is from 1 to 16.\n");
        while(1);
    }
    if((core_pll / div) < 80000000 || (core_pll / div) > 120000000)
    {
        printf("Error: The mipi_dhpy_clk frequency range is from 80MHz to 120MHz.\n");
        while(1);
    }

    // enable mipi dphy_cfgclk.
    REG32(MIPI_DPHY_CLK_REG) |= (1 << 8);
    
    // wait for mipi dphy_cfgclk divider adjustment is completed.
    while( (REG32(MIPI_DPHY_CLK_REG) & (1 << 9)) != 0)
    {;}

    // enable mipi dphy_cfgclk.
    REG32(MIPI_DPHY_CLK_REG) |= (1 << 8);

    REG32(MIPI_DPHY_CLK_REG) &= ~(0xF << 4);
    REG32(MIPI_DPHY_CLK_REG) |= ((div - 1) << 4);

    // start mipi dphy_cfgclk  divider adjustment.
    REG32(MIPI_DPHY_CLK_REG) |= (1 << 9);

    // wait for mipi dphy_cfgclk divider adjustment is completed.
    while( (REG32(MIPI_DPHY_CLK_REG) & (1 << 9)) != 0)
    {;}
}












/**
* @BRIEF  �õ���ǰCPUʱ�ӵĹ���ģʽ
* @AUTHOR Zou Tianxiang
* @DATE   2019-3 -20
* @CHANGE ON DATE   2021-2 -28 FOR H322
* @PARAM  
* @PARAM  
* @RETURN   T_CLK_JCLK_DDR2CLK_MODE
* @NOTE: 
*/
T_CLK_JCLK_DDR2CLK_MODE clk_get_cpu_clk_mode(T_VOID)
{
    T_CLK_JCLK_DDR2CLK_MODE cpu_clk_mode;
    T_U32 cpu_1x_2x_sel_cfg;
    T_U32 reg_val;

    reg_val = REG32(CPU_PLL_CFG_REG);
        
    //�õ��Ĵ�����ֵ
    cpu_1x_2x_sel_cfg = (reg_val>>31) & 0x1;
        
    //cpu_1x_2x_sel_cfgΪ1�������  ����JCLK_DDR2CLK_1X1X_MODE ��HCLK=DCLK=DDR2=JCLK = PLL/2
    if(cpu_1x_2x_sel_cfg == 1) 
    {
        cpu_clk_mode = JCLK_DDR2CLK_1X1X_MODE;
    }

    //cpu_1x_2x_sel_cfgΪ0������� ����JCLK_DDR2CLK_2X1X_MODE ��HCLK=DCLK=DDR2= PLL/2    JCLK = PLL
    else
    {
        cpu_clk_mode = JCLK_DDR2CLK_2X1X_MODE;
    }

    return cpu_clk_mode;
    
}





/**
* @BRIEF  ����CPUʱ�ӵĹ���ģʽ
* @AUTHOR Zou Tianxiang
* @DATE   2019-3 -20
* @CHANGE DATE   2021-11 -14 FOR H322
* @PARAM  
* @PARAM  
* @NOTE:  ��ȷ�ϣ�������2X1X ��1X1X֮���ģʽ�ı�DDR2CLK����ı䣬��˲���Ҫ�ŵ��ڲ�RAM���ܱ�Ƶ����
*/
T_VOID clk_set_cpu_clk_mode(T_CLK_JCLK_DDR2CLK_MODE  jclk_ddr2clk_mode)
{
    T_U32 cpu_pll_clk;
    T_U32 expect_ddr2clk;
    T_U32 refresh_val;
    T_CLK_JCLK_DDR2CLK_MODE cpu_now_clk_mode;
    void (*p_change_freq)();

    //�õ���ǰ��CPU PLL CLK
    cpu_pll_clk = clk_get_cpu_pll_freq();

    //�õ���ǰCPU CLKģʽ
    cpu_now_clk_mode = clk_get_cpu_clk_mode();

    //������ڵ�ģʽ����Ҫ�任��ģʽ������Ҫ�任��
    if(jclk_ddr2clk_mode == cpu_now_clk_mode)
    {
        printf("now is in this mode.\n");
        return;
    }

    if(jclk_ddr2clk_mode == JCLK_DDR2CLK_1X1X_MODE )
    {
        //����Ϊ1X1Xģʽ
        REG32(CPU_PLL_CFG_REG) |=  (0x1<<31);
    }
    else if( jclk_ddr2clk_mode == JCLK_DDR2CLK_2X1X_MODE )
    {
        //����Ϊ2X1Xģʽ
        REG32(CPU_PLL_CFG_REG) &= (~(0x1<<31));
    }
    
}







/**
* @BRIEF  set vclk div value
* @AUTHOR Zou Tianxiang
* @DATE   2013-3 -19
* @CHANGE ON DATE   2021-3 -1  FOR H322
* @PARAM   T_U32 div:    ���÷�Ƶ�� ���DIVΪ0�������ֱ���ͳ�PLL��ʱ��
* @PARAM  T_CLK_PLL_TYPE src_pll_type:   ����VCLK ��PLLԴ
* @PARAM  T_CLK_MODULE_IN_VCLK module_vclk:  VCLKģ������
* @RETURN   
* @NOTE:   XXX_vclk=pll_clk/(src_clk_div_num_cfg+1)
*/
T_VOID clk_set_vclk_div(T_U32 div, T_CLK_PLL_TYPE src_pll_type, T_CLK_MODULE_IN_VCLK module_vclk)
{
    T_U32 src_clk_sel_cfg, src_clk_div_num_cfg;
    T_U32 reg_val;
    T_U32 i;

    //1��Ƶ������ֱ��ʹ��PLL��ʱ��
    if(div == 1)
    {
        src_clk_sel_cfg = (src_pll_type | (0x1<<2));
        src_clk_div_num_cfg  = 0; 
    }
    else
    {
        //��Ƶ�ȱ�����1~ 8֮��
        if((div > 8) || (div == 0))
        {
            printf("Err: div must  1 <= div <= 8.\n");
            while(1);
        }
        
        src_clk_sel_cfg = src_pll_type;
        src_clk_div_num_cfg = div - 1;
    }

    

    //�ر�vclkʱ��
    switch(module_vclk)
    {
        case ISP_VCLK:
            //�ر� isp vclk ʱ��
            REG32(ISP_CNN_VIDEO_PP_SRC_CLK_CFG_REG) &= (~(0x1<<6));
            //��ʱ��������
            for(i = 0; i < 100; i++);
            //���÷�Ƶ�Ⱥ�PLLԴ
            REG32(ISP_CNN_VIDEO_PP_SRC_CLK_CFG_REG) &= (~0x3f);
            REG32(ISP_CNN_VIDEO_PP_SRC_CLK_CFG_REG) |= (src_clk_div_num_cfg | (src_clk_sel_cfg<<3));
            //����isp vclk ʱ��
            REG32(ISP_CNN_VIDEO_PP_SRC_CLK_CFG_REG) |= (0x1<<6);
            //��ʱ��������
            for(i = 0; i < 100; i++);
            break;
        
        case CNN_VCLK:
            //�ر� cnn vclk ʱ��
            REG32(ISP_CNN_VIDEO_PP_SRC_CLK_CFG_REG) &= (~(0x1<<14));
            //��ʱ��������
            for(i = 0; i < 100; i++);
            //���÷�Ƶ�Ⱥ�PLLԴ
            REG32(ISP_CNN_VIDEO_PP_SRC_CLK_CFG_REG) &= (~(0x3f<<8));
            REG32(ISP_CNN_VIDEO_PP_SRC_CLK_CFG_REG) |= ((src_clk_div_num_cfg<<8) | (src_clk_sel_cfg<<11));
            //����cnn vclk ʱ��
            REG32(ISP_CNN_VIDEO_PP_SRC_CLK_CFG_REG) |= (0x1<<14);
            //��ʱ��������
            for(i = 0; i < 100; i++);
            break;
            
        case ENC_VCLK:
            //�ر� cnn vclk ʱ��
            REG32(ISP_CNN_VIDEO_PP_SRC_CLK_CFG_REG) &= (~(0x1<<22));                        
            //��ʱ��������
            for(i = 0; i < 100; i++);
            //���÷�Ƶ�Ⱥ�PLLԴ
            REG32(ISP_CNN_VIDEO_PP_SRC_CLK_CFG_REG) &= (~(0x3f<<16));
            REG32(ISP_CNN_VIDEO_PP_SRC_CLK_CFG_REG) |= ((src_clk_div_num_cfg<<16) | (src_clk_sel_cfg<<19));
            //����cnn vclk ʱ��
            REG32(ISP_CNN_VIDEO_PP_SRC_CLK_CFG_REG) |= (0x1<<22);
            //��ʱ��������
            for(i = 0; i < 100; i++);
            break;

            break;

        case PP_VCLK:
            //�ر� PP vclk ʱ��
            REG32(ISP_CNN_VIDEO_PP_SRC_CLK_CFG_REG) &= (~(0x1<<30));                        
            //��ʱ��������
            for(i = 0; i < 100; i++);
            //���÷�Ƶ�Ⱥ�PLLԴ
            REG32(ISP_CNN_VIDEO_PP_SRC_CLK_CFG_REG) &= (~(0x3f<<24));
            REG32(ISP_CNN_VIDEO_PP_SRC_CLK_CFG_REG) |= ((src_clk_div_num_cfg<<24) | (src_clk_sel_cfg<<27));
            //����PP vclk ʱ��
            REG32(ISP_CNN_VIDEO_PP_SRC_CLK_CFG_REG) |= (0x1<<30);
            //��ʱ��������
            for(i = 0; i < 100; i++);
            break;

            
            break;            
            
        default:
            printf("Err in module_vclk.\n");
            while(1);
            break;
    }

}







/**
* @BRIEF  clk_set_csi_sclk_div
* @AUTHOR Zou Tianxiang
* @DATE   2024-5 -13 
* @PARAM  T_CLK_CSI_SCLK_CFG csi_sclk_cfg
* @PARAM  
* @RETURN 
* @NOTE: CSI DIV ��Ƶ����˳�����£����������ȷ��:
   1. �ض˿�CLK 
   2. �ص�һ��csi_sclk_srcԴena
   3. ѡ���һ��csi_sclk_src��Դ������DIV(���Էֱ�����Ҳ����ͬʱ���ã���Ϊ��ͬ1�Ĵ���0x18) 
   4. ��CSIԴENA 
   5. ѡ��˿ڵ�Դ
   6. �ȶ˿�ԴREADY�� ���ȴ��͵��˿�ǰ����ʱ���ȶ�
   7. �򿪶˿�CLK
*/
T_VOID clk_set_csi_sclk_div(T_CLK_CSI_SCLK_CFG csi_sclk_cfg)
{
    T_U32 i;
    T_U32 reg_val;

    //1. �ض˿�CLK 
    clk_close_csi_sclk(csi_sclk_cfg.csi_sclk_index);
    
    //��ѡ��24MHZʱ��
    if(csi_sclk_cfg.csi_sclk_src == CSI_SCLK_USE_24MHZ)
    {
        //ѡ��csi_sclk�˿ڵ�ԴΪ24MHZʱ��
        REG32(SENSOR_CLK_CFG_REG) &= ~(0x3<<(csi_sclk_cfg.csi_sclk_index * 4));
    }
    else
    {
         //��Ƶ�ȱ�����1~ 64֮��
        if(csi_sclk_cfg.div > 64)
        {
            printf("Err: div must  div <= 64.\n");
            while(1);
        }
        
        //�ж�PLLԴ�����Ƿ���ȷ
        if(csi_sclk_cfg.pll_type > AUDIO_PLL)
        {
            printf("Err: src_pll = %x\n", csi_sclk_cfg.pll_type); 
            while(1);        
        }
    
        //2. �ص�һ��csi_sclk_srcԴena
        REG32(MAC_AND_CSI_TOP_CFG1_REG) &= ~(0x1<<(csi_sclk_cfg.csi_sclk_src*10+9));
        //��ʱ��������
        for(i = 0; i < 100; i++);
        
        // 3. ѡ���һ��csi_sclk_src��Դ������DIV
        reg_val = REG32(MAC_AND_CSI_TOP_CFG1_REG);
        reg_val &= ~(0x1ff<<(csi_sclk_cfg.csi_sclk_src * 10));
        reg_val |= (((csi_sclk_cfg.div - 1) << (csi_sclk_cfg.csi_sclk_src * 10)) | \
                   (csi_sclk_cfg.pll_type << (csi_sclk_cfg.csi_sclk_src * 10 + 6)) );
        REG32(MAC_AND_CSI_TOP_CFG1_REG) = reg_val;
        
        //4. ��CSIԴENA   ���� csi_sclk_src ʱ��
        REG32(MAC_AND_CSI_TOP_CFG1_REG) |= (0x1<<(csi_sclk_cfg.csi_sclk_src * 10 + 9));

        //5. ѡ��˿ڵ�Դ
        reg_val = REG32(SENSOR_CLK_CFG_REG);
        reg_val &= ~(0x3<<(csi_sclk_cfg.csi_sclk_index * 4));
        reg_val |= (csi_sclk_cfg.csi_sclk_src << (csi_sclk_cfg.csi_sclk_index * 4));
        REG32(SENSOR_CLK_CFG_REG) = reg_val;
    }


    //6. �ȶ˿�ԴREADY�� ���ȴ��͵��˿�ǰ����ʱ���ȶ�
    while((REG32(SENSOR_CLK_CFG_REG) & (0x1 << (csi_sclk_cfg.csi_sclk_index*4 + 3))) == 0)
    {;}

    
    //7. �򿪶˿�CLK
    clk_open_csi_sclk(csi_sclk_cfg.csi_sclk_index);
}








/**
* @BRIEF  clk_open_csi_sclk
* @AUTHOR Zou Tianxiang
* @DATE   2013-3 -20   
* @PARAM  
* @PARAM  
* @RETURN 
* @NOTE: ��CSI_SCLK �˿�ʱ��
*/
T_VOID clk_open_csi_sclk(T_CLK_CSI_SCLK_INDEX  csi_sclk_index)
{
    //���� cis_sclk ʱ��
    REG32(SENSOR_CLK_CFG_REG) |= (0x1 << (csi_sclk_index*4 + 2));
}





/**
* @BRIEF  clk_close_csi_sclk
* @AUTHOR Zou Tianxiang
* @DATE   2013-3 -20  
* @PARAM  
* @PARAM  
* @RETURN 
* @NOTE: �رն˿�SCLK
*/
T_VOID clk_close_csi_sclk(T_CLK_CSI_SCLK_INDEX  csi_sclk_index)
{
    //�ر�cis_sclk ʱ��
    REG32(SENSOR_CLK_CFG_REG) &= ~(0x1 << (csi_sclk_index*4 + 2));
}




/**
* @BRIEF  clk_open_csi_src
* @AUTHOR Zou Tianxiang
* @DATE   2024-5 -14   
* @PARAM  
* @PARAM  T_CLK_CSI_SCLK_SRC  csi_sclk_src
* @RETURN 
* @NOTE: 
*/
T_VOID clk_open_csi_src(T_CLK_CSI_SCLK_SRC  csi_sclk_src)
{
    if((csi_sclk_src >= CSI_SCLK_USE_CSI_SRC0_CLK) && \
        (csi_sclk_src <= CSI_SCLK_USE_CSI_SRC1_CLK))
    {
        //����csi_sclk_src  ʱ��
        REG32(MAC_AND_CSI_TOP_CFG1_REG) |= (0x1<<(csi_sclk_src*10+9));
    }
    else
    {
        //�����������
        printf("Err in clk_open_csi_src %x\n", csi_sclk_src);
        while(1);
    }
}




/**
* @BRIEF  clk_close_csi_src
* @AUTHOR Zou Tianxiang
* @DATE   2024-5 -14   
* @PARAM  
* @PARAM  T_CLK_CSI_SCLK_SRC  csi_sclk_src
* @RETURN 
* @NOTE: 
*/
T_VOID clk_close_csi_src(T_CLK_CSI_SCLK_SRC  csi_sclk_src)
{
    if((csi_sclk_src >= CSI_SCLK_USE_CSI_SRC0_CLK) && \
        (csi_sclk_src <= CSI_SCLK_USE_CSI_SRC1_CLK))
    {
        //�ر�csi_sclk_src  ʱ��
        REG32(MAC_AND_CSI_TOP_CFG1_REG) &= ~(0x1<<(csi_sclk_src*10+9));
    }
    else
    {
        //�����������
        printf("Err in clk_close_csi_src %x\n", csi_sclk_src);
        while(1);
    }
}









/**
* @BRIEF  clk_set_opclk_div
* @AUTHOR Zou Tianxiang
* @DATE   2013-3 -20 
* @PARAM  T_CLK_PLL_TYPE src_pll_type
* @PARAM  div range 1 ~ 64
* @RETURN 
* @NOTE: OPCLK DIV ��Ƶ����˳�����£����������ȷ��:
   1. �ض˿�opclk  
   2. ��opclk srcԴena
   3. ѡ��opclk srcԴ������DIV(���Էֱ�����Ҳ����ͬʱ���ã���Ϊ��ͬ1�Ĵ���0x18) 
   4. ��opclk srcԴREADY(���ȴ��ܿ�opclk srcԴ�л�ʱ��ë��Ӱ��)    
   5. ��opclk srcԴENA 
   6. �ȶ˿�ԴREADY�� ���ȴ��͵��˿�ǰ����ʱ���ȶ�
   7. �򿪶˿�opclk
*/
T_VOID clk_set_opclk_div(T_U32 div, T_CLK_PLL_TYPE src_pll_type)
{
    T_U32 reg_val;

    if( (div > 64) || (div == 0) )
    {
        printf("Not support , div must be small than 64 and must not be 0.\n");
        while(1);
    }

    //2022.5.28 SKY:OPCLK��ż����Ƶ����������ż������������ȷ�ϴ������Ϊż��������ִ�С�
    //����ʵ�飬 ��DIV����Ϊ����ʱOPCLKҲ���������ʱ�Ӳ��θߵ͵�ƽռ�ձȲ���1:1��
    if( (div % 2) != 0 )
    {
        printf("Err: Opclk is even divider, div must even.\n");
        while(1);
    }


    //�ж�PLLԴ�����Ƿ���ȷ
    //M31�Ѹ�֪����jitter���ܱ�֤������USB PHY PLL��������ģ�飬���ȥ��USB PHY PLLѡ��֧��
    //if(src_pll_type > USB_PHY_PLL)   
    if(src_pll_type > AUDIO_PLL)
    {
        printf("Err: src_pll = %x\n", src_pll_type); 
        while(1);
    }

    //1. �ض˿�opclk  
    clk_close_opclk();

    //2. ��opclk srcԴena
    clk_close_opclk_src();


    //3. ѡ��opclk srcԴ������DIV
    reg_val = REG32(MAC_AND_CSI_TOP_CFG1_REG);
    reg_val &= ~(0xff);
    reg_val |= ((div-1) | (src_pll_type<<6));
    REG32(MAC_AND_CSI_TOP_CFG1_REG) = reg_val;

    //4. ��opclk srcԴREADY(���ȴ��ܿ�opclk srcԴ�л�ʱ��ë��Ӱ��)    
    while( (REG32(MAC_AND_CSI_TOP_CFG1_REG) & (1<<31)) == 0)
    {;}

    //5. ��opclk srcԴENA 
    clk_open_opclk_src();

    //6. �ȶ˿�ԴREADY�� ���ȴ��͵��˿�ǰ����ʱ���ȶ�
    while( (REG32(MAC_AND_CSI_TOP_CFG1_REG) & (1<<30)) == 0)
    {;}
    
    //7. �򿪶˿�opclk
    clk_open_opclk();
    
}




/**
* @BRIEF  open opclk 
* @AUTHOR Zou Tianxiang
* @DATE   2013-3 -20  
* @CHANGE ON DATE   2021-3 -2  FOR H322
* @PARAM  
* @PARAM  
* @RETURN 
* @NOTE: 
*/
T_VOID clk_open_opclk()
{
    //opclk enable
    REG32(MAC_AND_CSI_TOP_CFG1_REG) |= (1<<8);

}




/**
* @BRIEF  close opclk
* @AUTHOR Zou Tianxiang
* @DATE   2013-3 -20 
* @CHANGE ON DATE   2021-3 -2  FOR H322
* @PARAM  
* @PARAM  
* @RETURN 
* @NOTE: 
*/
T_VOID clk_close_opclk()
{
    //close enable
    REG32(MAC_AND_CSI_TOP_CFG1_REG) &= ~(1<<8);
}



/**
* @BRIEF  clk_open_opclk_src
* @AUTHOR Zou Tianxiang
* @DATE   2024-5 -14  
* @PARAM  
* @PARAM  
* @RETURN 
* @NOTE: 
*/
T_VOID clk_open_opclk_src()
{
    //opclk src open
    REG32(MAC_AND_CSI_TOP_CFG1_REG) |= (1<<9);

}




/**
* @BRIEF  clk_close_opclk_src
* @AUTHOR Zou Tianxiang
* @DATE   2024-5 -14  
* @PARAM  
* @PARAM  
* @RETURN 
* @NOTE: 
*/
T_VOID clk_close_opclk_src()
{
    //close opclk src
    REG32(MAC_AND_CSI_TOP_CFG1_REG) &= ~(1<<9);

}





/**
* @BRIEF  clk_set_gclk_div
* @AUTHOR Zou Tianxiang
* @DATE   2013-3 -20 
* @CHANGE ON DATE   2021-3 -2  FOR H322
* @PARAM  
* @PARAM  div ֻ����2��4�� 6��8
                   gclk = PLL2/div
* @RETURN 
* @NOTE: 
*/
T_VOID clk_set_gclk_div(T_U32 div)
{
    T_U32 i;

    for(i = 0; i < 4; i++)
    {
        if(div == (2*i+2))
        {
            break;
        }
    }

    if( i == 4)
    {
        printf("Not support , div must be 2 , 4 , 6 , 8.\n");
        while(1);
    }


    //wait the state
    while( (REG32(CORE_PLL_CFG_REG) & (1<<25)) != 0)
    {;}

    
    //gclk div cfg
    REG32(CORE_PLL_CFG_REG) &= (~(0x3<<23));
    REG32(CORE_PLL_CFG_REG) |= ((div/2-1)<<23);
    

    //div valid
    REG32(CORE_PLL_CFG_REG) |= (1<<25);


    //wait the state
    while( (REG32(CORE_PLL_CFG_REG) & (1<<25)) != 0)
    {;}
    
}

















/**
* @BRIEF  reset peripheral module 
* @AUTHOR Zou Tianxiang
* @DATE   2013-3 -2
* @PARAM  T_CLK_PERI_TYPE_CLKGATE  PERI_TYPE
* @PARAM  
* @RETURN
* @NOTE: 
*/
T_VOID clk_reset_peripheral_module( T_CLK_PERI_TYPE  peri_type )
{
    if( peri_type <= CLK_I2C2)
    {
        REG32(RESET_CTRL_REG) |= (1<<peri_type);
        REG32(RESET_CTRL_REG) &= (~(1<<peri_type));
    }
    else if( (peri_type >= CLK_ISP) && (peri_type <= CLK_PRE_PROCESSOR))
    {
        REG32(ISP_CNN_VIDEO_PP_SRC_CLK_CFG_REG) |= (1 << ((peri_type-CLK_ISP)*8+7));
        REG32(ISP_CNN_VIDEO_PP_SRC_CLK_CFG_REG) &= ~(1 << ((peri_type-CLK_ISP)*8+7));
    }
    else if( (peri_type >= CLK_HASH) && (peri_type <= CLK_TRNG))
    {
        REG32(TRNG_HASH_RESET_CTRL_REG) |= (1<<(peri_type-CLK_HASH));
        REG32(TRNG_HASH_RESET_CTRL_REG) &= ~(1<<(peri_type-CLK_HASH));
    }   
}











/**
* @BRIEF  copy data to inter ram
* @AUTHOR Zou Tianxiang
* @DATE   2015-10 -29
* @PARAM  
* @PARAM  
* @RETURN 
* @NOTE: 
*/
T_VOID clk_copy_data_to_interram(T_U8 *addr, T_U32 size)
{
    T_U32 i,j;
    T_U32 tmp;
    
    //Copy the standby code to the interram.
    for( i = 0; i < size; i=i+4)
    {
        REG32(L2_BUF_MEM_BASE_ADDR+i) = ( addr[i] | 
                                (addr[i+1]<<8) | 
                                (addr[i+2]<<16) |
                                (addr[i+3]<<24) );
    }

    
    if( (size % 4) != 0)
    {
        tmp = 0;
        for( j = 0; j < size%4; j++)
        {
            tmp = (tmp | (addr[j]<<(j*8)));
        }
        
        REG32(L2_BUF_MEM_BASE_ADDR+i) = tmp;
    }
    
}

#endif




