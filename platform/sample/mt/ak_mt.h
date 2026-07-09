#ifndef _AK_MT_H_
#define _AK_MT_H_

#define MD_SIZE  32  // horizontal and vertical size of vpss_md matrix

typedef struct
{
    short px;
    short py;
}MT_POINT;

typedef struct
{
    MT_POINT pt;    // center coordinate of motion region
    signed char width;
    signed char height;
}MT_RECTANGLE;

/**
 * ak_mt_init - init motion tracking parameters
 * return: motion tracking parameter handle
 * notes:
 */
void *ak_mt_init(void);

/**
 * ak_mt_destroy - release motion tracking parameter handle
 * @mt_handle[IN]: motion tracking parameter handle
 * return: 0 on success; -1 on failed
 * notes:
 */
int ak_mt_destroy(void *mt_handle);

/**
 * ak_mt_reset_tracking_param - reset parameters of mt library
 * @mt_handle[IN]: motion tracking parameter handle
 * return: 0 on success; -1 on failed
 * notes:
 */
int ak_mt_reset_tracking_param(void *mt_handle);

/**
 * ak_mt_get_version - get the version of mt library
 * return: the version of mt library
 * notes:
 */
char *ak_mt_get_version();

/**
 * ak_mt_set_fps - set encode frames per second
 * @mt_handle[IN]: motion tracking parameter handle
 * @fps[IN]: encode frames per second
 * return: 0 on success; -1 on failed
 * notes:
 */
int ak_mt_set_fps(void *mt_handle, short fps);

/**
 * ak_mt_set_max_invalid_frames - set max frames to determine invalid motion
 * @mt_handle[IN]: motion tracking parameter handle
 * @nFrames[IN]: max frames to determine invalid motion
 * return: 0 on success; -1 on failed
 * notes:
 */
int ak_mt_set_max_invalid_frames(void *mt_handle, unsigned short nFrames);

/**
 * ak_mt_set_max_decision_frames - set max frames to determine camera rotation
 * @mt_handle[IN]: motion tracking parameter handle
 * @nFrames[IN]: max frames to determine camera rotation
 * return: 0 on success; -1 on failed
 * notes:
 */
int ak_mt_set_max_decision_frames(void *mt_handle, unsigned short nFrames);

/**
 * ak_mt_set_invalid_distance - set distance to seperate invalid and valid motion
 * @mt_handle[IN]: motion tracking parameter handle
 * @dis[IN]: distance to seperate invalid and valid motion
 * return: 0 on success; -1 on failed
 * notes:
 */
int ak_mt_set_invalid_distance(void *mt_handle, unsigned char dis);

/**
 * ak_mt_set_camera_rotation_factor - set camera rotation factor
 * @mt_handle[IN]: motion tracking parameter handle
 * @factor[IN]: camera rotation factor
 * return: 0 on success; -1 on failed
 * notes: if offset caused by motion speed isn't essential, set the input parameter factor = x(x is a negative number, example -1)
 */
int ak_mt_set_camera_rotation_factor(void *mt_handle, int factor);

/**
 * ak_mt_open_camera_rotation_switch_v - open camera rotation switch in vertical direction
 * @mt_handle[IN]: motion tracking parameter handle
 * return: 0 on success; -1 on failed
 * notes:
 */
int ak_mt_open_camera_rotation_switch_v(void *mt_handle);

/**
 * ak_mt_close_camera_rotation_switch_v - close camera rotation switch in vertical direction
 * @mt_handle[IN]: motion tracking parameter handle
 * return: 0 on success; -1 on failed
 * notes:
 */
int ak_mt_close_camera_rotation_switch_v(void *mt_handle);

/**
 * ak_mt_reverse_main_decision_direction_v - reverse main decision direction in vertical direction
 * @mt_handle[IN]: motion tracking parameter handle
 * return: 0 on success; -1 on failed
 * notes:
 */
int ak_mt_reverse_main_decision_direction_v(void *mt_handle);

/**
 * ak_mt_set_params - set motion tracking parameters from outer input
 * @mt_handle[IN]: motion tracking parameter handle
 * @len[IN]: VPSS_MD_DIMENSION in vertical direction
 * @valid_size_min[IN]: min size of valid motion region
 * @valid_size_max[IN]: max size of valid motion region
 * @center_size_h[IN]: horizontal size of video frame center
 * @center_size_v[IN]: vertical size of video frame center
 * return: 0 on success; -1 on failed
 * notes:
 */
int ak_mt_set_motion_region_params(void *mt_handle, short len, short valid_size_min,  short valid_size_max,
    short boundary_size_h, short boundary_size_v);

/**
* ak_mt_set_field_angles - set the field angles of target sensor
* @mt_handle[IN]: motion tracking parameter handle
* @fov_h[IN]: field angle of sensor in horizontal direction
* @fov_v[IN]: field angle of sensor in vertical direction
* @block_num_h[IN]: VPSS_MD_DIMENSION in horizontal direction
* @block_num_v[IN]: VPSS_MD_DIMENSION in vertical direction
* return: 0 on success; -1 on failed
* notes:
*/
int ak_mt_set_field_angles(void *mt_handle, unsigned char fov_h, unsigned char fov_v, unsigned short block_num_h, unsigned short block_num_v);

/**
* ak_mt_set_rotation_range - set the rotation range of camera
* @mt_handle[IN]: motion tracking parameter handle
* @min_agl_h[IN]: min rotation angle in horizontal direction
* @max_agl_h[IN]: max rotation angle in horizontal direction
* @min_agl_v[IN]: min rotation angle in vertical direction
* @max_agl_v[IN]: max rotation angle in vertical direction
* return: 0 on success; -1 on failed
* notes:
*/
int ak_mt_set_rotation_range(void *mt_handle, unsigned char min_agl_h, unsigned char max_agl_h, unsigned char min_agl_v, unsigned char max_agl_v);

/**
 * ak_mt_tracking - track the desired motion region
 * @mt_handle[IN]: motion tracking parameter handle
 * @len[IN]: VPSS_MD_DIMENSION in vertical direction
 * @rect[IN]: rectangle box of the largest motion region
 * @rt_angle[OUT]: relative rotation angel in horizontal and vertical direction
 * return: 0 need no rotation; 1 need rotation; 2 reset to inital position, -1 param error, no effect
 * notes:
 */
int ak_mt_tracking(void *mt_handle, short len, MT_RECTANGLE *rect, MT_POINT *rt_angle);

#endif