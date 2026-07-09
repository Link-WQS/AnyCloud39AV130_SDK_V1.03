#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>

#ifndef __USE_POSIX199309
#define __USE_POSIX199309
#endif
#include <time.h>

#include "timecount.h"
#include "print_color_simple.h"
//用于性能统计的全局结构体指针
static struct timeperf *gp_timeperf = NULL;
 //结构体总数量,当前已经使用的结构体数量
static int gi_num_perf = 0, gi_perf_now = 0;
/*
    timeval_mark: 对一个timeval结构体时间赋予当前时间戳
    @p_timeval[IN]: 传入的struct timeval结构体指针
    return: 传入的结构体指针地址
*/
struct timeval *timeval_mark( struct timeval *p_timeval )
{
    gettimeofday( p_timeval , 0 ) ;
    return p_timeval ;
}

/*
    timeval_count: 对两个timeval之间的时间差进行统计
    @p_timeval_begin[IN]: 传入的struct timeval结构体指针,记录开始值
    @p_timeval_end[IN]: 传入的struct timeval,记录结束值
    return: 两个struct timeval的usec差值
*/
long long timeval_count( struct timeval *p_timeval_begin , struct timeval *p_timeval_end )
{
    long long i_time = ( p_timeval_end->tv_sec - p_timeval_begin->tv_sec ) * SEC2USEC
        + ( p_timeval_end->tv_usec - p_timeval_begin->tv_usec ) ;
    if ( i_time > 0 ) {
        return i_time ;
    }
    else {
        return 0 ;
    }
}

/*
    timespec_mark: 对一个timespec结构体时间赋予当前时间戳
    @p_timespec[IN]: 传入的struct timespec结构体指针
    return: 传入的结构体指针地址
*/
struct timespec *timespec_mark( struct timespec *p_timespec )
{
    clock_gettime( CLOCK_MONOTONIC , p_timespec ) ;
    return p_timespec ;
}

/*
    timespec_count: 对两个timespec之间的时间差进行统计
    @p_timespec_begin[IN]: 传入的struct timespec结构体指针,记录开始值
    @p_timespec_end[IN]: 传入的struct timespec,记录结束值
    return: 两个struct timespec的nsec差值
*/
long long timespec_count( struct timespec *p_timespec_begin , struct timespec *p_timespec_end )
{
    long long i_time = ( p_timespec_end->tv_sec - p_timespec_begin->tv_sec ) * ( long long )SEC2NSEC +
        ( ( long long )p_timespec_end->tv_nsec - p_timespec_begin->tv_nsec ) ;
    if ( i_time > 0 ) {
        return i_time ;
    }
    else {
        return 0 ;
    }
}

int time_perf_init( int i_num_perf )
{
    FREE_POINT( gp_timeperf );
    gp_timeperf = ( struct timeperf * )calloc( i_num_perf, sizeof( struct timeperf ) );
    gi_num_perf = i_num_perf;
    gi_perf_now = 0;

    return 0;
}

int time_perf_reset( void )
{
    int i;

    for( i = 0 ; i < gi_perf_now ; i ++ ) {
        gp_timeperf[ i ].i_us_min = 0 ;
        gp_timeperf[ i ].i_us_max = 0 ;
        gp_timeperf[ i ].i_us_total = 0 ;
        gp_timeperf[ i ].i_times = 0 ;
    }

    return 0;
}

/**
 * 启动性能计时器
 * 记录函数执行开始时间，用于性能测试和优化
 * @param _pi_num 指向计时器索引的指针，如果为负数则自动分配
 * @param pc_file 调用此函数的源文件名
 * @param i_line 调用此函数的行号
 * @param pc_func 调用此函数的函数名
 * @return 0表示成功，其他值表示失败
 */
int time_perf_start( int *pi_num, char *pc_file, unsigned int i_line, const char *pc_func )
{
    if ( ( gp_timeperf == NULL ) || ( *pi_num >= gi_num_perf ) ) {
        return 0;
    }
    if ( *pi_num < 0 ) {
        if ( gi_perf_now < gi_num_perf ) {
            *pi_num = gi_perf_now ;
            gi_perf_now ++ ;
        }
        else {
            return 0;
        }
    }

    if ( gp_timeperf[ *pi_num ].c_mark != PROG_TRUE ) {
        memcpy( gp_timeperf[ *pi_num ].ac_file, pc_file, strlen( pc_file ) );
        gp_timeperf[ *pi_num ].i_line = i_line;
        memcpy( gp_timeperf[ *pi_num ].ac_func, pc_func, strlen( pc_func ) );
        gp_timeperf[ *pi_num ].c_mark = PROG_TRUE;
    }
    timeval_mark( &gp_timeperf[ *pi_num ].timeval_start );
    return 0;
}

int time_perf_end( int i_num )
{
    unsigned long long i_us ;
    if ( ( gp_timeperf == NULL ) || ( i_num >= gi_num_perf ) || ( i_num < 0 ) ) {
        return 0;
    }
    timeval_mark( &gp_timeperf[ i_num ].timeval_end );
    i_us = ( unsigned long long )timeval_count( &gp_timeperf[ i_num ].timeval_start,
            &gp_timeperf[ i_num ].timeval_end );
    if ( ( i_us < gp_timeperf[ i_num ].i_us_min ) ||
         ( gp_timeperf[ i_num ].i_us_min == 0 ) ) {
        gp_timeperf[ i_num ].i_us_min = i_us ;
    }
    if ( ( i_us > gp_timeperf[ i_num ].i_us_max ) ||
         ( gp_timeperf[ i_num ].i_us_max == 0 ) ) {
        gp_timeperf[ i_num ].i_us_max = i_us ;
    }

    gp_timeperf[ i_num ].i_us_total += i_us ;
    gp_timeperf[ i_num ].i_times ++ ;
    return 0;
}

int time_perf_print( struct timeperf **pp_timeperf )
{
    int i;

    for( i = 0 ; i < gi_perf_now ; i ++ ) {
        DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_BLUE ,
                     "[ %d ] %s:%u:%s()\t TOTAL= %f ms TIMES= %u AVG= %f ms MIN= %f ms MAX= %f ms\n",
                     i, gp_timeperf[ i ].ac_file , gp_timeperf[ i ].i_line , gp_timeperf[ i ].ac_func,
                     ( double )gp_timeperf[ i ].i_us_total / 1000 , gp_timeperf[ i ].i_times,
                     ( double )gp_timeperf[ i ].i_us_total / gp_timeperf[ i ].i_times / 1000,
                     ( double )gp_timeperf[ i ].i_us_min / 1000,
                     ( double )gp_timeperf[ i ].i_us_max / 1000  ) ;
    }
    if ( pp_timeperf != NULL ) {
        *pp_timeperf = gp_timeperf ;
    }
    return gi_perf_now;
}

int time_perf_exit( void )
{
    FREE_POINT( gp_timeperf );
    gi_num_perf = 0;
    gi_perf_now = 0;
    return 0;
}