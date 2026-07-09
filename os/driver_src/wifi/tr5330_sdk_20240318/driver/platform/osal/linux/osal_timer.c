/*
 * Copyright (c) CompanyNameMagicTag 2012-2021. All rights reserved.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0)
#include <linux/sched/clock.h>
#else
#include <linux/sched.h>
#endif
#include <linux/rtc.h>
#include "soc_osal.h"
#include "osal_inner.h"

int osal_hrtimer_create(osal_hrtimer *hrtimer)
{
    return -1;
}
EXPORT_SYMBOL(osal_hrtimer_create);

int osal_hrtimer_start(osal_hrtimer *hrtimer)
{
    return -1;
}
EXPORT_SYMBOL(osal_hrtimer_start);

int osal_hrtimer_destroy(osal_hrtimer *hrtimer)
{
    return -1;
}
EXPORT_SYMBOL(osal_hrtimer_destroy);

struct timer_list_info {
    struct timer_list time_list;
    unsigned long data;
    void (*handler)(td_uintptr_t);
};

unsigned long osal_timer_get_private_data(const void *data)
{
    struct timer_list_info *list_info = container_of(data, typeof(*list_info), time_list);
    return list_info->data;
}
EXPORT_SYMBOL(osal_timer_get_private_data);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,14,0)
static void timer_callback(struct timer_list *os_timer)
{
    struct timer_list_info *time_info = container_of(os_timer, struct timer_list_info, time_list);
    td_uintptr_t timer_ptr = (td_uintptr_t)os_timer;
    time_info->handler(timer_ptr);
}
#endif

int osal_timer_init(osal_timer *timer)
{
    struct timer_list_info *time_info = NULL;

    if (timer == NULL || timer->handler == NULL) {
        osal_printk("%s - parameter invalid!\n", __FUNCTION__);
        return -1;
    }

    time_info = (struct timer_list_info *)kmalloc(sizeof(struct timer_list_info), GFP_KERNEL);
    if (time_info == NULL) {
        osal_printk("%s - kmalloc error!\n", __FUNCTION__);
        return -1;
    }
    time_info->data = timer->data;
    time_info->handler = timer->handler;
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,14,0)
    init_timer(&time_info->time_list);
    time_info->time_list.function = timer->handler;
#else
    timer_setup(&time_info->time_list, timer_callback, 0);
#endif
    time_info->time_list.expires = jiffies + msecs_to_jiffies(timer->interval) - 1;
    timer->timer = time_info;
    return 0;
}
EXPORT_SYMBOL(osal_timer_init);

int osal_timer_mod(osal_timer *timer, unsigned int interval)
{
    unsigned long expires;
    struct timer_list_info *time_info = NULL;
    if ((timer == NULL) || (timer->timer == NULL) || (timer->handler == NULL)) {
        osal_printk("%s - parameter invalid!\n", __FUNCTION__);
        return -1;
    }
    time_info = (struct timer_list_info *)timer->timer;
    expires = jiffies + msecs_to_jiffies(interval) - 1;
    mod_timer(&time_info->time_list, expires);
    timer->interval = interval;
    return 0;
}
EXPORT_SYMBOL(osal_timer_mod);

int osal_timer_start(osal_timer *timer)
{
    struct timer_list_info *time_info = NULL;
    if ((timer == NULL) || (timer->timer == NULL) || (timer->handler == NULL)) {
        osal_printk("%s - parameter invalid!\n", __FUNCTION__);
        return -1;
    }
    time_info = (struct timer_list_info *)timer->timer;
    add_timer(&time_info->time_list);
    return 0;
}

int osal_timer_stop(osal_timer *timer)
{
    struct timer_list_info *time_info = NULL;

    if ((timer == NULL) || (timer->timer == NULL) || (timer->handler == NULL)) {
        osal_printk("%s - parameter invalid!\n", __FUNCTION__);
        return -1;
    }

    time_info = (struct timer_list_info *)timer->timer;
    return del_timer(&time_info->time_list);
}
EXPORT_SYMBOL(osal_timer_stop);

int osal_timer_destroy(osal_timer *timer)
{
    struct timer_list_info *time_info = NULL;

    if ((timer == NULL) || (timer->timer == NULL)) {
        osal_printk("%s - parameter invalid!\n", __FUNCTION__);
        return -1;
    }

    time_info = (struct timer_list_info *)timer->timer;
    del_timer(&time_info->time_list);
    kfree(time_info);
    timer->timer = NULL;
    return 0;
}
EXPORT_SYMBOL(osal_timer_destroy);

unsigned long osal_msleep(unsigned int msecs)
{
    return msleep_interruptible(msecs);
}
EXPORT_SYMBOL(osal_msleep);

void osal_msleep_uninterruptible(unsigned int msecs)
{
    msleep(msecs);
}
EXPORT_SYMBOL(osal_msleep_uninterruptible);

void osal_udelay(unsigned int usecs)
{
    udelay(usecs);
}
EXPORT_SYMBOL(osal_udelay);

void osal_mdelay(unsigned int msecs)
{
    mdelay(msecs);
}
EXPORT_SYMBOL(osal_mdelay);

unsigned int osal_jiffies_to_msecs(const unsigned int n)
{
    return jiffies_to_msecs(n);
}
EXPORT_SYMBOL(osal_jiffies_to_msecs);

unsigned long long osal_sched_clock(void)
{
    return sched_clock();
}
EXPORT_SYMBOL(osal_sched_clock);

void osal_gettimeofday(osal_timeval *tv)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(5,0,0)
    struct timeval t;
#else
    struct timespec64 t;
#endif
    if (tv == NULL) {
        osal_printk("%s - parameter invalid!\n", __FUNCTION__);
        return;
    }
#if LINUX_VERSION_CODE < KERNEL_VERSION(5,0,0)
    do_gettimeofday(&t);
#else
    ktime_get_real_ts64(&t);
#endif

    tv->tv_sec = t.tv_sec;

#if LINUX_VERSION_CODE < KERNEL_VERSION(5,0,0)
    tv->tv_usec = t.tv_usec;
#else
    tv->tv_usec = t.tv_nsec / 1000; // 1000: nsec->usec
#endif
}
EXPORT_SYMBOL(osal_gettimeofday);

unsigned long osal_msecs_to_jiffies(const unsigned int m)
{
    return msecs_to_jiffies(m);
}
EXPORT_SYMBOL(osal_msecs_to_jiffies);

unsigned long long osal_get_jiffies(void)
{
    return jiffies;
}
EXPORT_SYMBOL(osal_get_jiffies);

#define OSAL_TM_COVERT(a, b) do { \
    (a)->tm_sec = (b)->tm_sec;    \
    (a)->tm_min = (b)->tm_min;    \
    (a)->tm_hour = (b)->tm_hour;    \
    (a)->tm_mday = (b)->tm_mday;    \
    (a)->tm_mon = (b)->tm_mon;    \
    (a)->tm_year = (b)->tm_year;    \
    (a)->tm_wday = (b)->tm_wday;    \
    (a)->tm_yday = (b)->tm_yday;    \
    (a)->tm_isdst = (b)->tm_isdst;    \
} while (0)

void osal_rtc_time_to_tm(unsigned long time, osal_rtc_time *tm)
{
    struct rtc_time _tm = { 0 };
    struct rtc_time *systm = &_tm;

    if (tm == NULL) {
        osal_log("parameter invalid!\n");
        return;
    }

#if LINUX_VERSION_CODE < KERNEL_VERSION(5,10,0)
    rtc_time_to_tm(time, &_tm);
#else
    _tm = rtc_ktime_to_tm(time);
#endif

    OSAL_TM_COVERT(tm, systm);
}
EXPORT_SYMBOL(osal_rtc_time_to_tm);

void osal_rtc_tm_to_time(const osal_rtc_time *tm, unsigned long *time)
{
    struct rtc_time _tm;
    struct rtc_time *systm = &_tm;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0)
    ktime_t tmp_tm;
#endif

    if (tm == NULL || time == NULL) {
        osal_log("parameter invalid!\n");
        return;
    }

    OSAL_TM_COVERT(systm, tm);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)
    *time = rtc_tm_to_time64(systm);
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(5,10,0)
    rtc_tm_to_time(systm, time);
#else
    tmp_tm = rtc_tm_to_ktime(_tm);
    *time = (unsigned long)tmp_tm;
#endif
}
EXPORT_SYMBOL(osal_rtc_tm_to_time);

int osal_rtc_valid_tm(const osal_rtc_time *tm)
{
    struct rtc_time _tm;
    struct rtc_time *systm = &_tm;

    if (tm == NULL) {
        osal_log("parameter invalid!\n");
        return OSAL_FAILURE;
    }

    OSAL_TM_COVERT(systm, tm);
    return rtc_valid_tm(systm);
}
EXPORT_SYMBOL(osal_rtc_valid_tm);
