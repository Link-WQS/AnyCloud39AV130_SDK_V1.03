
// Copyright     :  Copyright (c) CompanyNameMagicTag 2020-2020. All rights reserved.











#ifndef __PHY0_CTL_RB_C_UNION_DEFINE_H__
#define __PHY0_CTL_RB_C_UNION_DEFINE_H__


typedef union {
    
    struct {
        
        
        osal_u32 _CtCw6LowBF_: 16; 
        
        
        osal_u32 reserved_0 : 16; 
    } bits;

    
    osal_u32 u32;
} u_phy_ctl_id;


typedef union {
    
    struct {
        
        
        osal_u32 _CiNLw_CLwTuywFG6wy6Qwh_: 1; 
        
        
        osal_u32 _CiNLw_CLwTuywGF6wy6Qwh_: 1; 
        
        
        osal_u32 _CiNLw_CLwTuywuQtwUywh_: 1; 
        
        
        osal_u32 _CiNLw_CLwTuwuQtwh_: 1; 
        
        
        osal_u32 _CiNLw_CLwTuywuQtwLPwh_: 1; 
        
        
        osal_u32 _CiNLw_CLwTuywuQtw_Pwh_: 1; 
        
        
        osal_u32 _svtROwsROqG3OGAQORw8Oo_: 1; 
        
        
        osal_u32 _svtROwsROqG3OKKOo_: 1; 
        
        
        osal_u32 _svtROwsROqGO4mCv4mwOo_: 1; 
        
        
        osal_u32 _CiNLw_CLwTuywuQtwNLSwh_: 1; 
        
        
        osal_u32 _svtROwsROqG3OGAQOwm2Oo_: 1; 
        
        
        osal_u32 _svtROwsROqG3O4zCOrCAOo_: 1; 
        
        
        osal_u32 _svtROwsROqG3Oz4COrCAOo_: 1; 
        
        
        osal_u32 _CiNLw_CLwFuFwGF6wy6Qwh_: 1; 
        
        
        osal_u32 _svtROwsRO4G4Oz4COrCAOo_: 1; 
        
        
        osal_u32 reserved_0 : 17; 
    } bits;

    
    osal_u32 u32;
} u_wcbb_soft_reset;


typedef union {
    
    struct {
        
        
        osal_u32 _qG3O4zCO3CAOq4ROwsROsmx_: 1; 
        
        
        osal_u32 _qG3Oz4CO3CAOq4ROwsROsmx_: 1; 
        
        
        osal_u32 _qG3OGAQOn3Oq4ROwsROsmx_: 1; 
        
        
        osal_u32 _qGOGAQOq4ROwsROsmx_: 1; 
        
        
        osal_u32 _qG3OGAQOR8Oq4ROwsROsmx_: 1; 
        
        
        osal_u32 _qG3OGAQOw8Oq4ROwsROsmx_: 1; 
        
        
        osal_u32 _TuywuQtwL_PwTFLw_CLwCWo_: 1; 
        
        
        osal_u32 _TuywDDwTFLw_CLwCWo_: 1; 
        
        
        osal_u32 _TuwFW6iFW_wTFLw_CLwCWo_: 1; 
        
        
        osal_u32 _qG3OGAQOtRIOq4ROwsROsmx_: 1; 
        
        
        osal_u32 _TuywuQtw_WgwTFLw_CLwCWo_: 1; 
        
        
        osal_u32 _TuywFG6w06QwTFLw_CLwCWo_: 1; 
        
        
        osal_u32 _TuywGF6w06QwTFLw_CLwCWo_: 1; 
        
        
        osal_u32 _4G4Oz4CO3CAOq4ROwsROsmx_: 1; 
        
        
        osal_u32 _FuFwGF6w06QwTFLw_CLwCWo_: 1; 
        
        
        osal_u32 reserved_0 : 17; 
    } bits;

    
    osal_u32 u32;
} u_wdt_rst_sel;


typedef union {
    
    struct {
        
        
        osal_u32 _qG3Owm2ltOCxdmo_: 1; 
        
        
        osal_u32 reserved_0 : 1; 
        
        
        osal_u32 _qG3O2xyOYK3IOCxdmo_: 1; 
        
        
        osal_u32 _TuywXDyw6oKWh_: 1; 
        
        
        osal_u32 _TuywkVyw6oKWh_: 1; 
        
        
        osal_u32 _Tuyw0Xyw6oKWh_: 1; 
        
        
        osal_u32 reserved_1 : 26; 
    } bits;

    
    osal_u32 u32;
} u_wphy_soft_clken0;


typedef union {
    
    struct {
        
        
        osal_u32 _qG3OR8OZ4vIOCxdmo_: 1; 
        
        
        osal_u32 _qG3OK8OCxdmo_: 1; 
        
        
        osal_u32 _qG3OCzxlOw84tmOz2COCxdmo_: 1; 
        
        
        osal_u32 _qG3OR8O4tmOr4vIOCxdmo_: 1; 
        
        
        osal_u32 reserved_0 : 1; 
        
        
        osal_u32 _TuywVPw6oKWh_: 1; 
        
        
        osal_u32 _qG3OsQoCO4zRzOGzRAOCxdmo_: 1; 
        
        
        osal_u32 _qG3OCCzOCxdmo_: 1; 
        
        
        osal_u32 reserved_1 : 1; 
        
        
        osal_u32 _TuywUyw6oKWh_: 1; 
        
        
        osal_u32 _qG3OKKOCxdmo_: 1; 
        
        
        osal_u32 _TuwFW6iFWw6oKWh_: 1; 
        
        
        osal_u32 _qGOGZIOCxdmo_: 1; 
        
        
        osal_u32 _TuywLPw6oKWh_: 1; 
        
        
        osal_u32 reserved_2 : 1; 
        
        
        osal_u32 _Tuyw6GoBwLPw6oKWh_: 1; 
        
        
        osal_u32 _TuywNLSw6oKWh_: 1; 
        
        
        osal_u32 reserved_3 : 1; 
        
        
        osal_u32 _qG3OR8O4tmOr4vIOR8sCvOCxdmo_: 1; 
        
        
        osal_u32 reserved_4 : 13; 
    } bits;

    
    osal_u32 u32;
} u_wphy_soft_clken1;


typedef union {
    
    struct {
        
        
        osal_u32 _qG3O4zCOCxdmo_: 1; 
        
        
        osal_u32 _qG3Oz4COCxdmo_: 1; 
        
        
        osal_u32 _Tuyw_CCBwGF6w6oKWh_: 1; 
        
        
        osal_u32 reserved_0 : 29; 
    } bits;

    
    osal_u32 u32;
} u_wcbb_soft_clken2;


typedef union {
    
    struct {
        
        
        osal_u32 _Tuyw0XywFBqwhdS_: 4; 
        
        
        osal_u32 _TuywkVywFBqwhdS_: 4; 
        
        
        osal_u32 _TuywXDywFBqwhdS_: 4; 
        osal_u32 reserved_0 : 20;     
    } bits;

    
    osal_u32 u32;
} u_wphy_clk_div_0;


typedef union {
    
    struct {
        
        
        osal_u32 _TuywUywFBqwhdS_: 2; 
        
        
        osal_u32 reserved_0 : 6; 
        
        
        osal_u32 _TuwFW6iFWwFBqwhdS_: 2; 
        
        
        osal_u32 _qGOGZIO4lPOoSI_: 2; 
        
        
        osal_u32 reserved_1 : 20; 
    } bits;

    
    osal_u32 u32;
} u_wphy_clk_div_1;


typedef union {
    
    struct {
        
        
        osal_u32 _qG3OK8O4lPOoSI_: 2; 
        
        
        osal_u32 reserved_0 : 2; 
        
        
        osal_u32 _qG3OCzxlOw84tmOz2CO4lPOoSI_: 2; 
        
        
        osal_u32 reserved_1 : 2; 
        
        
        osal_u32 _TuywLPwFNWw0FiSwFBqwhdSwSGh_: 2; 
        
        
        osal_u32 reserved_2 : 2; 
        
        
        osal_u32 _qG3OR8O4tmOr4vIO4lPOoSI_: 2; 
        
        
        osal_u32 reserved_3 : 2; 
        
        
        osal_u32 _TuywLPwFNWw0FiSwLPC6iwFBqwhdSwSGh_: 2; 
        
        
        osal_u32 reserved_4 : 2; 
        
        
        osal_u32 _qG3OR8O4tmOr4vIOR8sCvO4lPOoSI_: 2; 
        
        
        osal_u32 reserved_5 : 10; 
    } bits;

    
    osal_u32 u32;
} u_wcbb_clk_div_2;


typedef union {
    
    struct {
        
        
        osal_u32 _TuywVPwFBqwhdS_: 2; 
        
        
        osal_u32 reserved_0 : 2; 
        
        
        osal_u32 _qG3OsQoCO4zRzOGzRAO4lPOoSI_: 2; 
        
        
        osal_u32 reserved_1 : 2; 
        
        
        osal_u32 _qG3OCCzO4lPOoSI_: 2; 
        
        
        osal_u32 reserved_2 : 6; 
        
        
        osal_u32 _TuywNLSwFBqwhdS_: 2; 
        
        
        osal_u32 reserved_3 : 14; 
    } bits;

    
    osal_u32 u32;
} u_wcbb_clk_div_3;


typedef union {
    
    struct {
        
        
        osal_u32 reserved_0 : 4; 
        
        
        osal_u32 _qG3Owm2ltO4lPOoSI_: 2; 
        
        
        osal_u32 reserved_1 : 26; 
    } bits;

    
    osal_u32 u32;
} u_wcbb_clk_div_4;


typedef union {
    
    struct {
        
        
        osal_u32 _qG3OR8O4lPOoSIOIzo_: 7; 
        
        
        osal_u32 reserved_0 : 25; 
    } bits;

    
    osal_u32 u32;
} u_wcbb_clk_div_5;


typedef union {
    
    struct {
        
        
        osal_u32 reserved_0 : 2; 
        
        
        osal_u32 _TuywLPwFNWwVFiSwFBqwhdS_: 2; 
        
        
        osal_u32 reserved_1 : 28; 
    } bits;

    
    osal_u32 u32;
} u_wcbb_clk_div_6;


typedef union {
    
    struct {
        
        
        osal_u32 _TouQtywFBqwWh_: 1; 
        
        
        osal_u32 reserved_0 : 31; 
    } bits;

    
    osal_u32 u32;
} u_clk_div_en;


typedef union {
    
    struct {
        
        
        osal_u32 reserved_0 : 1; 
        
        
        osal_u32 _qG3On3OrryOCxdsmx_: 1; 
        
        
        osal_u32 _qG3OGAQOs3Otwm6OCxdsmx_: 2; 
        
        
        osal_u32 _qG3OttROCxdsmxOIzo_: 1; 
        
        
        osal_u32 reserved_1 : 3; 
        
        
        osal_u32 _TuywNNLw6oKCWowSiF_: 1; 
        
        
        osal_u32 _TuywFW6iFW_wuQGCWwCWo_: 1; 
        
        
        osal_u32 _TuywuVSwuQGCWwCWo_: 1; 
        
        
        osal_u32 reserved_2 : 1; 
        
        
        osal_u32 _qG3OsCvOCxdsmxOIzo_: 1; 
        
        
        osal_u32 _TuywC6iw6oKCWowSiF_: 1; 
        
        
        osal_u32 reserved_3 : 18; 
    } bits;

    
    osal_u32 u32;
} u_wcbb_clk_sel;


typedef union {
    
    struct {
        
        
        osal_u32 _TuywL_PwC6iw6oKjw6oKih_: 1; 
        
        
        osal_u32 _qG3ORw8OsCvOCxdzOCxdvo_: 1; 
        
        
        osal_u32 _TuywNNLwLPw6oKih_: 1; 
        
        
        osal_u32 _TuywNNLw_Pw6oKih_: 1; 
        
        
        osal_u32 _qG3OZ8OttROCxdyOCxdvo_: 1; 
        
        
        osal_u32 _TuywVPwNNLw6oKGw6oKih_: 1; 
        osal_u32 reserved_0 : 26;           
    } bits;

    
    osal_u32 u32;
} u_clkmux_sts;


typedef union {
    
    struct {
        
        
        osal_u32 _qG3OR8O4tmOr4vIO4lPOoSIOsmx_: 1; 
        
        
        osal_u32 _TuywLPwFBqwhdSwCWo_: 1; 
        
        
        osal_u32 _qGOGZIO4lPOoSIOsmx_: 1; 
        
        
        osal_u32 _qGO4mCv4mwO4lPOoSIOsmx_: 1; 
        
        
        osal_u32 _qG3OR8O4tmOr4vIOR8sCvO4lPOoSIOsmx_: 1; 
        
        
        osal_u32 reserved_0 : 27; 
    } bits;

    
    osal_u32 u32;
} u_div_num_sel;


typedef union {
    
    struct {
        
        
        osal_u32 _gojwu_W6QG_gWwWh_: 1; 
        
        
        osal_u32 reserved_0 : 31; 
    } bits;

    
    osal_u32 u32;
} u_wl_phy_memory_sleep;


typedef union {
    
    struct {
        osal_u32 _TowuQtwSWSw6oKwGdLiw6gwWh_: 1; 
        osal_u32 _TowuQtwSWSw6oKwNi_6Wwih_: 1;   
        
        
        osal_u32 reserved_0 : 30; 
    } bits;

    
    osal_u32 u32;
} u_wl_phy_memory_auto;


typedef union {
    
    struct {
        osal_u32 _TowuQtwSG_gBhwGFbwG_: 5; 
        
        
        osal_u32 reserved_0 : 3;          
        osal_u32 _qxOGAQOIzw2loOz4WOy_: 5; 
        
        
        osal_u32 reserved_1 : 19; 
    } bits;

    
    osal_u32 u32;
} u_wl_phy_memory_adj;


typedef union {
    
    struct {
        
        
        osal_u32 _qxOGAQORGOwzIORIv4_: 8; 
        
        
        osal_u32 reserved_0 : 24; 
    } bits;

    
    osal_u32 u32;
} u_wl_phy_tpram_tmod;


typedef union {
    
    struct {
        
        
        osal_u32 _qxOGAQOsGOwzIORIv4_: 7; 
        
        
        osal_u32 reserved_0 : 25; 
    } bits;

    
    osal_u32 u32;
} u_wl_phy_spram_tmod;


typedef union {
    
    struct {
        
        
        osal_u32 _qxOGAQOwtOwzIORIv4_: 7; 
        
        
        osal_u32 reserved_0 : 25; 
    } bits;

    
    osal_u32 u32;
} u_wl_phy_rf_ram_tmod;


typedef union {
    
    struct {
        
        
        osal_u32 _qxOGAQOICIOsRzRmOom8R_: 2; 
        
        
        osal_u32 reserved_0 : 30; 
    } bits;

    
    osal_u32 u32;
} u_wl_phy_md_cfg;


typedef union {
    
    struct {
        osal_u32 _TowuQtwFBGg_: 8; 
        
        
        osal_u32 reserved_0 : 24; 
    } bits;

    
    osal_u32 u32;
} u_wl_phy_diag;


typedef union {
    
    struct {
        
        
        osal_u32 wlbb_testmode : 3; 
        
        
        osal_u32 reserved_0 : 29; 
    } bits;

    
    osal_u32 u32;
} u_wlbb_testmode;


typedef union {
    
    struct {
        osal_u32 _CxmzwOszIGxmO4vom_: 1; 
        
        
        osal_u32 reserved_0 : 31; 
    } bits;

    
    osal_u32 u32;
} u_wlbb_clr_sample;


typedef union {
    
    struct {
        osal_u32 _CxmzwOtltvOzxI_: 4; 
        
        
        osal_u32 reserved_0 : 28; 
    } bits;

    
    osal_u32 u32;
} u_wlbb_clr_fifo;


typedef union {
    
    struct {
        
        
        osal_u32 _R8Ol6Om8CAzo2m_: 1; 
        
        
        osal_u32 reserved_0 : 3; 
        
        
        osal_u32 _w8Ol6Om8CAzo2m_: 1; 
        
        
        osal_u32 reserved_1 : 27; 
    } bits;

    
    osal_u32 u32;
} u_wlbb_iq_ex;


typedef union {
    
    struct {
        osal_u32 test_start : 1;  
        osal_u32 sample_done : 1; 
        
        
        osal_u32 reserved_0 : 30; 
    } bits;

    
    osal_u32 u32;
} u_test_start;


typedef union {
    
    struct {
        
        
        osal_u32 _6NgwNi_6Ww_Wgw6oKwih_: 1; 
        
        
        osal_u32 _6NgwNi_6Ww_Wg0w6oKwih_: 1; 
        
        
        osal_u32 _Ct2OtvwCmOwm2ZOCxdOvo_: 1; 
        
        
        osal_u32 _Ct2OtvwCmOwm2iOCxdOvo_: 1; 
        
        
        osal_u32 _6NgwNi_6Ww_WgDw6oKwih_: 1; 
        
        
        osal_u32 _Ct2OtvwCmOwm2aOCxdOvo_: 1; 
        
        
        osal_u32 _6NgwNi_6Ww_WgXw6oKwih_: 1; 
        
        
        osal_u32 _Ct2OtvwCmOwm29OCxdOvo_: 1; 
        
        
        osal_u32 reserved_0 : 24; 
    } bits;

    
    osal_u32 u32;
} u_reg_bank_clk_force_on;

#endif 
