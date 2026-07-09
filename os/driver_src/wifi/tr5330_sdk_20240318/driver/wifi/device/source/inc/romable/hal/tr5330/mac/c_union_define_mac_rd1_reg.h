/*
 * Copyright (c) CompanyNameMagicTag 2021-2023. All rights reserved.
 * Description: chip register header file.
 * Create: 2021-12-04
 */

#ifndef __C_UNION_DEFINE_MAC_RD1_REG_H__
#define __C_UNION_DEFINE_MAC_RD1_REG_H__


typedef union {
    
    struct {
        osal_u32 _Sihw6Ww6L_owNCS_: 4; 
        osal_u32 _SihwGSCFdwNCS_: 4; 
        osal_u32 _SihwLPwSuFdwNCS_: 4; 
        osal_u32 _IvoOR8OGs4SOtsI_: 4; 
        osal_u32 _SihwLPwjd_CLwNCS_: 4; 
        osal_u32 _SihwLPwQFowNCS_: 4; 
        osal_u32 _SihwFd_wNCS_: 4; 
        osal_u32 _IvoOwmsGOyzOtsI_: 4; 
    } bits;

    
    osal_u32 u32;
} u_fsm_mon1_st;


typedef union {
    
    struct {
        osal_u32 _SihwLPwFSGBNwNCS_: 4; 
        osal_u32 _IvoOCvIGO4IzOtsI_: 4; 
        osal_u32 _Sihw_PwQFowNCS_: 4; 
        osal_u32 _IvoOw8OCvoRwvxOtsI_: 2; 
        osal_u32 _Sihw_PwuowFWC6wNCS_: 2; 
        osal_u32 _Sihw_PwFSGBNwNCS_: 4; 
        osal_u32 _Sihw_PwFWC6wNCS_: 4; 
        osal_u32 _Sihw_PwSuFdwNCS_: 4; 
        osal_u32 _IvoOw8OGs4SOtsI_: 4; 
    } bits;

    
    osal_u32 u32;
} u_fsm_mon2_st;


typedef union {
    
    struct {
        osal_u32 _IvoOyCoOtsI_: 3; 
        osal_u32 reserved_0 : 1; 
        osal_u32 _SihwFjG6wNCS_: 4; 
        osal_u32 _IvoOsltsOtsI_: 4; 
        osal_u32 _Sihw6iWPwNCS_: 4; 
        osal_u32 _IvoOCAOzCCOtsI_: 4; 
        osal_u32 _SihwWF6GwNCS_: 4; 
        osal_u32 _SihwuLw6L_owNCS_: 4; 
        osal_u32 _IvoOxSROyIOtsI_: 4; 
    } bits;

    
    osal_u32 u32;
} u_fsm_mon3_st;


typedef union {
    
    struct {
        osal_u32 _wCKOvGOySttmw_: 4; 
        osal_u32 _wCKOlGOySttmw_: 4; 
        osal_u32 _GWCwiuwjdNNW__: 4; 
        osal_u32 _GWCwBuwjdNNW__: 4; 
        osal_u32 _6WwBiwNBNi_: 4; 
        osal_u32 reserved_0 : 12; 
    } bits;

    
    osal_u32 u32;
} u_ce_fifo_status;


typedef union {
    
    struct {
        osal_u32 _IvoOwqCAOtsI_: 8; 
        osal_u32 _IvoOCvIGOwGROtsI_: 4; 
        osal_u32 _SihwLPwRwuowNCS_: 4; 
        osal_u32 _SihwuQtwLPwNCS_: 4; 
        osal_u32 _SihwuQtw_PwNCS_: 4; 
        osal_u32 _SihwLPwRwNCS_: 4; 
        osal_u32 _Sihw_PwRwNCS_: 4; 
    } bits;

    
    osal_u32 u32;
} u_fsm_mon4_st;


typedef union {
    
    struct {
        osal_u32 _ySsOwqCAOtltv_: 4; 
        osal_u32 reserved_0 : 28; 
    } bits;

    
    osal_u32 u32;
} u_bus_fifo_status;


typedef union {
    
    struct {
        osal_u32 _ySsOwqCAOwm6Oxmo_: 16; 
        osal_u32 reserved_0 : 16; 
    } bits;

    
    osal_u32 u32;
} u_bus_rwch_req_cnt;


typedef union {
    
    struct {
        osal_u32 _R8OovwIzxOGwlOIG4SOCoR_: 16; 
        osal_u32 _R8OAlOGwlOIG4SOCoR_: 16; 
    } bits;

    
    osal_u32 u32;
} u_rpt_tx_tr_norm_mpdu_cnt;


typedef union {
    
    struct {
        osal_u32 _R8OIG4SOlozIG4SOCvSoR_: 16; 
        osal_u32 _LPwGSuFdw6idhL_: 16; 
    } bits;

    
    osal_u32 u32;
} u_rpt_tx_mpdu_inampdu_count;


typedef union {
    
    struct {
        osal_u32 _R8OymzCvoOCvSoR_: 16; 
        osal_u32 reserved_0 : 16; 
    } bits;

    
    osal_u32 u32;
} u_rpt_tx_bcn_count;


typedef union {
    
    struct {
        osal_u32 _ovwIO6OwmRwQOCvSoR_: 16; 
        osal_u32 _QBu_BwRw_WL_tw6idhL_: 16; 
    } bits;

    
    osal_u32 u32;
} u_rpt_hipri_normal_retry_cnt;


typedef union {
    
    struct {
        osal_u32 __PwGSuFdw6idhL_: 16; 
        osal_u32 __uLw_PwuCFdwFWC6wW__w6hL_: 8; 
        osal_u32 reserved_0 : 8; 
    } bits;

    
    osal_u32 u32;
} u_rpt_rx_ampdu_count;


typedef union {
    
    struct {
        osal_u32 __PwFduwSuFdw6hL_: 16; 
        osal_u32 __PwFWoBSBLwNGBow6idhL_: 16; 
    } bits;

    
    osal_u32 u32;
} u_rpt_rx_dup_mpdu_cnt;


typedef union {
    
    struct {
        osal_u32 _w8OymzCvoOCoR_: 16; 
        osal_u32 reserved_0 : 16; 
    } bits;

    
    osal_u32 u32;
} u_rpt_rx_bcn_cnt;


typedef union {
    
    struct {
        osal_u32 __PwuQtwW__wSG6wuGCCWFw6hL_: 16; 
        osal_u32 reserved_0 : 16; 
    } bits;

    
    osal_u32 u32;
} u_rpt_rx_phy_err_mac_passed_cnt;


typedef union {
    
    struct {
        osal_u32 __PwuQtwCQi_LW_wW__w6hL_: 16; 
        osal_u32 __PwuQtwoihgW_wW__w6hL_: 16; 
    } bits;

    
    osal_u32 u32;
} u_rpt_rx_phy_shorter_err_cnt;


typedef union {
    
    struct {
        osal_u32 __PwNBoLW_WFw6hL_: 16; 
        osal_u32 _4lz2Ow8O4wvGORQGm_: 6; 
        osal_u32 reserved_0 : 10; 
    } bits;

    
    osal_u32 u32;
} u_rpt_rx_filtered_cnt;


typedef union {
    
    struct {
        osal_u32 __uLwWPuW6LwGSuFdwoWh_: 20; 
        osal_u32 _R8OIloOsSyOIG4SOxmo_: 11; 
        osal_u32 reserved_0 : 1; 
    } bits;

    
    osal_u32 u32;
} u_rpt_expect_len_of_ampdu;


typedef union {
    
    struct {
        osal_u32 __uLwjGwhiLwNidhFwGFF_wSCj_: 16; 
        osal_u32 __uLwjGwhiLwNidhFwLBF_: 4; 
        osal_u32 reserved_0 : 12; 
    } bits;

    
    osal_u32 u32;
} u_rpt_ba_not_found_info1;


typedef union {
    
    struct {
        osal_u32 _wGROzIG4SOwmRwQOCoR_: 16; 
        osal_u32 reserved_0 : 16; 
    } bits;

    
    osal_u32 u32;
} u_rpt_tx_ampdu_retry_cnt;


typedef union {
    
    struct {
        osal_u32 _CCIGOwmGxzQOtzlxOCoR_: 16; 
        osal_u32 _LKBuw_WuoGtwNGBow6hL_: 16; 
    } bits;

    
    osal_u32 u32;
} u_tkip_ccmp_rep_fail_cnt;


typedef union {
    
    struct {
        osal_u32 __uLw_Pw66SuwSB6wNGBow6hL_: 16; 
        osal_u32 _wGROw8ORdlGOIlCOtzlxOCoR_: 16; 
    } bits;

    
    osal_u32 u32;
} u_tkip_ccmp_mic_fail_cnt;


typedef union {
    
    struct {
        osal_u32 _wGROw8OylGOwmGxzQOtzlxOCoR_: 16; 
        osal_u32 _wGROw8OylGOIlCOtzlxOCoR_: 16; 
    } bits;

    
    osal_u32 u32;
} u_bip_mic_replay_fail_cnt;


typedef union {
    
    struct {
        osal_u32 __uLw_PwKWtwCWG_6QwNGBow6hL_: 16; 
        osal_u32 _wGROw8OlCPOtzlxOCoR_: 16; 
    } bits;

    
    osal_u32 u32;
} u_icv_key_search_fail_cnt;


typedef union {
    
    struct {
        osal_u32 _wGROw8OtCsOtzlxOCoR_: 16; 
        
        osal_u32 _wGROw8OqzGlOIlCOtzlxOCoR_: 16; 
    } bits;

    
    osal_u32 u32;
} u_rpt_rx_fcsfail_cnt;


typedef union {
    
    struct {
        osal_u32 _wGROw8OlIOyzOCoR_: 16; 
        osal_u32 __uLw_PwFWoGtwjGw6hL_: 16; 
    } bits;

    
    osal_u32 u32;
} u_rpt_rx_ba_cnt;


typedef union {
    
    struct {
        osal_u32 _wGROw8OyssOovoO4lwmCRO4zRzOCoR_: 16; 
        osal_u32 reserved_0 : 16; 
    } bits;

    
    osal_u32 u32;
} u_rpt_rx_non_direct_data_cnt;


typedef union {
    
    struct {
        osal_u32 __uLw_PwijCCwd6wFGLGw6hL_: 16; 
        osal_u32 _wGROw8OvyssOICyCO4zRzOCoR_: 16; 
    } bits;

    
    osal_u32 u32;
} u_rpt_rx_obss_data_cnt;


typedef union {
    
    struct {
        osal_u32 __uLw_PwG6Kw6hL_: 16; 
        osal_u32 reserved_0 : 16; 
    } bits;

    
    osal_u32 u32;
} u_rx_ack_cnt;


typedef union {
    
    struct {
        osal_u32 __uLw_Pw6LCw6hL_: 16; 
        osal_u32 _wGROw8OwRsOCoR_: 16; 
    } bits;

    
    osal_u32 u32;
} u_rpt_rx_rtscts_cnt;


typedef union {
    
    struct {
        osal_u32 _wGROw8OISOIlIvOGs4SOqlRAOtzlxOCoR_: 16; 
        osal_u32 _wGROw8OISOIlIvOGs4SOqlRAOsSCCmssOCoR_: 16; 
    } bits;

    
    osal_u32 u32;
} u_rpt_mu_mimo_psdu_success_cnt;


typedef union {
    
    struct {
        osal_u32 _wGROw8Ovt4IzOGs4SOqlRAOtzlxOCoR_: 16; 
        osal_u32 _wGROw8Ovt4IzOGs4SOqlRAOsSCCmssOCoR_: 16; 
    } bits;

    
    osal_u32 u32;
} u_rpt_rx_ofdma_psdu_success_cnt;


typedef union {
    
    struct {
        osal_u32 __uLw_PwjNwuCFdwTBLQwNGBow6hL_: 16; 
        osal_u32 __uLw_PwjNwuCFdwTBLQwCd66WCCw6hL_: 16; 
    } bits;

    
    osal_u32 u32;
} u_rpt_rx_bf_psdu_success_cnt;


typedef union {
    
    struct {
        osal_u32 __uLw_PwNCSwLBSWidLw6hL_: 8; 
        osal_u32 _wGROw8OA4xORlImvSROCoR_: 8; 
        osal_u32 __uLwLPwNCSwLBSWidLw6hL_: 8; 
        osal_u32 _wGROR8OA4xORlImvSROCoR_: 8; 
    } bits;

    
    osal_u32 u32;
} u_rpt_fsm_timeout_cnt;


typedef union {
    
    struct {
        osal_u32 _wGROR8OloRwOCoR_: 16; 
        
        osal_u32 reserved_0 : 16; 
    } bits;

    
    osal_u32 u32;
} u_tx_intr_cnt;


typedef union {
    
    struct {
        osal_u32 _wGROw8OovwIOloRwOCoR_: 16; 
        osal_u32 __uLw_PwQBu_BwBhL_w6hL_: 16; 
    } bits;

    
    osal_u32 u32;
} u_rx_intr_cnt;


typedef union {
    
    struct {
        osal_u32 _wGROIzCOzSRvOwsROCoR_: 16; 
        osal_u32 reserved_0 : 16; 
    } bits;

    
    osal_u32 u32;
} u_mac_auto_rst_cnt;


typedef union {
    
    struct {
        osal_u32 __uLw66Gwu_BwVySw6hL_: 26; 
        osal_u32 reserved_0 : 6; 
    } bits;

    
    osal_u32 u32;
} u_cca_pri_20m_cnt;


typedef union {
    
    struct {
        osal_u32 _wGROCCzOsmCOZ3IOCoR_: 26; 
        osal_u32 reserved_0 : 6; 
    } bits;

    
    osal_u32 u32;
} u_cca_sec_20m_cnt;


typedef union {
    
    struct {
        osal_u32 __uLwWhLW_wLPiuwuCw6hL_: 16; 
        osal_u32 _wGROR8vGOGsOmo_: 1; 
        osal_u32 reserved_0 : 15; 
    } bits;

    
    osal_u32 u32;
} u_rpt_enter_txop_ps_param1;


typedef union {
    
    struct {
        osal_u32 _wGROR8vGOGsORlImOCoR_: 26; 
        osal_u32 reserved_0 : 6; 
    } bits;

    
    osal_u32 u32;
} u_rpt_enter_txop_ps_param2;


typedef union {
    
    struct {
        osal_u32 _wGROCvm8OzyvwRO4vomOCoR_: 16; 
        osal_u32 _wGROCvm8OzyvwROsRzwROCoR_: 16; 
    } bits;

    
    osal_u32 u32;
} u_rpt_coex_1;


typedef union {
    
    struct {
        osal_u32 __uLw6iWPwu_WWSuLw6hL_: 16; 
        osal_u32 _wGROCvm8OzyvwROmo4OCoR_: 16; 
    } bits;

    
    osal_u32 u32;
} u_rpt_coex_2;


typedef union {
    
    struct {
        osal_u32 __uLw6iWPwuiCLwu_WWSuLwNGBow6hL_: 16; 
        osal_u32 __uLw6iWPwuiCLwu_WWSuLw6hL_: 16; 
    } bits;

    
    osal_u32 u32;
} u_rpt_coex_3;


typedef union {
    
    struct {
        osal_u32 __uLwjG6KiNNwLBSWidLwhGqwLBSW_w6hL_: 16; 
        osal_u32 __uLwjG6KiNNwLBSWidLwu_BwVySw66GwNoGg_: 1; 
        osal_u32 _wGROyzCdvttORlImvSROsmCOZ3IOCCzOtxz2_: 1; 
        osal_u32 reserved_0 : 14; 
    } bits;

    
    osal_u32 u32;
} u_rpt_backoff_timeout_info;


typedef union {
    
    struct {
        osal_u32 __uLwLPwN_GSWwLBSWw6hL_: 26; 
        osal_u32 reserved_0 : 6; 
    } bits;

    
    osal_u32 u32;
} u_tx_frame_time_cnt;


typedef union {
    
    struct {
        osal_u32 __uLwSihwCLwuQtwLPwuCFdw6hL_: 16; 
        osal_u32 __uLwSihwCLwuQtwLPwuCFdwoWh_: 16; 
    } bits;

    
    osal_u32 u32;
} u_rpt_mon_st_phy_tx;


typedef union {
    
    struct {
        osal_u32 _wGROIvoOsROR8OtsIOGs4SOCoR_: 16; 
        osal_u32 _wGROIvoOsROR8OtsIORlImwOCoR_: 6; 
        osal_u32 reserved_0 : 10; 
    } bits;

    
    osal_u32 u32;
} u_rpt_mon_st1_tx_fsm;


typedef union {
    
    struct {
        osal_u32 __uLwSihwCLwLPwNCSwSuFdw6hL_: 16; 
        osal_u32 __uLwSihwCLwLPwNCSwSuFdwoWh_: 16; 
    } bits;

    
    osal_u32 u32;
} u_rpt_mon_st2_tx_fsm;


typedef union {
    
    struct {
        osal_u32 __uLwSihwCLwuQtw_PwuCFdw6hL_: 16; 
        osal_u32 __uLwSihwCLwuQtw_PwuCFdwoWh_: 16; 
    } bits;

    
    osal_u32 u32;
} u_rpt_mon_st_phy_rx;


typedef union {
    
    struct {
        osal_u32 _wGROIvoOsROw8OtsIOGs4SOCoR_: 16; 
        osal_u32 _wGROIvoOsROw8OtsIORlImwOCoR_: 6; 
        osal_u32 reserved_0 : 10; 
    } bits;

    
    osal_u32 u32;
} u_rpt_mon_st1_rx_fsm;


typedef union {
    
    struct {
        osal_u32 __uLwSihwCLw_PwNCSwSuFdw6hL_: 16; 
        osal_u32 __uLwSihwCLw_PwNCSwSuFdwoWh_: 16; 
    } bits;

    
    osal_u32 u32;
} u_rpt_mon_st2_rx_fsm;


typedef union {
    
    struct {
        osal_u32 __uLwSihwCLw_PwuWW_wBhFWP_: 3; 
        osal_u32 reserved_0 : 1; 
        osal_u32 __uLwSihwCLw_PwqGuwBhFWP_: 3; 
        osal_u32 reserved_1 : 1; 
        osal_u32 __uLwSihwCLwLPwuWW_wBhFWP_: 3; 
        osal_u32 reserved_2 : 1; 
        osal_u32 __uLwSihwCLwLPwqGuwBhFWP_: 2; 
        osal_u32 reserved_3 : 2; 
        osal_u32 __uLwSihwCLw_Pwj6wNoGg_: 1; 
        osal_u32 __uLwSihwCLwLPwj6wNoGg_: 1; 
        osal_u32 _wGROIvoOsROw8OzIG4SOmo_: 1; 
        osal_u32 _wGROIvoOsROR8OzIG4SOmo_: 1; 
        osal_u32 __uLwSihwCLw6WwKWtwBF_: 4; 
        osal_u32 _wGROIvoOsROCmOClGAmwORQGm_: 4; 
        osal_u32 _wGROIvoOsROCmOmoCwOGwvCOmo_: 4; 
    } bits;

    
    osal_u32 u32;
} u_rpt_mon_st_ce_param;


typedef union {
    
    struct {
        osal_u32 __uLwSihwCLwLPwCWRwhdS_: 12; 
        osal_u32 _wGROIvoOsROR8Oyq_: 4; 
        osal_u32 _wGROIvoOsROR8O4zRzOwzRm_: 8; 
        osal_u32 __uLwSihwCLwLPwQWwNoGg_: 1; 
        osal_u32 reserved_0 : 1; 
        osal_u32 _wGROIvoOsROR8OtwzImORQGm_: 6; 
    } bits;

    
    osal_u32 u32;
} u_rpt_mon_st_tx_param;


typedef union {
    
    struct {
        osal_u32 __uLwSihwCLw_PwCWRwhdS_: 12; 
        osal_u32 _wGROIvoOsROw8Oyq_: 4; 
        osal_u32 _wGROIvoOsROw8O4zRzOwzRm_: 8; 
        osal_u32 __uLwSihwCLw_PwQWwNoGg_: 1; 
        osal_u32 reserved_0 : 1; 
        osal_u32 _wGROIvoOsROw8OtwzImORQGm_: 6; 
    } bits;

    
    osal_u32 u32;
} u_rpt_mon_st_rx_param;


typedef union {
    
    struct {
        osal_u32 _wGROR8OwRsOtzlxOCoR_: 16; 
        osal_u32 _wGROR8OwRsOGzssOCoR_: 16; 
    } bits;

    
    osal_u32 u32;
} u_rpt_tx_rts_cnt;


typedef union {
    
    struct {
        osal_u32 __uLwLPwjG_w6hL_: 16; 
        osal_u32 reserved_0 : 16; 
    } bits;

    
    osal_u32 u32;
} u_rpt_tx_bar_rx_mubar_cnt;


typedef union {
    
    struct {
        osal_u32 _wGROR8OtvwmPmwOtwzImOCoR_: 16; 
        osal_u32 _wGROR8OtvwmPmwOCSwwOsR_: 4; 
        osal_u32 reserved_0 : 12; 
    } bits;

    
    osal_u32 u32;
} u_rpt_tx_forever_mon;


typedef union {
    
    struct {
        osal_u32 _wGROw8O4lwmCROtwzImORlImOCoR_: 26; 
        osal_u32 reserved_0 : 6; 
    } bits;

    
    osal_u32 u32;
} u_rpt_rx_direct_time_cnt;


typedef union {
    
    struct {
        osal_u32 __uLw_PwhihwFB_W6LwN_GSWwijCCwLBSWw6hL_: 26; 
        osal_u32 reserved_0 : 6; 
    } bits;

    
    osal_u32 u32;
} u_rpt_rx_non_direct_obss_time_cnt;


typedef union {
    
    struct {
        osal_u32 __uLw_PwhihwFB_W6LwSgSLwLBSWw6hL_: 26; 
        osal_u32 reserved_0 : 6; 
    } bits;

    
    osal_u32 u32;
} u_rpt_rx_nondirect_mgmt_time_cnt;


typedef union {
    
    struct {
        osal_u32 __uLw_PwhihwFB_W6LwFGLGwLBSWw6hL_: 26; 
        osal_u32 reserved_0 : 6; 
    } bits;

    
    osal_u32 u32;
} u_rpt_rx_nondirect_data_time_cnt;


typedef union {
    
    struct {
        osal_u32 __uLw_Pw66GwgGu_: 16; 
        osal_u32 reserved_0 : 16; 
    } bits;

    
    osal_u32 u32;
} u_rpt_rx_cca;


typedef union {
    
    struct {
        osal_u32 __uLwGhLBwBhLNwLBSWVw6hL_: 16; 
        osal_u32 _wGROzoRlOloRtORlImrOCoR_: 16; 
    } bits;

    
    osal_u32 u32;
} u_rpt_anti_intf_period_cnt;


typedef union {
    
    struct {
        osal_u32 __uLwSihwQwBuwCdjw6hL_: 8; 
        osal_u32 _wGROIvoOGAlOGslOCoR_: 8; 
        osal_u32 __uLwSihwQVqwhLP_: 4; 
        osal_u32 __uLwSihwQVqwh_P_: 4; 
        osal_u32 reserved_0 : 1; 
        osal_u32 _wGROIvoOCxdOytOmo_: 1; 
        osal_u32 _wGROIvoOAZPOCzxCOlo2Omo_: 1; 
        osal_u32 reserved_1 : 1; 
        osal_u32 __uLwSihwQVqwhgwSiFW_: 4; 
    } bits;

    
    osal_u32 u32;
} u_rpt_mon_st_txbf1;


typedef union {
    
    struct {
        osal_u32 __uLwSihw_PwuCFdwCLGLdC_: 3; 
        osal_u32 _wGROIvoOw8OGmmwOlo4m8Omww_: 1; 
        osal_u32 _wGROIvoOw8OR8ytOsvSo4lo2OIv4m_: 2; 
        osal_u32 reserved_0 : 1; 
        osal_u32 _wGROIvoOPAROtmm4yzCdORQGm_: 1; 
        osal_u32 __uLwSihwQWwNWWFjG6KwLtuW_: 1; 
        osal_u32 __uLwSihw6iSu_wu_i6wWh_: 1; 
        osal_u32 __uLwSihwLPjNw_uLwCWo_: 2; 
        osal_u32 _wGROIvoOR8ytOzyvwR_: 1; 
        osal_u32 _wGROIvoOR8ytOIzRwl8Ow4Q_: 1; 
        osal_u32 reserved_1 : 18; 
    } bits;

    
    osal_u32 u32;
} u_rpt_mon_st_txbf2;


typedef union {
    
    struct {
        osal_u32 _wGROIvoOqzlROwmsGORlIm_: 16; 
        osal_u32 _wGROIvoO4yzCOqzlRORlIm_: 16; 
    } bits;

    
    osal_u32 u32;
} u_rpt_mon_one_pkt1;


typedef union {
    
    struct {
        osal_u32 _wGRO4yzCOR8OCoR_: 12; 
        osal_u32 reserved_0 : 4; 
        osal_u32 _wGROvomOGdROR8OCoR_: 8; 
        osal_u32 reserved_1 : 8; 
    } bits;

    
    osal_u32 u32;
} u_rpt_mon_one_pkt2;


typedef union {
    
    struct {
        osal_u32 __uLwFjG6wLPwBhL_w6hL_: 8; 
        osal_u32 reserved_0 : 8; 
        osal_u32 __uLwFjG6wLPwLBSWwBhNi_: 16; 
    } bits;

    
    osal_u32 u32;
} u_rpt_mon_one_pkt3;


typedef union {
    
    struct {
        osal_u32 __uLwSihwjdCw_TwSGPwLBSW_: 16; 
        osal_u32 reserved_0 : 16; 
    } bits;

    
    osal_u32 u32;
} u_rpt_bus_rw_max_time;


typedef union {
    
    struct {
        osal_u32 rpt_srg_68_74_cnt : 16; 
        osal_u32 rpt_srg_62_68_cnt : 16; 
    } bits;

    
    osal_u32 u32;
} u_srg_62_74_cnt;


typedef union {
    
    struct {
        osal_u32 rpt_srg_78_82_cnt : 16; 
        osal_u32 rpt_srg_74_78_cnt : 16; 
    } bits;

    
    osal_u32 u32;
} u_srg_74_82_cnt;


typedef union {
    
    struct {
        osal_u32 rpt_non_srg_68_74_cnt : 16; 
        osal_u32 rpt_non_srg_62_68_cnt : 16; 
    } bits;

    
    osal_u32 u32;
} u_non_srg_62_74_cnt;


typedef union {
    
    struct {
        osal_u32 _wGROGswOswOR8OtzlxOoSI_: 8; 
        osal_u32 __uLwuC_wC_wWhwhdS_: 8; 
        osal_u32 _wGROvyssOG4OytOw8O4vomOoSI_: 8; 
        osal_u32 _wGROvyssOG4OmoOoSI_: 8; 
    } bits;

    
    osal_u32 u32;
} u_obss_pd_en_num;


typedef union {
    
    struct {
        osal_u32 rpt_non_srg_78_82_cnt : 16; 
        osal_u32 rpt_non_srg_74_78_cnt : 16; 
    } bits;

    
    osal_u32 u32;
} u_non_srg_74_82_cnt;


typedef union {
    
    struct {
        osal_u32 rpt_obss_pd_tx_success_num : 16; 
        osal_u32 rpt_obss_pd_tx_num : 16; 
    } bits;

    
    osal_u32 u32;
} u_obss_pd_tx_num;


typedef union {
    
    struct {
        osal_u32 rpt_psr_sr_tx_success_num : 16; 
        osal_u32 rpt_psr_sr_tx_num : 16; 
    } bits;

    
    osal_u32 u32;
} u_psr_sr_tx_num;


typedef union {
    
    struct {
        osal_u32 _wGROR8OwmsGOzCdOCoR_: 16; 
        osal_u32 __uLwLPw_WCuwjGw6hL_: 16; 
    } bits;

    
    osal_u32 u32;
} u_rssi_max_min;


typedef union {
    
    struct {
        osal_u32 _wGROw8OPAROsvSo4lo2OISOrsROovIzRCAOCoR_: 8; 
        osal_u32 __uLw_PwqQLwCidhFBhgwSdw0CLwSGL6Qw6hL_: 8; 
        osal_u32 __uLw_PwqQLwCidhFBhgwFB_W6Lw6hL_: 8; 
        osal_u32 __uLw_PwQLwCidhFBhgw6hL_: 8; 
    } bits;

    
    osal_u32 u32;
} u_rx_sounding_cnt1;


typedef union {
    
    struct {
        osal_u32 __uLwqQLwCidhFBhgwSdw0CLwSGL6Qw_WCuw6hL_: 8; 
        osal_u32 __uLwqQLwCidhFBhgwFB_W6Lw_WCuw6hL_: 8; 
        osal_u32 __uLw_PwQWwCidhFBhgwLjw6hL_: 8; 
        osal_u32 __uLw_PwQWwCidhFBhgwhihwLjw6hL_: 8; 
    } bits;

    
    osal_u32 u32;
} u_rx_sounding_cnt2;


typedef union {
    
    struct {
        osal_u32 __uLwQWwCidhFBhgwhihwLjw_WCuw6hL_: 8; 
        osal_u32 _wGROPAROsvSo4lo2OISOrsROovIzRCAOwmsGOCoR_: 8; 
        osal_u32 __uLwCidhFBhgw_PwqGuwBhFWP_: 3; 
        osal_u32 reserved_0 : 2; 
        osal_u32 __uLwCidhFBhgwu_iLi6io_: 2; 
        osal_u32 reserved_1 : 9; 
    } bits;

    
    osal_u32 u32;
} u_rx_sounding_cnt3;


typedef union {
    
    struct {
        osal_u32 _wGROsvSo4lo2OqzlROAORlImw_: 11; 
        osal_u32 reserved_0 : 5; 
        osal_u32 __uLwCidhFBhgwTGBLwqwLBSW__: 11; 
        osal_u32 reserved_1 : 5; 
    } bits;

    
    osal_u32 u32;
} u_rpt_sounding_wait_timer;


typedef union {
    
    struct {
        osal_u32 _wGROsvSo4lo2OIzCO4mxRzOsowOCoR_: 8; 
        osal_u32 _wGROsvSo4lo2OGAQO4mxRzOsowOCoR_: 8; 
        osal_u32 _wGROsvSo4lo2OIzCOGAlOGslOCoR_: 8; 
        osal_u32 _wGROsvSo4lo2OGAQOGAlOGslOCoR_: 8; 
    } bits;

    
    osal_u32 u32;
} u_sounding_ptr_psi_delta_snr;


typedef union {
    
    struct {
        osal_u32 __uLw6CBwCTwjoKwW__w6hL_: 8; 
        osal_u32 __uLw6CBwLBSWidLwW__w6hL_: 8; 
        osal_u32 _wGROCslOAORlIlo2OmwwOCoR_: 8; 
        osal_u32 __uLw6CBwQwhdSwW__w6hL_: 8; 
    } bits;

    
    osal_u32 u32;
} u_rpt_csi_err_cnt;


typedef union {
    
    struct {
        osal_u32 rpt_csi_succ_cnt : 16; 
        osal_u32 __uLw6CBwjoKwhdSw6hL_: 3; 
        osal_u32 reserved_0 : 1; 
        osal_u32 _wGROCslOtlxROCoR_: 8; 
        osal_u32 reserved_1 : 4; 
    } bits;

    
    osal_u32 u32;
} u_rpt_csi_cnt;


typedef union {
    
    struct {
        osal_u32 _RyOIvolRvwOw8ORyOtzlxOCoR_: 8; 
        osal_u32 _LjwSihBLi_w_PwLjwCd66w6hL_: 8; 
        osal_u32 _LjwSihBLi_wuG_GSwNWWFjG6Kw6hL_: 8; 
        osal_u32 _LjwSihBLi_w_PwL_BgwGBFwSGL6Qw6hL_: 8; 
    } bits;

    
    osal_u32 u32;
} u_rpt_tb_monitor1;


typedef union {
    
    struct {
        osal_u32 _RyOIvolRvwOGzwzIOmwwlotv_: 4; 
        osal_u32 _wGROAmORyOSGAORIG_: 8; 
        osal_u32 __uLwQWwLjwLPwuiTW__: 8; 
        osal_u32 __uLwLjwLPwLu6wqGo_: 8; 
        osal_u32 reserved_0 : 4; 
    } bits;

    
    osal_u32 u32;
} u_rpt_tb_monitor3;


typedef union {
    
    struct {
        osal_u32 __uLw6o_wSdwWF6GwLBSW_w6hL_: 8; 
        osal_u32 _wGROmoRmwOISOm4CzOCoR_: 8; 
        osal_u32 reserved_0 : 16; 
    } bits;

    
    osal_u32 u32;
} u_rpt_ac_mu_edca_param;


typedef union {
    
    struct {
        osal_u32 _PzGZORyRROCSwwOCoR_: 26; 
        osal_u32 reserved_0 : 6; 
    } bits;

    
    osal_u32 u32;
} u_vap2_tbtt_timer_status;


typedef union {
    
    struct {
        osal_u32 __uLwqGuVw_Pwj6hwuW_BiF_: 16; 
        osal_u32 _PzGZOxlsRmoOCoR_: 8; 
        osal_u32 _qGuVwFLBSw6d__w6hL_: 8; 
    } bits;

    
    osal_u32 u32;
} u_rpt_vap2_rx_bcn_param;


typedef union {
    
    struct {
        osal_u32 _PzGiORyRROCSwwOCoR_: 26; 
        osal_u32 reserved_0 : 6; 
    } bits;

    
    osal_u32 u32;
} u_vap3_tbtt_timer_status;


typedef union {
    
    struct {
        osal_u32 __uLwqGukw_Pwj6hwuW_BiF_: 16; 
        osal_u32 _PzGiOxlsRmoOCoR_: 8; 
        osal_u32 _qGukwFLBSw6d__w6hL_: 8; 
    } bits;

    
    osal_u32 u32;
} u_rpt_vap3_rx_bcn_param;


typedef union {
    
    struct {
        osal_u32 _wGROCslOAOoSI_: 8; 
        osal_u32 _wGROCslOAOoSIORAmwvOPzx_: 8; 
        osal_u32 reserved_0 : 16; 
    } bits;

    
    osal_u32 u32;
} u_rpt_csi_h_num;

#endif 
