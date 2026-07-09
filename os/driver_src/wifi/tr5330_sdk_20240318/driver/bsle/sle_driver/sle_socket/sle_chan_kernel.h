/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

#ifndef SLE_CHAN_KERNEL_H
#define SLE_CHAN_KERNEL_H

#include <net/netlink.h>

typedef void (*sle_send_frame_cb_t)(uint8_t *data, uint16_t data_len);

typedef enum {
    SLE_CHAN_SUCCESS = 0,
    SLE_CHAN_FAILED = -1,
    SLE_CHAN_MEMORY_ERR = -2,
    SLE_CHAN_PARAM_INVALID = -3,
} sle_chan_error_code_t;

typedef enum {
    SLE_CHAN_USER_PORT_HCI = 100,
    SLE_CHAN_USER_PORT_TM,
} sle_chan_user_port_t;

typedef enum {
    SLE_CHAN_NETLINK_HCI = 23,
    SLE_CHAN_NETLINK_TM,
} sle_chan_netlink_protocol_t;

typedef enum {
    SLE_CHAN_HCI,
    SLE_CHAN_TM,
    SLE_CHAN_MAX,
} sle_chan_id_t;

typedef struct sle_chan_cfg {
    sle_chan_id_t chan_id;
    sle_chan_user_port_t user_port;
    sle_chan_netlink_protocol_t netlink_protocol;
    sle_send_frame_cb_t send;
    struct netlink_kernel_cfg *netlink_cfg;
    struct sock *nlsk_sock;
} sle_chan_cfg_t;

int sle_hci_chan_recv_frame(uint8_t *data, uint16_t data_len);
int sle_tm_chan_recv_frame(uint8_t *data, uint16_t data_len);
void sle_hci_register_send_frame_cb(sle_send_frame_cb_t cb);
void sle_tm_register_send_frame_cb(sle_send_frame_cb_t cb);
int sle_chan_init(void);
void sle_chan_exit(void);

#endif