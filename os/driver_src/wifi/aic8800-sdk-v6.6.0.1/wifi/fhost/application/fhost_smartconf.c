/**
 ****************************************************************************************
 *
 * @file fhost_smartconf.c
 *
 * @brief implementation of smart config for Fully Hosted firmware.
 *
 * Copyright (C) RivieraWaves 2017-2019
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup FHOST_SMARTCONF
 * @{
 ****************************************************************************************
 */
/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "fhost_smartconf.h"
#include "fhost_smart_mode.h"
#include "fhost_config.h"
#include "cfgrwnx.h"
#include "fhost.h"
#include "fhost_cntrl.h"
#include "fhost_wpa.h"
#include "wlan_if.h"

#if NX_SMARTCONFIG
/*
 * GLOBAL VARIABLES
 ****************************************************************************************
 */
/// Smart Configuration state
struct fhost_smart_conf smart_conf = {0,};

/// Encryption Offset for supported frame
uint16_t rx_encry_offset[] = {
    [UNKOWN_ENCRY] = 0,
    [ENCRY_NONE] = 62,
    [ENCRY_CCMP] = 78,
    [ENCRY_TKIP] = 82,
    [ENCRY_WEP] = 70,
};

/// Encryption Offset for unsupported frame
uint16_t uf_encry_offset[] = {
    [UNKOWN_ENCRY] = 0,
    [ENCRY_NONE] = 70,
    [ENCRY_CCMP] = 86,
};

/// Number of rx encryption types
uint8_t rx_encry_size = sizeof(rx_encry_offset) / sizeof(rx_encry_offset[0]);
/// Number of uf encryption types
uint8_t uf_encry_size = sizeof(uf_encry_offset) / sizeof(uf_encry_offset[0]);

/*
 *
 * FUNCTIONS
 ****************************************************************************************
 */
/**
 ****************************************************************************************
 * @brief Get the scan results
 *
 * Retrieve a list of frequency in which BSSIDs have been found during the scan
 *
 * @param[out] scan_result Structure to retrieve scan results
 * @return number of frequency found
 ****************************************************************************************
 */
static int fhost_smart_get_scan_result(struct fhost_smart_scan_res *scan_result)
{
    struct mac_scan_result result;
    int result_idx = 0;

    scan_result->cnt = 0;
    while (fhost_get_scan_results(smart_conf.link_params, result_idx++, 1, &result))
    {
        int i;
        for (i = 0 ; i < scan_result->cnt; i++)
        {
            if (scan_result->chan[i] == result.chan)
                break;
        }
        if (i == scan_result->cnt)
            scan_result->chan[scan_result->cnt++] = result.chan;
    }
    dbg( "Scan result %d\r\n", scan_result->cnt);

    return scan_result->cnt;
}

/**
 ****************************************************************************************
 * @brief Start connection with the Access Point
 *
 * This will start a new wpa_supplicant task.
 * This function is blocking until connection is successful.
 *
 * @param[in] ssid      Network name
 * @param[in] password  Network password
 *
 * @return 0 on success and !=0 if error occurred
 ****************************************************************************************
 */
static int fhost_smart_connect(char *ssid, char *password)
{
    char *ap_cfg;
    int res, len = 141; // 141 is enough for worst case: CCMP with SSID (32char) and PSK (63 char)
    net_if_t *net_if = NULL;
    uint32_t ip_addr = 0;
    uint8_t* mac_addr =NULL;

    dbg("State: Smartconfig connect\r\n");

    fhost_wpa_init();

    ap_cfg = rtos_malloc(len * sizeof(char));
    if (!ap_cfg)
        return -1;

    switch (smart_conf.encryption_mode)
    {
        case ENCRY_NONE:
            dbg_snprintf(ap_cfg, len, "ssid \"%s\";key_mgmt NONE;scan_ssid 1", ssid);
            break;
        case ENCRY_CCMP:
        case ENCRY_TKIP:
            #ifdef CONFIG_MBEDTLS
            dbg_snprintf(ap_cfg, len, "ssid \"%s\";key_mgmt WPA-PSK SAE;psk \"%s\";scan_ssid 1;ieee80211w 1",
                         ssid, password);
            #else /* CONFIG_MBEDTLS */
            dbg_snprintf(ap_cfg, len, "ssid \"%s\";key_mgmt WPA-PSK;psk \"%s\";scan_ssid 1;ieee80211w 1",
                         ssid, password); //;proto WPA RSN
            #endif /* CONFIG_MBEDTLS */
            break;
        //case ENCRY_WEP:
        //    dbg_snprintf(ap_cfg, len, "ssid \"%s\";wep_key0 \"%s\";key_mgmt NONE;scan_ssid 1",
        //                 ssid, password);
        //    break;
        default:
            TRACE_FHOST("Smartconf: Unknown encryption\n");
            res = -1;
            goto exit;
    }

    res = fhost_wpa_create_network(smart_conf.idx, ap_cfg, true, 15000);
    if(res) {
        dbg("[AIC] fhost_wpa_create_network fail\r\n");
        res = -1;
        goto exit;
    }
    // Get network interface (created by CNTRL task).
    net_if = net_if_find_from_wifi_idx(smart_conf.idx);
    if (net_if == NULL) {
        dbg("[AIC] net_if_find_from_wifi_idx fail\r\n");
        res = -1;
        goto exit;
    }
    // Start DHCP client to retrieve ip address
    if (wlan_dhcp(net_if) < 0) {
        wlan_disconnect_sta(smart_conf.idx);
        dbg("[AIC] dhcp fail\r\n");
        res = -1;
        goto exit;
    }

    // Now that we got an IP address use this interface as default
    net_if_set_default(net_if);

    net_if_get_ip(net_if, &ip_addr, NULL, NULL);
    mac_addr = (uint8_t*)fhost_env.vif[smart_conf.idx].mac_addr.array;

    fhost_sendrsp_to_app(ip_addr, mac_addr);

    dbg("State: Smartconfig over\r\n");

exit:
    rtos_free(ap_cfg);
    ap_cfg = NULL;

    return res;
}

/**
 ****************************************************************************************
 * @brief Exit from smartconf Task
 *
 * Close opened sockets and delete current task
 ****************************************************************************************
 */
static void fhost_smartconf_exit()
{
    if (smart_conf.local_link) {
        fhost_cntrl_cfgrwnx_link_close(smart_conf.link_params);
        smart_conf.local_link = false;
    }
    if(smart_conf.semaphore) {
        rtos_semaphore_delete(smart_conf.semaphore);
        smart_conf.semaphore = NULL;
    }
    if (smart_conf.smart_handle) {
        rtos_task_delete(smart_conf.smart_handle);
        smart_conf.smart_handle = NULL;
    }
}

/**
 ****************************************************************************************
 * @brief Contrl task main loop
 *
 ****************************************************************************************
 */
static RTOS_TASK_FCT(fhost_smartconf_task)
{
    struct fhost_smart_scan_res scan_result;
    struct mac_chan_def *chan;
    struct fhost_vif_monitor_cfg cfg;
    int i = 0;

    fhost_smart_init();

    // Reset STA interface (this will end previous wpa_supplicant task)
    if (fhost_set_vif_type(smart_conf.link_params, smart_conf.idx, VIF_UNKNOWN, false) ||
        fhost_set_vif_type(smart_conf.link_params, smart_conf.idx, VIF_STA, false))
        return;

    if (fhost_scan(smart_conf.link_params, smart_conf.idx, NULL) < 0)
    {
        TRACE_FHOST("Smartconf: Failed to scan !\n");
        goto exit;
    }

    if (fhost_smart_get_scan_result(&scan_result) == 0)
    {
        TRACE_FHOST("Smartconf: No BSSID found!\n");
        goto exit;
    }

    if (fhost_set_vif_type(smart_conf.link_params, smart_conf.idx,
                           VIF_MONITOR, false))
    {
        TRACE_FHOST("Smartconf: Failed to enter monitor mode\n");
        goto exit;
    }

    chan = scan_result.chan[i];
    for( ;; )
    {
        cfg.chan.band = chan->band;
        cfg.chan.type = PHY_CHNL_BW_20;
        cfg.chan.prim20_freq = chan->freq;
        cfg.chan.center1_freq = chan->freq;
        cfg.chan.center2_freq = 0;
        cfg.chan.tx_power = chan->tx_power;
        cfg.uf = true;
        cfg.cb = fhost_smart_cb;
        cfg.cb_arg = NULL;

Monitor_cfg:
        if (fhost_cntrl_monitor_cfg(smart_conf.link_params, smart_conf.idx, &cfg))
        {
            TRACE_FHOST("Smartconf: Failed to configure monitor mode!\n");
            break;
        }

        if (smart_conf.status == SC_STATUS_WAIT)
        {
            // Wait until finding channel or until timeout has elapsed (120 ms)
            rtos_semaphore_wait(smart_conf.semaphore, 120);
        }

        if (smart_conf.status == SC_STATUS_CHANNEL_FOUND)
        {
            // Compare packet frequency with current smartconfig freq.
            // If different, switch monitor to smartconfig freq (Could this happen ?).
            if (smart_conf.freq != chan->freq)
            {
                chan = fhost_chan_get(smart_conf.freq);
                if (chan == NULL)
                    break;
                continue;
            }

            // Reset semaphore count to 0 if signaled after timeout
            if (rtos_semaphore_get_count(smart_conf.semaphore))
                rtos_semaphore_wait(smart_conf.semaphore, 0);

            // Wait until finding ssid and pwd or until timeout has elapsed (60 sec)
            rtos_semaphore_wait(smart_conf.semaphore, 60000);

            if (smart_conf.status == SC_STATUS_SSID_PSWD_FOUND)
            {
                // Change to a STA interface
                if (fhost_set_vif_type(smart_conf.link_params, smart_conf.idx,
                                       VIF_STA, false))
                {
                    TRACE_FHOST("Smartconf: Failed to change to STA type\n");
                    break;
                }

                if(fhost_smart_get_ssid_pswd())
                {
                    TRACE_FHOST("Smartconf: Failed to get ssid & password\n");
                    goto exit;
                }
                // and connect to network
                if (fhost_smart_connect(smart_conf.ssid, smart_conf.pwd))
                    TRACE_FHOST("Smartconf: ERROR while connecting..\n.");

                smart_conf.status = SC_STATUS_CONNECTED;
                break;
            }
            else
            {
                smart_conf.status = SC_STATUS_WAIT;
                smart_conf.encryption_mode = UNKOWN_ENCRY;
                smart_conf.pattern_idx = 0;
                fhost_smart_init();
            }
        }
        else if (smart_conf.pattern_idx)
        {
            // The full pattern should be detected on the same channel
            smart_conf.encryption_mode = UNKOWN_ENCRY;
            smart_conf.pattern_idx = 0;
        }

        if(((PHY_CHNL_BW_40 == cfg.chan.type) && ((cfg.chan.center1_freq > cfg.chan.prim20_freq) || ((cfg.chan.prim20_freq >= 2457) && (cfg.chan.center1_freq < cfg.chan.prim20_freq))))) {
            i++;
            if (i == scan_result.cnt)
                i = 0;
            chan = scan_result.chan[i];
        } else {
            cfg.chan.type = PHY_CHNL_BW_40;
            if(cfg.chan.prim20_freq == cfg.chan.center1_freq) {
                if((cfg.chan.prim20_freq == 5180) || (cfg.chan.prim20_freq <= 2427)) { //chan1~chan4, chan36
                    cfg.chan.center1_freq += 10;
                } else {
                    cfg.chan.center1_freq -= 10;
                }
            } else if((cfg.chan.center1_freq == (cfg.chan.prim20_freq - 10)) && (((cfg.chan.prim20_freq >= 2432) && (cfg.chan.prim20_freq <= 2452)) || (cfg.chan.prim20_freq > 5180))) {//chan10~chan13,chan40+
                cfg.chan.center1_freq = cfg.chan.prim20_freq + 10*((cfg.chan.center1_freq > cfg.chan.prim20_freq)? -1 : 1);
            }

            goto Monitor_cfg;
        }
    }

  exit:
    fhost_smartconf_exit();
    fhost_smart_deinit();
}

/****************************************************************************************
 * Task interface
 ***************************************************************************************/
int fhost_smartconf_start(int idx, struct fhost_cntrl_link *link)
{
    // init params
    smart_conf.idx = idx;
    smart_conf.pattern_idx = 0;
    smart_conf.encryption_mode = UNKOWN_ENCRY;
    smart_conf.status = SC_STATUS_WAIT;
    smart_conf.mac_addr_is_found = false;

    if (link)
    {
        smart_conf.local_link = false;
        smart_conf.link_params = link;
    }
    else
    {
        smart_conf.link_params = fhost_cntrl_cfgrwnx_link_open();
        if(smart_conf.link_params == NULL)
            return 2;
        smart_conf.local_link = true;
    }

    if (rtos_semaphore_create(&smart_conf.semaphore, 1, 0))
        return 4;

    if (rtos_task_create(fhost_smartconf_task, "smartconf", SMARTCONF_TASK,
                         TASK_STACK_SIZE_WIFI_SMARTCONF, NULL, TASK_PRIORITY_WIFI_SMARTCONF,
                         &smart_conf.smart_handle))
        return 5;

    return 0;
}

int fhost_smartconf_stop(void)
{
    if(smart_conf.status < SC_STATUS_SSID_PSWD_FOUND) {
        if (fhost_set_vif_type(smart_conf.link_params, smart_conf.idx, VIF_STA, false))
        {
            return -1;
        }
    }
    if(smart_conf.status >= SC_STATUS_SSID_PSWD_FOUND) {
        wlan_disconnect_sta(smart_conf.idx);
    }
    fhost_smartconf_exit();
    fhost_smart_deinit();

    return 0;
}
#endif // NX_SMARTCONFIG

/**
 * @}
 */
