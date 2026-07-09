#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <math.h>

#include "ak_mt_math.h"
#include "ak_common.h"
#include "ak_log.h"
#include "ak_mem.h"
#include "ak_thread.h"

#define LEN_HINT                512
#define LEN_OPTION_SHORT        512

double pan = -1; 		//[0,360) 单位:度 左减右加
double tilt = -100;		//[-90,90] -90 ~ 90,单位:度 0位置为水平,垂直向下为-90; 垂直向上为90; 原则:下减上加
double angle_h = -1;	//(0,180)	摄像头水平视场角
double angle_v = -1;	//(0,180)	摄像头垂直视场角
double video_u = -1;	//[0,1]		画面水平坐标，归一化到1
double video_v = -1;	//[0,1]		画面垂直坐标，归一化到1
static char *pc_prog_name = NULL;

static char ac_option_hint[  ][ LEN_HINT ] = {                                         //操作提示数组
    "help info" ,
    "[pan] ptz pan" ,
    "[tilt] ptz tilt" ,
    "[angle_h] sensor horizontal angle" ,
    "[angle_v] sensor vertical angle" ,
    "[video_u] video x pos (0-1)" ,
    "[video_v] video y pos (0-1)" ,
};

static struct option option_long[ ] = {
    { "help"          , no_argument       , NULL , 'h' } ,    //"help info"
    { "pan"           , required_argument , NULL , 'p' } ,    //
    { "tilt"          , required_argument , NULL , 't' } ,    //
    { "angle_h"       , required_argument , NULL , 'H' } ,    //
    { "angle_v"       , required_argument , NULL , 'V' } ,    //
    { "video_u"       , required_argument , NULL , 'x' } ,    //
    { "video_v"       , required_argument , NULL , 'y' } ,    //
    {0, 0, 0, 0}
};

static void usage(const char * name)
{
    ak_print_normal(MODULE_ID_APP, "Usage: %s -p pan -t tilt -H angle_h -V angle_v -x video_u -y video_v \n", name);
    ak_print_notice(MODULE_ID_APP, "e.g: %s -p 10.0 -t 20.1 -H 80 -V 45 -x 0.6 -y 0.7 \n", name);
}


static int help_hint(char *pc_prog_name)
{
    int i;

    printf("%s\n" , pc_prog_name);
    for(i = 0; i < sizeof(option_long) / sizeof(struct option); i ++) {
        if( option_long[ i ].val != 0 ) {
            printf("\t--%-16s -%c %s\n" , option_long[ i ].name , option_long[ i ].val , ac_option_hint[ i ]);
        }
    }

    usage(pc_prog_name);

    printf("\n\n");
    return 0;
}


static char *get_option_short( struct option *p_option, int i_num_option, char *pc_option_short, int i_len_option )
{
    int i;
    int i_offset = 0;
    char c_option;

    for( i = 0 ; i < i_num_option ; i ++ ) {
        if( ( c_option = p_option[ i ].val ) == 0 ) {
            continue;
        }
        switch( p_option[ i ].has_arg ){
        case no_argument:
            i_offset += snprintf( pc_option_short + i_offset , i_len_option - i_offset , "%c" , c_option );
            break;
        case required_argument:
            i_offset += snprintf( pc_option_short + i_offset , i_len_option - i_offset , "%c:" , c_option );
            break;
        case optional_argument:
            i_offset += snprintf( pc_option_short + i_offset , i_len_option - i_offset , "%c::" , c_option );
            break;
        }
    }
    return pc_option_short;
}

static int parse_option( int argc, char **argv )
{
    int i_option;
    char ac_option_short[ LEN_OPTION_SHORT ];
    int i_array_num = sizeof( option_long ) / sizeof( struct option ) ;
    char c_flag = 1;
    pc_prog_name = argv[ 0 ];

    get_option_short( option_long, i_array_num , ac_option_short , LEN_OPTION_SHORT );
    while((i_option = getopt_long(argc , argv , ac_option_short , option_long , NULL)) > 0) {
        switch(i_option) {
        case 'h' :  //help
            help_hint(argv[0]);
            c_flag = 0;
            goto parse_option_end;
        case 'p' :  //time
            pan = atof(optarg);
            break;
        case 't' :  //isp cfg path
            tilt = atof(optarg);
            break;
        case 'H' :  //save path
            angle_h = atof(optarg);
            break;
        case 'V' :  //time
            angle_v = atof(optarg);
            break;
        case 'x' :  //isp cfg path
            video_u = atof(optarg);
            break;
        case 'y' :  //save path
            video_v = atof(optarg);
            break;

        default :
            help_hint(argv[0]);
            c_flag = AK_FALSE;
            goto parse_option_end;
        }
    }
parse_option_end:
    return c_flag;
}



/**
 * 描述:此sample的功能为已知画面内任意一点A坐标的(u,v),
 *		通过ptz当前的状态参数(当前画面中心的pt值及摄像机的水平垂直视场角)，
 *		求出A点位置的pt值。
 * 		当前状态参数	:pan 、 tilt 、 angle_h 、 angle_v
 *		任意点A目标uv:	video_u 、video_v
 *		A目标的pt值		 :out.pan,out.tilt
 */

int main(int argc, char **argv)
{
    sdk_run_config config = {0};
    config.mem_trace_flag = SDK_RUN_NORMAL;
    ak_sdk_init( &config );

    if (0 == parse_option(argc, argv))
    {
        return 0;
    }

    ak_print_normal(MODULE_ID_APP, "mt math test start.\n");

    ak_mt_ptz_info in = {0};

    in.pan 	= (pan < 0) ? 100.0 : pan;
    in.tilt = (tilt < -90) ? 15.0 : tilt;

    in.angle_h = (angle_h < 0) ? 80.0 : angle_h;
    in.angle_v = (angle_v < 0) ? (in.angle_h * 9 / 16.0) : angle_v;	//v:h = 9:16

    double u = (video_u < 0) ? 0.7 : video_u;
    double v = (video_v < 0) ? 0.6 : video_v;

    printf("inparam: pan=%lf tilt=%lf angle_h=%lf angle_v=%lf video_u=%lf video_v=%lf\n",
        in.pan,in.tilt,in.angle_h,in.angle_v,u,v);
    ak_mt_ptz_info out = {0};
    ak_mt_math_uv_to_pt(&in, &out, u, v);

    printf("out.pan:%lf, out.tilt:%lf \n", out.pan, out.tilt);

exit:
    /*
     * step 4: release resource
     */
    ak_sdk_exit();
    ak_print_normal(MODULE_ID_APP, "exit mt sample\n");

    return 0;
}