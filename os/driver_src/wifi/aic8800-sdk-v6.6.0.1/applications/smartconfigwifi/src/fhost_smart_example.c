/**
 ****************************************************************************************
 *
 * @file fhost_smart_template.c
 *
 * @brief Definition of an template of smartconf for Fully Hosted firmware.
 *
 * Copyright (C) RivieraWaves 2019-2020
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup FHOST_SMARTCONF_EXAMPLE FHOST_SMARTCONF_EXAMPLE
 * @ingroup FHOST_SMARTCONF
 * @{
 *
 * This is a basic example of smartconf where SSID and password are hardcoded
 * to allow the connection to the AP.
 *
 ****************************************************************************************
 */
/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "string.h"
#include "console.h"
#include "fhost_smartconf.h"
#include "fhost_smart_mode.h"
#include "fhost_command_common.h"

#if NX_SMARTCONFIG
/*
 * DEFINITIONS
 ****************************************************************************************
 */
/// User code to replace default methods
#define CONFIG_USER_CODE_EN     0

/// dummy SSID for this example
#define SSID "SFL_24"
/// dummy password for this example
#define PWD ""

/*
 *
 * FUNCTIONS
 ****************************************************************************************
 */

#if (CONFIG_USER_CODE_EN)
/**
 ****************************************************************************************
 * @brief Dummy implementation of smartconfig state machine
 *
 * When a frame is received in SC_STATUS_WAIT state, update smart_conf.freq
 * and switch to SC_STATUS_CHANNEL_FOUND.
 *
 * When a frame is received in SC_STATUS_CHANNEL_FOUND state, update smart_conf.ssid,
 * smart_conf.pwd, smart_conf.encryption_mode (with hardcoded value) and switch to the
 * final state SC_STATUS_SSID_PSWD_FOUND.
 *
 * @param[in] info  RX Frame information.
 ****************************************************************************************
 */
static void fhost_smart_decode(struct fhost_frame_info *info)
{
    if (smart_conf.status == SC_STATUS_WAIT)
    {
        smart_conf.freq = info->freq;
        smart_conf.status = SC_STATUS_CHANNEL_FOUND;
        rtos_semaphore_signal(smart_conf.semaphore, false);
    }
    else if (smart_conf.status == SC_STATUS_CHANNEL_FOUND)
    {
        strcpy(smart_conf.ssid, SSID);
        strcpy(smart_conf.pwd, PWD);
        smart_conf.encryption_mode = ENCRY_NONE;

        smart_conf.status = SC_STATUS_SSID_PSWD_FOUND;
        rtos_semaphore_signal(smart_conf.semaphore, false);
    }
}

void fhost_smart_init(void)
{
}

void fhost_smart_deinit(void)
{
}

void fhost_smart_cb(struct fhost_frame_info *info, void *arg)
{
    fhost_smart_decode(info);
}

int32_t fhost_smart_get_ssid_pswd(void)
{
    return 0;
}

int32_t fhost_sendrsp_to_app(uint32_t ip_addr, uint8_t *mac_addr)
{
    (void)ip_addr;
    (void)mac_addr;
    return 0;
}

#endif /* CONFIG_USER_CODE_EN */

/******************************************************************************
 * Fhost FW interfaces:
 * These are the functions required by the fhost FW that are specific to the
 * final application.
 *****************************************************************************/
/*
 * fhost_application_init: Must initialize the minimum for the application to
 *                         start.
 * In this example only create a new task with the entry point set to the
 * 'fhost_example_task'. The rest on the initialization will be done in this
 * function.
 */
int fhost_application_init(void)
{
    console_cmd_add("smartconf",  "- Smart Config", 1, 1, do_smartconf);
    console_cmd_add("stopsc",     "- Stop Smart Config", 1, 1, do_stop_smartconf);
    return 0;
}

#endif /* NX_SMARTCONFIG */

/**
 * @}
 */
