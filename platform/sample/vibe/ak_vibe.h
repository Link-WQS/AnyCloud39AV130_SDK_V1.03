#ifndef _AK_VIBE_H_
#define _AK_VIBE_H_

#define VIBE_HEIGHT     60             // the desired height of frame
#define VIBE_WIDTH      80             // the desired width of input frame
#define VIBE_BOXES_CAP  10             // the maximum number of boxes to be saved


typedef struct
{
    int left;
    int top;
    int right;
    int bottom;
    int count; // the number of occurrences of the box
}VIBE_BOX;

/**
 * ak_vibe_init - init vibe parameters
 * return: vibe parameter handle
 * notes:
 */
void *ak_vibe_init(void);

/**
 * ak_vibe_destroy - release vibe parameter handle
 * @vibe_handle[IN]: vibe parameter handle
 * return: 0 on success; -1 on failed
 * notes:
 */
int ak_vibe_destroy(void *vibe_handle);

/**
 * ak_vibe_get_version - get the version of vibe library
 * return: the version of vibe library
 * notes:
 */
char *ak_vibe_get_version();

/**
 * ak_vibe_set_filters - set the filters that decide a pixel is foreground pixel or background pixel
 * @vibe_handle[IN]: vibe parameter handle
 * @flt_big[IN]: the bigger threshold
 * @flt_small[IN]: the smaller threshold
 * return: 0 on success; -1 on failed
 * notes:
 */
int ak_vibe_set_filters(void *vibe_handle, unsigned char flt_big, unsigned char flt_small);

/**
 * ak_vibe_set_frame_difference_filter - set the filter of frame difference algrithom
 * @vibe_handle[IN]: vibe parameter handle
 * @filter[IN]: the filter to separate foreground from background
 * return: 0 on success; -1 on failed
 * notes:
 */
int ak_vibe_set_frame_difference_filter(void *vibe_handle, unsigned char filter);

/**
 * ak_vibe_set_update_frames - set the minimun frames to update background modes of certain region
 * @vibe_handle[IN]: vibe parameter handle
 * @nFrames[IN]: the number of frames
 * return: 0 on success; -1 on failed
 * notes:
 */
int ak_vibe_set_update_frames(void *vibe_handle, unsigned char nFrames);

/**
 * ak_vibe_set_motion_region_size - set the range of  detected motion region size to obtain appropriate motion region
 * @vibe_handle[IN]: vibe parameter handle
 * @size_min[IN]: minimum size of detected motion region
 * @size_max[IN]: maximun size of detected motion region
 * return: 0 on success; -1 on failed
 * notes:
 */
int ak_vibe_set_motion_region_size(void *vibe_handle,
    unsigned int size_min, unsigned int size_max);

/**
 * ak_vibe_set_mode - set the mode for vibe algorithm
 * @vibe_handle[IN]: vibe parameter handle
 * @mode[IN]: the mode of vibe algorithm
 * return: 0 on success; -1 on failed
 * notes:
 */
int ak_vibe_set_mode(void *vibe_handle, unsigned char mode);

/**
* ak_vibe_set_filtering_flag - set vibe filtering flags that indicate certain filtering methods are utilized or not
* @vibe_handle[IN]: vibe parameter handle
* @flag[IN]: the filtering flag of vibe algorithm
* return: 0 on success; -1 on failed
* notes:1.the 4th bit indicates whether gradient difference between current frame and the previous frame is calculated or not
        2.the 3rd bit indicates whether a single box are separated into several small boxes or not
        3.the 2nd bit indicates whether a boundray filtering method is utilized or not
        4.the 1st bit indicates whether isolated point filtering method is used or not
        for example, 00001011=11 indicates the uper methods 1,3,4 are ready to work
*/
int ak_vibe_set_filtering_flag(void *vibe_handle, unsigned char flag);

/**
* ak_vibe_set_fast_updating_flag - set fast updating method flag for background model
* @vibe_handle[IN]: vibe parameter handle
* @flag[IN]: the state of fast updating method
* return: 0 on success; -1 on failed
* notes:
*/
int ak_vibe_set_fast_updating_flag(void *vibe_handle, unsigned char flag);

/**
 * ak_vibe_set_floating_wadding_params - set parameters for floating wadding filteration
 * @vibe_handle[IN]: vibe parameter handle
 * @m_frames[IN]: the number of continous frames
 * @s_frames[IN]: the number of frames besides continous m_frames, s_frames must be smaller than 8
 * @extra_frames[IN]: the accepted frames for obtaining  s_frames fitted data
 * @dis[IN]: the threshold of distance
 * @m_width[IN]:  the minimum width of motion region
 * @m_height[IN]: the minimum height of motion region
 * @speed_mode[IN]: fixed speed mode or size-adaptive speed mode
                    In size-adaptive speed mode, the input speed_mode not only indicates the speed mode that we used,
                    but also decides the maximum size that we use the input or default parameter dis. Thus, if the size
                    of the box is bigger than 8*speed_mode, we automatically change the input or default parameter dis.
 * return: 0 success, -1 falied
 * notes:
 */
int ak_vibe_set_floating_wadding_params(void *vibe_handle, int m_frames, int s_frames, int extra_frames,
    int dis, int m_width, int m_height, unsigned char speed_mode);

/**
 * ak_vibe_reconstruct_background_model - reconstruct background models
 * @vibe_handle[IN]: vibe parameter handle
 * return: 0 on success; -1 on failed
 * notes:
 */
int ak_vibe_reconstruct_background_model(void *vibe_handle);

/**
 * ak_vibe_set_false_detection_factor -  set the factor to determine the false detection
 * @vibe_handle[IN]: vibe parameter handle
 * @factor[IN]: input factor
 * return: -1 on failed, 0 on success
 * notes:
 */
int ak_vibe_set_false_detection_factor(void *vibe_handle, unsigned char factor);

/**
 * ak_vibe - detect motion region based on vibe algorithm
 * @vibe_handle[IN]: vibe parameter handle
 * @frame[IN]: video frame
 * @height[IN]: the height of input video frame
 * @width[IN]: the width of input video frame
 * @boxes[OUT]: motion regions
 * return: -1, parameter error;
          otherwise, the number of detected motion region
 * notes: height shouldn't be smaller than VIBE_HEIGHT*2, and width shouldn't be smaller than VIBE_WIDTH*2
 */
int ak_vibe(void *vibe_handle, unsigned char *frame, int height, int width,
    VIBE_BOX boxes[VIBE_BOXES_CAP]);
#endif

