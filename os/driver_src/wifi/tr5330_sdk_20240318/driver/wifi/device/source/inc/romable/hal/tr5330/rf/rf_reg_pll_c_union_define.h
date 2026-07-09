/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: tr5330 rf reg interface and struct adaption
 * Date: 2022-11-12
*/

#ifndef __RF_REG_PLL_C_UNION_DEFINE_H__
#define __RF_REG_PLL_C_UNION_DEFINE_H__

typedef union {
    
    struct {
        
        osal_u32 _moOGxx_              : 1;
        
        osal_u32 _GxxOwsROo_           : 1;
        
        osal_u32 _uoowSiFWwCWo_        : 1;
        
        osal_u32 _GxxO4l2Ox4OCdOmo_    : 1;
        
        osal_u32 _uoow6LwWhwFBg6oKwFBqV_: 1;
        
        osal_u32 _WhwFBgw6oKw_WCW_qW_  : 1;
        
        osal_u32 _WhwFBgw6oKwFBqD_     : 1;
        
        osal_u32 _moO4l2OCxdO4lPZ_     : 1;
        
        osal_u32 _moO4l2OCxd_          : 1;
        
        osal_u32 _6oKwNi_6Wwih_        : 1;
        
        osal_u32 _GxxO4l2OCd2Omo_      : 1;
        
        osal_u32 _WhwuQGCWwCoBu_       : 1;
        
        osal_u32 _moOGxxOx4_           : 1;
        
        osal_u32 _moOPCvOCR_           : 1;
        
        osal_u32 _moOzzC_              : 1;
        
        osal_u32 _WhwGG6w6L_           : 1;
        osal_u32                       : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_pll1_union;

typedef union {
    
    struct {
        
        osal_u32 _twzCOoOIsy_: 7;
        
        osal_u32 _BhLwh_    : 9;
        osal_u32            : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_pll2_union;

typedef union {
    
    struct {
        
        osal_u32 _twzCOoOxsy_: 16;
        osal_u32            : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_pll3_union;

typedef union {
    
    struct {
        osal_u32 __WCqwy_         : 3;
        
        osal_u32 _Fwq6iwGLWSuwWh_ : 1;
        
        osal_u32 _4OPCvOyStOxvOmo_: 1;
        
        osal_u32 _4OPCvOyStOGxxOmo_: 1;
        
        osal_u32 _Fwq6iwWh_       : 1;
        
        osal_u32 _4OylzsOmo_      : 1;
        
        osal_u32 _4Oo4lPOmo_      : 1;
        
        osal_u32 _4OCIGOmo_       : 1;
        
        osal_u32 _4OsQoCOmo_      : 1;
        
        osal_u32 _4O2IOmo_        : 1;
        
        osal_u32 _FwCuFwWh_       : 1;
        
        osal_u32 _FwFL6wWh_       : 1;
        
        osal_u32 _Fw_WNjdNwWh_    : 1;
        
        osal_u32 testmode         : 1;
        osal_u32                  : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_pll4_union;

typedef union {
    
    struct {
        osal_u32 __WCqwy_            : 5;
        
        osal_u32 _Fwq6iwLWCLwWh_     : 1;
        
        osal_u32 _FwgSwq_WNwLWCLwWh_ : 1;
        
        osal_u32 _FwCuFwLWCLwWh_     : 1;
        
        osal_u32 _Fw6SuwWhwq6SuwLWCL_: 1;
        
        osal_u32 _FwouNwWhwqLdhWwCWL_: 1;
        
        osal_u32 _4OxGtOmoOPRSomORmsR_: 1;
        
        osal_u32 _4OzRmIGORmsROmo_   : 1;
        
        osal_u32 _4ORmsRIS8Osmx_     : 3;
        
        osal_u32 _4OmoORmsRIS8_      : 1;
        osal_u32                     : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_pll5_union;

typedef union {
    
    struct {
        osal_u32 __WCqwy_               : 5;
        
        osal_u32 _4OwmtyStOwmsmwPm_     : 2;
        
        osal_u32 _4OwmtyStO4vSyxmwOmo_  : 1;
        
        osal_u32 _Fw_WNjdNwFidjoW_wFWoGt_: 4;
        
        osal_u32 _Fw_WNjdNwL_BSwidLjdN_ : 2;
        
        osal_u32 _Fw_WNjdNwL_BSwBhjdN_  : 2;
        osal_u32                        : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_pll6_union;

typedef union {
    
    struct {
        
        osal_u32 _4O4RCOyStOsq_: 4;
        
        osal_u32 _4O4RCOwOsmx_ : 5;
        
        osal_u32 _4O4RCOwmsmwPm_: 1;
        
        osal_u32 _4O4RCOyQGzss_: 1;
        
        osal_u32 _FwFL6w6NBP_  : 5;
        osal_u32               : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_pll7_union;

typedef union {
    
    struct {
        
        osal_u32 _4OsG4OwOsxvGm_        : 4;
        
        osal_u32 _FwCuFwoFwWh_          : 2;
        
        osal_u32 _FwFL6wCth6wFotwCWo_   : 3;
        
        osal_u32 _4O4RCOsQoCOwOsmx_     : 3;
        
        osal_u32 _4O4RCOsQoCOwsRO4xQOsmx_: 2;
        
        osal_u32 _FwFL6wCth6w6oKwBhq_   : 1;
        
        osal_u32 _4O4RCOsQoCOwsROmo_    : 1;
        osal_u32                        : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_pll8_union;

typedef union {
    
    struct {
        
        osal_u32 _4OsG4OCZ_       : 3;
        
        osal_u32 _FwCuFw_V_       : 3;
        
        osal_u32 _FwCuFw6CV_      : 3;
        
        osal_u32 _4OsG4OCsr_      : 3;
        osal_u32 __WCqwy_         : 1;
        
        osal_u32 _4OsG4Om42mOCd4lP_: 1;
        
        osal_u32 _FwCuFwWFgWw6K_WN_: 1;
        
        osal_u32 _4OsG4Ox4OIv4m_  : 1;
        osal_u32                  : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_pll9_union;

typedef union {
    
    struct {
        osal_u32 __WCqwy_       : 2;
        
        osal_u32 _FwgSwq_WNwCWo_: 3;
        
        osal_u32 _4O2IOsmx_     : 3;
        
        osal_u32 _4O2IOlGRzR_   : 3;
        
        osal_u32 _FwgSwBjg_     : 3;
        
        osal_u32 _4OsG4Ox4O4mxzQ_: 2;
        osal_u32                : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_pll10_union;

typedef union {
    
    struct {
        osal_u32 __WCqwy_       : 6;
        
        osal_u32 _4OCGOlCG_     : 2;
        
        osal_u32 _4OCGOmo_      : 2;
        
        osal_u32 _FwuNFwNi_6WwFh_: 1;
        
        osal_u32 _4OGt4OtvwCmOSG_: 1;
        
        osal_u32 _FwuNFwWhwFh_  : 1;
        
        osal_u32 _4OGt4OmoOSG_  : 1;
        
        osal_u32 _4OGt4Omo_     : 2;
        osal_u32                : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_pll11_union;

typedef union {
    
    struct {
        osal_u32 __WCqwy_                : 5;
        
        osal_u32 _4OxGtO4zCOPsmR_        : 3;
        
        osal_u32 _FwouNwFG6wq_WNwCWowuLGL_: 1;
        
        osal_u32 _4OxGtOzIGOlRwlI_       : 1;
        
        osal_u32 _4OxGtOmoOzIG_          : 2;
        
        osal_u32 _4OxGtOmoOsqtxx_        : 2;
        
        osal_u32 _4OxGtOmoOtxx_          : 2;
        osal_u32                         : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_pll12_union;

typedef union {
    
    struct {
        osal_u32 __WCqwy_ : 2;
        
        osal_u32 _4OxGtOwi_: 2;
        
        osal_u32 _FwouNw_0_: 2;
        
        osal_u32 _4OxGtOCK_: 2;
        
        osal_u32 _FwouNw6k_: 2;
        
        osal_u32 _4OxGtOCr_: 2;
        osal_u32 _wmsPOr_ : 1;
        
        osal_u32 _FwouNw6y_: 3;
        osal_u32          : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_pll13_union;

typedef union {
    
    struct {
        osal_u32 __WCqwy_         : 6;
        
        osal_u32 _4OCIGOGwmOCAzw2m_: 1;
        
        osal_u32 _4OCIGOCzxOl_    : 3;
        
        osal_u32 _4OCIGOCzxOC_    : 3;
        
        osal_u32 _4OCoROsmxOm42m_ : 1;
        
        osal_u32 _4OCoROmo_       : 2;
        osal_u32                  : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_pll14_union;

typedef union {
    
    struct {
        
        osal_u32 _FwhFBqwuTwCWo_     : 8;
        
        osal_u32 _FwhFBqw6oKjdNwL_BS_: 2;
        
        osal_u32 _FwhFBqw6oKVuNFwL_BS_: 2;
        
        osal_u32 _4Oo4lPOCxdZ4sIORwlI_: 2;
        
        osal_u32 _FwhFBqw6hLwFBqwCWo_: 2;
        osal_u32                     : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_pll15_union;

typedef union {
    
    struct {
        
        osal_u32 _Fwq6iwLGBowBhFw6L_BS_: 4;
        
        osal_u32 _Fwq6iwBjBGCwBuLGLwCWo_: 4;
        
        osal_u32 _4OPCvOlylzsOly2Osmx_ : 4;
        
        osal_u32 _4OPCvOlylzsOwCORwlI_ : 2;
        
        osal_u32 _4OPCvOlylzsOtzsRvo_  : 2;
        osal_u32                       : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_pll16_union;

typedef union {
    
    struct {
        
        osal_u32 _4OPCvOCROylzsOsmx_    : 4;
        
        osal_u32 _4OPCvOCROylzsOmo_     : 1;
        
        osal_u32 _Fwq6iwqG_wjBGCwWh_    : 1;
        
        osal_u32 _4OPCvOlylzsOyQGzssOmo_: 1;
        
        osal_u32 _Fwq6iwBjBGCwWh_       : 1;
        
        osal_u32 _4OPCvO2IOwRwlI_       : 3;
        
        osal_u32 _Fwq6iwgSw6iduoWwjtuGCC_: 1;
        
        osal_u32 _4OPCvOyStOxvORwlI_    : 2;
        
        osal_u32 _4OPCvOyStOGxxORwlI_   : 2;
        osal_u32                        : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_pll17_union;

typedef union {
    
    struct {
        
        osal_u32 _Fwq6iwqG_wKBwL_BS_   : 3;
        
        osal_u32 _Fwq6iwqG_wKuwL_BS_   : 3;
        
        osal_u32 _Fwq6iwjdNwFLtw6towGFb_: 5;
        
        osal_u32 _Fwq6iwqG_wjBGCwCWo_  : 3;
        
        osal_u32 _4OPCvOPzwOylzsOtzsRvo_: 2;
        osal_u32                       : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_pll18_union;

typedef union {
    
    struct {
        
        osal_u32 _4OPCvOwmsmwPm_             : 3;
        
        osal_u32 _4OPCvOylzsOzxqzQsOvo_      : 1;
        
        osal_u32 _Fwq6iwGLWSuwFG6wqCWL_      : 3;
        
        osal_u32 _4OPCvOzRmIGOGwmxvz4OIv4mOmo_: 1;
        
        osal_u32 _Fwq6iwGLWSuwuLGLwCoiuW_    : 2;
        
        osal_u32 _4OPCvOzRmIGOGRzROIv4m_     : 1;
        
        osal_u32 _4OPCvOzRmIGO2o4Omo_        : 1;
        
        osal_u32 _4OPCvOzRmIGOwCORwlI_       : 2;
        
        osal_u32 _4OPCvOzRmIGOtzsRvo_        : 2;
        osal_u32                             : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_pll19_union;

typedef union {
    
    struct {
        
        osal_u32 reserve_ana1 : 16;
        osal_u32              : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_pll20_union;

typedef union {
    
    struct {
        
        osal_u32 reserve_ana2 : 16;
        osal_u32              : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_pll21_union;

typedef union {
    
    struct {
        
        osal_u32 reserve_ana3 : 16;
        osal_u32              : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_pll22_union;

typedef union {
    
    struct {
        
        osal_u32 reserve_ana4 : 16;
        osal_u32              : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_pll23_union;

typedef union {
    
    struct {
        
        osal_u32 reserve_ana5 : 16;
        osal_u32              : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_pll24_union;

typedef union {
    
    struct {
        
        osal_u32 reserve_dig1 : 16;
        osal_u32              : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_pll25_union;

typedef union {
    
    struct {
        osal_u32 __WCqwy_   : 4;
        
        osal_u32 tphaseslip : 6;
        
        osal_u32 twarmup    : 6;
        osal_u32            : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_pll26_union;

typedef union {
    
    struct {
        osal_u32 __WCqwy_       : 7;
        
        osal_u32 _twm6OoIywOzP2_: 3;
        
        osal_u32 _IzoSzxOzP2OwSo_: 1;
        
        osal_u32 _NCSwCLiuwCLGLW_: 4;
        
        osal_u32 _NCSwCLiuwWh_  : 1;
        osal_u32                : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_pll27_union;

typedef union {
    
    struct {
        
        osal_u32 _PCvOCRtOylROsmx_: 3;
        
        osal_u32 _GG6w6LNwjBLwCWo_: 3;
        
        osal_u32 _LoFw_dh_       : 5;
        
        osal_u32 _oFw6hLwLQ_     : 4;
        
        osal_u32 _SGhdGowoFw_dh_ : 1;
        osal_u32                 : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_pll28_union;

typedef union {
    
    struct {
        osal_u32 __WCqwy_                : 4;
        
        osal_u32 _uoow6LwSGhdGowjGhFwq6i0_: 11;
        
        osal_u32 _uoow6LwSGhdGowq6i0_    : 1;
        osal_u32                         : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_pll29_union;

typedef union {
    
    struct {
        osal_u32 __WCqwy_                : 4;
        
        osal_u32 _GxxOCROIzoSzxOyzo4OPCvZ_: 11;
        
        osal_u32 _GxxOCROIzoSzxOPCvZ_    : 1;
        osal_u32                         : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_pll30_union;

typedef union {
    
    struct {
        osal_u32 __WCqwy_                : 2;
        
        osal_u32 _uoow6LwNBhWwGqgwLBSW__ : 4;
        
        osal_u32 _GxxOCROCvzwsmOzP2ORlImw_: 4;
        
        osal_u32 _CROyzo4OvttsmR_        : 3;
        
        osal_u32 _6Lwuio_                : 1;
        
        osal_u32 _uoow6LwWhwNBhFwjGhFy_  : 1;
        
        osal_u32 _uoow6L6wjBLwCWo_       : 1;
        osal_u32                         : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_pll31_union;

typedef union {
    
    struct {
        osal_u32 __WCqwy_                : 1;
        
        osal_u32 _GxxOCROqzlRORlImwOCRCZ_: 3;
        
        osal_u32 _uoow6LwTGBLwLBSW_w6L6D_: 3;
        
        osal_u32 _uoow6LwTGBLwLBSW_w6L6U_: 3;
        
        osal_u32 _uoow6LwTGBLwLBSW_w6L60X_: 3;
        
        osal_u32 _uoow6LwTGBLwLBSW_w6L6kV_: 3;
        osal_u32                         : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_pll32_union;

typedef union {
    
    struct {
        osal_u32 __WCqwy_                : 1;
        
        osal_u32 _GxxOCROqzlRORlImwOCRtZ_: 3;
        
        osal_u32 _uoow6LwTGBLwLBSW_w6LND_: 3;
        
        osal_u32 _uoow6LwTGBLwLBSW_w6LNU_: 3;
        
        osal_u32 _uoow6LwTGBLwLBSW_w6LN0X_: 3;
        
        osal_u32 _uoow6LwTGBLwLBSW_w6L60_: 3;
        osal_u32                         : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_pll33_union;

typedef union {
    
    struct {
        osal_u32 __WCqwy_                  : 1;
        
        osal_u32 _6Lw6L_owq6iw6L_          : 2;
        
        osal_u32 _uoow6LwCTGBLwLBSW_wq6iw6L_: 3;
        
        osal_u32 _moOCROsqzlROPCvOCR_      : 1;
        
        osal_u32 _CROCRwxOzzCOCR_          : 2;
        
        osal_u32 _GxxOCROsqzlRORlImwOzzCOCR_: 3;
        
        osal_u32 _Whw6LwCTGBLwGG6w6L_      : 1;
        
        osal_u32 _uoow6LwTGBLwLBSW_w6LN0_  : 3;
        osal_u32                           : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_pll34_union;

typedef union {
    
    struct {
        osal_u32 __WCqwy_           : 3;
        
        osal_u32 _szIGxlo2OqzlRORlIm_: 5;
        
        osal_u32 _RsmRRxmOtxxOCG_   : 8;
        osal_u32                    : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_pll35_union;

typedef union {
    
    struct {
        osal_u32 __WCqwy_          : 2;
        
        osal_u32 _Nqw6oKw6L_o_     : 2;
        
        osal_u32 _4OGAzsmOsxlGOsRmG_: 2;
        
        osal_u32 _FwuQGCWwCoBuwuio_: 1;
        
        osal_u32 _4OsmxOIzsAOvw4mw_: 1;
        
        osal_u32 _4Os4IvO4mxzQ_    : 2;
        
        osal_u32 _LdhWwBhLwhFBq_   : 2;
        
        osal_u32 _WhwBhLwFBLQW__   : 1;
        
        osal_u32 _WhwFCSwBhL_      : 1;
        
        osal_u32 _CFSwWh_          : 2;
        osal_u32                   : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_pll36_union;

typedef union {
    
    struct {
        
        osal_u32 _FwCBghwqCGSuwFWoGt_     : 2;
        
        osal_u32 _FL6w6iFWwCFSwSdowuiCLwWh_: 2;
        
        osal_u32 _FL6w6iFWwCFSwSdowu_WwWh_: 2;
        
        osal_u32 _CxmzwO4RCO2zloOCzx_     : 1;
        
        osal_u32 _CxmzwOCdwmtO4CC_        : 1;
        
        osal_u32 _wmxvz4O4RCO2zloOCzx_    : 1;
        
        osal_u32 _wmxvz4OCdwmtO4CC_       : 1;
        
        osal_u32 _moOPvsOCzx_             : 2;
        
        osal_u32 _FL6wg6wWh_              : 2;
        
        osal_u32 _CdwmtO4CCOmo_           : 2;
        osal_u32                          : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_pll37_union;

typedef union {
    
    struct {
        osal_u32 __WCqwy_               : 4;
        
        osal_u32 _moOCzxOzxxOvoCm_      : 1;
        
        osal_u32 _FwFL6w6iFWw6GowFWoGt_ : 3;
        
        osal_u32 _FwFL6w6iFWwFWoGt_     : 2;
        
        osal_u32 _4O4RCOylRs_           : 2;
        
        osal_u32 _txlGOPvsOCzxOGvxzwlRQ_: 1;
        
        osal_u32 _txlGO4RCO2zloOGvxzwlRQ_: 1;
        
        osal_u32 _NoBuwFL6w6Go_         : 1;
        
        osal_u32 _txlGOCdwmtO4CCOCzx_   : 1;
        osal_u32                        : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_pll38_union;

typedef union {
    
    struct {
        osal_u32 __WCqwy_              : 8;
        
        osal_u32 _Ct2OzozO4zRzOxvCd_   : 1;
        
        osal_u32 _Ct2O4RCO2zloOCzxOxvCd_: 1;
        
        osal_u32 _Ct2OCdwmtO4CCOxvCd_  : 1;
        
        osal_u32 _xstwO4lRAmwO2zlo_    : 4;
        
        osal_u32 _WhwoCN_wFBLQW__      : 1;
        osal_u32                       : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_pll39_union;

typedef union {
    
    struct {
        osal_u32 __WCqwy_                : 4;
        
        osal_u32 _zxGAzOxIsOCdwmt4CCOxvq_: 4;
        
        osal_u32 _GouQGwoSCw6K_WNF66wQBgQ_: 4;
        osal_u32 _wmsPOr_                : 2;
        
        osal_u32 _F66w6GowGouQGw6ihL_io_ : 2;
        osal_u32                         : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_pll40_union;

typedef union {
    
    struct {
        osal_u32 __WCqwy_              : 4;
        
        osal_u32 _GouQGwoSCwFL6wg6woiT_: 4;
        
        osal_u32 _zxGAzOxIsO4RCO2COAl2A_: 4;
        osal_u32 _wmsPOr_              : 2;
        
        osal_u32 _4RCOCzxOzxGAzOCvoRwvx_: 2;
        osal_u32                       : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_pll41_union;

typedef union {
    
    struct {
        osal_u32 __WCqwy_              : 4;
        
        osal_u32 _GouQGwqiCw6GowoiT_   : 4;
        
        osal_u32 _zxGAzOPvsOCzxOAl2A_  : 4;
        osal_u32 _wmsPOr_              : 2;
        
        osal_u32 _qiCw6GowGouQGw6ihL_io_: 2;
        osal_u32                       : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_pll42_union;

typedef union {
    
    struct {
        osal_u32 __WCqwy_            : 1;
        
        osal_u32 _4Owm2OCdwmtO4CCOIsy_: 15;
        osal_u32                     : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_pll43_union;

typedef union {
    
    struct {
        osal_u32 __WCqwy_            : 2;
        
        osal_u32 _4Owm2O4RCO2zloOCzx_: 10;
        
        osal_u32 _4Owm2OCdwmtO4CCOxsy_: 4;
        osal_u32                     : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_pll44_union;

typedef union {
    
    struct {
        
        osal_u32 _SGhdGow6K_WNwF66_         : 1;
        
        osal_u32 _Fw_Wgw6K_WNwF66wSCjwSGhdGo_: 15;
        osal_u32                            : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_pll45_union;

typedef union {
    
    struct {
        
        osal_u32 _SGhdGowFL6wgGBhw6Go_      : 1;
        osal_u32 __WCqwy_                   : 1;
        
        osal_u32 _Fw_WgwFL6wgGBhw6GowSGhdGo_: 10;
        
        osal_u32 _Fw_Wgw6K_WNwF66woCjwSGhdGo_: 4;
        osal_u32                            : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_pll46_union;

typedef union {
    
    struct {
        osal_u32 __WCqwy_              : 4;
        
        osal_u32 _Fw_WgwFL6w6iFWwSGhdGo_: 11;
        
        osal_u32 _SGhdGowFL6w6iFW_     : 1;
        osal_u32                       : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_pll47_union;

typedef union {
    
    struct {
        osal_u32 __WCqwy_ : 10;
        
        osal_u32 _C_GSwCSu_: 6;
        osal_u32          : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_pll48_union;

typedef union {
    
    struct {
        
        osal_u32 _qzlRORlImOzxGAzOxIsO4RCO2COAl2A_ : 6;
        
        osal_u32 _TGBLwLBSWwGouQGwoSCw6K_WNF66wQBgQ_: 6;
        
        osal_u32 _TGBLwLBSWwF66w6GowNB_CL_         : 4;
        osal_u32                                   : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_pll49_union;

typedef union {
    
    struct {
        osal_u32 __WCqwy_                     : 5;
        
        osal_u32 _GxxOx4OqzlRORlIm_           : 5;
        
        osal_u32 _qzlRORlImOzxGAzOPvsOCzxOAl2A_: 6;
        osal_u32                              : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_pll50_union;

typedef union {
    
    struct {
        osal_u32 __WCqwy_             : 8;
        
        osal_u32 _qzlRORlImOy2OCzxOvSR_: 8;
        osal_u32                      : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_pll51_union;

typedef union {
    
    struct {
        
        osal_u32 _6LwjGhFw_j_: 11;
        
        osal_u32 _x4OxvCdOCoR_: 4;
        
        osal_u32 _x4O4l2O4vom_: 1;
        osal_u32             : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_pll52_union;

typedef union {
    
    struct {
        
        osal_u32 _tsIOsRzRmOtlozx_    : 4;
        osal_u32 __WCqwy_             : 1;
        
        osal_u32 _jgw6Gow_WGFtwNoGg_  : 1;
        
        osal_u32 _6oiCWw_WGFtwCSuwNoGg_: 1;
        
        osal_u32 _CxvsmOwmz4QOtxxOtxz2_: 1;
        
        osal_u32 _GAzsmOwmz4QOtxz2_   : 1;
        
        osal_u32 _PCvOCROmo4Otxz2_    : 1;
        
        osal_u32 _GG6w6LwWhFwNoGg_    : 1;
        
        osal_u32 _qzwIOwmz4QOtxz2_    : 1;
        
        osal_u32 _6LwWhFwNoGg_        : 1;
        
        osal_u32 _6LNwFihWwNoGg_      : 1;
        
        osal_u32 _6L6wFihWwNoGg_      : 1;
        
        osal_u32 _uoowoi6KwBhFB6GLi__ : 1;
        osal_u32                      : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_pll53_union;

typedef union {
    
    struct {
        
        osal_u32 _N_WRwGqg_: 15;
        
        osal_u32 _GqgwWhF_: 1;
        osal_u32          : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_pll54_union;

typedef union {
    
    struct {
        
        osal_u32 _N_WRwGqgwGFF_: 5;
        osal_u32 __WCqwy_     : 11;
        osal_u32              : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_pll55_union;

typedef union {
    
    struct {
        osal_u32 __WCqwy_               : 1;
        
        osal_u32 _4Owm2OCdwmtO4CCOIsyOwy_: 15;
        osal_u32                        : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_pll56_union;

typedef union {
    
    struct {
        osal_u32 __WCqwy_               : 2;
        
        osal_u32 _4Owm2O4RCO2zloOCzxOwy_: 10;
        
        osal_u32 _4Owm2OCdwmtO4CCOxsyOwy_: 4;
        osal_u32                        : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_pll57_union;

typedef union {
    
    struct {
        osal_u32 __WCqwy_          : 5;
        
        osal_u32 _4Owm2O4RCOCv4mOwy_: 11;
        osal_u32                   : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_pll58_union;

typedef union {
    
    struct {
        osal_u32 __WCqwy_   : 6;
        
        osal_u32 _CFSiw_j_  : 9;
        
        osal_u32 _FWoLGwqw_j_: 1;
        osal_u32            : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_pll59_union;

#endif 
