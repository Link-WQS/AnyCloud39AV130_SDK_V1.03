#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <fcntl.h>
#include <getopt.h>
#include <unistd.h>
#include <errno.h>
#include "ak_common.h"
#include "ak_log.h"
#include "ak_common_video.h"
#include "ak_venc.h"
#include "ak_thread.h"
#include "ak_mem.h"

extern int video_entry(int argc, char **argv);
extern int video_exit(void);
extern void video_usage(const char *app);

//#define WEBCAM_AUDIO

#ifdef WEBCAM_AUDIO
extern void audio_entry(void);
extern int audio_exit(void);
#endif

/*
 * @BRIEF        Usage
 * @DATE date    2019-12-20
 * @PARAM[in]    *app:app pointer
 * @RETURN       void
 * @RETVAL       none
 */
//static inline void usage(const char *app)
//{
//    printf("usage: %s [mode] [uvc device]\n", app);
//    printf("\t\t<mode>\t0 - ISO UVC, 1 - BULK UVC\n");
//    printf("\t\t<uvc device>\tUVC device file path\n");
//}

static inline void usage(const char *app)
{
    printf("usage: %s [-v] [-a]\n", app);
    printf("-v : uvc enable\n");
    printf("-a : uac enable\n\n");
//    printf("\t\t<mode>\t0 - ISO UVC, 1 - BULK UVC\n");
//    printf("\t\t<uvc device>\tUVC device file path\n");
}

static int uac_enable = 0, uvc_enable = 0;

/*
 * start of webcam
 */
int main(int argc, char **argv)
{
    /* init sdk running */
    sdk_run_config config;
    int ret = 0;
    int i;

    if (argc < 2) {
        usage(argv[0]);
        video_usage(argv[0]);
        return 1;
    }

    for (i = 0; i < argc; ++i) {
        if ((argv[i][0] == '-') && (argv[i][1] == 'v')) {
            uvc_enable = 1;
        }
        else if ((argv[i][0] == '-') && (argv[i][1] == 'a')) {
            uac_enable = 1;
        }
    }

    memset(&config, 0, sizeof(config));             //init the struct
    config.mem_trace_flag = SDK_RUN_NORMAL;
    config.isp_tool_server_flag = 0;                //isp tool sever
    // sdk init
    ak_sdk_init( &config );

    #ifdef WEBCAM_AUDIO
    if (uac_enable) audio_entry();
    #endif

    // video startup
    if (uvc_enable) video_entry(argc, argv);

    #ifdef WEBCAM_AUDIO
    if (uac_enable) audio_exit();
    #endif

    // video exit
    if (uvc_enable) video_exit();
    // sdk exit
    ak_sdk_exit();
    return ret;
}
/* end of func */
