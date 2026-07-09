#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "ak_common.h"
#include "ak_demux.h"

#include "ak_thread.h"
#include "ak_log.h"
#include "ak_mem.h"
#include "ak_common_audio.h"
#include "ak_demux_sample.h"

#ifdef AK_RTOS
#include <kernel.h>
#endif

static FILE *gp_FILE_video = NULL;
static FILE *gp_FILE_audio = NULL;
static FILE *gp_FILE_demux = NULL;
static int gi_demux_handle = -1;
static char *gpc_demux_file = NULL ;    //demux的文件名称
static char *gpc_path = "./";            //demux的保存路径
static char gc_skip_serial = AK_FALSE;
static int gi_offset_ms = 0;

static char gac_option_hint[  ][ LEN_HINT ] = {
    "       打印帮助信息" ,
    "[PATH] 解析出来的音频文件、视频文件保存路径" ,
    "[FILE] 需要解析的文件全路径" ,
    "       视频解码文件用于媒体播放器播放时,写入文件时需要跳过序列号和时间戳" ,
    "       读文件时候的偏移值(ms)" ,
    "" ,
};
static struct ak_demux_video_info g_demux_video_info ;
static struct ak_demux_audio_info g_demux_audio_info ;

static char gac_audio_name[ ][ LEN_EXT ] = {
    "unknown"      ,
    "midi"         ,
    "mp3"          ,
    "amr"          ,
    "aac"          ,
    "wma"          ,
    "pcm.wav"      ,
    "adpcm_ima"    ,
    "adpcm_ms"     ,
    "adpcm_flash"  ,
    "ape"          ,
    "flac"         ,
    "ogg_flac"     ,
    "ra8lbr"       ,
    "dra"          ,
    "ogg_vorbis"   ,
    "ac3"          ,
    "pcm_alaw.wav" ,
    "pcm_ulaw.wav" ,
    "sbc"          ,
    "msbc"         ,
    "speex"        ,
    "speex_wb"     ,
};

static char gac_video_name[ ][ LEN_EXT ] = {
    "h264" ,
    "jpg"  ,
    "h265" ,
};

static struct option option_long[ ] = {
    { "help"        , no_argument       , NULL , 'h' } , //"       打印帮助信息" ,
    { "path"        , required_argument , NULL , 'p' } , //"[PATH] 解析出来的音频文件、视频文件保存路径" ,
    { "file"        , required_argument , NULL , 'f' } , //"[FILE] 需要解析的文件全路径" ,
    { "skip-serial" , no_argument       , NULL , 's' } , //"       视频解码文件用于媒体播放器播放时,写入文件时需要跳过序列号和时间戳" ,
    { "offset-ms"   , required_argument , NULL , 'x' } , //"       读文件时候的偏移值(ms)" ,
    {0              , 0                 , 0    , 0   }   //"" ,
};

/**
 * help_hint -
 *
 * notes:
 */
static int help_hint( char *pc_prog_name )
{
    int i;

    printf( "%s\n" , pc_prog_name);
    printf( "Version: \r\n\t%s \n\n", ak_demux_get_version());
    printf( "Usage: \r\n\t%s  [options] <value>\n\n", pc_prog_name);

    /* parse the all supported option */
    printf( "options: \r\n");
    for(i = 0; i < sizeof(option_long) / sizeof(struct option); i ++) {
        if( option_long[ i ].val != 0 ) {
            printf("\t-%c, --%-16s\t%s\n" , option_long[ i ].val , option_long[ i ].name , gac_option_hint[ i ]);
        }
    }
    printf("\nexample: \r\n\t%s -p /mnt/demux/ -f /mnt/mux.mp4 \n", pc_prog_name);
    printf("\n\n");
    return 0;
}

/**
 * get_option_short -
 *
 * notes:
 */
static char *get_option_short( struct option *p_option, int i_num_option, char *pc_option_short, int i_len_option )
{
    int i;
    int i_offset = 0;
    char c_option;

    for( i = 0 ; i < i_num_option ; i ++ ) {
        c_option = p_option[ i ].val;
        switch( p_option[ i ].has_arg ) {
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

/**
 * parse_option -
 *
 * notes:
 */
static int parse_option( int argc, char **argv )
{
    int i_option;
    char ac_option_short[ LEN_OPTION_SHORT ];
    int i_array_num = sizeof( option_long ) / sizeof( struct option ) ;
    char c_flag = 1;

    if (argc <= 1 ) {
        help_hint( argv[ 0 ] );
        c_flag = 0;
        goto parse_option_end;
    }

    optind = 0;

    get_option_short( option_long, i_array_num , ac_option_short , LEN_OPTION_SHORT );
    while((i_option = getopt_long(argc , argv , ac_option_short , option_long , NULL)) > 0) {
        switch(i_option) {
        case 'h' :                                                          //help
            help_hint( argv[ 0 ] );
            c_flag = 0;
            goto parse_option_end;
        case 'p' :                                                          //gpc_path
            gpc_path = optarg;
            break;
        case 'f' :                                                          //file
            gpc_demux_file = optarg;
            break;
        case 's' :                                                          //skip
            gc_skip_serial = AK_TRUE;
            break;
        case 'x' :                                                          //skip
            gi_offset_ms = atoi( optarg );
            if( gi_offset_ms < 0 ) {
                gi_offset_ms = 0;
            }
            break;
        default:
            help_hint( argv[ 0 ] );
            c_flag = 0;
            goto parse_option_end;
        }
    }
parse_option_end:
    return c_flag;
}

/**
 * thread_demux_data -
 *
 * notes:
 */
static void *thread_demux_data(void *arg)
{
    //char c_get_video = AK_FALSE;
    enum ak_demux_data_type type = DEMUX_DATA_UNKNOW;
    struct ak_demux_stream stream;
    memset(&stream, 0, sizeof(stream));
    ak_thread_set_name("demux_data");
    ak_print_notice_ex(MODULE_ID_APP, "************* START *************\n");
    unsigned long long ull_serial = 0;
    char ac_video_file[ LEN_FILENAME ] = {0};
    char ac_audio_file[ LEN_FILENAME ] = {0};
    audio_files audio_files_pcm = {0};
    uint32_t i_audio_len = 0 ;

    for( ;; ) {
        /* get demux data*/
        ak_demux_get_data(gi_demux_handle, &type, &stream);
        if (0 == stream.len) {
            break;
        }
        /* stream type */
        switch( type ) {
            /* video data */
            case DEMUX_DATA_VIDEO :
                ak_print_normal_ex( MODULE_ID_APP,
                                    "VIDEO stream.ts= %llu stream.frame_type= %d stream.len= %u\n" ,
                                    stream.ts, stream.frame_type, stream.len );
                if ( stream.len <= LEN_SERIAL ) {
                    ak_print_normal_ex(MODULE_ID_APP, "skip null video frame.\n" );
                    break;
                }
                /* video stream type */
                switch( g_demux_video_info.video_type ) {
                    case H264_ENC_TYPE :
                    case HEVC_ENC_TYPE :
                        if ( gp_FILE_video == NULL ) {
                            snprintf(ac_video_file, sizeof( ac_video_file ), "%s/demux_video.%s", gpc_path,
                                    gac_video_name[ g_demux_video_info.video_type ] );
                            gp_FILE_video = fopen(ac_video_file, "wb");
                        }
                        break;
                    case MJPEG_ENC_TYPE :
                        snprintf(ac_video_file, sizeof( ac_video_file ), "%s/demux_video_%08llu.jpg",
                                gpc_path, ull_serial );
                        ak_print_normal_ex(MODULE_ID_APP, "%s\n", ac_video_file );
                        gp_FILE_video = fopen(ac_video_file, "wb");
                        ull_serial ++ ;
                        break;
                }

                if ( g_demux_video_info.video_type  == MJPEG_ENC_TYPE ) {     //解码是图片文件的话则直接跳过8个字节输出文件
                    fwrite( stream.data + LEN_SERIAL, stream.len - LEN_SERIAL, 1, gp_FILE_video);
                    fclose( gp_FILE_video );
                    gp_FILE_video = NULL;
                }
                else {
                    if ( gc_skip_serial == AK_FALSE ) {                         //在视频解码文件中写入序列号和时间戳
                        fwrite(stream.data, stream.len, 1, gp_FILE_video);
                    }
                    else {                                                      //跳过写入序列号和时间戳
                        fwrite( stream.data + LEN_SERIAL, stream.len - LEN_SERIAL, 1, gp_FILE_video);
                    }
                }
                break;
            /* audio data */
            case DEMUX_DATA_AUDIO :
                ak_print_normal_ex( MODULE_ID_APP,
                                    "AUDIO stream.ts= %llu stream.frame_type= %d stream.len= %u\n" ,
                                    stream.ts, stream.frame_type, stream.len );
                if ( gp_FILE_audio == NULL ) {
                    snprintf( ac_audio_file, sizeof( ac_audio_file ), "%s/demux_audio.%s", gpc_path,
                            gac_audio_name[ g_demux_audio_info.audio_type ] );
                    gp_FILE_audio = fopen( ac_audio_file, "wb" );
                    if( gp_FILE_audio == NULL ) {
                        ak_print_error_ex(MODULE_ID_APP, "gp_FILE_audio= %p\n" , gp_FILE_audio );
                        break;
                    }
                    /* audio stream type */
                    switch( g_demux_audio_info.audio_type ) {                   //判断是否要写入音频文件头
                        case AK_AUDIO_TYPE_AMR :
                            fwrite( AMR_HEADER, strlen( AMR_HEADER ), 1, gp_FILE_audio);
                            break;
                        case AK_AUDIO_TYPE_PCM :
                        case AK_AUDIO_TYPE_PCM_ALAW :
                        case AK_AUDIO_TYPE_PCM_ULAW :
                            audio_files_pcm.samplerate = g_demux_audio_info.audio_data_attr.sample_rate;
                            audio_files_pcm.bits_per_sample = g_demux_audio_info.audio_data_attr.sample_bits;
                            audio_files_pcm.channels = g_demux_audio_info.audio_data_attr.channel_num;
                            write_wav_header( &audio_files_pcm, gp_FILE_audio );
                            break;
                        default :
                            break;
                    }
                }
                if( gp_FILE_audio != NULL ) {
                    switch( g_demux_audio_info.audio_type ) {                   //以下几种格式需要计算写入音频长度
                        case AK_AUDIO_TYPE_PCM :
                        case AK_AUDIO_TYPE_PCM_ALAW :
                        case AK_AUDIO_TYPE_PCM_ULAW :
                            i_audio_len += stream.len ;
                            break;
                        default :
                            break;
                    }
                    fwrite(stream.data, stream.len, 1, gp_FILE_audio);
                }
                break;
            default :
                break;
        }
        /* demux free data */
        ak_demux_free_data(gi_demux_handle, &stream);
    }
    switch( g_demux_audio_info.audio_type ) {                                   //判断是否要写入音频文件头
        case AK_AUDIO_TYPE_PCM :
            audio_files_pcm.fileType = _WAVE_LPCM ;
            break;
        case AK_AUDIO_TYPE_PCM_ALAW :
            audio_files_pcm.fileType = _WAVE_ALAW ;
            break;
        case AK_AUDIO_TYPE_PCM_ULAW :
            audio_files_pcm.fileType = _WAVE_ULAW ;
            break;
        default :
            break;
    }
    switch( g_demux_audio_info.audio_type ) {                                   //判断是否要写入音频文件头
        case AK_AUDIO_TYPE_PCM :
        case AK_AUDIO_TYPE_PCM_ALAW :
        case AK_AUDIO_TYPE_PCM_ULAW :
            audio_files_pcm.data_len = i_audio_len ;
            fseek( gp_FILE_audio , 0, SEEK_SET);                                //跳过头开始写入
            write_wav_header( &audio_files_pcm, gp_FILE_audio );
            break;
        default :
            break;
    }

    ak_print_notice_ex(MODULE_ID_APP, "%s exit\n", __func__);
    ak_thread_exit();
    return NULL;

}
/* end of thread_demux_data */

#ifdef AK_RTOS
static int demux_sample(int argc, char **argv)
#else
int main(int argc, char **argv)
#endif
{
    int ret = AK_SUCCESS;

    /* init sdk running */
    sdk_run_config config = {0};
    config.mem_trace_flag = SDK_RUN_NORMAL;
    ak_sdk_init(&config);

    /* start to parse the opt */
    if( parse_option( argc, argv ) == 0 ) {
        return 0;
    }

    ak_print_normal(MODULE_ID_APP, "*****************************************\n");
    ak_print_normal(MODULE_ID_APP, "** demux sample version: %s **\n", ak_demux_get_version());
    ak_print_normal(MODULE_ID_APP, "*****************************************\n");

    /* check save path */
    if ((access(gpc_path, W_OK) != 0))
    {
        /* create save path */
        if (mkdir(gpc_path, 0755))
        {
            ak_print_error_ex(MODULE_ID_APP, "mkdir: %s failed\n", gpc_path);
            return AK_FAILED;
        }
    }

    struct ak_demux_input demux_param;
    ak_pthread_t demux_tid;

    /* open file */
    gp_FILE_demux = fopen(gpc_demux_file, "rb");
    if (!gp_FILE_demux) {
        ak_print_error_ex(MODULE_ID_APP, "open %s failed\n", gpc_demux_file);
        ret = AK_FAILED;
        goto exit;
    }


    demux_param.data_source = DEMUX_DATA_FROM_LOCAL_FILE;
    demux_param.from_dir = gp_FILE_demux;
    demux_param.start_ms = gi_offset_ms;
    /* open demux lib */
    if (ak_demux_open(&demux_param, &gi_demux_handle)) {
        ak_print_error_ex(MODULE_ID_APP, "error: open file failed \n");
        return AK_FAILED;
    }

    /* get media video information */
    if ( ak_demux_get_video_info( gi_demux_handle, &g_demux_video_info ) != AK_SUCCESS ) {
        ak_print_error_ex(MODULE_ID_APP, "ak_demux_get_video_info( ) error.\n");
        return AK_FAILED;
    }
    /* get media audio information */
    if ( ak_demux_get_audio_info( gi_demux_handle, &g_demux_audio_info ) != AK_SUCCESS ) {
        ak_print_error_ex(MODULE_ID_APP, "ak_demux_get_audio_info( ) error.\n");
        return AK_FAILED;
    }

    /* create thread */
    ret = ak_thread_create(&(demux_tid), thread_demux_data, NULL, ANYKA_THREAD_MIN_STACK_SIZE, -1);
    if (ret) {
        goto exit;
    }

    /* wait for the thread exit */
    ak_thread_join(demux_tid);
    ak_print_notice_ex(MODULE_ID_APP, "join finish \n");

exit:
    /* close the demux lib */
    if(gi_demux_handle != -1) {
        ak_demux_close(gi_demux_handle);
        gi_demux_handle = -1;
    }

    /* close video file */
    if(gp_FILE_video != NULL) {
        fclose(gp_FILE_video);
        gp_FILE_video = NULL;
    }

    /* close audio file */
    if(gp_FILE_audio != NULL) {
        fclose(gp_FILE_audio);
        gp_FILE_audio = NULL;
    }

    /* close demux file */
    if(gp_FILE_demux != NULL) {
        fclose(gp_FILE_demux);
        gp_FILE_demux = NULL;
    }

    gc_skip_serial = AK_FALSE;
    ak_sdk_exit();
    return ret;
}
/* end of func */

/**
 * write_wav_header - 
 *
 * notes:
 */
int32_t write_wav_header(audio_files *aufile, FILE *outFile)
{
    FMT_CK *p_fmt_ck = &fmt_ck;
    WAVE_FORMAT_EX *wavFmt = &fmt_ck.wavFormat;
    ADPCM_FMT_CK adpcm_fmt_ck;
    int32_t have_fact = 1;
    uint32_t data_size = min(aufile->data_len, (uint32_t)MAXWAVESIZE);

    // RIFF
    riff_hdr.dwRiffSize = 4; // "WAVE"

    if (aufile->fileType == _WAVE_LPCM)
    {
        // fmt
        p_fmt_ck->hdr.len = 16; // WAVE_FORMAT_PCM ignores WAVE_FORMAT_EX.wExtraSize

        wavFmt->wFormatTag = 0x0001;
        wavFmt->wChannels = (uint16_t)aufile->channels;
        wavFmt->dwSamplesPerSec = aufile->samplerate;
        wavFmt->dwAvgBytesPerSec = aufile->samplerate * aufile->bits_per_sample * aufile->channels / 8;
        wavFmt->wBlockAlign = aufile->bits_per_sample * aufile->channels / 8;
        wavFmt->wBitsPerSample = aufile->bits_per_sample;

        // fact
        have_fact = 0; // WAVE_FORMAT_PCM does not need fact_ck

        //data
        data_ck.hdr.len = data_size;
    }
    else if (aufile->fileType == _WAVE_ALAW || aufile->fileType == _WAVE_ULAW)
    {
        // fmt
        p_fmt_ck->hdr.len = 18; // WAVE_FORMAT_EX

        wavFmt->wFormatTag = (aufile->fileType == _WAVE_ALAW)? 0x0006 : 0x0007;
        wavFmt->wChannels = (uint16_t)aufile->channels;
        wavFmt->dwSamplesPerSec = aufile->samplerate;
        wavFmt->dwAvgBytesPerSec = aufile->samplerate * aufile->bits_per_sample * aufile->channels / 8;
        wavFmt->wBlockAlign = aufile->bits_per_sample * aufile->channels / 8;
        wavFmt->wBitsPerSample = aufile->bits_per_sample;
        wavFmt->wExtraSize = 0;

        // fact
        fact_ck.hdr.len = 4;
        fact_ck.dwFileSize = data_size;

        //data
        data_ck.hdr.len = data_size;
    }
    else if (aufile->fileType == _WAVE_DVI_ADPCM)
    {
        p_fmt_ck = (FMT_CK *)&adpcm_fmt_ck;
        wavFmt = &adpcm_fmt_ck.fmt_ck.wavFormat;

        // fmt
        memcpy(p_fmt_ck, &fmt_ck, sizeof(FMT_CK));
        p_fmt_ck->hdr.len = 20; // WAVE_FORMAT_EX + wSamplesPerBlock

        wavFmt->wFormatTag = 0x0011;
        wavFmt->wChannels = (uint16_t)aufile->channels;
        wavFmt->dwSamplesPerSec = aufile->samplerate;
        wavFmt->dwAvgBytesPerSec = aufile->samplerate * aufile->block_align / aufile->samples_per_block;
        wavFmt->wBlockAlign = aufile->block_align;
        wavFmt->wBitsPerSample = aufile->bits_per_sample;
        wavFmt->wExtraSize = 2;
        adpcm_fmt_ck.wSamplesPerBlock = aufile->samples_per_block;

        // fact
        fact_ck.hdr.len = 4;
        fact_ck.dwFileSize = data_size * aufile->samples_per_block / aufile->block_align;

        //data
        data_ck.hdr.len = data_size;
    }
    else
    {
        printf("err: save wave file fail -- not support fileType %d\n", aufile->fileType);
        goto NOT_SUPPORT;
    }

    riff_hdr.dwRiffSize += CHUNK_SIZE(p_fmt_ck);
    if (have_fact)
        riff_hdr.dwRiffSize += CHUNK_SIZE(&fact_ck);
    riff_hdr.dwRiffSize += CHUNK_SIZE(&data_ck);

    fwrite(&riff_hdr, sizeof(RIFF_HEADER), 1, outFile);
    fwrite(p_fmt_ck, CHUNK_SIZE(p_fmt_ck), 1, outFile);
    if (have_fact)
        fwrite(&fact_ck, CHUNK_SIZE(&fact_ck), 1, outFile);
    fwrite(&data_ck, sizeof(DATA_CK), 1, outFile);

    return riff_hdr.dwRiffSize + sizeof(RIFF_HEADER);

NOT_SUPPORT:
    return 0;
}
/* end of func */

/**
 * min - 
 *
 * notes:
 */
uint32_t min( uint32_t u1 , uint32_t u2 )
{
    return ( u1 <= u2 )? u1 : u2 ;
}

#ifdef AK_RTOS
SHELL_CMD_EXPORT_ALIAS(demux_sample, ak_demux_sample, demux sample)
#endif

/* end of file */
