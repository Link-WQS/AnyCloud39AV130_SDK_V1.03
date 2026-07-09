#include "dbg.h"

#include "user_hal_gpio.h"
#include "user_app.h"
#include "sleep_api.h"
#ifdef CFG_RTOS
#include "rtos_al.h"
#endif /* CFG_RTOS */

#include "net_al.h"
#include "sys/socket.h"
#include "lwip/sys.h"
#include "hostif.h"

#define KM01A_POWER_ON_DELAY_MS (1000)

#define USER_TASK (MAX_TASK)
#define USER_TASK_STACK_SIZE (256)
#define USER_TASK_PRIORITY RTOS_TASK_PRIORITY(4)

static rtos_task_handle user_pwr_on_task_handle;

static int sock_tcp_wakeup = -1;
static uint8_t sock_keepalive_buffer[64] = "keepalive";
static uint8_t sock_target_host[64] = "192.168.1.100";
static uint16_t sock_target_port = 8080;
static uint32_t sock_keepalive_interval_ms = 3 * 1000;

extern uint8_t sleep_irq_evt_handled;
extern uint8_t sleep_irq_wakeup_soc;

extern TimerHandle_t hostif_timer;
extern void hostif_reset_handler(TimerHandle_t xTimer);

void sock_target_host_set(char *s)
{
    strcpy(sock_target_host, s);
}

char *sock_target_host_get(void)
{
    return sock_target_host;
}

void sock_target_port_set(uint16_t p)
{
    sock_target_port = p;
}

uint16_t sock_target_port_get(void)
{
    return sock_target_port;
}

void sock_keepalive_buffer_set(char *s)
{
    strcpy(sock_keepalive_buffer, s);
}

char *sock_keepalive_buffer_get(void)
{
    return sock_keepalive_buffer;
}

void sock_keepalive_interval_ms_set(uint32_t ms)
{
    sock_keepalive_interval_ms = ms;
}

uint32_t sock_keepalive_interval_ms_get(void)
{
    return sock_keepalive_interval_ms;
}

static void *memset(void *s, int c, size_t count)
{
    char *xs = s;
    while (count--)
        *xs++ = c;
    return s;
}

static int strncmp(const char *cs, const char *ct, size_t count)
{
    unsigned char c1, c2;

    while (count) {
        c1 = *cs++;
        c2 = *ct++;
        if (c1 != c2)
            return c1 < c2 ? -1 : 1;
        if (!c1)
            break;
        count--;
    }
    return 0;
}

static size_t strlen(const char *s)
{
    const char *sc;

    for (sc = s; *sc != '\0'; ++sc)
        /* nothing */;
    return sc - s;
}

static void user_sleepms(unsigned int ms)
{
#ifdef CFG_RTOS
    rtos_task_suspend(ms);
#endif
}

void user_soc_power_on(void)
{
    if (GPIO_WIFI_ST > 0) {
        gpio_init(GPIO_WIFI_ST);
        gpio_dir_out(GPIO_WIFI_ST);
        gpio_set(GPIO_WIFI_ST);
        user_sleepms(1);
    }

    gpio_mask(GPIO_KM01A_PWR_EN);
    gpio_dir_out(GPIO_KM01A_PWR_EN);
    gpio_set(GPIO_KM01A_PWR_EN);
    gpio_init(GPIO_KM01A_PWR_EN);
    //gpio_dir_out(GPIO_KM01A_PWR_EN);
    //gpio_set(GPIO_KM01A_PWR_EN);

    if (GPIO_WIFI_ST > 0) {
        user_sleepms(KM01A_POWER_ON_DELAY_MS);
        gpio_clr(GPIO_WIFI_ST);
    }

    if (GPIO_VBUS_DETECT > 0) {
        gpio_init(GPIO_VBUS_DETECT);
        gpio_dir_in(GPIO_VBUS_DETECT);
    }

    if (GPIO_LED1 > 0) {
        gpio_init(GPIO_LED1);
        gpio_dir_out(GPIO_LED1);
        gpio_clr(GPIO_LED1);
    }
    if (GPIO_LED2 > 0) {
        gpio_init(GPIO_LED2);
        gpio_dir_out(GPIO_LED2);
        gpio_clr(GPIO_LED2);
    }

    if (GPIO_WAKEUP_MCU_BUTTON > 0) {
        gpio_init(GPIO_WAKEUP_MCU_BUTTON);
        gpio_dir_out(GPIO_WAKEUP_MCU_BUTTON);
        gpio_set(GPIO_WAKEUP_MCU_BUTTON);
    }

    if (GPIO_ADC_CH1_MCU > 0) {
        gpio_init(GPIO_ADC_CH1_MCU);
        gpio_dir_in(GPIO_ADC_CH1_MCU);
    }

    if (GPIO_BAT_CHARGE_STA > 0) {
        gpio_init(GPIO_BAT_CHARGE_STA);
        gpio_dir_in(GPIO_BAT_CHARGE_STA);
    }

    if (GPIO_CHARGER_EN > 0) {
        gpio_init(GPIO_CHARGER_EN);
        gpio_dir_out(GPIO_CHARGER_EN);
        gpio_clr(GPIO_CHARGER_EN);
    }
}

void user_soc_power_off(void)
{
    dbg("soc power off\n");
    gpio_dir_out(GPIO_KM01A_PWR_EN);
    gpio_clr(GPIO_KM01A_PWR_EN);
}

static RTOS_TASK_FCT(user_pwr_on_routine)
{
    dbg("soc power on\n");
    user_soc_power_on();
    rtos_task_delete(NULL);
    user_pwr_on_task_handle = NULL;
}

void user_init(void)
{
    // Create the user task
    if (rtos_task_create(user_pwr_on_routine, "USER_PWR_ON", USER_TASK,
                         USER_TASK_STACK_SIZE, NULL, USER_TASK_PRIORITY, &user_pwr_on_task_handle)) {
        dbg("create user power on task failed!\n");
        return;
    }
}

int user_setup_wakeup_src(gpio_irq_handler_t handler)
{
#if 1
    if (GPIO_WAKEUP_MCU_BUTTON > 0) {
        if (PIN_TYPE(GPIO_WAKEUP_MCU_BUTTON) == PIN_TYPE_B) {
            gpiob_irq_init(PIN_IDX(GPIO_WAKEUP_MCU_BUTTON), GPIOIRQ_TYPE_EDGE_BOTH, handler);
            gpiob_force_pull_dn_enable(PIN_IDX(GPIO_WAKEUP_MCU_BUTTON));
            user_sleep_wakesrc_set(WAKESRC_GPIOB, 1, WAKEGPIO_ARG(WAKEGPIO_MUX_NUM_MAX, PIN_IDX(GPIO_WAKEUP_MCU_BUTTON)));
        } else {
            gpioa_irq_init(PIN_IDX(GPIO_WAKEUP_MCU_BUTTON), GPIOIRQ_TYPE_EDGE_BOTH, handler);
            gpioa_force_pull_dn_enable(PIN_IDX(GPIO_WAKEUP_MCU_BUTTON));
            user_sleep_wakesrc_set(WAKESRC_GPIOA, 1, WAKEGPIO_ARG(WAKEGPIO_MUX_NUM_MAX, PIN_IDX(GPIO_WAKEUP_MCU_BUTTON)));
        }
    }
#endif
    if (PIN_TYPE(GPIO_WAKEUP_WIFI) == PIN_TYPE_A) {
        gpioa_irq_init(PIN_IDX(GPIO_WAKEUP_WIFI), GPIOIRQ_TYPE_EDGE_RISE, handler);
        gpioa_force_pull_dn_enable(PIN_IDX(GPIO_WAKEUP_WIFI));
        user_sleep_wakesrc_set(WAKESRC_GPIOA, 1, WAKEGPIO_ARG(WAKEGPIO_MUX_0, PIN_IDX(GPIO_WAKEUP_WIFI)));
    } else {
        dbg("GPIO_WAKEUP_WIFI 0x%x not supported!\n", GPIO_WAKEUP_WIFI);
        return -1;
    }
    //gpio_dir_out(GPIO_WIFI_ST);
    //gpio_clr(GPIO_WIFI_ST);
    if (GPIO_ADC_CH1_MCU > 0) {
        gpio_dir_out(GPIO_ADC_CH1_MCU);
        gpio_set(GPIO_ADC_CH1_MCU);
    }
    if (GPIO_BAT_CHARGE_STA > 0) {
        gpio_dir_out(GPIO_BAT_CHARGE_STA);
        gpio_set(GPIO_BAT_CHARGE_STA);
    }

    return 0;
}

void user_clear_wakeup_src(void)
{
#if 1
    if (GPIO_WAKEUP_MCU_BUTTON > 0) {
        if (PIN_TYPE(GPIO_WAKEUP_MCU_BUTTON) == PIN_TYPE_B) {
            gpiob_force_pull_none_enable(PIN_IDX(GPIO_WAKEUP_MCU_BUTTON));
            gpiob_irq_deinit(PIN_IDX(GPIO_WAKEUP_MCU_BUTTON));
            user_sleep_wakesrc_set(WAKESRC_GPIOB, 0, WAKEGPIO_ARG(WAKEGPIO_MUX_NUM_MAX, PIN_IDX(GPIO_WAKEUP_MCU_BUTTON)));
        } else {
            user_sleep_wakesrc_set(WAKESRC_GPIOA, 0, WAKEGPIO_ARG(WAKEGPIO_MUX_NUM_MAX, PIN_IDX(GPIO_WAKEUP_MCU_BUTTON)));
            gpioa_force_pull_none_enable(PIN_IDX(GPIO_WAKEUP_MCU_BUTTON));
            gpioa_irq_deinit(PIN_IDX(GPIO_WAKEUP_MCU_BUTTON));
        }
    }
#endif
    if (PIN_TYPE(GPIO_WAKEUP_WIFI) == PIN_TYPE_A) {
        user_sleep_wakesrc_set(WAKESRC_GPIOA, 0, WAKEGPIO_ARG(WAKEGPIO_MUX_0, PIN_IDX(GPIO_WAKEUP_WIFI)));
        gpioa_force_pull_none_enable(PIN_IDX(GPIO_WAKEUP_WIFI));
        gpioa_irq_deinit(PIN_IDX(GPIO_WAKEUP_WIFI));
    } else {
        dbg("GPIO_WAKEUP_WIFI 0x%x not supported!\n", GPIO_WAKEUP_WIFI);
        return;
    }
}

#ifndef NO_SERVER_KEEPALIVE
static void sock_timer_handler(TimerHandle_t xTimer)
{
    dbg("sock_timer_handler\r\n");
    write(sock_tcp_wakeup, sock_keepalive_buffer, sizeof(sock_keepalive_buffer));
    return ;
}
#endif

/* This is a simple case about using special pkg to wakeup socket connect. */
static RTOS_TASK_FCT(tcp_wakeup_task)
{
    int     ret;
    int     opts;
    #if (NX_USING_NET_FD_SET)
    net_fd_set_ptr_t p_readfds;
    net_fd_set_ptr_t p_errofds;
    #else
    fd_set  readfds;
    fd_set  errofds;
    #endif
    struct sockaddr_in addr;
    struct timeval    timeo;
#ifndef NO_SERVER_KEEPALIVE
    TimerHandle_t sock_timer = NULL;
#endif
    int wakeup_soc = 0, wakeup_wifi = 0, reconnect = 0;

reconnect:
    reconnect = 0;
    wakeup_wifi = 0;
    wakeup_soc = 0;

    #if (NX_USING_NET_FD_SET)
    p_readfds = net_fd_alloc();
    p_errofds = net_fd_alloc();
    if (!p_readfds || !p_errofds) {
        dbg("net fd set alloc failed\n");
        goto exit;
    }
    #endif

    /* set up address to connect to */
    memset(&addr, 0, sizeof(addr));
    addr.sin_len = sizeof(addr);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(sock_target_port);
    addr.sin_addr.s_addr = inet_addr(sock_target_host);

    dbg("connect to %s port %d ...\n", sock_target_host, sock_target_port);

    /* connect */
    do {
        sock_tcp_wakeup = socket(AF_INET, SOCK_STREAM, 0);
        LWIP_ASSERT("sock >= 0", sock_tcp_wakeup >= 0);
        ret = connect(sock_tcp_wakeup, (struct sockaddr*)&addr, sizeof(addr));
        dbg("socket connect result [%d]\n", ret);
        if(ret != 0) {
            goto err_connect;
        }
    } while (ret != 0);

    opts = fcntl(sock_tcp_wakeup, F_GETFL, 0);
    if(opts < 0) {
        dbg("SET F_GETFL error\n");
        goto err_connect;
    }
    if(fcntl(sock_tcp_wakeup, F_SETFL, opts) < 0) {
        dbg("SET F_SETFL error\n");
        goto err_connect;
    }

#ifndef NO_SERVER_KEEPALIVE
    sock_timer = rtos_timer_create("tcp_wakeup_timer", sock_keepalive_interval_ms, pdTRUE, NULL, sock_timer_handler);
    ret = rtos_timer_start(sock_timer, 0, false);
    if (ret) {
        dbg("start timer failed: %d\n", ret);
        goto err_connect;
    }
#endif

    while(!sleep_irq_evt_handled) {
        #if (NX_USING_NET_FD_SET)
        net_fd_zero(p_readfds);
        net_fd_set(sock_tcp_wakeup, p_readfds);
        net_fd_zero(p_errofds);
        net_fd_set(sock_tcp_wakeup, p_errofds);
        #else
        FD_ZERO(&readfds);
        FD_SET(sock_tcp_wakeup, &readfds);
        FD_ZERO(&errofds);
        FD_SET(sock_tcp_wakeup, &errofds);
        #endif

        timeo.tv_sec = 1;
        timeo.tv_usec = 0;
        #if (NX_USING_NET_FD_SET)
        ret = select(sock_tcp_wakeup + 1, p_readfds, NULL, p_errofds, &timeo);
        #else
        //ret = select(sock + 1, &readfds, &writfds, &errofds, &timeo);
        ret = select(sock_tcp_wakeup + 1, &readfds, NULL, &errofds, &timeo);
        #endif
        //dbg("select ret: %d\n", ret);

        if (ret == 0) {
            continue;
        } else if (ret < 0) {
            dbg("Select err.. Exit %d\n", ret);
            break;
        } else {
            //
        }

        #if (NX_USING_NET_FD_SET)
        ret = net_fd_isset(sock_tcp_wakeup, p_errofds);
        #else
        ret = FD_ISSET(sock_tcp_wakeup, &errofds);
        #endif
        if (ret) {
            dbg("Connect err.. Exit\n");
            reconnect = 1;
            break;
        }
        #if (NX_USING_NET_FD_SET)
        ret = net_fd_isset(sock_tcp_wakeup, p_readfds);
        #else
        ret = FD_ISSET(sock_tcp_wakeup, &readfds);
        #endif
        if (ret) {
            dbg("FD_READ:\n");
            char buf[256];
            memset(buf, 0, sizeof(buf));
            ret = recv(sock_tcp_wakeup, buf, sizeof(buf), 0);
            if(ret <= 0){
               dbg("Recv error, ret=%d\n", ret);
               reconnect = 1;
               break;
            }
            dbg("GET data: %s\n", buf);

            if (strncmp(buf, "wakeup", strlen("wakeup")) == 0) {
                //if (sleep_level_get() != PM_LEVEL_ACTIVE) {
                    if (sleep_irq_evt_handled == 0) {
                        wakeup_soc = 1;
                        wakeup_wifi = 1;
                    } else if (sleep_irq_wakeup_soc == 0) {
                        //wakeup_soc = 1;
                    }
                //}
                break;
            }
        }
    }

    if (wakeup_wifi) {
        sleep_irq_evt_handled = 1;
        user_clear_wakeup_src();
    }

    if (wakeup_soc) {
        dbg("Set PM_LEVEL_ACTIVE\n");
        sleep_level_set(PM_LEVEL_ACTIVE);
        user_sleep_allow(0);

        if (wakeup_wifi == 0) {
            dbg("hostif poweroff\n");
            host_if_poweroff();
        }

        dbg("wakeup soc\n");
        sleep_irq_wakeup_soc = 1;

        gpio_init(GPIO_WAKEUP_AK);
        gpio_dir_out(GPIO_WAKEUP_AK);
        gpio_set(GPIO_WAKEUP_AK);

        gpio_mask(GPIO_KM01A_PWR_EN);
        gpio_dir_out(GPIO_KM01A_PWR_EN);
        gpio_set(GPIO_KM01A_PWR_EN);
        gpio_init(GPIO_KM01A_PWR_EN);

        if (GPIO_WIFI_ST > 0) {
            user_sleepms(100);
            gpio_dir_out(GPIO_WIFI_ST);
            gpio_set(GPIO_WIFI_ST);
        }

        user_sleepms(KM01A_POWER_ON_DELAY_MS);
        if (GPIO_WIFI_ST > 0) {
            gpio_clr(GPIO_WIFI_ST);
        }
        gpio_deinit(GPIO_WAKEUP_AK);

        dbg("hostif repower\n");
        host_if_repower();
    } else if (wakeup_wifi) {
        dbg("Set PM_LEVEL_ACTIVE\n");
        sleep_level_set(PM_LEVEL_ACTIVE);
        user_sleep_allow(0);
        dbg("repower hostif\n");
        host_if_repower();
    }

err_read:
#ifndef NO_SERVER_KEEPALIVE
    ret = rtos_timer_stop(sock_timer, 0);
    if (ret) {
        dbg("stop timer failed: %d\n", ret);
    }
    rtos_timer_delete(sock_timer, 0);
#endif
err_connect:
    closesocket(sock_tcp_wakeup);
#if (NX_USING_NET_FD_SET)
    net_fd_free(p_readfds);
    net_fd_free(p_errofds);
#endif
exit:
    if (reconnect && (sleep_irq_evt_handled == 0))
        goto reconnect;
    dbg("Exit tcp_wakeup_task.\n");
    rtos_task_delete(NULL);
}

int user_do_tcp_client_wakeup(void)
{
    if (rtos_task_create(tcp_wakeup_task, "tcp_wakeup", TCP_WAKEUP_TASK,
        1024, NULL, RTOS_TASK_PRIORITY(1), NULL)) {
        return 1;
    }
    return 0;
}

