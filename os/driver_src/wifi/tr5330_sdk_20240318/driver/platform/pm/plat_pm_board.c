/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: plat pm board source file
 * Author: CompanyName
 */

#if defined(_PRE_OS_VERSION) && defined(_PRE_OS_VERSION_LINUX) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#ifdef _PRE_CONFIG_USE_DTS
#include <linux/of.h>
#include <linux/of_gpio.h>
#endif
#include <linux/clk.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/fs.h>
#if defined(CONFIG_HCC_SUPPORT_UART) && defined(CONFIG_PLAT_UART_IO_CFG)
#include <linux/of_device.h>
#include <linux/pinctrl/consumer.h>
#endif
#endif
#include "plat_pm_board_tr5330.h"
#include "plat_pm_board.h"
#if defined(_PRE_OS_VERSION) && defined(_PRE_OS_VERSION_LINUX) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include "plat_debug.h"
#include "oal_ext_if.h"
#endif
#include "plat_firmware.h"
#include "hcc_bus.h"
#if defined(_PRE_OS_VERSION) && defined(_PRE_OS_VERSION_LINUX) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include "plat_pm.h"
#include "oam_ext_if.h"
#endif

#include "ini.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

pm_board_info g_pm_board_info;
EXPORT_SYMBOL(g_pm_board_info);

static osal_s32 g_pm_board_probe_ret = 0;
#if defined(_PRE_OS_VERSION) && defined(_PRE_OS_VERSION_LINUX) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
static struct completion g_pm_board_driver_complete;
static irqreturn_t pm_board_wakeup_gpio_irq(td_s32 irq, td_void *dev_id);
#endif

pm_board_info *get_pm_board_info(td_void)
{
    return &g_pm_board_info;
}

td_s32 pm_board_is_gpio_idx_valid(td_s32 gpio_idx)
{
    return (gpio_idx >= 0);
}

td_void pm_board_power_gpio_deinit(td_void)
{
#if defined(_PRE_OS_VERSION) && defined(_PRE_OS_VERSION_LINUX) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    if (g_pm_board_info.power_gpio_support && pm_board_is_gpio_idx_valid(g_pm_board_info.power_gpio)) {
        gpio_direction_output(g_pm_board_info.power_gpio, GPIO_LOWLEVEL);
        gpio_free(g_pm_board_info.power_gpio);
        g_pm_board_info.power_gpio_support = OSAL_FALSE;
        g_pm_board_info.power_gpio = GPIO_IDX_INVALID;
    }
#endif
}

td_void pm_board_wakeup_gpio_deinit(td_void)
{
#if defined(_PRE_OS_VERSION) && defined(_PRE_OS_VERSION_LINUX) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    if (g_pm_board_info.wkup_gpio_support && pm_board_is_gpio_idx_valid(g_pm_board_info.wkup_gpio_idx)) {
        free_irq(g_pm_board_info.wkup_gpio_irq, &g_pm_board_info);
        gpio_free(g_pm_board_info.wkup_gpio_idx);
        g_pm_board_info.wkup_gpio_support = OSAL_FALSE;
        g_pm_board_info.wkup_gpio_idx = GPIO_IDX_INVALID;
        g_pm_board_info.wkup_gpio_irq = GPIO_IRQ_INVALID;
    }
#endif
}

#if defined(_PRE_OS_VERSION) && defined(_PRE_OS_VERSION_LINUX) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
static irqreturn_t pm_board_wakeup_gpio_irq(td_s32 irq, td_void *dev_id)
{
    osal_s32 ret;
    (void)(irq);

    oal_print_info("wakeup_gpio_irq! \n");
    if (dev_id != (td_void *)(&g_pm_board_info)) {
        return IRQ_NONE;
    }

    if (g_pm_board_info.wkup_gpio_cb != OSAL_NULL) {
        ret = g_pm_board_info.wkup_gpio_cb();
        if (ret != OSAL_SUCCESS) {
            oal_print_err("wkup_gpio_cb: fail, ret=[%d]. \r\n", ret);
        }
    }

    return IRQ_HANDLED;
}
#endif

td_void pm_board_wkup_gpio_register(pm_board_cb cb)
{
    g_pm_board_info.wkup_gpio_cb = cb;
}

td_s32 pm_board_wakeup_gpio_init(td_void)
{
#if defined(_PRE_OS_VERSION) && defined(_PRE_OS_VERSION_LINUX) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    td_s32 ret = BOARD_FAIL;
    td_s32 wkup_gpio_idx;
    td_s32 wkup_gpio_trigger;
    td_s32 wkup_gpio_irq_num;
    td_u8 cfg_buff[DOWNLOAD_CHANNEL_LEN] = {0};

    // 获取配置的wkup_gpio管脚号
    ret = get_cust_conf_string_etc(INI_MODU_PLAT, "wkup_gpio_idx", cfg_buff, DOWNLOAD_CHANNEL_LEN);
    if (ret != BOARD_SUCC) {
        oal_print_err("board_wakeup_gpio_init::get wkup_gpio_idx fail. \n");
        goto GET_WKUP_GPIO_FAIL;
    }
    wkup_gpio_idx = oal_strtol(cfg_buff, NULL, NUM_BASE_10);
    if (!pm_board_is_gpio_idx_valid(wkup_gpio_idx)) {
        oal_print_err("board_wakeup_gpio_init::get wkup_gpio_idx invalid, value=[%d]. \n", wkup_gpio_idx);
        goto GET_WKUP_GPIO_FAIL;
    }

    // 获取配置的wkup_gpio电平
    ret = get_cust_conf_string_etc(INI_MODU_PLAT, "wkup_gpio_level", cfg_buff, DOWNLOAD_CHANNEL_LEN);
    if (ret != BOARD_SUCC) {
        oal_print_err("board_wakeup_gpio_init::get wkup_gpio_trigger fail. \n");
        goto GET_WKUP_GPIO_FAIL;
    }
    wkup_gpio_trigger = oal_strtol(cfg_buff, NULL, NUM_BASE_10);
    wkup_gpio_trigger = (wkup_gpio_trigger == 0) ? IRQF_TRIGGER_FALLING : IRQF_TRIGGER_RISING;

    // 配置默认输入
    ret = gpio_request_one(wkup_gpio_idx, GPIOF_IN, PROC_NAME_GPIO_DEVICE_WAKEUP_HOST);
    if (ret != BOARD_SUCC) {
        oal_print_err("board_wakeup_gpio_init::request wkup_gpio_idx fail. \n");
        goto GET_WKUP_GPIO_FAIL;
    }

    // 注册中断
    wkup_gpio_irq_num = gpio_to_irq(wkup_gpio_idx);
    ret = request_irq(wkup_gpio_irq_num, pm_board_wakeup_gpio_irq, wkup_gpio_trigger, "wkup_gpio_irq",
        (void *)(&g_pm_board_info));
    if (ret != BOARD_SUCC) {
        oal_print_err("board_wakeup_gpio_init: request irq fail, ret[%d] \n", ret);
        goto GET_WKUP_GPIO_FAIL;
    }

    g_pm_board_info.wkup_gpio_support = OSAL_TRUE;
    g_pm_board_info.wkup_gpio_idx = wkup_gpio_idx;
    g_pm_board_info.wkup_gpio_irq = wkup_gpio_irq_num;
    g_pm_board_info.wkup_gpio_cb = OSAL_NULL;

    oal_print_err("board_wakeup_gpio_init::succ, wkup_gpio_idx=[%d]! \n", wkup_gpio_idx);

    return BOARD_SUCC;

GET_WKUP_GPIO_FAIL:
#endif
    g_pm_board_info.wkup_gpio_support = OSAL_FALSE;
    g_pm_board_info.wkup_gpio_idx = GPIO_IDX_INVALID;
    g_pm_board_info.wkup_gpio_irq = GPIO_IRQ_INVALID;
    g_pm_board_info.wkup_gpio_cb = OSAL_NULL;
    return BOARD_SUCC;
}

td_s32 pm_board_power_gpio_init(td_void)
{
#if defined(_PRE_OS_VERSION) && defined(_PRE_OS_VERSION_LINUX) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    osal_s32 ret = BOARD_FAIL;
    osal_s32 power_gpio_idx;
    osal_u8 cfg_buff[DOWNLOAD_CHANNEL_LEN] = {0};

    // 获取配置的power_gpio管脚号
    ret = get_cust_conf_string_etc(INI_MODU_PLAT, "power_gpio_idx", cfg_buff, DOWNLOAD_CHANNEL_LEN);
    if (ret != BOARD_SUCC) {
        oal_print_err("board_power_gpio_init::get power_gpio_idx fail. \n");
        goto GET_POWER_GPIO_FAIL;
    }
    power_gpio_idx = oal_strtol(cfg_buff, NULL, NUM_BASE_10);
    if (!pm_board_is_gpio_idx_valid(power_gpio_idx)) {
        oal_print_err("board_power_gpio_init::get power_gpio_idx invalid, value=[%d]. \n", power_gpio_idx);
        goto GET_POWER_GPIO_FAIL;
    }

    // 配置默认输出, 初始配为输出+低电平
    ret = gpio_request_one(power_gpio_idx, GPIOF_OUT_INIT_HIGH, PROC_NAME_GPIO_POWER_ON);
    if (ret != BOARD_SUCC) {
        oal_print_err("board_power_gpio_init::request power_gpio fail. \n");
        goto GET_POWER_GPIO_FAIL;
    }
    g_pm_board_info.power_gpio_support = OSAL_TRUE;
    g_pm_board_info.power_gpio = power_gpio_idx;

    oal_print_err("board_power_gpio_init::succ, power_gpio_idx=[%d]! \n", power_gpio_idx);

    return BOARD_SUCC;

GET_POWER_GPIO_FAIL:
#endif
    g_pm_board_info.power_gpio_support = OSAL_FALSE;
    g_pm_board_info.power_gpio = GPIO_IDX_INVALID;
    return BOARD_SUCC;
}

#if defined(CONFIG_HCC_SUPPORT_UART) && defined(CONFIG_PLAT_UART_IO_CFG)
#define UART_RX_GPIO   44
#define GPIO_DELAY_MS  20
typedef struct {
    struct pinctrl *pinctrl;
    struct pinctrl_state *gpio_state;
    struct pinctrl_state *uart_state;
} tr_huart0_pinctrl;

tr_huart0_pinctrl g_uartio_cfg_gpio_ctrl;

static td_s32 uart_io_cfg_gpio_init(td_void)
{
    td_s32 ret;
    struct device_node *np = NULL;
    struct platform_device *pdev = NULL;
    struct pinctrl_state *uart_state0 = NULL;
    struct pinctrl_state *uart_state1 = NULL;
    tr_huart0_pinctrl* cur_pinctrl = &g_uartio_cfg_gpio_ctrl;
    (void)memset_s(cur_pinctrl, sizeof(tr_huart0_pinctrl), 0, sizeof(tr_huart0_pinctrl));
    // 获取设备节点
    np = of_find_compatible_node(NULL, NULL, "tr5330");
    if (np == NULL) {
        oal_print_err("can not find tr5330 node!\n");
        return BOARD_FAIL;
    }
    pdev = of_find_device_by_node(np);
    if (pdev == NULL) {
        oal_print_err("find tr5330 dev fail!\n");
        return BOARD_FAIL;
    }

    // 获取pinctrl信息
    cur_pinctrl->pinctrl = devm_pinctrl_get(&pdev->dev);
    if (cur_pinctrl->pinctrl == NULL) {
        oal_print_err("find tr5330 pinctrl fail!\n");
        return BOARD_FAIL;
    }
    // 获取复用配置信息
    uart_state0 = pinctrl_lookup_state(cur_pinctrl->pinctrl, "uart0_pad0");
    if (uart_state0 == NULL) {
        oal_print_err("uart0_pad0 non exsit\n");
        goto GET_STATE_FAIL;
    }
    cur_pinctrl->gpio_state = uart_state0;
    uart_state1 = pinctrl_lookup_state(cur_pinctrl->pinctrl, "uart0_pad1");
    if (uart_state1 == NULL) {
        oal_print_err("uart0_pad1 non exsit\n");
        goto GET_STATE_FAIL;
    }
    cur_pinctrl->uart_state = uart_state1;

    // 注册GPIO
    ret = gpio_request_one(UART_RX_GPIO, GPIOF_OUT_INIT_LOW, PROC_NAME_GPIO_DEVICE_UART);
    if (ret != EOK) {
        oal_print_err("tr5330_uart_gpio request: set gpio[%d] init fail, ret=[%d] \n", UART_RX_GPIO, ret);
        goto GET_STATE_FAIL;
    }
    return BOARD_SUCC;

GET_STATE_FAIL:
    devm_pinctrl_put(cur_pinctrl->pinctrl);
    cur_pinctrl->pinctrl = NULL;
    cur_pinctrl->gpio_state = NULL;
    cur_pinctrl->uart_state = NULL;
    return BOARD_FAIL;
}

static void uart_io_cfg_gpio_deinit(void)
{
    tr_huart0_pinctrl *cur_pinctrl = &g_uartio_cfg_gpio_ctrl;
    gpio_free(UART_RX_GPIO);
    if (cur_pinctrl->pinctrl != NULL) {
        devm_pinctrl_put(cur_pinctrl->pinctrl);
    }
    (void)memset_s(cur_pinctrl, sizeof(tr_huart0_pinctrl), 0, sizeof(tr_huart0_pinctrl));
}

td_s32 gpio_state_change_uart2cfg(void)
{
    td_s32 ret;
    tr_huart0_pinctrl *cur_pinctrl = &g_uartio_cfg_gpio_ctrl;
    if (cur_pinctrl->pinctrl == NULL && cur_pinctrl->gpio_state == NULL) {
        return BOARD_FAIL;
    }

    ret = pinctrl_select_state(cur_pinctrl->pinctrl, cur_pinctrl->gpio_state);
    if (ret != EOK) {
        oal_print_err("uart2cfg state change fail, ret %x", ret);
        return BOARD_FAIL;
    }
    ret = gpio_direction_output(UART_RX_GPIO, GPIO_LOWLEVEL);
    if (ret != EOK) {
        oal_print_err("set gpio[%d] to low fail, ret=[%d] \n", UART_RX_GPIO, ret);
        return BOARD_FAIL;
    }
    osal_mdelay(GPIO_DELAY_MS);

    return BOARD_SUCC;
}

td_s32 gpio_state_change_cfg2uart(void)
{
    td_s32 ret;
    tr_huart0_pinctrl *cur_pinctrl = &g_uartio_cfg_gpio_ctrl;
    if (cur_pinctrl->pinctrl == NULL && cur_pinctrl->uart_state == NULL) {
        return BOARD_FAIL;
    }

    ret = pinctrl_select_state(cur_pinctrl->pinctrl, cur_pinctrl->uart_state);
    if (ret != EOK) {
        oal_print_err("cfg2uart state change fail, ret %x", ret);
        return BOARD_FAIL;
    }
    osal_mdelay(GPIO_DELAY_MS);
    return BOARD_SUCC;
}
#endif

td_s32 pm_board_gpio_init(struct platform_device *pdev)
{
    pm_board_power_gpio_init();
    pm_board_wakeup_gpio_init();
#if defined(CONFIG_HCC_SUPPORT_UART) && defined(CONFIG_PLAT_UART_IO_CFG)
    uart_io_cfg_gpio_init();
#endif

    return BOARD_SUCC;
}

td_s32 pm_board_gpio_deinit(td_void)
{
#if defined(CONFIG_HCC_SUPPORT_UART) && defined(CONFIG_PLAT_UART_IO_CFG)
    uart_io_cfg_gpio_deinit();
#endif
    pm_board_wakeup_gpio_deinit();
    pm_board_power_gpio_deinit();

    return BOARD_SUCC;
}

#if defined(_PRE_OS_VERSION) && defined(_PRE_OS_VERSION_LINUX) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
td_s32 pm_board_is_support_wkup_gpio(td_void)
{
    return g_pm_board_info.wkup_gpio_support;
}
EXPORT_SYMBOL(pm_board_is_support_wkup_gpio);
#endif

td_s32 pm_board_irq_init(td_void)
{
    return BOARD_SUCC;
}

#if defined(_PRE_OS_VERSION) && defined(_PRE_OS_VERSION_LINUX) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
td_s32 pm_board_host_wakeup_dev_set(td_s32 value)
{
    if (g_pm_board_info.host_wakeup_wlan == 0) {
        oal_print_info("host_wakeup_wlan gpio is 0 \n");
        return 0;
    }
    oal_print_dbg("pm_board_host_wakeup_dev_set set %s \n", value ? "high":"low");
    if (value) {
        return gpio_direction_output(g_pm_board_info.host_wakeup_wlan, GPIO_HIGHLEVEL);
    } else {
        return gpio_direction_output(g_pm_board_info.host_wakeup_wlan, GPIO_LOWLEVEL);
    }
}

td_s32 pm_board_wifi_tas_set(td_s32 value)
{
    return 0;
}
#endif

td_s32 pm_board_power_reset(td_void)
{
    if (g_pm_board_info.bd_ops.board_power_reset == TD_NULL) {
        return BOARD_FAIL;
    }
    return g_pm_board_info.bd_ops.board_power_reset(TD_FALSE);
}

td_s32 pm_board_func_init(td_void)
{
    memset_s(&g_pm_board_info, sizeof(pm_board_info), 0, sizeof(pm_board_info));

    g_pm_board_info.bd_ops.wlan_power_off_etc             = tr5330_board_service_exit;
    g_pm_board_info.bd_ops.wlan_power_on_etc              = tr5330_board_service_enter;
    g_pm_board_info.bd_ops.board_power_on_etc             = tr5330_board_power_on;
    g_pm_board_info.bd_ops.board_power_off_etc            = tr5330_board_power_off;
    g_pm_board_info.bd_ops.board_power_reset              = tr5330_board_power_reset;

    return BOARD_SUCC;
}

static void pm_board_release(struct device * dev)
{
    return;
}

static osal_s32 pm_board_probe(struct platform_device *pdev)
{
    int ret = BOARD_FAIL;

    ret = pm_board_func_init();
    if (ret != BOARD_SUCC) {
        goto err_init;
    }

    ret = pm_board_gpio_init(pdev);
    if (ret != BOARD_SUCC) {
        goto err_init;
    }

    ret = pm_board_irq_init();
    if (ret != BOARD_SUCC) {
        goto err_gpio_source;
    }

#if defined(_PRE_OS_VERSION) && defined(_PRE_OS_VERSION_LINUX) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    g_pm_board_probe_ret = BOARD_SUCC;
    complete(&g_pm_board_driver_complete);
#endif

    return BOARD_SUCC;

err_gpio_source:
    oal_print_err("hh503_board_probe::fail, call board_gpio_deinit() \n");
    pm_board_gpio_deinit();

err_init:
    g_pm_board_probe_ret = BOARD_FAIL;
#if defined(_PRE_OS_VERSION) && defined(_PRE_OS_VERSION_LINUX) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    complete(&g_pm_board_driver_complete);
    oal_print_err("hh503_board_probe::fail! \n\n");
#endif
    return BOARD_FAIL;
}

static osal_s32 pm_board_remove(struct platform_device *pdev)
{
    pm_board_gpio_deinit();

    return BOARD_SUCC;
}

#if defined(_PRE_OS_VERSION) && defined(_PRE_OS_VERSION_LINUX) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
osal_s32 pm_board_suspend(struct platform_device *pdev, pm_message_t state)
{
    return BOARD_SUCC;
}

osal_s32 pm_board_resume(struct platform_device *pdev)
{
    return BOARD_SUCC;
}

static struct platform_device g_pm_board_device = {
    .name   = "pm_board",
    .dev    = {
        .release = pm_board_release,
    },
};

static struct platform_driver g_pm_board_driver = {
    .probe      = pm_board_probe,
    .remove     = pm_board_remove,
    .suspend    = pm_board_suspend,
    .resume     = pm_board_resume,
    .driver     = {
        .name   = "pm_board",
        .owner  = THIS_MODULE,
    },
};
#endif

osal_s32 pm_board_init(void)
{
    osal_s32 ret = BOARD_FAIL;

#if defined(_PRE_OS_VERSION) && defined(_PRE_OS_VERSION_LINUX) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    g_pm_board_probe_ret = BOARD_FAIL;
    init_completion(&g_pm_board_driver_complete);

    ret = platform_device_register(&g_pm_board_device);
    if (ret) {
        oal_print_err("hh503_board_init::call platform_device_register() fail, ret=[%d] \n", ret);
        return ret;
    }

    ret = platform_driver_register(&g_pm_board_driver);
    if (ret) {
        oal_print_err("hh503_board_init::call platform_driver_register() fail, ret=[%d] \n", ret);
        return ret;
    }

    if (wait_for_completion_timeout(&g_pm_board_driver_complete, WAIT_BOARD_DRIVER_S * HZ)) {
        /* completed */
        if (g_pm_board_probe_ret != BOARD_SUCC) {
            oal_print_err("hh503_board_init::board probe failed, ret=[%d] \n", g_pm_board_probe_ret);
            return g_pm_board_probe_ret;
        }
    } else {
        /* timeout */
        oal_print_err("hh503_board_init::board probe timeout \n");
        return BOARD_FAIL;
    }
#endif

#if defined(_PRE_OS_VERSION) && defined(_PRE_OS_VERSION_LITEOS) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
    ret = pm_board_probe(NULL);
#endif

    return ret;
}

void pm_board_exit(void)
{
#if defined(_PRE_OS_VERSION) && defined(_PRE_OS_VERSION_LITEOS) && (_PRE_OS_VERSION_LITEOS == _PRE_OS_VERSION)
    pm_board_remove(NULL);
#endif

#if defined(_PRE_OS_VERSION) && defined(_PRE_OS_VERSION_LINUX) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    platform_device_unregister(&g_pm_board_device);
    platform_driver_unregister(&g_pm_board_driver);
    complete_all(&g_pm_board_driver_complete);
    reinit_completion(&g_pm_board_driver_complete);
#endif
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

