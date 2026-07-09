/**
 ****************************************************************************************
 *
 * @file fhost_smart_mode.h
 *
 * @brief Decode smart config algorithm for Fully Hosted firmware.
 *
 * This file is provided as a reference example on an "AS IS" basis. As such the content
 * of this file comes without warranty or indemnification of any kind. Notwithstanding
 * anything to the contrary in the Agreement, RivieraWaves and its affiliates accept no
 * liability of any kind with respect to this delivery.
 *
 * Copyright (C) RivieraWaves 2017-2019
 *
 ****************************************************************************************
 */
#ifndef _FHOST_SMARTCONF_MODE_H_
#define _FHOST_SMARTCONF_MODE_H_

/**
 ****************************************************************************************
 * @defgroup FHOST_SMARTCONF_MODE FHOST_SMARTCONF_MODE
 * @ingroup FHOST_SMARTCONF
 * @brief Fully Hosted smart config algorithm.
 *
 * @{
 ****************************************************************************************
 */

#include "fhost_rx.h"
#include "net_al.h"

/*
 * FUNCTIONS
 ****************************************************************************************
 */
/**
 ****************************************************************************************
 * @brief Monitor mode callback function
 *
 * This function, called each time a new frame is received, is responsible to
 * retrieve the AP credentials from the received frames.
 *
 * The implementation is specific to each 'smartconfig mode' but each one must implement
 * the following state machine:
 * - SC_STATUS_WAIT: This is the init state. In this state it must search for specific
 * code indicating that smartconfig procedure is on going on this channel. When such code
 * is detected the function must update smart_conf.freq and switch to
 * SC_STATUS_CHANNEL_FOUND.
 * If no code is detected after 1sec, the smartconfig task will switch to another channel.
 *
 * - SC_STATUS_CHANNEL_FOUND: In this state it must start the 'smartconfig' algorithm to
 * retrieve AP credentials. Once credentials have been retrieved the function must copy
 * them in smart_conf.ssid, smart_conf.encryption_mode, smart_conf.pwd and switch to
 * SC_STATUS_SSID_PSWD_FOUND.
 * If state SC_STATUS_SSID_PSWD_FOUND is not reached after 60sec, the smartconfig task
 * will reset to SC_STATUS_WAIT and switch to another channel.
 *
 * - SC_STATUS_SSID_PSWD_FOUND: Once this state is reached the smartconfig task will start
 * the connection to the AP, so this function will never be called in this state.
 *
 * To update the state machine, the function must update the smart_conf.status variable
 * and signal the smartconfig using the smart_conf.semaphore.
 *
 * @param[in] info  Information of the frame received in monitor mode.
 * @param[in] arg   Not used
 ****************************************************************************************
 */
void fhost_smart_cb(struct fhost_frame_info *info, void *arg);

/**
 ****************************************************************************************
 * @brief This function initializes the smart protocol parameters
 ****************************************************************************************
 */
void fhost_smart_init();

void fhost_smart_deinit();

int32_t fhost_sendrsp_to_app(uint32_t ip_addr, uint8_t *mac_addr);

int32_t fhost_smart_get_ssid_pswd(void);

/**
 * @}
 */
#endif /* _FHOST_SMARTCONF_MODE_H_ */


