#ifndef _AK_VIDEO_INPUT_H_
#define _AK_VIDEO_INPUT_H_

typedef int VI_DEV;
typedef int VI_CHN;

#define VIDEO_DEV0	    0
#define VIDEO_DEV1	    1
#define VIDEO_DEV2	    2
#define VIDEO_DEV3	    3
#define VIDEO_DEV_MUX	4

#define BOX_COLOR_NUM   3
#define BOX_MAX_NUM     10


enum ak_vi_chn_enum{
    VIDEO_CHN0	=	0,
    VIDEO_CHN1,
    VIDEO_CHN2,
    VIDEO_CHN3,
    VIDEO_CHN4,
    VIDEO_CHN5,
    VIDEO_CHN6,
    VIDEO_CHN7,
    VIDEO_CHN_TOTAL,
    VIDEO_CHN16 = 16,
    VIDEO_CHN17,
    VIDEO_CHN18,
    VIDEO_CHN19,
    VIDEO_CHN24 = 24,		/* defined for stitch chn*/
    VIDEO_CHN25,
    VIDEO_CHN26,
    VIDEO_CHN31 = 31
};

enum video_channel {
    VIDEO_CHN_MAIN,
    VIDEO_CHN_SUB,
    VIDEO_CHN_NUM
};

/* vi module errno list */
enum ak_vi_errno {         //错误代码
    ERROR_VI_INVALID_DEVID					= ( MODULE_ID_VI << 24 ) + 0,
    ERROR_VI_DEVID_NOT_OPEN					= ( MODULE_ID_VI << 24 ) + 1,
    ERROR_VI_DEVID_NOT_MATCH				= ( MODULE_ID_VI << 24 ) + 2,
    ERROR_VI_ATTR_PARM_WRONG				= ( MODULE_ID_VI << 24 ) + 3,
    ERROR_VI_INVALID_CHNID					= ( MODULE_ID_VI << 24 ) + 4,
    ERROR_VI_DEV_NOT_ENABLE					= ( MODULE_ID_VI << 24 ) + 5,
    ERROR_VI_DEV_NOT_DISABLE				= ( MODULE_ID_VI << 24 ) + 6,
    ERROR_VI_DEV_REGISTER_FAIL				= ( MODULE_ID_VI << 24 ) + 7,
    ERROR_VI_CHANNEL_REGISTER_FAIL			= ( MODULE_ID_VI << 24 ) + 8,
    ERROR_VI_CHANNEL_DEVICE_NOT_OPEN		= ( MODULE_ID_VI << 24 ) + 9,
    ERROR_VI_CHANNEL_NOT_EXIST				= ( MODULE_ID_VI << 24 ) + 10,
    ERROR_VI_CHANNEL_NOT_ENABLE				= ( MODULE_ID_VI << 24 ) + 11,
    ERROR_VI_CHANNEL_NOT_DISABLE			= ( MODULE_ID_VI << 24 ) + 12,
    ERROR_VI_CHANNEL_ID_NOT_MATCH			= ( MODULE_ID_VI << 24 ) + 13,
    ERROR_VI_SET_BOX_COLOR_TABLE_ERROR     	= ( MODULE_ID_VI << 24 ) + 14,
    ERROR_VI_DRAW_BOX_ERROR     			= ( MODULE_ID_VI << 24 ) + 15,
    ERROR_VI_ISP_CONF_NOT_LOADED   			= ( MODULE_ID_VI << 24 ) + 16,
    ERROR_VI_SET_CHN_STATUS		   			= ( MODULE_ID_VI << 24 ) + 17,
    ERROR_VI_RESET_CHN_CROP_FAIL            = ( MODULE_ID_VI << 24 ) + 18,
};

typedef enum AK_VI_DATA_PATH {
    VI_PATH_BYPASS = 0,								//ISP is bypass
    VI_PATH_ISP,									//ISP enable
    VI_PATH_RAW,									//capture raw data, for debug
    VI_PATH_TOTAL
}VI_DATA_PATH_E;

typedef enum AK_VI_DATA_TYPE {
    VI_DATA_TYPE_YUV420SP = 0,						//NV12 (YUV420SP)
    VI_DATA_TYPE_YUV420P,							//I420 (YUV420P)
    VI_DATA_TYPE_RGB ,								//RGB Data
    VI_DATA_TYPE_RAW,								//Raw Data
    VI_DATA_TYPE_RGB_LINEINTL,						//RGB line-interlace Data
    VI_DATA_TYPE_TOTAL
}VI_DATA_TYPE_E;

typedef enum AK_VI_INTF_MODE {
    VI_INTF_DVP = 0,								//DVP 并行接口
    VI_INTF_MIPI_1,									//MIPI串行1线接口
    VI_INTF_MIPI_2,									//MIPI串行2线接口
    VI_INTF_DUAL_DVP,                               //双目模式DVP接口
    VI_INTF_DUAL_MIPI_1,                            //双目模式MIPI 1线接口
    VI_INTF_DUAL_MIPI_2,							//双目模式MIPI 2线接口
    VI_INTF_TOTAL
}VI_INTF_MODE_E;

typedef enum
{
    FRAME_MODE = 0x0,								//complete frame mode
    SLICE_MODE,										//slice mode
}VI_CHN_MODE_E;

enum ak_vi_daynight_mode {
    VI_MODE_DAY_OUTDOOR,
    VI_MODE_NIGHTTIME,
    VI_MODE_USR_DEFINE_1,
    VI_MODE_USR_DEFINE_2,
    VI_MODE_USR_DEFINE_3,
    VI_MODE_NUM
};

struct crop_info {
    int		left;	// x position of crop
    int		top;	// y position of crop
    int		width;	// width of crop
    int		height;	// height of crop
};

typedef struct AK_VI_DEV_ATTR{
    VI_DEV 				dev_id;						//vi设备号
    VI_INTF_MODE_E 		interf_mode;				//interface mode, dvp/mipi1/mipi2
    int					fd;							//vi设备对应的设备文件描述符
    VI_DATA_PATH_E		data_path;					//数据路径，1为启用ISP
    VI_DATA_TYPE_E		data_type;					//数据类型，RGB/ YUV
    struct crop_info	crop;						//crop info of device
    int 				max_width;					//max value of width of main chn resolution
    int					max_height;					//max value of height of main chn resolution
    int					frame_rate;					//source frame_rate of the ISP
    int 				sub_max_width;				//max value of width of sub chn resolution
    int					sub_max_height;				//max value of height of sub chn resolution
}VI_DEV_ATTR;

typedef struct AK_VI_RECTANGLE{
    int		width;									//width lenght of resolution
    int		height;									//height lenght of resolution
}RECTANGLE_S;

typedef struct AK_VI_CHN_ATTR {
    VI_CHN			chn_id;							//VI 通道号。
    int				frame_rate;						//当前通道的目标帧率
    RECTANGLE_S		res;							//目标分辨率
    int				frame_depth;					//frame buffer number of the channel
}VI_CHN_ATTR;

typedef struct AK_VI_CHN_ATTR_EX {
    VI_CHN			chn_id;							//VI 通道号。
    int				frame_rate;						//当前通道的目标帧率
    RECTANGLE_S		res;							//目标分辨率
    int				frame_depth;					//frame buffer number of the channel
    VI_DATA_TYPE_E	data_type;
    VI_CHN_MODE_E		mode;
    struct crop_info crop;
    RECTANGLE_S		max_res;						//目标分辨率
}VI_CHN_ATTR_EX;

struct video_input_frame {
    struct frame vi_frame;
    unsigned long phyaddr;
    int	type;
    void *mdinfo;
};

typedef struct ak_vi_box {
    unsigned short left;
    unsigned short top;
    unsigned short width;
    unsigned short height;

    unsigned char  enable;       //[0-1]

    unsigned short color_id;    //[0-2]
    unsigned short line_width;  //[0-15] actural draw width =  (line_width+1)*2
}VI_BOX;

typedef struct ak_vi_box_group {
    struct ak_vi_box box[BOX_MAX_NUM];
    // -1 draw forever ; 0 not draw; >0 draw frame num
    int draw_frame_num;
}VI_BOX_GROUP;

/* stitch mode */
typedef enum{
    DISABLED_STITCH	= 0x0,
    VERTICAL_STITCH,
    HORIZONTAL_STITCH,
}VI_STITCH_MODE_E;

#define MAX_STITCH_BIND_CHN_NUM	4
typedef struct ak_vi_stitch_attr{
    VI_STITCH_MODE_E	mode;										// stitch mode for chn
    // record the chn num binded to the stitch chn
    unsigned int 		stitch_chn_num;
    // the source chn id for stitch chn
    unsigned int 		stitch_bind_chn[MAX_STITCH_BIND_CHN_NUM];
}VI_STITCH_ATTR;

/* ak_vi_get_version
 * return the vi lib version
 */
const char* ak_vi_get_version(void);

/**
 * ak_vi_open: open video input device
 * @dev_id[IN]: video input device ID
 * return: AK_SUCCESS if successful, error code if failed
 * notes:
 */
int  ak_vi_open(VI_DEV dev_id);

/**
 * ak_vi_close: close video input device
 * @dev_id[IN]: video input device ID
 * return: AK_SUCCESS if successful, error code if failed
 * notes:
 */
int  ak_vi_close(VI_DEV dev_id);

/* ak_vi_load_sensor_cfg 	-- load isp config file
 * dev_id[IN]		: 	video input device ID
 * config_file[IN]	:	config file path
 * return AK_SUCCESS if success, Error Code if failed
 */
int ak_vi_load_sensor_cfg(VI_DEV dev_id, const char *config_file);

/* ak_vi_load_sensor_cfg_ex    -- load isp config file
 * dev_id[IN]       :   video input device ID
 * config_file[IN]  :   config file path
 * @mode[IN]:day or night, or usr define subfile
 * return AK_SUCCESS if success, Error Code if failed
 */
int ak_vi_load_sensor_cfg_ex(VI_DEV dev_id, const char *config_file, enum ak_vi_daynight_mode mode);


/* ak_vi_get_sensor_resolution	---	 get sensor resolution by device id
 * dev_id[IN]		: 	video input device ID
 * res[OUT]			:	pointer to record the resolution of vi device
 * return AK_SUCCESS if success, AK_FAILED if failed
 */
int ak_vi_get_sensor_resolution(VI_DEV dev_id, RECTANGLE_S *res);

/* ak_vi_set_dev_attr -- set dev basic attr
 * dev_id[IN] : 	video input device ID
 * dev_attr[IN]   :		video ipnout device attribution
 * return AK_SUCCESS if successful, error code if failed
 */
int ak_vi_set_dev_attr(VI_DEV dev_id,  VI_DEV_ATTR *dev_attr);

/* ak_vi_get_dev_attr -- set dev basic attr
 * dev_id[IN] : 	video input device ID
 * dev_attr[OUT]   :		video ipnout device attribution
 * return AK_SUCCESS if successful, error code if failed
 */
int ak_vi_get_dev_attr(VI_DEV dev_id, VI_DEV_ATTR *dev_attr);

/* ak_vi_set_chn_attr	--	set channel attribute
 * chn_id[IN]			: 		channel id
 * chn_attr[IN]			:		channel attribute
 */
int ak_vi_set_chn_attr(VI_CHN chn_id, VI_CHN_ATTR *chn_attr);

/* ak_vi_get_chn_attr	--	get channel attribute
 * chn_id[IN]			: 		channel id
 * chn_attr[OUT]			:		channel attribute
 * return AK_SUCCESS if successful, error code if failed
 */
int ak_vi_get_chn_attr(VI_CHN chn_id, VI_CHN_ATTR *chn_attr);

/* ak_vi_set_chn_attr_ex	--	set channel attribute with extend param
 * chn_id[IN]			: 		channel id
 * chn_attr[IN]			:		channel extend attribute
 */
int ak_vi_set_chn_attr_ex(VI_CHN chn_id, VI_CHN_ATTR_EX *chn_attr);

/* ak_vi_get_chn_attr_ex	--	get the channel extend attribute
 * chn_id[IN]			: 		channel id
 * chn_attr[OUT]		:		channel extend attribute
 * return AK_SUCCESS if successful, error code if failed
 */
int ak_vi_get_chn_attr_ex(VI_CHN chn_id, VI_CHN_ATTR_EX *chn_attr);

/* ak_vi_enable_dev  --		enable vi device to start capture
 * dev_id[IN]			: 		vi device id
 * return AK_SUCCESS if success, error code if failed
 * */
int ak_vi_enable_dev(VI_DEV dev_id);

/* ak_vi_disable_dev  --		disable vi device to start capture
 * dev_id[IN]			: 		vi device id
 * return AK_SUCCESS if success, error code if failed
 * */
int ak_vi_disable_dev(VI_DEV dev_id);

/* ak_vi_enable_chn  --		enable vi channel to start capture
 * chn_id[IN]			: 		vi channel id
 * return AK_SUCCESS if success, error code if failed
 * */
int ak_vi_enable_chn(VI_CHN chn_id);

/* ak_vi_disable_chn	  --		enable vi channel to start capture
 * chn_id[IN]			: 		vi channel id
 * return AK_SUCCESS if success, error code if failed
 * */
int ak_vi_disable_chn(VI_CHN chn_id);

/**
 * ak_vi_get_frame: get frame
 * chn_id[IN]	:	channel id
 * @frame[OUT]: store frames
 * return: 0 success, otherwise failed
 */
int ak_vi_get_frame(VI_CHN chn_id, struct video_input_frame *frame);

/**
 * ak_vi_release_frame:  release  frame
 * chn_id	[IN]:	channel id
 * @frame[IN]:  pointer to vi frames
 * return: AK_SUCCESS if  success, error code if failed
 */
int ak_vi_release_frame(VI_CHN chn_id, struct video_input_frame *frame);

/* ak_vi_change_chn_fps	--	change channel fps
 * chn_id[IN]			: 		channel id
 * frame_rate[IN]		:		frame_rate
 */
int ak_vi_change_chn_fps(VI_CHN chn_id, int frame_rate);

/* ak_vi_change_dev_fps	--	change the device fps
 * dev_id[IN]			: 		dev id
 * frame_rate[IN]		:		frame_rate
 */
int ak_vi_change_dev_fps(VI_DEV dev_id, int frame_rate);

/**
 * brief: switch day night mode
 * @dev[IN]: device id
 * @mode[IN]:day or night
 * return: 0 success, otherwise error code;
 * notes:
 */
int ak_vi_switch_mode(VI_DEV dev_id, enum ak_vi_daynight_mode mode);

/*
 *  brief:  get sensor id
 * @dev[IN]: device id
 * ak_vi_get_sensor_id
 * id[OUT]: sensor id
 * return: 0 success, -1 failed
 */
int ak_vi_get_sensor_id(int dev, int *id);

/**
 * brief: set box color table
 * @dev_id[IN]: device id
 * @p_color_table[IN]:color table , format : ARGB (32bit). size : 3.
 * return: 0 success, otherwise error code;
 * notes:
 */
int ak_vi_set_box_color_table(VI_DEV dev_id, unsigned int *p_color_table);

/**
 * brief: draw box
 * @chn_id[IN]: chn_id
 * @box_group[IN]:box group attr
 * return: 0 success, otherwise error code;
 * notes:
 */
int ak_vi_draw_box(VI_CHN chn_id, struct ak_vi_box_group *box_group);

/* ak_vi_set_cfg_file_path 	-- load isp config file
 * dev_id[IN]		: 	video input device ID
 * config_file[IN]	:	config file path
 * return AK_SUCCESS if success, Error Code if failed
 */
int ak_vi_set_cfg_file_path(VI_DEV dev_id, const char *config_file);

/* ak_vi_reset_chn_crop	--	ret the crop attribute of the chn
 * chn_id[IN]		: 		channel id
 * crop [IT]		:		channel crop attribute
 * step [IN]		:		the step num that the crop setting take effect
 * return AK_SUCCESS if successful, error code if failed
 */
int ak_vi_reset_chn_crop(VI_CHN chn_id, struct crop_info *crop, int step);

/* ak_vi_create_stitch_chn	-- create the stitch chn
 * chn_id[IN]			:		the channel id of stitch chn
 * chn_attr[IN]			:		the channel attr of the stitch chn
 * stitch_attr[IN]	:		the source channel info for stitch chn
 * return AK_SUCCESS if successful, error code if failed
 * */
int ak_vi_create_stitch_chn(VI_CHN chn_id, VI_CHN_ATTR_EX *chn_attr,
                                        VI_STITCH_ATTR *stitch_attr);

/* ak_vi_destroy_stitch_chn	-- destroy the stitch chn
 * chn_id[IN]			:		the channel id of stitch chn
 * return AK_SUCCESS if successful, error code if failed
 * */
int ak_vi_destroy_stitch_chn(VI_CHN chn_id);

/* ak_vi_set_dev_3DNR_memsize  --  set the memory size for 3DNR function of device 
 * dev_id[IN]		: 	video input device ID
 * mem_size[IN]     :    memory size of the 3DNR function
 * return AK_SUCCESS if successful, error code if failed
 */
int ak_vi_set_dev_3DNR_memsize(VI_DEV dev_id, const int mem_size);

/* ak_vi_get_dev_3DNR_memsize  --  get the memory size for 3DNR function of device 
 * dev_id[IN]		: 	video input device ID
 * mem_size[OUT]    :   memory buf size of the 3DNR function
 * return AK_SUCCESS if successful, error code if failed
 */
int ak_vi_get_dev_3DNR_memsize(VI_DEV dev_id, int *mem_size);

/* ak_vi_set_dev_3DNR_YUVsize  --  set the memory size for 3DNR function of device 
 * dev_id[IN]		: 	video input device ID
 * y_size[IN]     :    y size of the 3DNR function(KB)
 * u_size[IN]     :    u size of the 3DNR function(KB)
 * v_size[IN]     :    v size of the 3DNR function(KB)
 * return AK_SUCCESS if successful, error code if failed
 */
int ak_vi_set_dev_3DNR_YUVsize(VI_DEV dev_id, const int y_size, const int u_size, const int v_size);

/* ak_vi_get_dev_3DNR_YUVsize  --  get the memory size for 3DNR function of device 
 * dev_id[IN]		: 	video input device ID
 * y_size[OUT]    :   y buf size of the 3DNR function(KB)
 * u_size[OUT]    :   u buf size of the 3DNR function(KB)
 * v_size[OUT]    :   v buf size of the 3DNR function(KB)
 * return AK_SUCCESS if successful, error code if failed
 */
int ak_vi_get_dev_3DNR_YUVsize(VI_DEV dev_id, int *y_size, int *u_size, int *v_size);


/**
 * ak_vi_get_y_data: get y data
 * @chn_id[IN]: chn id
 * @rect[IN]:select rect
 * @buf[OUT]:buf, should malloc before call this function
 * @buf_len[IN]:buf_len
 * return: 0 success, otherwise error code;
 * notes:
 */
int ak_vi_get_y_data(VI_CHN chn_id, struct crop_info *rect, unsigned char *buf, unsigned int buf_len);

#endif

/* end of file */
