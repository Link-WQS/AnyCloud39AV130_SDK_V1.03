#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include <stdlib.h>

#include "ak_mt_inner.h"

static char mt_lib_version[] = "MT Lib V2.0.03";

/*********************************************/
/*********************************************/
/*********************************************/
// external interface function

void* ak_mt_init(void)
{
    TRACKING_PARAM *param = (TRACKING_PARAM *)malloc(sizeof(TRACKING_PARAM));

    if (NULL == param)
    {
        printf("memory allocation failed!\n");
        return NULL;
    }

    printf("########%s########\n", mt_lib_version);

    memset(param, 0, sizeof(TRACKING_PARAM));

    param->rotate_h_enable = 1;
    param->rotate_v_enable = 0;
    param->size_min = 1;
    param->size_max = 768;
    param->boundary_size_h = 10;
    param->boundary_size_v = 3;

    param->fps = 25;
    param->max_invalid_fm = MAX_INVALID_FRAMES;
    param->max_decision_fm = MAX_DECISION_FRAMES;
    param->max_decision_fm_backup = param->max_decision_fm;

    param->agl_h = 0;
    param->agl_v = 0;

    param->main_decision_dir_v = 1;

    param->mt_factor = 256;

    param->invalid_dis = INVALID_DISTANCE;
    param->undetected_num = 0;

    param->mt_handle = param;

    param->min_agl_h = 6;
    param->max_agl_h = 65;
    param->min_agl_v = 2;
    param->max_agl_v = 65;

    param->fov_h = 90;
    param->fov_v = 60;
    ak_mt_set_field_angles((void *)param, param->fov_h, param->fov_v, MD_SIZE, 24);

    return param;
}

int ak_mt_destroy(void *mt_handle)
{
    if (NULL == mt_handle)
    {
        printf("ak_mt_destroy: mt_handle is NULL\n");
        return -1;
    }

    TRACKING_PARAM *param = (TRACKING_PARAM *)mt_handle;

    if (mt_handle != param->mt_handle)
    {
        printf("ak_mt_destroy: input mt_handle is error\n");
        return -1;
    }
    free(param);

    return 0;
}

int ak_mt_reset_tracking_param(void *mt_handle)
{
    if (NULL == mt_handle)
    {
        printf("ak_mt_reset_tracking_param: mt_handle is NULL\n");
        return -1;
    }

    TRACKING_PARAM *param = (TRACKING_PARAM *)mt_handle;

    if (mt_handle != param->mt_handle)
    {
        printf("ak_mt_reset_tracking_param: input mt_handle is error\n");
        return -1;
    }

    param->invalid_rect_num = 0;       // no recorded invaild motion region
    param->valid_rect_num = 0;         // no recorded valid motion region
    param->valid_rect_left_num = 0;    // no recorded valid motion region on left, right, botoom, and top of valid video frame center
    param->valid_rect_right_num = 0;
    param->valid_rect_botoom_num = 0;
    param->valid_rect_top_num = 0;
    param->valid_rect_center_num = 0;  // no recorded valid motion region in the center of valid video frame
    param->current_max_invalid_rect_dis = 0;
    param->decision_enalbe = 0;  // disable change valid motion region
    param->undetected_num = 0;
    param->valid_rect_num_v = 0;

    return 0;
}

char *ak_mt_get_version()
{
    return mt_lib_version;
}

int ak_mt_set_fps(void *mt_handle, short fps)
{
    if (NULL == mt_handle)
    {
        printf("ak_mt_set_fps: mt_handle is NULL\n");
        return -1;
    }

    if (fps <= 0)
    {
        printf("ak_mt_set_fps: The value of fps %d is negative, use 25 instead\n", fps);
        fps = 25;
    }

    TRACKING_PARAM *param = (TRACKING_PARAM *)mt_handle;

    if (mt_handle != param->mt_handle)
    {
        printf("ak_mt_set_fps: input mt_handle is error\n");
        return -1;
    }

    param->fps = fps;

    return 0;
}

int ak_mt_set_max_invalid_frames(void *mt_handle, unsigned short nFrames)
{
    if (NULL == mt_handle)
    {
        printf("ak_mt_set_max_invalid_frames: mt_handle is NULL\n");
        return -1;
    }

    if (nFrames > INVALID_FRAMES_CAP)
    {
        printf("ak_mt_set_max_invalid_frames: input %d is out of capacity, now use the max accessible value %d instead\n",
            nFrames, INVALID_FRAMES_CAP);
        nFrames = INVALID_FRAMES_CAP;
    }

    if (nFrames < DECISION_FRAMES_CAP)
    {
        printf("ak_mt_set_max_invalid_frames: input %d is too small, now use the min accessible value %d instead\n",
            nFrames, DECISION_FRAMES_CAP);
        nFrames = DECISION_FRAMES_CAP;
    }

    TRACKING_PARAM *param = (TRACKING_PARAM *)mt_handle;

    if (mt_handle != param->mt_handle)
    {
        printf("ak_mt_set_max_invalid_frames: input mt_handle is error\n");
        return -1;
    }

    param->max_invalid_fm = nFrames;

    return 0;
}

int ak_mt_set_max_decision_frames(void *mt_handle, unsigned short nFrames)
{
    if (NULL == mt_handle)
    {
        printf("ak_mt_set_max_decision_frames: mt_handle is NULL\n");
        return -1;
    }

    if (nFrames > DECISION_FRAMES_CAP)
    {
        printf("ak_mt_set_max_decision_frames: input %d is out of capacity, now use the max accessible value %d instead\n",
            nFrames, DECISION_FRAMES_CAP);
        nFrames = DECISION_FRAMES_CAP;
    }

    if (nFrames < 3)
    {
        printf("ak_mt_set_max_decision_frames: input %d is too small, now use the min accessible value 3 instead\n",
            nFrames);
        nFrames = 3;
    }

    TRACKING_PARAM *param = (TRACKING_PARAM *)mt_handle;

    if (mt_handle != param->mt_handle)
    {
        printf("ak_mt_set_max_decision_frames: input mt_handle is error\n");
        return -1;
    }

    param->max_decision_fm = nFrames;
    param->max_decision_fm_backup = param->max_decision_fm;

    return 0;
}

int ak_mt_set_invalid_distance(void *mt_handle, unsigned char dis)
{
    if (NULL == mt_handle)
    {
        printf("ak_mt_set_invalid_distance: mt_handle is NULL\n");
        return -1;
    }

    TRACKING_PARAM *param = (TRACKING_PARAM *)mt_handle;

    if (mt_handle != param->mt_handle)
    {
        printf("ak_mt_set_invalid_distance: input mt_handle is error\n");
        return -1;
    }

    param->invalid_dis = dis;

    return 0;
}

int ak_mt_set_camera_rotation_factor(void *mt_handle, int factor)
{
    if (NULL == mt_handle)
    {
        printf("ak_mt_set_camera_rotation_factor: mt_handle is NULL\n");
        return -1;
    }

    TRACKING_PARAM *param = (TRACKING_PARAM *)mt_handle;

    if (mt_handle != param->mt_handle)
    {
        printf("ak_mt_set_camera_rotation_factor: input mt_handle is error\n");
        return -1;
    }

    if (factor == 0)
    {
        factor = 256;
    }

    param->mt_factor = factor;

    return 0;
}

int ak_mt_open_camera_rotation_switch_v(void *mt_handle)
{
    if (NULL == mt_handle)
    {
        printf("ak_mt_open_camera_rotation_switch_v: mt_handle is NULL\n");
        return -1;
    }

    TRACKING_PARAM *param = (TRACKING_PARAM *)mt_handle;

    if (mt_handle != param->mt_handle)
    {
        printf("ak_mt_open_camera_rotation_switch_v: input mt_handle is error\n");
        return -1;
    }

    param->rotate_v_enable = 1;

    return 0;
}

int ak_mt_close_camera_rotation_switch_v(void *mt_handle)
{
    if (NULL == mt_handle)
    {
        printf("ak_mt_close_camera_rotation_switch_v: mt_handle is NULL\n");
        return -1;
    }

    TRACKING_PARAM *param = (TRACKING_PARAM *)mt_handle;

    if (mt_handle != param->mt_handle)
    {
        printf("ak_mt_close_camera_rotation_switch_v: input mt_handle is error\n");
        return -1;
    }

    param->rotate_v_enable = 0;

    return 0;
}

int ak_mt_reverse_main_decision_direction_v(void *mt_handle)
{
    if (NULL == mt_handle)
    {
        printf("ak_mt_reverse_main_decision_direction_v: mt_handle is NULL\n");
        return -1;
    }

    TRACKING_PARAM *param = (TRACKING_PARAM *)mt_handle;

    if (mt_handle != param->mt_handle)
    {
        printf("ak_mt_reverse_main_decision_direction_v: input mt_handle is error\n");
        return -1;
    }

    param->main_decision_dir_v *= -1;

    return 0;
}

int ak_mt_set_motion_region_params(void *mt_handle, short len, short valid_size_min,  short valid_size_max,
    short boundary_size_h, short boundary_size_v)
{
    if (NULL == mt_handle)
    {
        printf("ak_mt_set_motion_region_params: mt_handle is NULL\n");
        return -1;
    }

    TRACKING_PARAM *param = (TRACKING_PARAM *)mt_handle;

    if (mt_handle != param->mt_handle)
    {
        printf("ak_mt_set_motion_region_params: input mt_handle is error\n");
        return -1;
    }

    if (len < 1 || len > MD_SIZE)
    {
        printf("ak_mt_set_motion_region_params: input %d is out of range ,the avaliable value of parameter len is 1 to %d\n", len , MD_SIZE);
        return -1;
    }

    if (valid_size_min > valid_size_max)
    {
        valid_size_min = valid_size_min + valid_size_max;
        valid_size_max = valid_size_min - valid_size_max;
        valid_size_min = valid_size_min - valid_size_max;
    }

    if (valid_size_max < 1)
    {
        valid_size_max = 1;
    }

    if (boundary_size_h < 0)
    {
        boundary_size_h *= -1;
    }

    if (boundary_size_h*2 > MD_SIZE)
    {
        boundary_size_h = MD_SIZE/2;
    }

    if (boundary_size_v < 0)
    {
        boundary_size_v *= -1;
    }

    if (boundary_size_v*2 > len)
    {
        boundary_size_v = len/2;
    }

    param->size_min = valid_size_min;
    param->size_max = valid_size_max;
    param->boundary_size_h = boundary_size_h;
    param->boundary_size_v = boundary_size_v;

    return 0;
}

int ak_mt_set_field_angles(void *mt_handle, unsigned char fov_h, unsigned char fov_v, unsigned short block_num_h, unsigned short block_num_v)
{
    if (NULL == mt_handle)
    {
        printf("ak_mt_set_field_angles: mt_handle is NULL\n");
        return -1;
    }

    TRACKING_PARAM *param = (TRACKING_PARAM *)mt_handle;

    if (mt_handle != param->mt_handle)
    {
        printf("ak_mt_set_field_angles: input mt_handle is error\n");
        return -1;
    }

    if ((fov_h >= 180) || (fov_v >= 180) || (fov_h == 0) || (fov_v == 0))
    {
        printf("ak_mt_set_field_angles: input field angle fov_h=%d or fov_v=%d is out of range (0, 180)!", fov_h, fov_v);
        return -1;
    }

    if ((block_num_h > MD_SIZE) || (block_num_v > MD_SIZE) || (block_num_h == 0) || (block_num_v == 0))
    {
        printf("ak_mt_set_field_angles: input block_num_h=%d or block_num_v=%d is out of range (0, %d]!", block_num_h, block_num_v, MD_SIZE);
        return -1;
    }

    double dis_h = block_num_h / tan(fov_h * 1.0 / 360 * PI);
    double dis_v = block_num_v / tan(fov_v * 1.0 / 360 * PI);

    for (int k = 0; k <= MD_SIZE * 2; k++)
    {
        param->block_angles_h[k] = floor(atan(k / dis_h) / PI * 180);
        param->block_angles_v[k] = floor(atan(k / dis_v) / PI * 180);
    }

    param->fov_h = fov_h;
    param->fov_v = fov_v;

    return 0;
}

int ak_mt_set_rotation_range(void *mt_handle, unsigned char min_agl_h, unsigned char max_agl_h, unsigned char min_agl_v, unsigned char max_agl_v)
{
    if (NULL == mt_handle)
    {
        printf("ak_mt_set_rotation_range: mt_handle is NULL\n");
        return -1;
    }

    TRACKING_PARAM *param = (TRACKING_PARAM *)mt_handle;

    if (mt_handle != param->mt_handle)
    {
        printf("ak_mt_set_rotation_range: input mt_handle is error\n");
        return -1;
    }

    if ((min_agl_h >= 90) || (min_agl_v >= 90) || (max_agl_h >= 90) || (max_agl_v >= 90))
    {
        printf("ak_mt_set_rotation_range: min_agl_h=%d or min_agl_v=%d or max_agl_h=%d or max_agl_v=%d is out of range [0, 90)\n",
            min_agl_h, min_agl_v, max_agl_h, max_agl_v);
        return -1;
    }

    if (min_agl_h > max_agl_h)
    {
        printf("ak_mt_set_rotation_range: min_agl_h=%d is bigger than max_agl_h=%d\n", min_agl_h, max_agl_h);
        return -1;
    }

    if (min_agl_v > max_agl_v)
    {
        printf("ak_mt_set_rotation_range: min_agl_v=%d is bigger than max_agl_v=%d\n", min_agl_v, max_agl_v);
        return -1;
    }

    param->min_agl_h = min_agl_h;
    param->max_agl_h = max_agl_h;
    param->min_agl_v = min_agl_v;
    param->max_agl_v = max_agl_v;

    return 0;
}

int ak_mt_tracking(void *mt_handle, short len, MT_RECTANGLE *rect,MT_POINT *rt_angle)
{
    if (NULL == mt_handle)
    {
        printf("ak_mt_tracking: mt_handle is NULL\n");
        return -1;
    }

    if (NULL == rt_angle)
    {
        printf("ak_mt_tracking: rt_angle pointer is NULL\n");
        return -1;
    }

    if (len < 1)
    {
        len = 1;
    }

    if (len > MD_SIZE)
    {
        len = MD_SIZE;
    }

    TRACKING_PARAM *param = (TRACKING_PARAM *)mt_handle;

    if (mt_handle != param->mt_handle)
    {
        printf("ak_mt_tracking: input mt_handle is error\n");
        return -1;
    }

    rt_angle->px = 0;
    rt_angle->py = 0;

    int ret = 0;
    if (rect->width && rect->height
        && (rect->width*rect->height>=param->size_min) && (rect->width*rect->height<=param->size_max))
        ret = 1;

    if (1 == ret)  // motion region is detected
    {
        MT_POINT distance = {0};
        ret = mt_mrd_Isoutofcenter(param, rect, &distance, len,
            param->boundary_size_h, param->boundary_size_v);

        if (1 == ret) // motion region is out of video frame center
        {
            param->valid_rect_num_v = 0;
            param->decision_enalbe = 1; // motion region change switch is open

            /* record  valid motion region and corresponding count */
            if (distance.px > 0)
            {
                if (param->valid_rect_right_num > 0)
                {
                    param->valid_rect_right_num = 0;
                    param->valid_rect_num = 0;
                }

                param->valid_rect_left_num += 1;
            }
            else if (distance.px < 0)
            {
                if (param->valid_rect_left_num > 0)
                {
                    param->valid_rect_left_num = 0;
                    param->valid_rect_num = 0;
                }

                param->valid_rect_right_num += 1;
            }

            mt_member_copy(&param->valid_rect[param->valid_rect_num], rect);
            param->valid_rect_num += 1;

            ret = mt_get_rotation_angle(param, len);

            /* determine the rotation direction */
            // max_decision_frame is accessible or motion region is ready to leave certain areas
            if (1 == ret)
            {
                rt_angle->py = param->agl_v;

                if ((param->valid_rect_left_num > param->valid_rect_right_num)
                    && (param->valid_rect_left_num > param->valid_rect_center_num))
                {
                    rt_angle->px = param->agl_h;
                }
                if ((param->valid_rect_right_num > param->valid_rect_left_num)
                    && (param->valid_rect_right_num > param->valid_rect_center_num))
                {
                    rt_angle->px = -1 * param->agl_h;
                }

                /* if  vertical rotation are forbidden, rotate camera in horizontal direction only */
                if (0 == param->rotate_v_enable)
                    rt_angle->py = 0;
                if (0 == param->rotate_h_enable)
                    rt_angle->px = 0;
                if (rt_angle->px || rt_angle->py)
                {
                    //printf("camera is ready to track motion region!\n");
                    if (param->max_decision_fm == param->max_decision_fm_backup)
                    {
                        param->max_decision_fm = param->max_decision_fm - 2;
                    }

                    //printf("inner output, angle_h: %d, angle_v: %d\n", rt_angle->px, rt_angle->py);
                    return 1;
                }
                else
                {
                    //printf("fake out of center\n");
                    param->decision_enalbe = 0;
                    param->valid_rect_num = 0;
                    param->valid_rect_left_num = 0;
                    param->valid_rect_right_num = 0;
                    param->valid_rect_botoom_num = 0;
                    param->valid_rect_top_num = 0;
                    param->valid_rect_center_num = 0;
                    return 0;
                }
            }
        }
        /* if motion region is not out of video frame center, do which follows */
        else
        {
            /* if vertical rotation switch is opened, do motion tracking in vertical direction */
            if (1 == param->rotate_v_enable)
            {
                int i = 0;
                int weight[DECISION_FRAMES_CAP_V] = {0};
                weight[1] = 2;
                weight[2] = 3;
                weight[3] = 5;
                MT_RECTANGLE tmp_rect = {{0}};

                mt_member_copy(&param->valid_rect_v[param->valid_rect_num_v], rect);
                param->valid_rect_num_v += 1;

                if (param->valid_rect_num_v >= DECISION_FRAMES_CAP_V)
                {
                    int t_py = 0;
                    int t_height = 0; // if use MT_RECTANGLE originally, the value will overflow
                    for (i = 1; i < param->valid_rect_num_v - 1; i++)
                    {
                        t_py += weight[i] * param->valid_rect_v[i].pt.py;
                        t_height += weight[i] * param->valid_rect_v[i].height;
                    }
                    tmp_rect.pt.py = t_py / 10;
                    tmp_rect.height = t_height / 10;

                    ret = mt_get_rotation_angle_v(param, len, &tmp_rect);

                    if (ret)
                    {
                        rt_angle->px = 0;
                        rt_angle->py = param->agl_v;

                        if (rt_angle->py)
                        {
                            return 1;
                        }
                    }
                    param->valid_rect_num_v = 0;
                }
            }

            /* once motion region change switch opened, motion region which is not out of center is also recorded */
            if (1 == param->decision_enalbe)
            {
                param->decision_enalbe = 0;
                param->valid_rect_num = 0;
                param->valid_rect_left_num = 0;
                param->valid_rect_right_num = 0;
                param->valid_rect_botoom_num = 0;
                param->valid_rect_top_num = 0;
                param->undetected_num = 0;
            }
            /* if invalid motion region has not been detected, record the first invalid motion region */
            if (param->invalid_rect_num == 0)
            {
                mt_member_copy(&param->invalid_rect[param->invalid_rect_num], rect);
                param->invalid_rect_num += 1;
            }
            /* calculate the max distance between recorded invalid motion region,
            once the value is greater than given threshold, recorded invalid motion region count is reset to zero */
            else
            {
                mt_tracking_max_distance(param, rect);
                mt_member_copy(&param->invalid_rect[param->invalid_rect_num], rect);
                param->invalid_rect_num += 1;
                if (param->current_max_invalid_rect_dis > param->invalid_dis)
                {
                    //printf("valid distance is accessible, invalid rects is reset\n");
                    param->invalid_rect_num = 0;
                    param->current_max_invalid_rect_dis = 0;
                }
            }

            /* once invalid motion region count is greater than given threshold, camera and tracking parameters are reset*/
            if (param->invalid_rect_num >= param->max_invalid_fm)
            {
                rt_angle->px = 0;
                rt_angle->py = 0;

                if (param->max_decision_fm != param->max_decision_fm_backup)
                {
                    param->max_decision_fm = param->max_decision_fm_backup;
                }

                return 2;
            }
        }
    }
    /* motion region is not deteted, so increase the count of invalid motion region */
    else
    {
        param->valid_rect_num_v = 0;

        if (1 == param->decision_enalbe)
        {
            if (param->undetected_num < 2 * param->max_decision_fm)
            {
                param->undetected_num++;
            }
            else
            {
                param->decision_enalbe = 0;
                param->valid_rect_num = 0;
                param->valid_rect_left_num = 0;
                param->valid_rect_right_num = 0;
                param->valid_rect_botoom_num = 0;
                param->valid_rect_top_num = 0;
                param->undetected_num = 0;
            }
        }

        mt_member_copy(&param->invalid_rect[param->invalid_rect_num], rect);
        param->invalid_rect_num += 1;
        if (param->invalid_rect_num >= param->max_invalid_fm)
        {
            rt_angle->px = 0;
            rt_angle->py = 0;

            if (param->max_decision_fm != param->max_decision_fm_backup)
            {
                param->max_decision_fm = param->max_decision_fm_backup;
            }

            return 2;
        }
    }

    return 0;
}


/*********************************************/
/*********************************************/
/*********************************************/
// inner interface function

void mt_member_copy(MT_RECTANGLE *rect1, MT_RECTANGLE *rect2)
{
    rect1->pt.px = rect2->pt.px;
    rect1->pt.py = rect2->pt.py;
    rect1->height = rect2->height;
    rect1->width = rect2->width;
}

int mt_mrd_Isoutofcenter(TRACKING_PARAM * param, MT_RECTANGLE *rect, MT_POINT *distance, short len, short size_h, short size_v)
{
    signed char pos_x = rect->pt.px+rect->width/2; // horizontal position of motion region center, if the type of pos_x is unsigned char, the value of distanece->px will always be bigger than zero

    /* distance between detecting motion region center and video frame center */
    if ((pos_x>size_h) &&  (pos_x<=MD_SIZE-size_h))
        distance->px = 0;
    else
        distance->px = pos_x - MD_SIZE/2;
    if (distance->px < 0)
        distance->px = distance->px - 1;

    if ((distance->px!=0) || (distance->py!=0))
        return 1;
    else
        return 0;
}

void mt_tracking_max_distance(TRACKING_PARAM *param, MT_RECTANGLE *rect)
{
    unsigned short tmp_dis = 0;
    unsigned short i;
    /* calculate distance between detecting motion region and recorded invalid motion region, and return the max distance */
    for (i=0; i<param->invalid_rect_num; i++)
    {
        if (param->invalid_rect[i].width == 0)
        {
            tmp_dis = 0;
        }
        else
        {
            tmp_dis = (param->invalid_rect[i].pt.px-rect->pt.px)*(param->invalid_rect[i].pt.px-rect->pt.px)
                + (param->invalid_rect[i].pt.py-rect->pt.py)*(param->invalid_rect[i].pt.py-rect->pt.py);
        }
        if (tmp_dis > param->current_max_invalid_rect_dis)
        {
            param->current_max_invalid_rect_dis = tmp_dis;
        }
    }
}

int mt_rect_optimization(TRACKING_PARAM *param)
{
    int i;
    MT_RECTANGLE *pre_rect = &param->valid_rect[param->valid_rect_num - 2];
    MT_RECTANGLE *after_rect = &param->valid_rect[param->valid_rect_num - 1];

    // the last recorded motion region is part of his nearest record
    if ((after_rect->pt.px >= pre_rect->pt.px)
        && (after_rect->pt.px + after_rect->width <= pre_rect->pt.px + pre_rect->width)
        && (after_rect->pt.py >= pre_rect->pt.py)
        && (after_rect->pt.py + after_rect->height <= pre_rect->pt.py + pre_rect->height)
        && (after_rect->height * after_rect->width * 10 < pre_rect->height * pre_rect->width * 5))
    {
        return -1;
    }

    for (i = 1; i < param->valid_rect_num; i++)
    {
        pre_rect = &param->valid_rect[param->valid_rect_num - i - 1];
        after_rect = &param->valid_rect[param->valid_rect_num - i];

        // there is no intersection between the latter motion region record and his nearest previous record
        if ((after_rect->pt.px >= pre_rect->pt.px + pre_rect->width)
            || (after_rect->pt.px + after_rect->width <= pre_rect->pt.px)
            || (after_rect->pt.py >= pre_rect->pt.py + pre_rect->height)
            || (after_rect->pt.py + after_rect->height <= pre_rect->pt.py))
        {
            return param->valid_rect_num - i;
        }
    }
    return 0;
}

int mt_get_rotation_angle(TRACKING_PARAM *param, short len)
{
    static const short angle_time[91] =
    {
        0, 19, 29, 39, 49, 59, 79, 89, 99, 109,
        119, 129, 139, 149, 169, 179, 189, 199, 209, 219,
        229, 249, 259, 269, 279, 289, 299, 309, 329, 339,
        349, 359, 369, 379, 389, 409, 419, 429, 439, 449,
        459, 469, 479, 499, 509, 519, 529, 539, 549, 559,
        579, 589, 599, 609, 619, 629, 639, 659, 669, 679,
        689, 699, 709, 719, 739, 749, 759, 769, 779, 789,
        799, 819, 829, 839, 849, 859, 869, 879, 899, 909,
        919, 929, 939, 949, 959, 969, 989, 999, 1009, 1019,
        1029
    };

    MT_POINT distance = {0};
    int speed = 0;
    int ret = -1;
    short pos_x = 0;
    short pos_y = 0;
    int tmp_dis = 0;
    int total_dis = 0;

    if (param->valid_rect_num >= 2)
    {
        ret = mt_rect_optimization(param);
        MT_RECTANGLE tmp_rect = {{0}};

        if (ret == -1)
        {
            if (param->valid_rect_num == 2)
            {
                mt_member_copy(&tmp_rect, &param->valid_rect[0]);
            }
            else if (param->valid_rect_num == 3)
            {
                tmp_rect.pt.py = (param->valid_rect[0].pt.py * 4 + param->valid_rect[1].pt.py * 6) / 10;
                tmp_rect.height = (param->valid_rect[0].height * 4 + param->valid_rect[1].height * 6) / 10;
            }
            else
            {
                tmp_rect.pt.py = (param->valid_rect[param->valid_rect_num - 2].pt.py * 5
                    + param->valid_rect[param->valid_rect_num - 3].pt.py * 3
                    + param->valid_rect[param->valid_rect_num - 4].pt.py * 2) / 10;
                tmp_rect.height = (param->valid_rect[param->valid_rect_num - 2].height * 5
                    + param->valid_rect[param->valid_rect_num - 3].height * 3
                    + param->valid_rect[param->valid_rect_num - 2].height * 2) / 10;
            }
        }
        else if (ret == param->valid_rect_num - 1)
        {
            mt_member_copy(&tmp_rect, &param->valid_rect[param->valid_rect_num - 1]);
        }
        else
        {
            if (param->valid_rect_num == 2)
            {
                tmp_rect.pt.py = (param->valid_rect[0].pt.py * 4 + param->valid_rect[1].pt.py * 6) / 10;
                tmp_rect.height = (param->valid_rect[0].height * 4 + param->valid_rect[1].height * 6) / 10;
            }
            else
            {
                tmp_rect.pt.py = (param->valid_rect[param->valid_rect_num - 2].pt.py * 5
                    + param->valid_rect[param->valid_rect_num - 3].pt.py * 3
                    + param->valid_rect[param->valid_rect_num - 4].pt.py * 2) / 10;
                tmp_rect.height = (param->valid_rect[param->valid_rect_num - 2].height * 5
                    + param->valid_rect[param->valid_rect_num - 3].height * 3
                    + param->valid_rect[param->valid_rect_num - 2].height * 2) / 10;
            }
        }
        mt_get_rotation_angle_v(param, len, &tmp_rect);
    }

    if (param->valid_rect_left_num > param->valid_rect_right_num)
    {
        pos_x = param->valid_rect[param->valid_rect_left_num-1].pt.px
            + param->valid_rect[param->valid_rect_left_num-1].width/2;

        if (param->valid_rect[param->valid_rect_left_num-1].width%2)
        {
            distance.px = 2*pos_x - MD_SIZE - 1;
        }
        else
        {
            distance.px = 2*pos_x - MD_SIZE - 2;
        }

        if ((ret != -1) && (ret != param->valid_rect_num - 1))
        {
            pos_y = param->valid_rect[ret].pt.px
                + param->valid_rect[ret].width/2;
            speed = (pos_x - pos_y)*2;
        }
        else
        {
            speed = 0;
        }
    }
    else if(param->valid_rect_right_num > param->valid_rect_left_num)
    {
        pos_x = param->valid_rect[param->valid_rect_right_num-1].pt.px
            + param->valid_rect[param->valid_rect_right_num-1].width/2;

        if (param->valid_rect[param->valid_rect_right_num-1].width%2)
        {
            distance.px = MD_SIZE - pos_x*2 + 1;
        }
        else
        {
            distance.px = MD_SIZE - pos_x*2 + 2;
        }

        if ((ret != -1) && (ret != param->valid_rect_num - 1))
        {
            pos_y = param->valid_rect[ret].pt.px
                + param->valid_rect[ret].width/2;
            speed = (pos_y - pos_x)*2;
        }
        else
        {
            speed = 0;
        }
    }

    if (param->mt_factor < 0)
    {
        speed = 0;
    }

    if (speed == 0)
    {
        total_dis = distance.px;
        if ((total_dis >= MD_SIZE * 2 - 4) || (total_dis <= 10)
            || (param->valid_rect_num >= param->max_decision_fm))
        {
            total_dis = distance.px;
        }
        else
        {
            param->agl_v = 0;
            return 0;
        }
    }

    if (speed > 0)
    {
        total_dis = distance.px;

        if ((total_dis >= MD_SIZE*2 - 4) || (param->valid_rect_num >= param->max_decision_fm))
        {
            tmp_dis = ((speed << 8) * angle_time[param->block_angles_h[total_dis]]
                / (1000 / param->fps * (param->valid_rect_num - 1 - ret + param->undetected_num))) >> 8;
            while ((total_dis - tmp_dis < distance.px) && (total_dis < 64))
            {
                total_dis = total_dis + 1;
                tmp_dis = ((speed << 8) * angle_time[param->block_angles_h[total_dis]]
                    / (1000 / param->fps * (param->valid_rect_num - 1))) >> 8;
            }
        }
        else
        {
            param->agl_v = 0;
            return 0;
        }
    }

    if (speed < 0)
    {
        total_dis = distance.px;

        if ((total_dis <= 10) || (param->valid_rect_num >= param->max_decision_fm))
        {
            tmp_dis = ((speed << 8) * angle_time[param->block_angles_h[total_dis]]
                / (1000 / param->fps * (param->valid_rect_num - 1 - ret + param->undetected_num))) >> 8;
            while ((total_dis - tmp_dis > distance.px) && (total_dis > 0))
            {
                total_dis = total_dis - 1;
                tmp_dis = ((speed << 8) * angle_time[param->block_angles_h[total_dis]]
                    / (1000 / param->fps * (param->valid_rect_num - 1))) >> 8;
            }
        }
        else
        {
            param->agl_v = 0;
            return 0;
        }
    }

    total_dis = (total_dis * param->mt_factor) >> 8;

    if (total_dis > MD_SIZE*2)
        total_dis = MD_SIZE*2;
    if (total_dis < 0)
        total_dis = 0;
    param->agl_h = param->block_angles_h[total_dis];
    if (param->agl_h < param->min_agl_h)
        param->agl_h = 0 ;

    if (param->agl_h > param->max_agl_h)
    {
        param->agl_h = param->max_agl_h;
    }

    if (param->agl_h == 0)
    {
        param->agl_v = 0;
        return 0;
    }

    return 1;
}

int mt_get_rotation_angle_v(TRACKING_PARAM *param, short len, MT_RECTANGLE *rect)
{
    //printf("mt_get_rotation_angle_v\n");
    //printf("rect->pt.py = %d, rect->height = %d\n", rect->pt.py, rect->height);
    int flag ,state = 0, center = 0;

    param->agl_v = 0;
    if (param->main_decision_dir_v > 0)
    {
        if (param->boundary_size_v * 2 + rect->height > len)
        {
            state = 1;
            if (rect->pt.py < len / 2)
            {
                param->agl_v = param->block_angles_v[len - rect->pt.py * 2] - param->block_angles_v[len - param->boundary_size_v * 2];
            }
            else
            {
                param->agl_v = (param->block_angles_v[rect->pt.py * 2 - len] + param->block_angles_v[len - param->boundary_size_v * 2]) * -1;
            }
        }
        else if ((rect->pt.py < param->boundary_size_v) || (rect->pt.py + rect->height - 1 >= len - param->boundary_size_v))
        {
            center = rect->pt.py * 2 + rect->height;
            if (center >= len)
            {

                param->agl_v = param->block_angles_v[center - len] * -1;
            }
            else
            {

                param->agl_v = param->block_angles_v[len - center];
            }
        }
    }
    else
    {
        if (param->boundary_size_v * 2 + rect->height > len)
        {
            state = 1;
            if (rect->pt.py + rect->height - 1 > len / 2)
            {
                param->agl_v = param->block_angles_v[len - param->boundary_size_v * 2] - param->block_angles_v[(rect->pt.py + rect->height - 1) * 2 - len];
            }
            else
            {
                param->agl_v = param->block_angles_v[len - param->boundary_size_v * 2] + param->block_angles_v[len - (rect->pt.py + rect->height - 1) * 2];
            }
        }
        else if ((rect->pt.py < param->boundary_size_v) || (rect->pt.py + rect->height - 1 >= len - param->boundary_size_v))
        {
            center = rect->pt.py * 2 + rect->height;
            if (center >= len)
            {
                param->agl_v = param->block_angles_v[center - len] * -1;
            }
            else
            {
                param->agl_v = param->block_angles_v[len - center];
            }
        }
    }

    if (state)
    {
        return (param->agl_v == 0) ? 0:1;
    }

    flag = (param->agl_v >= 0) ? 1:-1;
    if (abs(param->agl_v) < param->min_agl_v)
    {
        param->agl_v = 0;
    }

    if (abs(param->agl_v) > param->max_agl_v)
    {
        param->agl_v = param->max_agl_v * flag;
    }

    return (param->agl_v == 0) ? 0:1;
}

// The origin version, discard now.
int mt_get_rotation_angle_v0(TRACKING_PARAM *param, short len, MT_RECTANGLE *rect)
{
    //printf("mt_get_rotation_angle_v\n");
    //printf("rect->pt.py = %d, rect->height = %d\n", rect->pt.py, rect->height);
    if (param->main_decision_dir_v > 0)
    {
        if (rect->pt.py <= len / 8)
        {
            //printf("condition 1\n");
            if (rect->pt.py + rect->height - 1 >= len - 2)
            {
                param->agl_v = 5;
            }
            else if (rect->pt.py + rect->height - 1 >= len * 3 / 4)
            {
                param->agl_v = 10;
            }
            else
            {
                param->agl_v = 25;
            }
            return 1;
        }
        else if (rect->pt.py + rect->height - 1 >= len - 2)
        {
            if (rect->pt.py >= len * 2 / 3 - 1)
            {
                param->agl_v = -25;
            }
            else if (rect->pt.py >= len / 2 - 1)
            {
                param->agl_v = -10;
            }
            else
            {
                param->agl_v = -5;
            }
            return 1;
        }
        else
        {
            //printf("condition 3\n");
            param->agl_v = 0;
            return 0;
        }
    }
    else
    {
        if (rect->pt.py + rect->height - 1  >= len * 7 / 8)
        {
            if (rect->pt.py <= 2)
            {
                param->agl_v = -5;
            }
            else if (rect->pt.py <= len / 4)
            {
                param->agl_v = -10;
            }
            else
            {
                param->agl_v = -25;
            }
            return 1;
        }
        else if (rect->pt.py <= 2)
        {
            if (rect->pt.py + rect->height - 1 <= len / 3 + 1)
            {
                param->agl_v = 25;
            }
            else if (rect->pt.py + rect->height - 1 <= len / 2 + 1)
            {
                param->agl_v = 10;
            }
            else
            {
                param->agl_v = 5;
            }
            return 1;
        }
        else
        {
            param->agl_v = 0;
            return 0;
        }
    }
}
