/*
 * Copyright (c) CompanyNameMagicTag 2012-2021. All rights reserved.
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/kthread.h>
#include <linux/slab.h>
#include <linux/version.h>
#ifdef CONFIG_KERNEL_MODE_NEON
#include <asm/neon.h>
#endif
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
#include <uapi/linux/sched/types.h>
#endif
#include "soc_osal.h"
#include "securec.h"
#include "osal_inner.h"

#define PRIO_HALF_DEVIDER   2

osal_task *osal_kthread_create(osal_kthread_handler handler, void *data, const char *name, unsigned int stack_size)
{
    struct task_struct *k = NULL;

    osal_task *p = (osal_task *)kzalloc(sizeof(osal_task), GFP_KERNEL);
    if (p == NULL) {
        osal_log("kmalloc error!\n");
        return NULL;
    }

    k = kthread_run(handler, data, name);
    if (IS_ERR(k)) {
        osal_log("kthread create error! k = %#x!\n", k);
        kfree(p);
        p = NULL;
        return NULL;
    }
    p->task = k;
    return p;
}
EXPORT_SYMBOL(osal_kthread_create);

int osal_kthread_set_priority(osal_task *task, unsigned int priority)
{
    struct sched_param param;
    int ret = 0;

    if (task == NULL || task->task == NULL) {
        osal_log("task is invalid!\n");
        return OSAL_FAILURE;
    }

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 9, 0))
    if (priority >= (MAX_RT_PRIO / PRIO_HALF_DEVIDER)) {
        sched_set_fifo(task->task);
    } else {
        sched_set_fifo_low(task->task);
    }
    return OSAL_SUCCESS;
#endif

    param.sched_priority = priority;
    ret = sched_setscheduler(task->task, SCHED_RR, &param);
    if (ret != 0) {
        return OSAL_FAILURE;
    }
    return OSAL_SUCCESS;
}
EXPORT_SYMBOL(osal_kthread_set_priority);

void osal_kthread_set_affinity(osal_task *task, int cpu_mask)
{
    struct cpumask cpumask_set;

    if (task == NULL) {
        return;
    }

    if (cpu_mask == 0) {
        return;
    }

    cpumask_clear(&cpumask_set);

    ((OSAL_CPU_0 & (unsigned int)cpu_mask) == 0) ?
        0 : cpumask_set_cpu(0, &cpumask_set);

    ((OSAL_CPU_1 & (unsigned int)cpu_mask) == 0) ?
        0 : cpumask_set_cpu(1, &cpumask_set);

    ((OSAL_CPU_2 & (unsigned int)cpu_mask) == 0) ?
        0 : cpumask_set_cpu(2, &cpumask_set); /* cpu2 */

    ((OSAL_CPU_3 & (unsigned int)cpu_mask) == 0) ?
        0 : cpumask_set_cpu(3, &cpumask_set); /* cpu3 */

    set_cpus_allowed_ptr((struct task_struct *)task->task, &cpumask_set);
}
EXPORT_SYMBOL(osal_kthread_set_affinity);

int osal_kthread_should_stop(void)
{
    return kthread_should_stop();
}
EXPORT_SYMBOL(osal_kthread_should_stop);

void osal_kthread_destroy(osal_task *task, unsigned int stop_flag)
{
    if (task == NULL || task->task == NULL) {
        osal_log("parameter invalid!\n");
        return;
    }
    /* note: When you call the Kthread_stop function, the thread function cannot be finished, otherwise it will oops. */
    if (stop_flag != 0 && task->task != NULL) {
        kthread_stop((struct task_struct *)(task->task));
    }
    task->task = NULL;
    kfree(task);
}
EXPORT_SYMBOL(osal_kthread_destroy);

void osal_kthread_schedule(unsigned int sleep_ns)
{
    ktime_t wait_time;
    wait_time = ns_to_ktime(sleep_ns);
    set_current_state(TASK_UNINTERRUPTIBLE);
    schedule_hrtimeout(&wait_time, HRTIMER_MODE_REL);
}
EXPORT_SYMBOL(osal_kthread_schedule);

void osal_kthread_set_uninterrupt(void)
{
    current->flags |= PF_NOFREEZE;
    set_current_state(TASK_UNINTERRUPTIBLE);
}
EXPORT_SYMBOL(osal_kthread_set_uninterrupt);

void osal_kthread_set_running(void)
{
    set_current_state(TASK_RUNNING);
}
EXPORT_SYMBOL(osal_kthread_set_running);

void osal_cond_resched(void)
{
    cond_resched();
}
EXPORT_SYMBOL(osal_cond_resched);

/*
 * If you want to use yield() to wait for something, use wait_event().
 * If you want to use yield() to be 'nice' for others, use cond_resched().
 * If you still want to use yield(), do not!
 */
void osal_yield(void)
{
    yield();
}
EXPORT_SYMBOL(osal_yield);

void osal_kneon_begin(void)
{
#ifdef CONFIG_KERNEL_MODE_NEON
    kernel_neon_begin();
#endif
}
EXPORT_SYMBOL(osal_kneon_begin);

void osal_kneon_end(void)
{
#ifdef CONFIG_KERNEL_MODE_NEON
    kernel_neon_end();
#endif
}
EXPORT_SYMBOL(osal_kneon_end);

long osal_get_current_pid(void)
{
    return get_current()->pid;
}
EXPORT_SYMBOL(osal_get_current_pid);

long osal_get_current_tid(void)
{
    return get_current()->pid;
}
EXPORT_SYMBOL(osal_get_current_tid);

char *osal_get_current_taskname(void)
{
    return get_current()->comm;
}
EXPORT_SYMBOL(osal_get_current_taskname);

int osal_kthread_wakeup_process(osal_task *task)
{
    if (task == NULL) {
        return -1;
    }
    return wake_up_process(task->task);
}
EXPORT_SYMBOL(osal_kthread_wakeup_process);

void osal_kthread_bind(osal_task *task, unsigned int cpu)
{
    if (task == NULL) {
        return;
    }
    kthread_bind(task->task, cpu);
}
EXPORT_SYMBOL(osal_kthread_bind);
