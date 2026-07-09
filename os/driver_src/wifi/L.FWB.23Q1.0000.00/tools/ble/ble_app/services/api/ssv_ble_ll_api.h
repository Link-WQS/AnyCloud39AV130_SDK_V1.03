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
 * @file ssv_ble_ll_api.h
 * @brief SSV BLE link-layer API functions
 */


#ifndef __SSV_BLE_LL_API_H__
#define __SSV_BLE_LL_API_H__


#ifndef SSV_PACKED_STRUCT
//SSV PACK Definitionf
#define SSV_PACKED_STRUCT_BEGIN
#define SSV_PACKED_STRUCT               //__attribute__ ((packed))
#define SSV_PACKED_STRUCT_END           //__attribute__((packed))
#define SSV_PACKED_STRUCT_STRUCT        __attribute__ ((packed))
#define SSV_PACKED_STRUCT_FIELD(x)      x
#endif



/*******************************************************************************
 *         Include Files
 ******************************************************************************/


/*******************************************************************************
 *         Defines
 ******************************************************************************/
typedef void (*SSV_TASK_FUNC_P)(void *);                ///< Task function pointer type
typedef void (*ssv_ble_ll_rx_pkt_cb_t)(uint8_t *, uint32_t);         ///< BLE LL RX packet callback type


/*******************************************************************************
 *         Enumerations
 ******************************************************************************/


/*******************************************************************************
 *         Structures
 ******************************************************************************/
/// Ble scan parameters
typedef struct {
    uint8_t                 scan_type;              /*!< Scan type */
    uint8_t                 own_addr_type;          /*!< Owner address type */
    uint8_t                 scan_filter_policy;     /*!< Scan filter policy */
    uint16_t                scan_interval;          /*!< Scan interval. This is defined as the time interval from
                                                      when the Controller started its last LE scan until it begins the subsequent LE scan.
                                                      Range: 0x0004 to 0x4000 Default: 0x0010 (10 ms)
                                                      Time = N * 0.625 msec
                                                      Time Range: 2.5 msec to 10.24 seconds*/
    uint16_t                scan_window;            /*!< Scan window. The duration of the LE scan. LE_Scan_Window
                                                      shall be less than or equal to LE_Scan_Interval
                                                      Range: 0x0004 to 0x4000 Default: 0x0010 (10 ms)
                                                      Time = N * 0.625 msec
                                                      Time Range: 2.5 msec to 10240 msec */
    uint8_t                 scan_duplicate;       /*!< The Scan_Duplicates parameter controls whether the Link Layer should filter out
                                                        duplicate advertising reports (BLE_SCAN_DUPLICATE_ENABLE) to the Host, or if the Link Layer should generate
                                                        advertising reports for each packet received */
} ssv_ble_ll_scan_params_t;

/// Advertising parameters
typedef struct {
    uint16_t                adv_int_min;        /*!< Minimum advertising interval for
                                                  undirected and low duty cycle directed advertising.
                                                  Range: 0x0020 to 0x4000 Default: N = 0x0800 (1.28 second)
                                                  Time = N * 0.625 msec Time Range: 20 ms to 10.24 sec */
    uint16_t                adv_int_max;        /*!< Maximum advertising interval for
                                                  undirected and low duty cycle directed advertising.
                                                  Range: 0x0020 to 0x4000 Default: N = 0x0800 (1.28 second)
                                                  Time = N * 0.625 msec Time Range: 20 ms to 10.24 sec Advertising max interval */
    uint8_t                 adv_type;           /*!< Advertising type */
    uint8_t                 own_addr_type;      /*!< Owner bluetooth device address type */
    uint8_t                 peer_addr[6];       /*!< Peer device bluetooth device address */
    uint8_t                 peer_addr_type;     /*!< Peer device bluetooth device address type, only support public address type and random address type */
    uint8_t                 channel_map;        /*!< Advertising channel map */
    uint8_t                 adv_filter_policy;  /*!< Advertising filter policy */
} ssv_ble_ll_adv_params_t;


// scan swlist action defines
#define SCAN_SWLIST_ACTION_ADD    0x00
#define SCAN_SWLIST_ACTION_DEL    0x01
#define SCAN_SWLIST_ACTION_CLR    0x02
#define SCAN_SWLIST_ACTION_LIST   0x03
// scan swlist tags defines
#define SCAN_SWLIST_TAGS_NAME    0x00
#define SCAN_SWLIST_TAGS_MAC     0x01

#define SCAN_SWLIST_MAX_NUM   6
#define SCAN_SWLIST_NAME_MAX_LENGTH  17

SSV_PACKED_STRUCT_BEGIN
typedef struct st_ble_set_scan_swlist
{
     unsigned int action; // 0: add, 1: del,  2: clear
     unsigned int type;   // 0: name 1: mac
     unsigned int index;  // 0 to (SCAN_SWLIST_MAX_NUM -1)
     unsigned char target[1 + SCAN_SWLIST_NAME_MAX_LENGTH]; // len+name(17bytes), len+mac(6bytes)+mac_type(1byte)
}SSV_PACKED_STRUCT_STRUCT ST_BLE_SET_SCAN_SWLIST;
SSV_PACKED_STRUCT_END


/*******************************************************************************
 *         Variables
 ******************************************************************************/


/*******************************************************************************
 *         Functions
 ******************************************************************************/
int32_t ssv_ble_ll_set_rx_pkt_cb(ssv_ble_ll_rx_pkt_cb_t rx_pkt_cb);
int32_t ssv_ble_ll_rx_pkt_handler(uint8_t *rx_packet, uint32_t rx_len);
int32_t ssv_ble_ll_init(void);
int32_t ssv_ble_ll_deinit(void);
int32_t ssv_ble_ll_set_advertising_data(uint8_t *raw_data, uint32_t raw_data_len);
int32_t ssv_ble_ll_set_scan_rsp_data(uint8_t *raw_data, uint32_t raw_data_len);
int32_t ssv_ble_ll_start_advertising(ssv_ble_ll_adv_params_t *adv_params);
int32_t ssv_ble_ll_stop_advertising(void);
int32_t ssv_ble_ll_set_scan_params(ssv_ble_ll_scan_params_t *scan_params);
int32_t ssv_ble_ll_start_scanning(void);
int32_t ssv_ble_ll_stop_scanning(void);
int32_t ssv_ble_ll_set_scan_swlist(uint8_t action, uint8_t type, uint8_t index, char *tag, uint8_t len, uint8_t addr_type);

#endif /* __SSV_BLE_CMD_H__ */

