/*
 * Copyright (c) CompanyNameMagicTag 2021-2023. All rights reserved.
 * Description: chip register header file.
 * Create: 2021-12-04
 */

#ifndef __C_UNION_DEFINE_MAC_LUT0_REG_H__
#define __C_UNION_DEFINE_MAC_LUT0_REG_H__


typedef union {
    
    struct {
        osal_u32 _Ct2OGmmwOz44wOIsy_: 16; 
        osal_u32 reserved_0 : 16; 
    } bits;

    
    osal_u32 u32;
} u_peer_address_msb;


typedef union {
    
    struct {
        osal_u32 _6NgwuWW_wGFF_wodLwiuW_wWh_: 1; 
        osal_u32 reserved_0 : 3; 
        osal_u32 _6NgwuWW_wGFF_wodLwiuW__: 2; 
        osal_u32 _6NgwuWW_wGFF_wodLwiuW_wCWo_: 2; 
        osal_u32 _6NgwuWW_wGFF_wodLwLPwuhwLtuW_: 4; 
        osal_u32 _6NgwuWW_wGFF_wodLwBhFWP_: 4; 
        osal_u32 reserved_1 : 16; 
    } bits;

    
    osal_u32 u32;
} u_peer_addr_lut_config;


typedef union {
    
    struct {
        osal_u32 _6NgwodLwCWRw6L_owNoF_: 16; 
        osal_u32 reserved_0 : 16; 
    } bits;

    
    osal_u32 u32;
} u_dup_det_lut_seq_ctrl_fld;


typedef union {
    
    struct {
        osal_u32 _6NgwodLwCWRwiuW_wWh_: 1; 
        osal_u32 _6NgwodLwCWRwiuW_wCWo_: 1; 
        osal_u32 _6NgwodLwCWRwiuW__: 2; 
        osal_u32 _Ct2OxSROsm6ORl4_: 3; 
        osal_u32 _Ct2OxSROsm6O6vsOmo_: 1; 
        osal_u32 _6NgwodLwCWRwBhFWP_: 3; 
        osal_u32 reserved_0 : 21; 
    } bits;

    
    osal_u32 u32;
} u_dup_det_lut_config;


typedef union {
    
    struct {
        osal_u32 _Ct2OdmQOxSROvGmwOmo_: 1; 
        osal_u32 reserved_0 : 1; 
        osal_u32 _Ct2OdmQOxSROvGmw_: 2; 
        osal_u32 _6NgwKWtwodLwiuW_wSiFW_: 2; 
        osal_u32 reserved_1 : 10; 
        osal_u32 _6NgwKWtwodLwGFF__: 16; 
    } bits;

    
    osal_u32 u32;
} u_ce_lut_config;


typedef union {
    
    struct {
        osal_u32 _wGROR8OzIG4SOIloOIG4SOxmo_: 9; 
        osal_u32 reserved_0 : 23; 
    } bits;

    
    osal_u32 u32;
} u_tx_ampdu_min_mpdu_len;


typedef union {
    
    struct {
        osal_u32 _Ct2OyzOGmmwOz44wOIsyOR8Oyq_: 16; 
        osal_u32 reserved_0 : 16; 
    } bits;

    
    osal_u32 u32;
} u_cbmap_ba_staaddrh;


typedef union {
    
    struct {
        osal_u32 _Ct2OyzORl4OIIss_: 4; 
        osal_u32 _6NgwjGwTBhCLG_L_: 12; 
        osal_u32 _Ct2OyzOqlo4vqOslpm_: 6; 
        osal_u32 _Ct2OyzOAmOtxz2_: 1; 
        osal_u32 reserved_0 : 9; 
    } bits;

    
    osal_u32 u32;
} u_cbmap_ba_params;


typedef union {
    
    struct {
        osal_u32 _Ct2OyzOxSROvGmwOmo_: 1; 
        osal_u32 _Ct2OyzOxSROvGmwOsmx_: 1; 
        osal_u32 _Ct2OyzOxSROvGmw_: 2; 
        osal_u32 _Ct2OyzOxSROlo4m8_: 4; 
        osal_u32 reserved_0 : 24; 
    } bits;

    
    osal_u32 u32;
} u_cbmap_ba_ctrl;


typedef union {
    
    struct {
        osal_u32 _wGROxSROylRIzGOloRtOx2Zsq_: 1; 
        osal_u32 _wGROxSROylRIzGOloRtOx2Zx2_: 1; 
        osal_u32 _wGROxSROGmmwOz44wOloRtOx2Zsq_: 1; 
        osal_u32 _wGROxSROGmmwOz44wOloRtOx2Zx2_: 1; 
        osal_u32 _wGROxSROCmOdmQOloRtOx2Zsq_: 1; 
        osal_u32 _wGROxSROCmOdmQOloRtOx2Zx2_: 1; 
        osal_u32 _wGROxSROsm6OloRtOx2Zsq_: 1; 
        osal_u32 _wGROxSROsm6OloRtOx2Zx2_: 1; 
        osal_u32 reserved_0 : 24; 
    } bits;

    
    osal_u32 u32;
} u_rpt_lut_state_intf_flag;


typedef union {
    
    struct {
        osal_u32 _Ct2OxSROloRtOCxw_: 1; 
        osal_u32 reserved_0 : 31; 
    } bits;

    
    osal_u32 u32;
} u_lut_intf_clr;

#endif 
