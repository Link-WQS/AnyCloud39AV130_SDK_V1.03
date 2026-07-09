#define OBJECT_CAP  10

typedef struct
{
	short left;
	short top;
	short right;
	short bottom;
	short count;
	unsigned char md;
}OBJECT_BOX;

/**
* ak_object_filter_init - init object filter parameters
* return: NULL on failed; otherwise, object filter parameter handle
* notes:
*/
void* ak_object_filter_init(void);

/**
* ak_object_filter_destroy - release object filter parameter handle
* @filter_handle[IN]: object filter parameter handle
* return: 0 on success; -1 on failed
* notes:
*/
int ak_object_filter_destroy(void *filter_handle);

/**
* ak_object_filter_set_frames - set parameters for object filter
* @filter_handle[IN]: object filter parameter handle
* @main_frames[IN]: the number of continous frames
* @sub_frames[IN]: the number of frames besides continous main_frames
* @extra_frames[IN]: the accepted frames for obtaining sub_frames fitted data
* return: 0 success, -1 falied
* notes:
*/
int ak_object_filter_set_frames(void *filter_handle, int main_frames, int sub_frames, int extra_frames);

/**
* ak_object_filter_get_version - get the version of object filter library
* return: the version of lib obj_filter
* notes:
*/
char *ak_object_filter_get_version();

/**
* ak_object_filter_set_distance_enhancement_params - set the parameters of distance enhancement strategy
* @filter_handle[IN]: object filter parameter handle
* @level[IN]: the level of distance enhancement strategy
* @frames_h[IN]: the max working frames of the history record for distance enhancement strategy in horizontal direction
* @frames_v[IN]: the max working frames of the history record for distance enhancement strategy in vertical direction
* return: 0 success, -1 falied
* notes: level 0, disable distance enhancement strategy
		 level 1, enable distance enhancement strategy in horizontal direction
		 level >= 2, enabel distance enhancement strategy in horizontal and vertical direction
*/
int ak_object_filter_set_distance_enhancement_params(void *filter_handle, unsigned char level, unsigned char frames_h, unsigned char frames_v);

/**
* ak_object_filter_set_md_level - set the level of the usage of md information
* @filter_handle[IN]: object filter parameter handle
* @level[IN]: the level of the usage of md information
* return: 0 success, -1 falied
* notes: level 0, disable the usage of md information
		 level 1, use md information in the alarm frame
		 level 2, use md information in the last two frames(the alarm frame and the frame before)
		 level >= 3, input boxes whose attribute of md equals zero are not in consideration for alarm
*/
int ak_object_filter_set_md_level(void *filter_handle, unsigned char level);

/**
* ak_object_filter_set_continous_enhancement_params - set the parameters of continous enhancement strategy
* @filter_handle[IN]: object filter parameter handle
* @level[IN]: the level of continous enhancement strategy
* @frames[IN]: the max working frames of alarmed record for continous enhancement strategy
* return: 0 success, -1 falied
* notes: level 0, disable continous enhancement strategy
		 level >= 1, enable continous enhancement strategy
*/
int ak_object_filter_set_continous_enhancement_params(void *filter_handle, unsigned char level, unsigned char frames);

/**
* ak_object_filter_set_false_record_params - set the parameters of false record
* @filter_handle[IN]: object filter parameter handle
* @level[IN]: the level of false record
* @frames[IN]: the max working frames of false record
* return: 0 success, -1 falied
* notes: level 0, disable false record
		 level >= 1, enable false record
*/
int ak_object_filter_set_false_record_params(void *filter_handle, unsigned char level, unsigned char frames);

/**
* ak_object_filter_reset_inner_params - reset the inner parameters
* @filter_handle[IN]: object filter parameter handle
* return: 0 success, -1 falied
* notes:
*/
int ak_object_filter_reset_inner_params(void *filter_handle);

/**
* ak_object_filter_alarm - alarm the object detection
* @filter_handle[IN]: object filter parameter handle
* @rect[IN]: object information
			 (the boxes of svp output, but transform is required. 
			 [left, top, right, bottom] is the same as svp output, 
			 the value of md attribute is based on the relative location of motion region and svp boxes, 
			 generally, the interation area must be more than 30% of the corresponding svp box area)
* @box_num[IN]: the num of objects(the num of boxes of svp output)
* return: 0 no alarm; -1 parameter error;
*		  otherwise, alarm is required
* notes:
*/
int ak_object_filter_alarm(void *filter_handle, OBJECT_BOX rect[OBJECT_CAP], unsigned int box_num);