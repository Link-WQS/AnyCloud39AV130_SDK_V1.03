/*
* @FILENAME: ak_rtc_drv_demo.c
* @BRIEF
* Copyright (C) 2019 Anyka(Guangzhou) Microelcctronics Technology CO., LTD.
* @DATE 2019-12-22
*/

#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include "ak_drv_common.h"
#include <sys/time.h>
#include <time.h>


/* 设置定时器唤醒时间命令 */
#define RTC_TIMER_WK_SET    _IOW('p', 0x20, uint16_t)
/* 启动定时器唤醒命令 */
#define RTC_TIMER_WK_START  _IO('p', 0x21)
/* 停止定时器唤醒命令 */
#define RTC_TIMER_WK_STOP   _IO('p', 0x22)
/*rtc校准命令*/
#define INTENAL_RTC_ADJUST  _IO('p', 0x23)
/* internal RC OSC adjust read*/
#define INTENAL_RTC_ADJUST_READ  _IO('p', 0x24)

/* 电源管理路径 */
#define SYS_POWER_PATH      "/sys/power/state"
/* 设备节点 */
#define PROC_DEV_NODE       "/dev/rtc0"

/*
*hwclock -r:显示硬件时钟与日期
*hwclock -s：将系统时钟调整为与目前的硬件时钟一致
*hwclock -w：将硬件时钟调整为与目前的系统时钟一致
*date -s：设置Linux 系统时间, date -s 2020-01-07  date -s 16:43:25
*/

static inline void usage(const char *app)
{
    ak_print_normal("Usage: %s [options]\n\n"
        "options:\n"
        "1 rtc read \n"
        "2 linux system time sync from rtc. \n"
        "3 linux system timer sync to rtc. \n"
        "4 rtc alarm (单独休眠时间测试). \n"
        "    eg.1: ./ak_rtc_drv_demo 4 10 5 0        \
            - 4表示alarm定时，10表示定时10秒 5表示循环5次 0表示不死循环 \n"
        "5 rtc alarm (休眠+唤醒工作的整体时间测试). \n"
        "    eg.1: ./ak_rtc_drv_demo 5 10 5 0        \
            - 5表示alarm定时，10表示定时10秒 5表示循环5次 0表示不死循环 \n"
        "6 rtc time (单独休眠时间测试)\n"
        "  eg.2: ./ak_rtc_drv_demo 6 1000 5 0   表示单独休眠时间为1s\
             - 6表示time定时，1000表示定时1000ms maximum of 7999! 5表示循环5次 0表示不死循环 \n"
        "7 rtc time (休眠+唤醒工作的整体时间测试). \n"
        "  eg.2: ./ak_rtc_drv_demo 7 1000 5 0   表示休眠+唤醒工作的整体时间为1s \
             - 7表示time定时，1000表示定时1000ms maximum of 7999! 5表示循环5次 0表示不死循环 \n"
        "8 rtc alarm (单独mem时间测试)\n"
        "  eg.2: ./ak_rtc_drv_demo 8 10 5 0   表示单独mem时间为10s\
             - 8表示mem使用alarm定时，10表示定时10秒休眠 5表示循环5次 0表示不死循环 \n"
        "-h   help\n",app); 
}

/*
 * @BRIEF       ak_rtc_read
 * @PARAM[in]: void
 * @RETURN: void
 * @RETVAL:
 */
static inline void ak_rtc_read(void)
{
    char cmd_buf[200] = {0,}; 
    
    memset(cmd_buf, 0, sizeof(cmd_buf));

    sprintf(cmd_buf, "hwclock -r");

    printf("%s, line:%d, %s\n", __func__, __LINE__, cmd_buf);
    system(cmd_buf);
    
}

/*
 * @BRIEF       ak_sync_to_rtc
 * @PARAM[in]: void
 * @RETURN: void
 * @RETVAL:
 */
static inline void ak_sync_to_rtc(void)
{
    char cmd_buf[200] = {0,};
    
    sprintf(cmd_buf, "hwclock -w");

    printf("%s, line:%d, %s\n", __func__, __LINE__, cmd_buf);
    system(cmd_buf);
    
}

/*
 * @BRIEF       ak_sync_from_rtc
 * @PARAM[in]: void
 * @RETURN: void
 * @RETVAL:
 */
static inline void ak_sync_from_rtc(void)
{
    char cmd_buf[200] = {0,}; 
    
    memset(cmd_buf, 0, sizeof(cmd_buf));
    
    sprintf(cmd_buf, "hwclock -s " );
    
    printf("%s, line:%d, %s\n", __func__, __LINE__, cmd_buf);
    system(cmd_buf);
    
}

/**
 * test alarm wakeup

 * 
 * eg.1 : ./ak_rtc_drv_demo 4 10  10  0  - test ones
 * eg.2 : ./ak_rtc_drv_demo 4 10   0   1   - loop test
 */
 
 
static inline void ak_alarm_wakeup(int sec,int number, int loop)
{
    uint32_t count = 1;
    int i;
    char buffer[128];
    int status = 0;
    // set system time
    system("date -s \"2024-1-1 00:00:00\"");
    
    //设置时区
    setenv( "TZ", "CST-08", 1 );
    
    // sync to hardware
    system("hwclock -w");
    system("echo 1 > /sys/devices/platform/rtc/ak_rtc/standby_alarm_start ");  
    sprintf(buffer, "rtcwake -m standby -t $(($(date +%%s)+%d))",sec);

    /*死循环测试*/
    if(loop){
        do {
            /* entry standby*/
            status = system(buffer);
            if (status == -1) {
                perror("system error");
                exit(0);
            }
            printf("rtc alarm standby count: %u\n", count++);
        }while(1);

     /*循环设置的次数*/
    }else{
        for(i=0; i<number; i++){
            /* entry standby*/
            status = system(buffer);
            if (status == -1) {
                perror("system error");
                exit(0);
            }
            printf("rtc alarm standby count: %u\n", count++);
        }
    }

}


static inline void ak_alarm_mem_wakeup(int sec,int number, int loop)
{
    uint32_t count = 1;
    int i;
    char buffer[128];
    int status = 0;
    // set system time
    system("date -s \"2024-1-1 00:00:00\"");
    
    //设置时区
    setenv( "TZ", "CST-08", 1 );
    
    // sync to hardware
    system("hwclock -w");
    system("echo 1 > /sys/devices/platform/rtc/ak_rtc/standby_alarm_start ");  
    sprintf(buffer, "rtcwake -m mem -t $(($(date +%%s)+%d))",sec);

    /*死循环测试*/
    if(loop){
        do {
            /* entry mem*/
            status = system(buffer);
            if (status == -1) {
                perror("system error");
                exit(0);
            }
            printf("rtc alarm mem count: %u\n", count++);
        }while(1);

     /*循环设置的次数*/
    }else{
        for(i=0; i<number; i++){
            /* entry mem*/
            status = system(buffer);
            if (status == -1) {
                perror("system error");
                exit(0);
            }
            printf("rtc alarm mem count: %u\n", count++);
        }
    }

}

static inline void ak_alarm_wakeup2(int sec,int number, int loop)
{
    uint32_t count = 1;
    int i;
    char buffer[128];
    struct timeval start_tv = {0};
    struct timeval end_tv = {0};
    long int diff_time = 0;
    int status = 0;
    // set system time
    system("date -s \"2024-1-1 00:00:00\"");
    
    //设置时区
    setenv( "TZ", "CST-08", 1 );
    
    // sync to hardware
    system("hwclock -w");

   
    system("echo 0 > /sys/devices/platform/rtc/ak_rtc/standby_alarm_start ");  
    sprintf(buffer, "rtcwake -m no -t $(($(date +%%s)+%d))",sec);
    /*死循环测试*/
    if(loop){
        do {
             system(buffer);
            /* entry standby*/
             printf("模拟工作2s :sleep(2) \n");
             sleep(2);
             gettimeofday(&start_tv, NULL);
             status = system("echo standby > /sys/power/state");
             if (status == -1) {
                perror("system error");
                exit(0);
             }
             gettimeofday(&end_tv, NULL);
             diff_time = (end_tv.tv_sec - start_tv.tv_sec) * 1000 + (end_tv.tv_usec -start_tv.tv_usec)/1000;
             printf("alarm standby time :%ld ms \n",diff_time);
            printf("rtc alarm standby count: %u\n", count++);
        }while(1);

     /*循环设置的次数*/
    }else{
        for(i=0; i<number; i++){
             system(buffer);
            /* entry standby*/
             printf("模拟工作2s :sleep(2) \n");
             sleep(2);
             gettimeofday(&start_tv, NULL);
             status = system("echo standby > /sys/power/state");
             if (status == -1) {
                perror("system error");
                exit(0);
             }
             gettimeofday(&end_tv, NULL);
             diff_time = (end_tv.tv_sec - start_tv.tv_sec) * 1000 + (end_tv.tv_usec -start_tv.tv_usec)/1000;
             printf("alarm standby time :%ld ms \n",diff_time);
             
            printf("rtc alarm standby count: %u\n", count++);
        }
    }
	system("echo 1 > /sys/devices/platform/rtc/ak_rtc/standby_alarm_start ");  

}


/**
 * test timer wakeup
 * @param n (ms)
 * @param loop 0:test ones    1:loop test
 * @return void
 * 
 * eg.2 : ./ak_rtc_drv_demo 5 1000 5 1    - loop test (after 1000ms)
 */

static inline void ak_timer_wakeup(int ms,int number, int loop)
{
    int fd;
    int i;
    uint32_t count = 1;
    struct timeval start_tv = {0};
    struct timeval end_tv = {0};
    long int diff_time = 0;
    int status = 0;
    //struct timespec tpstart;
    //struct timespec tpend;

    
    /*打开rtc设备*/
    fd = open(PROC_DEV_NODE, O_WRONLY);
    if(fd==-1) {
        perror(PROC_DEV_NODE);
        return;
    }
    
    /*设置计时唤醒时间*/
    if(ioctl(fd, RTC_TIMER_WK_SET, &ms)<0) {
        perror(PROC_DEV_NODE);
    } 
 
    /*rtc校准*/
   if(ioctl(fd, INTENAL_RTC_ADJUST)<0) {
        perror(PROC_DEV_NODE);
    } 
    /*
    判断rtc校准有没完成,因为校准耗时，
    在判断其校准完成前可以做其他非rtc操作
    */
    /*400ms模拟非rtc操作，利用这个校准耗时，做其他非rtc操作*/
    usleep(400000);//400ms
    
   gettimeofday(&start_tv, NULL);
    /*判断rtc校准有没完成*/
   if(ioctl(fd, INTENAL_RTC_ADJUST_READ)<0) {
        perror(PROC_DEV_NODE);
    } 
   gettimeofday(&end_tv, NULL);
   diff_time = (end_tv.tv_sec - start_tv.tv_sec) * 1000 + (end_tv.tv_usec -start_tv.tv_usec)/1000;
   printf("ADJUST time :%ld ms \n",diff_time);
  
    /* standby_time_start模式打开 ，意思是在休眠时候开始使能time,
     *目的是在休眠时使能rtc 计时，唤醒时disable rtc，保证休眠时间为1s
     */
    system("echo 1 > /sys/devices/platform/rtc/ak_rtc/standby_time_start ");  

    if(loop){
        do {
            /*entry standby*/
            gettimeofday(&start_tv, NULL);
            status = system("echo standby > /sys/power/state");
            if (status == -1) {
               perror("system error");
               exit(0);
            }
            gettimeofday(&end_tv, NULL);

            diff_time = (end_tv.tv_sec - start_tv.tv_sec) * 1000 + (end_tv.tv_usec -start_tv.tv_usec)/1000;
            printf("standby time :%ld ms \n",diff_time);
            printf("standby count: %u\n", count++);
        }while(1);

     /*循环设置的次数*/
    }else{
        for(i=0; i<number; i++){
           
            /* entry standby*/
            gettimeofday(&start_tv, NULL);
            //clock_gettime(CLOCK_REALTIME, &tpstart);
            status = system("echo standby > /sys/power/state");
            if (status == -1) {
               perror("system error");
               exit(0);
            }
            // clock_gettime(CLOCK_REALTIME, &tpend);
            gettimeofday(&end_tv, NULL);

            diff_time = (end_tv.tv_sec - start_tv.tv_sec) * 1000 + (end_tv.tv_usec -start_tv.tv_usec)/1000;
            printf("standby time :%ld ms \n",diff_time);
            printf("standby count: %u\n", count++);
            /*
            sleep_time =  ((unsigned long)tpend.tv_sec*1000+(unsigned long)(tpend.tv_nsec/1000000)) -
            ((unsigned long)tpstart.tv_sec*1000+(unsigned long)(tpstart.tv_nsec/1000000));
            printf("clock_gettime sleep :%ld ms \n",sleep_time);
            */  
        }
    }
      /*关闭休眠时候使能rtc time*/
    system("echo 0 > /sys/devices/platform/rtc/ak_rtc/standby_time_start ");
    close(fd);
}



static inline void ak_timer_wakeup2(int ms,int number, int loop)
{
    int fd;
    int i;
    uint32_t count = 1;
    struct timeval start_tv = {0};
    struct timeval end_tv = {0};
    long int diff_time = 0;
    int status = 0;
    
    /*打开rtc设备*/
    fd = open(PROC_DEV_NODE, O_WRONLY);
    if(fd==-1) {
        perror(PROC_DEV_NODE);
        return;
    }
    
    /*设置休眠唤醒时间*/
    if(ioctl(fd, RTC_TIMER_WK_SET, &ms)<0) {
        perror(PROC_DEV_NODE);
    } 

    /*rtc校准*/
   if(ioctl(fd, INTENAL_RTC_ADJUST)<0) {
        perror(PROC_DEV_NODE);
    } 
    
    /*
    判断rtc校准有没完成,因为校准耗时，
    在判断其校准完成前可以做其他非rtc操作
    */
    /*400ms模拟非rtc操作，利用这个校准耗时，做其他非rtc操作*/
    usleep(400000);//400ms
 
   gettimeofday(&start_tv, NULL);
   if(ioctl(fd, INTENAL_RTC_ADJUST_READ)<0) {
        perror(PROC_DEV_NODE);
    } 
   gettimeofday(&end_tv, NULL);
   diff_time = (end_tv.tv_sec - start_tv.tv_sec) * 1000 + (end_tv.tv_usec -start_tv.tv_usec)/1000;
   printf("ADJUST time :%ld ms \n",diff_time);
 
   /*关闭休眠时候使能rtc time*/
    system("echo 0 > /sys/devices/platform/rtc/ak_rtc/standby_time_start ");
    
   /*使能定时器*/
    if(ioctl(fd, RTC_TIMER_WK_START)<0) {
        perror(PROC_DEV_NODE);
    } 
   
    /*死循环测试*/
    if(loop){
        do {
            printf("模拟工作500ms :usleep(500000) \n");
            usleep(500000);//500ms
            /*entry standby*/
            gettimeofday(&start_tv, NULL);
            status = system("echo standby > /sys/power/state");
            if (status == -1) {
               perror("system error");
               exit(0);
            }
            gettimeofday(&end_tv, NULL);
            diff_time = (end_tv.tv_sec - start_tv.tv_sec) * 1000 + (end_tv.tv_usec -start_tv.tv_usec)/1000;
            printf("standby time :%ld ms \n",diff_time);
            printf("standby count: %u\n", count++);
        }while(1);

     /*循环设置的次数*/
    }else{
        for(i=0; i<number; i++){
           printf("模拟工作500ms :usleep(500000) \n");
           usleep(500000);//500ms
            
           /*entry standby*/
           gettimeofday(&start_tv, NULL);
           status = system("echo standby > /sys/power/state");
           if (status == -1) {
              perror("system error");
              exit(0);
           }
           gettimeofday(&end_tv, NULL);
           diff_time = (end_tv.tv_sec - start_tv.tv_sec) * 1000 + (end_tv.tv_usec -start_tv.tv_usec)/1000;
           printf("standby time :%ld ms \n",diff_time);
           printf("standby count: %u\n", count++);
            
        }
    }
    
     /*停止使能rtc time*/
     if(ioctl(fd, RTC_TIMER_WK_STOP)<0) {
        perror(PROC_DEV_NODE);
     } 
     
     /*关闭设备*/
     close(fd);

}


/*
static void usage(const char *app)
{


}
*/

/*
* @BRIEF        app entry
* @AUTHOR       zhangzhipeng
* @DATE date    2019-12-20
* @PARAM[in]    argc:input command line parameter count
* @PARAM[in]    argc:input **argv:command line parameter address pointer
* @RETURN       int return success or fail
* @RETVAL       -1:fail, 0:success
*/
int main (int argc, char **argv)
{
    int ms = 1000;
    int sec = 10;
    int number = 10;
    int loop = 0;
    ak_print_normal( "rtc test start.\n");
    system("date -s \"2021-1-1 12:00:00\"");
    system("hwclock -w");
    if (argc < 2){
        usage(argv[0]);     
        return -1;
    }

    switch(*argv[1]){
        case '1': 
            ak_rtc_read(); 
            break;
        case '2': 
            ak_sync_from_rtc(); 
            break;
        case '3': 
            ak_sync_to_rtc(); 
            break;
        case '4':
            if(argc>=5){
                sscanf(argv[2], "%d", &sec);
                sscanf(argv[3], "%d", &number);
                sscanf(argv[4], "%d", &loop);
                ak_alarm_wakeup(sec,number,loop);
            }
            break;
        case '5':
            if(argc>=5){
                sscanf(argv[2], "%d", &sec);
                sscanf(argv[3], "%d", &number);
                sscanf(argv[4], "%d", &loop);
                ak_alarm_wakeup2(sec,number,loop);
            }
            break;
        case '6':
            if(argc>=5){
                sscanf(argv[2], "%d", &ms);
                sscanf(argv[3], "%d", &number);
                sscanf(argv[4], "%d", &loop);
                ak_timer_wakeup(ms, number,loop);
            }
            break;
        case '7':
            if(argc>=5){
                sscanf(argv[2], "%d", &ms);
                sscanf(argv[3], "%d", &number);
                sscanf(argv[4], "%d", &loop);
                ak_timer_wakeup2(ms, number,loop);
            }
            break;
            
        case '8':
            if(argc>=5){
                sscanf(argv[2], "%d", &ms);
                sscanf(argv[3], "%d", &number);
                sscanf(argv[4], "%d", &loop);
                ak_alarm_mem_wakeup(ms, number,loop);
            }
            break;
       case 'h':
       default:
           usage("rtc demo");

    }

    ak_print_normal( "rtc test end.\n");
    return 0;
}
