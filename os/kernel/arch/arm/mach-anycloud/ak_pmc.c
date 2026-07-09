/**
**  drivers/rtc/ak_rtc.c
**  AK RTC related routines
**  author    zhang zhipeng
**  date      2019-2-20
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License version 2 as
** published by the Free Software Foundation.
**   Copyright C 2019 Anyka CO.,LTD
**/


#include <linux/fs.h>
#include <linux/string.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/rtc.h>
#include <linux/bcd.h>
#include <linux/clk.h>
#include <linux/log2.h>
#include <linux/delay.h>
#include <linux/time.h>
#include <linux/types.h>
#include <linux/signal.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/mach/time.h>

#include <mach/map.h>
#include <mach/ak_l2_exebuf.h>



#define AK_RTC_CONF         (AK_VA_SYSCTRL + 0x50)
#define AK_RTC_DATA         (AK_VA_SYSCTRL + 0x54)
#define OTHER_WAKEUP_CTRL   (AK_VA_SYSCTRL + 0x34)
#define OTHER_WAKEUP_STAT   (AK_VA_SYSCTRL + 0x38)


#define RTC_RDY_INT_CTRL        (AK_VA_SYSCTRL + 0x2C)
#define RTC_RDY_INT_STAT        (AK_VA_SYSCTRL + 0x30)
#define RTC_INT_ALARM_STAT        (1 << 8)
#define RTC_INT_TIMER_STAT        (1 << 9)


#define RTC_RDY_CTRL_BIT        (1 << 7)
#define RTC_RDY_STAT_BIT        (1 << 7)
#define RTC_TIMER0_CTRL_BIT     (1 << 5)
#define RTC_TIMER0_STAT_BIT     (1 << 5)

#define RTC_TIMER_WAKEUP_EN     (1 << 13)
#define RTC_TIMER_INT_CLR       (1 << 8)
#define RTC_WAKEUP_EN           (1 << 12)
#define RTC_CONF_RTC_WR_EN  (1 << 25)
#define RTC_CONF_RTC_EN     (1 << 24)
#define	AK_FALSE				0
#define	AK_TRUE					1


#define RTC_CONF_RTC_READ   ((1 << 22) | (2 << 19) | (1 << 18))
#define RTC_CONF_RTC_WRITE  ((1 << 22) | (2 << 19) | (0 << 18))

#define AK_RTC_REAL_TIME1                     (0x0)
#define AK_RTC_REAL_TIME2                     (0x1)
#define AK_RTC_REAL_TIME3                     (0x2)
#define AK_RTC_ALARM_TIME1                    (0x3)
#define AK_RTC_ALARM_TIME2                    (0x4)
#define AK_RTC_ALARM_TIME3                    (0x5)
#define AK_RTC_WATCH_DOG_REG1                 (0x6)
#define AK_RTC_WATCH_DOG_REG2                 (0x7)
#define AK_RTC_32K_ADJUST_REG1                (0x8)
#define AK_RTC_32K_ADJUST_REG2                (0x9)
#define AK_RTC_SETTING                        (0xa)
#define AK_RTC_SIGNAL_to_ANALOG_CFG_REG       (0xb)
#define AK_RTC_REG_MAX  PMC_PAD_CONFIG_REG


int rtc_wait_timeout = 0;


/*
 * When the RTC module begins to receive/send data,
 * bit [24] of Interrupt Enable/Status
 * Register of System Control Module (Add: 0x0800, 004C) is set to 0;
 * and then this bit is set to 1 automatically to indicate that
 * the data has been well received/sent
 */

static int flag = 1;
static void inline ak_rtc_wait_ready(void)
{
    unsigned long timeout = 0;
    while (!(__raw_readl(RTC_RDY_INT_STAT) & RTC_RDY_STAT_BIT)) {
        /*
         * 只判断一次加载时注册rtc读写时候正常
        */
        if (flag){
            ++timeout;
            udelay(10);
            if (timeout >= 1000) {
                rtc_wait_timeout = 1;
                printk("ak_rtc_wait_ready timeout\n");
                break;
           }
            flag--;
        }
    }
}

static void inline rtc_ready_irq_enable(void)
{
    unsigned long regval;
    /*
     * Wait for RTC Ready Interrupt to be cleared
     */
    ak_rtc_wait_ready();

    /*
     * Enable RTC Register Read/Write
     */
    regval = __raw_readl(AK_RTC_CONF);
    regval |= RTC_CONF_RTC_WR_EN;
    __raw_writel(regval, AK_RTC_CONF);
}

static void inline rtc_ready_irq_disable(void)
{
    unsigned long regval;
    /*
     * Disable RTC Register Read/Write
     */
    regval = __raw_readl(AK_RTC_CONF);
    regval &= ~RTC_CONF_RTC_WR_EN;
    __raw_writel(regval, AK_RTC_CONF);
    
}


static unsigned int ak_rtc_read(unsigned int addr)
{
    unsigned long regval = 0;
    
    if (addr > AK_RTC_REG_MAX) {
        printk("%s(): Invalid RTC Register, address=%d\n",
            __func__, addr);
        return -1;
    }
    
    rtc_ready_irq_enable(); 

    regval  = __raw_readl(AK_RTC_CONF);
    regval  &= ~(0x3FFFFF) ;
    regval  |= (RTC_CONF_RTC_READ | (addr << 14));
    __raw_writel(regval, AK_RTC_CONF);
    ak_rtc_wait_ready();
    
    rtc_ready_irq_disable();
    

    // according to ATC drivers ,this want to wait 1/32K s here

    regval = __raw_readl(AK_RTC_DATA);
    regval &= 0x3FFF;
    
    return regval;
}

static unsigned int ak_rtc_write(unsigned int addr, unsigned int value)
{
    unsigned long regval = 0;
    

    if (addr > AK_RTC_REG_MAX) {
        printk("%s(): Invalid RTC Register, address=%d\n",
            __func__, addr);
        return -1;
    }

    //local_irq_save(flags);

    rtc_ready_irq_enable();

    regval  = __raw_readl(AK_RTC_CONF);
    regval  &= ~0x3FFFFF;
    regval  |= (RTC_CONF_RTC_WRITE | (addr << 14) | value);
    __raw_writel(regval, AK_RTC_CONF);


    ak_rtc_wait_ready();
    
    rtc_ready_irq_disable();

    //local_irq_restore(flags);

    // according to ATC drivers ,this want to wait 1/32K s here

    return 0;
}


/****************************pmc操作**********************************/



void pmc_cfg_val_update(void)
{
    unsigned reg_val;

    reg_val = ak_rtc_read(PMC_CONTROL_REG0);
    reg_val |= (0x1<<13);
    ak_rtc_write(PMC_CONTROL_REG0,reg_val);

    while(1){
        reg_val = ak_rtc_read(PMC_CONTROL_REG0);
        if((reg_val & (0x1<<13)) == 0)
            break;
        pr_info("PMC_CONTROL_REG0 = %x\n", reg_val);
    }
    //!!!注意：设置PMC_CONTROL_REG0 BIT13为1后必须等待1个16KHZ周期以上才能再次设置BIT13
    //因此这里直接延时1ms
     udelay(1000);
}


//pwr_td0设置
int pmc_cfg_pwr_td0(unsigned int pwr_td0_ms)
{
    unsigned int reg_val;

    if(pwr_td0_ms > 255){
        pr_err("Err: config pwr_td0. must < 256\n");
        return -1;
    }
    if(pwr_td0_ms == 0){
        pr_err("Err: pwr_td0 must not = 0\n");
        return -1;
    }

    reg_val = ak_rtc_read(PMC_CONTROL_REG1);
    reg_val &= (~0xff);
    reg_val |= pwr_td0_ms;
    ak_rtc_write(PMC_CONTROL_REG1, reg_val);
    //让设置值有效
    pmc_cfg_val_update();
    pr_info("[%s line:%d]pwr_td0_ms :%d ms ok \n",__func__,__LINE__,pwr_td0_ms);

    return 0;
}

//回读确认是否pwr_td0正常设置
unsigned int pmc_read_pwr_td0(void)
{
    unsigned int reg_val;

    reg_val = ak_rtc_read(PMC_CONTROL_REG1);
    reg_val &= 0xff;
    return reg_val;
}

/*
应用例子：
if(pmc_cfg_pwr_td0(pwr_td0) < 0)
    return;
if(pmc_read_pwr_td0() != pwr_td0){
    drv_err("Err: pwr_td0 config fail.\n");
    return;
}
*/
/************poweron_button*****************/

void pmc_poweron_button_press_time_cfg(unsigned int poweron_press_time)
{	
    unsigned int reg_val;
    unsigned int low_press_time;
    unsigned int high_press_time;

    if((poweron_press_time == 0) || (poweron_press_time > 16383)){
        pr_err("Err: config poweron_press_time must < 16384 & !=0\n");
        return;
    }

    low_press_time = (poweron_press_time &0x3f); 
    reg_val = ak_rtc_read(PMC_CONTROL_REG1);
    reg_val &= (~(0x3f<<8));
    reg_val |= (low_press_time<<8);
    ak_rtc_write(PMC_CONTROL_REG1,reg_val);

    high_press_time = (poweron_press_time>>6) & 0xff;
    reg_val = ak_rtc_read(PMC_CONTROL_REG0);
    reg_val &= (~(0xff<<5));
    reg_val |= (high_press_time<<5);
    ak_rtc_write(PMC_CONTROL_REG0,reg_val);

    pmc_cfg_val_update();
}


unsigned int pmc_read_poweron_button_press_time(void)
{
    unsigned int reg_val;
    unsigned int button_press_time;

    reg_val = ak_rtc_read(PMC_CONTROL_REG1);
    button_press_time = (reg_val>>8) & 0x3f;

    reg_val = ak_rtc_read(PMC_CONTROL_REG0);
    button_press_time |= (((reg_val>>5) &0xff)<<6);

    return button_press_time;
}

/*
drv_info("Input poweron button press time(1~16383) is %d, unit: ms \n", poweron_button_time);
pmc_poweron_button_press_time_cfg(poweron_button_time);
if( pmc_read_poweron_button_press_time() != poweron_button_time){
    drv_info("Err: poweron_button_time config fail.\n");
}
*/

/***************pwr_wakeup_pin****************/



void pmc_power_wakeup_cfg(T_PMC_PWR_WAKEUP_TRIG_MODE pwr_wakeup_trig_mode)
{
    unsigned int reg_val;

    if(pwr_wakeup_trig_mode > PMC_PWR_WAKEUP_LOW_LEVEL_TRIG){
        pr_err("Err: param pwr_wakeup_trig_mode.\n");
        return;
    }

    reg_val = ak_rtc_read(PMC_PAD_CONFIG_REG);
    reg_val &= ~(0x3<<8);

    switch(pwr_wakeup_trig_mode){
        case PMC_PWR_WAKEUP_RISING_EDGE_TRIG:
        case PMC_PWR_WAKEUP_HIGH_LEVEL_TRIG:
            reg_val |= (0x1<<9);
            break;
        case PMC_PWR_WAKEUP_FALLING_EDGE_TRIG:
        case PMC_PWR_WAKEUP_LOW_LEVEL_TRIG:	
            reg_val |= (0x1<<9);
            reg_val |= (0x1<<8);
            break;	
        default:
            break;
    }

    ak_rtc_write(PMC_PAD_CONFIG_REG,reg_val);

    reg_val = ak_rtc_read(PMC_CONTROL_REG0);
    reg_val &= ~(0x3<<1);
    reg_val |= (pwr_wakeup_trig_mode<<1);
    ak_rtc_write(PMC_CONTROL_REG0,reg_val);

    pmc_cfg_val_update();
}


void pmc_power_wakeup_enable(void)
{
    unsigned int reg_val;

    reg_val = ak_rtc_read(PMC_CONTROL_REG0);
    reg_val |= (0x1<<4);
    ak_rtc_write(PMC_CONTROL_REG0,reg_val);

    pmc_cfg_val_update();
}

/********************rtc int*******************************/

void pmc_rtc_int_wakeup_enable(void)
{
    unsigned int reg_val;

    reg_val = ak_rtc_read(PMC_CONTROL_REG0);
    reg_val |= (0x1<<3);
    ak_rtc_write(PMC_CONTROL_REG0,reg_val);

    pmc_cfg_val_update();
}

void pmc_rtc_int_wakeup_disable(void)
{
    unsigned int reg_val;

    reg_val = ak_rtc_read(PMC_CONTROL_REG0);

    //设置rtc_int 禁能
    reg_val &= ~(0x1<<3);
    ak_rtc_write(PMC_CONTROL_REG0,reg_val);

    //让设置值有效
    pmc_cfg_val_update();

}


T_PMC_WAKEUP_SOURCE pmc_read_wakeup_source(void)
{
    u32 reg_val;

    reg_val = ak_rtc_read(PMC_STATUS_REG);

    //取低两位
    reg_val &= 0x3;


    switch(reg_val)
    {
        case PMC_PWR_WAKEUP_PIN_WAKEUP:
            pr_info("pmc_wakeup_source WAKEUP_PIN_WAKEUP success. \n");
            break;

        case PMC_PWR_BUTTON_WAKEUP:
            pr_info("pmc_wakeup_source BUTTON_WAKEUP success.\n");
            break;

        case PMC_RTC_ALARM_WAKEUP:
            pr_info("pmc_wakeup_source RTC_ALARM_WAKEUP success.\n");
            break;

        default:
            pr_err("Not config wakeup source.\n");
            break;
    }

    return reg_val;


}


void pmc_read_all_reg_val(void)
{
    u32  reg_val;
    u32 pwr_td0, button_press_time;



    reg_val = __raw_readl(AK_VA_SYSCTRL+0x30);
    pr_err("%x, REG32(SYSINT_STATUS_REG) =  %x\n",
            SYSINT_STATUS_REG, __raw_readl(AK_VA_SYSCTRL+0x30));

    reg_val = ak_rtc_read(AK_RTC_SETTING);
    pr_err("%x, REG32(RTC_SETTING_REG) =  %x\n",
            AK_RTC_SETTING, ak_rtc_read(AK_RTC_SETTING));

    reg_val = ak_rtc_read(AK_RTC_SIGNAL_to_ANALOG_CFG_REG);
    pr_err("%x, REG32(RTC_SETTING_REG) =  %x\n",
            AK_RTC_SIGNAL_to_ANALOG_CFG_REG,
            ak_rtc_read(AK_RTC_SIGNAL_to_ANALOG_CFG_REG));


    reg_val = ak_rtc_read(PMC_CONTROL_REG0);
    pr_err("%x, REG32(PMC_CONTROL_REG0) =  %x\n",
            PMC_CONTROL_REG0, ak_rtc_read(PMC_CONTROL_REG0));

    reg_val = ak_rtc_read(PMC_CONTROL_REG1);
    pr_err("%x, REG32(PMC_CONTROL_REG1) =  %x\n",
            PMC_CONTROL_REG1, ak_rtc_read(PMC_CONTROL_REG1));


    reg_val = ak_rtc_read(PMC_STATUS_REG);
    pr_err("%x, REG32(PMC_STATUS_REG) =  %x\n",
            PMC_STATUS_REG, ak_rtc_read(PMC_STATUS_REG));

    reg_val = ak_rtc_read(PMC_PAD_CONFIG_REG);
    pr_err("%x, REG32(PMC_PAD_CONFIG_REG) =  %x\n",
            PMC_PAD_CONFIG_REG, ak_rtc_read(PMC_PAD_CONFIG_REG));

    //回读pwr_td0
    pwr_td0 = pmc_read_pwr_td0();
    pr_err("pwr_td0 = %d ms\n", pwr_td0);

    //回读按键时间
    button_press_time = pmc_read_poweron_button_press_time();
    pr_err("button_press_time = %d ms\n", button_press_time);

    //回读唤醒源
    pmc_read_wakeup_source();

}


void gpio80_set_pwr_seq(u32 seq){

    u32  reg_val;
    reg_val = ak_rtc_read(PMC_PAD_CONFIG_REG);
    if(seq){
        reg_val &= ~(0x1<<5);
      //  pr_err(" reg_val &= ~(0x1<<5);\n");
    }else{
        reg_val |= (0x1<<5);
       // pr_err("  reg_val |= (0x1<<5);\n");
    }

    ak_rtc_write(PMC_PAD_CONFIG_REG,reg_val);

    //让设置值有效
    pmc_cfg_val_update();
}

//POWER SAVE BOOT配置
void PowerSaveBootCfg(void){
    unsigned int reg_val;
   
    //读取RTC  中的analog寄存器
    reg_val = ak_rtc_read(PMC_STATUS_REG);

    //将powersave bit 标志位+ retention  写入PMC_STATUS_REG
    reg_val &= ~(0x1f<<8);
    reg_val |= (POWER_SAVE_SYMBOL | (0x1<<13));
     pr_err("%s , line:%d n",__func__, __LINE__);
    ak_rtc_write(PMC_STATUS_REG, reg_val);
     pr_err("%s , line:%d n",__func__, __LINE__);

}





/*软件关机*/
void pmc_software_poweroff(void)
{
    unsigned int reg_val;

    reg_val = ak_rtc_read(PMC_CONTROL_REG0);

    //设置power down 
    reg_val |= (0x1<<0);
    ak_rtc_write(PMC_CONTROL_REG0,reg_val);

    //让设置值有效
    pmc_cfg_val_update();

    //重新使这里设置为0，避免后续的回读读到1,重新操作该寄存器时又产生重新关机
    reg_val &= ~(0x1<<0);
    ak_rtc_write(PMC_CONTROL_REG0,reg_val);

    //reg_val = read_rtc_reg(PMC_CONTROL_REG0);
    //printf("1reg_val = %x\n",reg_val);

}


/**
 * @BRIEF  pmc_software_poweroff
 * @AUTHOR Zou Tianxiang
 * @DATE   2025-2 -23  
 * @PARAM  
* @PARAM  
* @RETURN 
* @NOTE:  清除poweroff bit, 在aov boot后，应调用该寄存器清除poweroff bit以避免后续
          其余RTC寄存器的update操作，造成关机
*/
void pmc_clear_poweroff_bit(void)
{
    unsigned int reg_val;
    reg_val = ak_rtc_read(PMC_CONTROL_REG0);
    reg_val &= ~(0x1<<0);
    ak_rtc_write(PMC_CONTROL_REG0,reg_val);
}



u8 read_rtc_alarm_interrupt(void)
{
    if((__raw_readl(RTC_RDY_INT_STAT) & (RTC_INT_ALARM_STAT)) == (RTC_INT_ALARM_STAT) ){
        pr_err("[%s ] success ! \n",__func__);
        return AK_TRUE;
    }else{
        return AK_FALSE;
    }
}

u8 read_rtc_timer_interrupt(void)
{
    if((__raw_readl(RTC_RDY_INT_STAT) & (RTC_INT_TIMER_STAT)) == (RTC_INT_TIMER_STAT)){
        pr_err("[%s ] success ! \n",__func__);
        return AK_TRUE;
    }else{
        return AK_FALSE;
    }
}

u8 read_rtc_interrupt(void)
{
    if(read_rtc_alarm_interrupt() || read_rtc_timer_interrupt()){
        return AK_TRUE;
    }else{
       // pr_err("[%s] no interrupt occurred ! \n",__func__);
        return AK_FALSE;
    }
}










