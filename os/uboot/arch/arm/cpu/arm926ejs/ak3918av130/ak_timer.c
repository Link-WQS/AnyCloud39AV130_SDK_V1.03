/*
 * (C) Copyright 2003
 * Anyka Ltd.
 * SPDX-License-Identifier:    GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch-ak3918av130/ak_cpu.h>

DECLARE_GLOBAL_DATA_PTR;

#define timestamp           gd->arch.tbl
#define lastdec             gd->arch.lastinc

/*use timer 0*/
#define TICK_TIMER          (0)
#define TICK_TIMER_BASE     (TIMER_BASE_ADDR + (TICK_TIMER * 0x1000))

#define TIMER_SCALER        (1)

#define TIMER_CLK_FREQ      (CONFIG_SYS_CLK_FREQ/TIMER_SCALER)
#define TIMER_LOAD_VAL      (TIMER_CLK_FREQ/CONFIG_SYS_HZ)
//CONFIG_SYS_HZ定义是默认1000

/*register Offset Address*/
 #define CFG_TIMER_LOAD         (0x00)
 #define RET_TIMER_VALUE        (0x04)
 #define CFG_TIMER_CTRL         (0x08)
 #define CFG_TIMER_INTCLR       (0x0c)
 #define CFG_TIMER_INT_EN       (0x10)
 #define CFG_TIMER_DIV          (0x14)
 #define RET_TIMER_INT_STA      (0x18)

#define AK_TIMER_INTCLR         (0x01<<0)
#define AK_TIMER_INT_EN         (0x01<<0)
#define AK_UPDATE_RELOAD        (0x1 <<3)
/*define working mode */
//00：自由计数模式（默认输出）
#define MODE_FREE_TIMER         (0x00<<1)

//enable timer
#define ENABLE_TIMER            (0x01<<0)
//中断的状态位
#define TIMER_INT_STA_BIT       (0x01<<0)

int timer_init(void)
{
    unsigned long regval;
    unsigned long timecnt = TIMER_LOAD_VAL; // 12ms

    //1. 设置timer的TIMER_LOAD计数初值。
    __raw_writel(timecnt, TICK_TIMER_BASE + CFG_TIMER_LOAD);

    //2. 设置timer的CFG_TIMER_INT_EN数值，使能中断。
    __raw_writel(AK_TIMER_INT_EN, TICK_TIMER_BASE + CFG_TIMER_INT_EN);

    //3. 设置timer的CFG_TIMER_DIV分频因子。
    //写0表示256分频，写1表示1分频，以此类推，写255表示255分频，最大支持256分频
    regval = __raw_readl(TICK_TIMER_BASE + CFG_TIMER_DIV);
    regval &= ~(0xFF);
    regval |=  TIMER_SCALER;
    __raw_writel(regval, TICK_TIMER_BASE + CFG_TIMER_DIV);

    /*4. 设置timer的CFG_TIMER_CTRL寄存器 工作模式*/
    regval = __raw_readl(TICK_TIMER_BASE + CFG_TIMER_CTRL);
    regval &= ~(AK_UPDATE_RELOAD);//don't update reload value
    regval &= ~(0x3 << 1);
    regval |= MODE_FREE_TIMER;
    __raw_writel(regval, TICK_TIMER_BASE + CFG_TIMER_CTRL);

    /*5、第0位为1使能定时器*/
    regval = __raw_readl(TICK_TIMER_BASE + CFG_TIMER_CTRL);
    regval |=ENABLE_TIMER;
    __raw_writel(regval, TICK_TIMER_BASE + CFG_TIMER_CTRL);

    return 0;
}

void reset_timer_masked(void)
{

}

unsigned long long get_ticks(void)
{
    unsigned long regval;
    ulong now;

    /*5、第0位为1使能定时器*/
    regval = __raw_readl(TICK_TIMER_BASE + CFG_TIMER_CTRL);
    regval |= ENABLE_TIMER;
    __raw_writel(regval, TICK_TIMER_BASE + CFG_TIMER_CTRL);

    now = __raw_readl(TICK_TIMER_BASE + RET_TIMER_VALUE);

    if (lastdec >= now) {        /* normal mode (non roll) */
        /* normal mode */
        timestamp += lastdec - now; /* move stamp fordward with absoulte diff ticks */
    } else {            /* we have overflow of the count down timer */
        /* nts = ts + ld + (TLV - now)
         * ts=old stamp, ld=time that passed before passing through -1
         * (TLV-now) amount of time after passing though -1
         * nts = new "advancing time stamp"...it could also roll and cause problems.
         */
        timestamp += lastdec + TIMER_LOAD_VAL - now;
    }
    lastdec = now;

    return timestamp;
}


ulong get_timer_masked(void)
{
    return get_ticks() / TIMER_LOAD_VAL;
}

/*
 * timer without interrupts
 */
ulong get_timer(ulong base)
{
    return get_timer_masked() - base;
}

/* waits specified delay value and resets timestamp */
void udelay_masked(unsigned long usec)
{
    u32 tmo;
    u32 endtime;
    signed long diff;

    tmo = TIMER_CLK_FREQ / 1000;
    tmo *= usec;
    tmo /= 1000;

    endtime = get_ticks() + tmo;

    do {
        u32 now = get_ticks();
        diff = endtime - now;
    } while (diff >= 0);
}


/* delay x useconds AND preserve advance timestamp value */
void __udelay(unsigned long usec)
{
    udelay_masked(usec);
}

/*
 * This function is derived from PowerPC code (timebase clock frequency).
 * On ARM it returns the number of timer ticks per second.
 */
ulong get_tbclk(void)
{
    ulong tbclk;

    tbclk = CONFIG_SYS_HZ;
    return tbclk;
}
