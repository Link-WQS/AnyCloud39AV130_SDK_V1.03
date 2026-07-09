/**
 * 
 */
#include <common.h>
#include <console.h>
#include <dm.h>
#include <usb.h>
#include <asm/io.h>
#include <linux/iopoll.h>
#include <power/regulator.h>
#include <clk.h>
#include <errno.h>
#include <dm/of_access.h>
#include "ak-musbhdrc.h"
#ifdef CONFIG_37_E_CODE
#include <asm/arch-ak37e/ak_cpu.h>
#include <asm/arch-ak37e/ak_types.h>
#include <asm/arch-ak37e/ak_l2buf.h>
#endif

#ifdef CONFIG_3918AV130_CODE
#include <asm/arch-ak3918av130/ak_cpu.h>
#include <asm/arch-ak3918av130/ak_types.h>
#include <asm/arch-ak3918av130/ak_l2buf.h>
#endif

// #define AK_USB_DEBUG
#ifdef AK_USB_DEBUG
#define ak_debug printf
#else
#define ak_debug(...)
#endif

static int bulk_nak_timeout_disabled = 0;

#ifdef CONFIG_37_E_CODE
static u8 ep_fifos[] = {USB_FIFO_EP0, USB_FIFO_EP1, USB_FIFO_EP2, USB_FIFO_EP3,
    USB_FIFO_EP4, USB_FIFO_EP5, USB_FIFO_EP6 };
__attribute__((unused))
static int ep_fifo_sizes[] = {512, 512, 512, 2048, 512, 512 };
#endif

#ifdef CONFIG_3918AV130_CODE
static u8 ep_fifos[] = {USB_FIFO_EP0, USB_FIFO_EP1, USB_FIFO_EP2, USB_FIFO_EP3,
    USB_FIFO_EP4, USB_FIFO_EP5, USB_FIFO_EP6};
__attribute__((unused))
static int ep_fifo_sizes[] = { 512, 512, 512, 512, 1024, 1024};
#endif
struct akotghc_epfifo_mapping akotg_epfifo_mapping;

/* Virtual Root Hub */
#include <usbroothubdes.h>

static struct usb_hub_descriptor roothub_desc;
static void usbhc_hub_descriptor (
	struct usb_hub_descriptor	*desc)
{
	u16		temp = 0;
	
	desc->bDescriptorType = 0x29;
	desc->bHubContrCurrent = 0;

	desc->bNbrPorts = 1;
	desc->bLength = 9;

	/* per-port power switching (gang of one!), or none */
	desc->bPwrOn2PwrGood = 0;

	/* no over current errors detection/handling */
	temp |= 0x0010; 

	desc->wHubCharacteristics = cpu_to_le16(temp);

	/* two bitmaps:  ports removable, and legacy PortPwrCtrlMask */
	desc->u.hs.DeviceRemovable[0] = 0 << 1;
	desc->u.hs.DeviceRemovable[1] = ~0;
}

static inline int get_ep_type(int pipe)
{
	int eptype = 0;

	switch(usb_pipetype(pipe)) {
		case PIPE_ISOCHRONOUS:
			eptype = 1;
			break;
		case PIPE_BULK:
			eptype = 2;
			break;
		case PIPE_INTERRUPT:
			eptype = 3;
			break;
	}
	return eptype;
}

static inline struct usbhc_ep *usb_epnum_to_usbhcep(
    struct _usbhc *priv, int epnum)
{
    struct usbhc_ep *ep = NULL;
    if(epnum == 0)
        ep = &priv->active_ep0;
    else
        ep = &priv->active_epx[epnum - 1];
    return ep;
}

static bool __map_epnum_to_epfifo(struct usb_device *dev,
	struct akotghc_epfifo_mapping *akotg_mapping,
	int epnum, int direction, int *epfifo)
{
	int i;
	struct epfifo_mapping *mapping;
	int epmaxpacketsize;
    struct epfifo_mapping *match_mapping = NULL;

	if (__is_epnum_mapped(akotg_mapping, epnum, direction))
		return false;
    if (direction) {
        epmaxpacketsize = dev->epmaxpacketout[epnum];
    } else {
        epmaxpacketsize = dev->epmaxpacketin[epnum];
    }
	//allocate 512(or 1024 or 2048) byte size of filo to epnum
	for (i = 0; i < MAX_EP_NUM; i++) {	
		mapping = &akotg_mapping->mapping[i];
        if (mapping->used)
            continue;

        if(mapping->fifosize < epmaxpacketsize)
            continue;

            /* find the best pipe */
        if(!match_mapping){
            match_mapping = mapping;
        } else{
            /* todo: check wMaxPacketSize, which is best */
            if((mapping->fifosize - epmaxpacketsize) <
                (match_mapping->fifosize - epmaxpacketsize))
                match_mapping = mapping;
        }
	}

    if (match_mapping) {
        match_mapping->used = 1;
        match_mapping->epnum = epnum;
        match_mapping->direction = direction;
        *epfifo = match_mapping->epfifo; //i + 1;	 EPFIFO1 ~ MAX_EP_NUM(EPFIFO4) is mapping */
        if (direction) { // TX
            hc_index_writew(match_mapping->epfifo, ((fls(match_mapping->fifosize) - 4) << 0), USB_REG_FIFOSIZE_CFG);
            hc_index_writel(match_mapping->epfifo, (match_mapping->fifostart >> 3) << 0, USB_REG_FIFOADDR_CFG);
        } else {
            hc_index_writew(match_mapping->epfifo, ((fls(match_mapping->fifosize) - 4) << 8), USB_REG_FIFOSIZE_CFG);
            hc_index_writel(match_mapping->epfifo, (match_mapping->fifostart >> 3) << 16, USB_REG_FIFOADDR_CFG);
        }
        return true;
	}
	return false;
}

static int epx_probe(struct _usbhc *priv,
    struct usb_device *dev, unsigned long pipe,
    void *buffer, int transfer_len,
    struct devrequest *req)
{
    int			epnum = usb_pipeendpoint(pipe);
    int			is_out = usb_pipeout(pipe);
    int			type = usb_pipetype(pipe);
    int			epfifo = 0;
    struct usbhc_ep *ep = NULL;
    int			retval;

	if(usb_pipeisoc(pipe) || usb_pipeint(pipe))
		return -ENOSPC;

    if(!__is_epnum_mapped(&akotg_epfifo_mapping, epnum, is_out)){
        if(!__map_epnum_to_epfifo(dev, &akotg_epfifo_mapping, epnum, is_out, &epfifo)){
            printf("ep%d can not mapping\n",epnum);
            return -ENOSPC;
        }
        ak_debug("###EPNum%d_%s mapping Host EPFIFO %d \n",epnum, is_out ? "OUT" : "IN", epfifo);
        if(epnum != 0){
            int eptype = get_ep_type(pipe);
            if(is_out){
                disable_epx_tx_interrupt(epfifo);
                set_epx_tx_type(epfifo, epnum, eptype);
                hc_index_writew(epfifo, dev->maxpacketsize, USB_REG_TXMAXP);

                if((type == PIPE_BULK) && (bulk_nak_timeout_disabled)){
                    /* 
					 * disable bulk nak timeout 
					 * for bulk transfer type,  a value of 1 or 0 disable nak timeout function.
					 */
					hc_index_writeb(epfifo, 1, USB_REG_TXINTERVAL);	
                }
                else
                    hc_index_writeb(epfifo, 16, USB_REG_TXINTERVAL);	/* Nak timeout or Tx Interval */

                clear_epx_tx_data_toggle(epfifo);
                set_epx_tx_mode(epfifo);
                flush_epx_tx_fifo(epfifo);
                enable_epx_tx_interrupt(epfifo);
            }
            else{
                disable_epx_rx_interrupt(epfifo);
                set_epx_rx_type(epfifo, epnum, eptype);
                hc_index_writew(epfifo, dev->maxpacketsize, USB_REG_RXMAXP);
                //if(usb_pipeisoc(pipe) || usb_pipeint(pipe))
                    //hc_index_writeb(epfifo, hep->desc.bInterval, USB_REG_RXINTERVAL);
                //else
                {
                    hc_index_writeb(epfifo, 0, USB_REG_RXINTERVAL);
                }
                set_epx_rx_mode(epfifo);
                clear_epx_rx_data_toggle(epfifo);
                flush_epx_rx_fifo(epfifo);
                enable_epx_rx_interrupt(epfifo);
            }
        }
        else{
            if(!epnum_to_epfifo(&akotg_epfifo_mapping, epnum, is_out, &epfifo))
                BUG();
        }

        ep = usb_epnum_to_usbhcep(priv, epnum);
        if(!ep->in_used){
            ep->udev = dev;
            ep->epnum = epnum;
            ep->epfifo = epfifo;
            ep->maxpacket = usb_maxpacket(dev, pipe);
            usb_settoggle(dev, epnum, is_out, 0);

            if(ep->maxpacket > H_MAXPACKET){
                /* iso packets up to 240 bytes could work... */
                printf("dev %d ep%d maxpacket %d\n",
                    dev->devnum, epnum, ep->maxpacket);			
                retval = -EINVAL;
                goto fail;
            }
            ep->in_used = true;
        }
    }

    return 0;

fail:
    return retval;
}

static int submit_rh_msg(struct udevice *udev, struct usb_device *dev,
				  unsigned long pipe, void *buffer,
				  int transfer_len, struct devrequest *req)
{
    struct _usbhc *priv = (struct _usbhc *)dev_get_priv(udev);
    int leni = transfer_len;
    int len = 0;
    int stat = 0;
    u16 typeReq;
    u16 wValue;
    u16 wLength;
    u8 reg8val;

    ak_debug("%s, req=%u (%#x), type=%u (%#x), value=%u, index=%u buflen=%d\n", __func__,
      req->request, req->request,
      req->requesttype, req->requesttype,
      le16_to_cpu(req->value), le16_to_cpu(req->index), req->length);

    typeReq = req->request | req->requesttype << 8;
    wValue  = cpu_to_le16 (req->value);
	wLength	= cpu_to_le16 (req->length);

    switch(typeReq){
        /**
         *  standard request
         */
        /* get descriptor */
        case USB_REQ_GET_DESCRIPTOR | DeviceRequest:
            switch((wValue & 0xff00) >> 8){
        		case (0x01): /* device descriptor */
        			len = min_t(unsigned int, leni,
                        min_t(unsigned int, sizeof(root_hub_dev_des), wLength));
        			memcpy(buffer, root_hub_dev_des, len);
                    ak_debug("standard: rh get device descriptor(len %d)\n", len);
        			break;
        		case (0x02): /* configuration descriptor */
                    ak_debug("standard: rh get configuration descriptor\n");
        			len = min_t(unsigned int, leni,
        		        min_t(unsigned int, sizeof(root_hub_config_des), wLength));
        			memcpy(buffer, root_hub_config_des, len);
        			break;
        		case (0x03): /* string descriptors */
                    ak_debug("standard: rh get string(%04x) descriptor\n", wValue);
        			if(wValue == 0x0300){
        				len = min_t(unsigned int, leni,
        		            min_t(unsigned int, sizeof(root_hub_str_index0), wLength));
        				memcpy(buffer, root_hub_str_index0, len);
        			}
        			if(wValue == 0x0301){
        				len = min_t(unsigned int, leni,
        			        min_t(unsigned int, sizeof(root_hub_str_index1), wLength));
        				memcpy(buffer, root_hub_str_index1, len);
        			}
        			break;
        		default:
                    printf("roothub get descriptor USB_ST_STALLED\n");
        			stat = USB_ST_STALLED;
        		}
            break;
        /* clear feature */
        case USB_REQ_CLEAR_FEATURE | DeviceOutRequest:
            printf("usb roothub unsupport clear feature request\n");
            stat = USB_ST_STALLED;
            break;
        /* get confoguration */
        case USB_REQ_GET_CONFIGURATION | DeviceRequest:
            ak_debug("standard: rh get configuration\n");
            memcpy(buffer, (unsigned char *)&priv->configuration, 1);
            len = 1;
            break;
        /* get status */
        case USB_REQ_GET_STATUS | DeviceRequest:
            printf("usb roothub unsupport get status request\n");
            stat = USB_ST_STALLED;
            break;
        /* set address */
        case USB_REQ_SET_ADDRESS | DeviceOutRequest:
            ak_debug("standard: rh set address\n");
            priv->rootdev = wValue;
            ak_debug("usb request rh set address %d\n", priv->rootdev);
            break;
        /* set configuration */
        case USB_REQ_SET_CONFIGURATION | DeviceOutRequest:
            ak_debug("standard: rh set configuration\n");
            memcpy((unsigned char *)&priv->configuration, &wValue, 2);
            ak_debug("usb request rh set configuration %d\n", priv->configuration);
            break;
        /* set descriptor */
        case USB_REQ_SET_DESCRIPTOR | DeviceOutRequest:
            printf("usb roothub unsupport set descriptor request\n");
            stat = USB_ST_STALLED;
            break;
        /* set feature */
        case USB_REQ_SET_FEATURE | DeviceOutRequest:
            printf("usb roothub unsupport set feature request\n");
            stat = USB_ST_STALLED;
            break;

        /** 
         * hub class request
         */
        /* clear hub feature */
        case USB_REQ_CLEAR_FEATURE | ((USB_DIR_OUT | USB_RT_HUB) << 8):
            switch(wValue){
                case C_HUB_LOCAL_POWER:
                case C_HUB_OVER_CURRENT:
                default:
                    printf("usb roothub unsupport "
                        "class requset clear hub feature fs %d\n",
                        wValue);
                    stat = USB_ST_STALLED;
                    break;
            }
            break;
        /* clear port feature */
        case USB_REQ_CLEAR_FEATURE | ((USB_DIR_OUT | USB_RT_PORT) << 8):
            switch(wValue){
                case USB_PORT_FEAT_ENABLE:
                case USB_PORT_FEAT_SUSPEND:
                case USB_PORT_FEAT_POWER:
                case 22: /* port indicator */
                    ak_debug("usb rh class requset clear port feature fs %d\n",
                        wValue);
                    priv->port_status &= ~(0x1 << wValue);
                    ak_debug("class request: rh clear port, port status %x\n", priv->port_status);
                    break;
                case USB_PORT_FEAT_C_CONNECTION:
                case USB_PORT_FEAT_C_ENABLE:
                case USB_PORT_FEAT_C_SUSPEND:
                case USB_PORT_FEAT_C_OVER_CURRENT:
                case USB_PORT_FEAT_C_RESET:
                    ak_debug("usb rh class requset clear port feature fs %d\n",
                        wValue);
                    priv->port_change &= ~(0x1 << wValue);
                    ak_debug("class request: rh clear port, change port status %x\n", priv->port_change);
                    break;
                default:
                    printf("usb roothub unsupport "
                        "class port requset clear feature fs %d\n",
                        wValue);
                    stat = USB_ST_STALLED;
                    break;
            }
            break;
        /* get hub descriptor */
        case USB_REQ_GET_DESCRIPTOR | ((USB_DIR_IN | USB_RT_HUB) << 8):
            ak_debug("class request: get hub descriptor\n");
            memcpy(buffer, &roothub_desc, roothub_desc.bLength);
            len = roothub_desc.bLength;
            break;
        /* get hub status */
        case USB_REQ_GET_STATUS | ((USB_DIR_IN | USB_RT_HUB) << 8):
            ak_debug("class request: get hub status\n");
            memset(buffer, 0, 4);
            len = 4;
            break;
        /* get port status */
        case USB_REQ_GET_STATUS | ((USB_DIR_IN | USB_RT_PORT) << 8):
            ak_debug("class request: get port status\n");
            memcpy(buffer, &priv->port_status, 2);
            memcpy((void *)((unsigned int)buffer + 2), &priv->port_change, 2);
            len = 4;
            break;
        /* set hub descriptor */
        case USB_REQ_SET_DESCRIPTOR | ((USB_DIR_OUT | USB_RT_HUB) << 8):
            printf("usb roothub unsupport "
                "class hub requset set descriptor\n");
            stat = USB_ST_STALLED;
            break;
        /* set hub feature */
        case USB_REQ_SET_FEATURE | ((USB_DIR_OUT | USB_RT_HUB) << 8):
            switch(wValue){
                case C_HUB_LOCAL_POWER:
                case C_HUB_OVER_CURRENT:
                default:
                    printf("usb roothub unsupport "
                        "class requset set hub feature fs %d\n",
                        wValue);
                    stat = USB_ST_STALLED;
                    break;
            }
            break;
        /* set port feature */
        case USB_REQ_SET_FEATURE | ((USB_DIR_OUT | USB_RT_PORT) << 8):
            switch(wValue){
                case USB_PORT_FEAT_RESET:
                    ak_debug("class request: set feature port reset\n");
                    /* reset device, 50 msec of reset/SE0 signaling, irqs blocked */
                    reg8val = hc_readb(USB_REG_POWER);
                    reg8val |= USB_POWER_RESET;
                    hc_writeb(reg8val, USB_REG_POWER);
                    usb_loop_delay(2000);
                    reg8val = hc_readb(USB_REG_POWER);
                    reg8val &= ~USB_POWER_RESET;
                    hc_writeb(reg8val, USB_REG_POWER);
                    priv->port_status |= 0x1 << wValue;
                    break;
                case USB_PORT_FEAT_SUSPEND:
                    ak_debug("class request: set feature port suspend\n");
                    priv->port_status |= 0x1 << wValue;
                    break;
                case USB_PORT_FEAT_POWER:
                    ak_debug("class request: set feature port power\n");
                    priv->port_status |= 0x1 << wValue;
                    break;
                case USB_PORT_FEAT_TEST:
                    ak_debug("class request: set feature port test\n");
                    priv->port_status |= 0x1 << wValue;
                    break;
                case 22: /* port indicator */
                    ak_debug("class request: set feature port indicator\n");
                    priv->port_status |= 0x1 << wValue;
                    break;
                case USB_PORT_FEAT_C_CONNECTION:
                    ak_debug("class request: set feature port c connect\n");
                    priv->port_change |= 0x1 << wValue;
                    break;
                case USB_PORT_FEAT_C_ENABLE:
                    ak_debug("class request: set feature port c enable\n");
                    priv->port_change |= 0x1 << wValue;
                    break;
                case USB_PORT_FEAT_C_SUSPEND:
                    ak_debug("class request: set feature port c suspend\n");
                    priv->port_change |= 0x1 << wValue;
                    break;
                case USB_PORT_FEAT_C_OVER_CURRENT:
                    ak_debug("class request: set feature port c over current\n");
                    priv->port_change |= 0x1 << wValue;
                    break;
                case USB_PORT_FEAT_C_RESET:
                    ak_debug("class request: set feature port c reset\n");
                    priv->port_change |= 0x1 << wValue;
                    break;
                default:
                    printf("usb roothub unsupport "
                        "class port requset set feature fs %d\n",
                        wValue);
                    stat = USB_ST_STALLED;
                    break;
            }
            break;
        /* clear tt buffer */
        /* reset tt */
        /* get tt status */
        /* stop tt */
        default:
            stat = USB_ST_STALLED;
            break;
    }

    len = min_t(int, len, leni);
    dev->act_len = len;
	dev->status = stat;
    return stat;
}

static void usb_trans_cancel(unsigned int epfifo)
{
    if(epfifo == 0){
    }
    else{
        hc_reg8(USB_REG_INDEX) = epfifo;
        if(hc_reg8(USB_REG_RXCSR1) & USB_RXCSR1_H_REQPKT)
            hc_reg8(USB_REG_RXCSR1) &= ~USB_RXCSR1_H_REQPKT;
        hc_reg8(USB_REG_TXCSR1) &= USB_TXCSR1_TXPKTRDY;
        flush_epx_tx_fifo(epfifo);
        flush_epx_rx_fifo(epfifo);
    }
}

/* This is a PIO-only HCD.  Queueing appends URBs to the endpoint's queue,
 * and may start I/O.  Endpoint queues are scanned during completion irq
 * handlers (one per packet: ACK, NAK, faults, etc) and urb cancellation.
 *
 * Using an external DMA engine to copy a packet at a time could work,
 * though setup/teardown costs may be too big to make it worthwhile.
 */

/* SETUP starts a new control request.  Devices are not allowed to
 * STALL or NAK these; they must cancel any pending control requests.
 */
static int setup_packet(struct _usbhc *priv,
    struct usb_device *dev, unsigned long pipe,
    void *buffer, int transfer_len,
    struct devrequest *req)
{
	int	i;
	u8	len, reg8val;
	unsigned char *buf = (unsigned char *)req;

	len = sizeof(struct devrequest);

    ak_debug("%s func address %d \n", __func__, usb_pipedevice(pipe));
    hc_writeb(usb_pipedevice(pipe), USB_REG_FADDR);
    hc_index_writeb(0, 16, USB_REG_TXINTERVAL);
	hc_index_writeb(0, 0, USB_REG_TXCSR1);
    // for(i = 0; i < len; i++)
    //     printf("%02x ", buf[i]);
    // printf("\n");
	for (i = 0; i < len; i++) {
		hc_writeb(buf[i], USB_FIFO_EP0);
	}
    hc_reg32(0x0330) = len;
	hc_index_writeb(0, USB_TXCSR1_FLUSHFIFO |USB_TXCSR1_FIFONOTEMPTY, USB_REG_TXCSR1);
    usb_loop_delay(5);

    /* wait finish */
    while(1){
        reg8val = hc_index_readb(0, USB_REG_TXCSR1);
        if(reg8val & USB_CSR01_H_RXSTALL){
            printf("setup rxstall(%x)\n", reg8val);
            goto err;
        }
        if(reg8val & USB_CSR01_H_ERROR){
            printf("setup err(%x)\n", reg8val);
            goto err;
        }
        if(reg8val & USB_CSR01_H_NAKTIMEOUT){
            printf("setup naktimeout(%x)\n", reg8val);
            goto err;
        }
        if(!(reg8val & USB_TXCSR1_FIFONOTEMPTY))
            break;
    }

    return req->length;
err:
    hc_index_writeb(0, 0, USB_REG_TXCSR1);
    return -1;
}

/* IN packets can be used with any type of endpoint. here we just
 * start the transfer, data from the peripheral may arrive later.
 * urb->iso_frame_desc is currently ignored here...
 */
static int in_packet(struct _usbhc *priv,
    struct usb_device *dev, unsigned long pipe,
    void *buffer, int transfer_len,
    struct devrequest *req)
{
    int			epnum = usb_pipeendpoint(pipe);
    int			epfifo = 0;
    int			is_out;
    struct usbhc_ep *ep;
    u8 reg8val;
    int fifo_count = 0;
    unsigned char *buf;
    int bufsize, rsize, i;

    ep = usb_epnum_to_usbhcep(priv, epnum);
    is_out = usb_pipeout(pipe);
    epnum_to_epfifo(&akotg_epfifo_mapping, ep->epnum, is_out, &epfifo);
    ep->error_count = 0;
    ep->length = 0;
    ep->nak_count = 0;
    buf = (unsigned char *)buffer;
    bufsize = transfer_len;
    rsize = 0;
	ak_debug("Packet: IN Packet (EP %d), Length=%d,ep->maxpacket=%d\n",
		ep->epnum, transfer_len, ep->maxpacket);

    hc_writeb(usb_pipedevice(pipe), USB_REG_FADDR);
    if(ep->epnum == 0){
        do{
            //printf("%s ep%d fifo count %d\n", __func__, ep->epfifo, hc_index_readw(0, USB_REG_COUNT0));
            /* start receive */
            hc_index_writeb(0, USB_TXCSR1_H_RXSTALL, USB_REG_TXCSR1);
            usb_loop_delay(5);
            /* wait receive one packet */
            while(1){
                reg8val = hc_index_readb(0, USB_REG_TXCSR1);
                if(reg8val & USB_CSR01_RXPKTRDY)
                    break;
            }
            /* get fifo count */
            fifo_count = hc_index_readw(0, USB_REG_COUNT0);
            /* check buffer size */
            if(bufsize < fifo_count){
                printf("ep0 overflow fifocount %d > bufsize %d\n",
                    fifo_count, bufsize);
                ep->error_count++;
            }
            /* receive */
            else if(!ep->error_count){
                if (fifo_count) {
                    for(i = 0; i < fifo_count; i++)
                        buf[i] = hc_readb(ep_fifos[epfifo]);
                    buf += fifo_count;
                    rsize += fifo_count;
                    bufsize -= fifo_count;
                }
            }
            flush_ep0_fifo();
        }while(fifo_count == ep->maxpacket && bufsize);
    }
    else{
        do{
            //printf("%s ep%d fifo count %d\n", __func__, ep->epfifo, hc_index_readw(0, USB_REG_COUNT0));
            /* start receive */
            hc_index_writeb(ep->epfifo, USB_RXCSR1_H_REQPKT, USB_REG_RXCSR1);
            usb_loop_delay(5);
            /* wait receive one packet */
            while(1){
                reg8val = hc_index_readb(epfifo, USB_REG_RXCSR1);
                if(reg8val & USB_RXCSR1_H_RXSTALL){
                    printf("%s ep%d rxstall(%x)\n", __func__, ep->epfifo, reg8val);
                    ep->error_count++;
                    break;
                }
                if(reg8val & USB_RXCSR1_H_NAKTIMEOUT){
                    printf("%s ep%d dataserror/nak timeout(%x)\n", __func__, ep->epfifo, reg8val);
                    ep->error_count++;
                    break;
                }
                if(reg8val & USB_RXCSR1_H_ERROR){
                    printf("%s ep%d error(%x)\n", __func__, ep->epfifo, reg8val);
                    ep->error_count++;
                    break;
                }
                if(reg8val & USB_RXCSR1_RXPKTRDY)
                    break;
            }
            /* get fifo count */
            fifo_count = hc_index_readw(epfifo, USB_REG_COUNT0);
            ak_debug("ep%d fifo datasize %d(buf %08x bufsize %d)\n", ep->epfifo, fifo_count, (unsigned int)buf, bufsize);
            /* check buffer size */
            if(bufsize < fifo_count){
                printf("ep%d overflow fifocount %d > bufsize %d\n",
                    ep->epfifo, fifo_count, bufsize);
                ep->error_count++;
            }
            /* receive */
            else if(!ep->error_count){
                for(i = 0; i < fifo_count; i++)
                    buf[i] = hc_readb(ep_fifos[epfifo]);
                buf += fifo_count;
                rsize += fifo_count;
                bufsize -= fifo_count;
                //printf("#");
            }
            flush_epx_rx_fifo(ep->epfifo);
        }while(fifo_count == ep->maxpacket && bufsize);
    }

    usb_dotoggle(dev, ep->epnum, 0);
    if(ep->error_count){
        usb_trans_cancel(ep->epfifo);
        rsize = -1;
    }
    return rsize;
}

static int out_packet(struct _usbhc *priv,
    struct usb_device *dev, unsigned long pipe,
    void *buffer, int transfer_len,
    struct devrequest *req)
{
    int         epnum = usb_pipeendpoint(pipe);
    int			is_out;
    int			epfifo;
    struct usbhc_ep *ep;
    unsigned char *buf;
    int lsize, wsize, i;
    u8 reg8val;

    ep = usb_epnum_to_usbhcep(priv, epnum);
    is_out = usb_pipeout(pipe);
    epnum_to_epfifo(&akotg_epfifo_mapping, ep->epnum, is_out, &epfifo);
    ep->error_count = 0;
    ep->length = 0;
    ep->nak_count = 0;
    buf = (unsigned char *)buffer;
    lsize = transfer_len;
    ak_debug("Packet: OUT Packet (EP %d), Length=%d\n",
        ep->epnum, transfer_len);

    hc_writeb(usb_pipedevice(pipe), USB_REG_FADDR);
    if(ep->epnum == 0){
        while(lsize){
            /* check one packet */
            wsize = min_t(u32, ep->maxpacket, lsize);
            hc_index_writeb(0, 0, USB_REG_TXCSR1);
            /* push data into buffer */
            for(i = 0; i < wsize; i ++)
                hc_writeb(buf[i], ep_fifos[ep->epfifo]);
            hc_reg32(0x0330) = wsize;
            /* start transmit */
            hc_index_writeb(0, 0x02, USB_REG_TXCSR1);
            usb_loop_delay(5);
            /* wait finish */
            while(1){
                reg8val = hc_index_readb(0, USB_REG_TXCSR1);
                if(!(reg8val & USB_TXCSR1_FIFONOTEMPTY))
                    break;
            }
            buf += wsize;
            lsize -= wsize;
            ak_debug("%s ep0 wsize %d(buf %08x bufsize %d)\n", __func__,
                wsize, (unsigned int)buf, lsize);
            flush_ep0_fifo();
        }
    }
    else{
        while(lsize){
            /* check one packet */
            wsize = min_t(u32, ep->maxpacket, lsize);
            hc_index_writeb(ep->epfifo, 0, USB_REG_TXCSR1);
            /* push data into buffer */
            for(i = 0; i < wsize; i ++){
                hc_writeb(buf[i], ep_fifos[ep->epfifo]);
            }
            /* start transmit */
            hc_index_writeb(ep->epfifo, USB_TXCSR1_TXPKTRDY, USB_REG_TXCSR1);
            usb_loop_delay(5);
            /* wait finish */
            while(1){
                reg8val = hc_index_readb(0, USB_REG_TXCSR1);
                if(reg8val & USB_TXCSR1_H_NAKTIMEOUT){
                    printf("%s ep%d naktimeout(%x)\n", __func__, ep->epfifo, reg8val);
                    break;
                }
                if(reg8val & USB_TXCSR1_H_ERROR){
                    printf("%s ep%d error(%x)\n", __func__, ep->epfifo, reg8val);
                    break;
                }
                if(!(reg8val & USB_TXCSR1_TXPKTRDY))
                    break;
            }
            buf += wsize;
            lsize -= wsize;
            ak_debug("%s ep%d wsize %d(buf %08x bufsize %d)\n", __func__,
                ep->epfifo, wsize, (unsigned int)buf, lsize);
            flush_epx_tx_fifo(ep->epfifo);
        }
    }
    hc_index_writeb(ep->epfifo, 0, USB_REG_TXCSR1);
    usb_dotoggle(dev, ep->epnum, 1);
    return transfer_len - lsize;
}

/* STATUS finishes control requests, often after IN or OUT data packets */
static int status_packet(struct _usbhc *priv,
    struct usb_device *dev, unsigned long pipe,
    void *buffer, int transfer_len,
    struct devrequest *req)
{
    int         epnum = usb_pipeendpoint(pipe);
    __attribute__((unused)) struct usbhc_ep *ep;
    int			is_in;

    ep = usb_epnum_to_usbhcep(priv, epnum);
    ak_debug("Packet: Status Packet (EP %d)\n", ep->epnum);

    is_in = usb_pipein(pipe);
    hc_writeb(usb_pipedevice(pipe), USB_REG_FADDR);
    hc_index_writeb(0, 16, USB_REG_TXINTERVAL);
    if(is_in){
        /* send a state packet when IN transaction is finished */
		hc_index_writeb(0, 0x42, USB_REG_TXCSR1);
    }
    else{
		/* request a  state packet when OUT transaction is finished */
		hc_index_writeb(0, 0x60, USB_REG_TXCSR1);
	}
    if(req->requesttype == 0x00 && req->request == 0x5){
        hc_writeb(req->value, USB_REG_FADDR);
    }

    return 0;
}

static int submit_dev_msg(struct udevice *udev, struct usb_device *dev,
				  unsigned long pipe, void *buffer,
				  int transfer_len, struct devrequest *req)
{
    struct _usbhc *priv = (struct _usbhc *)dev_get_priv(udev);
    int retval = 0;
    int act_len = 0;
    unsigned char reg8val;

    ak_debug("%s, req=%u (%#x), type=%u (%#x), value=%u, index=%u buflen=%d\n", __func__,
      req->request, req->request,
      req->requesttype, req->requesttype,
      le16_to_cpu(req->value), le16_to_cpu(req->index), req->length);

    /* setup phase */
    retval = setup_packet(priv, dev, pipe, buffer, transfer_len, req);
    if(retval < 0){
        printf("err, ep%ld setup packet failed(%d)\n",
            usb_pipeendpoint(pipe), retval);
        goto err;
    }
    switch (req->request) {
        case USB_REQ_SET_ADDRESS:
            usb_settoggle(dev, 0, 0, 1);
            retval = in_packet(priv, dev, pipe, NULL, 0, req);
            if(retval < 0){
                printf("err, ep%ld in packet failed(%d)\n",
                    usb_pipeendpoint(pipe), retval);
                goto err;
            }
            retval = req->length;
            break;
        default:
            break;
    }
    /* data phase */
    if(req->length){
        if(usb_pipeout(pipe)){
            usb_settoggle(dev, 0, 1, 1);
            retval = out_packet(priv, dev, pipe, buffer, transfer_len, req);
            if(retval < 0){
                printf("err, ep%ld out packet failed(%d)\n",
                    usb_pipeendpoint(pipe), retval);
                goto err;
            }
        }
        else{
            usb_settoggle(dev, 0, 0, 1);
            retval = in_packet(priv, dev, pipe, buffer, transfer_len, req);
            if(retval < 0){
                printf("err, ep%ld in packet failed(%d)\n",
                    usb_pipeendpoint(pipe), retval);
                goto err;
            }
        }
    }
    act_len = retval;

    /* status phase */
    retval = status_packet(priv, dev, pipe, buffer, transfer_len, req);
    if(retval < 0){
        printf("err, ep%ld status packet failed(%d)\n",
            usb_pipeendpoint(pipe), retval);
        goto err;
    }
    return act_len;
err:
    reg8val = hc_index_readb(0, USB_REG_TXCSR1);
    reg8val &= ~(USB_CSR01_RXPKTRDY);
    reg8val &= ~(USB_CSR01_H_SETUPPKT);
    hc_index_writeb(0, reg8val, USB_REG_TXCSR1);
    reg8val = hc_index_readb(0, USB_REG_TXCSR2);
    reg8val |= 1 << 0;
    hc_index_writeb(0, reg8val, USB_REG_TXCSR2);
    return -1;
}

static int ak_submit_control_msg(struct udevice *udev,
				       struct usb_device *dev,
				       unsigned long pipe, void *buffer,
				       int length, struct devrequest *setup)
{
    struct _usbhc *priv = (struct _usbhc *)dev_get_priv(udev);
    //u16 dev_address = setup->request == USB_REQ_SET_ADDRESS ?
					//0 : dev->devnum;
    int retval, i, time = 3;

	//ak_debug("%s: dev='%s', udev=%p, udev->dev='%s', portnr=%d\n", __func__,
	      //udev->name, dev, dev->dev->name, dev->portnr);

	if(usb_pipetype(pipe) != PIPE_CONTROL) {
		ak_debug("non-control pipe (type=%lu)", usb_pipetype(pipe));
		return -1;
	}

    /* config epx */
    if(epx_probe(priv, dev, pipe, buffer, length, setup) < 0){
        printf("%s %d, ep%ld config failed\n", __func__, __LINE__,
            usb_pipeendpoint(pipe));
        return -1;
    }

    /* control message for roothub dev */
    if(usb_pipedevice(pipe) == priv->rootdev){
        //if (!priv->rootdev)
			//dev->speed = USB_SPEED_HIGH;
        return submit_rh_msg(udev, dev, pipe, buffer, length, setup);
    }
    for(i = 0; i < time; i++){
        retval = submit_dev_msg(udev, dev, pipe, buffer, length, setup);
        if(retval >= 0)
            break;
        usb_loop_delay(5);
    }
    if(retval < 0)
        return -1;

    dev->act_len = retval;
    dev->status = 0;
    return 0;
}

static int ak_submit_bulk_msg(struct udevice *udev,
				    struct usb_device *dev, unsigned long pipe,
				    void *buffer, int length)
{
    struct _usbhc *priv = (struct _usbhc *)dev_get_priv(udev);
    int         epnum = usb_pipeendpoint(pipe);
    int	        is_out = usb_pipeout(pipe);
    int         retval = 0;
    struct usbhc_ep *ep;
    int i, time = 1;

    /* config epx */
    if(epx_probe(priv, dev, pipe, buffer, length, NULL) < 0){
        printf("%s %d, ep%ld config failed\n", __func__, __LINE__,
            usb_pipeendpoint(pipe));
        return -1;
    }

    ep = usb_epnum_to_usbhcep(priv, epnum);
    if(is_out){
        retval = out_packet(priv, dev, pipe, buffer, length, NULL);
        if(retval < 0){
            printf("err, ep%ld out packet failed(%d)\n",
                usb_pipeendpoint(pipe), retval);
            return retval;
        }
        usb_dotoggle(dev, ep->epnum, 1);
    }
    else{
        for(i = 0; i < time; i++){
            retval = in_packet(priv, dev, pipe, buffer, length, NULL);
            if(retval >= 0)
                break;
            usb_loop_delay(5);
        }
        if(retval < 0){
            printf("err, ep%ld in packet failed(%d)\n",
                usb_pipeendpoint(pipe), retval);
            return retval;
        }
        usb_dotoggle(dev, ep->epnum, 0);
    }

    dev->act_len = retval;
    dev->status = 0;
	return 0;
}

static inline void init_epfifo_mapping(struct akotghc_epfifo_mapping *akotg_mapping)
{
	int i;
	struct epfifo_mapping *mapping;
	int fifostart = 0;

	for (i = 0; i < MAX_EP_NUM; i++) {
		mapping = &akotg_mapping->mapping[i];
		mapping->epfifo = i + 1;	/* EPFIFO1 ~ MAX_EP_NUM(EPFIFO4) is mapping */
		mapping->used = 0;
		mapping->epnum = 0;
		mapping->direction = 0;
		mapping->fifosize = ep_fifo_sizes[i];
		mapping->fifostart = fifostart;
        fifostart += ep_fifo_sizes[i];
	}
}

static int usb_ofdata_to_platdata(struct udevice *dev)
{
	struct _usbhc *priv = (struct _usbhc *)dev_get_priv(dev);
	fdt_addr_t addr;
	int ret;

    /* endpoint fifo init */
    init_epfifo_mapping(&akotg_epfifo_mapping);

    /* init priv structure */
    memset(priv, 0x0, sizeof(struct _usbhc));
    usbhc_hub_descriptor(&roothub_desc);

	addr = dev_read_addr(dev);
	if(addr == FDT_ADDR_T_NONE){
		ak_debug("addr(%08x) == FDT_ADDR_T_NONE(%08x)\n",
			(unsigned int)addr, (unsigned int)FDT_ADDR_T_NONE);
		return -EINVAL;
	}
	/* prepare */
	priv->hcdc_baseaddr = addr;
	priv->sysctrl_baseaddr = SYSTEM_CTRL_BASE_ADDR;
	priv->udev = dev;
	ak_debug("ak usb controller base address %08lx \n", priv->hcdc_baseaddr);
	ak_debug("ak system control base address %08lx \n", priv->sysctrl_baseaddr);

	/* create hc clk object */
	ret = clk_get_by_index(dev, 0, &priv->clk);
	if(ret){
		dev_err(dev, "clk get by index failed\n");
		return ret;
	}
	ak_debug("ak usb controller clk index id is %ld\n", priv->clk.id);

	return 0;
}

static int port_power(struct _usbhc *priv, int port, unsigned char power)
{
    /* hub is inactive unless the port is powered */
    if(power){
        ak_debug("%s_%d, port_status=%x\n", __func__, __LINE__,priv->port_status);
        if(priv->port_status & USB_PORT_STAT_POWER)
            return 0;
        priv->port_status |= 0x1 << USB_PORT_FEAT_POWER;
        ak_debug("%s_%d, change port_status=%x\n", __func__, __LINE__,priv->port_status);
    }
    else{
        ak_debug("%s_%d, port_status=%x\n", __func__, __LINE__,priv->port_status);
        priv->port_status = 0;
        ak_debug("%s_%d, change port_status=%x\n", __func__, __LINE__,priv->port_status);
    }

    return 0;
}

static void usb_hwinit_control(struct _usbhc *priv)
{
	unsigned long flags;
    unsigned long con;
	spin_lock_irqsave(&priv->lock, flags);

	port_power(priv, 0, true);
	set_usb_as_host();

	/* release reset usb phy */
    con = __raw_readl(USB_OP_MOD_REG);
    con |= (0x7 << 0);
    con &= ~(1 << 0);
    __raw_writel(con, USB_OP_MOD_REG);

	clear_all_interrupts();
	reset_endpoints();
	hc_writeb(0x0, USB_REG_FADDR);
	
	//usb_power_up(priv);
	
	//hc_writeb(USB_POWER_ENSUSPEND|USB_HOSG_HIGH_SPEED, USB_REG_POWER);
    hc_reg8(USB_REG_POWER) = 0x20 | (0x1 << 7);
	hc_writeb(0xF7, USB_REG_INTRUSBE);

    /* close testmode */
    hc_reg8(USB_REG_TESEMODE) = 0x0;

    /* set the endpoint 0 nak timeout max */
    hc_index_writeb(0, 16, USB_REG_NAKLIMIT0);

	spin_unlock_irqrestore(&priv->lock, flags);
}

static int _usbhc_reset(struct _usbhc *priv)
{
    return 0;
}

static int _usbhc_start(struct _usbhc *priv)
{
    /* enable clk-gate and reset usb controller */
    clk_enable(&priv->clk);

    usb_hwinit_control(priv);

    /* start host session*/ // is end session ?
	//hc_writeb(USB_DEVCTL_SESSION, USB_REG_DEVCTL);
    hc_reg8(USB_REG_DEVCTL) |= 0x1;

    return 0;
}

static int _usbhc_stop(struct _usbhc *priv)
{
	unsigned long flags;

	/* disable usb host controller gate clk */
	clk_disable(&priv->clk);

	/* reset usb phy */
	usb_reset_phy(priv);

	spin_lock_irqsave(&priv->lock, flags);
	port_power(priv, 0, false);
	spin_unlock_irqrestore(&priv->lock, flags);

    return 0;
}

static int check_usb_device_connecting_on_bus(struct _usbhc *priv)
{
    int timeout = 5000;
    char reg8val;

    do{
        reg8val = hc_readb(USB_REG_INTRUSB);
        if(reg8val & USB_INTR_CONNECT)
            break;
        if(timeout-- < 0){
            printf("%s timeout.\n", __func__);
            return -1;
        }
        usb_loop_delay(5);
    }while(1);


    ak_debug("####connent happen\n");
    priv->port_status |= 1 << USB_PORT_FEAT_ENABLE;
    priv->port_status |= 1 << USB_PORT_FEAT_CONNECTION;
    priv->port_change |= 1 << USB_PORT_FEAT_C_CONNECTION;

    /* set default device address */
    usb_set_address(0);

    /* reset device, 50 msec of reset/SE0 signaling, irqs blocked */
    reg8val = hc_readb(USB_REG_POWER);
    reg8val |= USB_POWER_RESET;
    hc_writeb(reg8val, USB_REG_POWER);
    usb_loop_delay(2000);
    reg8val = hc_readb(USB_REG_POWER);
    reg8val &= ~USB_POWER_RESET;
    hc_writeb(reg8val, USB_REG_POWER);

    /* check speed */
    reg8val = hc_readb(USB_REG_POWER);
    if((reg8val & USB_POWER_HSMODE) && (reg8val & USB_HOSG_HIGH_SPEED)){
        /* high speed */
        printf("high speed!!!\n");
        priv->port_status &= ~(0x1 << USB_PORT_FEAT_LOWSPEED);
        priv->port_status &= ~(0x1 << USB_PORT_FEAT_HIGHSPEED);
        priv->port_status |= 0x1 << USB_PORT_FEAT_HIGHSPEED;
    }
    else{
        /* full speed */
        printf("full speed!!!\n");
        priv->port_status &= ~(0x1 << USB_PORT_FEAT_LOWSPEED);
        priv->port_status &= ~(0x1 << USB_PORT_FEAT_HIGHSPEED);
    }

    ak_debug("%s %d, port status %x port change status %x\n",
        __func__, __LINE__, priv->port_status, priv->port_change);
    ak_debug("USB device has detected ok...\n");

    /* reset device, 50 msec of reset/SE0 signaling, irqs blocked */
    reg8val = hc_readb(USB_REG_POWER);
    reg8val |= USB_POWER_RESET;
    hc_writeb(reg8val, USB_REG_POWER);
    usb_loop_delay(2000);
    reg8val = hc_readb(USB_REG_POWER);
    reg8val &= ~USB_POWER_RESET;
    hc_writeb(reg8val, USB_REG_POWER);
    return 0;
}

static int usb_probe(struct udevice *dev)
{
	ak_debug("ak usb controller probe ...\n");
	struct _usbhc *priv = (struct _usbhc *)dev_get_priv(dev);
	struct usb_bus_priv *bus_priv = dev_get_uclass_priv(dev);

	/**
	 * @desc_before_addr:
	 * true if we can read a device descriptor before it has been
	 * assigned an address. For XHCI this is not possible so this will
	 * be false.
	 */
	bus_priv->desc_before_addr = true;

    /* init */
    priv->rootdev = 0;
    priv->configuration = 0;
    priv->dev_address = 0;

	/* lock init */
	spin_lock_init(&priv->lock);

	/* endpoint object init */
	/* todo: */

	/* disable usb controller */
	_usbhc_stop(priv);
    usb_loop_delay(5);

    /* reset usb controller */
    _usbhc_reset(priv);

	/* enable usb controller as host mode */
	_usbhc_start(priv);
	usb_loop_delay(5);

	/* check usb device connecting */
	check_usb_device_connecting_on_bus(priv);

	return 0;
}

static int usb_remove(struct udevice *dev)
{
	ak_debug("ak usb controller remove ...\n");
    struct _usbhc *priv = (struct _usbhc *)dev_get_priv(dev);
    
    /* disable usb controller */
	_usbhc_stop(priv);

    return 0;
}

static struct dm_usb_ops usb_ops = {
	.control =  ak_submit_control_msg,
	.bulk =     ak_submit_bulk_msg,
};

static const struct udevice_id usb_ids[] = {
#ifdef CONFIG_37_E_CODE
	{ .compatible = "anyka,ak37e-usb" }
#endif
#ifdef CONFIG_3918AV130_CODE
	{ .compatible = "anyka,ak3918av130-usb" },
#endif
	{},
};

U_BOOT_DRIVER(usb_anyka) = {
	.name =                 "ak-musbhdrc",
	.id =                   UCLASS_USB,
	.of_match =             usb_ids,
	.ofdata_to_platdata =   usb_ofdata_to_platdata,
	.probe =                usb_probe,
	.remove =               usb_remove,
	.ops =                  &usb_ops,
	.priv_auto_alloc_size = sizeof(struct _usbhc),
	/* Allocate driver private data on a DMA boundary */
	.flags =                DM_FLAG_ALLOC_PRIV_DMA,
};
