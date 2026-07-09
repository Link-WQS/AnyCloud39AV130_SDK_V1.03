/*
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved.
 */

#include <net/net_namespace.h>
#include <linux/sched.h>
#include <linux/security.h>

#include "oal_schedule.h"
#include "securec.h"
#include "sle_chan_kernel.h"

static void sle_hci_chan_send_frame(struct sk_buff *skb);
static void sle_tm_chan_send_frame(struct sk_buff *skb);

struct netlink_kernel_cfg cfg_hci = {
    .groups = 0,
    .flags = 0,
    .input = sle_hci_chan_send_frame,
    .cb_mutex = NULL,
    .bind = NULL,
    .unbind = NULL,
    .compare = NULL
};
struct netlink_kernel_cfg cfg_tm = {
    .groups = 0,
    .flags = 0,
    .input = sle_tm_chan_send_frame,
    .cb_mutex = NULL,
    .bind = NULL,
    .unbind = NULL,
    .compare = NULL
};
sle_chan_cfg_t g_sle_chan_cfg[SLE_CHAN_MAX] = {
    {
        .chan_id = SLE_CHAN_HCI,
        .user_port = SLE_CHAN_USER_PORT_HCI,
        .netlink_protocol = SLE_CHAN_NETLINK_HCI,
        .netlink_cfg = &cfg_hci,
    },
    {
        .chan_id = SLE_CHAN_TM,
        .user_port = SLE_CHAN_USER_PORT_TM,
        .netlink_protocol = SLE_CHAN_NETLINK_TM,
        .netlink_cfg = &cfg_tm,
    }
};

static sle_chan_cfg_t *sle_chan_find_cfg(sle_chan_id_t chan_id)
{
    int i;
    for (i = 0; i < SLE_CHAN_MAX; i++) {
        if (g_sle_chan_cfg[i].chan_id == chan_id) {
            return &g_sle_chan_cfg[i];
        }
    }
    return NULL;
}
static void sle_register_send_frame_cb(sle_chan_id_t chan_id, sle_send_frame_cb_t cb)
{
    sle_chan_cfg_t *chan_cfg = sle_chan_find_cfg(chan_id);
    if (chan_cfg == NULL) {
        oal_print_err("[sle_register_send_frame_cb] not find chan, chan_id: %d\n", chan_id);
        return;
    }
    chan_cfg->send = cb;
}

void sle_hci_register_send_frame_cb(sle_send_frame_cb_t cb)
{
    sle_register_send_frame_cb(SLE_CHAN_HCI, cb);
}
void sle_tm_register_send_frame_cb(sle_send_frame_cb_t cb)
{
    sle_register_send_frame_cb(SLE_CHAN_TM, cb);
}

static int sle_chan_recv_frame(sle_chan_id_t chan_id, uint8_t *data, uint16_t data_len)
{
    struct nlmsghdr *nlh;
    int ret;
    struct sk_buff *nl_skb;
    sle_chan_cfg_t *chan_cfg;
    if (data == NULL || data_len == 0) {
        return SLE_CHAN_PARAM_INVALID;
    }
    nl_skb = nlmsg_new(data_len, GFP_ATOMIC);
    chan_cfg = sle_chan_find_cfg(chan_id);
    if (chan_cfg == NULL) {
        oal_print_err("[sle_chan_recv_frame] not find chan, chan_id: %d\n", chan_id);
        return SLE_CHAN_PARAM_INVALID;
    }
    /* 设置netlink消息头部 */
    nlh = nlmsg_put(nl_skb, 0, 0, chan_cfg->netlink_protocol, data_len, 0);
    if (nlh == NULL) {
        oal_print_err("nlmsg_put failaure\n");
        nlmsg_free(nl_skb);
        return SLE_CHAN_MEMORY_ERR;
    }
    ret = memcpy_s(nlmsg_data(nlh), data_len, data, data_len);
    if (ret != EOK) {
        return SLE_CHAN_MEMORY_ERR;
    }
    ret = netlink_unicast(chan_cfg->nlsk_sock, nl_skb, chan_cfg->user_port, MSG_DONTWAIT);
    return ret;
}
int sle_hci_chan_recv_frame(uint8_t *data, uint16_t data_len)
{
    return sle_chan_recv_frame(SLE_CHAN_HCI, data, data_len);
}
int sle_tm_chan_recv_frame(uint8_t *data, uint16_t data_len)
{
    return sle_chan_recv_frame(SLE_CHAN_TM, data, data_len);
}
static void sle_chan_send_frame(sle_chan_id_t chan_id, struct sk_buff *skb)
{
    struct nlmsghdr *nlh = NULL;
    uint8_t *umsg = NULL;
    sle_chan_cfg_t *chan_cfg;
    uint16_t udata_len = 0;
    if (skb == NULL || skb->len < nlmsg_total_size(0)) {
        oal_print_err("netlink channel skb err! %d\n", chan_id);
        return;
    }

    if (!netlink_capable(skb, CAP_NET_ADMIN)) {
        oal_print_err("sle chan send permissions error\n");
        return;
    }

    nlh = nlmsg_hdr(skb);
    umsg = NLMSG_DATA(nlh);
    udata_len = (uint16_t)nlmsg_len(nlh);
    chan_cfg = sle_chan_find_cfg(chan_id);
    if (chan_cfg == NULL) {
        oal_print_err("[sle_chan_send_frame] not find chan, chan_id: %d\n", chan_id);
        return;
    }
    if (chan_cfg->send) {
        chan_cfg->send(umsg, udata_len);
    } else {
        oal_print_err("[sle_chan_send_frame] chan send fun null, chan_id: %d\n", chan_id);
    }
}

static void sle_hci_chan_send_frame(struct sk_buff *skb)
{
    sle_chan_send_frame(SLE_CHAN_HCI, skb);
}

static void sle_tm_chan_send_frame(struct sk_buff *skb)
{
    sle_chan_send_frame(SLE_CHAN_TM, skb);
}

int sle_chan_init(void)
{
    uint32_t i;
    oal_print_err("[sle_chan_init] init sle chan\n");
    for (i = 0; i < SLE_CHAN_MAX; i++) {
        struct sock *sle_nlsk
            = netlink_kernel_create(&init_net, g_sle_chan_cfg[i].netlink_protocol, g_sle_chan_cfg[i].netlink_cfg);
        if (sle_nlsk == NULL) {
            return SLE_CHAN_FAILED;
        }
        g_sle_chan_cfg[i].nlsk_sock = sle_nlsk;
    }
    return SLE_CHAN_SUCCESS;
}

void sle_chan_exit(void)
{
    int i;
    for (i = 0; i < SLE_CHAN_MAX; i++) {
        if (g_sle_chan_cfg[i].nlsk_sock) {
            netlink_kernel_release(g_sle_chan_cfg[i].nlsk_sock);
        }
    }
}
