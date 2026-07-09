#ifndef _TUSB_RNDIS_HOST_H_
#define _TUSB_RNDIS_HOST_H_

#ifndef CFG_TUD_NET_MTU
#define CFG_TUD_NET_MTU           1514
#endif

#ifdef __cplusplus
 extern "C" {
#endif
//--------------------------------------------------------------------+
// RNDIS APPLICATION CALLBACKS
//--------------------------------------------------------------------+

TU_ATTR_WEAK extern void tuh_rndis_mount_cb(uint8_t daddr);
TU_ATTR_WEAK extern void tuh_rndis_umount_cb(uint8_t daddr);
TU_ATTR_WEAK void rndish_connect_cb(uint8_t mac[6]);

TU_ATTR_WEAK void rndish_receive_cb(uint8_t *data, uint16_t len);

void rndish_task       (void);
bool rndish_send       (uint8_t *data, uint16_t len);


void rndish_init       (void);
bool rndish_open       (uint8_t rhport, uint8_t dev_addr, tusb_desc_interface_t const *itf_desc, uint16_t max_len);
bool rndish_set_config (uint8_t dev_addr, uint8_t itf_num);
bool rndish_xfer_cb    (uint8_t dev_addr, uint8_t ep_addr, xfer_result_t event, uint32_t xferred_bytes);
void rndish_close      (uint8_t dev_addr);



#ifdef __cplusplus
 }
#endif

#endif