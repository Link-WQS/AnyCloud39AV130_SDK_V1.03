#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "printcolor.h"

#define ROW_HEX_VIEW 16                                                         //每行输出16个字符
#define LEN_HEX_BUFF ( 11 + ROW_HEX_VIEW * 3 + 2 + ROW_HEX_VIEW + 2 )           //每行的buff大小

static pthread_mutex_t g_pthread_mutex_hex = PTHREAD_MUTEX_INITIALIZER;
/**
 * 以十六进制格式详细打印数据，支持彩色显示
 * 用于调试和数据分析，可以高亮显示特定模式的数据
 * @param i_color_mode 彩色显示模式（0=关闭彩色显示，1=开启彩色显示）
 * @param i_color_back 背景颜色代码（ANSI颜色代码，如40-47）
 * @param i_color_front 前景颜色代码（ANSI颜色代码，如30-37）
 * @param pc_data 指向要打印的二进制数据的指针
 * @param i_len 要打印的数据长度（字节数）
 * @return 0表示打印成功，负值表示打印失败
 */
int print_hex_detail( int i_color_mode , int i_color_back , int i_color_front, unsigned char *pc_data , int i_len )
{
    int i , i_now = 0 , i_buff ;
    char ac_buff[ LEN_HEX_BUFF ] ;                                              //用来输出一行数据的缓冲区

    pthread_mutex_lock( &g_pthread_mutex_hex ) ;
    for( ;; ) {
        if ( i_now >= i_len ) {
            break;
        }
        i_buff = 0 ;

        i_buff += snprintf( ac_buff + i_buff, LEN_HEX_BUFF - i_buff , "%08xh: ", i_now ) ;          //16进制地址
        for( i = i_now ; i < i_now + ROW_HEX_VIEW ; i ++ ) {                    //创建16进制内容
            if( i < i_len ) {
                i_buff += snprintf( ac_buff + i_buff, LEN_HEX_BUFF - i_buff , "%02x ", pc_data[ i ] ) ;
            }
            else {
                i_buff += snprintf( ac_buff + i_buff, LEN_HEX_BUFF - i_buff , "   " ) ;
            }
        }
        i_buff += snprintf( ac_buff + i_buff, LEN_HEX_BUFF - i_buff , "; " ) ;
        for( i = i_now ; i < i_now + ROW_HEX_VIEW ; i ++ ) {                    //打印ASCII可见字符
            if ( ( i < i_len ) && ( pc_data[ i ] >= 32 ) && ( pc_data[ i ] <= 126 ) ) {
                i_buff += snprintf( ac_buff + i_buff, LEN_HEX_BUFF - i_buff , "%c", pc_data[ i ] ) ;
            }
            else {
                i_buff += snprintf( ac_buff + i_buff, LEN_HEX_BUFF - i_buff , " " ) ;
            }
        }
        i_buff += snprintf( ac_buff + i_buff, LEN_HEX_BUFF - i_buff , "\n" ) ;
        DEBUG_VAL( i_color_mode, i_color_back, i_color_front, "%s", ac_buff ) ;
        i_now += ROW_HEX_VIEW ;
    }
    pthread_mutex_unlock( &g_pthread_mutex_hex ) ;
    return 0;
}