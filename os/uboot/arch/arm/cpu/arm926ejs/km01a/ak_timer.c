/*
 * (C) Copyright 2003
 * Anyka Ltd.
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch-km01a/ak_cpu.h>


DECLARE_GLOBAL_DATA_PTR;

#define AK39_TIMER_CTRL1		TIMER8_CTRL1_REG
#define AK39_TIMER_CTRL2		TIMER8_CTRL2_REG

#define TIMER_USEC_SHIFT		16
#define TIMER_CNT_MASK			(0x3F<<26)

//define timer register bits
#define TIMER_CLEAR_BIT			(1<<30)
#define TIMER_FEED_BIT			(1<<29)
#define TIMER_ENABLE_BIT		(1<<28)
#define TIMER_STATUS_BIT		(1<<27)
#define TIMER_READ_SEL_BIT		(1<<26)

//define pwm/pwm mode
#define MODE_AUTO_RELOAD_TIMER	0x0
#define MODE_ONE_SHOT_TIMER		0x1
#define MODE_PWM				0x2   

#define timestamp gd->arch.tbl
#define lastdec gd->arch.lastinc

#define TIMER_SCALER 	1



#ifdef CONFIG_KM01A_CODE
#define TIMER_CLK_FREQ (CONFIG_SYS_CLK_FREQ/TIMER_SCALER)
#else
#define TIMER_CLK_FREQ 	(CONFIG_SYS_CLK_FREQ/TIMER_SCALER)
#endif
#define TIMER_LOAD_VAL 	(TIMER_CLK_FREQ/CONFIG_SYS_HZ)
//CONFIG_SYS_HZ定义是默认1000


ulong get_timer_masked(void);

#ifdef CONFIG_KM01A_CODE

/*use timer 0*/
#define KM01A_TIMER0_BASE 0X21110000

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

#endif

int timer_init(void)
{
	unsigned long regval;
 
	unsigned long timecnt = TIMER_LOAD_VAL; // 12ms

#ifdef CONFIG_KM01A_CODE
	//1. 设置timer的TIMER_LOAD计数初值。
	__raw_writel(timecnt, KM01A_TIMER0_BASE + CFG_TIMER_LOAD);

	//2. 设置timer的CFG_TIMER_INT_EN数值，使能中断。
	__raw_writel(AK_TIMER_INT_EN, KM01A_TIMER0_BASE + CFG_TIMER_INT_EN);

	//3. 设置timer的CFG_TIMER_DIV分频因子。
	//写0表示256分频，写1表示1分频，以此类推，写255表示255分频，最大支持256分频
	regval = __raw_readl(KM01A_TIMER0_BASE+CFG_TIMER_DIV);
	regval &= ~(0xFF);
	regval |=  0x1;
	__raw_writel(regval, KM01A_TIMER0_BASE+CFG_TIMER_DIV);

	/*4. 设置timer的CFG_TIMER_CTRL寄存器 工作模式*/
	regval = __raw_readl(KM01A_TIMER0_BASE+CFG_TIMER_CTRL);
	regval &= ~(AK_UPDATE_RELOAD);//don't update reload value
	regval &= ~(0x3 << 1);
	regval |= MODE_FREE_TIMER;
	__raw_writel(regval, KM01A_TIMER0_BASE+CFG_TIMER_CTRL);

	/*5、第0位为1使能定时器*/
	regval = __raw_readl(KM01A_TIMER0_BASE+CFG_TIMER_CTRL);
	regval |=ENABLE_TIMER;
	__raw_writel(regval, KM01A_TIMER0_BASE+CFG_TIMER_CTRL);

#else
	regval = (TIMER_ENABLE_BIT | TIMER_FEED_BIT | (MODE_AUTO_RELOAD_TIMER << 24));
	regval |= (TIMER_SCALER-1)<<16; /*pre divider*/
	writel(timecnt, AK39_TIMER_CTRL1);
	writel(regval, AK39_TIMER_CTRL2);
#endif


	return 0;
}

/*
 * timer without interrupts
 */
ulong get_timer (ulong base)
{
	return get_timer_masked() - base;
}

void reset_timer_masked(void)
{
	unsigned long regval;

	regval = readl(AK39_TIMER_CTRL2);
	writel(regval | TIMER_READ_SEL_BIT, AK39_TIMER_CTRL2);

	/* reset time */
	regval = readl(AK39_TIMER_CTRL1);
	lastdec = TIMER_LOAD_VAL - regval;
	timestamp = 0;	       /* start "advancing" time stamp from 0 */
}

unsigned long long get_ticks(void)
{
	unsigned long regval;
	ulong now;
#ifdef CONFIG_KM01A_CODE
	 /*5、第0位为1使能定时器*/
	  regval = __raw_readl(KM01A_TIMER0_BASE+CFG_TIMER_CTRL);
	  regval |= ENABLE_TIMER;
	  __raw_writel(regval, KM01A_TIMER0_BASE+CFG_TIMER_CTRL);

	   now = __raw_readl(KM01A_TIMER0_BASE+RET_TIMER_VALUE);
#else
	regval = __raw_readl(AK39_TIMER_CTRL2);
	__raw_writel(regval | TIMER_READ_SEL_BIT, AK39_TIMER_CTRL2);

	now = __raw_readl(AK39_TIMER_CTRL1);
#endif

   
	if (lastdec >= now) {		/* normal mode (non roll) */
		/* normal mode */
		timestamp += lastdec - now; /* move stamp fordward with absoulte diff ticks */
	} else {			/* we have overflow of the count down timer */
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
	return get_ticks()/TIMER_LOAD_VAL;
}


/* waits specified delay value and resets timestamp */
void udelay_masked (unsigned long usec)
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
void __udelay (unsigned long usec)
{
	udelay_masked(usec);
}

/*
 * This function is derived from PowerPC code (timebase clock frequency).
 * On ARM it returns the number of timer ticks per second.
 */
ulong get_tbclk (void)
{
	ulong tbclk;

	tbclk = CONFIG_SYS_HZ;
	return tbclk;
}


