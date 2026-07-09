#include "ak_mt.h"

#define MAX_INVALID_FRAMES      200       // max frames to determine whether motion is invalid or not
#define MAX_DECISION_FRAMES     7         // max frames to determine the position of largest size motion region
#define INVALID_FRAMES_CAP      400       // max frames to determine whether motion is invalid or not
#define DECISION_FRAMES_CAP     20        // max frames to determine the position of largest size motion region

#define DECISION_FRAMES_CAP_V   5         // max valid frames in vertical direction

#define INVALID_DISTANCE        8         // distance threshold to separate valid motion and invalid motion
#define PI 3.14159265

typedef struct TRACKING_PARAM_STRU
{
    void *mt_handle;
    MT_RECTANGLE valid_rect_v[DECISION_FRAMES_CAP_V];
    unsigned char valid_rect_num_v;
    MT_RECTANGLE invalid_rect[INVALID_FRAMES_CAP];	 // invalid motion region
    MT_RECTANGLE valid_rect[DECISION_FRAMES_CAP];   // valid motion region
    short invalid_rect_num;                      // invalid motion region count
    unsigned char valid_rect_num;                                 // valid motion region count
    unsigned char valid_rect_left_index[DECISION_FRAMES_CAP];     // valid motion region on the left
    unsigned char valid_rect_left_num;                            // the count of valid motion region on the left
    unsigned char valid_rect_right_index[DECISION_FRAMES_CAP];    // valid motion region on the right
    unsigned char valid_rect_right_num;                           // the count of valid motion region on the right
    unsigned char valid_rect_botoom_index[DECISION_FRAMES_CAP];   // valid motion region on the botoom
    unsigned char valid_rect_botoom_num;                          // the count of valid motion region on the botoom
    unsigned char valid_rect_top_index[DECISION_FRAMES_CAP];      // valid motion region on the top
    unsigned char valid_rect_top_num;                             // the count of valid motion region on the top
    unsigned char valid_rect_center_index[DECISION_FRAMES_CAP];   // valid motion region in the center
    unsigned char valid_rect_center_num;                          // the count of valid motion region in the center
    unsigned char current_max_invalid_rect_dis;   // current max distance between invalid motion region

    short fps;      // encode frames per second
    short size_min; // min and max size of valid motion region
    short size_max;
    short boundary_size_h;  // horizontal and vertical distance between center region and video frame boundary
    short boundary_size_v;
    signed char decision_enalbe;   // motion region changing switch
    signed char rotate_h_enable;   // camera horizontal rotation switch
    signed char rotate_v_enable;   // camera vertical rotation switch

    unsigned short max_invalid_fm;  // max frames to determine whether motion is invalid or not
    unsigned char max_decision_fm;  // max frames to determine the position of largest size motion region
    unsigned char max_decision_fm_backup; // backup of max frames to determine the position of largest size motion region

    short agl_h;  // rotate angle in horizontal direction
    short agl_v;  // rotate angle in vertical direction

    int mt_factor; // camera rotation factor
    short undetected_num; // undetected_num while making decision
    signed char main_decision_dir_v; // main decision direction in vertical direction

    unsigned char invalid_dis; // distance threshold to separate valid motion and invalid motion

    unsigned char fov_h;
    unsigned char fov_v;

    unsigned char block_angles_h[MD_SIZE*2+1];
    unsigned char block_angles_v[MD_SIZE*2+1];

    unsigned char min_agl_h;
    unsigned char max_agl_h;
    unsigned char min_agl_v;
    unsigned char max_agl_v;
}TRACKING_PARAM;

/**
 * mt_member_copy - copy member of struct rectangle
 * @rect_org[IN]: motion region
 * @rect_dst[OUT]: motion region
 * return: none
 * notes:
 */
void mt_member_copy(MT_RECTANGLE *rect_dst, MT_RECTANGLE *rect_org);

/**
 * mt_mrd_Isoutofcenter - whether motion region is out of the center region of video frame
 * @param[IN]: motion tracking parameter handle
 * @rect[IN]: motion region
 * @distance[OUT]: distance between motion region center and video frame center in horizontal and vertical direction
 * @len[IN]: VPSS_MD_DIMENSION in vertical direction
 * @size_h[IN]: video frame center size in horizontal direction
 * @size_v[IN]: video frame center size in vertical direction
 * return: 1 out of center; 0 not out of center
 * notes:
 */
int mt_mrd_Isoutofcenter(TRACKING_PARAM *param, MT_RECTANGLE *rect, MT_POINT *distance, short len, short size_h, short size_v);

/**
 * mt_tracking_max_distance - calculate the max distance between recorded invalid motion region
 * @param[IN]: motion tracking parameter handle
 * @rect[IN]: motion region
 * return: none
 * notes:
 */
void mt_tracking_max_distance(TRACKING_PARAM *param, MT_RECTANGLE *rect);

/**
 * mt_rect_optimization - optimize the detected rectangles and return some useful information for speed calculation
 * @param[IN]: motion tracking parameter handle
 * return: -1 speed calculation is not required, otherwise, the index of the start rect to calculate speed
 * notes:
 */
int mt_rect_optimization(TRACKING_PARAM *param);

/**
 * mt_get_rotation_angle - calculate the rotation angle based on the distance and speed of moving object
 * @param[IN]: motion tracking parameter handle
 * @len[IN]: VPSS_MD_DIMENSION in vertical direction
 * return: 1 ready to rotate, 0 wait for more frames
 * notes:
 */
int mt_get_rotation_angle(TRACKING_PARAM *param, short len);

/**
 * mt_get_rotation_angle_v - calculate the rotation angle based on the distance and speed of moving object
 * @param[IN]: motion tracking parameter handle
 * @len[IN]: VPSS_MD_DIMENSION in vertical direction
 * @rect[IN]: motion region
 * return: 1 ready to rotate, 0 no rotation is required
 * notes:
 */
int mt_get_rotation_angle_v(TRACKING_PARAM *param, short len, MT_RECTANGLE *rect);
