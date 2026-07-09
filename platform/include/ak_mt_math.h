#ifndef _AK_MT_MATH_H_
#define _AK_MT_MATH_H_

/**
* 
* current version: V1.0.01
* 
*/


typedef struct
{
	double	pan;   	//0 ~ 359.9999,单位:度 左减右加
	double	tilt;	//-90 ~ 90,单位:度 0位置为水平,垂直向下为-90; 垂直向上为90; 原则:下减上加
	double	angle_h;//水平视场角
	double	angle_v;//垂直视场角
	
	int		zoom_value; //保留
	int		ptz_state;  //保留 
	char 	reserve[128 - 40];
}ak_mt_ptz_info;


/**
 * ak_mt_math_get_version - get venc version
 * return: version string
 */
const char *ak_mt_math_get_version();

/**
 * ak_mt_math_uv_to_pt - Convert the coordinates (u,v) of position target in the image to to pan and tilt coordinates.
 * @in[IN]: current state's value,as pan tilt angle_h angle_v
 *		pan:[0,360);tilt:[-90,90];angle_h:(0,180); angle_v:(0,180);
 *
 * @u[IN]: target u ,normalization to 1; u:[0,1];
 * @v[IN]: target v ,normalization to 1; v:[0,1];
 * @out[OUT]: target's pan and tilt,pan:[0,360);tilt:[-90,90]
 * return: return AK_SUCCESS or error_code
 */
int ak_mt_math_uv_to_pt(ak_mt_ptz_info *in, ak_mt_ptz_info *out, double u, double v);

#endif
