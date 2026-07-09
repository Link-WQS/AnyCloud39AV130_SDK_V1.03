/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

/* This example current worked and tested with following controller
 * - Sony DualShock 4 [CUH-ZCT2x] VID = 0x054c, PID = 0x09cc
 */

#include "dbg.h"

#include "bsp/board_api.h"
#include "tusb_config.h"
#include "class/net/rndis_host.h"
#include "tusb.h"
#include "lwip/init.h"
#include "lwip/netif.h"
#include "lwip/etharp.h"
#include "lwip/dhcp.h"
#include "lwip/tcpip.h"
#include "lwip/udp.h"
#include "lwip/prot/dhcp.h"
#include "netif/ethernet.h"
#include "lwip/timeouts.h"

#define printf              dbg

struct netif rndis_netif;
uint8_t linkoutbuf[2048];
bool    netif_is_dhcp = false;
void    rndish_dhcp_start();
//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF PROTYPES
//--------------------------------------------------------------------+
void led_blinking_task(void);
/*------------- MAIN -------------*/
int tusb_main(void)
{
  board_init();
  //lwip_init();
  printf("TinyUSB Rndis Host Example\r\n");

  // init host stack on configured roothub port
  tuh_init(BOARD_TUH_RHPORT);

  while (1)
  {
    // tinyusb host task
    tuh_task();
    rndish_task();
    //led_blinking_task();
  }

  return 0;
}

void print_pbuf_info(struct pbuf *p)
{
  int i;
  struct eth_hdr *ethhdr = p->payload;
  printf("pbuf type:%d, src:", htons(ethhdr->type));
  for (i=0; i<6; i++) {
    printf(" %x", ethhdr->src.addr[i]);
  }
  printf(" desc:");
  for (i=0; i<6; i++) {
    printf(" %x", ethhdr->dest.addr[i]);
  }
  printf("\n");
}

void rndish_receive_cb(uint8_t *data, uint16_t len)
{
  struct pbuf *p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);
  if (p) {
    memcpy(p->payload, data, len);
    print_pbuf_info(p);
    ethernet_input(p, &rndis_netif);
    pbuf_free(p);
    //sys_check_timeouts();
  }
}

err_t rndish_netif_linkoutput(struct netif *netif, struct pbuf *p)
{
  (void)netif;
  uint16_t size = pbuf_copy_partial(p, linkoutbuf, p->tot_len, 0);
  printf("linkout pbuf size:%d\n", size);
  rndish_send(linkoutbuf, size);
  return ERR_OK;
}


err_t rndish_netif_init(struct netif *netif)
{
  LWIP_ASSERT("netif != NULL", (netif != NULL));
  netif->mtu = CFG_TUD_NET_MTU;
  netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP | NETIF_FLAG_UP;
  netif->state = NULL;
  netif->name[0] = 'E';
  netif->name[1] = 'X';
  netif->output = etharp_output;
  netif->linkoutput = rndish_netif_linkoutput;
  return ERR_OK;
}

void rndish_connect_cb(uint8_t mac[6])
{
  struct netif *netif = &rndis_netif;
  netif->hwaddr_len = 6;
  memcpy(netif->hwaddr, mac, 6);
  static ip_addr_t ipaddr, netmask, gateway;
  IP4_ADDR(&ipaddr, 0, 0, 0, 0);
  IP4_ADDR(&netmask, 0, 0, 0, 0);
  IP4_ADDR(&gateway, 0, 0, 0, 0);
  netif = netif_add(netif, &ipaddr, &netmask, &gateway, NULL, rndish_netif_init, ip_input);
  netif_set_default(netif);
  while (!netif_is_up(netif));
  printf("netif is up\n");
  dhcp_start(&rndis_netif);
}

void rndish_udp_send(ip_addr_t *remote_addr, uint16_t port, uint8_t *data,  uint16_t size);

//--------------------------------------------------------------------+
// Blinking Task
//--------------------------------------------------------------------+
/*void led_blinking_task(void)
{
  const uint32_t interval_ms = 1000;
  static uint32_t start_ms = 0;
  static bool led_state = false;
  // Blink every interval ms
  if (board_millis() - start_ms < interval_ms) return; // not enough time

  if (netif_is_dhcp) {
    uint8_t data[] = "Hello";
    rndish_udp_send(&rndis_netif.gw, 8888, data, 5);
  } else if (rndis_netif.ip_addr.addr != 0) {
    printf("IPv4 Address     : %s\r\n", ipaddr_ntoa(&rndis_netif.ip_addr));
    printf("IPv4 Subnet mask : %s\r\n", ipaddr_ntoa(&rndis_netif.netmask));
    printf("IPv4 Gateway     : %s\r\n\r\n", ipaddr_ntoa(&rndis_netif.gw));
    board_led_write(1);
    netif_is_dhcp = true;
  } else {
    start_ms += interval_ms;
    board_led_write(led_state);
    led_state = 1 - led_state; // toggle
  }

}
*/

static void rndish_udp_recv_cb(void *arg, struct udp_pcb *pcb, struct pbuf *p,
    const ip_addr_t *addr, u16_t port)
{
  (void)arg;
  (void)addr;
  (void)port;
  udp_send(pcb, p);
  pbuf_free(p);
}


void rndish_udp_send(ip_addr_t *remote_addr, uint16_t port, uint8_t *data,  uint16_t size)
{
  static struct udp_pcb *upcb = NULL;
  struct pbuf *p;

  if (upcb == NULL) {
    upcb = udp_new();
    err_t err = udp_connect(upcb, remote_addr, port);
    if (err == ERR_OK) {
      udp_recv(upcb, rndish_udp_recv_cb, NULL);
    }

  }

  p = pbuf_alloc(PBUF_TRANSPORT, size, PBUF_POOL);
  if (p != NULL) {
    pbuf_take(p, (char*)data, size);
    udp_send(upcb, p);
    pbuf_free(p);
  }
}

void tuh_rndis_umount_cb(uint8_t daddr)
{
  struct netif *netif = &rndis_netif;
  (void)daddr;
  printf("rndis umount addr:%d", daddr);
  if (netif_is_up(netif)) {
    netif_remove(netif);
  }
  netif_is_dhcp = false;
  memset(netif, 0x00, sizeof(struct netif));
}

#if 0
/* lwip has provision for using a mutex, when applicable */
sys_prot_t sys_arch_protect(void)
{
  return 0;
}
void sys_arch_unprotect(sys_prot_t pval)
{
  (void)pval;
}

/* lwip needs a millisecond time source, and the TinyUSB board support code has one available */
uint32_t sys_now(void)
{
  return board_millis();
}
#endif

/*
 * Entry point of this example application
 */
static RTOS_TASK_FCT(fhost_tusb_task)
{
    tusb_main();
    rtos_task_delete(NULL);
}

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
    if (rtos_task_create(fhost_tusb_task, "tusb task", APPLICATION_TASK,
                         512, NULL, RTOS_TASK_PRIORITY(1), NULL)) {
        return 1;
    }

    return 0;
}
