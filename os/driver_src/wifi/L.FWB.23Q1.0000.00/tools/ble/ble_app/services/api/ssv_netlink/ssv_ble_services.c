/*
 * Copyright (c) 2020 iComm-semi Ltd.
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
 * @file app_ctl.c
 * @brief application control functions.
 */


/*******************************************************************************
 *         Include Files
 ******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/timerfd.h>
#include <signal.h>

#include "ssv_gatts_api.h"
#include "ssv_gap_ble_api.h"

#ifdef BLE_USE_LL_ONLY
#include "ssv_ble_ll_api.h"
#endif

#include "ssv_ble_services.h"
#include "ssv_netlink.h"
#include "ssv_ctl_common.h"
#include "../ssv_nimble.h"
#include "cli.h"


/*******************************************************************************
 *         Global Variables
 ******************************************************************************/
int _g_ctl_sig_pipefd[2];
int _g_cli_sig_pipefd[2];

static int __g_ctl_sig_pipefd = -1;

static pthread_t _g_thread_ssv_loop = 0;
static pthread_t _g_thread_ble_cli = 0;

#ifdef BLE_USE_LL_ONLY
struct ctl_timer
{
    void (*timer_handler)(void);
};

static struct ctl_timer _g_ctl_timer;
static int __g_ctl_timerfd = -1;
#endif
/*******************************************************************************
 *         Local Funcations
 ******************************************************************************/
extern void cli_set_sig_pipefd(int fd);
extern void ssv_hal_gap_event_cb(ssv_gap_ble_cb_event_t evtId,char *data, unsigned int len);
extern void ssv_hal_gatts_event_cb(ssv_gatts_cb_event_t evtId,char *data, unsigned int len);

///////////////////////////////////////
static void ble_rx_data(struct netlink_msg *msg)
{
    struct nlattr *na;
    ST_NIMBLE_EVT *evt;
    int len;
    na = (struct nlattr *) GENLMSG_DATA(msg);
    // printf("nla_type : %d\n", na->nla_type);
    if (na->nla_type == E_SSV_CTL_ATTR_FROM_SSV_NIMBLE)
    {
        len = na->nla_len - NLA_HDRLEN - NIMBLE_EVT_HDR_LEN;
        evt = (ST_NIMBLE_EVT *)NLA_DATA(na);
        if (len != evt->datalen)
        {
            printf("evtid:%d, evtlen: %d != len:%d\n", evt->evtid, evt->datalen, len);
            return;
        }

        printf("evtid:0x%x datalen: %d\n", evt->evtid, evt->datalen);
        if(evt->evtid & E_SSV_GATTS_EVT_HOST_FLAG) //0x10000
        {
            evt->evtid &= ~E_SSV_GATTS_EVT_HOST_FLAG;
            ssv_hal_gatts_event_cb((ssv_gap_ble_cb_event_t)evt->evtid, (char *)evt->data,evt->datalen);
        }
        else
        {
            ssv_hal_gap_event_cb((ssv_gatts_cb_event_t)evt->evtid, (char *)evt->data,evt->datalen);
        }
        /*switch(evt->evtid)
        {
            case SSV_GAP_BLE_SCAN_RESULT_EVT:
                ssv_hal_gap_event_cb(SSV_GAP_BLE_SCAN_RESULT_EVT,(char *)evt->data,evt->datalen);
                break;
            case SSV_GATTS_CONNECT_EVT:
                ssv_hal_gatts_event_cb(SSV_GATTS_CONNECT_EVT,(char *)evt->data,evt->datalen);
                break;
            default:
                printf("\33[32m%s():%d \33[0m\r\n",__FUNCTION__,__LINE__);
                break;
        }*/
   }
#ifdef BLE_USE_LL_ONLY
    else if(na->nla_type == E_SSV_CTL_ATTR_FROM_SSV_BLE_LL)
    {
        uint8_t *data = (uint8_t *)NLA_DATA(na);

        len = na->nla_len-NLA_HDRLEN;
        // ssv_ble_ll_rx_pkt_cb(data, (uint32_t)len);
        ssv_ble_ll_rx_pkt_handler(data, (uint32_t)len);
    }
#endif
}

/*******************************************************************************
 *         Global Funcations
 ******************************************************************************/
void ctl_set_sig_pipefd(int fd)
{
    __g_ctl_sig_pipefd = fd;
}

#ifdef BLE_USE_LL_ONLY
#define MAX_EVENTS 3
#else
#define MAX_EVENTS 2
#endif

void *ssv_ble_loop(void *argv)
{
    struct netlink_msg msg;
    int epollfd;
    struct epoll_event ev, events[MAX_EVENTS];
    int sock = *((int *) argv);
    int nfds, i, len;

    epollfd = epoll_create1(0);
    if(epollfd == -1)
    {
        perror("epoll_create\n");
        return NULL;
    }
    
    ev.events = EPOLLIN | EPOLLRDHUP | EPOLLERR | EPOLLHUP | EPOLLPRI;
    ev.data.fd = sock;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, sock, &ev) == -1)
    {
        perror("epoll_ctl: sock");
        goto exit;
    }

    ev.events = EPOLLIN;
    ev.data.fd = __g_ctl_sig_pipefd;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, __g_ctl_sig_pipefd, &ev) == -1)
    {
        perror("epoll_ctl: ctl sig");
        goto exit;
    }
#ifdef BLE_USE_LL_ONLY
    __g_ctl_timerfd = timerfd_create(CLOCK_REALTIME, TFD_NONBLOCK | TFD_CLOEXEC);
    ev.events = EPOLLIN;
    ev.data.fd = __g_ctl_timerfd;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, __g_ctl_timerfd, &ev) == -1)
    {
        perror("epoll_ctl: ctl timer");
        goto exit;
    }
#endif
    while(1)
    {
        nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
        if (nfds == -1)
        {
            perror("epoll_wait");
            goto exit;
        }

        for(i = 0; i < nfds; i++)
        {
            if (events[i].data.fd == __g_ctl_sig_pipefd)
            {
                int sig = 0;
                read(__g_ctl_sig_pipefd, &sig, sizeof(sig));
                goto exit;
            }
            if (events[i].data.fd == sock)
            {
                if(events[i].events | EPOLLIN)
                {
                    len = recv(sock, &msg, sizeof(msg), 0);
                    if(len > 0)
                    {
                        if (msg.n.nlmsg_type == NLMSG_ERROR)
                        {
                            printf("Error, receive NACK\n");
                            goto exit;
                        }
                        if (!NLMSG_OK((&msg.n), len))
                        {
                             printf("Invalid reply message received via Netlink\n");
                             goto exit;
                        }

                        switch (msg.g.cmd) {
                            case E_SSV_CTL_CMD_FROM_SSV_NIMBLE:
#ifdef BLE_USE_LL_ONLY
                            case E_SSV_CTL_CMD_FROM_SSV_BLE_LL:
#endif
                                ble_rx_data(&msg);
                                break;
                            default:
                                printf("Unknown exce command %d\n", msg.g.cmd);
                                goto exit;
                        }
                    }
                    else
                    {
                        //printf("sock closed:%d\n", len);
                        break;
                    }
                }
            }
#ifdef BLE_USE_LL_ONLY
            if (events[i].data.fd == __g_ctl_timerfd)
            {
                int sig = 0;
                read(__g_ctl_timerfd, &sig, sizeof(sig));
                if (_g_ctl_timer.timer_handler)
                {
                    _g_ctl_timer.timer_handler();
                    _g_ctl_timer.timer_handler = NULL;
                }
                
                ssv_ble_services_set_timer(0, NULL);
                //struct timespec now;
                //clock_gettime(CLOCK_REALTIME, &now);
                //printf("expired, now:%ld.%ld\n", now.tv_sec, now.tv_nsec);                
            }
#endif
        }
    }

exit:
    close(epollfd);
#ifdef BLE_USE_LL_ONLY
    ssv_ble_services_set_timer(0, NULL);
    close(__g_ctl_timerfd);
    __g_ctl_timerfd = -1;
#endif
    return NULL;
}

int ssv_ble_services_start(void)
{
    int nl_fd = 0;

    nl_fd = ssv_netlink_start();

    if(nl_fd <= 0)
    {
        printf("please install driver \n");
        return -1;
    }
    pipe(_g_ctl_sig_pipefd);
    pipe(_g_cli_sig_pipefd);

    cli_set_sig_pipefd(_g_cli_sig_pipefd[0]);
    ctl_set_sig_pipefd(_g_ctl_sig_pipefd[0]);

    pthread_create(&_g_thread_ssv_loop, NULL, ssv_ble_loop, &nl_fd);
    pthread_create(&_g_thread_ble_cli, NULL, Cli_Init, NULL);
    
    pthread_join(_g_thread_ble_cli, NULL);
    pthread_join(_g_thread_ssv_loop, NULL);

    return 0;
}

void ssv_ble_services_stop(int signum)
{
    write(_g_cli_sig_pipefd[1], &signum, sizeof(signum));
    write(_g_ctl_sig_pipefd[1], &signum, sizeof(signum));
    close(_g_cli_sig_pipefd[1]);
    close(_g_ctl_sig_pipefd[1]);
    ssv_netlink_stop();
}



int ssv_ble_services_set_timer(int duration, void(*handler)(void))
{
#ifdef BLE_USE_LL_ONLY
    struct itimerspec new_time;
	
    if(__g_ctl_timerfd < 0)  return -1;
	
    if(duration > 0 && handler != NULL)
    {
        //struct timespec now;
        //clock_gettime(CLOCK_REALTIME, &now);
        //printf("start timer, now:%ld.%ld\n", now.tv_sec, now.tv_nsec);
        //new_time.it_value.tv_sec  =  duration / 1000;
        //new_time.it_value.tv_nsec = (duration % 1000) * 1000000;
        new_time.it_value.tv_sec  =  duration;  // seconds
        new_time.it_value.tv_nsec = 0;
        new_time.it_interval.tv_sec = 0;
        new_time.it_interval.tv_nsec = 0;
        timerfd_settime(__g_ctl_timerfd, 0, &new_time, NULL);

        _g_ctl_timer.timer_handler = handler;
        
        return 0;
    }
    else if (duration == 0 || handler == NULL)
    {
        _g_ctl_timer.timer_handler = NULL;
        new_time.it_value.tv_sec  =  0;
        new_time.it_value.tv_nsec = 0;
        new_time.it_interval.tv_sec = 0;
        new_time.it_interval.tv_nsec = 0;
        timerfd_settime(__g_ctl_timerfd, 0, &new_time, NULL);
        //printf("stop timer\n");
        return 0;
    }
#endif
    return -1;
}


