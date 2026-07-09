/*
 * Copyright (c) CompanyNameMagicTag 2022-2022. All rights reserved.
 * Description: tr5330 rf reg interface and struct adaption
 * Date: 2022-11-12
*/

#ifndef __RF_REG_ABB_C_UNION_DEFINE_H__
#define __RF_REG_ABB_C_UNION_DEFINE_H__

typedef union {
    
    struct {
        
        osal_u32 _4ORvGOwtOyzo4OxvCzx_ : 2;
        osal_u32 __WCqwy_              : 1;
        
        osal_u32 _FwTjwSiFWwCWowoi6Go_ : 1;
        
        osal_u32 _4OqyOxGtOyqOxvCzx_   : 2;
        osal_u32 _wmsPOr_              : 2;
        
        osal_u32 _FwTjwouNwSiFWwoi6Go_ : 1;
        
        osal_u32 _FwTjwLPwWhwoi6Go_    : 1;
        
        osal_u32 _4OqyOlGzOmoOxvCzx_   : 1;
        
        osal_u32 _FwTjw_PwWhwoi6Go_    : 1;
        
        osal_u32 _FwTjwNjwWhwoi6Go_    : 2;
        
        osal_u32 _4OqyOtyOwCCzxOmo_    : 1;
        
        osal_u32 _4OqyOwtzyyOR8l6CzxOmo_: 1;
        osal_u32                       : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb0_union;

typedef union {
    
    struct {
        
        osal_u32 _4OqyOGzOylzsOCRwxOxvCzx_: 3;
        osal_u32 __WCqwy_                : 1;
        
        osal_u32 _FwTjwouNwgGBhwoi6Go_   : 3;
        osal_u32 _wmsPOr_                : 9;
        osal_u32                         : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb1_union;

typedef union {
    
    struct {
        
        osal_u32 _4OqyOtyO2zloOxvCzx_: 3;
        osal_u32 __WCqwy_            : 1;
        
        osal_u32 _FwTjwNjwuQGCWwoi6Go_: 1;
        osal_u32 _wmsPOr_            : 3;
        
        osal_u32 _4OqyOP2zO2zloOxvCzx_: 5;
        osal_u32 resv_2              : 3;
        osal_u32                     : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb2_union;

typedef union {
    
    struct {
        
        osal_u32 _4OqyOxozO2zloOxvCzx_: 3;
        osal_u32 __WCqwy_            : 13;
        osal_u32                     : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb3_union;

typedef union {
    
    struct {
        
        osal_u32 _4OqyO4CvCOlOxvCzx_: 8;
        
        osal_u32 _4OqyO4CvCO6OxvCzx_: 8;
        osal_u32                   : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb4_union;

typedef union {
    
    struct {
        
        osal_u32 _4OqyOwtORmIGOCv4mOIzGOxvCzx_: 1;
        osal_u32 __WCqwy_                    : 3;
        
        osal_u32 _FwTjwiqW_wLWSuwu_Lwoi6Go_  : 1;
        osal_u32 _wmsPOr_                    : 11;
        osal_u32                             : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb5_union;

typedef union {
    
    struct {
        
        osal_u32 _FwLiuw_NwjGhFwSiFW_   : 1;
        
        osal_u32 _4OqyOIv4mOsmxOIv4m_   : 1;
        
        osal_u32 _FwTjwouNwjTwSiFW_     : 1;
        osal_u32 __WCqwy_               : 1;
        
        osal_u32 _4OqyOxGtOIv4mOCRwxOyQ_: 1;
        
        osal_u32 _4OqyOxGtOIv4mOIv4m_   : 1;
        
        osal_u32 _4OqyOR8OmoOIv4m_      : 1;
        
        osal_u32 _FwTjwBuGwWhwSiFW_     : 1;
        
        osal_u32 _4OqyOw8OmoOIv4m_      : 1;
        
        osal_u32 _4OqyOtyOmoOIv4m_      : 1;
        osal_u32 _wmsPOr_               : 2;
        
        osal_u32 _FwTjwuGwjBGCw6L_owSiFW_: 1;
        
        osal_u32 _4OqyOxGtO2zloOIv4m_   : 1;
        
        osal_u32 _FwTjwNjwgGBhwSiFW_    : 1;
        
        osal_u32 _4OqyOtyOGAzsmOIv4m_   : 1;
        osal_u32                        : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb6_union;

typedef union {
    
    struct {
        
        osal_u32 _FwTjwqgGwgGBhwSiFW_    : 1;
        
        osal_u32 _FwTjwohGwgGBhwSiFW_    : 1;
        osal_u32 __WCqwy_                : 2;
        
        osal_u32 _FwTjwF6i6wBwSiFW_      : 1;
        
        osal_u32 _FwTjwF6i6wRwSiFW_      : 1;
        
        osal_u32 _4OqyORmIGOCv4mOIv4m_   : 1;
        
        osal_u32 _4OqyOvPmwORmIGOGwROIv4m_: 1;
        osal_u32 _wmsPOr_                : 8;
        osal_u32                         : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb7_union;

typedef union {
    
    struct {
        
        osal_u32 _4OqyOwtOxv2moOssyOmo_          : 1;
        
        osal_u32 _4OqyOwtOxv2moOxvyStOxvZR8Omo_  : 1;
        
        osal_u32 _4OqyOwtOxv2moOxvyStOxvZw8Omo_  : 1;
        
        osal_u32 _FwTjw_NwoigWhwCCjwqFWLwWh_     : 1;
        
        osal_u32 _FwTjw_NwoigWhwoiThiBCWwWhwLiwoC_: 1;
        
        osal_u32 _FwTjw_NwLPwBjgwuGwWh_          : 1;
        
        osal_u32 _FwTjw_NwLPwBjgwuuGwWh_         : 1;
        
        osal_u32 _4OqyOwtOR8OlGRzROGzOmo_        : 1;
        
        osal_u32 _4OqyOwtOR8OlGRzROGGzOmo_       : 1;
        
        osal_u32 _4OqyOwtOR8OGzOmo_              : 1;
        
        osal_u32 _4OqyOwtOR8OGzOylzsOmo_         : 1;
        
        osal_u32 _4OqyOwtOR8OGGzOmo_             : 1;
        
        osal_u32 _4OqyOwtOR8OGGzOylzsOmo_        : 1;
        
        osal_u32 _4OqyOwtOR8OSGCOmo_             : 1;
        
        osal_u32 _FwTjw_NwLPwuGwGFjwWh_          : 1;
        
        osal_u32 _FwTjw_NwLPwoiFBqwWh_           : 1;
        osal_u32                                 : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb8_union;

typedef union {
    
    struct {
        
        osal_u32 _FwTjw_Nw_PwohGwWh_           : 1;
        
        osal_u32 _4OqyOwtOw8OxozOyStOmo_       : 1;
        
        osal_u32 _FwTjw_Nw_PwohGwCTwGiwWhwLiwoC_: 1;
        
        osal_u32 _4OqyOwtOw8OylzsOzvOmo_       : 1;
        
        osal_u32 _FwTjw_NwL_PwL_PCTwWh_        : 1;
        
        osal_u32 _FwTjw_NwL_PwBR6GowWh_        : 1;
        
        osal_u32 _FwTjw_Nw_PwoiFBqwWh_         : 1;
        
        osal_u32 _4OqyOwtOw8OxvylzsOmo_        : 1;
        
        osal_u32 _FwTjw_Nw_PwLBGwWh_           : 1;
        
        osal_u32 _FwTjw_NwL_PwBR6GowBjBGCwWh_  : 1;
        
        osal_u32 _4OqyOwtORw8Ol6CzxOyStOmo_    : 1;
        osal_u32 __WCqwy_                      : 1;
        
        osal_u32 _4OqxOGzOP4mROmo_             : 1;
        
        osal_u32 _4OqxOGGzOP4mROmo_            : 1;
        osal_u32 _wmsPOr_                      : 2;
        osal_u32                               : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb9_union;

typedef union {
    
    struct {
        
        osal_u32 _FwTjw_NGjjwGjjwjBGCwWh_     : 1;
        
        osal_u32 _FwTjw_NGjjwLPwNBoLW_wWh_    : 1;
        
        osal_u32 _4OqyOwtzyyOR8OxGtO4CCzxOlOmo_: 1;
        
        osal_u32 _4OqyOwtzyyOR8OxGtO4CCzxO6Omo_: 1;
        
        osal_u32 _FwTjw_NGjjw_PwNBoLW_wWh_    : 1;
        
        osal_u32 _FwTjw_NGjjw_PwqgGwBwWh_     : 1;
        
        osal_u32 _FwTjw_NGjjw_PwqgGwRwWh_     : 1;
        
        osal_u32 _4OqyOwtzyyOw8O4CvCOmo_      : 1;
        
        osal_u32 _4OqyOwtzyyOw8Ol6CzxOmo_     : 1;
        
        osal_u32 _FwTjw_NGjjwLPwLBGwWh_       : 1;
        
        osal_u32 _4OqyOwtzyyOG4yStOmo_        : 1;
        osal_u32 __WCqwy_                     : 5;
        osal_u32                              : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb10_union;

typedef union {
    
    struct {
        
        osal_u32 _4OqyOwtOxv2moOssyORmsR_       : 1;
        
        osal_u32 _4OqyOwtOxv2moOxvyStOxvZR8ORmsR_: 1;
        
        osal_u32 _4OqyOwtOxv2moOxvyStOxvZw8ORmsR_: 1;
        
        osal_u32 _FwTjw_NwoigWhwCCjwqFWLwLWCL_  : 1;
        osal_u32 __WCqwy_                       : 1;
        
        osal_u32 _FwTjw_NwLPwBjgwuGwLWCL_       : 1;
        
        osal_u32 _FwTjw_NwLPwBjgwuuGwLWCL_      : 1;
        
        osal_u32 _4OqyOwtOR8OlGRzROGzORmsR_     : 1;
        
        osal_u32 _4OqyOwtOR8OlGRzROGGzORmsR_    : 1;
        
        osal_u32 _4OqyOwtOR8OGzORmsR_           : 1;
        
        osal_u32 _4OqyOwtOR8OGzOylzsORmsR_      : 1;
        
        osal_u32 _4OqyOwtOR8OGGzORmsR_          : 1;
        
        osal_u32 _4OqyOwtOR8OGGzOylzsORmsR_     : 1;
        
        osal_u32 _4OqyOwtOR8OSGCORmsR_          : 1;
        
        osal_u32 _FwTjw_NwLPwuGwGFjwLWCL_       : 1;
        
        osal_u32 _FwTjw_NwLPwoiFBqwLWCL_        : 1;
        osal_u32                                : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb11_union;

typedef union {
    
    struct {
        
        osal_u32 _FwTjw_Nw_PwohGwLWCL_        : 1;
        
        osal_u32 _4OqyOwtOw8OxozOyStORmsR_    : 1;
        osal_u32 __WCqwy_                     : 1;
        
        osal_u32 _4OqyOwtOw8OylzsORmsR_       : 1;
        
        osal_u32 _FwTjw_NwL_PwL_PCTwLWCL_     : 1;
        
        osal_u32 _FwTjw_NwL_PwBR6GowLWCL_     : 1;
        
        osal_u32 _FwTjw_Nw_PwoiFBqwLWCL_      : 1;
        
        osal_u32 _4OqyOwtOw8OxvylzsORmsR_     : 1;
        
        osal_u32 _FwTjw_Nw_PwLBGwLWCL_        : 1;
        
        osal_u32 _FwTjw_NwL_PwBR6GowBjBGCwLWCL_: 1;
        
        osal_u32 _4OqyOwtORw8Ol6CzxOyStORmsR_ : 1;
        osal_u32 _wmsPOr_                     : 1;
        
        osal_u32 _4OqxOGzOP4mRORmsROmo_       : 1;
        
        osal_u32 _4OqxOGGzOP4mRORmsROmo_      : 1;
        osal_u32 resv_2                       : 2;
        osal_u32                              : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb12_union;

typedef union {
    
    struct {
        
        osal_u32 _FwTjw_NGjjwGjjwjBGCwLWCL_     : 1;
        
        osal_u32 _FwTjw_NGjjwLPwNBoLW_wLWCL_    : 1;
        
        osal_u32 _4OqyOwtzyyOR8OxGtO4CCzxOlORmsR_: 1;
        
        osal_u32 _4OqyOwtzyyOR8OxGtO4CCzxO6ORmsR_: 1;
        
        osal_u32 _FwTjw_NGjjw_PwNBoLW_wLWCL_    : 1;
        
        osal_u32 _FwTjw_NGjjw_PwqgGwBwLWCL_     : 1;
        
        osal_u32 _FwTjw_NGjjw_PwqgGwRwLWCL_     : 1;
        
        osal_u32 _4OqyOwtzyyOw8O4CvCORmsR_      : 1;
        
        osal_u32 _FwTjw_NGjjwLPwLBGwLWCL_       : 1;
        
        osal_u32 _4OqyOwtzyyOG4yStORmsR_        : 1;
        osal_u32 __WCqwy_                       : 6;
        osal_u32                                : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb13_union;

typedef union {
    
    struct {
        
        osal_u32 _FwTjwGF6w6L_oy_: 16;
        osal_u32                : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb14_union;

typedef union {
    
    struct {
        
        osal_u32 _4OqyOz4COCRwxr_: 16;
        osal_u32                : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb15_union;

typedef union {
    
    struct {
        
        osal_u32 _FwTjwFG6w6L_oy_: 16;
        osal_u32                : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb16_union;

typedef union {
    
    struct {
        
        osal_u32 _4OqyO4zCOCRwxr_: 16;
        osal_u32                : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb17_union;

typedef union {
    
    struct {
        
        osal_u32 _FwTjwLCLwCTw6L_o0_: 16;
        osal_u32                   : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb18_union;

typedef union {
    
    struct {
        
        osal_u32 _4OqyORsROsqOCRwxZ_: 16;
        osal_u32                   : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb19_union;

typedef union {
    
    struct {
        
        osal_u32 _4OqxOR8OwCOCv4mOl_: 6;
        osal_u32 __WCqwy_          : 2;
        
        osal_u32 _4OqxOR8OwCOCv4mO6_: 6;
        osal_u32 _wmsPOr_          : 2;
        osal_u32                   : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb20_union;

typedef union {
    
    struct {
        
        osal_u32 _4OqxOw8OwCOCv4mOl_: 6;
        osal_u32 __WCqwy_          : 2;
        
        osal_u32 _4OqxOw8OwCOCv4mO6_: 6;
        osal_u32 _wmsPOr_          : 2;
        osal_u32                   : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb21_union;

typedef union {
    
    struct {
        
        osal_u32 _FwjLwLPw_6w6iFWwB_: 6;
        osal_u32 __WCqwy_          : 2;
        
        osal_u32 _FwjLwLPw_6w6iFWwR_: 6;
        osal_u32 _wmsPOr_          : 2;
        osal_u32                   : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb22_union;

typedef union {
    
    struct {
        
        osal_u32 _FwjLw_Pw_6w6iFWwB_: 6;
        osal_u32 __WCqwy_          : 2;
        
        osal_u32 _FwjLw_Pw_6w6iFWwR_: 6;
        osal_u32 _wmsPOr_          : 2;
        osal_u32                   : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb23_union;

typedef union {
    
    struct {
        
        osal_u32 _4OqxOwtOw8OCOCv4m_: 3;
        osal_u32 __WCqwy_          : 1;
        
        osal_u32 _FwTow_Nw_Pw_w6iFW_: 3;
        osal_u32 _wmsPOr_          : 1;
        
        osal_u32 _FwjLw_Nw_Pw6w6iFW_: 3;
        osal_u32 resv_2            : 1;
        
        osal_u32 _4OyROwtOw8OwOCv4m_: 3;
        osal_u32 __WCqwk_          : 1;
        osal_u32                   : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb24_union;

typedef union {
    
    struct {
        
        osal_u32 _FwTjw_NwL_PwBR6Gow_66iFWwLiwoC_: 4;
        
        osal_u32 _FwTjw_NwL_PwBR6Gow6C6iFWwLiwoC_: 4;
        
        osal_u32 _4OqyOwtORw8Ol6CzxOCGCv4mORvOxs_: 4;
        osal_u32 __WCqwy_                       : 4;
        osal_u32                                : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb25_union;

typedef union {
    
    struct {
        
        osal_u32 _FwLiuw6L_owL_PCTwCTGuwWh_           : 1;
        osal_u32 __WCqwy_                             : 3;
        
        osal_u32 _4OqyOwtzyyOw8O4CvCOsqzGORvOxs_      : 1;
        
        osal_u32 _4OqyOwtzyyOw8O4CvCOl6OsqzGORvOxs_   : 1;
        osal_u32 _wmsPOr_                             : 2;
        
        osal_u32 _4OqyOwtzyyOR8OxGtO4CCzxOsqzGOlORvOxs_: 1;
        
        osal_u32 _4OqyOwtzyyOR8OxGtO4CCzxOsqzGO6ORvOxs_: 1;
        osal_u32 resv_2                               : 6;
        osal_u32                                      : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb26_union;

typedef union {
    
    struct {
        
        osal_u32 _4OqyOwtzyyOR8OxGtOyq_               : 2;
        
        osal_u32 _FwTjw_NGjjwLPwouNwjTw00jwCWo_       : 2;
        
        osal_u32 _FwTjw_NGjjwLPwouNwjTwBjBGCw00jwqGodW_: 3;
        osal_u32 __WCqwy_                             : 9;
        osal_u32                                      : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb27_union;

typedef union {
    
    struct {
        
        osal_u32 _4OqyOtyOw8wtOmoOxSR_  : 4;
        
        osal_u32 _4OqyOtyOGzO4mROmoOxSR_: 4;
        
        osal_u32 _4OqyOtyOGGzO4mROmoOxSR_: 4;
        osal_u32 __WCqwy_               : 4;
        osal_u32                        : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb28_union;

typedef union {
    
    struct {
        
        osal_u32 _4OqyOtyOGzOmoOxSR_  : 4;
        
        osal_u32 _4OqyOtyOR8wtOmoOxSR_: 4;
        
        osal_u32 _4OqyOtyORw8sqOmoOxSR_: 4;
        
        osal_u32 _4OqyOtyOl6CzxOmoOxSR_: 4;
        osal_u32                      : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb29_union;

typedef union {
    
    struct {
        
        osal_u32 _FwTjwNjwgGBhywohGwodL_: 3;
        osal_u32 __WCqwy_              : 4;
        
        osal_u32 _FwTjwNjwgGBhywqgGwodL_: 5;
        osal_u32 _wmsPOr_              : 4;
        osal_u32                       : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb30_union;

typedef union {
    
    struct {
        
        osal_u32 _4OqyOtyO2zlorOxozOxSR_: 3;
        osal_u32 __WCqwy_              : 4;
        
        osal_u32 _4OqyOtyO2zlorOP2zOxSR_: 5;
        osal_u32 _wmsPOr_              : 4;
        osal_u32                       : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb31_union;

typedef union {
    
    struct {
        
        osal_u32 _FwTjwNjwgGBhVwohGwodL_: 3;
        osal_u32 __WCqwy_              : 4;
        
        osal_u32 _FwTjwNjwgGBhVwqgGwodL_: 5;
        osal_u32 _wmsPOr_              : 4;
        osal_u32                       : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb32_union;

typedef union {
    
    struct {
        
        osal_u32 _FwTjwNjwgGBhkwohGwodL_: 3;
        osal_u32 __WCqwy_              : 4;
        
        osal_u32 _FwTjwNjwgGBhkwqgGwodL_: 5;
        osal_u32 _wmsPOr_              : 4;
        osal_u32                       : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb33_union;

typedef union {
    
    struct {
        
        osal_u32 _4OqyOtyO2zloKOxozOxSR_: 3;
        osal_u32 __WCqwy_              : 4;
        
        osal_u32 _4OqyOtyO2zloKOP2zOxSR_: 5;
        osal_u32 _wmsPOr_              : 4;
        osal_u32                       : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb34_union;

typedef union {
    
    struct {
        
        osal_u32 _FwTjwNjwgGBhxwohGwodL_: 3;
        osal_u32 __WCqwy_              : 4;
        
        osal_u32 _FwTjwNjwgGBhxwqgGwodL_: 5;
        osal_u32 _wmsPOr_              : 4;
        osal_u32                       : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb35_union;

typedef union {
    
    struct {
        
        osal_u32 _4OqyOtyO2zloYOxozOxSR_: 3;
        osal_u32 __WCqwy_              : 4;
        
        osal_u32 _4OqyOtyO2zloYOP2zOxSR_: 5;
        osal_u32 _wmsPOr_              : 4;
        osal_u32                       : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb36_union;

typedef union {
    
    struct {
        
        osal_u32 _FwTjwNjwgGBhpwohGwodL_: 3;
        osal_u32 __WCqwy_              : 4;
        
        osal_u32 _FwTjwNjwgGBhpwqgGwodL_: 5;
        osal_u32 _wmsPOr_              : 4;
        osal_u32                       : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb37_union;

typedef union {
    
    struct {
        
        osal_u32 _FwTjw_NwoigWhwCCjwFBqUg_      : 4;
        
        osal_u32 _4OqyOwtOxv2moOxvyStOxvZR8ORwlI_: 4;
        
        osal_u32 _4OqyOwtOxv2moOxvyStOxvZw8ORwlI_: 4;
        
        osal_u32 _FwTjw_NwoigWhwCCjwq6L_o0_     : 4;
        osal_u32                                : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb38_union;

typedef union {
    
    struct {
        
        osal_u32 _4OqyOwtOxv2moOssyORSomOy3OxSR_: 4;
        
        osal_u32 _FwTjw_NwoigWhwCCjwLdhWwj0wodL_: 4;
        
        osal_u32 _4OqyOwtOxv2moOssyORSomOyZOxSR_: 4;
        osal_u32 __WCqwy_                      : 4;
        osal_u32                               : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb39_union;

typedef union {
    
    struct {
        
        osal_u32 _FwTjw_NwoigWhwCCjwBNjwLPwodL_ : 4;
        
        osal_u32 _FwTjw_NwoigWhwCCjwBNjw_PwodL_ : 4;
        
        osal_u32 _4OqyOwtOxv2moOssyO2zloOR8ORwlI_: 3;
        osal_u32 __WCqwy_                       : 1;
        
        osal_u32 _4OqyOwtOxv2moOssyO2zloOw8ORwlI_: 3;
        
        osal_u32 _4OqyOwtOxv2moOssyOlCRwxOsmx_  : 1;
        osal_u32                                : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb40_union;

typedef union {
    
    struct {
        
        osal_u32 _4OqyOwtOw8Oyzo4smx_          : 2;
        osal_u32 __WCqwy_                      : 2;
        
        osal_u32 _4OqyOwtOw8OxozO2zloOsmxOxvCzx_: 3;
        osal_u32 _wmsPOr_                      : 1;
        
        osal_u32 _4OqyOwtOw8ORlzO2zloOsmxOxvCzx_: 3;
        osal_u32 resv_2                        : 1;
        
        osal_u32 _FwTjw_Nw_PwjGhFCWowGdLi_     : 1;
        osal_u32 __WCqwk_                      : 3;
        osal_u32                               : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb41_union;

typedef union {
    
    struct {
        
        osal_u32 _FwTjw_Nw_PwGdLi_                 : 1;
        
        osal_u32 _FwTjw_Nw_PwohGwQgwWhwoi6Go_      : 1;
        
        osal_u32 _FwTjw_Nw_PwohGwSgwWhwoi6Go_      : 1;
        
        osal_u32 _FwTjw_Nw_PwohGwogwWhwoi6Go_      : 1;
        
        osal_u32 _4OqyOwtOw8OxozOsRryOmoOxvCzx_    : 1;
        
        osal_u32 _4OqyOwtOw8OxozOICRSomOxvCzx_     : 3;
        
        osal_u32 _4OqyOwtOw8ORlzOwtyOwRwlIOxvCzx_  : 3;
        osal_u32 __WCqwy_                          : 1;
        
        osal_u32 _4OqyOwtORw8Ol6CzxOyStOlRwlIORvOxs_: 3;
        osal_u32 _wmsPOr_                          : 1;
        osal_u32                                   : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb42_union;

typedef union {
    
    struct {
        
        osal_u32 _FwTjw_Nw_PwohGwhdShGwoi6Go_: 4;
        
        osal_u32 _4OqyOwtOw8OxozOoSIozZOxvCzx_: 4;
        osal_u32 __WCqwy_                    : 8;
        osal_u32                             : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb43_union;

typedef union {
    
    struct {
        
        osal_u32 _4OqyOwtOw8OxozOwzRROxvCzx_    : 6;
        osal_u32 __WCqwy_                       : 2;
        
        osal_u32 _4OqyOwtOw8OxozOzRRsORwlIOxvCzx_: 6;
        osal_u32 _wmsPOr_                       : 2;
        osal_u32                                : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb44_union;

typedef union {
    
    struct {
        
        osal_u32 _FwTjw_Nw_PwohGwQgwWhwodLy_: 1;
        
        osal_u32 _4OqyOwtOw8OxozOA2OmoOxSRr_: 1;
        
        osal_u32 _FwTjw_Nw_PwohGwQgwWhwodLV_: 1;
        
        osal_u32 _FwTjw_Nw_PwohGwQgwWhwodLk_: 1;
        
        osal_u32 _4OqyOwtOw8OxozOA2OmoOxSRK_: 1;
        
        osal_u32 _FwTjw_Nw_PwohGwQgwWhwodLx_: 1;
        
        osal_u32 _4OqyOwtOw8OxozOA2OmoOxSRY_: 1;
        
        osal_u32 _FwTjw_Nw_PwohGwQgwWhwodLp_: 1;
        
        osal_u32 _FwTjw_Nw_PwohGwSgwWhwodLy_: 1;
        
        osal_u32 _4OqyOwtOw8OxozOI2OmoOxSRr_: 1;
        
        osal_u32 _FwTjw_Nw_PwohGwSgwWhwodLV_: 1;
        
        osal_u32 _FwTjw_Nw_PwohGwSgwWhwodLk_: 1;
        
        osal_u32 _4OqyOwtOw8OxozOI2OmoOxSRK_: 1;
        
        osal_u32 _FwTjw_Nw_PwohGwSgwWhwodLx_: 1;
        
        osal_u32 _4OqyOwtOw8OxozOI2OmoOxSRY_: 1;
        
        osal_u32 _FwTjw_Nw_PwohGwSgwWhwodLp_: 1;
        osal_u32                           : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb45_union;

typedef union {
    
    struct {
        
        osal_u32 _FwTjw_Nw_PwohGwogwWhwodLy_ : 1;
        
        osal_u32 _4OqyOwtOw8OxozOx2OmoOxSRr_ : 1;
        
        osal_u32 _FwTjw_Nw_PwohGwogwWhwodLV_ : 1;
        
        osal_u32 _FwTjw_Nw_PwohGwogwWhwodLk_ : 1;
        
        osal_u32 _4OqyOwtOw8OxozOx2OmoOxSRK_ : 1;
        
        osal_u32 _FwTjw_Nw_PwohGwogwWhwodLx_ : 1;
        
        osal_u32 _4OqyOwtOw8OxozOx2OmoOxSRY_ : 1;
        
        osal_u32 _FwTjw_Nw_PwohGwogwWhwodLp_ : 1;
        
        osal_u32 _4OqyOwtOw8OxozOsRryOmoOxSR3_: 1;
        
        osal_u32 _FwTjw_Nw_PwohGwCL0jwWhwodL0_: 1;
        
        osal_u32 _4OqyOwtOw8OxozOsRryOmoOxSRZ_: 1;
        
        osal_u32 _4OqyOwtOw8OxozOsRryOmoOxSRi_: 1;
        
        osal_u32 _FwTjw_Nw_PwohGwCL0jwWhwodLD_: 1;
        
        osal_u32 _4OqyOwtOw8OxozOsRryOmoOxSRa_: 1;
        
        osal_u32 _FwTjw_Nw_PwohGwCL0jwWhwodLX_: 1;
        
        osal_u32 _4OqyOwtOw8OxozOsRryOmoOxSR9_: 1;
        osal_u32                             : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb46_union;

typedef union {
    
    struct {
        
        osal_u32 _4OqyOwtOw8OxozOwzRROxSR3_: 6;
        osal_u32 __WCqwy_                 : 2;
        
        osal_u32 _FwTjw_Nw_PwohGw_GLLwodL0_: 6;
        osal_u32 _wmsPOr_                 : 2;
        osal_u32                          : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb47_union;

typedef union {
    
    struct {
        
        osal_u32 _4OqyOwtOw8OxozOwzRROxSRZ_: 6;
        osal_u32 __WCqwy_                 : 2;
        
        osal_u32 _4OqyOwtOw8OxozOwzRROxSRi_: 6;
        osal_u32 _wmsPOr_                 : 2;
        osal_u32                          : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb48_union;

typedef union {
    
    struct {
        
        osal_u32 _FwTjw_Nw_PwohGw_GLLwodLD_: 6;
        osal_u32 __WCqwy_                 : 2;
        
        osal_u32 _4OqyOwtOw8OxozOwzRROxSRa_: 6;
        osal_u32 _wmsPOr_                 : 2;
        osal_u32                          : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb49_union;

typedef union {
    
    struct {
        
        osal_u32 _FwTjw_Nw_PwohGw_GLLwodLX_: 6;
        osal_u32 __WCqwy_                 : 2;
        
        osal_u32 _4OqyOwtOw8OxozOwzRROxSR9_: 6;
        osal_u32 _wmsPOr_                 : 2;
        osal_u32                          : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb50_union;

typedef union {
    
    struct {
        
        osal_u32 _4OqyOwtOw8OxozOzRRsORwlIOxSR3_: 6;
        osal_u32 __WCqwy_                      : 2;
        
        osal_u32 _FwTjw_Nw_PwohGwGLLCwL_BSwodL0_: 6;
        osal_u32 _wmsPOr_                      : 2;
        osal_u32                               : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb51_union;

typedef union {
    
    struct {
        
        osal_u32 _4OqyOwtOw8OxozOzRRsORwlIOxSRZ_: 6;
        osal_u32 __WCqwy_                      : 2;
        
        osal_u32 _4OqyOwtOw8OxozOzRRsORwlIOxSRi_: 6;
        osal_u32 _wmsPOr_                      : 2;
        osal_u32                               : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb52_union;

typedef union {
    
    struct {
        
        osal_u32 _FwTjw_Nw_PwohGwGLLCwL_BSwodLD_: 6;
        osal_u32 __WCqwy_                      : 2;
        
        osal_u32 _4OqyOwtOw8OxozOzRRsORwlIOxSRa_: 6;
        osal_u32 _wmsPOr_                      : 2;
        osal_u32                               : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb53_union;

typedef union {
    
    struct {
        
        osal_u32 _FwTjw_Nw_PwohGwGLLCwL_BSwodLX_: 6;
        osal_u32 __WCqwy_                      : 2;
        
        osal_u32 _4OqyOwtOw8OxozOzRRsORwlIOxSR9_: 6;
        osal_u32 _wmsPOr_                      : 2;
        osal_u32                               : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb54_union;

typedef union {
    
    struct {
        
        osal_u32 _FwTjw_Nw_PwohGwhdShGwodLy_: 4;
        
        osal_u32 _4OqyOwtOw8OxozOoSIozOxSRr_: 4;
        
        osal_u32 _FwTjw_Nw_PwohGwhdShGwodLV_: 4;
        
        osal_u32 _FwTjw_Nw_PwohGwhdShGwodLk_: 4;
        osal_u32                           : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb55_union;

typedef union {
    
    struct {
        
        osal_u32 _4OqyOwtOw8OxozOoSIozOxSRK_: 4;
        
        osal_u32 _FwTjw_Nw_PwohGwhdShGwodLx_: 4;
        
        osal_u32 _4OqyOwtOw8OxozOoSIozOxSRY_: 4;
        
        osal_u32 _FwTjw_Nw_PwohGwhdShGwodLp_: 4;
        osal_u32                           : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb56_union;

typedef union {
    
    struct {
        
        osal_u32 _4OqyOwtOw8OxozOoSIozZOxSR3_: 4;
        
        osal_u32 _FwTjw_Nw_PwohGwhdShGVwodL0_: 4;
        
        osal_u32 _4OqyOwtOw8OxozOoSIozZOxSRZ_: 4;
        
        osal_u32 _4OqyOwtOw8OxozOoSIozZOxSRi_: 4;
        osal_u32                            : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb57_union;

typedef union {
    
    struct {
        
        osal_u32 _FwTjw_Nw_PwohGwhdShGVwodLD_: 4;
        
        osal_u32 _4OqyOwtOw8OxozOoSIozZOxSRa_: 4;
        
        osal_u32 _FwTjw_Nw_PwohGwhdShGVwodLX_: 4;
        
        osal_u32 _4OqyOwtOw8OxozOoSIozZOxSR9_: 4;
        osal_u32                            : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb58_union;

typedef union {
    
    struct {
        
        osal_u32 _FwTjw_Nw_PwohGwS6LdhWwjyw6x_: 3;
        osal_u32 __WCqwy_                    : 1;
        
        osal_u32 _4OqyOwtOw8OxozOICRSomOy3OCK_: 3;
        osal_u32 _wmsPOr_                    : 1;
        
        osal_u32 _FwTjw_Nw_PwohGwS6LdhWwjyw6k_: 3;
        osal_u32 resv_2                      : 1;
        
        osal_u32 _FwTjw_Nw_PwohGwS6LdhWwjyw6V_: 3;
        osal_u32 __WCqwk_                    : 1;
        osal_u32                             : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb59_union;

typedef union {
    
    struct {
        
        osal_u32 _4OqyOwtOw8OxozOICRSomOy3OCr_: 3;
        osal_u32 __WCqwy_                    : 1;
        
        osal_u32 _4OqyOwtOw8OxozOICRSomOyrOCa_: 3;
        osal_u32 _wmsPOr_                    : 1;
        
        osal_u32 _FwTjw_Nw_PwohGwS6LdhWwj0w6D_: 3;
        osal_u32 resv_2                      : 1;
        
        osal_u32 _4OqyOwtOw8OxozOICRSomOyrOCi_: 3;
        osal_u32 __WCqwk_                    : 1;
        osal_u32                             : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb60_union;

typedef union {
    
    struct {
        
        osal_u32 _4OqyOwtOw8OxozOICRSomOyrOCZ_: 3;
        osal_u32 __WCqwy_                    : 1;
        
        osal_u32 _FwTjw_Nw_PwohGwS6LdhWwj0w60_: 3;
        osal_u32 _wmsPOr_                    : 1;
        
        osal_u32 _FwTjw_Nw_PwohGwS6LdhWwjVw6x_: 3;
        osal_u32 resv_2                      : 1;
        
        osal_u32 _4OqyOwtOw8OxozOICRSomOyZOCK_: 3;
        osal_u32 __WCqwk_                    : 1;
        osal_u32                             : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb61_union;

typedef union {
    
    struct {
        
        osal_u32 _FwTjw_Nw_PwohGwS6LdhWwjVw6k_: 3;
        osal_u32 __WCqwy_                    : 1;
        
        osal_u32 _FwTjw_Nw_PwohGwS6LdhWwjVw6V_: 3;
        osal_u32 _wmsPOr_                    : 1;
        
        osal_u32 _4OqyOwtOw8OxozOICRSomOyZOCr_: 3;
        osal_u32 resv_2                      : 5;
        osal_u32                             : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb62_union;

typedef union {
    
    struct {
        
        osal_u32 _FwTjw_Nw_PwohGwhdShwLiwoC_  : 4;
        
        osal_u32 _4OqyOwtOw8OxozOoSIoZORvOxs_ : 4;
        
        osal_u32 _FwTjw_Nw_PwohGwhdShu6wLiwoC_: 4;
        
        osal_u32 _4OqyOwtOw8OxozOoSIoGCZORvOxs_: 4;
        osal_u32                              : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb63_union;

typedef union {
    
    struct {
        
        osal_u32 _FwTjw_Nw_PwohGwhdSh6wLiwoC_: 5;
        osal_u32 __WCqwy_                    : 3;
        
        osal_u32 _4OqyOwtOw8OxozOoSIoCZORvOxs_: 5;
        osal_u32 _wmsPOr_                    : 3;
        osal_u32                             : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb64_union;

typedef union {
    
    struct {
        
        osal_u32 _4OqyOwtOw8OxvylzsORwlIORvOxs_       : 4;
        
        osal_u32 _FwTjw_Nw_PwLBGw6SNjw_L_BSwLiwoC_    : 3;
        osal_u32 __WCqwy_                             : 1;
        
        osal_u32 _FwTjw_Nw_PwLBGwjBGCwBL_BSwLiwoC_    : 4;
        
        osal_u32 _4OqyOwtOw8ORlzOylzsOlRwlIOCItyORvOxs_: 4;
        osal_u32                                      : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb65_union;

typedef union {
    
    struct {
        
        osal_u32 _4OqyOwtOw8ORlzOPwmtORwlIORvOxs_: 4;
        
        osal_u32 _FwTjw_Nw_PwLBGwiuwBL_BSwLiwoC_: 2;
        osal_u32 __WCqwy_                       : 2;
        
        osal_u32 _FwTjw_Nw_PwLBGwiuw6w6GowLiwoC_: 4;
        
        osal_u32 _4OqyOwtOw8ORlzOvGOwOCzxORvOxs_: 2;
        osal_u32 _wmsPOr_                       : 2;
        osal_u32                                : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb66_union;

typedef union {
    
    struct {
        
        osal_u32 _4OqyOwtOw8ORlzOwtyOwRwlIOwa_: 3;
        osal_u32 __WCqwy_                    : 1;
        
        osal_u32 _FwTjw_Nw_PwLBGw_Njw_L_BSw_D_: 3;
        osal_u32 _wmsPOr_                    : 1;
        
        osal_u32 _4OqyOwtOw8ORlzOwtyOwRwlIOwi_: 3;
        osal_u32 resv_2                      : 1;
        
        osal_u32 _4OqyOwtOw8ORlzOwtyOwRwlIOwZ_: 3;
        osal_u32 __WCqwk_                    : 1;
        osal_u32                             : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb67_union;

typedef union {
    
    struct {
        
        osal_u32 _FwTjw_Nw_PwLBGw_Njw_L_BSw_0_: 3;
        osal_u32 __WCqwy_                    : 13;
        osal_u32                             : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb68_union;

typedef union {
    
    struct {
        
        osal_u32 _FwTow_Nw_PwohGwgGBhwodLy_: 3;
        osal_u32 __WCqwy_                 : 1;
        
        osal_u32 _4OqxOwtOw8OxozO2zloOxSRr_: 3;
        osal_u32 _wmsPOr_                 : 1;
        
        osal_u32 _FwTow_Nw_PwohGwgGBhwodLV_: 3;
        osal_u32 resv_2                   : 1;
        
        osal_u32 _FwTow_Nw_PwohGwgGBhwodLk_: 3;
        osal_u32 __WCqwk_                 : 1;
        osal_u32                          : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb69_union;

typedef union {
    
    struct {
        
        osal_u32 _4OqxOwtOw8OxozO2zloOxSRK_: 3;
        osal_u32 __WCqwy_                 : 1;
        
        osal_u32 _FwTow_Nw_PwohGwgGBhwodLx_: 3;
        osal_u32 _wmsPOr_                 : 1;
        
        osal_u32 _4OqxOwtOw8OxozO2zloOxSRY_: 3;
        osal_u32 resv_2                   : 1;
        
        osal_u32 _FwTow_Nw_PwohGwgGBhwodLp_: 3;
        osal_u32 __WCqwk_                 : 1;
        osal_u32                          : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb70_union;

typedef union {
    
    struct {
        
        osal_u32 _FwTow_Nw_PwLBGwgGBhwodLy_: 3;
        osal_u32 __WCqwy_                 : 1;
        
        osal_u32 _4OqxOwtOw8ORlzO2zloOxSRr_: 3;
        osal_u32 _wmsPOr_                 : 1;
        
        osal_u32 _FwTow_Nw_PwLBGwgGBhwodLV_: 3;
        osal_u32 resv_2                   : 1;
        
        osal_u32 _FwTow_Nw_PwLBGwgGBhwodLk_: 3;
        osal_u32 __WCqwk_                 : 1;
        osal_u32                          : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb71_union;

typedef union {
    
    struct {
        
        osal_u32 _4OqxOwtOw8ORlzO2zloOxSRK_: 3;
        osal_u32 __WCqwy_                 : 1;
        
        osal_u32 _FwTow_Nw_PwLBGwgGBhwodLx_: 3;
        osal_u32 _wmsPOr_                 : 1;
        
        osal_u32 _4OqxOwtOw8ORlzO2zloOxSRY_: 3;
        osal_u32 resv_2                   : 1;
        
        osal_u32 _FwTow_Nw_PwLBGwgGBhwodLp_: 3;
        osal_u32 __WCqwk_                 : 1;
        osal_u32                          : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb72_union;

typedef union {
    
    struct {
        
        osal_u32 _4OyROwtOw8OxozO2zloOxSR3_: 3;
        osal_u32 __WCqwy_                 : 1;
        
        osal_u32 _FwjLw_Nw_PwohGwgGBhwodL0_: 3;
        osal_u32 _wmsPOr_                 : 1;
        
        osal_u32 _4OyROwtOw8OxozO2zloOxSRZ_: 3;
        osal_u32 resv_2                   : 1;
        
        osal_u32 _4OyROwtOw8OxozO2zloOxSRi_: 3;
        osal_u32 __WCqwk_                 : 1;
        osal_u32                          : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb73_union;

typedef union {
    
    struct {
        
        osal_u32 _FwjLw_Nw_PwohGwgGBhwodLD_: 3;
        osal_u32 __WCqwy_                 : 1;
        
        osal_u32 _4OyROwtOw8OxozO2zloOxSRa_: 3;
        osal_u32 _wmsPOr_                 : 1;
        
        osal_u32 _FwjLw_Nw_PwohGwgGBhwodLX_: 3;
        osal_u32 resv_2                   : 1;
        
        osal_u32 _4OyROwtOw8OxozO2zloOxSR9_: 3;
        osal_u32 __WCqwk_                 : 1;
        osal_u32                          : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb74_union;

typedef union {
    
    struct {
        
        osal_u32 _4OyROwtOw8ORlzO2zloOxSR3_: 3;
        osal_u32 __WCqwy_                 : 1;
        
        osal_u32 _FwjLw_Nw_PwLBGwgGBhwodL0_: 3;
        osal_u32 _wmsPOr_                 : 1;
        
        osal_u32 _4OyROwtOw8ORlzO2zloOxSRZ_: 3;
        osal_u32 resv_2                   : 1;
        
        osal_u32 _4OyROwtOw8ORlzO2zloOxSRi_: 3;
        osal_u32 __WCqwk_                 : 1;
        osal_u32                          : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb75_union;

typedef union {
    
    struct {
        
        osal_u32 _FwjLw_Nw_PwLBGwgGBhwodLD_: 3;
        osal_u32 __WCqwy_                 : 1;
        
        osal_u32 _4OyROwtOw8ORlzO2zloOxSRa_: 3;
        osal_u32 _wmsPOr_                 : 1;
        
        osal_u32 _FwjLw_Nw_PwLBGwgGBhwodLX_: 3;
        osal_u32 resv_2                   : 1;
        
        osal_u32 _4OyROwtOw8ORlzO2zloOxSR9_: 3;
        osal_u32 __WCqwk_                 : 1;
        osal_u32                          : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb76_union;

typedef union {
    
    struct {
        
        osal_u32 _4OqyOwtOR8OlGRzROGzOsmx_ : 2;
        osal_u32 __WCqwy_                  : 2;
        
        osal_u32 _4OqyOwtOR8OlGRzROGGzOsmx_: 2;
        osal_u32 _wmsPOr_                  : 2;
        
        osal_u32 _FwTjw_NwLPwuGwGFjwGLLwCWo_: 3;
        osal_u32 resv_2                    : 1;
        
        osal_u32 _4OqyOwtOR8OGzOPRvGOsmx_  : 4;
        osal_u32                           : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb77_union;

typedef union {
    
    struct {
        
        osal_u32 _4OqyOwtOR8OGzOz4yOPRAOsmxOmtSsm_: 4;
        osal_u32 __WCqwy_                        : 4;
        
        osal_u32 _4OqyOwtOR8OGGzOCzGOsq_         : 5;
        osal_u32 _wmsPOr_                        : 3;
        osal_u32                                 : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb78_union;

typedef union {
    
    struct {
        
        osal_u32 _FwTjw_NwLPwuGwGFjwBidLwCWowGdLi_: 1;
        
        osal_u32 _FwTjw_NwLPwuGwqjwCWowGdLi_     : 1;
        
        osal_u32 _4OqyOwtOR8OGzOPIl4OsmxOzSRv_   : 1;
        
        osal_u32 _FwTjw_NwLPwuuGwq6GCwCWowGdLi_  : 1;
        
        osal_u32 _4OqyOwtOR8OGzOz4yOPyOsmxOzSRv_ : 1;
        
        osal_u32 _FwTjw_NwLPwuuGwqjGwCWowGdLi_   : 1;
        
        osal_u32 _FwTjw_NwLPwuuGwqj6wCWowGdLi_   : 1;
        
        osal_u32 _FwTjw_NwLPwdu6wqjwCWowGdLi_    : 1;
        
        osal_u32 _4OqyOwtOR8OGzOzIGIOPtl8OzSRv_  : 1;
        osal_u32 __WCqwy_                        : 7;
        osal_u32                                 : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb79_union;

typedef union {
    
    struct {
        
        osal_u32 _FwTjw_NwLPwuGwGFjwBidLwCWowSGhdGo_: 6;
        osal_u32 __WCqwy_                          : 2;
        
        osal_u32 _FwTjw_NwLPwuGwqjwCWowSGhdGo_     : 5;
        osal_u32 _wmsPOr_                          : 3;
        osal_u32                                   : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb80_union;

typedef union {
    
    struct {
        
        osal_u32 _4OqyOwtOR8OGzOPIl4OsmxOIzoSzx_ : 4;
        
        osal_u32 _FwTjw_NwLPwuuGwq6GCwCWowSGhdGo_: 4;
        
        osal_u32 _4OqyOwtOR8OGzOz4yOPyOsmxOIzoSzx_: 6;
        osal_u32 __WCqwy_                        : 2;
        osal_u32                                 : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb81_union;

typedef union {
    
    struct {
        
        osal_u32 _FwTjw_NwLPwuuGwqjGwCWowSGhdGo_: 5;
        osal_u32 __WCqwy_                      : 3;
        
        osal_u32 _FwTjw_NwLPwuuGwqj6wCWowSGhdGo_: 5;
        osal_u32 _wmsPOr_                      : 3;
        osal_u32                               : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb82_union;

typedef union {
    
    struct {
        
        osal_u32 _FwTjw_NwLPwdu6wqjwCWowSGhdGo_ : 4;
        
        osal_u32 _4OqyOwtOR8OGzOzIGIOPtl8OIzoSzx_: 5;
        osal_u32 __WCqwy_                       : 7;
        osal_u32                                : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb83_union;

typedef union {
    
    struct {
        
        osal_u32 _FwTjw_NGjjwLBGwq6SwL_BS_    : 4;
        
        osal_u32 _4OqyOwtzyyORlzOPCIORwlIOw8l6_: 4;
        osal_u32 __WCqwy_                     : 8;
        osal_u32                              : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb84_union;

typedef union {
    
    struct {
        
        osal_u32 _4OqyOwtzyyORlzOly2ORwlIOr3I_: 4;
        
        osal_u32 _FwTjw_NGjjwLBGwBjgwL_BSwVyS_: 4;
        
        osal_u32 _4OqyOwtzyyORlzOly2ORwlIOw8l6_: 4;
        osal_u32 __WCqwy_                     : 4;
        osal_u32                              : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb85_union;

typedef union {
    
    struct {
        
        osal_u32 _4OqyOwtzyyORlzOCC_    : 3;
        osal_u32 __WCqwy_               : 1;
        
        osal_u32 _FwTjw_NGjjwLBGw66w_PBR_: 3;
        osal_u32 _wmsPOr_               : 9;
        osal_u32                        : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb86_union;

typedef union {
    
    struct {
        
        osal_u32 _FwTjw_NGjjwLBGw_1_    : 2;
        osal_u32 __WCqwy_               : 2;
        
        osal_u32 _4OqyOwtzyyORlzOwpOw8l6_: 2;
        osal_u32 _wmsPOr_               : 10;
        osal_u32                        : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb87_union;

typedef union {
    
    struct {
        
        osal_u32 _4OqyOwtzyyORlzOCItyOw_    : 3;
        osal_u32 __WCqwy_                   : 1;
        
        osal_u32 _FwTjw_NGjjwLBGw6SNjw_w_PBR_: 3;
        osal_u32 _wmsPOr_                   : 9;
        osal_u32                            : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb88_union;

typedef union {
    
    struct {
        
        osal_u32 _4OqyOwtzyyORlzOvGrOlRwlI_    : 1;
        osal_u32 __WCqwy_                      : 3;
        
        osal_u32 _FwTjw_NGjjwLBGwiu0wBL_BSw_PBR_: 1;
        osal_u32 _wmsPOr_                      : 11;
        osal_u32                               : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb89_union;

typedef union {
    
    struct {
        
        osal_u32 _4OqyOwtzyyOtlxRmwOPCIORwlIOw8OqxOr3I_: 4;
        
        osal_u32 _FwTjw_NGjjwNBoLW_wq6SwL_BSw_PwTowVyS_: 4;
        
        osal_u32 _4OqyOwtzyyOtlxRmwOPCIORwlIOw8OyROZGaI_: 4;
        
        osal_u32 _FwTjw_NGjjwNBoLW_wq6SwL_BSw_PwjLwxS_ : 4;
        osal_u32                                       : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb90_union;

typedef union {
    
    struct {
        
        osal_u32 _FwTjw_NGjjwNBoLW_wq6SwL_BSwLPw0yS_: 4;
        
        osal_u32 _4OqyOwtzyyOtlxRmwOPCIORwlIOR8OZ3I_: 4;
        osal_u32 __WCqwy_                          : 8;
        osal_u32                                   : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb91_union;

typedef union {
    
    struct {
        
        osal_u32 _4OqyOwtzyyOtlxRmwOly2ORwlIOw8OqxOr3I_: 4;
        
        osal_u32 _FwTjw_NGjjwNBoLW_wBjgwL_BSw_PwTowVyS_: 4;
        
        osal_u32 _4OqyOwtzyyOtlxRmwOly2ORwlIOw8OyROZGaI_: 4;
        
        osal_u32 _FwTjw_NGjjwNBoLW_wBjgwL_BSw_PwjLwxS_ : 4;
        osal_u32                                       : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb92_union;

typedef union {
    
    struct {
        
        osal_u32 _FwTjw_NGjjwNBoLW_wBjgwL_BSwLPw0yS_: 4;
        
        osal_u32 _4OqyOwtzyyOtlxRmwOly2ORwlIOR8OZ3I_: 4;
        osal_u32 __WCqwy_                          : 8;
        osal_u32                                   : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb93_union;

typedef union {
    
    struct {
        
        osal_u32 _4OqyOwtzyyOtlxRmwOwlGCzxORwlIOw8OqxOr3I_: 5;
        osal_u32 __WCqwy_                                : 3;
        
        osal_u32 _FwTjw_NGjjwNBoLW_w_Bu6GowL_BSw_PwTowVyS_: 5;
        osal_u32 _wmsPOr_                                : 3;
        osal_u32                                         : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb94_union;

typedef union {
    
    struct {
        
        osal_u32 _4OqyOwtzyyOtlxRmwOwlGCzxORwlIOw8OyROZGaI_: 5;
        osal_u32 __WCqwy_                                 : 3;
        
        osal_u32 _FwTjw_NGjjwNBoLW_w_Bu6GowL_BSw_PwjLwxS_ : 5;
        osal_u32 _wmsPOr_                                 : 3;
        osal_u32                                          : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb95_union;

typedef union {
    
    struct {
        
        osal_u32 _FwTjw_NGjjwNBoLW_w_Bu6GowL_BSwLPw0yS_: 5;
        osal_u32 __WCqwy_                             : 3;
        
        osal_u32 _4OqyOwtzyyOtlxRmwOwlGCzxORwlIOR8OZ3I_: 5;
        osal_u32 _wmsPOr_                             : 3;
        osal_u32                                      : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb96_union;

typedef union {
    
    struct {
        
        osal_u32 _4OqyOwtzyyOw8OxGtOvGrOCC_: 3;
        osal_u32 __WCqwy_                 : 1;
        
        osal_u32 _FwTjw_NGjjw_PwouNwiuVw66_: 3;
        osal_u32 _wmsPOr_                 : 1;
        
        osal_u32 _4OqyOwtzyyOR8OxGtOvGrOCC_: 3;
        osal_u32 resv_2                   : 1;
        
        osal_u32 _FwTjw_NGjjwLPwouNwiuVw66_: 3;
        osal_u32 __WCqwk_                 : 1;
        osal_u32                          : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb97_union;

typedef union {
    
    struct {
        
        osal_u32 _FwTjw_NGjjw_PwouNwiu0w_1_: 2;
        
        osal_u32 _4OqyOwtzyyOw8OxGtOvGZOwp_: 2;
        
        osal_u32 _FwTjw_NGjjwLPwouNwiu0w_1_: 2;
        
        osal_u32 _4OqyOwtzyyOR8OxGtOvGZOwp_: 2;
        osal_u32 __WCqwy_                 : 8;
        osal_u32                          : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb98_union;

typedef union {
    
    struct {
        
        osal_u32 _4OqyOwtzyyOw8OxGtOvGrOCItyOw_: 3;
        osal_u32 __WCqwy_                     : 1;
        
        osal_u32 _FwTjw_NGjjw_PwouNwiuVw6SNjw__: 3;
        osal_u32 _wmsPOr_                     : 1;
        
        osal_u32 _4OqyOwtzyyOR8OxGtOvGrOCItyOw_: 3;
        osal_u32 resv_2                       : 1;
        
        osal_u32 _FwTjw_NGjjwLPwouNwiuVw6SNjw__: 3;
        osal_u32 __WCqwk_                     : 1;
        osal_u32                              : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb99_union;

typedef union {
    
    struct {
        
        osal_u32 _4OqyOwtzyyOw8OxGtOvGrOlRwlI_     : 1;
        
        osal_u32 _4OqyOwtzyyOR8OxGtOvGrOlRwlI_     : 1;
        osal_u32 __WCqwy_                          : 2;
        
        osal_u32 _FwTjw_NGjjw_PwouNwiuVwBL_BSwLiwoC_: 2;
        
        osal_u32 _FwTjw_NGjjwLPwouNwiuVwBL_BSwLiwoC_: 2;
        osal_u32 _wmsPOr_                          : 8;
        osal_u32                                   : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb100_union;

typedef union {
    
    struct {
        
        osal_u32 _FwTjw_NGjjwqgGwq6SwLiwoC_: 4;
        osal_u32 __WCqwy_                 : 12;
        osal_u32                          : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb101_union;

typedef union {
    
    struct {
        
        osal_u32 _FwTjw_NGjjwqgGwBjgwL_BSwLiwoC_: 4;
        osal_u32 __WCqwy_                      : 12;
        osal_u32                               : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb102_union;

typedef union {
    
    struct {
        
        osal_u32 _4OqyOwtzyyOP2zOlRwlIORvOxs_: 1;
        osal_u32 __WCqwy_                   : 15;
        osal_u32                            : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb103_union;

typedef union {
    
    struct {
        
        osal_u32 _4OqyOwtzyyOP2zOCCOxSR3_: 3;
        osal_u32 __WCqwy_               : 1;
        
        osal_u32 _FwTjw_NGjjwqgGw66wodL0_: 3;
        osal_u32 _wmsPOr_               : 1;
        
        osal_u32 _4OqyOwtzyyOP2zOCCOxSRZ_: 3;
        osal_u32 resv_2                 : 1;
        
        osal_u32 _4OqyOwtzyyOP2zOCCOxSRi_: 3;
        osal_u32 __WCqwk_               : 1;
        osal_u32                        : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb104_union;

typedef union {
    
    struct {
        
        osal_u32 _FwTjw_NGjjwqgGw66wodLD_: 3;
        osal_u32 __WCqwy_               : 1;
        
        osal_u32 _4OqyOwtzyyOP2zOCCOxSRa_: 3;
        osal_u32 _wmsPOr_               : 1;
        
        osal_u32 _FwTjw_NGjjwqgGw66wodLX_: 3;
        osal_u32 resv_2                 : 1;
        
        osal_u32 _4OqyOwtzyyOP2zOCCOxSR9_: 3;
        osal_u32 __WCqwk_               : 1;
        osal_u32                        : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb105_union;

typedef union {
    
    struct {
        
        osal_u32 _FwTjw_NGjjwqgGw_1wodLy_: 2;
        
        osal_u32 _4OqyOwtzyyOP2zOwpOxSRr_: 2;
        
        osal_u32 _FwTjw_NGjjwqgGw_1wodLV_: 2;
        
        osal_u32 _FwTjw_NGjjwqgGw_1wodLk_: 2;
        
        osal_u32 _4OqyOwtzyyOP2zOwpOxSRK_: 2;
        
        osal_u32 _FwTjw_NGjjwqgGw_1wodLx_: 2;
        
        osal_u32 _4OqyOwtzyyOP2zOwpOxSRY_: 2;
        
        osal_u32 _FwTjw_NGjjwqgGw_1wodLp_: 2;
        osal_u32                        : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb106_union;

typedef union {
    
    struct {
        
        osal_u32 _4OqyOwtzyyOP2zOCItyOwOxSR3_: 3;
        osal_u32 __WCqwy_                   : 1;
        
        osal_u32 _FwTjw_NGjjwqgGw6SNjw_wodL0_: 3;
        osal_u32 _wmsPOr_                   : 1;
        
        osal_u32 _4OqyOwtzyyOP2zOCItyOwOxSRZ_: 3;
        osal_u32 resv_2                     : 1;
        
        osal_u32 _4OqyOwtzyyOP2zOCItyOwOxSRi_: 3;
        osal_u32 __WCqwk_                   : 1;
        osal_u32                            : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb107_union;

typedef union {
    
    struct {
        
        osal_u32 _FwTjw_NGjjwqgGw6SNjw_wodLD_: 3;
        osal_u32 __WCqwy_                   : 1;
        
        osal_u32 _4OqyOwtzyyOP2zOCItyOwOxSRa_: 3;
        osal_u32 _wmsPOr_                   : 1;
        
        osal_u32 _FwTjw_NGjjwqgGw6SNjw_wodLX_: 3;
        osal_u32 resv_2                     : 1;
        
        osal_u32 _4OqyOwtzyyOP2zOCItyOwOxSR9_: 3;
        osal_u32 __WCqwk_                   : 1;
        osal_u32                            : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb108_union;

typedef union {
    
    struct {
        
        osal_u32 _FwTjw_NGjjwF6i6wBuiotwL_BSwLiwoC_   : 2;
        
        osal_u32 _4OqyOwtzyyO4CvCOCvzwsmRwlIORvOxs_   : 2;
        
        osal_u32 _4OqyOwtzyyOR8OxGtO4CCzxORwlIOlORvOxs_: 3;
        osal_u32 __WCqwy_                             : 1;
        
        osal_u32 _4OqyOwtzyyOR8OxGtO4CCzxORwlIO6ORvOxs_: 3;
        osal_u32 _wmsPOr_                             : 1;
        
        osal_u32 _FwTjw_NGjjwuFjdNwq6SwLiwoC_         : 3;
        osal_u32 resv_2                               : 1;
        osal_u32                                      : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb109_union;

typedef union {
    
    struct {
        
        osal_u32 _4OqxOGzOP4mROsAltROylzsOmoORvOxs_: 1;
        
        osal_u32 _4OqxOGzOP4mROPCIOZ2ORvOxs_      : 3;
        
        osal_u32 _FwTowuGwqFWLw_WCwgGBhwVgwLiwoC_ : 3;
        
        osal_u32 _4OqxOGzOP4mROwmsOsAltROZ2ORvOxs_: 3;
        
        osal_u32 _4OqxOGGzOP4mROPCIOZ2ORvOxs_     : 3;
        osal_u32 __WCqwy_                         : 3;
        osal_u32                                  : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb110_union;

typedef union {
    
    struct {
        
        osal_u32 _FwTjw_NwLPwuGwGFjwBidLwCWowLjwyyyyyywodL_: 6;
        
        osal_u32 _4OqyOwtOR8OGzOz4yOlvSROsmxORyO33333rOxSR_: 6;
        
        osal_u32 _FwTjw_NwoigWhwCCjwq6L_oVwLywodL_        : 4;
        osal_u32                                          : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb111_union;

typedef union {
    
    struct {
        
        osal_u32 _4OqyOwtOR8OGzOz4yOlvSROsmxORyO3333r3OxSR_: 6;
        
        osal_u32 _FwTjw_NwLPwuGwGFjwBidLwCWowLjwyyyy00wodL_: 6;
        
        osal_u32 _4OqyOwtOR8OGzOPIl4OsmxORO333OxSR_       : 4;
        osal_u32                                          : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb112_union;

typedef union {
    
    struct {
        
        osal_u32 _4OqyOwtOR8OGzOz4yOlvSROsmxORyO333r33OxSR_: 6;
        
        osal_u32 _FwTjw_NwLPwuGwGFjwBidLwCWowLjwyyy0y0wodL_: 6;
        
        osal_u32 _FwTjw_NwLPwuuGwq6GCwCWowLwyyywodL_      : 4;
        osal_u32                                          : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb113_union;

typedef union {
    
    struct {
        
        osal_u32 _FwTjw_NwLPwuGwGFjwBidLwCWowLjwyyy00ywodL_: 6;
        
        osal_u32 _4OqyOwtOR8OGzOz4yOlvSROsmxORyO333rrrOxSR_: 6;
        
        osal_u32 _FwTjw_NwLPwdu6wqjwCWowLwyyywodL_        : 4;
        osal_u32                                          : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb114_union;

typedef union {
    
    struct {
        
        osal_u32 _FwTjw_NwLPwuGwqjwCWowLjwyyyyyywodL_: 5;
        
        osal_u32 _4OqyOwtOR8OGzOPyOsmxORyO33333rOxSR_: 5;
        
        osal_u32 _4OqyOwtOR8OGzOPyOsmxORyO3333r3OxSR_: 5;
        osal_u32 __WCqwy_                           : 1;
        osal_u32                                    : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb115_union;

typedef union {
    
    struct {
        
        osal_u32 _FwTjw_NwLPwuGwqjwCWowLjwyyyy00wodL_: 5;
        
        osal_u32 _4OqyOwtOR8OGzOPyOsmxORyO333r33OxSR_: 5;
        
        osal_u32 _FwTjw_NwLPwuGwqjwCWowLjwyyy0y0wodL_: 5;
        osal_u32 __WCqwy_                           : 1;
        osal_u32                                    : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb116_union;

typedef union {
    
    struct {
        
        osal_u32 _FwTjw_NwLPwuGwqjwCWowLjwyyy00ywodL_: 5;
        
        osal_u32 _4OqyOwtOR8OGzOPyOsmxORyO333rrrOxSR_: 5;
        
        osal_u32 _FwTjw_NwLPwuuGwqjGwCWowLwyyywodL_ : 5;
        osal_u32 __WCqwy_                           : 1;
        osal_u32                                    : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb117_union;

typedef union {
    
    struct {
        
        osal_u32 _FwTjw_NwLPwuuGwqj6wCWowLwyyywodL_ : 5;
        
        osal_u32 _4OqyOwtOR8OGzOzIGIOPtl8ORO333OxSR_: 5;
        
        osal_u32 _4OqyOwtOR8OGzOz4yOPyOsmxORO333OxSR_: 6;
        osal_u32                                    : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb118_union;

typedef union {
    
    struct {
        
        osal_u32 _4OqyOwtOR8OGzOz4yOlvSROsmxORyO33r333OxSR_: 6;
        
        osal_u32 _FwTjw_NwLPwuGwGFjwBidLwCWowLjwyy0yy0wodL_: 6;
        
        osal_u32 _4OqyOwtOxv2moOssyOPCRwxZORrOxSR_        : 4;
        osal_u32                                          : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb119_union;

typedef union {
    
    struct {
        
        osal_u32 _FwTjw_NwLPwuGwGFjwBidLwCWowLjwyy0y0ywodL_: 6;
        
        osal_u32 _4OqyOwtOR8OGzOz4yOlvSROsmxORyO33r3rrOxSR_: 6;
        
        osal_u32 _FwTjw_NwLPwuGwqSBFwCWowLwyy0wodL_       : 4;
        osal_u32                                          : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb120_union;

typedef union {
    
    struct {
        
        osal_u32 _FwTjw_NwLPwuGwGFjwBidLwCWowLjwyy00yywodL_: 6;
        
        osal_u32 _4OqyOwtOR8OGzOz4yOlvSROsmxORyO33rr3rOxSR_: 6;
        
        osal_u32 _4OqyOwtOR8OGGzOPCzsOsmxORO33rOxSR_      : 4;
        osal_u32                                          : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb121_union;

typedef union {
    
    struct {
        
        osal_u32 _4OqyOwtOR8OGzOz4yOlvSROsmxORyO33rrr3OxSR_: 6;
        
        osal_u32 _FwTjw_NwLPwuGwGFjwBidLwCWowLjwyy0000wodL_: 6;
        
        osal_u32 _4OqyOwtOR8OSGCOPyOsmxORO33rOxSR_        : 4;
        osal_u32                                          : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb122_union;

typedef union {
    
    struct {
        
        osal_u32 _4OqyOwtOR8OGzOPyOsmxORyO33r333OxSR_: 5;
        
        osal_u32 _FwTjw_NwLPwuGwqjwCWowLjwyy0yy0wodL_: 5;
        
        osal_u32 _FwTjw_NwLPwuGwqjwCWowLjwyy0y0ywodL_: 5;
        osal_u32 __WCqwy_                           : 1;
        osal_u32                                    : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb123_union;

typedef union {
    
    struct {
        
        osal_u32 _4OqyOwtOR8OGzOPyOsmxORyO33r3rrOxSR_: 5;
        
        osal_u32 _FwTjw_NwLPwuGwqjwCWowLjwyy00yywodL_: 5;
        
        osal_u32 _4OqyOwtOR8OGzOPyOsmxORyO33rr3rOxSR_: 5;
        osal_u32 __WCqwy_                           : 1;
        osal_u32                                    : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb124_union;

typedef union {
    
    struct {
        
        osal_u32 _4OqyOwtOR8OGzOPyOsmxORyO33rrr3OxSR_: 5;
        
        osal_u32 _FwTjw_NwLPwuGwqjwCWowLjwyy0000wodL_: 5;
        
        osal_u32 _4OqyOwtOR8OGGzOPyzOsmxORO33rOxSR_ : 5;
        osal_u32 __WCqwy_                           : 1;
        osal_u32                                    : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb125_union;

typedef union {
    
    struct {
        
        osal_u32 _4OqyOwtOR8OGGzOPyCOsmxORO33rOxSR_ : 5;
        
        osal_u32 _FwTjw_NwLPwuGwGSuSwqNBPwLwyy0wodL_: 5;
        
        osal_u32 _FwTjw_NwLPwuGwGFjwqjwCWowLwyy0wodL_: 6;
        osal_u32                                    : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb126_union;

typedef union {
    
    struct {
        
        osal_u32 _4OqyOwtORmIGOCv4mOyzodOsmx_: 1;
        osal_u32 __WCqwy_                   : 15;
        osal_u32                            : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb127_union;

typedef union {
    
    struct {
        
        osal_u32 _FwTjw_NwjGhFwuQt_: 2;
        osal_u32 __WCqwy_         : 14;
        osal_u32                  : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb128_union;

typedef union {
    
    struct {
        
        osal_u32 _4OqyOxlomCRwxOGzOylzsOCRwx_: 3;
        osal_u32 __WCqwy_                    : 1;
        
        osal_u32 _FwTjwoBhW6L_owiqW_wLWSuwu_L_: 1;
        
        osal_u32 _4OqyOxlomCRwxO4tROl446OIv4m_: 1;
        
        osal_u32 _FwTjwoBhW6L_owSiFWwCWo_    : 1;
        
        osal_u32 _FwTjwoBhW6L_owouNwSiFW_    : 1;
        
        osal_u32 _FwTjwoBhW6L_owLPwWh_       : 1;
        
        osal_u32 _4OqyOxlomCRwxOlGzOmo_      : 1;
        osal_u32 _wmsPOr_                    : 1;
        
        osal_u32 _FwTjwoBhW6L_ow_PwWh_       : 1;
        
        osal_u32 _FwTjwoBhW6L_owNjwWh_       : 2;
        
        osal_u32 _4OqyOxlomCRwxOxGtOyq_      : 2;
        osal_u32                             : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb129_union;

typedef union {
    
    struct {
        
        osal_u32 _4OqyOxlomCRwxOxozO2zlo_: 3;
        osal_u32 __WCqwy_               : 1;
        
        osal_u32 _FwTjwoBhW6L_owouNwgGBh_: 3;
        
        osal_u32 _4OqyOxlomCRwxOP2zO2zlo_: 5;
        
        osal_u32 _4OqyOxlomCRwxOtyO2zlo_: 3;
        
        osal_u32 _FwTjwoBhW6L_owNjwuQGCW_: 1;
        osal_u32                        : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb130_union;

typedef union {
    
    struct {
        
        osal_u32 _4OqyOxlomCRwxO4CvCOl_: 8;
        
        osal_u32 _4OqyOxlomCRwxO4CvCO6_: 8;
        osal_u32                      : 16; 
    } bits;
    
    osal_u16 u16; 
} u_rf_abb131_union;

#endif 