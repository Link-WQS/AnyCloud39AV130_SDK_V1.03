
//**************************
//*   print color start    *
//**************************
/*
字背景颜色范围: 40--49                   字颜色: 30--39

                40: 黑                           30: 黑
                41: 红                           31: 红
                42: 绿                           32: 绿
                43: 黄                           33: 黄
                44: 蓝                           34: 蓝
                45: 紫                           35: 紫
                46: 青                           36: 青
                47: 白色                         37: 白色

echo -e "\e[0;32;40m 0;32;40 \e[0m"
echo -e "\e[1;32;40m 1;32;40 \e[0m"
echo -e "\e[2;32;40m 2;32;40 \e[0m"
echo -e "\e[3;32;40m 3;32;40 \e[0m"
echo -e "\e[4;32;40m 4;32;40 \e[0m"
echo -e "\e[5;32;40m 5;32;40 \e[0m"
echo -e "\e[6;32;40m 6;32;40 \e[0m"
echo -e "\e[7;32;40m 7;32;40 \e[0m"
echo -e "\e[22;32;40m 22;32;40 \e[0m"
echo -e "\e[24;32;40m 24;32;40 \e[0m"
echo -e "\e[25;32;40m 25;32;40 \e[0m"
echo -e "\e[27;32;40m 27;32;40 \e[0m"
*/
#ifndef __PRINTCOLOR_H__
#define __PRINTCOLOR_H__
#define COLOR_LEN                32
#define COLOR_BEGIN              "\033[%d;%d;%dm"
#define COLOR_END                "\033[0m"

#define COLOR_MODE_NORMAL        0                                              //缺省模式
#define COLOR_MODE_BOLD          1                                              //高亮
#define COLOR_MODE_UNDERLINED    4                                              //下划线
#define COLOR_MODE_BLINK         5                                              //闪烁
#define COLOR_MODE_NEGATIVE      7                                              //反色

#define COLOR_BACK_BLACK         40
#define COLOR_BACK_RED           41
#define COLOR_BACK_GREEN         42
#define COLOR_BACK_YELLOW        43
#define COLOR_BACK_BLUE          44
#define COLOR_BACK_PURPLE        45
#define COLOR_BACK_CYAN          46
#define COLOR_BACK_WHITE         47

#define COLOR_FRONT_BLACK        30
#define COLOR_FRONT_RED          31
#define COLOR_FRONT_GREEN        32
#define COLOR_FRONT_YELLOW       33
#define COLOR_FRONT_BLUE         34
#define COLOR_FRONT_PURPLE       35
#define COLOR_FRONT_CYAN         36
#define COLOR_FRONT_WHITE        37


enum COLOR_MODE {                                                               //颜色模式
    CM_NORMAL     = 0 ,                                                         //缺省模式
    CM_BOLD       = 1 ,                                                         //高亮
    CM_UNDERLINED = 4 ,                                                         //下划线
    CM_BLINK      = 5 ,                                                         //闪烁
    CM_NEGATIVE   = 7 ,                                                         //反色
};

enum COLOR_BACKGROUND {                                                         //背景颜色
    CB_BLACK  = 40 ,
    CB_RED    = 41 ,
    CB_GREEN  = 42 ,
    CB_YELLOW = 43 ,
    CB_BLUE   = 44 ,
    CB_PURPLE = 45 ,
    CB_CYAN   = 46 ,
    CB_WHITE  = 47 ,
};

enum COLOR_FONT {                                                               //字体颜色
    CF_BLACK  = 30 ,
    CF_RED    = 31 ,
    CF_GREEN  = 32 ,
    CF_YELLOW = 33 ,
    CF_BLUE   = 34 ,
    CF_PURPLE = 35 ,
    CF_CYAN   = 36 ,
    CF_WHITE  = 37 ,
};

//**************************
//*   print color end    *
//**************************
#define LEN_DEBUG                131072
#define LEN_TIME                 64
typedef int pid_t ;
/**
 * 获取ANSI颜色代码字符串
 * 根据颜色模式和前后景色生成相应的ANSI转义序列
 * @param i_color_mode 彩色模式（0=禁用，1=启用）
 * @param i_color_back 背景颜色代码（ANSI 40-47）
 * @param i_color_front 前景颜色代码（ANSI 30-37）
 * @return 指向ANSI颜色代码字符串的指针
 */
char *get_color( int i_color_mode , int i_color_back , int i_color_front ) ;
/**
 * 获取当前线程ID
 * 系统调用封装，返回调用线程的唯一标识符
 * @return 当前线程的PID（线程ID）
 */
pid_t gettid( ) ;
/**
 * 获取当前系统时间字符串
 * 格式化当前时间并存储到指定缓冲区
 * @param pc_time 指向存储时间字符串的缓冲区指针
 * @return 0表示成功，负值表示失败
 */
char *get_time_now( char *pc_time ) ;

 /**
 * 获取程序名称
 * 返回当前运行程序的名称字符串
 * @return 指向程序名称字符串的指针
 */
char *get_progname( ) ;
/**
 * 设置程序名称
 * 设置全局程序名称变量
 * @param pc_name 指向程序名称字符串的指针
 * @return 0表示设置成功，负值表示设置失败
 */
char *set_progname( char *pc_name ) ;
/**
 * 获取全局互斥锁
 * 返回用于线程同步的互斥锁指针
 * @return 指向pthread_mutex_t互斥锁的指针
 */
pthread_mutex_t *get_mutex( );
/**
 * 线程安全的本地时间转换函数
 * 将时间戳转换为本地时间结构体（可重入版本）
 * @param timep 指向时间戳的指针
 * @param result 指向存储结果的tm结构体指针
 * @return 转换后的tm结构体指针
 */
struct tm *localtime_r(const time_t *timep, struct tm *result) ;
/**
 * 系统调用接口函数
 * 用于直接调用Linux系统调用
 * @param number 系统调用号
 * @param ... 可变参数，根据具体系统调用而定
 * @return 系统调用的返回值
 */
long syscall( long number , ... ) ;

enum VAL_TYPE {                                                               //字体颜色
    VAL_SUCCESS = 0 ,
    VAL_FAILURE = -1,
};

#ifndef FREE_POINT
#define FREE_POINT( POINT )  \
if( POINT != NULL ) {\
    free( POINT ) ;\
    POINT = NULL ;\
}
#endif

#define DEBUG_SET_MAINPROG_NAME set_progname( argv[ 0 ] );

#ifndef DEBUG_MUTEX
#define DEBUG_MUTEX 0
#endif

#ifndef DEBUG_OUTPUT
#define DEBUG_OUTPUT 1
#endif

#ifdef  DEBUG_NONE
#define DEBUG_SET_MAINPROG_NAME ;
#define DEBUG_PRINT( COLOR_MODE , COLOR_BACK , COLOR_FRONT , ... ) ;
#define DEBUG_VAL( COLOR_MODE , COLOR_BACK , COLOR_FRONT , ... ) ;
#else
#define DEBUG_PRINT( COLOR_MODE , COLOR_BACK , COLOR_FRONT , ... )  \
if( DEBUG_OUTPUT ) { \
    if( DEBUG_MUTEX ) { \
        pthread_mutex_lock( get_mutex() ) ;\
    } \
    printf( "\033[%d;%d;%dm[%s] [%d:%d] %s:%s:%d:%s( ) " , \
            COLOR_MODE , COLOR_BACK , COLOR_FRONT , \
            get_time_now( NULL ) , getpid( ) , gettid( ) , get_progname( ) , __FILE__ , __LINE__ , __func__ ); \
    printf( __VA_ARGS__ ); \
    printf( "%s" , COLOR_END ) ; \
    fflush( stdout ); \
    if( DEBUG_MUTEX ) { \
        pthread_mutex_unlock( get_mutex() ) ;\
    } \
}

#define DEBUG_VAL( COLOR_MODE , COLOR_BACK , COLOR_FRONT , ... )  \
if( DEBUG_OUTPUT ) { \
    if( DEBUG_MUTEX ) { \
        pthread_mutex_lock( get_mutex() ) ;\
    } \
    printf( "\033[%d;%d;%dm" , \
            COLOR_MODE , COLOR_BACK , COLOR_FRONT ); \
    printf( __VA_ARGS__ ); \
    printf( "%s" , COLOR_END ) ; \
    fflush( stdout ); \
    if( DEBUG_MUTEX ) { \
        pthread_mutex_unlock( get_mutex() ) ;\
    } \
}
#endif

#endif