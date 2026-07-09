#ifndef _PRINT_COLOR_SIMPLE_H_
#define _PRINT_COLOR_SIMPLE_H_

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

#define PROG_TRUE                1
#define PROG_FALSE               0
#define PROG_SUCCESS             0
#define PROG_FAILURE            -1

#define DEBUG_PRINT( COLOR_MODE , COLOR_BACK , COLOR_FRONT , ... )  \
if( 1 ) { \
    printf( "\033[%d;%d;%dm[%d:%ld]%s:%d:%s( ) " , \
            COLOR_MODE , COLOR_BACK , COLOR_FRONT , \
            getpid(), ( long int )syscall( 224 ), __FILE__ , __LINE__ , __func__ ); \
    printf( __VA_ARGS__ ); \
    printf( "%s" , COLOR_END ) ; \
    fflush( stdout ); \
}

#define DEBUG_VAL( COLOR_MODE , COLOR_BACK , COLOR_FRONT , ... )  \
if( 1 ) { \
    printf( "\033[%d;%d;%dm" , \
            COLOR_MODE , COLOR_BACK , COLOR_FRONT ); \
    printf( __VA_ARGS__ ); \
    printf( "%s" , COLOR_END ) ; \
    fflush( stdout ); \
}

#ifndef FREE_POINT
#define FREE_POINT( POINT )  \
if( POINT != NULL ) {\
    free( POINT ) ;\
    POINT = NULL ;\
}
#endif

#endif
