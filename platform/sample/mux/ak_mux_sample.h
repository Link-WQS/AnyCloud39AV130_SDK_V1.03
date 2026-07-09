#ifndef _AK_MUX_SAMPLE_H_
#define _AK_MUX_SAMPLE_H_

#define LEN_CMD             2048                                                //模板长度
#define LEN_FILE_UNION      2048
#define LEN_FILE            1024                                                //文件长度
#define LEN_TEMPLATE        512                                                 //模板长度
#define LEN_HINT            512                                                 //提示长度
#define LEN_NUM             16                                                  //整形数字长度
#define LEN_EXT             8                                                   //扩展名长度
#define LEN_OPTION_SHORT    512                                                 //短选项长度
#define LEN_BUFF_RECORD     ( 512 *1024 )                                       //录像缓存长度
#define LEN_BUFF_INDEX      0                                                   //索引缓存长度为0,目前需要立即写入,因为录像修复需要索引文件完整
#define LEN_BUFF_VIDEO      ( 2 * 1024 * 1024 )                                 //待合成的视频缓存长度,默认2048k
#define LEN_BUFF_AUDIO      ( 128 * 1024 )                                      //待合成的音频缓存长度,默认128k
#define LEN_CONF_RES        8192                                                //配置文件长度
#define LEN_PATTERN         1024
#define LEN_FIX_NAME        18                                                  //修复文件的文件名
#define HANDLE_NOT_INIT     -1
#define HANDLE_INIT         0

#define NUM_NOT_INIT        -1
#define RECORD_PATH_SAVE    "/mnt/dvr"
#define RECORD_PATH_TMP     "/mnt/tmp"
#define FILE_EXT_MP4        ".mp4"
#define FILE_EXT_AVI        ".avi"

#define AI_VOLUME           6
#define AENC_CHANNEL_NUM    1
#define AENC_SAMPLE_RATE    8000
#define VI_WIDTH            1920
#define VI_HEIGHT           1080
#define VI_SUB_WIDTH        640
#define VI_SUB_HEIGHT       360
#define VI_FPS              25
#define VI_FRAME_DEPTH      3                                                   //vi采集的frame buff 数量,设置为2可能会导致采集帧率不足
#define VI_FRAME_DEPTH_MIN  2
#define VI_FRAME_DEPTH_MAX  4

#define VENC_GOP_LEN        50
#define VENC_TARGET_KBPS    800
#define VENC_MAX_KBPS       1024
#define VENC_MINQP          25
#define VENC_MAXQP          50
#define VENC_INITQP         ( ( VENC_MINQP + VENC_MAXQP ) / 2 )
#define VENC_LEVEL          30
#define VENC_SMART_MODE     0
#define VENC_SMART_GOPLEN   100
#define VENC_SMART_QUALITY  50
#define RECORD_SEC          10                                                  //录像文件时长
#define RECORD_NUM          1                                                   //录像录制个数
#define MONITOR_SEC         10                                                  //监控检测线程的休眠时长
#define ISP_CONF_FILE       "/etc/config/isp.conf"
#define S2MS                1000
#define MS2US               1000
#define S2US                1000000
#define MS_CUT_ADD          100                                                 //在判断当前是否满足视频录制时长的时候加上该时长
#define TIMES_CUT_LOOP      10                                                  //在截断视频时候如果没有aenc数据的轮询次数
#define MIN_AUDIO_INTERVAL  32                                                  //音频最小间隔

#define PATH_TEMPLATE       "%Y%M%D"                                            //路径模板
#define FILE_TEMPLATE       "%Y%M%D.%h%m%s.%S"                                  //文件名模板
                                                                                //%Y - 年
                                                                                //%M - 月
                                                                                //%D - 日
                                                                                //%h - 时
                                                                                //%m - 分
                                                                                //%s - 秒
                                                                                //%S - 录制长度ms

enum dvr_template_type {                                                        //文件模板的保存元素序号
    FILE_TEMPLATE_YEAR,
    FILE_TEMPLATE_MONTH,
    FILE_TEMPLATE_DAY,
    FILE_TEMPLATE_HOUR,
    FILE_TEMPLATE_MIN,
    FILE_TEMPLATE_SEC,
    FILE_TEMPLATE_MS,
    FILE_TEMPLATE_NUM,
};

enum thread_serial {                                                            //录像线程的编号
    THREAD_VI_VENC,
    THREAD_AI,
    THREAD_AENC,
    THREAD_MUX,
    THREAD_NUM,
};

struct dvr_date {
    int year;                                                                   //4 number
    int mon;                                                                    //1-12
    int day;                                                                    //1-31
    int hour;                                                                   //0-23
    int min;                                                                    //0-59
    int sec;                                                                    //0-59
    int timezone;                                                               //local time zone 0-23

    struct timeval timeval_val;                                                 //tv_sec tv_usec
    struct timezone timezone_val;                                               //时区
    struct tm tm_val;                                                           //tm_year tm_mon tm_mday tm_hour tm_min tm_sec
};

struct venc_stream_entry                                                        //venc编码后的链表结构体
{
    struct video_stream stream;                                                 //venc编码后的数据
    struct dvr_date dvr_date_param;                                             //当前帧的编码时间
    struct list_head list;                                                      //链表指针
};

struct aenc_stream_entry
{
    struct audio_stream stream;
    struct list_head list;
};

struct frame_entry
{
    struct frame frame_stream;
    struct list_head list;
};


struct dvr_conf {
    int i_vi_dev;
    int i_vi_handle;
    int i_ai_handle;
    int i_venc_handle;
    int i_aenc_handle;
    int i_mux_handle;

    int i_vi_width ;
    int i_vi_height ;
    int i_vi_sub_width ;
    int i_vi_sub_height ;
    int i_vi_fps ;
    int i_vi_frame_depth;

    void *pv_aenc_handle;
    void *pv_aenc_stream_handle;

    struct ak_mux_input mux_input_param;
    struct ak_audio_in_param audio_in_param;
    struct venc_param venc_param ;

    int i_aenc_type ;                                                           //aenc_type与dvr_conf_param.mux_input_param.audio_type保持一致
    int i_ai_frame_len ;                                                        //音频帧长度
    int i_ai_frame_interval ;                                                   //音频帧间隔
    int i_ai_volume ;                                                           //ai音量设置
    char ac_vi_isp_cfg[ LEN_FILE ] ;                                            //isp配置文件

    int i_sec ;                                                                 //录像时长
    int i_run ;                                                                 //是否在运行
    pthread_t a_pthread_t[ THREAD_NUM ];                                        //录像的线程号
    char ac_path_save[ LEN_FILE ];                                              //录像路径
    char ac_path_tmp[ LEN_FILE ];                                               //在录录像的路径
    char ac_template_path[ LEN_FILE ];                                          //录像路径模板 - 支持%Y %M %D %h %m %s %S
    char ac_template_file[ LEN_FILE ];                                          //录像名模板 - 支持%Y %M %D %h %m %s %S
    struct dbdesc *p_dbdesc_param;                                              //数据库检索所用的结构体

    struct dvr_date dvr_date_param;                                             //当前录像的开始时间
    unsigned long long i_record_ms;                                             //录像的长度ms
    char ac_file_video[ LEN_FILE_UNION ];                                       //录制过程中暂存的录像文件名称
    char ac_file_idx[ LEN_FILE_UNION ];                                         //录制过程中暂存的录像文件索引
    pthread_mutex_t pthread_mutex_t_aenc ;                                      //aenc锁
    pthread_mutex_t pthread_mutex_t_venc ;                                      //venc锁
    pthread_mutex_t pthread_mutex_t_ai ;                                        //ai锁
    struct list_head list_head_aenc ;                                           //记录aenc的编码数据
    struct list_head list_head_venc ;                                           //记录venc的编码数据
    struct list_head list_head_ai ;                                             //记录ai的编码数据

    int i_buff_record;                                                          //录像暂存文件的缓存长度
    char *pc_buff_record;                                                       //录像缓存指针
    int i_buff_index;                                                           //录像索引文件的缓存长度
    char *pc_buff_index;                                                        //录像索引缓存指针

    int i_buff_video;                                                           //待合成的视频缓存长度,默认2048k,设置为0则不限制
    int i_buff_audio;                                                           //待合成的音频缓存长度,默认128k,设置为0则不限制

    int i_list_video;                                                           //当前列表的视频缓存长度
    int i_list_audio;                                                           //当前列表的音频缓存长度

    enum ak_mux_status mux_status;                                              //合成库运行状态
    char c_mux_init;                                                            //合成库是否初始化
    char c_mux_enable;                                                          //合成库是否可用
};

#ifndef FREE_POINT
#define FREE_POINT( POINT ) \
if( POINT != NULL ) {\
    free( POINT ) ;\
    POINT = NULL ;\
}
#endif

#define MUTEX_LOCK_AENC   pthread_mutex_lock( &p_dvr_conf->pthread_mutex_t_aenc )
#define MUTEX_UNLOCK_AENC pthread_mutex_unlock( &p_dvr_conf->pthread_mutex_t_aenc );

#define MUTEX_LOCK_VENC   pthread_mutex_lock( &p_dvr_conf->pthread_mutex_t_venc );
#define MUTEX_UNLOCK_VENC pthread_mutex_unlock( &p_dvr_conf->pthread_mutex_t_venc );

#define MUTEX_LOCK_AI     pthread_mutex_lock( &p_dvr_conf->pthread_mutex_t_ai );
#define MUTEX_UNLOCK_AI   pthread_mutex_unlock( &p_dvr_conf->pthread_mutex_t_ai );

#define HING_USAGE \
"For example:\n" \
"#MP3 H264 AVI\n" \
"%s --path_save ./dvr --record_sec 60 --ai_sample_rate 8000 --ai_volume 6 --vi_isp_cfg `ls /etc/config/isp*` --vi_width 1920 --vi_height 1080 --vi_fps 20 --venc_goplen 40 --aenc_type 2 --venc_type 0 --mux_type 0 --path_template '' --path_tmp ./tmp\n" \
"#AAC H265 AVI\n" \
"%s --path_save ./dvr --record_sec 60 --ai_sample_rate 8000 --ai_volume 6 --vi_isp_cfg `ls /etc/config/isp*` --vi_width 1920 --vi_height 1080 --vi_fps 20 --venc_goplen 40 --aenc_type 4 --venc_type 2 --mux_type 0 --path_template '' --path_tmp ./tmp\n" \
"#AMR H264 MP4\n" \
"%s --path_save ./dvr --record_sec 60 --ai_sample_rate 8000 --ai_volume 6 --vi_isp_cfg `ls /etc/config/isp*` --vi_width 1920 --vi_height 1080 --vi_fps 20 --venc_goplen 40 --aenc_type 3 --venc_type 0 --mux_type 1 --path_template '' --path_tmp ./tmp\n" \
"#g711a H265 MP4\n" \
"%s --path_save ./dvr --record_sec 60 --ai_sample_rate 8000 --ai_volume 6 --vi_isp_cfg `ls /etc/config/isp*` --vi_width 1920 --vi_height 1080 --vi_fps 20 --venc_goplen 40 --aenc_type 17 --venc_type 2 --mux_type 1 --path_template '' --path_tmp ./tmp\n"

#define COLOR_BEGIN              "\033[%d;%d;%dm"
#define COLOR_END                "\033[0m"

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

#ifndef AK_RTOS
#define PRINTD( COLOR_MODE , COLOR_BACK , COLOR_FRONT , ... )  \
if( 1 ) { \
    printf( "\033[%d;%d;%dm[%d:%ld]%s:%d:%s( ) " , \
            COLOR_MODE , COLOR_BACK , COLOR_FRONT , \
            getpid(), ( long int )syscall( 224 ), __FILE__ , __LINE__ , __func__ ); \
    printf( __VA_ARGS__ ); \
    printf( "%s" , COLOR_END ) ; \
    fflush( stdout ); \
}
#else

#define PRINTD( COLOR_MODE , COLOR_BACK , COLOR_FRONT , ... )  \
if( 1 ) { \
    printf( "\033[%d;%d;%dm%s:%d:%s( ) " , \
            COLOR_MODE , COLOR_BACK , COLOR_FRONT , \
            __FILE__ , __LINE__ , __func__ ); \
    printf( __VA_ARGS__ ); \
    printf( "%s" , COLOR_END ) ; \
    fflush( stdout ); \
}
#endif


static int dvr_init_ai( struct dvr_conf *p_dvr_conf );
static int dvr_init_vi( struct dvr_conf *p_dvr_conf );
static int dvr_init_aenc( struct dvr_conf *p_dvr_conf );
static int dvr_init_venc( struct dvr_conf *p_dvr_conf );
static int dvr_init_mux( struct dvr_conf *p_dvr_conf );
static void* thread_ai( void *pv_conf );
static void* thread_aenc( void *pv_conf );
static void* thread_vi_venc( void *pv_conf );
static void* thread_mux( void *pv_conf );
static int ak_dvr_init( struct dvr_conf *p_dvr_conf );
static int ak_dvr_start( struct dvr_conf *p_dvr_conf );
static int ak_dvr_stop( struct dvr_conf *p_dvr_conf );
static int ak_dvr_exit( struct dvr_conf *p_dvr_conf );

#endif
