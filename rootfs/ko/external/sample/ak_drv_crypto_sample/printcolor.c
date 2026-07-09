#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <signal.h>
#ifndef __USE_POSIX199309
#define __USE_POSIX199309
#endif
#include <time.h>

#include "printcolor.h"

#ifdef MEMVIEW
//#include "mempool.h"
#endif

static char gac_time[ LEN_TIME ] ;
static char *gpc_progname = NULL ;//, gc_run = true ;
static pthread_mutex_t g_pthread_mutex_print = PTHREAD_MUTEX_INITIALIZER;
/*
    get_color: 根据输入的颜色编码返回颜色打印的字符串
    @i_color_mode[IN]: 颜色模式
    @i_color_back[IN]: 背景颜色
    @i_color_front[IN]: 字体颜色
    return: 颜色打印字符串头部
*/
char *get_color( int i_color_mode , int i_color_back , int i_color_front )
{
    static char ac_color[ COLOR_LEN ] ;
    snprintf( ac_color , COLOR_LEN , COLOR_BEGIN , i_color_mode , i_color_back , i_color_front ) ;
    return ac_color ;
}

/*
    get_time_now: 获取当前时间的字符串
    @pc_time[IN]: 颜色模式
    return: 当前时间的字符串地址
*/
char *get_time_now( char *pc_time )
{
    static struct tm tm_now ;
    static struct timeval timeval_now ;
    char *pc_now = gac_time ;

    if ( pc_time != NULL ) {
        pc_now = pc_time ;
    }
    gettimeofday( &timeval_now , NULL ) ;
    localtime_r( &timeval_now.tv_sec , &tm_now ) ;
    snprintf( pc_now , LEN_TIME , "%04d-%02d-%02d %02d:%02d:%02d.%06u",
                       tm_now.tm_year + 1900 , tm_now.tm_mon + 1 , tm_now.tm_mday ,
                       tm_now.tm_hour, tm_now.tm_min , tm_now.tm_sec , ( unsigned int )timeval_now.tv_usec ) ;
    return pc_now ;
}

/*
    gettid: 获取线程id
    return: 当前的线程id号
*/
pid_t gettid()
{
    return syscall( SYS_gettid ) ;
}

/*
    signal_exit: 当前应用获取到信号后,将全局的运行标志置为否
    return: void
*/
/*
void signal_exit( int i_sig )
{
    gc_run = PROG_FALSE ;
    return ;
}
*/

/*
    signal_exit: 当前应用获取到信号后,将全局的运行标志置为否
    return: void
*/
/*
int if_signal_exit( void )
{
    return gc_run;
}
*/

char *get_progname( )
{
    return gpc_progname;
}

char *set_progname( char *pc_name )
{
    return gpc_progname = pc_name;
}

pthread_mutex_t *get_mutex( )
{
    return &g_pthread_mutex_print;
}