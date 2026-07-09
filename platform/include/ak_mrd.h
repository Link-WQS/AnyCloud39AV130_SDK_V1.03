#ifndef _AK_MRD_H_
#define _AK_MRD_H_

#define MD_SIZE  32  // horizontal and vertical size of vpss_md matrix
#define MAX_CAP  20  // max capacity of detected motion regions

typedef struct
{
	int left;
	int top;
	int right;
	int bottom;
	int count;
}MRD_RECTANGLE;

/**
 * ak_mrd_init - init motion region detetion parameters
 * return: motion region detetion parameter handle
 * notes:
 */
void *ak_mrd_init(void);

/**
 * ak_mrd_destroy - release motion region detection parameter handle
 * @mrd_handle[IN]: motion region detection parameter handle
 * return: 0 on success; -1 on failed
 * notes:
 */
int ak_mrd_destroy(void *mrd_handle);

/**
 * ak_mrd_set_param_rectification_level - set the level for parameters rectification
 * @mrd_handle[IN]: motion region detection parameter handle
 * @level[IN]: the level of parameters rectification
 * return: 0 on success; -1 on failed
 * notes: level = 0, without parameters rectification
		  otherwise, rectify parameters automatically if necessary
 */
int ak_mrd_set_param_rectification_level(void *mrd_handle, unsigned char level);

/**
 * ak_mrd_set_filters - set filters for motion region detection
 * @mrd_handle[IN]: motion region detection parameter handle
 * @flt_big[IN]: big filter to roughly detect motion region
 * @flt_small[IN]: small filter to expand rough detection result
 * return: 0 on success; -1 on failed
 * notes:
 */
int ak_mrd_set_filters(void *mrd_handle, unsigned short flt_big, unsigned short flt_small);

/**
 * ak_mrd_set_motion_region_size - set the range of  detected motion region size to obtain appropriate motion region
 * @mrd_handle[IN]: motion region detection parameter handle
 * @size_min[IN]: minimum size of detected motion region
 * @size_max[IN]: maximun size of detected motion region
 * return: 0 on success; -1 on failed
 * notes:
 */
int ak_mrd_set_motion_region_size(void *mrd_handle, int size_min, int size_max);

/**
 * ak_mrd_set_detection_region - set interested region for motion region detection
 * @mrd_handle[IN]: motion region detection parameter handle
 * @len[IN]: VPSS_MD_DIMENSION in vertical direction
 * @left[IN]: left coordinate of detection region
 * @top[IN]: top coordinate of detection region
 * @right[IN]: right coordinate of detection region
 * @bottom[IN]: bottom coordinate of detection region
 * return: 0 on success; -1 on failed
 * notes:
 */
int ak_mrd_set_detection_region(void *mrd_handle, int len, int left, int top,
	int right, int bottom);

/**
 * ak_mrd_set_mode - set mode for motion region detection
 * @mrd_handle[IN]: motion region detection parameter handle
 * @mode[IN]: 0, multiple frames mode with floating wadding method
 			 otherwise, single frame mode without floating wadding method
 * return: 0 success; -1  failed
 * notes:
 */
int ak_mrd_set_mode(void *mrd_handle, unsigned char mode);


/**
 * ak_mrd_set_floating_wadding_params - set parameters for floating wadding filteration
 * @mrd_handle[IN]: motion region detection parameter handle
 * @m_frames[IN]: the number of continous frames
 * @s_frames[IN]: the number of frames besides continous m_frames
 * @extra_frames[IN]: the accepted frames for obtaining  s_frames fitted data 
 * @dis[IN]: the threshold of distance
 * @m_width[IN]:  the minimum width of motion region
 * @m_height[IN]: the minimum height of motion region
 * return: 0 success, -1 falied
 * notes:
 */	
int ak_mrd_set_floating_wadding_params(void *mrd_handle, int m_frames, int s_frames, int extra_frames, 
	int dis, int m_width, int m_height);

/**
 * ak_mrd - detect motion region
 * @mrd_handle[IN]: motion region detection parameter handle
 * @md[IN]: motion information
 * @len[IN]: VPSS_MD_DIMENSION in vertical direction
 * @boxes[OUT]: motion region
 * return: 0 no motion region; -1 parameter error; 
 *		 otherwise, effect motion region is detected and the amount is equal to the return value; 
 * notes:
 */
int ak_mrd(void *mrd_handle, unsigned short md[][MD_SIZE], int len, MRD_RECTANGLE boxes[MAX_CAP]);

/**
* ak_mrd_get_version - get the version of lib mrd
* return: the version of lib mrd;
* notes:
*/
char *ak_mrd_get_version();
#endif

