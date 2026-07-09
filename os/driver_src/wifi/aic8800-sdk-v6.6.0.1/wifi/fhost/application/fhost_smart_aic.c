/**
 ****************************************************************************************
 *
 * @file fhost_smart_aic.c
 *
 * @brief Definition of an example of smartconf for Fully Hosted firmware.
 *
 * Copyright (C) AIC 2019-2020
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup FHOST_SMARTCONF_EXAMPLE FHOST_SMARTCONF_EXAMPLE
 * @ingroup FHOST_SMARTCONF
 * @{
 *
 * This is a basic example of smartconf.
 *
 ****************************************************************************************
 */
/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "string.h"
#include "sockets.h"
#include "rwnx_config.h"
#include "fhost_smartconf.h"
#include "fhost_smart_mode.h"
#include "def.h"
#include "mac_frame.h"

#if NX_SMARTCONFIG
/*
 * DEFINITIONS
 ****************************************************************************************
 */
#define SC_RXBUF_MAX_LEN        200

#define GUIDE_CODE_LEN              4
#define GUIDE_CODE_START            356
#define GUIDE_CODE_STEP             3
#define SMART_CONFIG_EXTRA_LEN      45

#define SMARTCONFIG_REPORT_MSG_OFFSET 9
#define SMARTCONFIG_REPORT_PORT       18800

#define SMART_CONF_FRM_LEN_VALID_CHECK(len) do { if (len < SMART_CONFIG_EXTRA_LEN || len > SMART_CONFIG_EXTRA_LEN + 0x1FF) { \
                                                        return -1;                                                           \
                                                    }                                                                        \
                                                    if (len >= GUIDE_CODE_START) {                                           \
                                                        return -1;                                                           \
                                                    }                                                                        \
                                                    len -= SMART_CONFIG_EXTRA_LEN;                                           \
                                                    if (((len & 0xFF00)) && ((len & 0xFF00) != 0x100)) {                     \
                                                        return -1;                                                           \
                                                    }                                                                        \
                                                } while(0)

#define MACDBG                          "%02x:%02x:%02x:%02x:%02x:%02x"
#define MAC2STRDBG(ea)                  (ea)[0], (ea)[1], (ea)[2], (ea)[3], (ea)[4], (ea)[5]

struct guide_code
{
    uint16_t gc_len;
    uint16_t gc[GUIDE_CODE_LEN];
    uint8_t  is_uf[GUIDE_CODE_LEN];
};

enum smart_conf_element_idx
{
    SC_TOTAL_LEN    = 0,
    SC_PWD_LEN      = 1,
    SC_SSID_CRC     = 2,
    SC_BSSID_CRC    = 3,
    SC_TOTAL_XOR    = 4,
    SC_IP_0         = 5,
    SC_IP_1         = 6,
    SC_IP_2         = 7,
    SC_IP_3         = 8,
    SC_ELEMENT_MAX
};
#define SMART_CONFIG_EXTRA_HEAD_LEN SC_ELEMENT_MAX
#define GET_ELM(idx) (p_smart_conf_user->element[idx])

struct smartconfig_t
{
    uint8_t element[SC_ELEMENT_MAX];

    uint8_t ssid_hidden;
    uint8_t ssid_len;
    char SSID[32+1];
    uint8_t BSSID[6];

    char pwd[32+1];

    uint8_t channel_found;
    int32_t from_ds;
    int32_t uf;
    uint8_t buf_len;
    uint8_t buf[SC_RXBUF_MAX_LEN+1];
    uint8_t buf_valid[SC_RXBUF_MAX_LEN+1];
    uint16_t len_offset;
    struct guide_code normal_gc;
    struct guide_code uf_gc;
    uint16_t parse_info[3];
};

/*
 * GLOBAL VARIABLES
 ****************************************************************************************
 */
struct smartconfig_t *p_smart_conf_user;

/*
 *
 * FUNCTIONS
 ****************************************************************************************
 */
static int32_t sc_is_multicast_ether_addr(const uint8_t *a)
{
    return a[0] & 0x01;
}

static void smart_parse_element(uint16_t *info)
{
    uint32_t i = 0;
    uint8_t pos = 0, val = 0, crc = 0;
    uint8_t check_crc;
    uint8_t check_xor;

    pos = info[1] & 0xFF;
    crc = (info[0] & 0xF0) | ((info[2] & 0xF0) >> 4);
    val = ((info[0] & 0x0F) << 4) | (info[2] & 0x0F);
    check_crc = co_crc8(&val, 1, 0);
    check_crc = co_crc8(&pos, 1, check_crc);
    if (check_crc == crc) {
        dbg(D_INF "p_smart_conf_user->buf_len %d\r\n", p_smart_conf_user->buf_len);
        if (pos > p_smart_conf_user->buf_len)
            p_smart_conf_user->buf_len = pos;
        dbg(D_INF "p_smart_conf_user->buf_valid[%d] %d\r\n", pos, p_smart_conf_user->buf_valid[pos]);
        if (!p_smart_conf_user->buf_valid[pos]) {
            p_smart_conf_user->buf[pos] = val;
            p_smart_conf_user->buf_valid[pos] = 1;
            dbg(D_INF "[%02d] = %02x\r\n", pos, val);
            if(pos < SC_ELEMENT_MAX)
                p_smart_conf_user->element[pos] = val;
        }
        dbg(D_INF "p_smart_conf_user->total_len %d:%d\r\n", GET_ELM(SC_TOTAL_LEN), p_smart_conf_user->buf_len);
        if (GET_ELM(SC_TOTAL_LEN) != 0 &&
                p_smart_conf_user->buf_len + 1 >= SMART_CONFIG_EXTRA_HEAD_LEN) {
            check_xor = 0;
            for (i = 0; i <= p_smart_conf_user->buf_len; ++i) {
                if (p_smart_conf_user->buf_valid[i] == 0)
                    break;
                else if (i != 4)
                    check_xor ^= p_smart_conf_user->buf[i];
            }
            dbg(D_INF "check_xor %x: total_xor %x\r\n", check_xor, GET_ELM(SC_TOTAL_XOR));
            if (i == p_smart_conf_user->buf_len + 1 && check_xor == GET_ELM(SC_TOTAL_XOR)) {
                if (p_smart_conf_user->BSSID[0] != 0xFF) {//Normal
                    if (co_crc8(p_smart_conf_user->BSSID, MAC_ADDR_LEN, 0) == GET_ELM(SC_BSSID_CRC)) {
                        dbg(D_INF "pwdlen %d\r\n", GET_ELM(SC_PWD_LEN));
                        if (p_smart_conf_user->buf_len + 1 == GET_ELM(SC_TOTAL_LEN)) {
                            p_smart_conf_user->ssid_hidden = 1;
                            smart_conf.status = SC_STATUS_SSID_PSWD_FOUND;
                            dbg("State: Hidden SSID/password found\r\n");
                        } else if (p_smart_conf_user->buf_len + 1 == SMART_CONFIG_EXTRA_HEAD_LEN + GET_ELM(SC_PWD_LEN)) {
                            p_smart_conf_user->ssid_hidden = 0;
                            smart_conf.status = SC_STATUS_SSID_PSWD_FOUND;
                            dbg("State: Unhidden SSID/password found\r\n");
                        }
                    } else {
                        dbg(D_INF "BSSID %02x:%02x\r\n", co_crc8(p_smart_conf_user->BSSID, MAC_ADDR_LEN, 0), GET_ELM(SC_BSSID_CRC));
                    }
                } else {//UF
                    dbg(D_INF "UF pwdlen %d, buf_len %d, total_len %d\r\n", GET_ELM(SC_PWD_LEN), p_smart_conf_user->buf_len, GET_ELM(SC_TOTAL_LEN));
                    if (p_smart_conf_user->buf_len + 1 == GET_ELM(SC_TOTAL_LEN)) {
                        p_smart_conf_user->ssid_hidden = 1;
                        smart_conf.status = SC_STATUS_SSID_PSWD_FOUND;
                        dbg("State: Hidden SSID/password found\r\n");
                    } else if (p_smart_conf_user->buf_len + 1 == SMART_CONFIG_EXTRA_HEAD_LEN + GET_ELM(SC_PWD_LEN)) {
                        p_smart_conf_user->ssid_hidden = 0;
                        smart_conf.status = SC_STATUS_SSID_PSWD_FOUND;
                        dbg("State: Unhidden SSID/password found\r\n");
                    }
                }
            }
        }
    }
    return;
}

static uint8_t smart_check_valid_frame(void *frame, uint16_t frame_len)
{
    struct mac_hdr *machdr_ptr;
    struct mac_addr addr1;
    struct mac_addr addr2;
    struct mac_addr addr3;
    struct mac_addr temp_bssid;
    struct mac_addr temp_da;
    static uint16_t seq_num = 0xFFFF;


    if(frame) {
        machdr_ptr = (struct mac_hdr *)frame;
        if ((machdr_ptr->fctl & MAC_FCTRL_TYPE_MASK) != MAC_FCTRL_DATA_T)
            return 0;

        dbg(D_INF "len:%d, data %x\r\n", frame_len, frame);

        addr1 = machdr_ptr->addr1;
        addr2 = machdr_ptr->addr2;
        addr3 = machdr_ptr->addr3;

        if (p_smart_conf_user->from_ds == 1) { //from DS
            if (machdr_ptr->fctl & MAC_FCTRL_TODS) {
                return 0;
            }
            temp_bssid = addr2;
            temp_da = addr1;
        } else { //to DS
            if (machdr_ptr->fctl & MAC_FCTRL_FROMDS) {
                return 0;
            }
            temp_bssid = addr1;
            temp_da = addr3;
        }

        if (!sc_is_multicast_ether_addr((uint8_t *)temp_da.array))  {//Destion addr is broadcast addr
            return 0;
        }
        dbg(D_INF "sn %x, %x\r\n", machdr_ptr->seq>>4, seq_num>>4);

        if (seq_num == machdr_ptr->seq)
            return 0;

        seq_num = machdr_ptr->seq;

        if(smart_conf.status == SC_STATUS_WAIT) {
            memcpy(p_smart_conf_user->BSSID, temp_bssid.array, MAC_ADDR_LEN);
        }
    }else {
        if(smart_conf.status == SC_STATUS_WAIT) {
            p_smart_conf_user->BSSID[0] = 0xFF;
        }
    }

    return 1;
}

static int32_t smart_decode_for_ssid_pswd(uint16_t frame_len, void *frame)
{
    int32_t len = frame_len - p_smart_conf_user->len_offset;
    uint16_t *info = p_smart_conf_user->parse_info;

    dbg(D_INF "Enter %s:len:%d\r\n", __func__, frame_len);

    if(0 == smart_check_valid_frame(frame, frame_len))
        return -1;

    if ((p_smart_conf_user->uf && !frame) || (!p_smart_conf_user->uf && frame)) {

        SMART_CONF_FRM_LEN_VALID_CHECK(len);

        info[0] = info[1];
        info[1] = info[2];
        info[2] = len;

        dbg(D_INF "info %02x %02x %02x\r\n", info[0], info[1], info[2]);

        if (!(len & 0xFF00) && ((info[1] & 0xFF00) == 0x100) && ((info[0] & 0xFF00) == 0)) {
            smart_parse_element(info);
            info[0] = info[1] = info[2] = 0;
        }
    }
    return 0;
}

static int32_t smart_decode_for_channel(uint16_t frame_len, void *frame)
{
    uint16_t *gc;
    uint8_t  *is_uf;
    uint16_t *gc_len;

    if(0 == smart_check_valid_frame(frame, frame_len))
        return -1;

    if (p_smart_conf_user->len_offset == 0) {
        if (frame_len < GUIDE_CODE_START || frame_len> GUIDE_CODE_START + 128)
            return -1;

        if (frame == NULL) { //UF
            gc = p_smart_conf_user->uf_gc.gc;
            gc_len = &(p_smart_conf_user->uf_gc.gc_len);
            is_uf = p_smart_conf_user->uf_gc.is_uf;
        } else {
            gc = p_smart_conf_user->normal_gc.gc;
            gc_len = &(p_smart_conf_user->normal_gc.gc_len);
            is_uf = p_smart_conf_user->normal_gc.is_uf;
        }

        dbg(D_INF "len(%d), %p\r\n", frame_len, frame);
        if ((*gc_len > 0) && (gc[*gc_len - 1] == (frame_len + GUIDE_CODE_STEP))) {
            gc[*gc_len] = frame_len;
            is_uf[*gc_len] = (frame == NULL);
            ++ (*gc_len);
        } else {
            gc[0] = frame_len;
            is_uf[0] = (frame == NULL);
            (*gc_len) = 1;
        }

        if (*gc_len == GUIDE_CODE_LEN) {
            p_smart_conf_user->len_offset = frame_len - GUIDE_CODE_START;

            if (!frame && (is_uf[0]&is_uf[1]&is_uf[2]&is_uf[3]))
                p_smart_conf_user->uf = 1;
            else if (frame && !(is_uf[0]|is_uf[1]|is_uf[2]|is_uf[3]))
                p_smart_conf_user->uf = 0;
            else
                dbg("Err: unkown\r\n");

            smart_conf.status = SC_STATUS_CHANNEL_FOUND;
            dbg("State: Channel found\r\n");
        }
    }

    return 0;
}

/**
 ****************************************************************************************
 * @brief implementation of p_smart_conf_user state machine
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
    dbg(D_INF "%s, state %d\r\n", __func__, smart_conf.status);
    if(smart_conf.status == SC_STATUS_WAIT)
    {
        smart_decode_for_channel(info->length, info->payload);

        if(smart_conf.status == SC_STATUS_CHANNEL_FOUND) {
            smart_conf.freq = info->freq;
            rtos_semaphore_signal(smart_conf.semaphore, false);
        }
    }
    else if(smart_conf.status == SC_STATUS_CHANNEL_FOUND)
    {
        smart_decode_for_ssid_pswd(info->length, info->payload);
        if(smart_conf.status == SC_STATUS_SSID_PSWD_FOUND) {
            rtos_semaphore_signal(smart_conf.semaphore, false);
        }
    }
}

__WEAK void fhost_smart_init(void)
{
    if (NULL == p_smart_conf_user) {
        p_smart_conf_user = (void *)rtos_malloc(sizeof_b(struct smartconfig_t));
        if (NULL == p_smart_conf_user) {
            dbg("Err: malloc p_smart_conf_user fail\r\n");
            return;
        }
    }
    memset(p_smart_conf_user, 0, sizeof_b(struct smartconfig_t));
}

__WEAK void fhost_smart_deinit(void)
{
    if(p_smart_conf_user) {
        rtos_free(p_smart_conf_user);
        p_smart_conf_user = NULL;
    }
}

__WEAK void fhost_smart_cb(struct fhost_frame_info *info, void *arg)
{
    fhost_smart_decode(info);
}

__WEAK int32_t fhost_smart_get_ssid_pswd(void)
{
    struct mac_scan_result result;
    int32_t result_idx = 0, result_cnt = 0;
    uint8_t times = 0;
    int freq_list[2];

    char *msg = (char *)p_smart_conf_user->buf;
    int32_t ret = 0;
    freq_list[0] = smart_conf.freq;
    freq_list[1] = 0;

    dbg(" %s, %d\r\n", __func__, smart_conf.freq);

    if (!p_smart_conf_user->ssid_hidden) {
scan:
        result_idx = 0;
        result_cnt = fhost_scan(smart_conf.link_params, smart_conf.idx, freq_list);
        if (result_cnt <= 0)
        {
            dbg("Smartconf: Failed to scan !\n");
        }

        while (0 != (result_cnt = fhost_get_scan_results(smart_conf.link_params, result_idx++, 1, &result)))
        {
            uint8_t bssid_crc, ssid_crc;
            bssid_crc = co_crc8((uint8_t *)result.bssid.array, MAC_ADDR_LEN, 0);
            ssid_crc = co_crc8((uint8_t *)result.ssid.array, result.ssid.length, 0);
            if (bssid_crc == GET_ELM(SC_BSSID_CRC) && ssid_crc == GET_ELM(SC_SSID_CRC)) {
                memcpy(p_smart_conf_user->BSSID, result.bssid.array, MAC_ADDR_LEN);
                memcpy(p_smart_conf_user->SSID, (const char *)(result.ssid.array), result.ssid.length);
                p_smart_conf_user->SSID[result.ssid.length] = '\0';
                p_smart_conf_user->ssid_len = result.ssid.length;
                dbg(D_INF "SSID_len %d\r\n", p_smart_conf_user->ssid_len);
                break;
            }
        }

        if (0 == result_cnt) {
            if (times < 5) {
                times++;
                goto scan;
            } else {
                dbg("ssid not found !\r\n");
                return -1;
            }
        }
    }else {
        p_smart_conf_user->ssid_len = GET_ELM(SC_TOTAL_LEN) - SMART_CONFIG_EXTRA_HEAD_LEN - GET_ELM(SC_PWD_LEN);
        memcpy(p_smart_conf_user->SSID, msg + SMART_CONFIG_EXTRA_HEAD_LEN + GET_ELM(SC_PWD_LEN), p_smart_conf_user->ssid_len);
        p_smart_conf_user->SSID[p_smart_conf_user->ssid_len] = '\0';
    }

    dbg("SSID(%s), BSSID:("MACDBG")\r\n", p_smart_conf_user->SSID, MAC2STRDBG(p_smart_conf_user->BSSID));

    if (GET_ELM(SC_PWD_LEN) > 0) {
        memcpy(p_smart_conf_user->pwd, msg + SMART_CONFIG_EXTRA_HEAD_LEN, GET_ELM(SC_PWD_LEN));
        p_smart_conf_user->pwd[GET_ELM(SC_PWD_LEN)] = '\0';
        dbg("Password:%s\r\n", p_smart_conf_user->pwd);
    } else {
        dbg("Open\r\n");
    }

    if (ret < 0) {
        dbg("error(%d), start smart_conf again\r\n", ret);
    } else {
        dbg("State: Smartconfig Done.\r\n");

        strcpy(smart_conf.ssid, p_smart_conf_user->SSID);
        smart_conf.encryption_mode = ENCRY_NONE;
        if(GET_ELM(SC_PWD_LEN) > 0) {
            if(GET_ELM(SC_PWD_LEN) > sizeof(smart_conf.pwd)) {
                dbg("Err: pwd too long \r\n");
                return -1;
            }
            memcpy(smart_conf.pwd, p_smart_conf_user->pwd, GET_ELM(SC_PWD_LEN));
            smart_conf.encryption_mode = ENCRY_CCMP;
        }
    }
    return ret;
}

__WEAK int32_t fhost_sendrsp_to_app(uint32_t ip_addr, uint8_t *mac_addr)
{
    char sm_rsp[11];
    int32_t bytes, retry;
    uint32_t sm_ip = 0;
    struct sockaddr_in sock_addr;
    int32_t sock = -1;

    memcpy(&sm_ip, &(GET_ELM(SC_IP_0)), 4);

    sm_rsp[0] = p_smart_conf_user->ssid_len + GET_ELM(SC_PWD_LEN) + SMARTCONFIG_REPORT_MSG_OFFSET;
    for (int32_t i = 0; i < MAC_ADDR_LEN; ++i)
        sm_rsp[1 + i] = mac_addr[i];

    memcpy(&sm_rsp[7], &ip_addr, 4);

    // create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        dbg("open socket error: %d\r\n", sock);
        return -1;
    }
    memset(&sock_addr, 0, sizeof(sock_addr));
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_addr.s_addr = sm_ip;
    sock_addr.sin_port = htons(SMARTCONFIG_REPORT_PORT);
    if (connect(sock, (struct sockaddr *)&sock_addr, sizeof(sock_addr)) < 0) {
        close(sock);
        return -2;
    }
    retry = 3;
retry_send:
    bytes = send(sock, &sm_rsp[0], 11, 0);
    if (bytes < 0) {
        dbg(D_INF "udp socket send: %d\r\n", bytes);
    }
    if (retry--)
        goto retry_send;

    close(sock);
    return 0;
}

#endif /* NX_SMARTCONFIG */
/**
 * @}
 */
