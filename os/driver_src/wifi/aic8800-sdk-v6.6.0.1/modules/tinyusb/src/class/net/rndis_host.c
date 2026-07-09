#include "tusb_option.h"
#include "host/usbh.h"
#include "host/usbh_pvt.h" //#include "host/usbh_classdriver.h"
#include "rndis_host.h"
#include "../cdc/cdc.h"
#include "rndis_protocol.h"
#include "board.h"
#include "bsp/board_api.h"
#include "dbg.h"

#define RNDISH_SUBCLASS_CDC        0x02
#define RNDISH_SUBCLASS_WIRELESS   0x01

#define RNDISH_PROTOCOL_CDC        0xFF
#define RNDISH_PROTOCOL_WIRELESS   0x03

#define RESET_DELAY 50

typedef struct {
  uint8_t daddr;
  uint8_t bInterfaceNumber;
  uint8_t bInterfaceSubClass;
  uint8_t bInterfaceProtocol;

  struct
  {
    uint32_t offset;
    uint32_t count;
    uint32_t list[6];
  } oid_support;
  uint32_t oid;
  uint32_t frame_size;
  uint32_t msg_type;
  uintptr_t reply_type;
  uint8_t  mac[6];
  uint32_t link_speed;
  bool     is_set;
  uint8_t  *receive_data;
  uint16_t receive_length;
  uint32_t request_id;
  uint8_t ep_notif;
  uint8_t ep_in;
  uint16_t epin_size;
  uint8_t ep_out;
  uint16_t epout_size;

} rndish_interface_t;

//#define DEBUG(format, ...) printf(format, ##__VA_ARGS__)
#define DEBUG(format, ...)  dbg(format, ##__VA_ARGS__)


bool rndish_receive(rndish_interface_t *p_randis);
bool rndish_get_control_cmplt(uintptr_t cmpltcmd);
void rndish_set_control_query_msg(uint32_t oid, uint16_t query_len);
void rndish_set_control_keepalive_msg(void);
void rndish_set_control_set_msg(uint32_t oid,  uint8_t *msg, uint16_t msg_len);
bool rndish_control_xfer(uint32_t command, uint8_t *data, uint16_t len);

CFG_TUSB_MEM_SECTION CFG_TUSB_MEM_ALIGN static uint8_t received[sizeof(rndis_data_packet_t) + 0x1000 + sizeof(rndis_data_packet_t)];
CFG_TUSB_MEM_SECTION CFG_TUSB_MEM_ALIGN static uint8_t transmitted[sizeof(rndis_data_packet_t) + 0x1000 + sizeof(rndis_data_packet_t)];

static rndish_interface_t rndish_data[CFG_TUH_RNDIS];

void rndish_init(void)
{
    DEBUG("rndish_init\n");
    tu_memclr(rndish_data, sizeof(rndish_data));
    tu_memclr(received, sizeof(received));
    tu_memclr(transmitted, sizeof(transmitted));
}


void rndish_task(void)
{
    rndish_interface_t *p_randis = &rndish_data[0];

    if (!tuh_ready(p_randis->daddr) || usbh_edpt_busy(p_randis->daddr, p_randis->ep_notif)) {
        return;
    }

    if (p_randis->receive_data) { // has receive data.
        if (rndish_receive_cb != NULL) {
            rndish_receive_cb(p_randis->receive_data, p_randis->receive_length);
        }
        p_randis->receive_data = NULL;
        p_randis->receive_length = 0;
        rndish_receive(p_randis); //receive next data.
    }

    if (p_randis->is_set) {
        const uint32_t interval_ms = 1500;
        static uint32_t start_ms = 0;
        if (board_millis() - start_ms < interval_ms) return; // not enough time
        DEBUG("keepalive\n");
        rndish_set_control_keepalive_msg(); //keepalive
        start_ms += interval_ms;
    }

    switch (p_randis->msg_type)
    {
        case REMOTE_NDIS_INITIALIZE_MSG:
            DEBUG("init msg complete\n");
            rndish_get_control_cmplt(REMOTE_NDIS_INITIALIZE_CMPLT);
            break;
        case REMOTE_NDIS_KEEPALIVE_MSG:
            if (p_randis->is_set) {
                rndish_get_control_cmplt(REMOTE_NDIS_KEEPALIVE_CMPLT);
            }
            break;
        case REMOTE_NDIS_QUERY_MSG: {
            rndish_get_control_cmplt(REMOTE_NDIS_QUERY_CMPLT);
            DEBUG("query msg complete,oid:0x%x\n", p_randis->oid);
            break;
        }
        case REMOTE_NDIS_SET_MSG: {
            DEBUG("set msg complete,oid:0x%x\n", p_randis->oid);
            rndish_get_control_cmplt(REMOTE_NDIS_SET_CMPLT);
            break;
        }
        default:
            DEBUG("invalid msg_type: \n", p_randis->msg_type);
            break;
    }
    p_randis->msg_type = 0;

    if (p_randis->reply_type > 0) {
        osal_task_delay(RESET_DELAY);
    }
    switch (p_randis->reply_type)
    {
        case REMOTE_NDIS_INITIALIZE_CMPLT: {
            rndis_initialize_cmplt_t *cmplt = ((void*)usbh_get_enum_buf());
            (void)cmplt;
            DEBUG("init cmplt len:%d\n",  cmplt->MessageLength);
            rndish_set_control_query_msg(OID_GEN_SUPPORTED_LIST, 0);
            break;
        }
        case REMOTE_NDIS_QUERY_CMPLT: {
            rndis_query_cmplt_t *query = ((void*)usbh_get_enum_buf());
            if (p_randis->oid == OID_GEN_SUPPORTED_LIST) {
                uint32_t i;
                uint32_t count = 0;
                uint32_t length = query->InformationBufferLength / 4;
                uint32_t *data = (uint32_t*)((void*)(query+1));
                DEBUG("query oid length:%d\n", length);
                for (i=0; i<length; i++) {
                    uint32_t oid = *(data+i);
                    if (oid == OID_GEN_MAXIMUM_FRAME_SIZE || oid == OID_GEN_LINK_SPEED
                        || oid == OID_GEN_MEDIA_CONNECT_STATUS || oid == OID_802_3_CURRENT_ADDRESS
                        || oid == OID_802_3_MAXIMUM_LIST_SIZE || oid == OID_802_3_PERMANENT_ADDRESS
                    ) {
                        DEBUG("query oid:0x%x\n", oid);
                        p_randis->oid_support.list[count++] = oid;
                    }
                }
                p_randis->oid_support.offset = 0;
                p_randis->oid_support.count = count;
            } else if (p_randis->oid == OID_GEN_MAXIMUM_FRAME_SIZE) {
                p_randis->frame_size = *(uint32_t*)((void*)(query+1));
                DEBUG("query frame size:%d\n", p_randis->frame_size);
            } else if (p_randis->oid == OID_GEN_LINK_SPEED) {
                p_randis->link_speed = *(uint32_t*)((void*)(query+1));
                DEBUG("query link speed:%d\n", p_randis->link_speed);
            } else if (p_randis->oid == OID_GEN_MEDIA_CONNECT_STATUS) {
                DEBUG("query link state\n");
            } else if (p_randis->oid == OID_802_3_CURRENT_ADDRESS) {
                uint8_t i;
                uint8_t *mac = (uint8_t*)((void*)(query+1));
                DEBUG("query mac address:");
                for (i=0; i<6; i++) {
                    p_randis->mac[i] = *(mac+i);
                    DEBUG(" %x", p_randis->mac[i]);
                }
                DEBUG("\n");

            } else if (p_randis->oid == OID_802_3_MAXIMUM_LIST_SIZE) {
                DEBUG("query maximum list size:%d\n", *(uint32_t*)((void*)(query+1)));
            } else if (p_randis->oid == OID_802_3_PERMANENT_ADDRESS) {
                uint8_t i;
                uint8_t *addr = (uint8_t*)((void*)(query+1));
                (void)addr;
                DEBUG("query permanent address:");
                for (i=0; i<6; i++) {
                    DEBUG(" %x", *(addr+i));
                }
                DEBUG("\n");
            }

            if (p_randis->oid_support.offset < p_randis->oid_support.count) {
                uint32_t oid = p_randis->oid_support.list[p_randis->oid_support.offset];
                uint16_t query_len = oid == OID_802_3_CURRENT_ADDRESS ||
                                                OID_802_3_PERMANENT_ADDRESS ? 6 : 4;
                DEBUG("query msg offset:%d, oid:0x%x, \n", p_randis->oid_support.offset, oid);
                rndish_set_control_query_msg(oid, query_len);
                p_randis->oid_support.offset++;
            } else if (!p_randis->is_set) {
                uint32_t packet_filter = 0x0f;
                DEBUG("set msg oid:0x%x\n", OID_GEN_CURRENT_PACKET_FILTER);
                rndish_set_control_set_msg(OID_GEN_CURRENT_PACKET_FILTER, (uint8_t*)&packet_filter, 4);
            }

            break;
        }
        case REMOTE_NDIS_SET_CMPLT: {
            DEBUG("set cmplt, oid:0x%x\n", p_randis->oid);
            if (p_randis->oid == OID_802_3_MULTICAST_LIST) {
                p_randis->is_set = true;
                rndish_receive(p_randis);
                if (rndish_connect_cb != NULL) {
                    rndish_connect_cb(p_randis->mac);
                }
            }
            else {
                uint8_t multicast_list[6] = { 0x01, 0x00, 0x5E, 0x00, 0x00, 0x01};
                DEBUG("set msg oid:0x%x\n", OID_802_3_MULTICAST_LIST);
                rndish_set_control_set_msg(OID_802_3_MULTICAST_LIST, multicast_list, 6);
            }
            break;
        }
        default:
            DEBUG("invalid reply_type: \n", p_randis->reply_type);
            break;
    }
    p_randis->reply_type = 0;
}

bool rndish_open(uint8_t rhport, uint8_t dev_addr, tusb_desc_interface_t const *itf_desc, uint16_t max_len)
{
    // Protocol 0xFF can be RNDIS device for windows XP
    (void)rhport;
    bool is_cdc_rndis = ( TUSB_CLASS_CDC    == itf_desc->bInterfaceClass &&
                        RNDISH_SUBCLASS_CDC == itf_desc->bInterfaceSubClass &&
                        RNDISH_PROTOCOL_CDC == itf_desc->bInterfaceProtocol);

    bool is_wireless_rndis = (TUSB_CLASS_WIRELESS_CONTROLLER    == itf_desc->bInterfaceClass  &&
                                        RNDISH_SUBCLASS_WIRELESS == itf_desc->bInterfaceSubClass &&
                                        RNDISH_PROTOCOL_WIRELESS == itf_desc->bInterfaceProtocol);

    TU_VERIFY(is_cdc_rndis || is_wireless_rndis);

    rndish_interface_t *p_randis = &rndish_data[0];
    p_randis->daddr              = dev_addr;
    p_randis->bInterfaceNumber   = itf_desc->bInterfaceNumber;
    p_randis->bInterfaceSubClass = itf_desc->bInterfaceSubClass;
    p_randis->bInterfaceProtocol = itf_desc->bInterfaceProtocol;

    DEBUG("rndish_open:rhport:%d, dev_addr:%d\n", rhport, dev_addr);
    DEBUG("desc_endpoint_num:%d\n", itf_desc->bNumEndpoints);

    uint8_t const * p_desc_end = ((uint8_t const*) itf_desc) + max_len;
    uint8_t const * p_desc = tu_desc_next(itf_desc);
    while (p_desc < p_desc_end) {
        if (tu_desc_type(p_desc) == TUSB_DESC_ENDPOINT) {
            tusb_desc_endpoint_t const * desc_ep = (tusb_desc_endpoint_t const *) p_desc;
            if (p_randis->ep_notif == 0x00) {
                TU_ASSERT( tuh_edpt_open(dev_addr, desc_ep) );
                p_randis->ep_notif = desc_ep->bEndpointAddress;
                DEBUG("config control endport:%d\n", p_randis->ep_notif);
            } else {
                TU_ASSERT(tuh_edpt_open(dev_addr, desc_ep));
                if (tu_edpt_dir(desc_ep->bEndpointAddress) == TUSB_DIR_IN) {
                    p_randis->ep_in     = desc_ep->bEndpointAddress;
                    p_randis->epin_size = tu_edpt_packet_size(desc_ep);
                    DEBUG("config in endport:%d, size:%d\n", p_randis->ep_in, p_randis->epin_size);
                } else {
                    p_randis->ep_out     = desc_ep->bEndpointAddress;
                    p_randis->epout_size = tu_edpt_packet_size(desc_ep);
                    DEBUG("config out endport:%d, size:%d\n", p_randis->ep_out, p_randis->epout_size);
                }
            }
        }
        p_desc = tu_desc_next(p_desc);
    }

    if (tuh_rndis_mount_cb) {
        tuh_rndis_mount_cb(dev_addr);
    }
    return true;
}

static void rndish_control_cmplt_cb(tuh_xfer_t* xfer)
{
    uintptr_t command = xfer->user_data;
    rndish_interface_t *p_randis = &rndish_data[0];
    if (xfer->result == XFER_RESULT_SUCCESS) {
        DEBUG("control cmplt:%x\n", (uint32_t)command);
        p_randis->reply_type = command;
    } else {
        DEBUG("control cmplt err:%d", (uint32_t)command);
    }
}

static void rndish_control_complete_cb(tuh_xfer_t* xfer)
{
    uintptr_t command = xfer->user_data;
    rndish_interface_t *p_randis = &rndish_data[0];
    if (xfer->result == XFER_RESULT_SUCCESS) {
        DEBUG("control complete:%x\n", (uint32_t)command);
        if (command == REMOTE_NDIS_QUERY_MSG ) {
            p_randis->oid = ((rndis_query_msg_t *)((void*)xfer->buffer))->Oid;
        } else if (command == REMOTE_NDIS_SET_MSG) {
            p_randis->oid = ((rndis_set_msg_t *)((void*)xfer->buffer))->Oid;
        }
        p_randis->msg_type = (uint32_t)command;
    } else {
         DEBUG("control complete err:%d", (uint32_t)command);
    }
}

static void rndish_set_control_init_msg(void)
{
    rndish_interface_t *p_randis = &rndish_data[0];

    rndis_initialize_msg_t *init_msg = (void*)usbh_get_enum_buf();
    uint16_t len = sizeof(rndis_initialize_msg_t);
    memset(init_msg, 0x00, len);

    init_msg->MessageType = REMOTE_NDIS_INITIALIZE_MSG;
    init_msg->MessageLength = len;
    init_msg->RequestId = p_randis->request_id++;
    init_msg->MajorVersion = 1;
    init_msg->MinorVersion = 0;
    init_msg->MaxTransferSize = 0x1000;

    rndish_control_xfer(REMOTE_NDIS_INITIALIZE_MSG, (uint8_t*)init_msg, len);

}

void rndish_set_control_query_msg(uint32_t oid, uint16_t query_len)
{
    rndish_interface_t *p_randis = &rndish_data[0];
    rndis_query_msg_t *query_msg = (void*)usbh_get_enum_buf();
    uint16_t len = query_len + sizeof(rndis_query_msg_t);
    memset(query_msg, 0x00, len);
    query_msg->MessageType = REMOTE_NDIS_QUERY_MSG;
    query_msg->MessageLength = len;
    query_msg->RequestId = p_randis->request_id++;
    query_msg->Oid = oid;
    query_msg->InformationBufferLength = query_len;
    query_msg->InformationBufferOffset = 20;
    query_msg->DeviceVcHandle = 0;

    rndish_control_xfer(REMOTE_NDIS_QUERY_MSG, (uint8_t*)query_msg, len);
}


void rndish_set_control_set_msg(uint32_t oid,  uint8_t *msg, uint16_t msg_len)
{
    rndish_interface_t *p_randis = &rndish_data[0];
    rndis_set_msg_t *set_msg = (void*)usbh_get_enum_buf();
    uint16_t len = msg_len + sizeof(rndis_query_msg_t);
    memset(set_msg, 0x00, len);
    set_msg->MessageType = REMOTE_NDIS_SET_MSG;
    set_msg->MessageLength = len;
    set_msg->RequestId = p_randis->request_id++;
    set_msg->Oid = oid;
    set_msg->InformationBufferLength = msg_len;
    set_msg->InformationBufferOffset = 20;
    set_msg->DeviceVcHandle = 0;

    memcpy(set_msg+1, msg, msg_len);

    rndish_control_xfer(REMOTE_NDIS_SET_MSG, (uint8_t*)set_msg, len);
}

void rndish_set_control_keepalive_msg(void)
{
    rndish_interface_t *p_randis = &rndish_data[0];
    rndis_keepalive_msg_t *keepalive_msg = (void*)usbh_get_enum_buf();
    uint16_t len = sizeof(rndis_keepalive_msg_t);
    memset(keepalive_msg, 0x00, len);
    keepalive_msg->MessageType = REMOTE_NDIS_KEEPALIVE_MSG;
    keepalive_msg->MessageLength = len;
    keepalive_msg->RequestId = p_randis->request_id++;
    rndish_control_xfer(REMOTE_NDIS_KEEPALIVE_MSG, (uint8_t*)keepalive_msg, len);
}


bool rndish_control_xfer(uint32_t command, uint8_t *data, uint16_t len)
{
    rndish_interface_t *p_randis = &rndish_data[0];
    tusb_control_request_t const request =
    {
        .bmRequestType_bit =
        {
            .recipient = TUSB_REQ_RCPT_INTERFACE,
            .type      = TUSB_REQ_TYPE_CLASS,
            .direction = TUSB_DIR_OUT
        },
        .bRequest = CDC_REQUEST_SEND_ENCAPSULATED_COMMAND,
        .wValue   = 0,
        .wIndex   = tu_htole16(p_randis->bInterfaceNumber),
        .wLength  = tu_htole16(len)
    };
    tuh_xfer_t xfer =
    {
    .daddr       = p_randis->daddr,
    .ep_addr     = 0,
    .setup       = &request,
    .buffer      = data,
    .complete_cb = rndish_control_complete_cb,
    .user_data   = command
    };

    if (!tuh_ready(p_randis->daddr) || usbh_edpt_busy(p_randis->daddr, p_randis->ep_notif)) {
        return false;
    };

    return tuh_control_xfer(&xfer);
}

bool rndish_get_control_cmplt(uintptr_t cmpltcmd)
{
    rndish_interface_t *p_randis = &rndish_data[0];
    tusb_control_request_t const request =
    {
        .bmRequestType_bit =
        {
            .recipient = TUSB_REQ_RCPT_INTERFACE,
            .type      = TUSB_REQ_TYPE_CLASS,
            .direction = TUSB_DIR_IN
        },
        .bRequest = CDC_REQUEST_GET_ENCAPSULATED_RESPONSE,
        .wValue   = 0,
        .wIndex   = tu_htole16(p_randis->bInterfaceNumber),
        .wLength  = tu_htole16(CFG_TUH_ENUMERATION_BUFSIZE)
    };

    uint8_t* enum_buf = usbh_get_enum_buf();
    tuh_xfer_t xfer =
    {
        .daddr       = p_randis->daddr,
        .ep_addr     = 0,
        .setup       = &request,
        .buffer      = enum_buf,
        .complete_cb = rndish_control_cmplt_cb,
        .user_data   = cmpltcmd
    };

    if (!tuh_ready(p_randis->daddr) && usbh_edpt_busy(p_randis->daddr, p_randis->ep_notif)) {
        return false;
    }
    return tuh_control_xfer(&xfer);
}


bool rndish_receive(rndish_interface_t *p_randis)
{
    uint8_t dev_addr = p_randis->daddr;

    if (!p_randis->is_set || !tuh_ready(p_randis->daddr) || usbh_edpt_busy(p_randis->daddr, p_randis->ep_in)) {
        return false;
    }

    TU_VERIFY( usbh_edpt_claim(dev_addr, p_randis->ep_in) );
    if (!usbh_edpt_xfer(dev_addr, p_randis->ep_in, received, sizeof(received)) ) {
        usbh_edpt_release(dev_addr, p_randis->ep_in);
        return false;
    }
    return true;
}


bool rndish_set_config(uint8_t dev_addr, uint8_t itf_num)
{
    (void)dev_addr;
    (void)itf_num;
    DEBUG("rndish config init msg.\n");
    rndish_set_control_init_msg();
    return true;
}

bool rndish_xfer_cb(uint8_t dev_addr, uint8_t ep_addr, xfer_result_t event, uint32_t xferred_bytes)
{
    rndish_interface_t *p_randis = &rndish_data[0];
    (void)dev_addr;
    (void)ep_addr;
    (void)event;
    (void)xferred_bytes;
    if (ep_addr == p_randis->ep_in)
    {
        DEBUG("receive lenth:%d\n", xferred_bytes);
        rndis_data_packet_t *r = (rndis_data_packet_t *) ((void*) received);
        if (xferred_bytes >= sizeof(rndis_data_packet_t)) {
            if ((r->MessageType == REMOTE_NDIS_PACKET_MSG) && (r->MessageLength <= xferred_bytes) &&
                ((r->DataOffset + offsetof(rndis_data_packet_t, DataOffset) + r->DataLength) <= xferred_bytes)) {
                uint8_t *data = &received[r->DataOffset + offsetof(rndis_data_packet_t, DataOffset)];
                p_randis->receive_data = data;
                p_randis->receive_length = (uint16_t)r->DataLength;
            }
        }

    } else if (ep_addr == p_randis->ep_out)
    {
        DEBUG("rndish_xfer_cb send length:%d\n", xferred_bytes);
    }

    return true;
}

bool rndish_send(uint8_t *data, uint16_t len)
{
    rndish_interface_t *p_randis = &rndish_data[0];
    rndis_data_packet_t *hdr = (rndis_data_packet_t *) ((void*) transmitted);

    memset(hdr, 0, sizeof(rndis_data_packet_t));

    len += sizeof(rndis_data_packet_t);
    hdr->MessageType = REMOTE_NDIS_PACKET_MSG;
    hdr->MessageLength = len;
    hdr->DataOffset = sizeof(rndis_data_packet_t) - offsetof(rndis_data_packet_t, DataOffset);
    hdr->DataLength = len - sizeof(rndis_data_packet_t);
    memcpy(hdr+1, data, hdr->DataLength);
    DEBUG("rndish_send:%d\n",len);

    if (!p_randis->is_set || !tuh_ready(p_randis->daddr)
        || usbh_edpt_busy(p_randis->daddr, p_randis->ep_out)) {
        return false;
    }

    uint8_t dev_addr = p_randis->daddr;
    TU_VERIFY( usbh_edpt_claim(dev_addr, p_randis->ep_out) );
    if (!usbh_edpt_xfer(dev_addr, p_randis->ep_out, transmitted, len) ) {
        usbh_edpt_release(dev_addr, p_randis->ep_out);
        return false;
    }
    return true;
}

void rndish_close(uint8_t dev_addr)
{
    rndish_interface_t *p_randis = &rndish_data[0];
    if (dev_addr != p_randis->daddr) {
        return;
    }

    usbh_edpt_release(dev_addr, p_randis->ep_notif);
    usbh_edpt_release(dev_addr, p_randis->ep_in);
    usbh_edpt_release(dev_addr, p_randis->ep_out);


    tu_memclr(rndish_data, sizeof(rndish_data));
    tu_memclr(received, sizeof(received));
    tu_memclr(transmitted, sizeof(transmitted));

    if (tuh_rndis_umount_cb != NULL) {
        tuh_rndis_umount_cb(dev_addr);
    }
}
