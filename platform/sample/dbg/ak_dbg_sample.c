#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <signal.h>

#include "ak_common.h"
#include "ak_log.h"
#include "ak_dbg.h"

#ifdef DBG_BENCHMARK
#include "ak_timer.h"
#include "timecount.h"
#include "print_color_simple.h"
#endif

#ifdef AK_RTOS
#include <kernel.h>
#define __STATIC__  static
#else
#define __STATIC__
#endif

extern int ak_debug_enable(char on_off);

#ifdef DBG_BENCHMARK
char gc_run = true;

//用来测试json 批量插入和逐条插入的
//目前此函数不开启使用
void benchmark()
{
    //mark time
    TIME_PERF_START
#if 1
    ak_dbg_json( MODULE_ID_VENC, DBG_TYPE_PARAM, DBG_ACT_DROP, NULL );
#endif
#if 0
    /*
    {
         "vi0_id":    "0",
         "vi0_intf_mode":    "0",
         "vi0_crop":    "[0,0,2560,1440]",
         "vi0_max_res":    "[1920*1080]",
         "vi0_frame_rate":    "0",
         "vi0_sub_max_res":    "[640*360]",
         "chn0_id":    "0",
         "chn0_frame_rate":    "20",
         "chn0_res":    "[1920*1080]",
         "chn0_frame_depth":    "3",
         "chn0_data_type":    "0",
         "chn0_crop":    "[0]",
         "chn0_slice_num":    "[0]",
         "chn0_buf_len":    "[3110400]",
         "chn1_id":    "1",
         "chn1_frame_rate":    "20",
         "chn1_res":    "[640*360]",
         "chn1_frame_depth":    "3",
         "chn1_data_type":    "0",
         "chn1_crop":    "[0",
         "chn1_slice_num":    "[0]",
         "chn1_buf_len":    "[345600]"
    }
    */
    ak_dbg_json( MODULE_ID_VENC, DBG_TYPE_PARAM, DBG_ACT_ADD_OBJ, NULL,
                 "{ "
                 "\"vi0_id\":    \"0\",                 "
                 "\"vi0_intf_mode\":    \"0\",          "
                 "\"vi0_crop\":    \"[0,0,2560,1440]\", "
                 "\"vi0_max_res\":    \"[1920*1080]\",  "
                 "\"vi0_frame_rate\":    \"0\",         "
                 "\"vi0_sub_max_res\":    \"[640*360]\","
                 "\"chn0_id\":    \"0\",                "
                 "\"chn0_frame_rate\":    \"20\",       "
                 "\"chn0_res\":    \"[1920*1080]\",     "
                 "\"chn0_frame_depth\":    \"3\",       "
                 "\"chn0_data_type\":    \"0\",         "
                 "\"chn0_crop\":    \"[0]\",            "
                 "\"chn0_slice_num\":    \"[0]\",       "
                 "\"chn0_buf_len\":    \"[3110400]\",   "
                 "\"chn1_id\":    \"1\",                "
                 "\"chn1_frame_rate\":    \"20\",       "
                 "\"chn1_res\":    \"[640*360]\",       "
                 "\"chn1_frame_depth\":    \"3\",       "
                 "\"chn1_data_type\":    \"0\",         "
                 "\"chn1_crop\":    \"[0\",             "
                 "\"chn1_slice_num\":    \"[0]\",       "
                 "\"chn1_buf_len\":    \"[345600]\"     "
                 " } ");
#else
    //"vi0_id", "0"
    ak_dbg_json( MODULE_ID_VENC, DBG_TYPE_PARAM, DBG_ACT_ADD_STR,
                 "vi0_id", "0" );
    //"vi0_intf_mode", "0"
    ak_dbg_json( MODULE_ID_VENC, DBG_TYPE_PARAM, DBG_ACT_ADD_STR,
                 "vi0_intf_mode", "0" );
    //"vi0_crop", "[0,0,2560,1440]"
    ak_dbg_json( MODULE_ID_VENC, DBG_TYPE_PARAM, DBG_ACT_ADD_STR,
                 "vi0_crop", "[0,0,2560,1440]" );
    //"vi0_max_res", "[1920*1080]"
    ak_dbg_json( MODULE_ID_VENC, DBG_TYPE_PARAM, DBG_ACT_ADD_STR,
                 "vi0_max_res", "[1920*1080]" );
    //"vi0_frame_rate", "0"
    ak_dbg_json( MODULE_ID_VENC, DBG_TYPE_PARAM, DBG_ACT_ADD_STR,
                 "vi0_frame_rate", "0" );
    //"vi0_sub_max_res", "[640*360],"
    ak_dbg_json( MODULE_ID_VENC, DBG_TYPE_PARAM, DBG_ACT_ADD_STR,
                 "vi0_sub_max_res", "[640*360]," );
    //"chn0_id", "0"
    ak_dbg_json( MODULE_ID_VENC, DBG_TYPE_PARAM, DBG_ACT_ADD_STR,
                 "chn0_id", "0" );
    //"chn0_frame_rate", "20"
    ak_dbg_json( MODULE_ID_VENC, DBG_TYPE_PARAM, DBG_ACT_ADD_STR,
                 "chn0_frame_rate", "20" );
    //"chn0_res", "[1920*1080]"
    ak_dbg_json( MODULE_ID_VENC, DBG_TYPE_PARAM, DBG_ACT_ADD_STR,
                 "chn0_res", "[1920*1080]" );
    //"chn0_frame_depth", "3"
    ak_dbg_json( MODULE_ID_VENC, DBG_TYPE_PARAM, DBG_ACT_ADD_STR,
                 "chn0_frame_depth", "3" );
    //"chn0_data_type", "0"
    ak_dbg_json( MODULE_ID_VENC, DBG_TYPE_PARAM, DBG_ACT_ADD_STR,
                 "chn0_data_type", "0" );
    //"chn0_crop", "[0]"
    ak_dbg_json( MODULE_ID_VENC, DBG_TYPE_PARAM, DBG_ACT_ADD_STR,
                 "chn0_crop", "[0]" );
    //"chn0_slice_num", "[0]"
    ak_dbg_json( MODULE_ID_VENC, DBG_TYPE_PARAM, DBG_ACT_ADD_STR,
                 "chn0_slice_num", "[0]" );
    //"chn0_buf_len", "[3110400]"
    ak_dbg_json( MODULE_ID_VENC, DBG_TYPE_PARAM, DBG_ACT_ADD_STR,
                 "chn0_buf_len", "[3110400]" );
    //"chn1_id", "1"
    ak_dbg_json( MODULE_ID_VENC, DBG_TYPE_PARAM, DBG_ACT_ADD_STR,
                 "chn1_id", "1" );
    //"chn1_frame_rate", "20"
    ak_dbg_json( MODULE_ID_VENC, DBG_TYPE_PARAM, DBG_ACT_ADD_STR,
                 "chn1_frame_rate", "20" );
    //"chn1_res", "[640*360]"
    ak_dbg_json( MODULE_ID_VENC, DBG_TYPE_PARAM, DBG_ACT_ADD_STR,
                 "chn1_res", "[640*360]" );
    //"chn1_frame_depth", "3"
    ak_dbg_json( MODULE_ID_VENC, DBG_TYPE_PARAM, DBG_ACT_ADD_STR,
                 "chn1_frame_depth", "3" );
    //"chn1_data_type", "0"
    ak_dbg_json( MODULE_ID_VENC, DBG_TYPE_PARAM, DBG_ACT_ADD_STR,
                 "chn1_data_type", "0" );
    //"chn1_crop", "[0]"
    ak_dbg_json( MODULE_ID_VENC, DBG_TYPE_PARAM, DBG_ACT_ADD_STR,
                 "chn1_crop", "[0]" );
    //"chn1_slice_num", "[0]"
    ak_dbg_json( MODULE_ID_VENC, DBG_TYPE_PARAM, DBG_ACT_ADD_STR,
                 "chn1_slice_num", "[0]" );
    //"chn1_buf_len", "[345600]"
    ak_dbg_json( MODULE_ID_VENC, DBG_TYPE_PARAM, DBG_ACT_ADD_STR,
                 "chn1_buf_len", "[345600]" );
#endif
    TIME_PERF_END
    return ;
}
/* end of func */

/*
ctrl-c exit program
*/
void prog_exit( int i_sig )
{
    gc_run = false ;
    PRINTD( CM_NORMAL, CB_BLACK, CF_RED,
            "i_sig= %d gc_run= %d\n",i_sig, gc_run )
    return ;
}
/* end of func */
#endif

/*
print json value
*/
int print_status( )
{
    char ac_data[ 1024 ];
    //以不排序键值方式读取json内容
    ak_dbg_json( MODULE_ID_VENC, DBG_TYPE_PARAM, DBG_ACT_READ, AK_FALSE,
                 ac_data, sizeof( ac_data ) );
    ak_print_normal(MODULE_ID_APP, "JSON DATA:\n%s\n", ac_data );

    //以排序键值方式读取json内容
    ak_dbg_json( MODULE_ID_VENC, DBG_TYPE_PARAM, DBG_ACT_READ, AK_TRUE,
                 ac_data, sizeof( ac_data ) );
    ak_print_normal(MODULE_ID_APP, "JSON SORT:\n%s\n", ac_data );

    return 0;
}
/* end of func */

#ifdef AK_RTOS
static int dbg_sample(int argc, char **argv)
#else
int main(int argc, char **argv)
#endif
{
    sdk_run_config config = {0};
    config.mem_trace_flag = SDK_RUN_NORMAL;
    ak_sdk_init( &config );

    //打开debug_info
    ak_debug_enable(1);

    //进行benchmark测试的开关
#ifdef DBG_BENCHMARK
    signal( SIGINT, prog_exit ) ;
    signal( SIGTERM, prog_exit ) ;

    //初始化一个定时器
    struct timeval timeval_timer= { 0, 20000 };
    time_perf_init( 8 );
    ak_timer_t ak_timer;
    ak_timer_init( 0, 0 );
    ak_timer_create( &ak_timer, benchmark, NULL );
    ak_timer_start( ak_timer, &timeval_timer );

    //循环进行benchmark测试
    for( ;; ) {
        sleep( 1 );
        time_perf_print( NULL );
        time_perf_reset( );
        if( gc_run == false ) {
            break;
        }
    }
    //释放定时器
    ak_timer_stop( ak_timer );
    ak_timer_destroy( ak_timer );
    time_perf_exit( );
    return 0;
#endif
    //清除当前配置的所有信息
    ak_print_notice_ex(MODULE_ID_APP, "Clear all data.\n" );
    ak_dbg_json( MODULE_ID_VENC, DBG_TYPE_PARAM, DBG_ACT_DROP, NULL );
    print_status( );

    //用原来的接口添加三个键值
    ak_print_notice_ex(MODULE_ID_APP, "ak_debug_info() add 3 keys.\n" );
    ak_debug_info( MODULE_ID_VENC, DBG_TYPE_PARAM, "hello=tmp" );
    ak_debug_info( MODULE_ID_VENC, DBG_TYPE_PARAM, "hello1=tmp" );
    ak_debug_info( MODULE_ID_VENC, DBG_TYPE_PARAM,
                   "   hell___o2 = t333333mp   " );
    print_status( );

    //添加字符串 整形 浮点数键值
    ak_print_notice_ex(MODULE_ID_APP,
                   "ak_dbg_json( ) Add string|int|double .\n" );
    //添加字符串
    ak_dbg_json( MODULE_ID_VENC, DBG_TYPE_PARAM, DBG_ACT_ADD_STR,
                 "venc_stat", "open" );
    //添加整形
    ak_dbg_json( MODULE_ID_VENC, DBG_TYPE_PARAM, DBG_ACT_ADD_INT,
                 "venc_encode_num", 10 );
    //添加浮点数
    ak_dbg_json( MODULE_ID_VENC, DBG_TYPE_PARAM, DBG_ACT_ADD_DOUBLE,
                 "venc_success_percent", 20.22 );
    print_status( );

    //添加一个对象chn0的键值
    ak_print_notice_ex(MODULE_ID_APP, "ak_dbg_json( ) Add obj 'chn0'.\n" );
    ak_dbg_json( MODULE_ID_VENC, DBG_TYPE_PARAM, DBG_ACT_ADD_OBJ,
        "chn0", "{\"Width\": 1920, \"Height\": 1080,\"type\": \"h264\" }" );
    print_status( );

    //添加一个对象chn1的键值
    ak_print_notice_ex(MODULE_ID_APP, "ak_dbg_json( ) Add obj 'chn1'.\n" );
    ak_dbg_json( MODULE_ID_VENC, DBG_TYPE_PARAM, DBG_ACT_ADD_OBJ,
        "chn1", "{ \"Height\": 600, \"Width\": 800, \"type\": \"h264\" }" );
    print_status( );

    //覆盖对象chn0数据
    ak_print_notice_ex(MODULE_ID_APP, "ak_dbg_json( ) Replace obj 'chn0'.\n" );
    ak_dbg_json( MODULE_ID_VENC, DBG_TYPE_PARAM, DBG_ACT_ADD_OBJ,
                 "chn0", "{ \"Height\": 600, \"Width\": 800 }" );
    print_status( );

    //删除一个'venc_success_percent'键值
    ak_print_notice_ex(MODULE_ID_APP,
                 "ak_dbg_json( ) Drop key 'venc_success_percent'.\n" );
    ak_dbg_json( MODULE_ID_VENC, DBG_TYPE_PARAM, DBG_ACT_DROP,
                 "venc_success_percent" );
    print_status( );

    //删除chn0对象数据
    ak_print_notice_ex(MODULE_ID_APP, "ak_dbg_json( ) Drop obj 'chn0'.\n" );
    ak_dbg_json( MODULE_ID_VENC, DBG_TYPE_PARAM, DBG_ACT_DROP, "chn0" );
    print_status( );

    //清除全部数据
    ak_print_notice_ex(MODULE_ID_APP, "Clear all data.\n" );
    ak_dbg_json( MODULE_ID_VENC, DBG_TYPE_PARAM, DBG_ACT_DROP, NULL );
    print_status( );

    //根路径添加一个json
    ak_print_notice_ex(MODULE_ID_APP, "Replace all from obj.\n" );
    ak_dbg_json( MODULE_ID_VENC, DBG_TYPE_PARAM, DBG_ACT_ADD_OBJ, NULL,
        "{ \"number_1\" : 210, \"number_2\" : -210, \"number_3\" : 21.05,"
        " \"number_4\" : 1.0E+2, \"message\" : true, \"11ok\" : false, "
        "\"000tttt\" : null }" );
    print_status( );

    ak_print_notice_ex(MODULE_ID_APP, "----- %s exit -----\n", argv[0]);
    ak_sdk_exit();

    return 0;
}
/* end of func */


#ifdef AK_RTOS
SHELL_CMD_EXPORT_ALIAS(dbg_sample, ak_dbg_sample, dbg sample)
#endif

/* end of file */
