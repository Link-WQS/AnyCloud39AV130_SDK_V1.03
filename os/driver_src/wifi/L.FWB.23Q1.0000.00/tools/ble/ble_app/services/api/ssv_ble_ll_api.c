/*
 * Copyright (c) 2023 iComm-semi Ltd.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * @file ssv_ble_ll_api.c
 * @brief SSV BLE link-layer API functions
 */


/*******************************************************************************
 *         Include Files
 ******************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

#include "ssv_ctl_common.h"
#include "ssv_netlink.h"
#include "ssv_ble_ll_api.h"


/*******************************************************************************
 *         Local Defines
 ******************************************************************************/
#define SCAN_SWLIST_MAX_NUM 6
#define SWILIST_MAX_NAME_LENGTH 17
#define BL_MAC_ADDR_LEN 6


/*******************************************************************************
 *         Local Enumerations
 ******************************************************************************/


/*******************************************************************************
 *         Local Structures
 ******************************************************************************/
struct swlist_name{
    uint8_t len;
    char val[SWILIST_MAX_NAME_LENGTH];
};

struct swlist_mac{
    uint8_t len;
    uint8_t addr[6]; 
    uint8_t type;
};


struct scan_swlist_tag {
    struct swlist_name name;
    struct swlist_mac mac;
};


/*******************************************************************************
 *         Local Variables
 ******************************************************************************/
ssv_ble_ll_rx_pkt_cb_t _gp_rx_pkt_cb = NULL;
static uint8_t _g_scan_duplicate = 1;
static bool g_ble_scanning = false;
static struct scan_swlist_tag swlist_tags[SCAN_SWLIST_MAX_NUM] = {0};

/*******************************************************************************
 *         Global Variables
 ******************************************************************************/


/*******************************************************************************
 *         Local Functions
 ******************************************************************************/
#if 0
static int32_t _ssv_ble_ll_rx_pkt_handler(uint8_t *rx_packet, uint32_t rx_len)
{
    if(rx_packet[0]==0x04) //Packet Type
    {
        switch(rx_packet[1])
        {
            case 0x3E: //LE Event Code
                //sub event code
                switch(rx_packet[3])
                {
                    case 0x01:printf("BLE RX: LE Connection Complete. len=%d\n",rx_len); break;
                    case 0x02:printf("BLE RX: LE Advertising Report.  len=%d\n",rx_len); break;
                    case 0x03:printf("BLE RX: LE Connection Update Complete. len=%d\n",rx_len); break;
                    case 0x04:printf("BLE RX: LE Read Remote Used Features Complete. len=%d\n",rx_len); break;
                    case 0x05:printf("BLE RX: LE Long Term Key Requested. len=%d\n",rx_len); break;
                    case 0x06:printf("BLE RX: LE Remote Connection Parameter Request. len=%d\n",rx_len); break;
                    case 0x07:printf("BLE RX: LE Data Length Change. len=%d\n",rx_len); break;
                    case 0x08:printf("BLE RX: LE Read Local P256 Public Key Complete. len=%d\n",rx_len); break;
                    case 0x09:printf("BLE RX: LE Generate DHKey Complete. len=%d\n",rx_len); break;
                    case 0x0A:printf("BLE RX: LE Enhanced Connection Complete. len=%d\n",rx_len); break;
                    case 0x0B:printf("BLE RX: LE Direct Advertising Report. len=%d\n",rx_len); break;
                    default:printf("BLE RX: Unknow sub event (%d)\n",rx_packet[3]); break;
                }
                break;
            //BT Event code
            case 0x05:printf("BLE RX: Disconnection Complete. len=%d\n",rx_len); break;
            case 0x08:printf("BLE RX: Encryption Change. len=%d\n",rx_len); break;
            case 0x0C:printf("BLE RX: Read Remote Version Information Complete. len=%d\n",rx_len); break;
            case 0x0E:printf("BLE RX: Command(0x%04X) Complete. len=%d\n",(rx_packet[4]|(rx_packet[5]<<8)), rx_len); break;
            case 0x0F:printf("BLE RX: Command Status. len=%d\n",rx_len); break;
            case 0x10:printf("BLE RX: Hardware Error. len=%d\n",rx_len); break;
            case 0x13:printf("BLE RX: Number Of Completed Packets. len=%d\n",rx_len); break;
            case 0x1A:printf("BLE RX: Data Buffer Overflow. len=%d\n",rx_len); break;
            case 0x30:printf("BLE RX: Encryption Key Refresh Complete. len=%d\n",rx_len); break;
            case 0x57:printf("BLE RX: Authenticated Payload Timeout Expired. len=%d\n",rx_len); break;
            default:printf("BLE RX: Unknow event (%d)\n",rx_packet[3]); break;
        }
    }

    return 0;
}
#endif

static int32_t _ssv_ble_ll_init(void)
{
    ST_BLE_DRV_CMD cmd = {0};

    cmd.cmdid = E_SSV_BLE_DRV_CMD_INIT;
    ssv_netlink_send_drv_cmd((char *)&cmd, BLE_DRV_CMD_HDR_LEN);
    return 0;
}

static int32_t _ssv_ble_ll_deinit(void)
{
    ST_BLE_DRV_CMD cmd = {0};

    cmd.cmdid = E_SSV_BLE_DRV_CMD_DEINIT;
    ssv_netlink_send_drv_cmd((char *)&cmd, BLE_DRV_CMD_HDR_LEN);
    return 0;
}

static int32_t _ssv_ble_ll_reset(void)
{
    char hcmd_buf[12] = {0};

    memset(hcmd_buf,0,sizeof(hcmd_buf));
    hcmd_buf[0]=0x1;
    //opcode: scan parameter
    hcmd_buf[1]=0x03;
    hcmd_buf[2]=0x0C;
    //parameter total length
    hcmd_buf[3]=0;
    ssv_netlink_send_cmd((char *)hcmd_buf, 4);
    printf("ble: HCI Reset\n");

    memset(hcmd_buf,0,sizeof(hcmd_buf));
    hcmd_buf[0]=0x1;
    //opcode: scan parameter
    hcmd_buf[1]=0x01;
    hcmd_buf[2]=0x0C;
    //parameter total length
    hcmd_buf[3]=8;
    //
    hcmd_buf[4]=0xFF;
    hcmd_buf[5]=0xFF;
    hcmd_buf[6]=0xFF;
    hcmd_buf[7]=0xFF;
    hcmd_buf[8]=0xFF;
    hcmd_buf[9]=0xFF;
    hcmd_buf[10]=0xFF;
    hcmd_buf[11]=0xFF;
    ssv_netlink_send_cmd((char *)hcmd_buf, 12);
    printf("ble: set event mask\n");

    hcmd_buf[0]=0x1;
    //opcode: scan parameter
    hcmd_buf[1]=0x01;
    hcmd_buf[2]=0x20;
    //parameter total length
    hcmd_buf[3]=8;
    //
    hcmd_buf[4]=0xFF;
    hcmd_buf[5]=0xFF;
    hcmd_buf[6]=0xFF;
    hcmd_buf[7]=0xFF;
    hcmd_buf[8]=0xFF;
    hcmd_buf[9]=0xFF;
    hcmd_buf[10]=0xFF;
    hcmd_buf[11]=0xFF;
    ssv_netlink_send_cmd((char *)hcmd_buf, 12);
    printf("ble: set le event mask\n");

    return 0;
}

static int32_t _ssv_ble_ll_set_adv_params(ssv_ble_ll_adv_params_t *adv_params)
{
    char hcmd_buf[19] = {0};

    hcmd_buf[0]=0x1;
    //opcode: set advertising parameters
    hcmd_buf[1]=0x06;
    hcmd_buf[2]=0x20;
    //parameter total length
    hcmd_buf[3]=15;
    //advertising interval min
    hcmd_buf[4]=(adv_params->adv_int_min&0xFF);
    hcmd_buf[5]=((adv_params->adv_int_min>>8)&0xFF);
    //advertising interval max
    hcmd_buf[6]=(adv_params->adv_int_max&0xFF);
    hcmd_buf[7]=((adv_params->adv_int_max>>8)&0xFF);
    //advertising type
    hcmd_buf[8]=adv_params->adv_type;
    //own address type
    hcmd_buf[9]=adv_params->own_addr_type;
    //direct address type
    hcmd_buf[10]=adv_params->peer_addr_type;
    //direct address
    memcpy((void *)&hcmd_buf[11], (const void *)&adv_params->peer_addr[0], 6);
    //advertising channel map
    hcmd_buf[17]=adv_params->channel_map;
    //advertising filter policy
    hcmd_buf[18]=adv_params->adv_filter_policy;
    ssv_netlink_send_cmd((char *)hcmd_buf, 19);
    printf("ble: set advertising parameters\n");

    return 0;
}

static int32_t _ssv_ble_ll_set_adv_enable(uint8_t enable)
{
    char hcmd_buf[5] = {0};

    hcmd_buf[0] = 0x1;
    //opcode: set advertising enable
    hcmd_buf[1] = 0x0A;
    hcmd_buf[2] = 0x20;
    //parameter total length
    hcmd_buf[3] = 0x01;
    //enable
    hcmd_buf[4] = enable;
    ssv_netlink_send_cmd((char *)hcmd_buf, 5);
    printf("ble: set advertising enable\n");

    return 0;
}


/*******************************************************************************
 *         Global Functions
 ******************************************************************************/
int32_t ssv_ble_ll_set_rx_pkt_cb(ssv_ble_ll_rx_pkt_cb_t rx_pkt_cb)
{
    _gp_rx_pkt_cb = rx_pkt_cb;
    return 0;
}

int32_t ssv_ble_ll_rx_pkt_handler(uint8_t *rx_packet, uint32_t rx_len)
{
    if(NULL != _gp_rx_pkt_cb)
    {
        _gp_rx_pkt_cb(rx_packet, rx_len);
    }
#if 0
    else
    {
        _ssv_ble_ll_rx_pkt_handler(rx_packet, rx_len);
    }
#endif
    return 0;
}

static void ssv_ble_ll_list_add(uint8_t type, uint8_t index, char *tag_str, uint8_t len, uint8_t addr_type)
{
    uint8_t i = 0;
    if (type == SCAN_SWLIST_TAGS_NAME)
    {
        swlist_tags[index].name.len = len;
        memcpy(&swlist_tags[index].name.val[0], (char*)tag_str, len);
    }
    else
    {
        for (i = 0; i < BL_MAC_ADDR_LEN; i++)
        {
            swlist_tags[index].mac.addr[i] = strtoul(tag_str+3*i, NULL, 16);
        }
        swlist_tags[index].mac.type = addr_type;
        swlist_tags[index].mac.len = BL_MAC_ADDR_LEN;
    }
}

static void ssv_ble_ll_list_del(uint8_t type, uint8_t index)
{
    if (type == SCAN_SWLIST_TAGS_NAME)
        memset(&swlist_tags[index].name, 0x0, sizeof(struct swlist_name));
    else
        memset(&swlist_tags[index].mac, 0x0, sizeof(struct swlist_mac));
}

static void ssv_ble_ll_list_clr(uint8_t type)
{
    uint8_t num = 0;
    if (type == SCAN_SWLIST_TAGS_NAME)
    {
        for (num = 0; num < SCAN_SWLIST_MAX_NUM; num++)
        {
            memset(&swlist_tags[num].name, 0x0, sizeof(struct swlist_name));
        }
    }
    else
    {
        for (num = 0; num < SCAN_SWLIST_MAX_NUM; num++)
        {
            memset(&swlist_tags[num].mac, 0x0, sizeof(struct swlist_mac));
        }
    }
}

static void ssv_ble_ll_list_reset(void)
{
     ssv_ble_ll_set_scan_swlist(SCAN_SWLIST_ACTION_CLR, SCAN_SWLIST_TAGS_MAC, 0, NULL, 0, 0xFF);
     ssv_ble_ll_set_scan_swlist(SCAN_SWLIST_ACTION_CLR, SCAN_SWLIST_TAGS_NAME, 0, NULL, 0, 0xFF);
    
}

static void ssv_ble_ll_list_scan_swlist(void)
{
    uint8_t i = 0;
    uint8_t num = 0;

    for(num = 0; num < SCAN_SWLIST_MAX_NUM; num++)
    {
        if(swlist_tags[num].name.len)
        {
            printf("swlist_tags[%d] name %s\n", num, &swlist_tags[num].name.val[0]);
        }

        if(swlist_tags[num].mac.len)
        {
            printf("swlist_tags[%d] mac type:%d, mac_addr:", num, swlist_tags[num].mac.type);
            for (i = 0; i < BL_MAC_ADDR_LEN; i++)
            {
                printf("0x%X ", swlist_tags[num].mac.addr[i]);
            }
            printf("\n");
        }
    }
}

int32_t ssv_ble_ll_set_scan_swlist(uint8_t action, uint8_t type, uint8_t index, char *tag, uint8_t len, uint8_t addr_type)
{
    ST_BLE_DRV_CMD *pcmd = NULL;
    ST_BLE_SET_SCAN_SWLIST *ptag=NULL;
    uint8_t bl_mac_addr[BL_MAC_ADDR_LEN];
    uint8_t i = 0;
    unsigned int size = sizeof(ST_BLE_DRV_CMD)+sizeof(ST_BLE_SET_SCAN_SWLIST);

    printf("action %d, type %d , index %d, len %d, addr_type %d\n",\
                   action,  type,  index, len, addr_type);
    if(action > SCAN_SWLIST_ACTION_LIST) goto error_exit;
    if(type > SCAN_SWLIST_TAGS_MAC) goto error_exit;
    if(index >= SCAN_SWLIST_MAX_NUM) goto error_exit;
    if(g_ble_scanning)
    {
        printf("Set the white list failed.\n");
        printf("plese stop scanning to set whitelist.\n");
        goto error_exit;
    }

    pcmd=malloc(size);
    if(NULL!=pcmd)
    {
        memset(pcmd, 0x0, size);
        pcmd->cmdid=E_SSV_BLE_DRV_CMD_SET_SCAN_SWLIST;
        pcmd->datalen=sizeof(ST_BLE_SET_SCAN_SWLIST);
        ptag=(ST_BLE_SET_SCAN_SWLIST *)pcmd->data;
        ptag->action = action;
        ptag->type = type;
        ptag->index = index;
        ptag->target[0] = (len > SCAN_SWLIST_NAME_MAX_LENGTH) ? SCAN_SWLIST_NAME_MAX_LENGTH : len;
        
        switch (action)
        {
            case SCAN_SWLIST_ACTION_ADD:
                if(type == SCAN_SWLIST_TAGS_MAC)
                {
                    if(ptag->target[0] != 6) goto error_exit;
                    //revert mac address to match to advertising report mac address
                    for (i = 0; i < BL_MAC_ADDR_LEN; i++)
                    {
                        bl_mac_addr[BL_MAC_ADDR_LEN-1-i] = strtoul(tag+3*i, NULL, 16);
                    }
                
                    memcpy(&ptag->target[1], &bl_mac_addr[0], ptag->target[0]);
                    ptag->target[0] += 1; // 0x07 0x01 0x02 0x03 0x04 0x05 0x06 0x01
                    // printf("mac addr: \n");
                    // ssv_hex_dump(&ptag->target[1], 6);
                    ptag->target[7] = addr_type;
                    ssv_ble_ll_list_add(type, index, tag, len, addr_type);
                }
                else
                {
                    if(len==0) goto error_exit;
                    memcpy(&ptag->target[1], tag, ptag->target[0]);
                    // printf("Name: \n");
                    // ssv_hex_dump(&ptag->target[1], 6);
                    ssv_ble_ll_list_add(type, index, tag, len, addr_type);
                }
                break;
            case SCAN_SWLIST_ACTION_DEL:
                ssv_ble_ll_list_del(type, index);
                break;
            case SCAN_SWLIST_ACTION_CLR:
                ssv_ble_ll_list_clr(type);
                break;
            case SCAN_SWLIST_ACTION_LIST:
                ssv_ble_ll_list_scan_swlist();
                break;
            default:
                break;
        }
    }
    ssv_netlink_send_drv_cmd((char *)pcmd, size);
    free(pcmd);
    return 0;
error_exit:
    return -1;
}

int32_t ssv_ble_ll_init(void)
{
    _ssv_ble_ll_init();
    usleep(100000);
    _ssv_ble_ll_reset();
    g_ble_scanning = false;
    ssv_ble_ll_list_reset();
    return 0;
}

int32_t ssv_ble_ll_deinit(void)
{
    _ssv_ble_ll_deinit();
    usleep(100000);
    _ssv_ble_ll_reset();
    g_ble_scanning = false;
    ssv_ble_ll_list_reset();
    return 0;
}

int32_t ssv_ble_ll_set_advertising_data(uint8_t *raw_data, uint32_t raw_data_len)
{
    char hcmd_buf[36] = {0};

    if(31 < raw_data_len)
    {
        return -1;
    }

    hcmd_buf[0] = 0x1;
    //opcode: set advertising data
    hcmd_buf[1] = 0x08;
    hcmd_buf[2] = 0x20;
    //parameter total length
    hcmd_buf[3] = raw_data_len+1;
    // scan response data total length
    hcmd_buf[4] = raw_data_len;
    //scan response data
    memcpy((void *)&hcmd_buf[5], (const void *)raw_data, raw_data_len);
    printf("ble: set advertising data\n");
    ssv_netlink_send_cmd((char *)hcmd_buf, raw_data_len+5);

    return 0;
}

int32_t ssv_ble_ll_set_scan_rsp_data(uint8_t *raw_data, uint32_t raw_data_len)
{
    char hcmd_buf[36] = {0};

    if(31 < raw_data_len)
    {
        return -1;
    }

    hcmd_buf[0] = 0x1;
    //opcode: set scan response
    hcmd_buf[1] = 0x09;
    hcmd_buf[2] = 0x20;
    //parameter total length
    hcmd_buf[3] = raw_data_len+1;
    //scan response data total length
    hcmd_buf[4] = raw_data_len;
    //scan response data
    memcpy((void *)&hcmd_buf[5], (const void *)raw_data, raw_data_len);
    printf("ble: set scan response data\n");
    ssv_netlink_send_cmd((char *)hcmd_buf, raw_data_len+5);
    return 0;
}


int32_t ssv_ble_ll_start_advertising(ssv_ble_ll_adv_params_t *adv_params)
{
    _ssv_ble_ll_set_adv_params(adv_params);
    _ssv_ble_ll_set_adv_enable(1);
    return 0;
}

int32_t ssv_ble_ll_stop_advertising(void)
{
    _ssv_ble_ll_set_adv_enable(0);
    return 0;
}

int32_t ssv_ble_ll_set_scan_params(ssv_ble_ll_scan_params_t *scan_params)
{
    char hcmd_buf[11] = {0};

    memset(hcmd_buf, 0, sizeof(hcmd_buf));
    //HCI command packet
    hcmd_buf[0] = 0x1;
    //opcode: scan parameter
    hcmd_buf[1] = 0x0B;
    hcmd_buf[2] = 0x20;
    //parameter total length
    hcmd_buf[3] = 7;
    //scan type.
    hcmd_buf[4] = scan_params->scan_type;
    //scan interval
    hcmd_buf[5] = (scan_params->scan_interval&0xFF);
    hcmd_buf[6] = ((scan_params->scan_interval>>8)&0xFF);
    //scan window
    hcmd_buf[7] = (scan_params->scan_window&0xFF);
    hcmd_buf[8] = ((scan_params->scan_window>>8)&0xFF);
    //own address type
    hcmd_buf[9] = scan_params->own_addr_type;
    //scan filter policy
    hcmd_buf[10] = scan_params->scan_filter_policy;
    _g_scan_duplicate = scan_params->scan_duplicate;
    ssv_netlink_send_cmd((char *)hcmd_buf, 11);
    printf("ble: set scan parameter\n");

    return 0;
}

int32_t ssv_ble_ll_start_scanning(void)
{
    char hcmd_buf[6] = {0};

    hcmd_buf[0] = 0x1;
    //opcode: scan enable
    hcmd_buf[1] = 0x0C;
    hcmd_buf[2] = 0x20;
    //parameter total length
    hcmd_buf[3] = 2;
    //scan enable
    hcmd_buf[4] = 0x1;
    //duplicate filter
    hcmd_buf[5] = _g_scan_duplicate;
    ssv_netlink_send_cmd((char *)hcmd_buf, 6);
    g_ble_scanning = true;
    printf("ble: scan enable\n");
    return 0;
}

int32_t ssv_ble_ll_stop_scanning(void)
{
    char hcmd_buf[6] = {0};

    hcmd_buf[0] = 0x1;
    //opcode: scan enable
    hcmd_buf[1] = 0x0C;
    hcmd_buf[2] = 0x20;
    //parameter total length
    hcmd_buf[3] = 2;
    //scan disable
    hcmd_buf[4] = 0x0;
    //duplicate filter disable
    hcmd_buf[5] = 0x0;
    ssv_netlink_send_cmd((char *)hcmd_buf, 6);

    g_ble_scanning = false;
    printf("ble: scan disable\n");

    return 0;
}
