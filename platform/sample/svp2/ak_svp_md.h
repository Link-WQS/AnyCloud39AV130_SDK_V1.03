#ifndef __AK_SVP_MD_H__
#define __AK_SVP_MD_H__

#define MAX_BOX_NUM 	5

struct svp_md_param{
    int dev_id;
    int md_fps;
    int move_size_min;
    int move_size_max;
    int flt_big;
    int flt_small;

};

struct md_box
{
    int left;
    int top;
    int right;
    int bottom;
};

struct md_result_info
{
    int result;
    int md_sec;
    int move_box_num;
    struct md_box boxes[MAX_BOX_NUM];
};

/**
 * ak_svp_md_init - md init .
 * @param[IN]:  svp_md_param
 * return: 0 - success; other error code;
 */
int ak_svp_md_init(struct svp_md_param *param);

/**
 * ak_svp_md_get_result - get md result.
 * @dev[IN]:  dev id
 * @ret_info[OUT]:  result info
 * @timeout[IN]:  timeout <0  block mode, ==0 non-block mode, >0 waittime
 * return: 0 - success;  other error code;
 */
int ak_svp_md_get_result(int dev, struct md_result_info *ret_info, int timeout);

/**
 * ak_svp_md_enable - start or stop md .
 * @dev[IN]:  dev id
 * @enable[IN]:  [0,1],  0 -> stop md; 1 -> start md
 * return: 0 - success; other error code;
 */
int ak_svp_md_enable(int dev, int enable);

/*
 * ak_svp_md_uninit - free md resource and quit md .
 * @dev[IN]:  dev id
 * return: 0 - success; other error code;
 */
int ak_svp_md_uninit(int dev);

#endif

/* end of file */
