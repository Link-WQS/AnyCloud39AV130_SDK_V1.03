#include <stdlib.h>
#include <string.h>

#include "ak_log.h"
#include "ak_mem.h"
#include "ak_common.h"
#include "ak_thread.h"
#include "ak_vi.h"
#include "ak_vpss.h"
#include "ak_svp_md.h"
#include "ak_mrd.h"

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
    int md_fps;	//msecond. interval time data to detect
    void *mrd_handle;
} ;

static struct hw_md_result md_result[VIDEO_DEV_MUX] = {0};
static struct hw_md_ctrl md_ctrl[VIDEO_DEV_MUX] = {0};
static int md_init_flag[VIDEO_DEV_MUX] = {0};

/**
 * svp_md_get_stat: get motion detection stat params
 * @dev[IN]: dev id
 * @md[OUT]: md params
 * return: 0 - success; other error code;
 * notes:
 */
static int svp_md_get_stat(int dev, struct vpss_md_info *md)
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
 * svp_md_check: judge function
 * @md_ctrl[IN]: md ctrl struct pointer
 * return: 0 no move, 1 has move
 * notes:
 */
static int svp_md_check(struct hw_md_ctrl *md_ctrl)
{
    int ret = 0;
    int hasmove = 0;
    struct vpss_md_info md = {{{0}}};
    MRD_RECTANGLE boxes[MAX_CAP] = {{0}};
    int dev = md_ctrl->dev;

    if (svp_md_get_stat(dev, &md))
    {
        ak_print_warning_ex(MODULE_ID_MD, "get md stat fail.\n");
        return 0;
    }

    ret = ak_mrd(md_ctrl->mrd_handle, md.stat, VPSS_MD_DIMENSION_V_MAX, boxes);

    if (ret > 0)
    {
        //ak_print_normal(MODULE_ID_MD, "box 0: [%d, %d, %d, %d]\n",
            //boxes[0].left, boxes[0].top, boxes[0].right, boxes[0].bottom);
        ak_thread_mutex_lock(&md_result[dev].lock);

        memset(&md_result[dev].ret_info.boxes, 0, MAX_BOX_NUM * sizeof(struct md_box));
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

    return hasmove;
}

/**
 * svp_md_thread - move detect thread, do detect thing here
 * compare move detect success over 2 times, then one detect message is emit really
 */
static void *svp_md_thread(void *arg)
{
    int detect_interval = 0;
    struct ak_timeval md_tv = {0}, now_tv = {0};
    struct ak_date date = {0};
    int move_count = 0;

    ak_thread_set_name("svp_md");

    ak_print_normal_ex(MODULE_ID_MD, "thread id : %ld\n", ak_thread_get_tid());

    int dev = *(int*)arg;

    while (md_ctrl[dev].thread_stat)
    {
        detect_interval = 1000/md_ctrl[dev].md_fps;

        /*
        * stop mode ,don't run to check, wait here
        */
        do {
            ak_sleep_ms(detect_interval);
            if(	MD_THREAD_STOP == md_ctrl[dev].thread_stat){
                move_count = 0;
            }
        } while(MD_THREAD_STOP == md_ctrl[dev].thread_stat);

        if (svp_md_check(&md_ctrl[dev]))
        {
            move_count++;
            if(move_count >= 1)
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
             * alarm module do not take away md result and it is over 4 second,
             * clean it
             */
            ak_thread_mutex_lock(&md_result[dev].lock);
            if (1 == md_result[dev].ret_info.result)
            {
                ak_get_ostime(&now_tv);
                if ((now_tv.sec > (md_tv.sec + 4)) || (now_tv.sec < md_tv.sec ))
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
 * ak_svp_md_init - md init .
 * @param[IN]:  svp_md_param
 * return: 0 - success; other error code;
 */
int ak_svp_md_init(struct svp_md_param *param)
{
    if(param == NULL)
    {
        ak_print_error_ex(MODULE_ID_MD, "Param is NULL!\n");
        return ERROR_TYPE_INVALID_ARG;
    }
    int dev = param->dev_id;
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

    /* init the ak_mrd lib*/
    md_ctrl[dev].mrd_handle = ak_mrd_init();
    if (NULL == md_ctrl[dev].mrd_handle)
    {
        ak_print_error_ex(MODULE_ID_MD, "ak_mrd_init failed!\n");
        return AK_FAILED;
    }

    md_ctrl[dev].dev = dev;
    md_ctrl[dev].thread_stat = MD_THREAD_STOP;
    md_ctrl[dev].md_fps = param->md_fps;
    ak_thread_sem_init(&md_ctrl[dev].send_sem, 0);

    /* set the mrd to single frame mode */
    ak_mrd_set_mode(md_ctrl[dev].mrd_handle, 1);

    /* set mrd filter param*/
    ak_mrd_set_filters(md_ctrl[dev].mrd_handle, param->flt_big, param->flt_small);

    /* set motion region size*/
    ak_mrd_set_motion_region_size(md_ctrl[dev].mrd_handle, param->move_size_min, param->move_size_max);

    /* set mrd floating param */
    ak_mrd_set_floating_wadding_params(md_ctrl[dev].mrd_handle, 3, 1, 3, 36, 2, 2);

    /* create the svp_md_process*/
    if (ak_thread_create(&md_ctrl[dev].md_tid, svp_md_thread,
    &(md_ctrl[dev].dev), ANYKA_THREAD_NORMAL_STACK_SIZE, -1))
    {
        ak_print_error_ex(MODULE_ID_MD, "create move detect thread failed.\n");
        return AK_FAILED;
    }

    md_init_flag[dev] = 1;
    return AK_SUCCESS;
}

/**
 * submit - get md result.
 * @dev[IN]:  dev id
 * @ret_info[OUT]:	result info
 * return: 0 - success; other error code;
 */
int get_svp_md_result(int dev, struct md_result_info *ret_info)
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
 * ak_svp_md_get_result - get md result.
 * @dev[IN]:  dev id
 * @ret_info[OUT]:  result info
 * @timeout[IN]:  timeout <0  block mode, ==0 non-block mode, >0 waittime
 * return: 0 - success;  other error code;
 */
int ak_svp_md_get_result(int dev, struct md_result_info *ret_info, int timeout)
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
        return AK_FAILED;
    }

    if (MD_THREAD_RUN != md_ctrl[dev].thread_stat) {
        ak_print_error_ex(MODULE_ID_MD, "fail,not run\n");
        return AK_FAILED;
    }

    /*md have been triggered*/
    ak_thread_mutex_lock(&md_result[dev].lock);
    if (md_result[dev].ret_info.result)
    {
        get_svp_md_result(dev, ret_info);
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
                get_svp_md_result(dev, ret_info);
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
            get_svp_md_result(dev, ret_info);
            ak_thread_mutex_unlock(&md_result[dev].lock);
            return AK_SUCCESS;
        }
        ak_thread_mutex_unlock(&md_result[dev].lock);
    }

    return AK_SUCCESS;
}

/**
 * ak_svp_md_enable - start or stop md .
 * @dev[IN]:  dev id
 * @enable[IN]:  [0,1],  0 -> stop md; 1 -> start md
 * return: 0 - success; other error code;
 */
int ak_svp_md_enable(int dev, int enable)
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
        return AK_FAILED;
    }

    ak_print_normal_ex(MODULE_ID_MD, "enable:%d\n", enable);

    if ((1 == enable) && (MD_THREAD_STOP == md_ctrl[dev].thread_stat))
    {
        md_ctrl[dev].thread_stat = MD_THREAD_RUN;
    }
    else if ((0 == enable) && (MD_THREAD_RUN == md_ctrl[dev].thread_stat))
    {
        md_ctrl[dev].thread_stat = MD_THREAD_STOP;
    }

    return AK_SUCCESS;
}

/*
 * ak_svp_md_uninit - free md resource and quit md .
 * @dev[IN]:  dev id
 * return: 0 - success; other error code;
 */
int ak_svp_md_uninit(int dev)
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
        return AK_FAILED;
    }

    md_init_flag[dev] = 0;

    if (MD_THREAD_STOP == md_ctrl[dev].thread_stat)
    {
        md_ctrl[dev].thread_stat = MD_THREAD_EXIT;
    }
    else
        md_ctrl[dev].thread_stat = MD_THREAD_EXIT;

    ret = ak_thread_join(md_ctrl[dev].md_tid);
    ak_print_normal_ex(MODULE_ID_MD, "disable, %s\n", ret ? "failed" : "success");

    ak_mrd_destroy(md_ctrl[dev].mrd_handle);
    md_ctrl[dev].mrd_handle = NULL;

    ak_thread_sem_destroy(&md_ctrl[dev].send_sem);
    ak_thread_mutex_destroy(&md_result[dev].lock);

    return AK_SUCCESS;
}

/* end of file */
