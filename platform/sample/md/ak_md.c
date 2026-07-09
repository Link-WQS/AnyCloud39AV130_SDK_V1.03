#include <stdlib.h>
#include <string.h>

#include "ak_log.h"
#include "ak_mem.h"
#include "ak_common.h"
#include "ak_thread.h"
#include "ak_vi.h"
#include "ak_vpss.h"
#include "ak_md.h"
#include "ak_mrd.h"
#include "ak_dbg.h"

#define DEFAULT_SENS 				50
#define DEFAULT_MOVESIZE_MIN 		12
#define DEFAULT_MOVESIZE_MAX 		300
#define DEFAULT_FLT_BIG       		10000
#define DEFAULT_FLT_SMALL           5000


#define BIG_FLT_MAX 				24000
#define BIG_FLT_MIN 				4000
#define SMALL_FLT_MAX 				12000
#define SMALL_FLT_MIN 				4000

#define DAY_FRAME_CNT 				1
#define NIGHT_FRAME_CNT 			1

#define MD_DEBUG 					0

const int flt_val[101][2]={{20000,10000},
{19760,9880}, {19520,9760}, {19280,9640}, {19040,9520}, {18800,9400},
{18560,9280}, {18320,9160}, {18080,9040}, {17840,8920}, {17600,8800},
{17360,8680}, {17120,8560}, {16880,8440}, {16640,8320}, {16400,8200},
{16160,8080}, {15920,7960}, {15680,7840}, {15440,7720}, {15200,7600},
{14960,7480}, {14720,7360}, {14480,7240}, {14240,7120}, {14000,7000},
{13840,6920}, {13680,6840}, {13520,6760}, {13360,6680}, {13200,6600},
{13040,6520}, {12880,6440}, {12720,6360}, {12560,6280}, {12400,6200},
{12240,6120}, {12080,6040}, {11920,5960}, {11760,5880}, {11600,5800},
{11440,5720}, {11280,5640}, {11120,5560}, {10960,5480}, {10800,5400},
{10640,5320}, {10480,5240}, {10320,5160}, {10160,5080}, {10000,5000},
{9880,4960}, {9760,4920}, {9640,4880}, {9520,4840}, {9400,4800},
{9280,4760}, {9160,4720}, {9040,4680}, {8920,4640}, {8800,4600},
{8680,4560}, {8560,4520}, {8440,4480}, {8320,4440}, {8200,4400},
{8080,4360}, {7960,4320}, {7840,4280}, {7720,4240}, {7600,4200},
{7480,4160}, {7360,4120}, {7240,4080}, {7120,4040}, {7000,4000},
{6880,3960}, {6760,3920}, {6640,3880}, {6520,3840}, {6400,3800},
{6280,3760}, {6160,3720}, {6040,3680}, {5920,3640}, {5800,3600},
{5680,3560}, {5560,3520}, {5440,3480}, {5320,3440}, {5200,3400},
{5080,3360}, {4960,3320}, {4840,3280}, {4720,3240}, {4600,3200},
{4480,3160}, {4360,3120}, {4240,3080}, {4120,3040}, {4000,3000}
};
/*
const int size_min[101]={50,
49, 49, 49, 49,
48, 48, 48, 48,
45, 45, 45, 45,
42, 42, 42, 42,
40, 40, 40, 40,
36, 36, 36, 36,
35, 35, 35, 35,
32, 32, 32, 32,
30, 30, 30, 30,
28, 28, 28, 28,
27, 27, 27, 27,
25, 25, 25, 25,
24, 24, 24, 24,
21, 21, 21, 21,
20, 20, 20, 20,
18, 18, 18, 18,
16, 16, 16, 16,
15, 15, 15, 15,
14, 14, 14, 14,
12, 12, 12, 12,
10, 10, 10, 10,
9, 9, 9, 9,
8, 8, 8, 8,
6, 6, 6, 6,
4, 4, 4, 4
};
*/
const int size_min[101]={50,
49, 48, 47, 46,
45, 44, 43, 42,
41, 40, 39, 38,
37, 36, 35, 34,
33, 32, 31, 30,
29, 28, 27, 27,
26, 25, 24, 24,
23, 22, 21, 21,
20, 19, 18, 18,
17, 16, 15, 15,
14, 14, 13, 13,
12, 12, 12, 12,
12, 12, 12, 12,
11, 11, 11, 11,
11, 11, 11, 11,
10, 10, 10, 10,
9, 9, 9, 9,
9, 9, 9, 9,
8, 8, 8, 8,
8, 8, 8, 8,
7, 7, 7, 7,
6, 6, 6, 6,
6, 6, 6, 6,
5, 5, 5, 5,
4, 4, 4, 4
};



enum md_thread_stat{
    MD_THREAD_EXIT = 0,
    MD_THREAD_STOP ,
    MD_THREAD_RUN ,
};

struct hw_md_result {
    ak_mutex_t lock;
    struct md_result_info ret_info;
};

struct hw_md_ctrl {
    int thread_stat;
    ak_pthread_t md_tid;
    ak_sem_t send_sem;
    int dev;
    /*msecond. interval time data to detect*/
    int md_fps;
    /*global sensivity*/
    int sensitivity;
    /*area mode flag*/
    int area_md_flag;
    char area_mask[VPSS_MD_DIMENSION_H_MAX * VPSS_MD_DIMENSION_V_MAX];
    int v_size_max;
    int move_size_min;
    int move_size_max;
    void *mrd_handle;
    int day_night_mode;
    int res_width;
    int res_height;
} ;

static const char *md_version = "libapp_md V1.0.08";
static struct hw_md_result md_result[VIDEO_DEV_MUX] = {0};
static struct hw_md_ctrl md_ctrl[VIDEO_DEV_MUX] = {0};
static int md_init_flag[VIDEO_DEV_MUX] = {0};

/**
 * ak_hw_md_get_stat: get motion detection stat params
 * @dev[IN]: dev id
 * @md[OUT]: md params
 * return: 0 - success; other error code;
 * notes:
 */
static int md_get_stat(int dev, struct vpss_md_info *md)
{
    if (dev >= VIDEO_DEV_MUX)
    {
        ak_print_error_ex(MODULE_ID_MD, "dev:%d error!\n", dev);
        return ERROR_TYPE_INVALID_ARG;
    }

    if (NULL == md)
    {
        ak_print_error_ex(MODULE_ID_MD, "md is null!\n");
        return ERROR_TYPE_POINTER_NULL;
    }

    /*get isp 3d stat info*/
    int ret = ak_vpss_md_get_stat(dev, md);

    if (0 != ret)
    {
        ak_print_error_ex(MODULE_ID_MD, "get 3d nr stat info fail\n");
    }

    return ret;
}

/**
 * submit -  set the sensitivity that  md do
 * @dev[IN]: dev id
 * @sensitivity[IN]:   sensitivity to do global md.
 * return:  0 - success;  -1  - fail;
 */
int md_set_sensitivity (int dev, int sensitivity)
{
/*
    unsigned short flt_big = 0;
    unsigned short step_big = 0;
    unsigned short flt_small = 0;
    unsigned short step_small = 0;


    step_big = (BIG_FLT_MAX - BIG_FLT_MIN) / 100;
    step_small = (SMALL_FLT_MAX - SMALL_FLT_MIN) / 100;

    flt_big = BIG_FLT_MAX -  step_big * sensitivity;
    flt_small = SMALL_FLT_MAX - step_small * sensitivity;

    return ak_mrd_set_filters(md_ctrl[dev].mrd_handle, flt_big, flt_small);
    return ak_mrd_set_filters(md_ctrl[dev].mrd_handle, flt_val[sensitivity][0], flt_val[sensitivity][1]);
*/

    md_ctrl[dev].move_size_min = size_min[sensitivity];
    return ak_mrd_set_motion_region_size(md_ctrl[dev].mrd_handle, \
        md_ctrl[dev].move_size_min, md_ctrl[dev].move_size_max);
}


/**
 * md_area_judge: judge function for area mode
 * @md[IN]: md params
 * return: 0 no move, >0 move box num
 * notes:
 */
static int md_area_judge(int dev, int move_box_num, \
    MRD_RECTANGLE boxes[MAX_CAP])
{
    int i = 0, j = 0, k = 0;
    char flag[MAX_CAP][VPSS_MD_DIMENSION_V_MAX][VPSS_MD_DIMENSION_H_MAX] = {{{0}}};
    int sum[MAX_CAP] = {0};
    int select_id[MAX_BOX_NUM] = {-1, -1, -1, -1, -1};
    int max_sum = md_ctrl[dev].move_size_min - 1;
    int cnt = 0;
    MRD_RECTANGLE boxes_new[MAX_CAP] = {{0}};
    int value = 0;
    int min_x = 0;
    int min_y = 0;
    int max_x = 0;
    int max_y = 0;

#if MD_DEBUG
    ak_print_normal(MODULE_ID_MD, "total move num:%d\n", move_box_num);
#endif

    if (move_box_num > MAX_CAP)
        move_box_num = MAX_CAP;

    /*
    * 找出与选择的区域模版重叠的移动区域
    */
    for (k=0; k<move_box_num; k++)
    {
        min_x = boxes[k].right;
        min_y = boxes[k].bottom;
        max_x = boxes[k].left;
        max_y = boxes[k].top;

        for(i=0; i<md_ctrl[dev].v_size_max; i++)
        {
            for(j=0; j<VPSS_MD_DIMENSION_H_MAX; j++)
            {
                if ((i >= boxes[k].top)
                    && (i <= boxes[k].bottom)
                    && (j >= boxes[k].left)
                    && (j <= boxes[k].right))
                    flag[k][i][j] = 1;
                else
                    flag[k][i][j] = 0;

                value = md_ctrl[dev].area_mask[i * VPSS_MD_DIMENSION_H_MAX + j]\
                    * flag[k][i][j];

                sum[k] += value;

                if (value)
                {
                    if (i < min_y)
                        min_y = i;

                    if (j < min_x)
                        min_x = j;

                    if (i > max_y)
                        max_y = i;

                    if (j > max_x)
                        max_x = j;
                }
            }

        }
#if MD_DEBUG
        ak_print_normal(MODULE_ID_MD, "sum[%d]:%d\n", k, sum[k]);
#endif
        boxes_new[k].left = min_x;
        boxes_new[k].top = min_y;
        boxes_new[k].right = max_x;
        boxes_new[k].bottom = max_y;
    }

    for (i=0; (i<MAX_BOX_NUM) && (i<move_box_num); i++)
    {
        max_sum = md_ctrl[dev].move_size_min - 1;

        for (k=0; k<move_box_num; k++)
        {
            if ((sum[k] > max_sum) && (sum[k] < md_ctrl[dev].move_size_max))
            {
                max_sum = sum[k];
                select_id[i] = k;
            }
        }

        if (select_id[i] >= 0)
        {
            sum[select_id[i]] = 0;
            cnt++;
        }
    }

#if MD_DEBUG
    ak_print_normal(MODULE_ID_MD, "cnt:%d\n", cnt);

    for (i=0; i<cnt; i++)
    {
        ak_print_normal(MODULE_ID_MD, \
            "select_id[%d]:%d [%d, %d, %d, %d], [%d, %d, %d, %d]\n", i,\
            select_id[i],
            boxes_new[select_id[i]].left, boxes_new[select_id[i]].top,
            boxes_new[select_id[i]].right, boxes_new[select_id[i]].bottom,
            boxes[select_id[i]].left, boxes[select_id[i]].top,
            boxes[select_id[i]].right, boxes[select_id[i]].bottom);
    }
#endif

    if (cnt > 0)
    {
        ak_thread_mutex_lock(&md_result[dev].lock);

        memset(&md_result[dev].ret_info.boxes, 0, \
            MAX_BOX_NUM * sizeof(struct md_box));
        md_result[dev].ret_info.move_box_num = cnt;

        for (i=0; i<cnt; i++)
        {
            md_result[dev].ret_info.boxes[i].left = \
                boxes_new[select_id[i]].left;
            md_result[dev].ret_info.boxes[i].top = \
                boxes_new[select_id[i]].top;
            md_result[dev].ret_info.boxes[i].right = \
                boxes_new[select_id[i]].right;
            md_result[dev].ret_info.boxes[i].bottom = \
                boxes_new[select_id[i]].bottom;
        }
        ak_thread_mutex_unlock(&md_result[dev].lock);
    }

    return cnt;
}

/**
 * md_check: judge function
 * @md_ctrl[IN]: md ctrl struct pointer
 * return: 0 no move, 1 has move
 * notes:
 */
static int md_check(struct hw_md_ctrl *md_ctrl)
{
    int ret = 0;
    int area_ret = 0;
    int hasmove = 0;
    struct vpss_md_info md = {{{0}}};
    MRD_RECTANGLE boxes[MAX_CAP] = {{0}};
    int dev = md_ctrl->dev;

    if (md_get_stat(dev, &md))
    {
        ak_print_warning_ex(MODULE_ID_MD, "get md stat fail.\n");
        return 0;
    }

    ret = ak_mrd(md_ctrl->mrd_handle, md.stat, md_ctrl->v_size_max, boxes);

    if (ret > 0)
    {
        //ak_print_normal(MODULE_ID_MD, "box 0: [%d, %d, %d, %d]\n",
                    //boxes[0].left, boxes[0].top, boxes[0].right, boxes[0].bottom);

        if(md_ctrl->area_md_flag)
        {
            area_ret = md_area_judge(dev, ret, boxes);
            if (area_ret > 0)
            {
                hasmove = 1;
            }
        }
        else
        {
            ak_thread_mutex_lock(&md_result[dev].lock);

            memset(&md_result[dev].ret_info.boxes, 0, \
                MAX_BOX_NUM * sizeof(struct md_box));
            if (ret > MAX_BOX_NUM)
                md_result[dev].ret_info.move_box_num = MAX_BOX_NUM;
            else
                md_result[dev].ret_info.move_box_num = ret;

            for (int i=0; i<md_result[dev].ret_info.move_box_num; i++)
            {
                md_result[dev].ret_info.boxes[i].left = boxes[i].left;
                md_result[dev].ret_info.boxes[i].top = boxes[i].top;
                md_result[dev].ret_info.boxes[i].right = boxes[i].right;
                md_result[dev].ret_info.boxes[i].bottom = boxes[i].bottom;
            }
            ak_thread_mutex_unlock(&md_result[dev].lock);
            hasmove = 1;
        }
    }

    return hasmove;
}

/**
 * submit - get md result.
 * @dev[IN]:  dev id
 * @ret_info[OUT]:	result info
 * return: 0 - success; other error code;
 */
int get_md_result(int dev, struct md_result_info *ret_info)
{
    if (dev >= VIDEO_DEV_MUX)
    {
        ak_print_error_ex(MODULE_ID_MD, "dev:%d error!\n", dev);
        return ERROR_TYPE_INVALID_ARG;
    }

    if (NULL == ret_info)
    {
        ak_print_error_ex(MODULE_ID_MD, "ret_info is null!\n");
        return ERROR_TYPE_POINTER_NULL;
    }

    memcpy(ret_info, &md_result[dev].ret_info, sizeof(struct md_result_info));

    memset(&md_result[dev].ret_info, 0, sizeof(struct md_result_info));

    return AK_SUCCESS;
}

/**
 * md_thread - move detect thread, do detect thing here
 * compare move detect success over 2 times, \
 * then one detect message is emit really
 */
static void *md_thread(void *arg)
{
    int detect_interval = 0;
    struct ak_timeval md_tv = {0}, now_tv = {0};
    struct ak_date date = {0};
    int move_count = 0;
    int frame_cnt = 0;

    ak_thread_set_name("mpp_md");

    ak_print_normal_ex(MODULE_ID_MD, "thread id : %ld\n", ak_thread_get_tid());

    int dev = *(int*)arg;

    while (md_ctrl[dev].thread_stat)
    {
        detect_interval = 1000/md_ctrl[dev].md_fps;

        if (md_ctrl[dev].day_night_mode)    //day
            frame_cnt = DAY_FRAME_CNT;
        else
            frame_cnt = NIGHT_FRAME_CNT;

        /*
        * stop mode ,don't run to check, wait here
        */
        do {
            ak_sleep_ms(detect_interval);
            if(	MD_THREAD_STOP == md_ctrl[dev].thread_stat){
                move_count = 0;
            }
        } while(MD_THREAD_STOP == md_ctrl[dev].thread_stat);

        if (md_check(&md_ctrl[dev]))
        {
            move_count++;
            if(move_count >= frame_cnt)
            {
                move_count = 0;
                ak_get_ostime(&md_tv);
                ak_get_localdate(&date);
                ak_thread_mutex_lock(&md_result[dev].lock);
                md_result[dev].ret_info.md_sec = ak_date_to_seconds(&date);
                md_result[dev].ret_info.result = 1;
                ak_thread_mutex_unlock(&md_result[dev].lock);

                ak_thread_sem_post(&md_ctrl[dev].send_sem);
            }
        }
        else
        {
            /*
             * alarm module donot take away md result and it is over 4 second,
             * clean it
             */
            ak_thread_mutex_lock(&md_result[dev].lock);
            if (1 == md_result[dev].ret_info.result)
            {
                ak_get_ostime(&now_tv);
                if ((now_tv.sec > (md_tv.sec + 4)) || (now_tv.sec < md_tv.sec))
                {
                    md_result[dev].ret_info.result = 0;
                }
            }
            ak_thread_mutex_unlock(&md_result[dev].lock);
            move_count = 0;
        }
    }

    ak_print_normal_ex(MODULE_ID_MD, "### thread id: %ld exit ###\n",
        ak_thread_get_tid());
    ak_thread_exit();
    return NULL;
}

/**
 * submit - md init. set defaut fps ,global sensitivity
 * @dev[IN]:  dev id
 *
 * return:  0 - success; other error code;
 */
int ak_md_init(int dev)
{
    int ret = AK_SUCCESS;
    RECTANGLE_S attr = {0};  // code_updating
    int flt_big = 0;
    int flt_small = 0;

    if (dev >= VIDEO_DEV_MUX)
    {
        ak_print_error_ex(MODULE_ID_MD, "dev:%d error!\n", dev);
        return ERROR_TYPE_INVALID_ARG;
    }

    if (md_init_flag[dev])
    {
        ak_print_error_ex(MODULE_ID_MD, "have been init \n");
        return AK_SUCCESS;
    }

    memset(&md_ctrl[dev], 0, sizeof(struct hw_md_ctrl));
    memset(&md_result[dev], 0, sizeof(struct hw_md_result));

    md_ctrl[dev].mrd_handle = ak_mrd_init();

    if (NULL == md_ctrl[dev].mrd_handle)
    {
        ak_print_error_ex(MODULE_ID_MD, "ak_mrd_init failed!\n");
        return ERROR_MD_LIB_INIT_FAILED;
    }

    md_ctrl[dev].dev = dev;
    md_ctrl[dev].thread_stat = MD_THREAD_STOP;
    ak_thread_sem_init(&md_ctrl[dev].send_sem, 0);
    md_ctrl[dev].sensitivity = DEFAULT_SENS;
    md_ctrl[dev].area_md_flag = 0;
    md_ctrl[dev].md_fps = 10;
    md_ctrl[dev].move_size_min = DEFAULT_MOVESIZE_MIN;
    md_ctrl[dev].move_size_max = DEFAULT_MOVESIZE_MAX;
    md_ctrl[dev].day_night_mode = 1;    //day

    ak_mrd_set_mode(md_ctrl[dev].mrd_handle, md_ctrl[dev].day_night_mode);

    ak_thread_mutex_init(&md_result[dev].lock, NULL);

    ak_vi_get_sensor_resolution(dev, &attr); // code_updating

    if (0 == attr.height % VPSS_MD_DIMENSION_V_MAX)
        md_ctrl[dev].v_size_max = VPSS_MD_DIMENSION_V_MAX;
    else
        md_ctrl[dev].v_size_max = 16;

    md_ctrl[dev].res_width = attr.width;
    md_ctrl[dev].res_height = attr.height;

    if (md_ctrl[dev].res_width * md_ctrl[dev].res_height > 3500000)
    {
        flt_big = 12000;
        flt_small = 6000;
    }
    else
    {
        flt_big = DEFAULT_FLT_BIG;
        flt_small = DEFAULT_FLT_SMALL;
    }

    ak_mrd_set_filters(md_ctrl[dev].mrd_handle, flt_big, flt_small);

    ak_mrd_set_motion_region_size(md_ctrl[dev].mrd_handle, \
        md_ctrl[dev].move_size_min, md_ctrl[dev].move_size_max);

    ak_mrd_set_floating_wadding_params(md_ctrl[dev].mrd_handle, \
        3, 2, 4, 36, 2, 3);

    if (ak_thread_create(&md_ctrl[dev].md_tid, md_thread, &(md_ctrl[dev].dev),
                ANYKA_THREAD_NORMAL_STACK_SIZE, -1))
    {
        ak_print_error_ex(MODULE_ID_MD, "create move detect thread failed.\n");
        return ERROR_MD_CREATE_THREAD_FAILED;
    }

    md_init_flag[dev] = 1;

    /*add device status*/
    char cmd[64]={0};
    /*record the version info*/
    memset(cmd, 0, sizeof(cmd));
    snprintf(cmd, sizeof(cmd), "md_version=%s", (char *)ak_md_get_version());
    ak_debug_info(MODULE_ID_MD, DBG_TYPE_STAT, cmd);

    memset(cmd, 0, sizeof(cmd));
    snprintf(cmd, sizeof(cmd), "mrd_version=%s", \
        (char *)ak_md_get_mrd_version());
    ak_debug_info(MODULE_ID_MD, DBG_TYPE_STAT, cmd);

    memset(cmd, 0, sizeof(cmd));
    snprintf(cmd, sizeof(cmd), "sensitivity=%d", md_ctrl[dev].sensitivity);
    ak_debug_info(MODULE_ID_MD, DBG_TYPE_PARAM, cmd);

    memset(cmd, 0, sizeof(cmd));
    snprintf(cmd, sizeof(cmd), "flt_big=%d", flt_big);
    ak_debug_info(MODULE_ID_MD, DBG_TYPE_PARAM, cmd);

    memset(cmd, 0, sizeof(cmd));
    snprintf(cmd, sizeof(cmd), "flt_small=%d", flt_small);
    ak_debug_info(MODULE_ID_MD, DBG_TYPE_PARAM, cmd);

    return ret;
}
/* end of ak_md_init */

/**
 * submit -  set how much frame to do md in one second
 * @dev[IN]:  dev id
 * @fps[IN]:     how much frame to do md in one second.
 * return:  0 - success;  other error code;
 */
int ak_md_set_fps(int dev, int fps)
{
    if (dev >= VIDEO_DEV_MUX)
    {
        ak_print_error_ex(MODULE_ID_MD, "dev:%d error!\n", dev);
        return ERROR_TYPE_INVALID_ARG;
    }

    if (0 == md_init_flag[dev])
    {
        ak_print_error_ex(MODULE_ID_MD, "fail,no init\n");
        return ERROR_MD_NOT_INIT;
    }

    if (fps <= 0)
    {
        ak_print_error_ex(MODULE_ID_MD, "invalid arg\n");
        return ERROR_TYPE_INVALID_ARG;
    }

    md_ctrl[dev].md_fps = fps;

    return AK_SUCCESS;
}

/**
 * submit -  get fps
 * @dev[IN]:  dev id
 * @fps[OUT]:   how much frame that md do  in one second.
 * return:  0 - success;  other error code;
 */
int ak_md_get_fps(int dev, int *fps)
{
    if (dev >= VIDEO_DEV_MUX)
    {
        ak_print_error_ex(MODULE_ID_MD, "dev:%d error!\n", dev);
        return ERROR_TYPE_INVALID_ARG;
    }

    if (NULL == fps)
    {
        ak_print_error_ex(MODULE_ID_MD, "fps is null!\n");
        return ERROR_TYPE_POINTER_NULL;
    }

    if (0 == md_init_flag[dev])
    {
        ak_print_error_ex(MODULE_ID_MD, "fail,no init\n");
        return ERROR_MD_NOT_INIT;
    }


    *fps = md_ctrl[dev].md_fps;

    return AK_SUCCESS;
}

/**
 * submit -  set the sensitivity that  md do
 * @dev[IN]:  dev id
 * @sensitivity[IN]:   sensitivity
 * return:  0 - success;  other error code;
 */
int ak_md_set_sensitivity (int dev, int sensitivity)
{
    ak_print_normal_ex(MODULE_ID_MD, "sensitivity:%d \n",sensitivity);

    if (dev >= VIDEO_DEV_MUX)
    {
        ak_print_error_ex(MODULE_ID_MD, "dev:%d error!\n", dev);
        return ERROR_TYPE_INVALID_ARG;
    }

    if (0 == md_init_flag[dev])
    {
        ak_print_error_ex(MODULE_ID_MD, "fail,no init\n");
        return ERROR_MD_NOT_INIT;
    }

    if ((sensitivity < 0) || (sensitivity > 100)){
        ak_print_error_ex(MODULE_ID_MD, "invalid arg sensitivity\n");
        return ERROR_TYPE_INVALID_ARG;
    }

    if (md_set_sensitivity(dev, sensitivity)) {
        ak_print_error_ex(MODULE_ID_MD, "fail, md_set_sensitivity err\n");
        return ERROR_TYPE_MEDIA_LIB_FAILED;
    }

    md_ctrl[dev].sensitivity = sensitivity;

    char cmd[64]={0};
    memset(cmd, 0, sizeof(cmd));
    snprintf(cmd, sizeof(cmd), "sensitivity=%d", md_ctrl[dev].sensitivity);
    ak_debug_info(MODULE_ID_MD, DBG_TYPE_PARAM, cmd);

    return AK_SUCCESS;
}

/**
 * submit -  get sensitivity
 * @dev[IN]:  dev id
 * @sensitivity[OUT]:   how sensitivity that md do
 * return: 0 - success;  other error code;
 */
int ak_md_get_sensitivity (int dev, int *sensitivity)
{
    if (dev >= VIDEO_DEV_MUX)
    {
        ak_print_error_ex(MODULE_ID_MD, "dev:%d error!\n", dev);
        return ERROR_TYPE_INVALID_ARG;
    }

    if (NULL == sensitivity)
    {
        ak_print_error_ex(MODULE_ID_MD, "sensitivity is null!\n");
        return ERROR_TYPE_POINTER_NULL;
    }

    if (0 == md_init_flag[dev])
    {
        ak_print_error_ex(MODULE_ID_MD, "fail,no init\n");
        return ERROR_MD_NOT_INIT;
    }

    *sensitivity = md_ctrl[dev].sensitivity;

    return AK_SUCCESS;
}

/**
 * submit -  set mrd filters
 * @dev[IN]:  dev id
 * @flt_big[IN]:   flt_big
 * @flt_small[IN]:   flt_small
 * return:  0 - success;  other error code;
 */
int ak_md_set_filters (int dev, unsigned short flt_big, unsigned short flt_small)
{
    if (dev >= VIDEO_DEV_MUX)
    {
        ak_print_error_ex(MODULE_ID_MD, "dev:%d error!\n", dev);
        return ERROR_TYPE_INVALID_ARG;
    }

    if (0 == md_init_flag[dev])
    {
        ak_print_error_ex(MODULE_ID_MD, "fail,no init\n");
        return ERROR_MD_NOT_INIT;
    }

    if (flt_big < flt_small)
    {
        ak_print_error_ex(MODULE_ID_MD, "flt_big < flt_small, error.\n");
        return ERROR_TYPE_INVALID_ARG;
    }

    if (ak_mrd_set_filters(md_ctrl[dev].mrd_handle, flt_big, flt_small))
    {
        ak_print_error_ex(MODULE_ID_MD, "ak_mrd_set_filters failed.\n");
        return ERROR_TYPE_MEDIA_LIB_FAILED;
    }

    char cmd[64]={0};

    memset(cmd, 0, sizeof(cmd));
    snprintf(cmd, sizeof(cmd), "flt_big=%d", flt_big);
    ak_debug_info(MODULE_ID_MD, DBG_TYPE_PARAM, cmd);

    memset(cmd, 0, sizeof(cmd));
    snprintf(cmd, sizeof(cmd), "flt_small=%d", flt_small);
    ak_debug_info(MODULE_ID_MD, DBG_TYPE_PARAM, cmd);

    return AK_SUCCESS;
}


/**
 * submit -  set how much frame to do md in one second
 * @dev[IN]:  dev id
 * @day_night_mode[IN]:  0  night, 1 day
 * return:  0 - success;  other error code;
 */
int ak_md_set_day_night_mode(int dev, int day_night_mode)
{
    int ret = 0;
    /*add device status*/
    //char cmd[64]={0};

    ak_print_normal_ex(MODULE_ID_MD, "day_night_mode:%d \n", day_night_mode);

    if (dev >= VIDEO_DEV_MUX)
    {
        ak_print_error_ex(MODULE_ID_MD, "dev:%d error!\n", dev);
        return ERROR_TYPE_INVALID_ARG;
    }

    if (0 == md_init_flag[dev])
    {
        ak_print_error_ex(MODULE_ID_MD, "fail,no init\n");
        return ERROR_MD_NOT_INIT;
    }

    if (day_night_mode < 0 || day_night_mode > 1)
    {
        ak_print_error_ex(MODULE_ID_MD, \
            "day_night_mode %d err.must be 0 or 1.\n", day_night_mode);
        return ERROR_TYPE_INVALID_ARG;
    }

    md_ctrl[dev].day_night_mode = day_night_mode;

    ret = ak_mrd_set_mode(md_ctrl[dev].mrd_handle, day_night_mode);

    if (ret)
    {
        ak_print_error_ex(MODULE_ID_MD, "ak_mrd_set_mode failed.\n");
        return ERROR_TYPE_MEDIA_LIB_FAILED;
    }

    if (day_night_mode) //day
    {
        ak_mrd_set_floating_wadding_params(md_ctrl[dev].mrd_handle, \
            3, 2, 4, 36, 2, 3);
/*
        if (md_ctrl[dev].res_width * md_ctrl[dev].res_height > 3500000)
        {
            ak_mrd_set_filters(md_ctrl[dev].mrd_handle, 12000, 6000);

            memset(cmd, 0, sizeof(cmd));
            snprintf(cmd, sizeof(cmd), "flt_big=%d", 12000);
            ak_debug_info(MODULE_ID_MD, DBG_TYPE_PARAM, cmd);
            memset(cmd, 0, sizeof(cmd));
            snprintf(cmd, sizeof(cmd), "flt_small=%d", 6000);
            ak_debug_info(MODULE_ID_MD, DBG_TYPE_PARAM, cmd);
        }
*/
    }
    else
    {
        ak_mrd_set_floating_wadding_params(md_ctrl[dev].mrd_handle, 4, 2, 4, 9, 2, 3);
/*
        if (md_ctrl[dev].res_width * md_ctrl[dev].res_height > 3500000)
        {
            ak_mrd_set_filters(md_ctrl[dev].mrd_handle, 15000, 10000);

            memset(cmd, 0, sizeof(cmd));
            snprintf(cmd, sizeof(cmd), "flt_big=%d", 15000);
            ak_debug_info(MODULE_ID_MD, DBG_TYPE_PARAM, cmd);
            memset(cmd, 0, sizeof(cmd));
            snprintf(cmd, sizeof(cmd), "flt_small=%d", 10000);
            ak_debug_info(MODULE_ID_MD, DBG_TYPE_PARAM, cmd);
        }
*/
    }

    return ret;
}


/**
 * submit - set area that md to do.
 * @dev[IN]:  dev id
 * @area_mask[IN]: area mask .
 * return:  0 - success;  other error code;
 */
int ak_md_set_area(int dev, char *area_mask)
{
    int i = 0;
    int j = 0;

    if (dev >= VIDEO_DEV_MUX)
    {
        ak_print_error_ex(MODULE_ID_MD, "dev:%d error!\n", dev);
        return ERROR_TYPE_INVALID_ARG;
    }

    if (NULL == area_mask)
    {
        ak_print_error_ex(MODULE_ID_MD, "area_mask is null!\n");
        return ERROR_TYPE_POINTER_NULL;
    }

    if (0 == md_init_flag[dev])
    {
        ak_print_error_ex(MODULE_ID_MD, "fail,not init\n");
        return ERROR_MD_NOT_INIT;
    }

    for (i=0; i<VPSS_MD_DIMENSION_H_MAX * VPSS_MD_DIMENSION_V_MAX; i++)
    {
        if (area_mask[i] != 0 && area_mask[i] != 1)
        {
            ak_print_error_ex(MODULE_ID_MD,
                "area_mask, data value error! must be 0 or 1.\n");
            return ERROR_TYPE_INVALID_ARG;
        }
    }

    memcpy(md_ctrl[dev].area_mask, area_mask, \
        VPSS_MD_DIMENSION_H_MAX * VPSS_MD_DIMENSION_V_MAX);

    md_ctrl[dev].area_md_flag = 1;

    for (i=0; i<md_ctrl[dev].v_size_max; i++)
    {
        for (j=0; j<VPSS_MD_DIMENSION_H_MAX; j++)
        {
            printf("%d ", \
                md_ctrl[dev].area_mask[i * VPSS_MD_DIMENSION_H_MAX + j]);
        }
        printf("\n");
    }

    return AK_SUCCESS;
}

/**
 * submit - get dimensionality.
 * @dev[IN]:  dev id
 * @horizon_num[OUT]:   dimension number of horizontal.
 * @vertical_num[OUT]:   dimension number of vertial.
 * return:  0 - success;  other error code;
 */
int ak_md_get_dimensionality(int dev, int *horizon_num, int *vertical_num)
{
    if (dev >= VIDEO_DEV_MUX)
    {
        ak_print_error_ex(MODULE_ID_MD, "dev:%d error!\n", dev);
        return ERROR_TYPE_INVALID_ARG;
    }

    if (0 == md_init_flag[dev])
    {
        ak_print_error_ex(MODULE_ID_MD, "fail,not init\n");
        return ERROR_MD_NOT_INIT;
    }

    if (horizon_num && vertical_num){
        *vertical_num = md_ctrl[dev].v_size_max;
        *horizon_num = VPSS_MD_DIMENSION_H_MAX;
        return AK_SUCCESS;
    }

    ak_print_error_ex(MODULE_ID_MD, "NULL pointer\n");
    return ERROR_TYPE_POINTER_NULL;
}

/**
 * submit - get md result.
 * @dev[IN]:  dev id
 * @ret_info[OUT]:  result info
 * @timeout[IN]:  timeout <0  block mode, ==0 non-block mode, >0 waittime
 * return: 0 - success;  other error code;
 */
int ak_md_get_result(int dev, struct md_result_info *ret_info, int timeout)
{
    int ret = 0;

    if (dev >= VIDEO_DEV_MUX)
    {
        ak_print_error_ex(MODULE_ID_MD, "dev:%d error!\n", dev);
        return ERROR_TYPE_INVALID_ARG;
    }

    if (NULL == ret_info)
    {
        ak_print_error_ex(MODULE_ID_MD, "ret_info is null!\n");
        return ERROR_TYPE_POINTER_NULL;
    }

    memset(ret_info, 0, sizeof(struct md_result_info));

    if (0 == md_init_flag[dev])
    {
        ak_print_error_ex(MODULE_ID_MD, "fail,not init\n");
        return ERROR_MD_NOT_INIT;
    }

    if (MD_THREAD_RUN != md_ctrl[dev].thread_stat) {
        ak_print_error_ex(MODULE_ID_MD, "fail,not run\n");
        return ERROR_MD_NOT_RUN;
    }

    /*md have been triggered*/
    ak_thread_mutex_lock(&md_result[dev].lock);
    if (md_result[dev].ret_info.result)
    {
        get_md_result(dev, ret_info);
        ak_thread_mutex_unlock(&md_result[dev].lock);
        return AK_SUCCESS;
    }
    ak_thread_mutex_unlock(&md_result[dev].lock);

    if (0 == timeout)
        return 0;
    else if (timeout > 0) {
        while (timeout > 0) {
            ak_sleep_ms(20);
            ak_thread_mutex_lock(&md_result[dev].lock);
            if (md_result[dev].ret_info.result)
            {
                get_md_result(dev, ret_info);
                ak_thread_mutex_unlock(&md_result[dev].lock);
                return AK_SUCCESS;
            }
            ak_thread_mutex_unlock(&md_result[dev].lock);
            timeout -= 20;
        }
    } else if (timeout < 0) {
        ret = ak_thread_sem_wait(&md_ctrl[dev].send_sem);
    }

    /* sem_wait exit normally, md is triggered*/
    if (0 == ret)
    {
        ak_thread_mutex_lock(&md_result[dev].lock);
        if (md_result[dev].ret_info.result)
        {
            get_md_result(dev, ret_info);
            ak_thread_mutex_unlock(&md_result[dev].lock);
            return AK_SUCCESS;
        }
        ak_thread_mutex_unlock(&md_result[dev].lock);
    }

    return AK_SUCCESS;
}

/**
 * ak_md_enable - start or stop md .
 * @dev[IN]:  dev id
 * @enable[IN]:  [0,1],  0 -> stop md; 1 -> start md
 * return: 0 - success; other error code;
 */
int ak_md_enable(int dev, int enable)
{
    if (dev >= VIDEO_DEV_MUX)
    {
        ak_print_error_ex(MODULE_ID_MD, "dev:%d error!\n", dev);
        return ERROR_TYPE_INVALID_ARG;
    }

    if ((enable != 0) && (enable != 1))
    {
        ak_print_error_ex(MODULE_ID_MD, "enable:%d error!\n", enable);
        return ERROR_TYPE_INVALID_ARG;
    }

    if (0 == md_init_flag[dev])
    {
        ak_print_error_ex(MODULE_ID_MD, "fail,no init\n");
        return ERROR_MD_NOT_INIT;
    }

    ak_print_normal_ex(MODULE_ID_MD, "enable:%d\n", enable);

    if ((1 == enable) && (MD_THREAD_STOP == md_ctrl[dev].thread_stat))
    {
        md_ctrl[dev].thread_stat = MD_THREAD_RUN;
        //ak_thread_sem_post(&md_ctrl.run_sem);
    }
    else if ((0 == enable) && (MD_THREAD_RUN == md_ctrl[dev].thread_stat))
    {
        md_ctrl[dev].thread_stat = MD_THREAD_STOP;
    }

    return AK_SUCCESS;
}

/*
 * ak_md_destroy - free md resource and quit md .
 * @dev[IN]:  dev id
 * return: 0 - success; other error code;
 */
int ak_md_destroy(int dev)
{
     int ret = 0;

    if (dev >= VIDEO_DEV_MUX)
    {
        ak_print_error_ex(MODULE_ID_MD, "dev:%d error!\n", dev);
        return ERROR_TYPE_INVALID_ARG;
    }

    if (0 == md_init_flag[dev])
    {
        ak_print_error_ex(MODULE_ID_MD, "fail, no init\n");
        return ERROR_MD_NOT_INIT;
    }

    md_init_flag[dev] = 0;

    if (MD_THREAD_STOP == md_ctrl[dev].thread_stat)
    {
        md_ctrl[dev].thread_stat = MD_THREAD_EXIT;
        //ak_thread_sem_post(&md_ctrl.run_sem);
    }
    else
        md_ctrl[dev].thread_stat = MD_THREAD_EXIT;

    ret = ak_thread_join(md_ctrl[dev].md_tid);
    ak_print_normal_ex(MODULE_ID_MD, "disable, %s\n", \
        ret ? "failed" : "success");

    ak_mrd_destroy(md_ctrl[dev].mrd_handle);
    md_ctrl[dev].mrd_handle = NULL;

    ak_thread_sem_destroy(&md_ctrl[dev].send_sem);
    ak_thread_mutex_destroy(&md_result[dev].lock);

    return AK_SUCCESS;
}

/**
 * ak_md_get_version - get md version
 * return: version string
 * notes:
 */
const char* ak_md_get_version(void)
{
    return md_version;
}

/**
 * ak_md_get_mrd_version - get mrd version
 * return: version string
 * notes:
 */
const char* ak_md_get_mrd_version(void)
{
    return ak_mrd_get_version();
}

/* end of file */
