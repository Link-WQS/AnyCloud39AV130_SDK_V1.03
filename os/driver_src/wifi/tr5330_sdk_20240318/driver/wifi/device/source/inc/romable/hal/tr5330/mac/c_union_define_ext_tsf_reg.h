/*
 * Copyright (c) CompanyNameMagicTag 2021-2023. All rights reserved.
 * Description: chip register header file.
 * Create: 2021-12-04
 */

#ifndef __C_UNION_DEFINE_EXT_TSF_REG_H__
#define __C_UNION_DEFINE_EXT_TSF_REG_H__


typedef union {
    
    struct {
        osal_u32 _6NgwSG6wLCNwCoWWuwWh_: 1; 
        osal_u32 _6NgwWPLywLCNwWh_: 1; 
        osal_u32 _Ct2Om8RrORstOmo_: 1; 
        osal_u32 _6NgwWPLVwLCNwWh_: 1; 
        osal_u32 _Ct2Om8RORstOqvwdORlImwOSsOmo_: 1; 
        osal_u32 reserved_0 : 1; 
        osal_u32 _Ct2Om8ROGZGOGsOmo_: 1; 
        osal_u32 _Ct2Om8R3OxlsRmoOloRwOmo_: 1; 
        osal_u32 _6NgwWPL0woBCLWhwBhL_wWh_: 1; 
        osal_u32 _Ct2Om8RZOxlsRmoOloRwOmo_: 1; 
        osal_u32 _6NgwWPLywFLBSwBhL_wWh_: 1; 
        osal_u32 _Ct2Om8RrO4RlIOloRwOmo_: 1; 
        osal_u32 _6NgwWPLVwFLBSwBhL_wWh_: 1; 
        osal_u32 reserved_1 : 19; 
    } bits;
    
    osal_u32 u32;
} u_wl0_ext_tsf_ctrl;


typedef union {
    
    struct {
        osal_u32 _m8R3ORstOqvwdOIv4m_: 1; 
        osal_u32 _WPL0wLCNwTi_KwSiFW_: 1; 
        osal_u32 _m8RZORstOqvwdOIv4m_: 1; 
        osal_u32 reserved_0 : 29; 
    } bits;
    
    osal_u32 u32;
} u_wl0_ext_tsf_work_vaptus;


typedef union {
    
    struct {
        osal_u32 _Ct2Om8RORstOCxdOGmwlv4OAl_: 10; 
        osal_u32 reserved_0 : 22; 
    } bits;
    
    osal_u32 u32;
} u_wl0_ext_tsf_clk_period_h;


typedef union {
    
    struct {
        osal_u32 _Ct2Om8RORstOCxdOGmwlv4Oxv_: 10; 
        osal_u32 reserved_0 : 22; 
    } bits;
    
    osal_u32 u32;
} u_wl0_ext_tsf_clk_period_l;


typedef union {
    
    struct {
        osal_u32 _Ct2Om8R3ORyRROvttsmR_: 16; 
        osal_u32 reserved_0 : 16; 
    } bits;
    
    osal_u32 u32;
} u_wl0_ext0_tbtt_offset;


typedef union {
    
    struct {
        osal_u32 _6NgwWPLywj6hwuW_BiF_: 16; 
        osal_u32 reserved_0 : 16; 
    } bits;
    
    osal_u32 u32;
} u_wl0_ext0_bcn_period;


typedef union {
    
    struct {
        osal_u32 _WPLywLjLLw6hL_: 26; 
        osal_u32 reserved_0 : 6; 
    } bits;
    
    osal_u32 u32;
} u_wl0_ext0_tbtt_cnt;


typedef union {
    
    struct {
        osal_u32 _m8R3O4RlIOCoR_: 8; 
        osal_u32 reserved_0 : 24; 
    } bits;
    
    osal_u32 u32;
} u_wl0_ext0_dtim_cnt;


typedef union {
    
    struct {
        osal_u32 _WPLywoBCLWhw6hL_: 8; 
        osal_u32 reserved_0 : 24; 
    } bits;
    
    osal_u32 u32;
} u_wl0_ext0_listen_cnt;


typedef union {
    
    struct {
        osal_u32 _6NgwWPL0wLjLLwiNNCWL_: 16; 
        osal_u32 reserved_0 : 16; 
    } bits;
    
    osal_u32 u32;
} u_wl0_ext1_tbtt_offset;


typedef union {
    
    struct {
        osal_u32 _Ct2Om8RrOyCoOGmwlv4_: 16; 
        osal_u32 reserved_0 : 16; 
    } bits;
    
    osal_u32 u32;
} u_wl0_ext1_bcn_period;


typedef union {
    
    struct {
        osal_u32 _m8RrORyRROCoR_: 26; 
        osal_u32 reserved_0 : 6; 
    } bits;
    
    osal_u32 u32;
} u_wl0_ext1_tbtt_cnt;


typedef union {
    
    struct {
        osal_u32 _WPL0wFLBSw6hL_: 8; 
        osal_u32 reserved_0 : 24; 
    } bits;
    
    osal_u32 u32;
} u_wl0_ext1_dtim_cnt;


typedef union {
    
    struct {
        osal_u32 _m8RrOxlsRmoOCoR_: 8; 
        osal_u32 reserved_0 : 24; 
    } bits;
    
    osal_u32 u32;
} u_wl0_ext1_listen_cnt;


typedef union {
    
    struct {
        osal_u32 _Ct2Om8RZORyRROvttsmR_: 16; 
        osal_u32 reserved_0 : 16; 
    } bits;

    
    osal_u32 u32;
} u_wl0_ext2_tbtt_offset;


typedef union {
    
    struct {
        osal_u32 _6NgwWPLVwj6hwuW_BiF_: 16; 
        osal_u32 reserved_0 : 16; 
    } bits;

    
    osal_u32 u32;
} u_wl0_ext2_bcn_period;


typedef union {
    
    struct {
        osal_u32 _WPLVwLjLLw6hL_: 26; 
        osal_u32 reserved_0 : 6; 
    } bits;

    
    osal_u32 u32;
} u_wl0_ext2_tbtt_cnt;


typedef union {
    
    struct {
        osal_u32 _m8RZO4RlIOCoR_: 8; 
        osal_u32 reserved_0 : 24; 
    } bits;

    
    osal_u32 u32;
} u_wl0_ext2_dtim_cnt;


typedef union {
    
    struct {
        osal_u32 _WPLVwoBCLWhw6hL_: 8; 
        osal_u32 reserved_0 : 24; 
    } bits;

    
    osal_u32 u32;
} u_wl0_ext2_listen_cnt;


typedef union {
    
    struct {
        osal_u32 _6NgwWPLwuVuwhiGwBhLW_qGowoi_: 16; 
        osal_u32 _6NgwWPLwuVuwhiGwBhLW_qGowQB_: 16; 
    } bits;
    
    osal_u32 u32;
} u_wl0_ext_p2p_noa_interval;


typedef union {
    
    struct {
        osal_u32 _6NgwWPLwuVuwhiGwFd_GLBihwoi_: 16; 
        osal_u32 _6NgwWPLwuVuwhiGwFd_GLBihwQB_: 16; 
    } bits;
    
    osal_u32 u32;
} u_wl0_ext_p2p_noa_duration_hi;


typedef union {
    
    struct {
        osal_u32 _6NgwWPLwuVuwhiGwiNNCWL_: 16; 
        osal_u32 reserved_0 : 16; 
    } bits;
    
    osal_u32 u32;
} u_wl0_ext_p2p_noa_offset;

#endif 
