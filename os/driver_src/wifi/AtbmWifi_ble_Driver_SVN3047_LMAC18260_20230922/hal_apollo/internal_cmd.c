#include <linux/module.h>
#include <linux/kobject.h>
#include <linux/kthread.h>
#include <linux/dcache.h>
#include <linux/netdevice.h>
#include <linux/rtnetlink.h>
#include <linux/poll.h>
#include "apollo.h"
#include "bh.h"
#include "hwio.h"
#include "wsm.h"
#include "sbus.h"
#include "debug.h"
#include "apollo_plat.h"
#include "sta.h"
#include "ap.h"
#include "scan.h"
#include "internal_cmd.h"
#include "svn_version.h"
#include "mac80211/rc80211_hmac.h"
#include "dev_ioctl.h"

unsigned int HW_READ_REG(unsigned int addr);
void HW_WRITE_REG(unsigned int addr, unsigned int data);


#define ATBM_WEXT_PROCESS_PARAMS(_start_pos,_length,_val,_process,_err_code,_exit_lable,_status)		\
do{													\
	char const *pos_next = NULL;					\
	pos_next = memchr(_start_pos,ATBM_SPACE,_length);	\
	if(pos_next == NULL) pos_next = memchr(_start_pos,ATBM_TAIL,_length);	\
	if(_process(_start_pos,pos_next - _start_pos,&_val) == _err_code){	\
		atbm_printk_err("%s:line(%d) err\n",__func__,__LINE__);			\
		_status = -EINVAL;											\
		goto _exit_lable;												\
	}															\
	pos_next++;													\
	_length -= (pos_next-_start_pos);										\
	_start_pos = pos_next;												\
}while(0)


extern struct atbm_common *g_hw_priv;

static struct atbm_gpio_config atbm_gpio_table[]=
{
	{
		.gpio = 4,
		.flags = 0,
		.fun_ctrl = {.base_addr	= 0x1740000C,.start_bit  = 0,.width		= 4,.val = 0,},
		.pup_ctrl = {.base_addr	= 0x1740000C,.start_bit  = 11,.width	= 1,.val = 0,},
		.pdu_ctrl = {.base_addr	= 0x1740000C,.start_bit  = 12,.width	= 1,.val = 0,},
		.dir_ctrl = {.base_addr	= 0x16800028,.start_bit  = 4,.width		= 1,.val = 0,},
		.out_val  = {.base_addr	= 0x16800024,.start_bit  = 4,.width		= 1,.val = 0,},
		.out_val  = {.base_addr	= 0x16800020,.start_bit  = 4,.width		= 1,.val = 0,},
	},
	{
		.gpio = 20,
		.flags = 0,
		.fun_ctrl = {.base_addr	= 0x1740002C,.start_bit  = 0,.width		= 4,.val = 0,},
		.pup_ctrl = {.base_addr	= 0x1740002C,.start_bit  = 11,.width	= 1,.val = 0,},
		.pdu_ctrl = {.base_addr	= 0x1740002C,.start_bit  = 12,.width	= 1,.val = 0,},
		.dir_ctrl = {.base_addr	= 0x16800028,.start_bit  = 20,.width	= 1,.val = 0,},
		.out_val  = {.base_addr	= 0x16800024,.start_bit  = 20,.width	= 1,.val = 0,},
		.in_val  = {.base_addr	= 0x16800020,.start_bit  = 20,.width	= 1,.val = 0,},
	},
	{
		.gpio = 21,
		.flags = 0,
		.fun_ctrl = {.base_addr	= 0x1740002C,.start_bit  = 16,.width	= 4,.val = 0,},
		.pup_ctrl = {.base_addr	= 0x1740002C,.start_bit  = 27,.width	= 1,.val = 0,},
		.pdu_ctrl = {.base_addr	= 0x1740002C,.start_bit  = 28,.width	= 1,.val = 0,},
		.dir_ctrl = {.base_addr	= 0x16800028,.start_bit  = 21,.width	= 1,.val = 0,},
		.out_val  = {.base_addr	= 0x16800024,.start_bit  = 21,.width	= 1,.val = 0,},
		.in_val  = {.base_addr	= 0x16800020,.start_bit  = 21,.width	= 1,.val = 0,},
	},
	{
		.gpio = 22,
		.flags = 0,
		.fun_ctrl = {.base_addr	= 0x17400030,.start_bit  = 0,.width		= 4,.val = 0,},
		.pup_ctrl = {.base_addr	= 0x17400030,.start_bit  = 11,.width	= 1,.val = 0,},
		.pdu_ctrl = {.base_addr	= 0x17400030,.start_bit  = 12,.width	= 1,.val = 0,},
		.dir_ctrl = {.base_addr	= 0x16800028,.start_bit  = 22,.width	= 1,.val = 0,},
		.out_val  = {.base_addr	= 0x16800024,.start_bit  = 22,.width	= 1,.val = 0,},
		.in_val  = {.base_addr	= 0x16800020,.start_bit  = 22,.width	= 1,.val = 0,},
	},
	{
		.gpio = 23,
		.flags = 0,
		.fun_ctrl = {.base_addr	= 0x17400030,.start_bit  = 16,.width	= 4,.val = 0,},
		.pup_ctrl = {.base_addr	= 0x17400030,.start_bit  = 27,.width	= 1,.val = 0,},
		.pdu_ctrl = {.base_addr	= 0x17400030,.start_bit  = 28,.width	= 1,.val = 0,},
		.dir_ctrl = {.base_addr	= 0x16800028,.start_bit  = 23,.width	= 1,.val = 0,},
		.out_val  = {.base_addr	= 0x16800024,.start_bit  = 23,.width	= 1,.val = 0,},
		.in_val  = {.base_addr	= 0x16800020,.start_bit  = 23,.width	= 1,.val = 0,},
	},
};
#define DCXO_TRIM_REG 0x16101410 //bit 5:0


#define ATBM_WSM_ADAPTIVE		"set_adaptive"ATBM_SPACE_STR
#define ATBM_WSM_TXPWR_DCXO		"set_txpwr_and_dcxo"ATBM_SPACE_STR
#define ATBM_WSM_TXPWR			"set_txpower"ATBM_SPACE_STR
#define ATBM_WSM_SET_FREQ		"set_freq"ATBM_SPACE_STR"%d"ATBM_SPACE_STR"%d"
#define ATBM_WSM_FIX_RATE		"lmac_rate"ATBM_SPACE_STR"%d"
#define ATBM_WSM_TOP_RATE		"lmac_max_rate"ATBM_SPACE_STR"%d"
#define ATBM_WSM_MIN_RATE		"lmac_min_rate"ATBM_SPACE_STR"%d"
#define ATBM_WSM_SET_RATE_POWER	"set_spec_rate_txpower_mode"ATBM_SPACE_STR"%d"ATBM_SPACE_STR"%d"

#ifdef CONFIG_ATBM_MONITOR_SPECIAL_MAC
#define ATBM_WSM_MONITOR_MAC	"set_sta_monitor"ATBM_SPACE_STR"%d"ATBM_SPACE_STR"%d"ATBM_SPACE_STR"%x,%x,%x,%x,%x,%x"
#endif
#define ATBM_WSM_CMD_LEN		1680
const char *chip_6038  = "6038";
const char *chip_6032  = "6032";
const char *chip_6032i = "6032i";
const char *chip_101B  = "101B";


unsigned char char2Hex(const char chart)
{
	unsigned char ret = 0;
	if((chart>='0')&&(chart<='9')){
		ret = chart-'0';		
	}else if((chart>='a')&&(chart<='f')){
		ret = chart - 'a'+0x0a;		
	}else if((chart>='A')&&(chart<='F')){
		ret = chart - 'A'+0x0a;
	}
	return ret;
}

/*
Func: str2mac
Param: 
	str->string format of MAC address
	i.e. 00:11:22:33:44:55
Return: 
	error -1
	OK 0
*/
int str2mac(char *dst_mac, char *src_str)
{
	int i;
	
	if(dst_mac == NULL || src_str == NULL)
		return -1;

	for(i=0; i<6; i++){
		dst_mac[i] = (char2Hex(src_str[i*3]) << 4) + (char2Hex(src_str[i*3 + 1]));
		atbm_printk_wext("str2mac: %x\n", dst_mac[i]);
	}

	return 0;	
}


int rate_txpower_cfg[64] = {0};
//#define RFIP_MCS_LUT_BASE_ADDR	(0xACBE000)
const u8 MCS_LUT_20M[36] = {	
							0x48,//1M/2M/
							0x48,//5.5M/11M/
							0x44, //6M/MCS0 /
							0x44, //9M/
							0x40, //12M/MCS1/
							0x40, //18M/MCS2/
							0x3C, //24M/MCS3/
							0x3C, //36M/MCS4/
							0x3C, //48M/MCS5/
							0x3C, //54M/MCS6/
							0x38,  //MCS7/
							0x44,//SU/ER_SU242 MCS0
							0x44,//SU/ER_SU242 MCS1
							0x40,//SU/ER_SU242 MCS2
							0x40,//SU MCS3
							0x3C,//SU MCS4
							0x3C,//SU MCS5
							0x3C,//SU MCS6
							0x38,//SU MCS7
							0x38,//SU MCS8
							0x38,//SU MCS9
							0x34,//SU MCS10
							0x34,//SU MCS11
							0x40,//ER_SU106 MCS0
							0x44,//TB MCS0
							0x44,//TB MCS1
							0x40,//TB MCS2
							0x40,//TB MCS3
							0x3C,//TB MCS4
							0x3C,//TB MCS5
							0x3C,//TB MCS6
							0x38,//TB MCS7
							0x38,//TB MCS8
							0x38,//TB MCS9
							0x34,//TB MCS10
							0x34,//TB MCS11
						};
 u8 MCS_LUT_20M_Modify[36] = {	
							0x48,//1M/2M/
							0x48,//5.5M/11M/
							0x44, //6M/MCS0 /
							0x44, //9M/
							0x40, //12M/MCS1/
							0x40, //18M/MCS2/
							0x3C, //24M/MCS3/
							0x3C, //36M/MCS4/
							0x3C, //48M/MCS5/
							0x3C, //54M/MCS6/
							0x38,  //MCS7/
							0x44,//SU/ER_SU242 MCS0
							0x44,//SU/ER_SU242 MCS1
							0x40,//SU/ER_SU242 MCS2
							0x40,//SU MCS3
							0x3C,//SU MCS4
							0x3C,//SU MCS5
							0x3C,//SU MCS6
							0x38,//SU MCS7
							0x38,//SU MCS8
							0x38,//SU MCS9
							0x34,//SU MCS10
							0x34,//SU MCS11
							0x40,//ER_SU106 MCS0
							0x44,//TB MCS0
							0x44,//TB MCS1
							0x40,//TB MCS2
							0x40,//TB MCS3
							0x3C,//TB MCS4
							0x3C,//TB MCS5
							0x3C,//TB MCS6
							0x38,//TB MCS7
							0x38,//TB MCS8
							0x38,//TB MCS9
							0x34,//TB MCS10
							0x34,//TB MCS11
						};

s8 MCS_LUT_20M_delta_mode[36] = {	
							0x0,//1M/2M/
							0x0,//5.5M/11M/
							0x0, //6M/MCS0 /
							0x0, //9M/
							0x0, //12M/MCS1/
							0x0, //18M/MCS2/
							0x0, //24M/MCS3/
							0x0, //36M/MCS4/
							0x0, //48M/MCS5/
							0x0, //54M/MCS6/
							0x0,  //MCS7/
							0x0,//SU/ER_SU242 MCS0
							0x0,//SU/ER_SU242 MCS1
							0x0,//SU/ER_SU242 MCS2
							0x0,//SU MCS3
							0x0,//SU MCS4
							0x0,//SU MCS5
							0x0,//SU MCS6
							0x0,//SU MCS7
							0x0,//SU MCS8
							0x0,//SU MCS9
							0x0,//SU MCS10
							0x0,//SU MCS11
							0x0,//ER_SU106 MCS0
							0x0,//TB MCS0
							0x0,//TB MCS1
							0x0,//TB MCS2
							0x0,//TB MCS3
							0x0,//TB MCS4
							0x0,//TB MCS5
							0x0,//TB MCS6
							0x0,//TB MCS7
							0x0,//TB MCS8
							0x0,//TB MCS9
							0x0,//TB MCS10
							0x0,//TB MCS11
						};
						
s8 MCS_LUT_20M_delta_bw[36] = {	
							0x0,//1M/2M/
							0x0,//5.5M/11M/
							0x0, //6M/MCS0 /
							0x0, //9M/
							0x0, //12M/MCS1/
							0x0, //18M/MCS2/
							0x0, //24M/MCS3/
							0x0, //36M/MCS4/
							0x0, //48M/MCS5/
							0x0, //54M/MCS6/
							0x0,  //MCS7/
							0x0,//SU/ER_SU242 MCS0
							0x0,//SU/ER_SU242 MCS1
							0x0,//SU/ER_SU242 MCS2
							0x0,//SU MCS3
							0x0,//SU MCS4
							0x0,//SU MCS5
							0x0,//SU MCS6
							0x0,//SU MCS7
							0x0,//SU MCS8
							0x0,//SU MCS9
							0x0,//SU MCS10
							0x0,//SU MCS11
							0x0,//ER_SU106 MCS0
							0x0,//TB MCS0
							0x0,//TB MCS1
							0x0,//TB MCS2
							0x0,//TB MCS3
							0x0,//TB MCS4
							0x0,//TB MCS5
							0x0,//TB MCS6
							0x0,//TB MCS7
							0x0,//TB MCS8
							0x0,//TB MCS9
							0x0,//TB MCS10
							0x0,//TB MCS11
						};


const u8 MCS_LUT_40M[36] = {	
							0x48, //1M/2M/
							0x48, //5.5M/11M/
							0x44, //6M/MCS0 /
							0x44, //9M/
							0x40, //12M/MCS1/
							0x40, //18M/MCS2/
							0x3C, //24M/MCS3/
							0x3C, //36M/MCS4/
							0x3C, //48M/MCS5/
							0x3C, //54M/MCS6/
							0x38,  //MCS7/
							0x44,//SU MCS0
							0x44,//SU MCS1
							0x40,//SU MCS2
							0x40,//SU MCS3
							0x3C,//SU MCS4
							0x3C,//SU MCS5
							0x3C,//SU MCS6
							0x38,//SU MCS7
							0x38,//SU MCS8
							0x38,//SU MCS9
							0x34,//SU MCS10
							0x34,//SU MCS11
							0x40,//ER_SU MCS0
							0x44,//TB MCS0
							0x44,//TB MCS1
							0x40,//TB MCS2
							0x40,//TB MCS3
							0x3C,//TB MCS4
							0x3C,//TB MCS5
							0x3C,//TB MCS6
							0x38,//TB MCS7
							0x38,//TB MCS8
							0x38,//TB MCS9
							0x34,//TB MCS10
							0x34,//TB MCS11
						};	
u8 MCS_LUT_40M_Modify[36] = {	
							0x48,//1M/2M/
							0x48,//5.5M/11M/
							0x44, //6M/MCS0 /
							0x44, //9M/
							0x40, //12M/MCS1/
							0x40, //18M/MCS2/
							0x3C, //24M/MCS3/
							0x3C, //36M/MCS4/
							0x3C, //48M/MCS5/
							0x3C, //54M/MCS6/
							0x38,  //MCS7/
							0x44,//SU/ER_SU242 MCS0
							0x44,//SU/ER_SU242 MCS1
							0x40,//SU/ER_SU242 MCS2
							0x40,//SU MCS3
							0x3C,//SU MCS4
							0x3C,//SU MCS5
							0x3C,//SU MCS6
							0x38,//SU MCS7
							0x38,//SU MCS8
							0x38,//SU MCS9
							0x34,//SU MCS10
							0x34,//SU MCS11
							0x40,//ER_SU MCS0
							0x44,//TB MCS0
							0x44,//TB MCS1
							0x40,//TB MCS2
							0x40,//TB MCS3
							0x3C,//TB MCS4
							0x3C,//TB MCS5
							0x3C,//TB MCS6
							0x38,//TB MCS7
							0x38,//TB MCS8
							0x38,//TB MCS9
							0x34,//TB MCS10
							0x34,//TB MCS11
						};

s8 MCS_LUT_40M_delta_mode[36] = {	
							0x0,//1M/2M/
							0x0,//5.5M/11M/
							0x0, //6M/MCS0 /
							0x0, //9M/
							0x0, //12M/MCS1/
							0x0, //18M/MCS2/
							0x0, //24M/MCS3/
							0x0, //36M/MCS4/
							0x0, //48M/MCS5/
							0x0, //54M/MCS6/
							0x0,  //MCS7/
							0x0,//SU/ER_SU242 MCS0
							0x0,//SU/ER_SU242 MCS1
							0x0,//SU/ER_SU242 MCS2
							0x0,//SU MCS3
							0x0,//SU MCS4
							0x0,//SU MCS5
							0x0,//SU MCS6
							0x0,//SU MCS7
							0x0,//SU MCS8
							0x0,//SU MCS9
							0x0,//SU MCS10
							0x0,//SU MCS11
							0x0,//ER_SU106 MCS0
							0x0,//TB MCS0
							0x0,//TB MCS1
							0x0,//TB MCS2
							0x0,//TB MCS3
							0x0,//TB MCS4
							0x0,//TB MCS5
							0x0,//TB MCS6
							0x0,//TB MCS7
							0x0,//TB MCS8
							0x0,//TB MCS9
							0x0,//TB MCS10
							0x0,//TB MCS11
						};
						
s8 MCS_LUT_40M_delta_bw[36] = {	
							0x0,//1M/2M/
							0x0,//5.5M/11M/
							0x0, //6M/MCS0 /
							0x0, //9M/
							0x0, //12M/MCS1/
							0x0, //18M/MCS2/
							0x0, //24M/MCS3/
							0x0, //36M/MCS4/
							0x0, //48M/MCS5/
							0x0, //54M/MCS6/
							0x0,  //MCS7/
							0x0,//SU/ER_SU242 MCS0
							0x0,//SU/ER_SU242 MCS1
							0x0,//SU/ER_SU242 MCS2
							0x0,//SU MCS3
							0x0,//SU MCS4
							0x0,//SU MCS5
							0x0,//SU MCS6
							0x0,//SU MCS7
							0x0,//SU MCS8
							0x0,//SU MCS9
							0x0,//SU MCS10
							0x0,//SU MCS11
							0x0,//ER_SU106 MCS0
							0x0,//TB MCS0
							0x0,//TB MCS1
							0x0,//TB MCS2
							0x0,//TB MCS3
							0x0,//TB MCS4
							0x0,//TB MCS5
							0x0,//TB MCS6
							0x0,//TB MCS7
							0x0,//TB MCS8
							0x0,//TB MCS9
							0x0,//TB MCS10
							0x0,//TB MCS11
						};
							

const char *rate_name[36] = {
"1M/2M",
"5.5M/11M",
"6M/MCS0",
"9M",
"12M/MCS1",
"18M/MCS2",
"24M/MCS3",
"36M/MCS4",
"48M/MCS5",
"54M/MCS6",
"MCS7",
"SU MCS0",
"SU MCS1",
"SU MCS2",
"SU MCS3",
"SU MCS4",
"SU MCS5",
"SU MCS6",
"SU MCS7",
"SU MCS8",
"SU MCS9",
"SU MCS10",
"SU MCS11",
"ER_SU MCS0",
"TB MCS0",
"TB MCS1",
"TB MCS2",
"TB MCS3",
"TB MCS4",
"TB MCS5",
"TB MCS6",
"TB MCS7",
"TB MCS8",
"TB MCS9",
"TB MCS10",
"TB MCS11",
};

int init_cfg_rate_power(void)
{
	int i = 0;
	atbm_printk_always("use default power\n");
	memset(rate_txpower_cfg, 0, sizeof(rate_txpower_cfg));
	for(i=0;i<sizeof(MCS_LUT_20M)/sizeof(MCS_LUT_20M[0]);i++)
		HW_WRITE_REG_BIT(RFIP_MCS_LUT_BASE_ADDR+0x10+(i<<2), 7, 0, (u32)MCS_LUT_20M[i]);
	for(i=0;i<sizeof(MCS_LUT_40M)/sizeof(MCS_LUT_40M[0]);i++)
		HW_WRITE_REG_BIT(RFIP_MCS_LUT_BASE_ADDR+0xA0+(i<<2), 7, 0, (u32)MCS_LUT_40M[i]);

	return 0;
}


int DCXOCodeWrite(struct atbm_common *hw_priv,int data)
{
#ifndef SPI_BUS
	u32 uiRegData;
	atbm_direct_read_reg_32(hw_priv, DCXO_TRIM_REG, &uiRegData);
	//hw_priv->sbus_ops->sbus_read_sync(hw_priv->sbus_priv,DCXO_TRIM_REG,&uiRegData,4);
	uiRegData &= ~0x00007F;

	uiRegData |= (data&0x7F);
	
	atbm_direct_write_reg_32(hw_priv, DCXO_TRIM_REG, uiRegData);
	//hw_priv->sbus_ops->sbus_write_sync(hw_priv->sbus_priv,DCXO_TRIM_REG,&uiRegData,4);
#endif
	return 0;
}

u8 DCXOCodeRead(struct atbm_common *hw_priv)
{	
#ifndef SPI_BUS

	u32 uiRegData;
	u8 dcxo;
	//u8 dcxo_hi,dcxo_low;

	atbm_direct_read_reg_32(hw_priv, DCXO_TRIM_REG, &uiRegData);
	dcxo = uiRegData&0x7F;
	
	return dcxo;
#else
	return 0;
#endif
}

void atbmwifi_efuse_read_byte(u32 byteIndex, u32 *value)
{
	
	HW_WRITE_REG(0x16b00000, (byteIndex<<8));
	*value = HW_READ_REG(0x16b00004);	
}

u32 atbmwifi_efuse_read_chip_flag(void)
{
	int byteIndex = 0;
	u32 value = 0;
	u8 buff[4] = {0};

	for (byteIndex = 0; byteIndex < 3; byteIndex++)
	{
		atbmwifi_efuse_read_byte(byteIndex, &value);
		buff[byteIndex] = value;
	}

	memcpy(&value, buff, 3);

	/*
		bit12:valid = 1
		bit13:ble,0:support;1:not support
		bit14:40M,0:support;1:not support
		bit15:LDPC,0:support;1:not support
		
		bit16:reserved
		bit17:reserved
	*/
	value = (value >> 12);
	value &= 0x3F;

	return value;
}


extern int atbm_direct_read_reg_32(struct atbm_common *hw_priv, u32 addr, u32 *val);
extern int atbm_direct_write_reg_32(struct atbm_common *hw_priv, u32 addr, u32 val);
extern struct etf_test_config etf_config;
//get chip crystal type
u32 GetChipCrystalType(struct atbm_common *hw_priv)
{	
#ifndef SPI_BUS
	u32 pin_reg;
	u32 pin_reg17400000;
	
	atbm_direct_read_reg_32(hw_priv, 0x17400000, &pin_reg17400000);
	atbm_direct_write_reg_32(hw_priv, 0x17400000, pin_reg17400000 | BIT(8));
	atbm_direct_read_reg_32(hw_priv, 0x17400000, &pin_reg17400000);
	if (pin_reg17400000 & BIT(17))
	{
		etf_config.chip_crystal_type = 1;
	}
	atbm_direct_read_reg_32(hw_priv, 0x16101010, &pin_reg);
	if (pin_reg & BIT(5))
	{
		etf_config.chip_crystal_type |= BIT(1);
	}
	if (pin_reg & BIT(27))
	{
		etf_config.chip_crystal_type |= BIT(2);
	}
	atbm_direct_write_reg_32(hw_priv, 0x17400000, pin_reg17400000);

	atbm_printk_always("crystal:%d\n",etf_config.chip_crystal_type);
	return pin_reg17400000;
#else
	return 0;
#endif
}

int ieee80211_set_channel(struct wiphy *wiphy,
				 struct net_device *netdev,
				 struct ieee80211_channel *chan,
				 enum nl80211_channel_type channel_type);

static void atbm_internal_cmd_scan_dump(struct ieee80211_internal_scan_request *scan_req)
{
	int i;
	if(scan_req->n_ssids){
		for(i = 0;i<scan_req->n_ssids;i++){
			atbm_printk_debug("%s: ssid[%s][%d]\n",__func__,scan_req->ssids[i].ssid,scan_req->ssids[i].ssid_len);
		}
	}	
	if(scan_req->n_channels){
		for(i = 0;i<scan_req->n_channels;i++){
			atbm_printk_debug("%s: channel[%d]\n",__func__,scan_req->channels[i]);
		}
	}
	if(scan_req->n_macs){
		for(i = 0;i<scan_req->n_macs;i++){
			atbm_printk_debug("%s: mac[%pM]\n",__func__,scan_req->macs[i].mac);
		}
	}
	atbm_printk_debug("%s: ie_len[%d]\n",__func__,scan_req->ie_len);
}

bool  atbm_internal_cmd_scan_build(struct ieee80211_local *local,struct ieee80211_internal_scan_request *req,
											   u8* channels,int n_channels,struct cfg80211_ssid *ssids,int n_ssids,
											   struct ieee80211_internal_mac *macs,int n_macs)
{
	u8* local_scan_ie;
	u8* scan_ie;
	int ie_len;
	/*
	*use default internal handle
	*/
	req->result_handle = NULL;
	req->priv = NULL;

	req->channels = channels;
	req->n_channels = n_channels;
	
	req->ssids =  ssids;
	req->n_ssids = n_ssids;

	req->macs = macs;
	req->n_macs = n_macs;

	req->no_cck = true;
	
	rcu_read_lock();
	local_scan_ie = rcu_dereference(local->internal_scan_ie);
	ie_len  = local->internal_scan_ie_len;

	if(local_scan_ie && ie_len){
		scan_ie = atbm_kzalloc(ie_len,GFP_ATOMIC);

		if(scan_ie == NULL){
			rcu_read_unlock();
			return false;
		}
		memcpy(scan_ie,local_scan_ie,ie_len);
		req->ies = scan_ie;
		req->ie_len = ie_len;
	}else {
		req->ies = NULL;
		req->ie_len = 0;
	}
	rcu_read_unlock();

	return true;
}
bool atbm_internal_cmd_scan_triger(struct ieee80211_sub_if_data *sdata,struct ieee80211_internal_scan_request *req)
{
	struct cfg80211_scan_request *scan_req = NULL;
	struct ieee80211_local *local  = sdata->local;
	u8 n_channels = 0;
	int i;
	struct wiphy *wiphy = local->hw.wiphy;
	u8 index;
	void *pos;
	void *pos_end;
	long status = 20*HZ;
	
	ASSERT_RTNL();
	ieee80211_scan_cancel(local);
	atbm_flush_workqueue(local->workqueue);
	
	mutex_lock(&local->mtx);

	if(!ieee80211_sdata_running(sdata)){
		atbm_printk_scan("%s:%d\n",__func__,__LINE__);
		goto err;
	}
	
	if (local->scan_req)
	{
		atbm_printk_scan("%s:%d\n",__func__,__LINE__);
		goto err;
	}
#ifdef CONFIG_ATBM_SUPPORT_P2P
	if (!list_empty(&local->roc_list))
	{
		goto err;
	}
#endif
	if (ieee80211_work_busy(local)) {
		
		atbm_printk_scan("%s(%s):work_list is not empty,pend scan\n",__func__,sdata->name);
		goto err;
	}
	
	if(atbm_ieee80211_suspend(sdata->local)==true){
		
		atbm_printk_err("ieee80211_scan drop:suspend\n");
		goto err;
	}
	
	if(req->n_channels == 0){
		for (i = 0; i < IEEE80211_NUM_BANDS; i++)
			if (wiphy->bands[i])
				n_channels += wiphy->bands[i]->n_channels;
	}else {
		n_channels = req->n_channels;
	}
	scan_req = atbm_kzalloc(sizeof(*scan_req)
			+ sizeof(*scan_req->ssids) * req->n_ssids
			+ sizeof(*scan_req->channels) * n_channels
			+ req->ie_len + req->n_channels + sizeof(struct ieee80211_internal_mac)*req->n_macs, GFP_KERNEL);
	
	if(scan_req == NULL){
		atbm_printk_scan("%s:atbm_kzalloc scan_req err\n",__func__);
		goto err;
	}
	pos = (void *)&scan_req->channels[n_channels];
	pos_end = (void*)((u8*)pos+sizeof(*scan_req->ssids) * req->n_ssids+
			  req->ie_len + req->n_channels + sizeof(struct ieee80211_internal_mac)*req->n_macs);
	/*
	*set channel
	*/
	if(req->n_channels){
		int freq;
		for (i = 0;i<req->n_channels;i++){
			
			if(req->channels[i] <= 14){
				freq = 2412+(req->channels[i] - 1)*5;
				if(req->channels[i] == 14)
						freq = 2484;
			}else {
				freq = 5000 + (5*req->channels[i]);
			}

			atbm_printk_debug("%s:channel(%d),freq(%d)\n",__func__,req->channels[i],freq);

			scan_req->channels[i] = ieee80211_get_channel(wiphy,freq);

			if(scan_req->channels[i] == NULL){
				goto err;
			}
		}
	}else {
		enum ieee80211_band band;
		i = 0;
		/* all channels */
		for (band = 0; band < IEEE80211_NUM_BANDS; band++) {
			int j;
			if (!wiphy->bands[band])
				continue;
			for (j = 0; j < wiphy->bands[band]->n_channels; j++) {
				scan_req->channels[i] =  &wiphy->bands[band]->channels[j];
				i++;
			}
		}
	}
	scan_req->n_channels = n_channels;
	/*
	*set ssid
	*/
	if( req->n_ssids){
		scan_req->ssids = (void *)pos;
		for(i=0;i<req->n_ssids;i++){			
			atbm_printk_debug("%s:scan ssid(%s)(%d)\n",__func__,req->ssids[i].ssid,req->ssids[i].ssid_len);
			scan_req->ssids[i].ssid_len = req->ssids[i].ssid_len;
			memcpy(scan_req->ssids[i].ssid,req->ssids[i].ssid,req->ssids[i].ssid_len);
		}
		pos = scan_req->ssids+req->n_ssids;
	}
	scan_req->n_ssids = req->n_ssids;
	/*
	*set macs
	*/
	local->internal_scan.req.n_macs = req->n_macs;	
	if(req->n_macs){
		local->internal_scan.req.macs = pos;
		memcpy(local->internal_scan.req.macs, req->macs,sizeof(struct ieee80211_internal_mac)*req->n_macs);
		pos = local->internal_scan.req.macs + req->n_macs;
	}
	/*
	*set ie
	*/
	if (req->ie_len) {		
		scan_req->ie = (void *)pos;
		memcpy((void*)scan_req->ie,req->ies,req->ie_len);
		scan_req->ie_len = req->ie_len;
		pos = (u8*)scan_req->ie+req->ie_len;
	}

	/*
	*set channel
	*/
	if(req->channels){
		local->internal_scan.req.channels = pos;
		memcpy(local->internal_scan.req.channels,req->channels,req->n_channels);
	    pos = local->internal_scan.req.channels+req->n_channels;
	}
	WARN_ON(pos != pos_end);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 1, 0))
	for (i = 0; i < IEEE80211_NUM_BANDS; i++)
		if (wiphy->bands[i])
			scan_req->rates[i] =
				(1 << wiphy->bands[i]->n_bitrates) - 1;
		
	scan_req->no_cck = req->no_cck;
#endif
	
	scan_req->wiphy = wiphy;

	local->internal_scan.req.n_channels = req->n_channels;	
	local->internal_scan.req.ies = (u8*)scan_req->ie;
	local->internal_scan.req.ie_len = scan_req->ie_len;
	local->internal_scan.req.ssids = scan_req->ssids;
	local->internal_scan.req.n_ssids = scan_req->n_ssids;
	memcpy(local->internal_scan.req.bssid,req->bssid,6);

	local->internal_scan.req.req_flags = req->req_flags;
	
	rcu_assign_pointer(local->internal_scan.req.result_handle,req->result_handle);
	rcu_assign_pointer(local->internal_scan.req.priv,req->priv);

	atbm_common_hash_list_init(local->internal_scan.mac_hash_list,IEEE80211_INTERNAL_SCAN_HASHENTRIES);
	
	for(index = 0;index<local->internal_scan.req.n_macs;index++){
		int hash_index = atbm_hash_index(local->internal_scan.req.macs[index].mac,6,IEEE80211_INTERNAL_SCAN_HASHBITS);
		struct hlist_head *hlist = &local->internal_scan.mac_hash_list[hash_index];
		hlist_add_head(&local->internal_scan.req.macs[index].hnode,hlist);
	}
	
	atbm_internal_cmd_scan_dump(&local->internal_scan.req);
	
	if(ieee80211_internal_scan_triger(sdata,scan_req) == false){
		atbm_printk_scan("%s scan triger err\n",__func__);
		
		for(index = 0;index<local->internal_scan.req.n_macs;index++){
			hlist_del(&local->internal_scan.req.macs[index].hnode);
		}
		rcu_assign_pointer(local->internal_scan.req.result_handle,NULL);
		rcu_assign_pointer(local->internal_scan.req.priv,NULL);
		memset(&local->internal_scan.req,0,sizeof(struct ieee80211_internal_scan_sta));
		goto err;
	}	
	if(local->scan_req_wrap.flags & IEEE80211_SCAN_REQ_SPILT){
		status = 60*HZ;
	}
	mutex_unlock(&local->mtx);

	status = wait_event_timeout(local->internal_scan_wq,atomic_read(&local->internal_scan_status) != IEEE80211_INTERNAL_SCAN_STATUS__WAIT,status);

	if(status == 0){
		return false;
	}

	atbm_printk_debug("%s: status(%ld)\n",__func__,status);

	if(atomic_read(&local->internal_scan_status) == IEEE80211_INTERNAL_SCAN_STATUS__ABORT)
		return false;
	
	return true;
err:
	if(scan_req)
		atbm_kfree(scan_req);
	mutex_unlock(&local->mtx);

	return false;
}

bool atbm_internal_cmd_stainfo(struct ieee80211_local *local,struct ieee80211_internal_sta_req *sta_req)
{
	struct ieee80211_internal_sta_info stainfo;
	struct sta_info *sta;
	u8 index = 0;
	struct hlist_head *hhead;
	struct hlist_node *node;
	struct ieee80211_internal_mac *mac_node;
	unsigned int hash_index = 0;
	bool (__rcu *sta_handle)(struct ieee80211_internal_sta_info *stainfo,void *priv);	
	struct hlist_head atbm_sta_mac_hlist[ATBM_COMMON_HASHENTRIES];
	unsigned char maxrate_id = 0;
	unsigned char dcm_used  = 0;
	int rate_val = 0;
	unsigned char support_40M_symbol = 0;
	
	memset(&stainfo,0,sizeof(struct ieee80211_internal_sta_info));	
	
	WARN_ON(sta_req->sta_handle == NULL);
	BUG_ON((sta_req->n_macs != 0)&&(sta_req->macs == NULL));
	
	atbm_common_hash_list_init(atbm_sta_mac_hlist,ATBM_COMMON_HASHENTRIES);

	for(index = 0;index<sta_req->n_macs;index++){
		hash_index = atbm_hash_index(sta_req->macs[index].mac,
								 6,ATBM_COMMON_HASHBITS);

		hhead = &atbm_sta_mac_hlist[hash_index];
		hlist_add_head(&sta_req->macs[index].hnode,&atbm_sta_mac_hlist[hash_index]);
	}
	
	mutex_lock(&local->sta_mtx);
	sta_handle = rcu_dereference(sta_req->sta_handle);
	list_for_each_entry_rcu(sta, &local->sta_list, list) {
		struct ieee80211_channel_state *chan_state = ieee80211_get_channel_state(local, sta->sdata);

		if(sta->sdata->vif.type != sta_req->type){
			continue;
		}
		
		if(sta->uploaded == false){
			continue;
		}
		
		if(sta->dead == true){
			continue;
		}
		
		if(sta_req->n_macs){
			u8 sta_needed = false;
			
			hash_index = atbm_hash_index(sta->sta.addr,6,ATBM_COMMON_HASHBITS);
			hhead = &atbm_sta_mac_hlist[hash_index];
			hlist_for_each(node,hhead){
				mac_node = hlist_entry(node,struct ieee80211_internal_mac,hnode);
				if (memcmp(mac_node->mac,sta->sta.addr,6) == 0){
					sta_needed = true;
					break;
				}
			}
			
			if(sta_needed == false){
				continue;
			}
		}
		stainfo.sdata = sta->sdata;
		
		if(sta_req->req_flag&IEEE80211_INTERNAL_STA_FLAGS_CHANNEL){
			stainfo.channel = channel_hw_value(chan_state->oper_channel);
			stainfo.channel_type = !!(test_sta_flag(sta,WLAN_STA_40M_CH)&&!test_sta_flag(sta,WLAN_STA_40M_CH_SEND_20M));
		}
		
		if(sta_req->req_flag&IEEE80211_INTERNAL_STA_FLAGS_SIGNAL){
			stainfo.signal = sta->last_signal2;
			stainfo.avg_signal = (s8) -atbm_ewma_read(&sta->avg_signal2);
		}
		
		if(sta_req->req_flag&IEEE80211_INTERNAL_STA_FLAGS_TXRXBYTE){
			stainfo.rx_bytes = sta->rx_bytes;
			stainfo.tx_bytes = sta->tx_bytes;
		}

		if(sta_req->req_flag&IEEE80211_INTERNAL_STA_FLAGS_TOPRATE){			
			//struct atbm_common *hw_priv = (struct atbm_common *)local->hw.priv;
			//struct atbm_vif *priv = (struct atbm_vif *)sta->sdata->vif.drv_priv;
			if(sta->sdata->vif.type == NL80211_IFTYPE_STATION){				
//				wsm_read_mib(hw_priv, WSM_MIB_ID_GET_RATE, &stainfo.top_rate, sizeof(unsigned int), priv->if_id);
//				rcu_read_lock();
//				stainfo = sta_info_get(stainfo->sdata, priv->bssid);
//				rcu_read_unlock();
				
			}else if(sta->sdata->vif.type == NL80211_IFTYPE_AP){
				//struct atbm_sta_priv *sta_priv = (struct atbm_sta_priv *)&sta->sta.drv_priv;
//				u8 sta_id = (u8)sta_priv->link_id;
//				if(sta_id != 0){					
//					wsm_write_mib(hw_priv, WSM_MIB_ID_GET_RATE, &sta_id, 1,priv->if_id);
//					wsm_read_mib(hw_priv, WSM_MIB_ID_GET_RATE, &stainfo.top_rate, sizeof(unsigned int), priv->if_id);
//				rcu_read_lock();
				//according to mac hex, find out sta link id
//				sta = ieee80211_find_sta(&stainfo->sdata->vif, sta->sta.addr);
//				if (sta){
//					sta_id = sta->aid;			
//		    		stainfo = container_of(sta, struct sta_info, sta);
//				}
//				rcu_read_unlock();
				}
			if(sta != NULL)
    		{
   		 		RATE_CONTROL_RATE *rate_info = (RATE_CONTROL_RATE*)sta->rate_ctrl_priv;
		 		if(rate_info!= NULL){
    	 			maxrate_id = rate_info->max_tp_rate_idx;
					if(rate_info->tx_rc_flags&IEEE80211_TX_RC_HE_DCM)
					{
						dcm_used = 1;
			}
					if(rate_info->bw_20M_cnt <= 0)
					{
						support_40M_symbol = 1;
					}
//					atbm_printk_always("id:%d,max_tp_rate_idx: %d\n", maxrate_id, rate_info->max_tp_rate_idx);
				}
   			}

			rate_val = atbm_get_rate_from_rateid(maxrate_id, dcm_used, support_40M_symbol);
			stainfo.top_rate = rate_val;
		}

		if(sta_req->req_flag&IEEE80211_INTERNAL_STA_FLAGS_SSID){
			rcu_read_lock();
			
			stainfo.ssid_len = 0;
			memset(stainfo.ssid,0,IEEE80211_MAX_SSID_LEN);
			
			if(sta->sdata->vif.type == NL80211_IFTYPE_STATION){
				struct cfg80211_bss *cbss = sta->sdata->u.mgd.associated;
				
				if(cbss){
					const char *ssid = NULL;
                    ssid = ieee80211_bss_get_ie(cbss, ATBM_WLAN_EID_SSID);
                    if(ssid){						
                        memcpy(stainfo.ssid, &ssid[2], ssid[1]);
                        stainfo.ssid_len = ssid[1];
                    }
				}				
			}else if(sta->sdata->vif.type == NL80211_IFTYPE_AP){
				struct ieee80211_bss_conf *bss_conf = &sta->sdata->vif.bss_conf;
				stainfo.ssid_len = bss_conf->ssid_len;
				if(stainfo.ssid_len)
					memcpy(stainfo.ssid,bss_conf->ssid,stainfo.ssid_len);
				
			}else {
				WARN_ON(1);
			}
			rcu_read_unlock();
		}
		memcpy(stainfo.mac,sta->sta.addr,6);
		stainfo.filled = sta_req->req_flag;
		if(sta_handle)
			sta_handle(&stainfo,sta_req->priv);
		
		memset(&stainfo,0,sizeof(struct ieee80211_internal_sta_info));
	}
	mutex_unlock(&local->sta_mtx);

	return true;
}
bool atbm_internal_cmd_monitor_req(struct ieee80211_sub_if_data *sdata,struct ieee80211_internal_monitor_req *monitor_req)
{
	struct ieee80211_local *local  = sdata->local;
	struct atbm_vif *priv = (struct atbm_vif *)sdata->vif.drv_priv;
	bool res = false;
	unsigned int freq;
	struct ieee80211_sub_if_data *other_sdata;
	
	struct ieee80211_channel *chan;
	enum nl80211_iftype old_type = sdata->vif.type;
	
	if(!ieee80211_sdata_running(sdata)){
		return false;
	}
	
	if(priv->join_status != ATBM_APOLLO_JOIN_STATUS_PASSIVE){
		return false;
	}

	list_for_each_entry(other_sdata, &local->interfaces, list){

		if(!ieee80211_sdata_running(other_sdata)){
			continue;
		}

		priv = (struct atbm_vif *)other_sdata->vif.drv_priv;

		if(priv->join_status != ATBM_APOLLO_JOIN_STATUS_PASSIVE){
			return false;
		}
	}
	if(ieee8011_channel_valid(&local->hw,monitor_req->ch) == false){
		return false;
	}
	
	switch(monitor_req->chtype){
	case NL80211_CHAN_NO_HT:
	case NL80211_CHAN_HT20:
	case NL80211_CHAN_HT40MINUS:
	case NL80211_CHAN_HT40PLUS:
		break;
	default:
		atbm_printk_err("error, %d\n", monitor_req->chtype);
		return false;
	}

	if(monitor_req->ch <= 14){
		freq = 2412+(monitor_req->ch - 1)*5;
	}else {
		freq = 5000 + (5*monitor_req->ch);
	}
	chan = ieee80211_get_channel(local->hw.wiphy, freq);

	if(chan == NULL){
		return false;
	}
	
	local->internal_monitor.req.ch = monitor_req->ch;
	local->internal_monitor.req.chtype = monitor_req->chtype;
	
	rcu_assign_pointer(local->internal_monitor.req.monitor_rx,monitor_req->monitor_rx);
	rcu_assign_pointer(local->internal_monitor.req.priv,monitor_req->priv);
	
	atbm_printk_debug("%s:[%s] channel %d\n",__func__,sdata->name,local->internal_monitor.req.ch);
	if(ieee80211_if_change_type(sdata, NL80211_IFTYPE_MONITOR)){
		res  = false;
		goto err;
	}

	if(ieee80211_set_channel(local->hw.wiphy,sdata->dev,chan,monitor_req->chtype)){
		goto err;
	}

	return true;
err:
	ieee80211_if_change_type(sdata,old_type);
	rcu_assign_pointer(local->internal_monitor.req.monitor_rx,NULL);
	rcu_assign_pointer(local->internal_monitor.req.priv,NULL);
	local->internal_monitor.req.ch = 0;
	
	return res;
}

bool atbm_internal_cmd_stop_monitor(struct ieee80211_sub_if_data *sdata)
{
	if(!ieee80211_sdata_running(sdata)){
		return false;
	}

	if(sdata->vif.type != NL80211_IFTYPE_MONITOR){
		return false;
	}

	ieee80211_if_change_type(sdata,NL80211_IFTYPE_STATION);

	rcu_assign_pointer(sdata->local->internal_monitor.req.monitor_rx,NULL);
	rcu_assign_pointer(sdata->local->internal_monitor.req.priv,NULL);

	synchronize_rcu();
	sdata->local->internal_monitor.req.ch = 0;
	sdata->local->internal_monitor.req.chtype = 0;
	
	return true;
}
bool atbm_internal_cmd_req_iftype(struct ieee80211_sub_if_data *sdata,struct ieee80211_internal_iftype_req *req)
{
	enum nl80211_iftype new_iftype;
	enum nl80211_iftype old_iftype = sdata->vif.type;
	struct ieee80211_local *local = sdata->local;
	bool change_channel = true;
	bool change_iftype  = true;
	
	ASSERT_RTNL();
	atbm_printk_debug("%s:type(%d),channel(%d)\n",__func__,req->if_type,req->channel);
	
	if (sdata->vif.type == NL80211_IFTYPE_STATION && sdata->u.mgd.associated){
		
		goto params_err;
	}
	
	if (sdata->vif.type == NL80211_IFTYPE_AP && sdata->u.ap.beacon){
		
		goto params_err;
	}
	
	switch(req->if_type){
	case IEEE80211_INTERNAL_IFTYPE_REQ__MANAGED:
		new_iftype = NL80211_IFTYPE_STATION;
		if(new_iftype == sdata->vif.type){
			change_iftype  = false;
		}
		change_channel = false;
		break;
	case IEEE80211_INTERNAL_IFTYPE_REQ__MONITOR:
		new_iftype = NL80211_IFTYPE_MONITOR;		
		if(new_iftype == sdata->vif.type){
			change_iftype = false;
		}
		break;
	default:
		goto params_err;
	}

	if(change_iftype == true){
		if(ieee80211_if_change_type(sdata, new_iftype)){
			goto params_err;
		}
	}
	if(change_channel == true) {
		struct ieee80211_channel *chan = ieee8011_chnum_to_channel(&sdata->local->hw,req->channel);

		if(chan == NULL){
			goto interface_err;
		}

		if(ieee80211_set_channel(local->hw.wiphy,sdata->dev,chan,NL80211_CHAN_HT20)){
			goto interface_err;
		}
	}

	return true;
interface_err:
	ieee80211_if_change_type(sdata,old_iftype);
params_err:
	return false;
}
bool atbm_internal_wsm_adaptive(struct atbm_common *hw_priv,struct ieee80211_internal_wsm_adaptive *adaptive)
{
	char* cmd = NULL;
	int len;
	bool res = true;
	
	cmd = atbm_kzalloc(ATBM_WSM_CMD_LEN,GFP_KERNEL);

	if(cmd == NULL){
		res = false;
		goto err;
	}
	
	len = snprintf(cmd,ATBM_WSM_CMD_LEN,ATBM_WSM_ADAPTIVE"%d",adaptive->enable);

	if(len<=0){
		res = false;
		goto err;
	}
	if(len+1>ATBM_WSM_CMD_LEN){
		res = false;
		goto err;
	}
	atbm_printk_debug("%s:wsm [%s][%d]\n",__func__,cmd,len);
	
	if( wsm_write_mib(hw_priv, WSM_MIB_ID_FW_CMD, cmd, len+1,0) < 0){
		res = false;
	}
	
err:
	if(cmd)
		atbm_kfree(cmd);
	return res;
}

bool atbm_internal_wsm_txpwr_dcxo(struct atbm_common *hw_priv,struct ieee80211_internal_wsm_txpwr_dcxo *txpwr_dcxo)
{
	int len;
	char* cmd = NULL;
	bool res = true;
	
	if(txpwr_dcxo->txpwr_L > 32 || txpwr_dcxo->txpwr_L < -32){
		atbm_printk_err("error, txpwr_L %d\n", txpwr_dcxo->txpwr_L);
		res = false;
		goto err;
	}
	
	if(txpwr_dcxo->txpwr_M > 32 || txpwr_dcxo->txpwr_M < -32){
		atbm_printk_err("error, txpwr_M %d\n", txpwr_dcxo->txpwr_M);
		res = false;
		goto err;
	}
	
	if(txpwr_dcxo->txpwr_H > 32 || txpwr_dcxo->txpwr_H < -32){
		atbm_printk_err("error, txpwr_H %d\n", txpwr_dcxo->txpwr_H);
		res = false;
		goto err;
	}
	
	if(txpwr_dcxo->dcxo > 127 || txpwr_dcxo->dcxo < 0){
		atbm_printk_err("error, dcxo %d\n", txpwr_dcxo->dcxo);
		res = false;
		goto err;
	}

	cmd = atbm_kzalloc(ATBM_WSM_CMD_LEN,GFP_KERNEL);

	if(cmd == NULL){
		res = false;
		goto err;
	}

	len = snprintf(cmd, ATBM_WSM_CMD_LEN,"set_txpwr_and_dcxo,%d,%d,%d,%d ",
		           txpwr_dcxo->txpwr_L,txpwr_dcxo->txpwr_M, txpwr_dcxo->txpwr_H, txpwr_dcxo->dcxo);

	if(len<=0){
		res = false;
		goto err;
	}
	if(len+1>ATBM_WSM_CMD_LEN){
		res = false;
		goto err;
	}
	atbm_printk_debug("%s:wsm [%s][%d]\n",__func__,cmd,len);
	if(wsm_write_mib(hw_priv, WSM_MIB_ID_FW_CMD, cmd, len+1, 0) < 0){
		res = false;
	}
err:
	if(cmd)
		atbm_kfree(cmd);
	return res;
}

bool atbm_internal_wsm_txpwr(struct atbm_common *hw_priv,struct ieee80211_internal_wsm_txpwr *txpwr)
{
	int len;
	char* cmd = NULL;
	bool res = true;
	/*
	*0,3,15,63
	*/
	if(txpwr->txpwr_indx != 0 && 
	   txpwr->txpwr_indx != 3 &&
	   txpwr->txpwr_indx != 15 &&
	   txpwr->txpwr_indx != 63){
		atbm_printk_err("error, txpwr_indx %d\n", txpwr->txpwr_indx);
		res = false;
		goto err;
	}

	cmd = atbm_kzalloc(ATBM_WSM_CMD_LEN,GFP_KERNEL);

	if(cmd == NULL){
		res = false;
		goto err;
	}

	len = snprintf(cmd, ATBM_WSM_CMD_LEN, ATBM_WSM_TXPWR"%d",txpwr->txpwr_indx);

	if(len<=0){
		res = false;
		goto err;
	}

	if(len+1>ATBM_WSM_CMD_LEN){
		res = false;
		goto err;
	}
	atbm_printk_debug("%s:wsm [%s][%d]\n",__func__,cmd,len);
	if(wsm_write_mib(hw_priv, WSM_MIB_ID_FW_CMD, cmd, len+1, 0) < 0){
		res = false;
	}
err:
	if(cmd)
		atbm_kfree(cmd);
	return res;
}
bool atbm_internal_wsm_set_rate(struct atbm_common *hw_priv,struct ieee80211_internal_rate_req *req)
{
	int len;
	char* cmd = NULL;
	bool res = true;

	cmd = atbm_kzalloc(ATBM_WSM_CMD_LEN,GFP_KERNEL);

	if(cmd == NULL){
		res = false;
		goto err;
	}

	if(req->flags & IEEE80211_INTERNAL_RATE_FLAGS_CLEAR_TX_RATE){
		len = snprintf(cmd, ATBM_WSM_CMD_LEN, ATBM_WSM_FIX_RATE,0);

		if(len<=0){
			res = false;
			goto err;
		}

		if(wsm_write_mib(hw_priv, WSM_MIB_ID_FW_CMD, cmd, len+1, 0) < 0){
			res = false;
			goto err;
		}

		memset(cmd,0,ATBM_WSM_CMD_LEN);
	}

	if(req->flags & IEEE80211_INTERNAL_RATE_FLAGS_CLEAE_TOP_RATE){
		len = snprintf(cmd, ATBM_WSM_CMD_LEN, ATBM_WSM_TOP_RATE,0);

		if(len<=0){
			res = false;
			goto err;
		}

		if(wsm_write_mib(hw_priv, WSM_MIB_ID_FW_CMD, cmd, len+1, 0) < 0){
			res = false;
			goto err;
		}

		memset(cmd,0,ATBM_WSM_CMD_LEN);
	}

	if(req->flags & IEEE80211_INTERNAL_RATE_FLAGS_CLEAR_MIN_RATE){
		len = snprintf(cmd, ATBM_WSM_CMD_LEN, ATBM_WSM_MIN_RATE,0);

		if(len<=0){
			res = false;
			goto err;
		}

		if(wsm_write_mib(hw_priv, WSM_MIB_ID_FW_CMD, cmd, len+1, 0) < 0){
			res = false;
			goto err;
		}

		memset(cmd,0,ATBM_WSM_CMD_LEN);
	}
	
	if(req->flags & IEEE80211_INTERNAL_RATE_FLAGS_SET_TX_RATE){
		len = snprintf(cmd, ATBM_WSM_CMD_LEN, ATBM_WSM_FIX_RATE,req->rate);

		if(len<=0){
			res = false;
			goto err;
		}

		if(wsm_write_mib(hw_priv, WSM_MIB_ID_FW_CMD, cmd, len+1, 0) < 0){
			res = false;
			goto err;
		}

		memset(cmd,0,ATBM_WSM_CMD_LEN);
	}

	if(req->flags & IEEE80211_INTERNAL_RATE_FLAGS_SET_TOP_RATE){
		len = snprintf(cmd, ATBM_WSM_CMD_LEN, ATBM_WSM_TOP_RATE,req->rate);

		if(len<=0){
			res = false;
			goto err;
		}

		if(wsm_write_mib(hw_priv, WSM_MIB_ID_FW_CMD, cmd, len+1, 0) < 0){
			res = false;
			goto err;
		}

		memset(cmd,0,ATBM_WSM_CMD_LEN);
	}

	if(req->flags & IEEE80211_INTERNAL_RATE_FLAGS_SET_MIN_RATE){
		len = snprintf(cmd, ATBM_WSM_CMD_LEN, ATBM_WSM_MIN_RATE,req->rate);

		if(len<=0){
			res = false;
			goto err;
		}

		if(wsm_write_mib(hw_priv, WSM_MIB_ID_FW_CMD, cmd, len+1, 0) < 0){
			res = false;
			goto err;
		}

		memset(cmd,0,ATBM_WSM_CMD_LEN);
	}
err:
	if(cmd)
		atbm_kfree(cmd);
	return res;
}

bool atbm_internal_wsm_set_rate_power(struct atbm_common *hw_priv,
												   struct ieee80211_internal_rate_power_req *req)
{
	#define MIN_RATE_INDEX	(0)
	#define MAX_RATE_INDEX	(10)
	#define MIN_POWER		(-16)
	#define MAX_POWER		(16)

	bool ret = true;
	char* cmd = NULL;
	int len = 0;
	
	if((req->rate_index < MIN_RATE_INDEX) ||(req->rate_index > MAX_RATE_INDEX)){
		ret = false;
		goto exit;
	}

	if((req->power < MIN_POWER) ||(req->power > MAX_POWER)){
		ret = false;
		goto exit;
	}
	
	cmd = atbm_kzalloc(ATBM_WSM_CMD_LEN,GFP_KERNEL);

	if(cmd == NULL){
		ret = false;
		goto exit;
	}

	len = snprintf(cmd, ATBM_WSM_CMD_LEN, ATBM_WSM_SET_RATE_POWER,req->rate_index,req->power);

	if(len <= 0){
		ret = false;
		goto exit;
	}

	if(wsm_write_mib(hw_priv, WSM_MIB_ID_FW_CMD, cmd, len+1, 0) < 0){
		ret = false;
		goto exit;
	}
	
exit:
	if(cmd)
		atbm_kfree(cmd);

	return ret;

	#undef MIN_RATE_INDEX
	#undef MAX_RATE_INDEX
	#undef MIN_POWER
	#undef MAX_POWER
}
static char spec_oui_buf[256];
static char *spec_oui = "NULL";
module_param(spec_oui,charp,0644);
MODULE_PARM_DESC(spec_oui,"special oui");
void atbm_set_special_oui(struct atbm_common *hw_priv, char *pdata, int len)
{
    memset(spec_oui_buf, 0, 256);
    memcpy(spec_oui_buf, pdata, len);
    spec_oui = spec_oui_buf;
}
static int wifi_tx_pw = 0;
static char wifi_txpw_buf[64]={0};
static char *wifi_txpw = "NULL";
module_param(wifi_txpw,charp,0644);
MODULE_PARM_DESC(wifi_txpw,"wifi tx power");

int atbm_get_tx_power(void)
{
	return wifi_tx_pw;
}

void atbm_set_tx_power(struct atbm_common *hw_priv, int txpw)
{
	char *p20, *p40, *pHT;
	
	wifi_tx_pw = txpw;

	if(wifi_tx_pw & BIT(0))
		p20 = "20M-High ";
	else
		p20 = "20M-Normal ";


	if(wifi_tx_pw & BIT(1))
		p40 = "40M-High ";
	else
		p40 = "40M-Normal ";

	if((hw_priv->channel_type == NL80211_CHAN_HT20)||(hw_priv->channel_type == NL80211_CHAN_NO_HT))
		pHT = "20M-Mode";
	else
		pHT = "40M-Mode";

	memset(wifi_txpw_buf, 0, sizeof(wifi_txpw_buf));
	sprintf(wifi_txpw_buf, "%s, %s, %s", p20, p40, pHT);
	wifi_txpw = wifi_txpw_buf;

	return;
}												   
#define ATBM_SPECIAL_FREQ_MAX_LEN		128
static char wifi_freq_buf[ATBM_SPECIAL_FREQ_MAX_LEN]={0};
static char *wifi_freq = "NULL";
module_param(wifi_freq,charp,0644);
MODULE_PARM_DESC(wifi_freq,"wifi freq");
void atbm_set_freq(struct ieee80211_local *local)
{

   struct hlist_head *hlist;
   struct hlist_node *node;
   struct hlist_node *node_temp;
   struct ieee80211_special_freq *freq_node;
   int hash_index = 0;
   int n_freqs = 0;
   int len = 0;
   int total_len = 0;
   char *freq_show = wifi_freq_buf;
   
   memset(freq_show,0,ATBM_SPECIAL_FREQ_MAX_LEN);
   
   for(hash_index = 0;hash_index<ATBM_COMMON_HASHENTRIES;hash_index++){
	   hlist = &local->special_freq_list[hash_index];
	   hlist_for_each_safe(node,node_temp,hlist){
		   freq_node = hlist_entry(node,struct ieee80211_special_freq,hnode);
		   n_freqs ++ ;
		   len = scnprintf(freq_show+total_len,ATBM_SPECIAL_FREQ_MAX_LEN-total_len,"ch:%d, freq:%d \n",
			   channel_hw_value(freq_node->channel),freq_node->freq);
		   total_len += len;
	   }
   }

   if(n_freqs == 0){
	   wifi_freq = "NULL";
   }else {
	   wifi_freq = wifi_freq_buf;
   }
   
#if 0
   int i;
   
   memset(wifi_freq_buf, 0, sizeof(wifi_freq_buf));
   for(i=0; i<CHANNEL_NUM; i++){
	   if(pdata[i].flag == 1){
		   sprintf(wifi_freq_buf+strlen(wifi_freq_buf), "ch:%d, freq:%d \n", i+1, pdata[i].special_freq);
	   }
   }
   
   wifi_freq = wifi_freq_buf;

   return;
#endif
}
bool atbm_internal_freq_set(struct ieee80211_hw *hw,struct ieee80211_internal_set_freq_req *req)
{
	struct ieee80211_local *local = hw_to_local(hw);
	struct atbm_common *hw_priv = (struct atbm_common *)hw->priv;
	struct ieee80211_channel *channel;
	char* cmd = NULL;
	int len;
	bool res = true;
	struct ieee80211_special_freq special_req;
	
	ASSERT_RTNL();

	channel = ieee8011_chnum_to_channel(hw,req->channel_num);

	if(channel == NULL){
		res = false;
		goto out;
	}
	
	if(req->set == false){
		req->freq = channel_center_freq(channel);
	}
	
	if((req->freq < 2300) || (req->freq>2600)){
		res = false;
		goto out;
	}
	
	mutex_lock(&local->mtx);
	__ieee80211_recalc_idle(local);
	mutex_unlock(&local->mtx);

	if((local->hw.conf.flags & IEEE80211_CONF_IDLE) == 0){
		res = false;
		goto out;
	}

	cmd = atbm_kzalloc(ATBM_WSM_CMD_LEN,GFP_KERNEL);

	if(cmd == NULL){
		res = false;
		goto out;
	}

	len = snprintf(cmd, ATBM_WSM_CMD_LEN, ATBM_WSM_SET_FREQ,req->channel_num,(int)req->freq);

	if(len <= 0){
		res = false;
		goto out;
	}

	if(len+1>ATBM_WSM_CMD_LEN){
		res = false;
		goto out;
	}
	special_req.channel = channel;
	special_req.freq    = req->freq;
	
	if(channel_center_freq(channel) != req->freq){		
		if(ieee80211_special_freq_update(local,&special_req) == false){
			res = false;
			goto out;
		}
	}else {
		ieee80211_special_freq_clear(local,&special_req);
	}
	if(wsm_write_mib(hw_priv, WSM_MIB_ID_FW_CMD, cmd, len+1, 0) < 0){
		ieee80211_special_freq_clear(local,&special_req);
		res = false;
		goto out;
	}
out:
	if(cmd)
		atbm_kfree(cmd);

	return res;
}
bool atbm_internal_channel_auto_select(struct ieee80211_sub_if_data *sdata,
													  struct ieee80211_internal_channel_auto_select_req *req)
{
	struct ieee80211_internal_scan_request scan_req;
	
	scan_req.req_flags = IEEE80211_INTERNAL_SCAN_FLAGS__CCA;
	/*
	*all off supported channel will be scanned
	*/
	scan_req.channels   = NULL;
	scan_req.n_channels = 0;
	scan_req.macs       = NULL;
	scan_req.n_macs     = 0;
	scan_req.ies		= NULL;
	scan_req.ie_len		= 0;
	scan_req.no_cck     = false;
	scan_req.priv		= NULL;
	scan_req.result_handle = NULL;
	scan_req.ssids      = NULL;
	scan_req.n_ssids    = 0;

	return atbm_internal_cmd_scan_triger(sdata,&scan_req);
}

static bool atbm_internal_channel_auto_select_results_handle(struct ieee80211_hw *hw,struct atbm_internal_scan_results_req *req,struct ieee80211_internal_scan_sta *sta_info)
{
	struct ieee80211_internal_channel_auto_select_results *cca_results = (struct ieee80211_internal_channel_auto_select_results *)req->priv;
	s8 signal = (s8)sta_info->signal;
	u8 cur_channel = sta_info->channel;
	u8 index = 0;
	struct ieee80211_channel *channel;
	
	if(ieee8011_channel_valid(hw,cur_channel) == false){
		return false;
	}

	if(sta_info->cca == false){
		return false;
	}
	
	req->n_stas ++;
	cca_results->n_aps[cur_channel-1]++;
	
	if(cca_results->version == 1)
		cca_results->weight[cur_channel-1] += ieee80211_rssi_weight(signal);
	else 
		cca_results->weight[cur_channel-1]++;
	
	channel = ieee8011_chnum_to_channel(hw,cur_channel);

	if(channel_in_special(channel) == true){
		return true;
	}
	/*
	*2.4G channel
	*/
	atbm_printk_debug("ssid[%s],channel[%d],signal(%d)\n",sta_info->ssid,cur_channel,signal);
	/*
	*channel 1-13
	*weight[x] +=  val[x] + val[x-1] + val[x-2] + val[x-3] + val[x+1] + val[x+2] + val[x+3]
	*/
	if(cur_channel<=13){
		u8 low;
		u8 high;

		low = cur_channel>=4?cur_channel-3:1;
		high = cur_channel<= 10 ? cur_channel+3:13;
		
		for(index=cur_channel+1;index<=high;index++){
			channel = ieee8011_chnum_to_channel(hw,index);
			/*
			*skip special freq
			*/
			if(channel_in_special(channel) == true){
				atbm_printk_debug("%s:skip special freq(%d)\n",__func__,channel_hw_value(channel));
				continue;
			}
			
			if(cca_results->version == 1)
				cca_results->weight[index-1] += ieee80211_rssi_weight(signal - 2*(index-cur_channel));
			else 
				cca_results->weight[index-1] ++;
		}

		for(index=cur_channel-1;index>=low;index--){
			channel = ieee8011_chnum_to_channel(hw,index);
			/*
			*skip special freq
			*/
			if(channel_in_special(channel) == true){
				atbm_printk_debug("%s:skip special freq(%d)\n",__func__,channel_hw_value(channel));
				continue;
			}
			if(cca_results->version == 1)
				cca_results->weight[index-1] += ieee80211_rssi_weight(signal - 2*(cur_channel-index));
			else 
				cca_results->weight[index-1] ++;
		}
	}
	/*
	*channel 14
	*/
	else if(cur_channel == 14){
		
	}
	/*
	*5G channel
	*/
	else {
		
	}

	for(index = 0;index<IEEE80211_ATBM_MAX_SCAN_CHANNEL_INDEX;index++){
		atbm_printk_debug("weight[%d]=[%d]\n",index,cca_results->weight[index]);
	}
	return true;
}
bool atbm_internal_channel_auto_select_results(struct ieee80211_sub_if_data *sdata,
												struct ieee80211_internal_channel_auto_select_results *results)
{
	#define ATBM_BUSY_RATIO_MIN		100
	struct atbm_internal_scan_results_req results_req;
	struct ieee80211_local *local = sdata->local;
	u8 *busy_ratio;
	u8 i;
	u32 min_ap_num = (u32)(-1);
	u8  min_busy_ratio = 128;
	u8  min_ap_num_ration = 128;
	u8 channel = 0;
	int band;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0))
	u32 ignore_flags = IEEE80211_CHAN_DISABLED;
#endif
	struct ieee80211_supported_band *sband;
	u8 ignor_channel_mask[IEEE80211_ATBM_MAX_SCAN_CHANNEL_INDEX];
	u8 channel_mask[IEEE80211_ATBM_MAX_SCAN_CHANNEL_INDEX];

	results_req.n_stas = 0;
	results_req.flush = true;
	results_req.priv = results;
	results_req.result_handle = atbm_internal_channel_auto_select_results_handle;
	busy_ratio = ieee80211_scan_cca_val_get(&local->hw);
	
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0))
	ignore_flags |= IEEE80211_CHAN_NO_OFDM;
#endif
	if(ieee80211_scan_internal_req_results(local,&results_req) == false){
		goto err;
	}
	
	for(i = 0;i<14;i++){
		atbm_printk_debug("busy_ratio[%d]=[%d]\n",i,busy_ratio[i]);
	}
	
	memset(ignor_channel_mask,0,IEEE80211_ATBM_MAX_SCAN_CHANNEL_INDEX);
	memset(channel_mask,1,IEEE80211_ATBM_MAX_SCAN_CHANNEL_INDEX);

	for(i= 0;i<results->ignore_n_channels;i++){
		
		BUG_ON(results->ignore_channels == NULL);
		
		if(ieee8011_channel_valid(&local->hw,results->ignore_channels[i]) == false){
			goto err;
		}
		ignor_channel_mask[results->ignore_channels[i]-1] = 1;
		
		atbm_printk_debug("%s channel %d ignored\n",__func__,results->ignore_channels[i]);
	}

	if(results->n_channels){
		memset(channel_mask,0,IEEE80211_ATBM_MAX_SCAN_CHANNEL_INDEX);
		for(i = 0;i<results->n_channels;i++){
			BUG_ON(results->channels == NULL);
			if(ieee8011_channel_valid(&local->hw,results->channels[i]) == false){
				goto err;
			}

			channel_mask[results->channels[i]-1] = 1;
		}
	}
	for (band = 0; band < IEEE80211_NUM_BANDS; band++) {
		
		sband = local->hw.wiphy->bands[band];
		
		if (!sband)
			continue;
		/*
		*2.4G channel and 5G
		*/
		for(i = 0;i<sband->n_channels;i++){
			/*
			*0 means that the channel do not process cca
			*/
			if(busy_ratio[channel_hw_value(&sband->channels[i])-1] == 0){
				continue;
			}
			
			if(ignor_channel_mask[channel_hw_value(&sband->channels[i])-1] == 1){
				continue;
			}

			if(channel_mask[channel_hw_value(&sband->channels[i])-1] == 0){
				continue;
			}
			/*
			*special freq must be skiped
			*/
			if(channel_in_special(&sband->channels[i])){
				atbm_printk_debug("%s:skip special freq(%d)\n",__func__,channel_hw_value(&sband->channels[i]));
				continue;
			}
			/*
			*some disabled channel must be skiped
			*/
			/**
			if(ignore_flags&sband->channels[i].flags){
				atbm_printk_debug("%s: channel[%d] not support ofdm\n",__func__,channel_hw_value(&sband->channels[i]));
				continue;
			}
			*/
		//	atbm_printk_err("\n");
	//		atbm_printk_err("channel[%d] min_ap_num [%d]  min_ap_num_ration[%d] min_busy_ratio[%d] \n",channel,min_ap_num,min_ap_num_ration,min_busy_ratio);
	//		atbm_printk_err("i = %d , busy_ratio[%d] = %d \n",i,channel_hw_value(&sband->channels[i])-1,busy_ratio[channel_hw_value(&sband->channels[i])-1]);
			
			if(busy_ratio[channel_hw_value(&sband->channels[i])-1]<ATBM_BUSY_RATIO_MIN){

				if(results->weight[channel_hw_value(&sband->channels[i])-1]<=min_ap_num){
					if(results->weight[channel_hw_value(&sband->channels[i])-1]==min_ap_num){
						if(busy_ratio[channel_hw_value(&sband->channels[i])-1]<=min_ap_num_ration){
							min_ap_num = results->weight[channel_hw_value(&sband->channels[i])-1];
							channel = channel_hw_value(&sband->channels[i]);
							min_ap_num_ration = busy_ratio[channel_hw_value(&sband->channels[i])-1];
						}
					}else {
						min_ap_num = results->weight[channel_hw_value(&sband->channels[i])-1];
						channel = channel_hw_value(&sband->channels[i]);
						min_ap_num_ration = busy_ratio[channel_hw_value(&sband->channels[i])-1];
					}
				}
				
			}else if(min_ap_num == (u32)(-1)){
				if(busy_ratio[channel_hw_value(&sband->channels[i])-1]<min_busy_ratio){
					min_busy_ratio = busy_ratio[channel_hw_value(&sband->channels[i])-1];
					channel = channel_hw_value(&sband->channels[i]);
				}
			}
		}			
	}

	if(channel == 0){
		//WARN_ON(channel == 0);
		atbm_printk_err("auto select fail! \n");
		goto err;
	}
	atbm_printk_debug("auto_select channel %d\n",channel);
	memcpy(results->busy_ratio,busy_ratio,IEEE80211_ATBM_MAX_SCAN_CHANNEL_INDEX);
	results->susgest_channel = channel;
	ieee80211_scan_cca_val_put(&local->hw);
	return true;
err:
	ieee80211_scan_cca_val_put(&local->hw);
	return false;
}
#ifdef CONFIG_ATBM_MONITOR_SPECIAL_MAC
bool atbm_internal_mac_monitor(struct ieee80211_hw *hw,struct ieee80211_internal_mac_monitor *monitor)
{
	
	struct atbm_common *hw_priv = (struct atbm_common *)hw->priv;
	char* cmd = NULL;
	int len = 0;
	bool ret = true;

	cmd = atbm_kzalloc(ATBM_WSM_CMD_LEN,GFP_KERNEL);

	if(cmd == NULL){
		ret = false;
		goto exit;
	}
	
	if(monitor->flags & (IEEE80211_INTERNAL_MAC_MONITOR_START | IEEE80211_INTERNAL_MAC_MONITOR_STOP)){

		atbm_printk_err("mac_monitor:enable(%d),mac[%pM]\n",__func__,
						!!(monitor->flags&IEEE80211_INTERNAL_MAC_MONITOR_START),
						monitor->mac);
		len = scnprintf(cmd, ATBM_WSM_CMD_LEN, ATBM_WSM_MONITOR_MAC,monitor->index,
						!!(monitor->flags&IEEE80211_INTERNAL_MAC_MONITOR_START),
						monitor->mac[0],monitor->mac[1],
						monitor->mac[2],monitor->mac[3],
						monitor->mac[4],monitor->mac[5]);

		if(len<=0){
			ret = false;
			goto exit;
		}

		if(wsm_write_mib(hw_priv, WSM_MIB_ID_FW_CMD, cmd, len+1, 0) < 0){
			ret = false;
			goto exit;
		}
	}

	if(monitor->flags & IEEE80211_INTERNAL_MAC_MONITOR_RESULTS){
		
		int i = 0;
		if(wsm_read_mib(hw_priv,WSM_MIB_ID_GET_MONITOR_MAC_STATUS,cmd,ATBM_WSM_CMD_LEN,0) != 0){
			ret = false;
			goto exit;
		}

		for (i = 0;i<IEEE80211_INTERNAL_MAC_MONITOR_RESULTS;i++){
			monitor->reults[i].found     = 	*cmd++;
			monitor->reults[i].rssi      = 	*cmd++;
			monitor->reults[i].forcestop =	*cmd++;
			monitor->reults[i].used   	 =	*cmd++;
			monitor->reults[i].index     =  *cmd++;
			monitor->reults[i].enabled   =  *cmd++; 
			memcpy(monitor->reults[i].mac,cmd,6); cmd += 6;
			monitor->reults[i].delta_time = __le32_to_cpu(*((u32*)cmd)); cmd += 4;

			if(monitor->reults[i].used == 0){
				monitor->reults[i].used = 1;
				break;
			}
		}
	}
	
exit:
	if(cmd)
		atbm_kfree(cmd);
	return ret;
}
#endif
bool atbm_internal_request_chip_cap(struct ieee80211_hw *hw,struct ieee80211_internal_req_chip *req)
{
	struct atbm_common *hw_priv = (struct atbm_common *)hw->priv;

	if(req->flags & IEEE80211_INTERNAL_REQ_CHIP_FLAGS__CHIP_VER){
		if(hw_priv->wsm_caps.firmwareCap &CAPABILITIES_EFUSE8){
			req->chip_version = chip_6038;
		}else if(hw_priv->wsm_caps.firmwareCap &CAPABILITIES_EFUSEI){
			req->chip_version = chip_6032i;
		}else if(hw_priv->wsm_caps.firmwareCap &CAPABILITIES_EFUSEB){
			req->chip_version = chip_101B;
		}else {
			req->chip_version = chip_6032i;
		}
	}

	/*other code */

	return true;
}
#ifdef CONFIG_ATBM_SUPPORT_AP_CONFIG
bool atbm_internal_update_ap_conf(struct ieee80211_sub_if_data *sdata,
									     struct ieee80211_internal_ap_conf *conf_req,bool clear)
{
	
	if(!ieee80211_sdata_running(sdata)){
		atbm_printk_scan("%s:%d\n",__func__,__LINE__);
		goto err;
	}

	if(conf_req&&conf_req->channel){
		if(ieee8011_channel_valid(&sdata->local->hw,(int)conf_req->channel) == false){
			goto err;
		}
	}

	return ieee80211_update_ap_config(sdata,conf_req,clear);
err:
	return false;
}
#endif
int atbm_internal_addr_read_bit(struct atbm_common *hw_priv,u32 addr,u8 endBit,
	u8 startBit,u32 *data )
{                                                              
	u32	reg_val=0;                                        
	u32 regmask=0;
	int ret = 0;
	
	ret=atbm_direct_read_reg_32(hw_priv,addr,&reg_val); 
	if(ret<0){
		goto rw_end;
	}                             
	regmask = ~((1<<startBit) -1);                               
	regmask &= ((1<<endBit) -1)|(1<<endBit);                     
	reg_val &= regmask;                                      
	reg_val >>= startBit; 
	
	*data = reg_val;
rw_end:
	return ret;
}   

int atbm_internal_addr_write_bit(struct atbm_common *hw_priv,u32 addr,u8 endBit,
											u8 startBit,u32 data)
{                                                              
	u32	reg_val=0;                                        
	u32 regmask=0;
	int ret = 0;
	
	ret = atbm_direct_read_reg_32(hw_priv,addr,&reg_val); 
	
	if(ret<0){
		atbm_printk_err("%s:read err\n",__func__);
		goto rw_end;
	} 
	atbm_printk_err("%s:ret(%d)\n",__func__,ret);
	regmask = ~((1<<startBit) -1);                               
	regmask &= ((1<<endBit) -1)|(1<<endBit);                     
	reg_val &= ~regmask;                                      
	reg_val |= (data <<startBit)&regmask;                     
	ret = atbm_direct_write_reg_32(hw_priv,addr,reg_val);
	
	if(ret<0)
	{
		atbm_printk_err("%s:write err\n",__func__);
		goto rw_end;
	}
	
	if(ret)
		ret = 0;
rw_end:
	atbm_printk_err("%s:ret(%d)\n",__func__,ret);

	return ret;
}  

static int atbm_internal_gpio_set(struct atbm_common *hw_priv,struct atbm_ctr_addr *gpio_addr)
{
	unsigned int status = -1; 
	
	if(atbm_bh_is_term(hw_priv)){
		atbm_printk_err("%s:atbm term\n",__func__);
		goto exit;
	}
	
	status = atbm_internal_addr_write_bit(hw_priv,gpio_addr->base_addr,
			gpio_addr->start_bit+gpio_addr->width,gpio_addr->start_bit,gpio_addr->val);
exit:
	return status;
}

static int atbm_internal_gpio_get(struct atbm_common *hw_priv,struct atbm_ctr_addr *gpio_addr)
{
	unsigned int status = -1; 
	
	if(atbm_bh_is_term(hw_priv)){
		atbm_printk_err("%s:atbm term\n",__func__);
		goto exit;
	}
	
	status = atbm_internal_addr_read_bit(hw_priv,gpio_addr->base_addr,
			gpio_addr->start_bit+gpio_addr->width-1,gpio_addr->start_bit,&gpio_addr->val);
exit:
	return status;
}

static struct atbm_gpio_config *atbm_internal_gpio_reqest(struct atbm_common *hw_priv,int gpio)
{
	int i = 0;
	struct atbm_gpio_config *gpio_dev = NULL;
	
	for(i = 0;i < ARRAY_SIZE(atbm_gpio_table);i++){
		gpio_dev = &atbm_gpio_table[i];
		if(gpio_dev->gpio == gpio){
			return gpio_dev;
		}
	}

	return NULL;
	
}
bool atbm_internal_gpio_config(struct atbm_common *hw_priv,int gpio,bool dir ,bool pu,bool default_val)
{
	struct atbm_gpio_config *gpio_dev = NULL;
	bool ret = true;
	int status = -1;
	
	gpio_dev = atbm_internal_gpio_reqest(hw_priv,gpio);

	if(gpio_dev == NULL){
		atbm_printk_err("%s:gpio (%d) is err\n",__func__,gpio);
		ret =  false;
		goto exit;
	}

	gpio_dev->fun_ctrl.val = 3;
	gpio_dev->dir_ctrl.val = dir == true ? 1:0;
	gpio_dev->pup_ctrl.val = pu  == true ? 1:0;
	gpio_dev->pdu_ctrl.val = pu  == false ? 1:0;

	status = atbm_internal_gpio_set(hw_priv,&gpio_dev->fun_ctrl);

	if(status){
		atbm_printk_err("%s:gpio function(%d)(%d) is err\n",__func__,gpio,status);
		ret =  false;
		goto exit;
	}

	status = atbm_internal_gpio_set(hw_priv,&gpio_dev->dir_ctrl);
	
	if(status){
		atbm_printk_err("%s:gpio dir(%d) is err\n",__func__,gpio);
		ret =  false;
		goto exit;
	}

	status = atbm_internal_gpio_set(hw_priv,&gpio_dev->pup_ctrl);
	
	if(status){
		atbm_printk_err("%s:gpio pup(%d) is err\n",__func__,gpio);
		ret =  false;
		goto exit;
	}

	status = atbm_internal_gpio_set(hw_priv,&gpio_dev->pup_ctrl);
	
	if(status){
		atbm_printk_err("%s:gpio pdu(%d) is err\n",__func__,gpio);
		ret =  false;
		goto exit;
	}

	if(dir == true){
		gpio_dev->out_val.val = default_val == true ? 1:0;
		status = atbm_internal_gpio_set(hw_priv,&gpio_dev->out_val);
		if(status){
			atbm_printk_err("%s:gpio out(%d) is err\n",__func__,gpio);
			ret =  false;
			goto exit;
		}
	}

	gpio_dev->flags = ATBM_GPIO_CONFIG__FUNCTION_CONFIGD;

	if(dir == true)
		gpio_dev->flags |= ATBM_GPIO_CONFIG__OUTPUT;
	else
		gpio_dev->flags |= ATBM_GPIO_CONFIG__INPUT;

	if(pu)
		gpio_dev->flags |= ATBM_GPIO_CONFIG__PUP;
	else
		gpio_dev->flags |= ATBM_GPIO_CONFIG__PUD;
exit:	
	return ret;
}

bool atbm_internal_gpio_output(struct atbm_common *hw_priv,int gpio,bool set)
{
	struct atbm_gpio_config *gpio_dev = NULL;
	bool ret =true;
	
	gpio_dev = atbm_internal_gpio_reqest(hw_priv,gpio);

	if(gpio_dev == NULL){
		atbm_printk_err("%s:gpio (%d) is err\n",__func__,gpio);
		ret =  false;
		goto exit;
	}

	if(!(gpio_dev->flags & ATBM_GPIO_CONFIG__FUNCTION_CONFIGD)){
		atbm_printk_err("%s:gpio (%d) is not configed\n",__func__,gpio);
		ret =  false;
		goto exit;
	}

	if(!(gpio_dev->flags & ATBM_GPIO_CONFIG__OUTPUT)){
		atbm_printk_err("%s:gpio (%d) is not output mode\n",__func__,gpio);
		ret =  false;
		goto exit;
	}

	gpio_dev->out_val.val = set == true ? 1:0;
	
	if(atbm_internal_gpio_set(hw_priv,&gpio_dev->out_val)){
		atbm_printk_err("%s:gpio out(%d) is err\n",__func__,gpio);
		ret =  false;
		goto exit;
	}
exit:
	return ret;
}

bool atbm_internal_gpio_input(struct atbm_common *hw_priv,int gpio,bool *set)
{
	struct atbm_gpio_config *gpio_dev = NULL;
	bool ret =true;
	
	gpio_dev = atbm_internal_gpio_reqest(hw_priv,gpio);

	if(gpio_dev == NULL){
		atbm_printk_err("%s:gpio (%d) is err\n",__func__,gpio);
		ret =  false;
		goto exit;
	}

	if(!(gpio_dev->flags & ATBM_GPIO_CONFIG__FUNCTION_CONFIGD)){
		atbm_printk_err("%s:gpio (%d) is not configed\n",__func__,gpio);
		ret =  false;
		goto exit;
	}

	if(!(gpio_dev->flags & ATBM_GPIO_CONFIG__INPUT)){
		atbm_printk_err("%s:gpio (%d) is not input mode\n",__func__,gpio);
		ret =  false;
		goto exit;
	}
	
	if(atbm_internal_gpio_get(hw_priv,&gpio_dev->in_val)){
		atbm_printk_err("%s:gpio out(%d) is err\n",__func__,gpio);
		ret =  false;
		goto exit;
	}

	*set = gpio_dev->in_val.val ? true:false;
exit:
	return ret;
}
/*
WSM_EDCA_SET(&priv->edca, queue, params->aifs,
                                params->cw_min, params->cw_max, params->txop, 0xc8,
                                params->uapsd);

*/
bool atbm_internal_edca_update(struct ieee80211_sub_if_data *sdata,int queue,int aifs,int cw_win,int cw_max,int txop)
{
	bool ret = false;
	struct atbm_vif *priv = (struct atbm_vif *)sdata->vif.drv_priv;
	
	if(!ieee80211_sdata_running(sdata)){
		atbm_printk_scan("%s:%d\n",__func__,__LINE__);
		goto exit;
	}

	if(atomic_read(&priv->enabled) == 0){
		atbm_printk_err("%s:disabled\n",__func__);
		goto exit;
	}

	WSM_EDCA_SET(&priv->edca, queue, aifs,
                 cw_win, cw_max, txop, 0xc8,
                 priv->edca.params[queue].uapsdEnable);
	ret = wsm_set_edca_params(priv->hw_priv, &priv->edca, priv->if_id);
	if (ret) {
		atbm_printk_err("%s:wsm_set_edca_params\n",__func__);
		goto exit;
	}

	ret = true;
exit:
	
	return ret;
}

struct atbm_vendor_cfg_ie private_ie;


int atbm_internal_recv_6441_vendor_ie(struct atbm_vendor_cfg_ie *recv_ie)
{
	
//	if(recv_ie){
	//	if(memcmp(recv_ie,&private_ie,sizeof(struct atbm_vendor_cfg_ie))){
			memcpy(&private_ie,recv_ie,sizeof(struct atbm_vendor_cfg_ie));
			return 0;
	//	}
//	}
//	return -1;
}
struct atbm_vendor_cfg_ie * atbm_internal_get_6441_vendor_ie(void)
{
	struct atbm_vendor_cfg_ie ie;
	memset(&ie,0,sizeof(struct atbm_vendor_cfg_ie));
	if(memcmp(&ie,&private_ie,sizeof(struct atbm_vendor_cfg_ie)) == 0)
		return NULL;

	return &private_ie;


}


#include "country_code.h"
int country_chan_found(const char *country)
{
	int i = 0,chan = 0;
	struct country_chan{
		  	char *country;
		  	u8 chan;
	 };
	struct country_chan country_t[]={
		  {"CN",13},{"EU",13},
		  {"JP",14},{"00",14},{"01",14},
		  {"US",11},{"CA",11},{"CO",11},{"DO",11},{"GT",11},
		  {"MX",11},{"PA",11},{"PT",11},{"TW",11},{"UZ",11},
		  {NULL,0}
	};

	do{
		if(memcmp(country,country_t[i].country,2) == 0){
			chan = country_t[i].chan;
			break;
		}
		i++;
	}while(country_t[i].country);

	if(country_t[i].country == NULL){
		return 14;
	}

	return chan;

}

#ifdef CONFIG_CPTCFG_CFG80211_COUNTRY_CODE


int atbm_set_country_code_on_driver(struct ieee80211_local *local,char *country)
{

	int i = 0,found = 0;
	if(!local || !country){
		atbm_printk_err("%s %d : %s,%s\n",__func__,__LINE__,local==NULL?"local is NULL":" ",country?" ":"country is NULL");
		return -1;
	}
	//country_code = atbm_country_code;
	for(i = 0;memcmp(atbm_country_code[i],"00",2)!=0;i++){
		if(memcmp(country,atbm_country_code[i],2) == 0){
			found = 1;
			break;	
		}
	}

	if(found == 0){
#if 0
		if(memcmp(country,"00",2) == 0){
			atbm_printk_err("country code is 00 , The original country code is [%c%c] "
							",support channel[%d] is not change !!!!",
				local->country_code[0],local->country_code[1],local->country_support_chan);
		}
#endif
		atbm_printk_err("unknow country code (%c%c) \n",country[0],country[1]);
		return 0;
	}
	
	if(regulatory_hint(local->hw.wiphy,country) != 0){
		atbm_printk_err("not set country code to cfg80211\n");
		return -1;
	}
	
	memcpy(local->country_code,country,2);
	local->country_support_chan = country_chan_found(country);
	return 0;
}
#else
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(4, 9, 84)) 
#include <net/cfg80211.h>
#endif
int atbm_set_country_code_on_driver(struct ieee80211_local *local,char *country)
{

	if(!local || !country){
		atbm_printk_err("%s %d : %s,%s\n",__func__,__LINE__,local==NULL?"local is NULL":" ",country?" ":"country is NULL");
		return -1;
	}

	//memcpy(country_code,&msg->externData[0],2);
//	atbm_printk_err("atbm_dev_set_country_code:country_code = %c%c---------------\n",country[0],country[1]);

		
	local->country_support_chan = country_chan_found(country);
	memcpy(local->country_code,country,2);
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(5, 4, 0)) 
//	struct cfg80211_registered_device *rdev = 
//		container_of(local->hw.wiphy, struct cfg80211_registered_device,wiphy);
	//wiphy_to_rdev(local->hw.wiphy);
//	struct cfg80211_internal_bss *bss, *tmp;

//	list_for_each_entry_safe(bss, tmp, &rdev->bss_list, list) {
//
//		cfg80211_unlink_bss(rdev, bss);
		
//	}


#else
	cfg80211_bss_flush(local->hw.wiphy);

#endif
	
	return 0;
}



#endif

unsigned int atbm_get_rate_from_rateid(unsigned char maxrate_id,unsigned char dcm_used,unsigned char support_40M_symbol)
{
	unsigned int rate_val = 0;
	switch(maxrate_id)
	{
				case WSM_TRANSMIT_RATE_1:
					rate_val = 10;
				break;
				case WSM_TRANSMIT_RATE_2:
					rate_val = 20;
				break;
				case WSM_TRANSMIT_RATE_5:
					rate_val = 55;
				break;
				case WSM_TRANSMIT_RATE_11:
					rate_val = 110;
				break;
				case WSM_TRANSMIT_RATE_6:
					rate_val = 60;
				break;
				case WSM_TRANSMIT_RATE_9:
					rate_val = 90;
				break;
				case WSM_TRANSMIT_RATE_12:
					rate_val = 120;
				break;
				case WSM_TRANSMIT_RATE_18:
					rate_val = 180;
				break;
				case WSM_TRANSMIT_RATE_24:
					rate_val = 240;
				break;
				case WSM_TRANSMIT_RATE_36:
					rate_val = 260;
				break;
				case WSM_TRANSMIT_RATE_48:
					rate_val = 480;
				break;
				case WSM_TRANSMIT_RATE_54:
					rate_val = 540;
				break;
				case WSM_TRANSMIT_RATE_HT_6:
					rate_val = 65;
				break;
				case WSM_TRANSMIT_RATE_HT_13:
					rate_val = 130;
				break;
				case WSM_TRANSMIT_RATE_HT_19:
					rate_val = 195;
				break;
				case WSM_TRANSMIT_RATE_HT_26:
					rate_val = 260;
				break;
				case WSM_TRANSMIT_RATE_HT_39:
					rate_val = 390;
				break;
				case WSM_TRANSMIT_RATE_HT_52:
					rate_val = 520;
				break;
				case WSM_TRANSMIT_RATE_HT_58:
					rate_val = 580;
				break;
				case WSM_TRANSMIT_RATE_HT_65:
					rate_val = 650;
				break;
				 case WSM_TRANSMIT_RATE_HE_MCSER0:
					if(dcm_used){
						rate_val = 35;
					}else{
						rate_val = 70;
					}
					break;
				case WSM_TRANSMIT_RATE_HE_MCSER1:
					if(dcm_used){
						rate_val = 70;
					}else
					{
						rate_val = 140;
					}
					break;
				case WSM_TRANSMIT_RATE_HE_MCSER2:
					if(dcm_used){
						rate_val = 110;
				   } else
					{
						rate_val = 220;
					}		   
					break;
				case WSM_TRANSMIT_RATE_HE_MCS0: 			  
					rate_val = 75;
					break;
				case WSM_TRANSMIT_RATE_HE_MCS1:
					rate_val = 145;
					break;
				case WSM_TRANSMIT_RATE_HE_MCS2:
					rate_val = 220;
					break; 
				case WSM_TRANSMIT_RATE_HE_MCS3:
					rate_val = 340;
					break;
				case WSM_TRANSMIT_RATE_HE_MCS4:
					rate_val = 515;
					break;
				case WSM_TRANSMIT_RATE_HE_MCS5:
					rate_val = 690;
					break;
				case WSM_TRANSMIT_RATE_HE_MCS6:
					rate_val = 770;
					break;
				case WSM_TRANSMIT_RATE_HE_MCS7:
					rate_val = 860;
					break;
				case WSM_TRANSMIT_RATE_HE_MCS8:
					rate_val = 1030;
					break;
				case WSM_TRANSMIT_RATE_HE_MCS9:
					rate_val = 1140;
					break;
				case WSM_TRANSMIT_RATE_HE_MCS10:
					rate_val = 1290;
					break;
				case WSM_TRANSMIT_RATE_HE_MCS11:
					rate_val = 1430;
					break; 
				
				default:
					rate_val = 0;
					atbm_printk_err("maxrate_id:%d,invalid rate!\n",maxrate_id);
					break;
	}
	if(support_40M_symbol == 1)
	{
		rate_val = rate_val * 2;
	}
	
    rate_val = rate_val *50000;
	return rate_val;
	
}

unsigned int HW_READ_REG(unsigned int addr)
{
#ifndef SPI_BUS
	unsigned int regdata=0;
	
	atbm_direct_read_reg_32(atbm_hw_priv_dereference(), addr, &regdata);
	
	return regdata;
#else
	return 0;
#endif	
}

void HW_WRITE_REG(unsigned int addr, unsigned int data)
{
#ifndef SPI_BUS	
	atbm_direct_write_reg_32(atbm_hw_priv_dereference(), addr, data);	
#else
	return;
#endif	
}

unsigned int HW_READ_REG_BIT(unsigned int addr,int endbit,int startbit)
{	
	unsigned int regdata=0;
	unsigned int regmask=0;
	

	regdata = HW_READ_REG(addr);

	regmask = ~((1<<startbit) -1);
	regmask &= ((1<<endbit) -1)|(1<<endbit);
	regdata &= regmask;
	regdata >>=  startbit;
	return regdata;
}
void HW_WRITE_REG_BIT(unsigned int addr,unsigned int endBit,unsigned int startBit,unsigned int data )
{
	unsigned int	uiRegValue=0;
	unsigned int  regmask=0;

	uiRegValue=HW_READ_REG(addr);		
	regmask = ~((1<<startBit) -1);
	regmask &= ((1<<endBit) -1)|(1<<endBit);
	uiRegValue &= ~regmask;
	uiRegValue |= (data <<startBit)&regmask;
	HW_WRITE_REG(addr,uiRegValue);
}


//void HW_WRITE_REG_BIT(u32 addr,u8 endBit,u8 startBit,u32 data )
//void ATBMPhyRegBitsSet(unsigned int  u32address, unsigned int u32Value,unsigned char bitLen, unsigned char bitOffset)
#define ATBMPhyRegBitsSet(addr, data, bitLen, bitOffset)   (HW_WRITE_REG_BIT((addr),  (bitOffset) + (bitLen) - 1, (bitOffset), (data)))
//uint32 HW_READ_REG_BIT(uint32 addr,int endbit,int startbit)
//UINT32 HW_READ_BIT(uint32 addr,int startbit,int endbit)
#define HW_READ_BIT(addr, startbit, endbit)		(HW_READ_REG_BIT((addr), (endbit), (startbit)))
#define ATBMPhyRegI2CWrite(addr, data)			(HW_WRITE_REG((addr), (data)))
//void HW_WRITE_REG_BIT(u32 addr,u8 endBit,u8 startBit,u32 data )
//void HW_WRITE_BIT(uint32 addr,uint32 data,int startbit,int endbit)
#define HW_WRITE_BIT(addr, data, startbit, endbit)   (HW_WRITE_REG_BIT((addr), (endbit), (startbit), (data)))

//get current paramters from registers
void ETF_PHY_Cont_Tx_Param_Get(ContTxParam_t *pContTxParam)
{
    if(NULL == pContTxParam)
    {
        return;
    }
    
    pContTxParam->nt_contf.Reg 	= HW_READ_REG(WIFI_AX_REG_NT_CONFIG);
    pContTxParam->tx_rate 	  	= HW_READ_REG(WIFI_AX_REG_TX_RATE);
    pContTxParam->tx_length 	= HW_READ_REG(WIFI_AX_REG_TX_LEN);
    pContTxParam->TxVector0.Reg = HW_READ_REG(WIFI_AX_REG_TX_VECTOR0);
    pContTxParam->TxVector1.Reg = HW_READ_REG(WIFI_AX_REG_TX_VECTOR1);
    pContTxParam->TxVector2.Reg = HW_READ_REG(WIFI_AX_REG_TX_VECTOR2);
    pContTxParam->TxVector3.Reg = HW_READ_REG(WIFI_AX_REG_TX_VECTOR3);
    pContTxParam->TxVector4.Reg = HW_READ_REG(WIFI_AX_REG_TX_VECTOR4);
    pContTxParam->TxVector5.Reg = HW_READ_REG(WIFI_AX_REG_TX_VECTOR5);
    //ATBMPhyRegI2CRead(WIFI_AX_REG_TX_MPDU_NUM, &pContTxParam->MPDUNum);
    pContTxParam->TxIfsTime 	= HW_READ_REG(WIFI_AX_REG_TX_IFS_TIME);
    pContTxParam->ScrambleSeed	= HW_READ_REG(WIFI_AX_REG_SCRAMBLE_SEED);
    
}
//set all paramters except "pContTxParam->nt_contf"
void ETF_PHY_Cont_Tx_Param_Set(ContTxParam_t *pContTxParam)
{
	atbm_printk_always("tx_rate:%d\n", pContTxParam->tx_rate);
	atbm_printk_always("tx_length:%d\n", pContTxParam->tx_length);
	
	atbm_printk_always("TxVector0:0x%x\n", pContTxParam->TxVector0.Reg);
	atbm_printk_always("TxVector1:0x%x\n", pContTxParam->TxVector1.Reg);
	atbm_printk_always("TxVector2:0x%x\n", pContTxParam->TxVector2.Reg);
	atbm_printk_always("TxVector3:0x%x\n", pContTxParam->TxVector3.Reg);
	atbm_printk_always("TxVector4:0x%x\n", pContTxParam->TxVector4.Reg);
	atbm_printk_always("TxVector5:0x%x\n", pContTxParam->TxVector5.Reg);

	atbm_printk_always("TxIfsTime:%d\n", pContTxParam->TxIfsTime);
	atbm_printk_always("ScrambleSeed:%d\n", pContTxParam->ScrambleSeed);


	
    HW_WRITE_REG(WIFI_AX_REG_TX_RATE, 	 (unsigned int )pContTxParam->tx_rate);
    HW_WRITE_REG(WIFI_AX_REG_TX_LEN, 	 pContTxParam->tx_length);
    HW_WRITE_REG(WIFI_AX_REG_TX_VECTOR0, pContTxParam->TxVector0.Reg);
    HW_WRITE_REG(WIFI_AX_REG_TX_VECTOR1, pContTxParam->TxVector1.Reg);
    HW_WRITE_REG(WIFI_AX_REG_TX_VECTOR2, pContTxParam->TxVector2.Reg);
    HW_WRITE_REG(WIFI_AX_REG_TX_VECTOR3, pContTxParam->TxVector3.Reg);
    HW_WRITE_REG(WIFI_AX_REG_TX_VECTOR4, pContTxParam->TxVector4.Reg);
    HW_WRITE_REG(WIFI_AX_REG_TX_VECTOR5, pContTxParam->TxVector5.Reg);
    HW_WRITE_REG(WIFI_AX_REG_TX_IFS_TIME, pContTxParam->TxIfsTime);
    HW_WRITE_REG(WIFI_AX_REG_SCRAMBLE_SEED, pContTxParam->ScrambleSeed);
}

void ETF_PHY_Cont_Tx_Enable(ContTxParam_t *pContTxParam,bool TxEnable)
{
    NtContf_t NtContf = {{0}};
    if(NULL == pContTxParam)
    {
        NtContf.Reg = HW_READ_REG(WIFI_AX_REG_NT_CONFIG);
    }
    else
    {
        NtContf = pContTxParam->nt_contf;
    }
    if(TxEnable)
    {
        NtContf.Bits.CONT = 1;
    }
    else
    {
        NtContf.Bits.CONT = 0;
    }
    HW_WRITE_REG(WIFI_AX_REG_NT_CONFIG, NtContf.Reg);
}
//start tx (set only "pContTxParam->nt_contf")
void ETF_PHY_Cont_Tx_Start(ContTxParam_t *pContTxParam)
{
    ETF_PHY_Cont_Tx_Enable(pContTxParam,true);
}

//stop tx (set only "pContTxParam->nt_contf")
void ETF_PHY_Cont_Tx_Stop(ContTxParam_t *pContTxParam)
{
    ETF_PHY_Cont_Tx_Enable(pContTxParam,false);
}

static WLAN_RATE_T ATBMPhyRateMapping(ETF_PHY_TX_PARAM_T *pTxParam)
{
    WLAN_RATE_T eRate = _6Mbps_BPSK_Code1_2_;
	pTxParam->DataRateMbps = 0;
    if(ATBM_WIFI_MODE_DSSS == pTxParam->WiFiMode)
    {
        switch(pTxParam->Rate) //int type, the value differentiated by WiFiMode(DSS,LM,MM,GF)
        {
            case ATBM_WIFI_RATE_1M:
                eRate = _1Mbps_DSSS_;
				pTxParam->DataRateMbps = 1;
                break;
            case ATBM_WIFI_RATE_2M:
                eRate = _2Mbps_DSSS_;
				pTxParam->DataRateMbps = 2;
                break;
            case ATBM_WIFI_RATE_5D5M:
                eRate = _5_5Mbps_CCK_;
				pTxParam->DataRateMbps = 5.5;
                break;
            case ATBM_WIFI_RATE_11M:
                eRate = _11Mbps_CCK_;
				pTxParam->DataRateMbps = 11;
                break;
            default:
                eRate = _1Mbps_DSSS_;
				pTxParam->DataRateMbps = 1;
                break;
        }
    }
    else if((ATBM_WIFI_MODE_OFDM == pTxParam->WiFiMode)&&(ATBM_WIFI_OFDM_MD_LM == pTxParam->OFDMMode)) //legacy mode
    {
        switch(pTxParam->Rate)
        {
            case ATBM_WIFI_RATE_6M:
                eRate = _6Mbps_BPSK_Code1_2_;
				pTxParam->DataRateMbps = 6;
                break;
            case ATBM_WIFI_RATE_9M:
                eRate = _9Mbps_BPSK_Code3_4_;
				pTxParam->DataRateMbps = 9;
                break;
            case ATBM_WIFI_RATE_12M:
                eRate = _12Mbps_QPSK_Code1_2_;
				pTxParam->DataRateMbps = 12;
                break;
            case ATBM_WIFI_RATE_18M:
                eRate = _18Mbps_QPSK_Code3_4_;
				pTxParam->DataRateMbps = 18;
                break;
            case ATBM_WIFI_RATE_24M:
                eRate = _24Mbps_16QAM_Code1_2_;
				pTxParam->DataRateMbps = 24;
                break;
            case ATBM_WIFI_RATE_36M:
                eRate = _36Mbps_16QAM_Code3_4_;
				pTxParam->DataRateMbps = 36;
				break;
            case ATBM_WIFI_RATE_48M:
                eRate = _48Mbps_64QAM_Code2_3_;
				pTxParam->DataRateMbps = 48;
				break;
            case ATBM_WIFI_RATE_54M:
                eRate = _54Mbps_64QAM_Code3_4_;
				pTxParam->DataRateMbps = 54;
				break;
            default:
                eRate = _6Mbps_BPSK_Code1_2_;
				pTxParam->DataRateMbps = 6;
                break;
        }
    }
    else if((ATBM_WIFI_MODE_OFDM == pTxParam->WiFiMode)  
            &&((ATBM_WIFI_OFDM_MD_MM == pTxParam->OFDMMode)||(ATBM_WIFI_OFDM_MD_GF == pTxParam->OFDMMode))) //MM,GF mode
    {
	
		if(pTxParam->BW == ATBM_WIFI_BW_40M)
		{
			if(pTxParam->GIMode == ATBM_WIFI_GI_MD_NORMAL)
			{
				switch(pTxParam->Rate)
				{
				case ATBM_WIFI_RATE_MCS0:
					eRate = _6_5Mbps_BPSK_Code_1_2_;
					pTxParam->DataRateMbps = 13.5;
					break;
				case ATBM_WIFI_RATE_MCS1:
					eRate = _13_5Mbps_QPSK_Code_1_2_;
					pTxParam->DataRateMbps = 27;
					break;
				case ATBM_WIFI_RATE_MCS2:
					eRate = _19_5Mbps_QPSK_Code_3_4_;
					pTxParam->DataRateMbps = 40.5;
					break;
				case ATBM_WIFI_RATE_MCS3:
					eRate = _26_Mbps_16QAM_Code_1_2_;
					pTxParam->DataRateMbps = 54;
					break;
				case ATBM_WIFI_RATE_MCS4:
					eRate = _39_Mbps_16QAM_Code_3_4_;
					pTxParam->DataRateMbps = 81;
					break;
				case ATBM_WIFI_RATE_MCS5:
					eRate = _52_Mbps_64QAM_Code_2_3_;
					pTxParam->DataRateMbps = 108;
					break;
				case ATBM_WIFI_RATE_MCS6:
					eRate = _58_5Mbps_64QAM_Code_3_4_;
					pTxParam->DataRateMbps = 121.5;
					break;
				case ATBM_WIFI_RATE_MCS7:
					eRate = _65_Mbps_64QAM_Code_5_6_;
					pTxParam->DataRateMbps = 135;
					break;
				case ATBM_WIFI_RATE_MCS32:
					eRate = _6Mbps_MCS32_BPSK_Code1_2_; /*Driver need add MCS32 */
					pTxParam->DataRateMbps = 6;
					break;
				default:
					eRate = _6_5Mbps_BPSK_Code_1_2_;
					pTxParam->DataRateMbps =13.5;
					break;
				}
			}
			if(pTxParam->GIMode == ATBM_WIFI_GI_MD_SHORT)
			{
		/*
		0 BPSK 1/2 1 108 6 108 54   13.5 15.0
		1 QPSK 1/2 2 108 6 216 108	27.0 30.0
		2 QPSK 3/4 2 108 6 216 162	40.5 45.0
		3 16-QAM 1/2 4 108 6 432 216 54.0 60.0
		4 16-QAM 3/4 4 108 6 432 324 81.0 90.0
		5 64-QAM 2/3 6 108 6 648 432 108.0 120.0
		6 64-QAM 3/4 6 108 6 648 486 121.5 135.0
		7 64-QAM 5/6 6 108 6 648 540 135.0 150.0
		*/
				switch(pTxParam->Rate)
				{
				case ATBM_WIFI_RATE_MCS0:
					eRate = _6_5Mbps_BPSK_Code_1_2_;
					pTxParam->DataRateMbps = 15;
					break;
				case ATBM_WIFI_RATE_MCS1:
					eRate = _13_5Mbps_QPSK_Code_1_2_;
					pTxParam->DataRateMbps = 30;
					break;
				case ATBM_WIFI_RATE_MCS2:
					eRate = _19_5Mbps_QPSK_Code_3_4_;
					pTxParam->DataRateMbps = 45;
					break;
				case ATBM_WIFI_RATE_MCS3:
					eRate = _26_Mbps_16QAM_Code_1_2_;
					pTxParam->DataRateMbps = 60;
					break;
				case ATBM_WIFI_RATE_MCS4:
					eRate = _39_Mbps_16QAM_Code_3_4_;
					pTxParam->DataRateMbps = 90;
					break;
				case ATBM_WIFI_RATE_MCS5:
					eRate = _52_Mbps_64QAM_Code_2_3_;
					pTxParam->DataRateMbps = 120;
					break;
				case ATBM_WIFI_RATE_MCS6:
					eRate = _58_5Mbps_64QAM_Code_3_4_;
					pTxParam->DataRateMbps =135;
					break;
				case ATBM_WIFI_RATE_MCS7:
					eRate = _65_Mbps_64QAM_Code_5_6_;
					pTxParam->DataRateMbps = 150;
					break;
				case ATBM_WIFI_RATE_MCS32:
					eRate = _6Mbps_MCS32_BPSK_Code1_2_; /*Driver need add MCS32 */
					pTxParam->DataRateMbps = 6.7;
					break;
				default:
					eRate = _6_5Mbps_BPSK_Code_1_2_;
					pTxParam->DataRateMbps = 15;
					break;
				}
			} //short GI


		}
		else
		{
			if(pTxParam->GIMode == ATBM_WIFI_GI_MD_NORMAL)
			{
				switch(pTxParam->Rate)
				{
				case ATBM_WIFI_RATE_MCS0:
					eRate = _6_5Mbps_BPSK_Code_1_2_;
					pTxParam->DataRateMbps = 6.5;
					break;
				case ATBM_WIFI_RATE_MCS1:
					eRate = _13_5Mbps_QPSK_Code_1_2_;
					pTxParam->DataRateMbps = 13.5;
					break;
				case ATBM_WIFI_RATE_MCS2:
					eRate = _19_5Mbps_QPSK_Code_3_4_;
					pTxParam->DataRateMbps = 19.5;
					break;
				case ATBM_WIFI_RATE_MCS3:
					eRate = _26_Mbps_16QAM_Code_1_2_;
					pTxParam->DataRateMbps = 26;
					break;
				case ATBM_WIFI_RATE_MCS4:
					eRate = _39_Mbps_16QAM_Code_3_4_;
					pTxParam->DataRateMbps = 39;
					break;
				case ATBM_WIFI_RATE_MCS5:
					eRate = _52_Mbps_64QAM_Code_2_3_;
					pTxParam->DataRateMbps = 52;
					break;
				case ATBM_WIFI_RATE_MCS6:
					eRate = _58_5Mbps_64QAM_Code_3_4_;
					pTxParam->DataRateMbps = 58.5;
					break;
				case ATBM_WIFI_RATE_MCS7:
					eRate = _65_Mbps_64QAM_Code_5_6_;
					pTxParam->DataRateMbps = 65;
					break;
				case ATBM_WIFI_RATE_MCS32:
					eRate = _6Mbps_MCS32_BPSK_Code1_2_; /*Driver need add MCS32 */
					pTxParam->DataRateMbps = 6;
					break;
				default:
					eRate = _6_5Mbps_BPSK_Code_1_2_;
					pTxParam->DataRateMbps = 6.5;
					break;
				}
			}
			if(pTxParam->GIMode == ATBM_WIFI_GI_MD_SHORT)
			{
				switch(pTxParam->Rate)
				{
				case ATBM_WIFI_RATE_MCS0:
					eRate = _6_5Mbps_BPSK_Code_1_2_;
					pTxParam->DataRateMbps = 7.2;
					break;
				case ATBM_WIFI_RATE_MCS1:
					eRate = _13_5Mbps_QPSK_Code_1_2_;
					pTxParam->DataRateMbps = 14.4;
					break;
				case ATBM_WIFI_RATE_MCS2:
					eRate = _19_5Mbps_QPSK_Code_3_4_;
					pTxParam->DataRateMbps = 21.7;
					break;
				case ATBM_WIFI_RATE_MCS3:
					eRate = _26_Mbps_16QAM_Code_1_2_;
					pTxParam->DataRateMbps = 28.9;
					break;
				case ATBM_WIFI_RATE_MCS4:
					eRate = _39_Mbps_16QAM_Code_3_4_;
					pTxParam->DataRateMbps = 43.3;
					break;
				case ATBM_WIFI_RATE_MCS5:
					eRate = _52_Mbps_64QAM_Code_2_3_;
					pTxParam->DataRateMbps = 57.8;
					break;
				case ATBM_WIFI_RATE_MCS6:
					eRate = _58_5Mbps_64QAM_Code_3_4_;
					pTxParam->DataRateMbps =65;
					break;
				case ATBM_WIFI_RATE_MCS7:
					eRate = _65_Mbps_64QAM_Code_5_6_;
					pTxParam->DataRateMbps = 72.2;
					break;
				case ATBM_WIFI_RATE_MCS32:
					eRate = _6Mbps_MCS32_BPSK_Code1_2_; /*Driver need add MCS32 */
					pTxParam->DataRateMbps = 6.7;
					break;
				default:
					eRate = _6_5Mbps_BPSK_Code_1_2_;
					pTxParam->DataRateMbps = 6.5;
					break;
				}
			} //short GI
		}   //20M
	}
    else if((ATBM_WIFI_MODE_OFDM == pTxParam->WiFiMode) && OFDM_MD_IS_HE(pTxParam->OFDMMode))
    {
        switch(pTxParam->Rate)
        {
        case ATBM_WIFI_RATE_MCS0:
            eRate = _6_5Mbps_BPSK_Code_1_2_;
            pTxParam->DataRateMbps = 6.5;
            break;
        case ATBM_WIFI_RATE_MCS1:
            eRate = _13_5Mbps_QPSK_Code_1_2_;
            pTxParam->DataRateMbps = 13.5;
            break;
        case ATBM_WIFI_RATE_MCS2:
            eRate = _19_5Mbps_QPSK_Code_3_4_;
            pTxParam->DataRateMbps = 19.5;
            break;
        case ATBM_WIFI_RATE_MCS3:
            eRate = _26_Mbps_16QAM_Code_1_2_;
            pTxParam->DataRateMbps = 26;
            break;
        case ATBM_WIFI_RATE_MCS4:
            eRate = _39_Mbps_16QAM_Code_3_4_;
            pTxParam->DataRateMbps = 39;
            break;
        case ATBM_WIFI_RATE_MCS5:
            eRate = _52_Mbps_64QAM_Code_2_3_;
            pTxParam->DataRateMbps = 52;
            break;
        case ATBM_WIFI_RATE_MCS6:
            eRate = _58_5Mbps_64QAM_Code_3_4_;
            pTxParam->DataRateMbps = 58.5;
            break;
        case ATBM_WIFI_RATE_MCS7:
            eRate = _65_Mbps_64QAM_Code_5_6_;
            pTxParam->DataRateMbps = 65;
            break;

        case ATBM_WIFI_RATE_MCS8:
            eRate = _xx_Mbps_256QAM_Code_3_4_;
            pTxParam->DataRateMbps = 65; //need check 
            break;
        case ATBM_WIFI_RATE_MCS9:
            eRate = _xx_Mbps_256QAM_Code_5_6_;
            pTxParam->DataRateMbps = 65; //need check 
            break;
        case ATBM_WIFI_RATE_MCS10:
            eRate = _xx_Mbps_1024QAM_Code_3_4_;
            pTxParam->DataRateMbps = 65; //need check 
            break;
        case ATBM_WIFI_RATE_MCS11:
            eRate = _xx_Mbps_1024QAM_Code_5_6_;
            pTxParam->DataRateMbps = 65; //need check 
            break;

        default:
            eRate = _6_5Mbps_BPSK_Code_1_2_;
            pTxParam->DataRateMbps = 6.5;
            break;
        }
    }
    return eRate;
}

/*
    1001: 802.11ax HE MU PPDU
    1000: 802.11ax HE TB PPDU
    0111: 802.11ax HE ER SU PPDU
    0110: 802.11ax HE SU PPDU
    0101: 802.11n OFDM long (mixed-mode) preamble
    0100: 802.11n OFDM short (greenfield-mode) preamble
    0010: 802.11a OFDM preamble
    0001: 802.11b DSSS long preamble
    0000: 802.11b DSSS short preamble (note this is not supported for 1MBit/s)

*/
unsigned int TxVector1TxModeGet(ATBMWiFiMode_e WiFiMode,ATBMWiFiOFDMMode_e OFDMMode,ATBMWiFiPreamble_e PreambleMode)
{
    unsigned int Vector1TxMode = 0;
    if(ATBM_WIFI_MODE_DSSS == WiFiMode)
    {
        if(ATBM_WIFI_PREAMBLE_SHORT == PreambleMode)
        {
            Vector1TxMode = 0x00;
        }
        else
        {
            Vector1TxMode = 0x01;
        }
    }
    else
    {//OFDM
        switch(OFDMMode)
        {
            case ATBM_WIFI_OFDM_MD_LM:
                Vector1TxMode = 0x02;
                break;
            case ATBM_WIFI_OFDM_MD_GF:
                Vector1TxMode = 0x04;
                break;
            case ATBM_WIFI_OFDM_MD_MM:
                Vector1TxMode = 0x05;
                break;
            case ATBM_WIFI_OFDM_MD_HE_SU:
                Vector1TxMode = 0x06;
                break;
            case ATBM_WIFI_OFDM_MD_HE_ER_SU:
                Vector1TxMode = 0x07;
                break;
            case ATBM_WIFI_OFDM_MD_HE_TB:
                Vector1TxMode = 0x08;
                break;
            case ATBM_WIFI_OFDM_MD_HE_MU:
                Vector1TxMode = 0x09;
                break;
            default:
                Vector1TxMode = 0x06;
                break;
        }
    }
    return Vector1TxMode;
}

/*
0: 1x HE-LTF
1: 2x HE-LTF
2: 4x HE-LTF
*/
unsigned int TxVector4LtfTypeGet(ETF_PHY_TX_PARAM_T *pTxParam)
{
    unsigned int LtfType = 0;
    if((ATBM_WIFI_MODE_OFDM == pTxParam->WiFiMode)&&OFDM_MD_IS_HE(pTxParam->OFDMMode))
    {//OFDM
        switch(pTxParam->GIMode)
        {
            case ATBM_WIFI_GILTF_0P8_1X:
            case ATBM_WIFI_GILTF_1P6_1X:
                LtfType = 0x00;
                break;
            case ATBM_WIFI_GILTF_0P8_2X:
            case ATBM_WIFI_GILTF_1P6_2X:
                LtfType = 0x01;
                break;
            case ATBM_WIFI_GILTF_0P8_4X:
            case ATBM_WIFI_GILTF_3P2_4X:
                LtfType = 0x02;
                break;
            default:
                LtfType = 0x00;
                break;
        }
    }
    return LtfType;
}

/*
0: 0.4us.  not supported in HE PPDU.
1: 0.8us
2: 1.6us 
3: 3.2us
*/
unsigned int TxVector4GiTypeGet(ETF_PHY_TX_PARAM_T *pTxParam)
{
    unsigned int GiType = 0;
    if(ATBM_WIFI_MODE_OFDM == pTxParam->WiFiMode)
    {//OFDM
        if(OFDM_MD_IS_HE(pTxParam->OFDMMode))
        {
            switch(pTxParam->GIMode)
            {
                case ATBM_WIFI_GILTF_0P8_1X:
                case ATBM_WIFI_GILTF_0P8_2X:
                case ATBM_WIFI_GILTF_0P8_4X:
                    GiType = 0x01;
                    break;
                case ATBM_WIFI_GILTF_1P6_1X:
                case ATBM_WIFI_GILTF_1P6_2X:
                    GiType = 0x02;
                    break;
                case ATBM_WIFI_GILTF_3P2_4X:
                    GiType = 0x03;
                    break;
                default:
                    GiType = 0x00;
                    break;
            }
        }
        else if((ATBM_WIFI_OFDM_MD_MM == pTxParam->OFDMMode)||(ATBM_WIFI_OFDM_MD_GF == pTxParam->OFDMMode))
        {
            switch(pTxParam->GIMode)
            {
                case ATBM_WIFI_GI_MD_SHORT:
                    GiType = 0x00;
                    break;
                case ATBM_WIFI_GI_MD_NORMAL:
                    GiType = 0x01;
                    break;
                default:
                    GiType = 0x00;
                    break;
            }
        }
    }

    return GiType;
}

void ETF_PHY_Cont_Tx_Param_Parse(ETF_PHY_TX_PARAM_T *pTxParam,ContTxParam_t *pContTxParam)
{
    //pContTxParam->nt_contf.Bits.CONT = 1;
    pContTxParam->nt_contf.Bits.INF = (pTxParam->InfiniteLongPacket)?(1):(0);
    pContTxParam->nt_contf.Bits.IFS = pTxParam->PacketInterval*160; //old FPGA code with "*160" (from us to colock cyles)
    pContTxParam->nt_contf.Bits.NFRAMES = pTxParam->PacketNum;
    
    pContTxParam->TxIfsTime = pTxParam->PacketInterval*160;

    pContTxParam->tx_rate = ATBMPhyRateMapping(pTxParam);
    pContTxParam->tx_length = pTxParam->PSDULen;


    /* TxVector0 */
    if(ATBM_WIFI_MODE_DSSS == pTxParam->WiFiMode)
    {//802.11b
        pContTxParam->TxVector0.Bits11b.ServiceField = pTxParam->ServiceField;
    }
    else
    {
        pContTxParam->TxVector0.Bits.Smoothing = pTxParam->Smoothing;
        pContTxParam->TxVector0.Bits.Sounding = pTxParam->Sounding;
        pContTxParam->TxVector0.Bits.Aggregation = pTxParam->Aggregation;
        pContTxParam->TxVector0.Bits.STBC = pTxParam->STBC;
        pContTxParam->TxVector0.Bits.Beamformed = pTxParam->BeamFormed;
        pContTxParam->TxVector0.Bits.Doppler = pTxParam->Doppler;
        pContTxParam->TxVector0.Bits.BurstLength = pTxParam->BurstLen;
        if(pTxParam->TxopDuration>=512)
        {
            pContTxParam->TxVector0.Bits.TxopDuration = (((pTxParam->TxopDuration-512)/128)<<1)|0x01;
        }
        else
        {
            pContTxParam->TxVector0.Bits.TxopDuration = ((pTxParam->TxopDuration/8)<<1);
        }
        //pContTxParam->TxVector0.Bits.TxopDuration = pTxParam->TxopDuration;
        pContTxParam->TxVector0.Bits.NoSigExtn = pTxParam->NoSigExtn;
    }
    /*    0: 1 LTF symbols; 1: 2LTF symbols;  3: 4LTF symbols;   5: 6LTF symbols;    7: 8 LTF symbols    */
    if((ATBM_WIFI_MODE_OFDM == pTxParam->WiFiMode)&&(ATBM_WIFI_OFDM_MD_HE_TB == pTxParam->OFDMMode))
    {
        pContTxParam->TxVector0.Bits.NumOfLTF = pTxParam->LTFNum - 1;
    }
    else
    {
        pContTxParam->TxVector0.Bits.NumOfLTF = 0;
    }
    
    
    /* TxVector1 */
    pContTxParam->TxVector1.Bits.TxPower = pTxParam->TxPower;
    pContTxParam->TxVector1.Bits.TxStreams = pTxParam->TxStreams;
    pContTxParam->TxVector1.Bits.TxAntennas = pTxParam->TxAntennas;
    pContTxParam->TxVector1.Bits.TxMode = TxVector1TxModeGet(pTxParam->WiFiMode,pTxParam->OFDMMode,pTxParam->PreambleMode);
    pContTxParam->TxVector1.Bits.ChBW = ((ATBM_WIFI_BW_20M==pTxParam->BW) ||(ATBM_WIFI_BW_RU242==pTxParam->BW))?(0):(1);
    pContTxParam->TxVector1.Bits.ChOffset = pTxParam->ChOffset;
    pContTxParam->TxVector1.Bits.TxAbsPower = pTxParam->TxAbsPower;
    pContTxParam->TxVector1.Bits.TxPowerModeSel = pTxParam->TxPowerModeSel;
    pContTxParam->TxVector1.Bits.BeamChange = pTxParam->BeamChange;
    
    /* TxVector2 */
    pContTxParam->TxVector2.Bits.PEDisambiguity = pTxParam->PEDisambiguity;
    pContTxParam->TxVector2.Bits.StartingStsNum = pTxParam->StartingStsNum;
    pContTxParam->TxVector2.Bits.HELTFMode = pTxParam->HELTFMode;
    pContTxParam->TxVector2.Bits.HESigA2Reserved = pTxParam->HESigA2Reserved;
    pContTxParam->TxVector2.Bits.PreFecPaddingFactor = pTxParam->AFactor;
    pContTxParam->TxVector2.Bits.SpatialReuse1 = pTxParam->SpatialReuse1;
    pContTxParam->TxVector2.Bits.SpatialReuse2 = pTxParam->SpatialReuse2;
    pContTxParam->TxVector2.Bits.SpatialReuse3 = pTxParam->SpatialReuse3;
    pContTxParam->TxVector2.Bits.SpatialReuse4 = pTxParam->SpatialReuse4;
    
    /* TxVector3 */
    pContTxParam->TxVector3.Bits.TriggerResponding = pTxParam->TriggerResponding;
    pContTxParam->TxVector3.Bits.TriggerMethod = pTxParam->TriggerMethod;
    pContTxParam->TxVector3.Bits.NomPacketPadding = pTxParam->Padding;
    pContTxParam->TxVector3.Bits.BSSColor = pTxParam->BSSColor;
    pContTxParam->TxVector3.Bits.UplinkFlag = pTxParam->UplinkFlag;
    pContTxParam->TxVector3.Bits.ScramblerInitialvalue = pTxParam->ScramblerValue;
    pContTxParam->TxVector3.Bits.ScramblerInitialvalue_en = pTxParam->ScramblerValueEn;
    pContTxParam->TxVector3.Bits.LdpcExtraSymbol = pTxParam->LdpcExtrSysm;
    
    /* TxVector4 */
    pContTxParam->TxVector4.Bits.HELTFType = TxVector4LtfTypeGet(pTxParam);
    pContTxParam->TxVector4.Bits.MidamblePeriod = pTxParam->MidamblePeriod;
    pContTxParam->TxVector4.Bits.Dcm = pTxParam->DCM;
    pContTxParam->TxVector4.Bits.Coding = pTxParam->Coding;
    pContTxParam->TxVector4.Bits.GI_Type = TxVector4GiTypeGet(pTxParam);
    //Ru_allocation分为ru_allocation[0] �?ru_allocation[7:1] （ru_allocation_code�?    pContTxParam->TxVector4.Bits.RUAllocation = pTxParam->RuAllocation;
    pContTxParam->TxVector4.Bits.ReservedForMAC = pTxParam->ReservedForMAC;
    
    /* TxVector5 */
    pContTxParam->TxVector5.Bits.CFO = pTxParam->CFO;
    pContTxParam->TxVector5.Bits.PPM = pTxParam->PPM;

    //AMPDU: MPDU length and number
    //pContTxParam->MPDUNum = pTxParam->MPDUNum;
#if 0 //no need to set mpdu_len in new fpga
    pContTxParam->MPDULen = pTxParam->MPDULen;
#endif

    pContTxParam->ScrambleSeed = pTxParam->ScramblerValue;

	pContTxParam->precom = pTxParam->precompensation;//only channel1 20M used


}

typedef struct
{
    unsigned int Addr;
    unsigned int Val;
    unsigned int Msb;
    unsigned int Lsb;
}RegSetting_t;
static RegSetting_t NTPCOMPRegsBW40M[]=
{
    {0x0ACA010C, 0x00, 19, 19},//params_prc_bypass

    {0x0ACA2000, 0xA6, 7, 0},
    {0x0ACA2004, 0xA6, 7, 0},
    {0x0ACA2008, 0x9A, 7, 0},
    {0x0ACA200C, 0x78, 7, 0},
    {0x0ACA2010, 0x65, 7, 0},
    {0x0ACA2014, 0x59, 7, 0},
    {0x0ACA2018, 0x53, 7, 0},
    {0x0ACA201C, 0x4F, 7, 0},
    {0x0ACA2020, 0x4E, 7, 0},
    {0x0ACA2024, 0x4D, 7, 0},
    {0x0ACA2028, 0x4D, 7, 0},
    {0x0ACA202C, 0x4C, 7, 0},
    {0x0ACA2030, 0x4A, 7, 0},
    {0x0ACA2034, 0x48, 7, 0},
    {0x0ACA2038, 0x45, 7, 0},
    {0x0ACA203C, 0x43, 7, 0},
    {0x0ACA2040, 0x41, 7, 0},
    {0x0ACA2044, 0x40, 7, 0},
    {0x0ACA2048, 0x3F, 7, 0},
    {0x0ACA204C, 0x3E, 7, 0},
    {0x0ACA2050, 0x3D, 7, 0},
    {0x0ACA2054, 0x3C, 7, 0},
    {0x0ACA2058, 0x3B, 7, 0},
    {0x0ACA205C, 0x39, 7, 0},
    {0x0ACA2060, 0x38, 7, 0},
    {0x0ACA2064, 0x37, 7, 0},
    {0x0ACA2068, 0x35, 7, 0},
    {0x0ACA206C, 0x35, 7, 0},
    {0x0ACA2070, 0x34, 7, 0},
    {0x0ACA2074, 0x34, 7, 0},
    {0x0ACA2078, 0x34, 7, 0},
    {0x0ACA207C, 0x33, 7, 0},
    {0x0ACA2080, 0x32, 7, 0},
    {0x0ACA2084, 0x31, 7, 0},
    {0x0ACA2088, 0x31, 7, 0},
    {0x0ACA208C, 0x30, 7, 0},
    {0x0ACA2090, 0x30, 7, 0},
    {0x0ACA2094, 0x30, 7, 0},
    {0x0ACA2098, 0x30, 7, 0},
    {0x0ACA209C, 0x30, 7, 0},
    {0x0ACA20A0, 0x2F, 7, 0},
    {0x0ACA20A4, 0x2F, 7, 0},
    {0x0ACA20A8, 0x2F, 7, 0},
    {0x0ACA20AC, 0x2F, 7, 0},
    {0x0ACA20B0, 0x2F, 7, 0},
    {0x0ACA20B4, 0x2F, 7, 0},
    {0x0ACA20B8, 0x2F, 7, 0},
    {0x0ACA20BC, 0x2F, 7, 0},
    {0x0ACA20C0, 0x30, 7, 0},
    {0x0ACA20C4, 0x30, 7, 0},
    {0x0ACA20C8, 0x30, 7, 0},
    {0x0ACA20CC, 0x30, 7, 0},
    {0x0ACA20D0, 0x30, 7, 0},
    {0x0ACA20D4, 0x30, 7, 0},
    {0x0ACA20D8, 0x31, 7, 0},
    {0x0ACA20DC, 0x31, 7, 0},
    {0x0ACA20E0, 0x31, 7, 0},
    {0x0ACA20E4, 0x32, 7, 0},
    {0x0ACA20E8, 0x32, 7, 0},
    {0x0ACA20EC, 0x32, 7, 0},
    {0x0ACA20F0, 0x32, 7, 0},
    {0x0ACA20F4, 0x32, 7, 0},
    {0x0ACA20F8, 0x32, 7, 0},
    {0x0ACA20FC, 0x32, 7, 0},
    {0x0ACA2100, 0x32, 7, 0},
    {0x0ACA2104, 0x32, 7, 0},
    {0x0ACA2108, 0x32, 7, 0},
    {0x0ACA210C, 0x32, 7, 0},
    {0x0ACA2110, 0x32, 7, 0},
    {0x0ACA2114, 0x32, 7, 0},
    {0x0ACA2118, 0x31, 7, 0},
    {0x0ACA211C, 0x31, 7, 0},
    {0x0ACA2120, 0x31, 7, 0},
    {0x0ACA2124, 0x30, 7, 0},
    {0x0ACA2128, 0x30, 7, 0},
    {0x0ACA212C, 0x30, 7, 0},
    {0x0ACA2130, 0x30, 7, 0},
    {0x0ACA2134, 0x30, 7, 0},
    {0x0ACA2138, 0x30, 7, 0},
    {0x0ACA213C, 0x2F, 7, 0},
    {0x0ACA2140, 0x2F, 7, 0},
    {0x0ACA2144, 0x2F, 7, 0},
    {0x0ACA2148, 0x2F, 7, 0},
    {0x0ACA214C, 0x2F, 7, 0},
    {0x0ACA2150, 0x2F, 7, 0},
    {0x0ACA2154, 0x2F, 7, 0},
    {0x0ACA2158, 0x2F, 7, 0},
    {0x0ACA215C, 0x30, 7, 0},
    {0x0ACA2160, 0x30, 7, 0},
    {0x0ACA2164, 0x30, 7, 0},
    {0x0ACA2168, 0x30, 7, 0},
    {0x0ACA216C, 0x30, 7, 0},
    {0x0ACA2170, 0x31, 7, 0},
    {0x0ACA2174, 0x31, 7, 0},
    {0x0ACA2178, 0x32, 7, 0},
    {0x0ACA217C, 0x33, 7, 0},
    {0x0ACA2180, 0x34, 7, 0},
    {0x0ACA2184, 0x34, 7, 0},
    {0x0ACA2188, 0x34, 7, 0},
    {0x0ACA218C, 0x35, 7, 0},
    {0x0ACA2190, 0x35, 7, 0},
    {0x0ACA2194, 0x37, 7, 0},
    {0x0ACA2198, 0x38, 7, 0},
    {0x0ACA219C, 0x39, 7, 0},
    {0x0ACA21A0, 0x3B, 7, 0},
    {0x0ACA21A4, 0x3C, 7, 0},
    {0x0ACA21A8, 0x3D, 7, 0},
    {0x0ACA21AC, 0x3E, 7, 0},
    {0x0ACA21B0, 0x3F, 7, 0},
    {0x0ACA21B4, 0x40, 7, 0},
    {0x0ACA21B8, 0x41, 7, 0},
    {0x0ACA21BC, 0x43, 7, 0},
    {0x0ACA21C0, 0x45, 7, 0},
    {0x0ACA21C4, 0x48, 7, 0},
    {0x0ACA21C8, 0x4A, 7, 0},
    {0x0ACA21CC, 0x4C, 7, 0},
    {0x0ACA21D0, 0x4D, 7, 0},
    {0x0ACA21D4, 0x4D, 7, 0},
    {0x0ACA21D8, 0x4E, 7, 0},
    {0x0ACA21DC, 0x4F, 7, 0},
    {0x0ACA21E0, 0x53, 7, 0},
    {0x0ACA21E4, 0x59, 7, 0},
    {0x0ACA21E8, 0x65, 7, 0},
    {0x0ACA21EC, 0x78, 7, 0},
    {0x0ACA21F0, 0x9A, 7, 0},
    {0x0ACA21F4, 0xA6, 7, 0},
    {0x0ACA21F8, 0xA6, 7, 0},
};

#define NTP_COMP_REG_BW40_NUM sizeof(NTPCOMPRegsBW40M)/sizeof(NTPCOMPRegsBW40M[0])

static RegSetting_t NTPCOMPRegsBW20M[]=
{
    {0x0ACA010C, 0x00, 19, 19},//params_prc_bypass

   	{0x0ACA21FC, 0x96, 7, 0},
    {0x0ACA2200, 0x93, 7, 0},
    {0x0ACA2204, 0x94, 7, 0},
    {0x0ACA2208, 0x8D, 7, 0},
    {0x0ACA220C, 0x8F, 7, 0},
    {0x0ACA2210, 0x8C, 7, 0},
    {0x0ACA2214, 0x87, 7, 0},
    {0x0ACA2218, 0x89, 7, 0},
    {0x0ACA221C, 0x86, 7, 0},
    {0x0ACA2220, 0x81, 7, 0},
    {0x0ACA2224, 0x87, 7, 0},
    {0x0ACA2228, 0x80, 7, 0},
    {0x0ACA222C, 0x7F, 7, 0},
    {0x0ACA2230, 0x82, 7, 0},
    {0x0ACA2234, 0x82, 7, 0},
    {0x0ACA2238, 0x7F, 7, 0},
    {0x0ACA223C, 0x7E, 7, 0},
    {0x0ACA2240, 0x81, 7, 0},
    {0x0ACA2244, 0x7D, 7, 0},
    {0x0ACA2248, 0x7E, 7, 0},
    {0x0ACA224C, 0x7F, 7, 0},
    {0x0ACA2250, 0x7E, 7, 0},
    {0x0ACA2254, 0x82, 7, 0},
    {0x0ACA2258, 0x81, 7, 0},
    {0x0ACA225C, 0x81, 7, 0},
    {0x0ACA2260, 0x7F, 7, 0},
    {0x0ACA2264, 0x7F, 7, 0},
    {0x0ACA2268, 0x81, 7, 0},
    {0x0ACA226C, 0x83, 7, 0},
    {0x0ACA2270, 0x82, 7, 0},
    {0x0ACA2274, 0x7D, 7, 0},
    {0x0ACA2278, 0x81, 7, 0},
    {0x0ACA227C, 0x85, 7, 0},
    {0x0ACA2280, 0x80, 7, 0},
    {0x0ACA2284, 0x84, 7, 0},
    {0x0ACA2288, 0x82, 7, 0},
    {0x0ACA228C, 0x81, 7, 0},
    {0x0ACA2290, 0x7E, 7, 0},
    {0x0ACA2294, 0x82, 7, 0},
    {0x0ACA2298, 0x7F, 7, 0},
    {0x0ACA229C, 0x7C, 7, 0},
    {0x0ACA22A0, 0x82, 7, 0},
    {0x0ACA22A4, 0x82, 7, 0},
    {0x0ACA22A8, 0x7F, 7, 0},
    {0x0ACA22AC, 0x80, 7, 0},
    {0x0ACA22B0, 0x80, 7, 0},
    {0x0ACA22B4, 0x81, 7, 0},
    {0x0ACA22B8, 0x80, 7, 0},
    {0x0ACA22BC, 0x80, 7, 0},
    {0x0ACA22C0, 0x82, 7, 0},
    {0x0ACA22C4, 0x85, 7, 0},
    {0x0ACA22C8, 0x82, 7, 0},
    {0x0ACA22CC, 0x83, 7, 0},
    {0x0ACA22D0, 0x86, 7, 0},
    {0x0ACA22D4, 0x82, 7, 0},
    {0x0ACA22D8, 0x8A, 7, 0},
    {0x0ACA22DC, 0x89, 7, 0},
    {0x0ACA22E0, 0x84, 7, 0},
    {0x0ACA22E4, 0x8D, 7, 0},
    {0x0ACA22E8, 0x8F, 7, 0},
    {0x0ACA22EC, 0x8C, 7, 0},
    {0x0ACA22F0, 0x92, 7, 0},
    {0x0ACA22F4, 0x9A, 7, 0},

};
//spectrum arc:attenuate 3dB at 8M 
static RegSetting_t NTPCOMPRegsBW20M_arc8M[]=
{
    {0x0ACA010C, 0x00, 19, 19},//params_prc_bypass

   	{0x0ACA21FC, 0x5B, 7, 0},
    {0x0ACA2200, 0x5D, 7, 0},
    {0x0ACA2204, 0x60, 7, 0},
    {0x0ACA2208, 0x62, 7, 0},
    {0x0ACA220C, 0x64, 7, 0},
    {0x0ACA2210, 0x66, 7, 0},
    {0x0ACA2214, 0x68, 7, 0},
    {0x0ACA2218, 0x6B, 7, 0},
    {0x0ACA221C, 0x6D, 7, 0},
    {0x0ACA2220, 0x6F, 7, 0},
    {0x0ACA2224, 0x72, 7, 0},
    {0x0ACA2228, 0x75, 7, 0},
    {0x0ACA222C, 0x77, 7, 0},
    {0x0ACA2230, 0x79, 7, 0},
    {0x0ACA2234, 0x7C, 7, 0},
    {0x0ACA2238, 0x7F, 7, 0},
    {0x0ACA223C, 0x81, 7, 0},
    {0x0ACA2240, 0x84, 7, 0},
    {0x0ACA2244, 0x86, 7, 0},
    {0x0ACA2248, 0x89, 7, 0},
    {0x0ACA224C, 0x8B, 7, 0},
    {0x0ACA2250, 0x8D, 7, 0},
    {0x0ACA2254, 0x90, 7, 0},
    {0x0ACA2258, 0x91, 7, 0},
    {0x0ACA225C, 0x93, 7, 0},
    {0x0ACA2260, 0x95, 7, 0},
    {0x0ACA2264, 0x96, 7, 0},
    {0x0ACA2268, 0x98, 7, 0},
    {0x0ACA226C, 0x98, 7, 0},
    {0x0ACA2270, 0x99, 7, 0},
    {0x0ACA2274, 0x99, 7, 0},
    {0x0ACA2278, 0x9A, 7, 0},
    {0x0ACA227C, 0x99, 7, 0},
    {0x0ACA2280, 0x9A, 7, 0},
    {0x0ACA2284, 0x98, 7, 0},
    {0x0ACA2288, 0x98, 7, 0},
    {0x0ACA228C, 0x97, 7, 0},
    {0x0ACA2290, 0x95, 7, 0},
    {0x0ACA2294, 0x94, 7, 0},
    {0x0ACA2298, 0x92, 7, 0},
    {0x0ACA229C, 0x90, 7, 0},
    {0x0ACA22A0, 0x8E, 7, 0},
    {0x0ACA22A4, 0x8B, 7, 0},
    {0x0ACA22A8, 0x89, 7, 0},
    {0x0ACA22AC, 0x87, 7, 0},
    {0x0ACA22B0, 0x84, 7, 0},
    {0x0ACA22B4, 0x82, 7, 0},
    {0x0ACA22B8, 0x7F, 7, 0},
    {0x0ACA22BC, 0x7D, 7, 0},
    {0x0ACA22C0, 0x7A, 7, 0},
    {0x0ACA22C4, 0x77, 7, 0},
    {0x0ACA22C8, 0x75, 7, 0},
    {0x0ACA22CC, 0x73, 7, 0},
    {0x0ACA22D0, 0x70, 7, 0},
    {0x0ACA22D4, 0x6D, 7, 0},
    {0x0ACA22D8, 0x6B, 7, 0},
    {0x0ACA22DC, 0x69, 7, 0},
    {0x0ACA22E0, 0x67, 7, 0},
    {0x0ACA22E4, 0x65, 7, 0},
    {0x0ACA22E8, 0x62, 7, 0},
    {0x0ACA22EC, 0x61, 7, 0},
    {0x0ACA22F0, 0x5E, 7, 0},
    {0x0ACA22F4, 0x5C, 7, 0},

};

//spectrum arc:attenuate 3dB at 9M 
static RegSetting_t NTPCOMPRegsBW20M_arc9M[]=
{
    {0x0ACA010C, 0x00, 19, 19},//params_prc_bypass

   	{0x0ACA21FC, 0x66, 7, 0},
    {0x0ACA2200, 0x67, 7, 0},
    {0x0ACA2204, 0x69, 7, 0},
    {0x0ACA2208, 0x6A, 7, 0},
    {0x0ACA220C, 0x6C, 7, 0},
    {0x0ACA2210, 0x6E, 7, 0},
    {0x0ACA2214, 0x70, 7, 0},
    {0x0ACA2218, 0x72, 7, 0},
    {0x0ACA221C, 0x74, 7, 0},
    {0x0ACA2220, 0x75, 7, 0},
    {0x0ACA2224, 0x78, 7, 0},
    {0x0ACA2228, 0x7A, 7, 0},
    {0x0ACA222C, 0x7C, 7, 0},
    {0x0ACA2230, 0x7E, 7, 0},
    {0x0ACA2234, 0x80, 7, 0},
    {0x0ACA2238, 0x83, 7, 0},
    {0x0ACA223C, 0x85, 7, 0},
    {0x0ACA2240, 0x87, 7, 0},
    {0x0ACA2244, 0x89, 7, 0},
    {0x0ACA2248, 0x8B, 7, 0},
    {0x0ACA224C, 0x8D, 7, 0},
    {0x0ACA2250, 0x8F, 7, 0},
    {0x0ACA2254, 0x91, 7, 0},
    {0x0ACA2258, 0x92, 7, 0},
    {0x0ACA225C, 0x94, 7, 0},
    {0x0ACA2260, 0x95, 7, 0},
    {0x0ACA2264, 0x97, 7, 0},
    {0x0ACA2268, 0x98, 7, 0},
    {0x0ACA226C, 0x99, 7, 0},
    {0x0ACA2270, 0x99, 7, 0},
    {0x0ACA2274, 0x99, 7, 0},
    {0x0ACA2278, 0x9A, 7, 0},
    {0x0ACA227C, 0x99, 7, 0},
    {0x0ACA2280, 0x9A, 7, 0},
    {0x0ACA2284, 0x98, 7, 0},
    {0x0ACA2288, 0x98, 7, 0},
    {0x0ACA228C, 0x97, 7, 0},
    {0x0ACA2290, 0x96, 7, 0},
    {0x0ACA2294, 0x94, 7, 0},
    {0x0ACA2298, 0x93, 7, 0},
    {0x0ACA229C, 0x91, 7, 0},
    {0x0ACA22A0, 0x8F, 7, 0},
    {0x0ACA22A4, 0x8D, 7, 0},
    {0x0ACA22A8, 0x8B, 7, 0},
    {0x0ACA22AC, 0x89, 7, 0},
    {0x0ACA22B0, 0x87, 7, 0},
    {0x0ACA22B4, 0x85, 7, 0},
    {0x0ACA22B8, 0x83, 7, 0},
    {0x0ACA22BC, 0x81, 7, 0},
    {0x0ACA22C0, 0x7E, 7, 0},
    {0x0ACA22C4, 0x7C, 7, 0},
    {0x0ACA22C8, 0x7A, 7, 0},
    {0x0ACA22CC, 0x78, 7, 0},
    {0x0ACA22D0, 0x76, 7, 0},
    {0x0ACA22D4, 0x74, 7, 0},
    {0x0ACA22D8, 0x72, 7, 0},
    {0x0ACA22DC, 0x71, 7, 0},
    {0x0ACA22E0, 0x6F, 7, 0},
    {0x0ACA22E4, 0x6D, 7, 0},
    {0x0ACA22E8, 0x6B, 7, 0},
    {0x0ACA22EC, 0x6A, 7, 0},
    {0x0ACA22F0, 0x68, 7, 0},
    {0x0ACA22F4, 0x66, 7, 0},

};
#if 0
//spectrum straight line:attenuate 3dB at 10M
static RegSetting_t NTPCOMPRegsBW20M_straightLine10M[]=
{
    {0x0ACA010C, 0x00, 19, 19},//params_prc_bypass

   	{0x0ACA21FC, 0x6D, 7, 0},
    {0x0ACA2200, 0x6D, 7, 0},
    {0x0ACA2204, 0x6E, 7, 0},
    {0x0ACA2208, 0x6E, 7, 0},
    {0x0ACA220C, 0x6E, 7, 0},
    {0x0ACA2210, 0x6F, 7, 0},
    {0x0ACA2214, 0x6F, 7, 0},
    {0x0ACA2218, 0x70, 7, 0},
    {0x0ACA221C, 0x71, 7, 0},
    {0x0ACA2220, 0x72, 7, 0},
    {0x0ACA2224, 0x73, 7, 0},
    {0x0ACA2228, 0x75, 7, 0},
    {0x0ACA222C, 0x76, 7, 0},
    {0x0ACA2230, 0x77, 7, 0},
    {0x0ACA2234, 0x79, 7, 0},
    {0x0ACA2238, 0x7B, 7, 0},
    {0x0ACA223C, 0x7D, 7, 0},
    {0x0ACA2240, 0x7E, 7, 0},
    {0x0ACA2244, 0x80, 7, 0},
    {0x0ACA2248, 0x83, 7, 0},
    {0x0ACA224C, 0x85, 7, 0},
    {0x0ACA2250, 0x86, 7, 0},
    {0x0ACA2254, 0x89, 7, 0},
    {0x0ACA2258, 0x8B, 7, 0},
    {0x0ACA225C, 0x8D, 7, 0},
    {0x0ACA2260, 0x8F, 7, 0},
    {0x0ACA2264, 0x91, 7, 0},
    {0x0ACA2268, 0x93, 7, 0},
    {0x0ACA226C, 0x95, 7, 0},
    {0x0ACA2270, 0x97, 7, 0},
    {0x0ACA2274, 0x98, 7, 0},
    {0x0ACA2278, 0x9A, 7, 0},
    {0x0ACA227C, 0x98, 7, 0},
    {0x0ACA2280, 0x97, 7, 0},
    {0x0ACA2284, 0x94, 7, 0},
    {0x0ACA2288, 0x93, 7, 0},
    {0x0ACA228C, 0x91, 7, 0},
    {0x0ACA2290, 0x8E, 7, 0},
    {0x0ACA2294, 0x8C, 7, 0},
    {0x0ACA2298, 0x8A, 7, 0},
    {0x0ACA229C, 0x88, 7, 0},
    {0x0ACA22A0, 0x86, 7, 0},
    {0x0ACA22A4, 0x84, 7, 0},
    {0x0ACA22A8, 0x82, 7, 0},
    {0x0ACA22AC, 0x80, 7, 0},
    {0x0ACA22B0, 0x7E, 7, 0},
    {0x0ACA22B4, 0x7D, 7, 0},
    {0x0ACA22B8, 0x7B, 7, 0},
    {0x0ACA22BC, 0x79, 7, 0},
    {0x0ACA22C0, 0x77, 7, 0},
    {0x0ACA22C4, 0x76, 7, 0},
    {0x0ACA22C8, 0x75, 7, 0},
    {0x0ACA22CC, 0x73, 7, 0},
    {0x0ACA22D0, 0x72, 7, 0},
    {0x0ACA22D4, 0x71, 7, 0},
    {0x0ACA22D8, 0x70, 7, 0},
    {0x0ACA22DC, 0x70, 7, 0},
    {0x0ACA22E0, 0x6F, 7, 0},
    {0x0ACA22E4, 0x6E, 7, 0},
    {0x0ACA22E8, 0x6E, 7, 0},
    {0x0ACA22EC, 0x6E, 7, 0},
    {0x0ACA22F0, 0x6E, 7, 0},
    {0x0ACA22F4, 0x6D, 7, 0},

};
#endif

//SRRC 
static u8 NTPCOMPRegsBW20M_SRRC[]={
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x5C,
	0x5C,
	0x5B,
	0x5A,
	0x59,
	0x57,
	0x55,
	0x53,
	0x50,
	0x4E,
	0x4B,
	0x47,
	0x44,
	0x40,
	0x3D,
	0x39,
	0x36,
	0x32,
	0x2E,
	0x2B,
	0x28,
	0x24,
	0x21,
	0x1E,
	0x1B,
	0x19,
	0x16,
	0x14,
	0x12,
	0x10,
	0x0E,
	0x0C,
	0x0B,
	0x09,
	0x08,
	0x07,
	0x06,
	0x05,
	0x04,
	0x04,
	0x03,
	0x03,
	0x02,
	0x02,
	0x02,
	0x01,
	0x01,
	0x01,
	0x01,
	0x01,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
};

static u8 NTPCOMPRegsBW40M_SRRC[]={
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x01,
	0x01,
	0x01,
	0x01,
	0x01,
	0x02,
	0x02,
	0x02,
	0x03,
	0x03,
	0x04,
	0x04,
	0x05,
	0x06,
	0x07,
	0x08,
	0x09,
	0x0B,
	0x0C,
	0x0E,
	0x10,
	0x12,
	0x14,
	0x16,
	0x19,
	0x1B,
	0x1E,
	0x21,
	0x24,
	0x28,
	0x2B,
	0x2E,
	0x32,
	0x36,
	0x39,
	0x3D,
	0x40,
	0x44,
	0x47,
	0x4B,
	0x4E,
	0x50,
	0x53,
	0x55,
	0x57,
	0x59,
	0x5A,
	0x5B,
	0x5C,
	0x5C,
	0x5C,
	0x5B,
	0x5A,
	0x59,
	0x57,
	0x55,
	0x53,
	0x50,
	0x4E,
	0x4B,
	0x47,
	0x44,
	0x40,
	0x3D,
	0x39,
	0x36,
	0x32,
	0x2E,
	0x2B,
	0x28,
	0x24,
	0x21,
	0x1E,
	0x1B,
	0x19,
	0x16,
	0x14,
	0x12,
	0x10,
	0x0E,
	0x0C,
	0x0B,
	0x09,
	0x08,
	0x07,
	0x06,
	0x05,
	0x04,
	0x04,
	0x03,
	0x03,
	0x02,
	0x02,
	0x02,
	0x01,
	0x01,
	0x01,
	0x01,
	0x01,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
};

#define NTP_COMP_REG_BW20_NUM sizeof(NTPCOMPRegsBW20M)/sizeof(NTPCOMPRegsBW20M[0])

static RegSetting_t NTPCOMPRegsBW20M_U10[]=
{
    {0x0ACA010C, 0x00, 19, 19},//params_prc_bypass

    {0x0ACA21FC, 0x4B, 7, 0},
    {0x0ACA2200, 0x4B, 7, 0},
    {0x0ACA2204, 0x4B, 7, 0},
    {0x0ACA2208, 0x4A, 7, 0},
    {0x0ACA220C, 0x4A, 7, 0},
    {0x0ACA2210, 0x4A, 7, 0},
    {0x0ACA2214, 0x49, 7, 0},
    {0x0ACA2218, 0x49, 7, 0},
    {0x0ACA221C, 0x49, 7, 0},
    {0x0ACA2220, 0x49, 7, 0},
    {0x0ACA2224, 0x48, 7, 0},
    {0x0ACA2228, 0x48, 7, 0},
    {0x0ACA222C, 0x47, 7, 0},
    {0x0ACA2230, 0x48, 7, 0},
    {0x0ACA2234, 0x47, 7, 0},
    {0x0ACA2238, 0x47, 7, 0},
    {0x0ACA223C, 0x47, 7, 0},
    {0x0ACA2240, 0x46, 7, 0},
    {0x0ACA2244, 0x47, 7, 0},
    {0x0ACA2248, 0x46, 7, 0},
    {0x0ACA224C, 0x46, 7, 0},
    {0x0ACA2250, 0x47, 7, 0},
    {0x0ACA2254, 0x47, 7, 0},
    {0x0ACA2258, 0x47, 7, 0},
    {0x0ACA225C, 0x47, 7, 0},
    {0x0ACA2260, 0x48, 7, 0},
    {0x0ACA2264, 0x48, 7, 0},
    {0x0ACA2268, 0x49, 7, 0},
    {0x0ACA226C, 0x49, 7, 0},
    {0x0ACA2270, 0x4A, 7, 0},
    {0x0ACA2274, 0x4B, 7, 0},
    {0x0ACA2278, 0x4C, 7, 0},
    {0x0ACA227C, 0x4C, 7, 0},
    {0x0ACA2280, 0x4D, 7, 0},
    {0x0ACA2284, 0x4F, 7, 0},
    {0x0ACA2288, 0x50, 7, 0},
    {0x0ACA228C, 0x51, 7, 0},
    {0x0ACA2290, 0x52, 7, 0},
    {0x0ACA2294, 0x54, 7, 0},
    {0x0ACA2298, 0x56, 7, 0},
    {0x0ACA229C, 0x57, 7, 0},
    {0x0ACA22A0, 0x59, 7, 0},
    {0x0ACA22A4, 0x5B, 7, 0},
    {0x0ACA22A8, 0x5D, 7, 0},
    {0x0ACA22AC, 0x5F, 7, 0},
    {0x0ACA22B0, 0x61, 7, 0},
    {0x0ACA22B4, 0x63, 7, 0},
    {0x0ACA22B8, 0x65, 7, 0},
    {0x0ACA22BC, 0x68, 7, 0},
    {0x0ACA22C0, 0x6A, 7, 0},
    {0x0ACA22C4, 0x6D, 7, 0},
    {0x0ACA22C8, 0x70, 7, 0},
    {0x0ACA22CC, 0x73, 7, 0},
    {0x0ACA22D0, 0x76, 7, 0},
    {0x0ACA22D4, 0x79, 7, 0},
    {0x0ACA22D8, 0x7D, 7, 0},
    {0x0ACA22DC, 0x80, 7, 0},
    {0x0ACA22E0, 0x84, 7, 0},
    {0x0ACA22E4, 0x88, 7, 0},
    {0x0ACA22E8, 0x8D, 7, 0},
    {0x0ACA22EC, 0x92, 7, 0},
    {0x0ACA22F0, 0x97, 7, 0},
    {0x0ACA22F4, 0x9A, 7, 0},

};

#define NTP_COMP_REG_BW20_U10_NUM sizeof(NTPCOMPRegsBW20M_U10)/sizeof(NTPCOMPRegsBW20M_U10[0])

static RegSetting_t NTPCOMPRegsBW20M_L10[]=
{
    {0x0ACA010C, 0x00, 19, 19},//params_prc_bypass

    {0x0ACA21FC, 0x9A, 7, 0},
    {0x0ACA2200, 0x97, 7, 0},
    {0x0ACA2204, 0x92, 7, 0},
    {0x0ACA2208, 0x8D, 7, 0},
    {0x0ACA220C, 0x88, 7, 0},
    {0x0ACA2210, 0x84, 7, 0},
    {0x0ACA2214, 0x80, 7, 0},
    {0x0ACA2218, 0x7D, 7, 0},
    {0x0ACA221C, 0x79, 7, 0},
    {0x0ACA2220, 0x76, 7, 0},
    {0x0ACA2224, 0x73, 7, 0},
    {0x0ACA2228, 0x70, 7, 0},
    {0x0ACA222C, 0x6D, 7, 0},
    {0x0ACA2230, 0x6A, 7, 0},
    {0x0ACA2234, 0x68, 7, 0},
    {0x0ACA2238, 0x65, 7, 0},
    {0x0ACA223C, 0x63, 7, 0},
    {0x0ACA2240, 0x61, 7, 0},
    {0x0ACA2244, 0x5F, 7, 0},
    {0x0ACA2248, 0x5D, 7, 0},
    {0x0ACA224C, 0x5B, 7, 0},
    {0x0ACA2250, 0x59, 7, 0},
    {0x0ACA2254, 0x57, 7, 0},
    {0x0ACA2258, 0x56, 7, 0},
    {0x0ACA225C, 0x54, 7, 0},
    {0x0ACA2260, 0x52, 7, 0},
    {0x0ACA2264, 0x51, 7, 0},
    {0x0ACA2268, 0x50, 7, 0},
    {0x0ACA226C, 0x4F, 7, 0},
    {0x0ACA2270, 0x4D, 7, 0},
    {0x0ACA2274, 0x4C, 7, 0},
    {0x0ACA2278, 0x4C, 7, 0},
    {0x0ACA227C, 0x4B, 7, 0},
    {0x0ACA2280, 0x4A, 7, 0},
    {0x0ACA2284, 0x49, 7, 0},
    {0x0ACA2288, 0x49, 7, 0},
    {0x0ACA228C, 0x48, 7, 0},
    {0x0ACA2290, 0x48, 7, 0},
    {0x0ACA2294, 0x47, 7, 0},
    {0x0ACA2298, 0x47, 7, 0},
    {0x0ACA229C, 0x47, 7, 0},
    {0x0ACA22A0, 0x47, 7, 0},
    {0x0ACA22A4, 0x46, 7, 0},
    {0x0ACA22A8, 0x46, 7, 0},
    {0x0ACA22AC, 0x47, 7, 0},
    {0x0ACA22B0, 0x46, 7, 0},
    {0x0ACA22B4, 0x47, 7, 0},
    {0x0ACA22B8, 0x47, 7, 0},
    {0x0ACA22BC, 0x47, 7, 0},
    {0x0ACA22C0, 0x48, 7, 0},
    {0x0ACA22C4, 0x47, 7, 0},
    {0x0ACA22C8, 0x48, 7, 0},
    {0x0ACA22CC, 0x48, 7, 0},
    {0x0ACA22D0, 0x49, 7, 0},
    {0x0ACA22D4, 0x49, 7, 0},
    {0x0ACA22D8, 0x49, 7, 0},
    {0x0ACA22DC, 0x49, 7, 0},
    {0x0ACA22E0, 0x4A, 7, 0},
    {0x0ACA22E4, 0x4A, 7, 0},
    {0x0ACA22E8, 0x4A, 7, 0},
    {0x0ACA22EC, 0x4B, 7, 0},
    {0x0ACA22F0, 0x4B, 7, 0},
    {0x0ACA22F4, 0x4B, 7, 0},
};

#define NTP_COMP_REG_BW20_L10_NUM sizeof(NTPCOMPRegsBW20M_L10)/sizeof(NTPCOMPRegsBW20M_L10[0])

void wifiax_ntp_comp_setting(ContTxParam_t *pContTxParam)
{
    unsigned int i = 0;
    unsigned int RegNum = 0;
    RegSetting_t *pReg = NULL;
	unsigned int addr = 0;
	int reg_idx = 0;

    if(NULL == pContTxParam)
    {
        return;
    }

    //0ACB8900  40  [12:4]  //params_tx_digital_scale
    ATBMPhyRegBitsSet(0x0ACB8900, 0x40, 9, 4);

#if 0
    ACA0104[9:0]  21c // NtScale_512
    ACA0104[29:20] 21c // NtScale_128
    
    ���??0U/20L
    ACA0104[19:10] 175 // NtScale_256
    ACA0110[9:0] 175 // NtScale_64
    ���??0M
    ACA0104[19:10] 104 // NtScale_256
    ACA0110[9:0] 104 // NtScale_64
#endif

    //only valid for 40M
    ATBMPhyRegBitsSet(0x0ACA0104, 0x21C, 10, 0);// NtScale_512
    ATBMPhyRegBitsSet(0x0ACA0104, 0x21C, 10, 20);// NtScale_128

    //precomp
    if(1 == pContTxParam->TxVector1.Bits.ChBW)
    {//Bandwidth 40M
        RegNum = NTP_COMP_REG_BW40_NUM;
        pReg = NTPCOMPRegsBW40M;
    }
    else if(0 == pContTxParam->TxVector1.Bits.ChBW)
    {//Bandwidth 20M
        if(0 == pContTxParam->TxVector1.Bits.ChOffset)
        {//ZERO
            RegNum = NTP_COMP_REG_BW20_NUM;
			if(pContTxParam->precom == 0)
            	pReg = NTPCOMPRegsBW20M;
			else if(pContTxParam->precom == 1)
				pReg = NTPCOMPRegsBW20M_arc8M;
			else if(pContTxParam->precom == 2)
				pReg = NTPCOMPRegsBW20M_arc9M;
            ATBMPhyRegBitsSet(0x0ACA0104, 0x104, 10, 10);// NtScale_256
            ATBMPhyRegBitsSet(0x0ACA0110, 0x104, 10, 0);// NtScale_64
        }
        else if(1 == pContTxParam->TxVector1.Bits.ChOffset)
        {//U10
            RegNum = NTP_COMP_REG_BW20_U10_NUM;
            pReg = NTPCOMPRegsBW20M_U10;
            ATBMPhyRegBitsSet(0x0ACA0104, 0x168, 10, 10);// NtScale_256
            ATBMPhyRegBitsSet(0x0ACA0110, 0x168, 10, 0);// NtScale_64
        }
        else
        {//L10
            RegNum = NTP_COMP_REG_BW20_L10_NUM;
            pReg = NTPCOMPRegsBW20M_L10;
            ATBMPhyRegBitsSet(0x0ACA0104, 0x168, 10, 10);// NtScale_256
            ATBMPhyRegBitsSet(0x0ACA0110, 0x168, 10, 0);// NtScale_64
        }
    }

	if(pContTxParam->precom == 3)
    {
		//SRRC 20M
		ATBMPhyRegBitsSet(0x0ACA0104, 0x380, 10, 0);// NtScale_512
        ATBMPhyRegBitsSet(0x0ACA0110, 0x380, 10, 20);// NtScale_128
        
		addr = 0xACA2000;
		for (reg_idx=0; reg_idx<127; reg_idx++)
		{
			HW_WRITE_REG_BIT(addr, 7, 0, NTPCOMPRegsBW20M_SRRC[reg_idx]);
			addr = addr + 0x4;		
		}
		
	}
	else if(pContTxParam->precom == 4)
    {
		//SRRC 40M
		ATBMPhyRegBitsSet(0x0ACA0104, 0x250, 10, 0);// NtScale_512
        ATBMPhyRegBitsSet(0x0ACA0110, 0x250, 10, 20);// NtScale_128
        
		addr = 0xACA2000;
		for (reg_idx=0; reg_idx<127; reg_idx++)
		{
			HW_WRITE_REG_BIT(addr, 7, 0, NTPCOMPRegsBW40M_SRRC[reg_idx]);
			addr = addr + 0x4;		
		}
	}
	else
	{
		for(i=0;i<RegNum;i++)
	    {
	        ATBMPhyRegBitsSet(pReg->Addr, pReg->Val, (pReg->Msb + 1 - pReg->Lsb), pReg->Lsb);
	        pReg++;
	    }
	}
}

//set tx parameters, tx action not starting
void ETF_PHY_TxParamSet(ContTxParam_t *pContTxParam)
{
    ETF_PHY_Cont_Tx_Param_Set(pContTxParam);
    //NTPComp
    wifiax_ntp_comp_setting(pContTxParam);
}

static RegSetting_t BASIC_REG_SCRIPT_TX[]=
{
	{0xACD0080, 0x0   ,0,0}, 
	{0xACB8900, 0x1   ,3,3},
	{0xACB8900, 0x0   ,2,2},	
	{0xACB8908, 0x801 ,21,10},
	{0xACB892C, 0x801 ,11,0},	
	{0xACB892C, 0x801 ,23,12},
	{0xACB8930, 0x801 ,11,0},	
	{0xACB8998, 0x0   ,1,0},	
	{0xACB8A00, 0xc0  ,11,0},	
	{0xACB8A00, 0xc0  ,23,12},
	{0xACB8A04, 0x0   ,11,0},	
	{0xACB8A04, 0x0   ,23,12},
	{0xACB8A08, 0x0   ,11,0},	
	{0xACB8A08, 0x0   ,23,12},
	{0xACB8A0C, 0x1   ,2,0},	
	{0xACB8A0C, 0x1   ,5,3},	
	{0xACB8A10, 0x1   ,2,0},	
	{0xACB8A10, 0x1   ,5,3},	
	{0xACB8AB8, 0x20  ,6,0},	
	{0xACB8AB8, 0x20  ,13,7},	
	{0xACB8AD0, 0x0   ,13,0},	
	{0xACB8ACC, 0x0   ,12,0},	
	{0xACB8A14, 0x1   ,0,0},	
	{0xAC98040, 0x0   ,31,0},
	{0xac980cc, 0x1   ,0,0},
};

#define BASIC_REG_SCRIPT_TX_NUM sizeof(BASIC_REG_SCRIPT_TX)/sizeof(BASIC_REG_SCRIPT_TX[0])

////load Tx basic reigster setting, mem etc
void ETF_PHY_Load_Basic_Reg_TxSetting(void)
{	
    RegSetting_t *pReg = NULL;
	int i;
    unsigned int RegNum = 0;
	
    pReg = BASIC_REG_SCRIPT_TX;
    RegNum = BASIC_REG_SCRIPT_TX_NUM;
	
    for(i=0;i<RegNum;i++)
    {
        ATBMPhyRegBitsSet(pReg->Addr, pReg->Val, (pReg->Msb + 1 - pReg->Lsb), pReg->Lsb);
        pReg++;
    }
}

void ETF_PHY_Chip_Reset(void)
{
	ATBMPhyRegI2CWrite(0xACD0008, 0x04);
	ATBMPhyRegI2CWrite(0xACD0008, 0x07);
}


//stop rx
void ETF_PHY_Cont_Rx_Stop(ContRxParam_t *pRxParam)
{
    ATBMPhyRegBitsSet(0x0ACD0000, 0, 1, 16);//EnableRx        0000        16  ACD0000
}

unsigned int ETF_PHY_ChannelCenterFrequencyGet(unsigned int ChannelNo)
{
	unsigned int ChFreqMHz = 0;
    if((ChannelNo >= 1)&&(ChannelNo <= 13))
    {
        ChFreqMHz = 2412 + 5*(ChannelNo-1);
    }
    else if(ChannelNo < 1)
    {
        ChFreqMHz = 2412;
    }
    else if(ChannelNo > 13)
    {
        ChFreqMHz = 2484;
    }
	return ChFreqMHz;
}

void ETF_PHY_Cont_Rx_Param_Parse(ETF_PHY_RX_PARAM_T *pRxParam,ContRxParam_t *pContRxParam)
{  
    //UI parameters parse
    //带宽寄存器：ACB8004 [0]�?  1�?40M�?0�?20M
    //params_primary_channel_is_upper_flag      094C        0   ACB894C 1   1: primary is upper; 0: primayr is lower;
    if(((ATBM_WIFI_BW_20M == pRxParam->BW)||(ATBM_WIFI_BW_RU242 == pRxParam->BW)||(ATBM_WIFI_BW_RU106 == pRxParam->BW))
        &&(ATBM_WIFI_CH_OFFSET_0 == pRxParam->ChOffset))
    {//bandwidth is 20M and channel offset is ZERO
        pContRxParam->BandWidthMHz = 20;
    }
    else
    {
        pContRxParam->BandWidthMHz = 40;
    }

    if(ATBM_WIFI_CH_OFFSET_10U == pRxParam->ChOffset)
    {
        pContRxParam->PrimaryChUpper = 1; //U10, primary is upper
    }
    else if(ATBM_WIFI_CH_OFFSET_10L == pRxParam->ChOffset)
    {
        pContRxParam->PrimaryChUpper = 0;//L10, primary is lower
    }
    else
    {
        pContRxParam->PrimaryChUpper = 0;
    }
    pContRxParam->StationID0 = pRxParam->StationID0;
    pContRxParam->StationID1 = 0xFFF;
    pContRxParam->StationID2 = 0xFFF;
    pContRxParam->StationID3 = 0xFFF;
    pContRxParam->DefaultRFBW = pRxParam->DefaultRFBW;
    //pContRxParam->FreqMHz = ETF_PHY_ChannelCenterFrequencyGet(pRxParam->ChannelNum);
    pContRxParam->FreqMHz 	  = pRxParam->FreqMHz;
}
#if 0
static void wifiax_rx_rf_set(ContRxParam_t *pRxParam)
{
/*
20M 带宽RF 芯片寄存器配置：
HW_WRITE_REG_BIT(0xacc0298,0,0,0);
HW_WRITE_REG_BIT(0xacc0298,1,1,0);

40M 带宽RF 芯片寄存器配置：
HW_WRITE_REG_BIT(0xacc0298,0,0,1);
HW_WRITE_REG_BIT(0xacc0298,1,1,1);

*/
    if(20 == pRxParam->BandWidthMHz)
    {
        HW_WRITE_BIT(0x0ACC0298,0, 0,0);
        HW_WRITE_BIT(0x0ACC0298,0, 1,1);
    }
    else
    {
        HW_WRITE_BIT(0x0ACC0298,1, 0,0);
        HW_WRITE_BIT(0x0ACC0298,1, 1,1);
    }

//    if(!pRxParam->DefaultRFBW)
{
    unsigned int RegACC00A4Val = 0;
    unsigned int RegACB8004Val = 0;
    if((40 == pRxParam->BandWidthMHz)&&(!pRxParam->DefaultRFBW))
    {
        RegACC00A4Val = 0x00;
    }
    else
    {
        RegACC00A4Val = 0x0F;
    }
    RegACB8004Val = HW_READ_BIT(0x0ACB8004,0,31);//backup ACB8004
    // force rx enable = 0
    //ACB8004[19:18] 2’b10 
    HW_WRITE_BIT(0x0ACB8004,0x02, 18,19);
    //ACB8004[15] 1’b0
    HW_WRITE_BIT(0x0ACB8004,0x00, 15,15);
    // force rf bw
    //ACC009C [4] 1   （这个是defalut 值）
    HW_WRITE_BIT(0x0ACC009C,0x01, 4,4);
    //ACC009C [8] 1   （这个是defalut 值）
    HW_WRITE_BIT(0x0ACC009C,0x01, 8,8);
    //ACC00A4 [8:4] 0   
    HW_WRITE_BIT(0x0ACC00A4,RegACC00A4Val, 4,8);
    
    // normal rx, not rx
    //ACB8004[19:18]  default �?    //ACB8004[15]  default �?    HW_WRITE_BIT(0x0ACB8004,RegACB8004Val, 0,31);//restore ACB8004
}
    
}
#endif
static RegSetting_t RxRegsBW20M[]=
{
    {0x0ACB89A0, 0x3FD,    9,  0},//rx precompen filter
    {0x0ACB89A0,    0x4,  19, 10},
    {0x0ACB89A0,    0x8,  29, 20},
    {0x0ACB89A4,  0x3EE,   9,  0},
    {0x0ACB89A4,  0x3CC,  19, 10},
    {0x0ACB89A4,  0x179,  29, 20},
};
#define RX_REG_BW20_NUM sizeof(RxRegsBW20M)/sizeof(RxRegsBW20M[0])

static RegSetting_t RxRegsBW40M[]=
{
    {0x0ACB89A0, 0x3FA,    9,  0},//rx precompen filter
    {0x0ACB89A0,    0xB,  19, 10},
    {0x0ACB89A0,    0xF,  29, 20},
    {0x0ACB89A4,  0x3DF,   9,  0},
    {0x0ACB89A4,  0x39F,  19, 10},
    {0x0ACB89A4,  0x1DE,  29, 20},
};
#define RX_REG_BW40_NUM sizeof(RxRegsBW40M)/sizeof(RxRegsBW40M[0])

//rx counter reset
void wifiax_cont_rx_cnt_rst(void)
{
    ATBMPhyRegBitsSet(0x0AC88334, 1, 1,  8);//params_rst_rnn_evm      334     8   AC88334 0       REG_CFG ro_evm reset
    ATBMPhyRegBitsSet(0x0AC88334, 0, 1,  8);
    
    ATBMPhyRegBitsSet(0x0AC882F4, 1, 1,  2); //ch_len_small_flag_stat_clr
    ATBMPhyRegBitsSet(0x0AC882F4, 0, 1, 2);

    ATBMPhyRegBitsSet(0x0ACD0020, 1, 1,  0); //params_rst_frame_num        0020        0   ACD0020 0
    ATBMPhyRegBitsSet(0x0ACD0020, 0, 1, 0);

    ATBMPhyRegBitsSet(0x0AC90298, 1, 1,  0); //params_rst_ro_sounding_counter      0298        0:0 AC90298
    ATBMPhyRegBitsSet(0x0AC90298, 0, 1, 0);
}

//start rx
void ETF_PHY_Cont_Rx_Start(ContRxParam_t *pRxParam)
{
    unsigned int i = 0;
    unsigned int RegNum = 0;
    RegSetting_t *pReg = NULL;

    //RF register setting
    //if(pRxParam->RFRegNeedSet)
    //{
    //   wifiax_rx_rf_set(pRxParam);
    //}

    //rx counter reset
    wifiax_cont_rx_cnt_rst();

    //带宽寄存器：ACB8004 [0]�?  1�?40M�?0�?20M
    //params_primary_channel_is_upper_flag      094C        0   ACB894C 1   1: primary is upper; 0: primayr is lower;
    if(20 == pRxParam->BandWidthMHz)
    {
        ATBMPhyRegBitsSet(0x0ACB8004, 0, 1, 0); //bandwidth 20MHz
        RegNum = RX_REG_BW20_NUM;
        pReg = RxRegsBW20M;
    }
    else
    {
        ATBMPhyRegBitsSet(0x0ACB8004, 1, 1, 0); //bandwidth 40MHz
        RegNum = RX_REG_BW40_NUM;
        pReg = RxRegsBW40M;
    }
    for(i=0;i<RegNum;i++)
    {
        ATBMPhyRegBitsSet(pReg->Addr, pReg->Val, (pReg->Msb + 1 - pReg->Lsb), pReg->Lsb);
        pReg++;
    }

    if(1 == pRxParam->PrimaryChUpper)
    {
        ATBMPhyRegBitsSet(0x0ACB894C, 1, 1, 0); //U10, primary is upper
    }
    else
    {
        ATBMPhyRegBitsSet(0x0ACB894C, 0, 1, 0); //L10, primayr is lower
    }
    
    ATBMPhyRegBitsSet(0x0AC901B8, pRxParam->StationID0, 12, 0); //StationID0
    ATBMPhyRegBitsSet(0x0AC901BC, pRxParam->StationID1, 12, 0); //StationID1
    ATBMPhyRegBitsSet(0x0AC901C0, pRxParam->StationID2, 12, 0); //StationID2
    ATBMPhyRegBitsSet(0x0AC901C4, pRxParam->StationID3, 12, 0); //StationID3

    //params_fs_over_fc_clock_lock, AC88230 [11:0], fs_over_fc_clock_lock = dec2hex(round[2^18*(20*10^6)/FreqHz])
    //ATBMPhyRegBitsSet(0x0AC88230, pRxParam->FreqMHz, 12, 0); //params_fs_over_fc_clock_lock
}


static RegSetting_t BASIC_REG_SCRIPT_RX[]=
{
	{0xAC80800	,0x25BC60  ,31, 0},
	{0xAC80804	,0x25BC60  ,31, 0},
	{0xAC80808	,0x25BC60  ,31, 0},
	{0xAC8080C	,0x25BC60  ,31, 0},
	{0xAC80810	,0x25BC60  ,31, 0},
	{0xAC80814	,0x25BC60  ,31, 0},
	{0xAC80818	,0x25BC60  ,31, 0},
	{0xAC8081C	,0x25BA64  ,31, 0},
	{0xAC80820	,0x25B868  ,31, 0},
	{0xAC80824	,0x25B66C  ,31, 0},
	{0xAC80828	,0x25B46D  ,31, 0},
	{0xAC8082C	,0x25B26E  ,31, 0},
	{0xAC80830	,0x25B07C  ,31, 0},
	{0xAC80834	,0x25AE7D  ,31, 0},
	{0xAC80838	,0x25AC7E  ,31, 0},
	{0xAC8083C	,0x25AA70  ,31, 0},
	{0xAC80840	,0x25A840  ,31, 0},
	{0xAC80844	,0x25A644  ,31, 0},
	{0xAC80848	,0x25A448  ,31, 0},
	{0xAC8084C	,0x25224C  ,31, 0},
	{0xAC80850	,0x24204D  ,31, 0},
	{0xAC80854	,0x231E4E  ,31, 0},
	{0xAC80858	,0x221C5C  ,31, 0},
	{0xAC8085C	,0x211A20  ,31, 0},
	{0xAC80860	,0x201824  ,31, 0},
	{0xAC80864	,0x1F1628  ,31, 0},
	{0xAC80868	,0x1E142C  ,31, 0},
	{0xAC8086C	,0x1D122D  ,31, 0},
	{0xAC80870	,0x1C102E  ,31, 0},
	{0xAC80874	,0x1B0E3C  ,31, 0},
	{0xAC80878	,0x1A0C3D  ,31, 0},
	{0xAC8087C	,0x190A3E  ,31, 0},
	{0xAC80880	,0x180830  ,31, 0},
	{0xAC80884	,0x170631  ,31, 0},
	{0xAC80888	,0x160432  ,31, 0},
	{0xAC8088C	,0x150234  ,31, 0},
	{0xAC80890	,0x140035  ,31, 0},
	{0xAC80894	,0x137E36  ,31, 0},
	{0xAC80898	,0x127C38  ,31, 0},
	{0xAC8089C	,0x117A39  ,31, 0},
	
	{0xAC80444	,0x9a		,9 ,0 },  //one over r
	{0xACB8004	,0x20024002 ,31 ,1 },
	{0xACD0060	,0x2		  ,31 ,0 },
	{0xACD0000	,0x00010000 ,31 ,0 },
	{0xAC90164	,0xFFFFFFFF  ,31 ,0 },
	{0xACB8904	,0xFA,	  31 ,0 },
	{0xAC88320	,0x00000010, 31 ,0 },
	{0xaca805c	,0x4309261A ,31 ,0 },//dsss sync reg
	{0xACA803C	,0x77671F7B ,31 ,0 },// BRAGCT2
	{0xACA8574	,0x090a0f0f ,31 ,0 },// BRDLYSCCA
	{0xACB0078	,0x480035	,31 ,0 },// BRCALC2
	{0xACA8540	,0x0000017f ,31 ,0 },// BRCCACALC
	{0xACA8048	,0x342D6F0	,31 ,0 },// BRAGCT1
	{0xACA8040	,0x77bbd	,31 ,0 },// BRCONF
	{0xac80ca0	,0xf ,7 ,4 },  // power up enable
	{0xAC880AC	,0xc ,5 ,0 }, // params_ch_len_th1_high_snr 00AC	5 ,0	AC880AC 6
	{0xAC880AC	,0xc ,11 ,6 }, // ch_len_th1_low_snr 
	{0xAC880AC	,0xc ,17 ,12 }, // params_ch_len_th2_high_snr	00AC	17 ,12	AC880AC 6
	{0xAC880AC	,0xc ,23 ,18 }, // ch_len_th2_low_snr 
	{0xAC880B0	,0xc ,17 ,12 }, // params_ch_len_th_high_snr	00B0	17 ,12	AC880B0 6
	{0xAC880B0	,0xc ,23 ,18 }, // params_ch_len_th_low_snr 
	{0xAC880C4	,0xc  ,5 ,0 }, //params_ch_len_th1_high_snr_2x
	{0xAC880C4	,0xc  ,11 ,6  }, // params_ch_len_th1_high_snr_4x
	{0xAC880C4	,0xc  ,17 ,12 }, // params_ch_len_th2_high_snr_2x
	{0xAC880C4	,0xc  ,23 ,18 }, // params_ch_len_th2_high_snr_4x
	{0xAC880C8	,0x1e ,5 ,0 },	 //params_ch_len_th3_high_snr_2x
	{0xAC880C8	,0x1e ,11 ,6 }, // params_ch_len_th3_high_snr_4x
	{0xAC880C8	,0xc  ,17 ,12 }, // params_ch_len_th_high_snr_2x 
	{0xAC880C8	,0xc  ,23 ,18 }, // params_ch_len_th_high_snr_4x 
	{0xac80154	 ,0x1c1c4F66   ,29 ,0 },
	{0xac8020c	 ,0x1a		   ,5 ,0 },
	{0xac8020c	 ,0x1a		   ,11 ,6 },
	{0xAC88234 ,0x6 ,3 ,0 }, //thr_bpsk_data 
	{0xac80d40 ,0x6 ,24 ,21 },
	{0xac80444 ,0x1d ,19 ,14 },
	{0xac80464 ,0x0 ,17 ,12 },
	{0xac80434 ,0x0 ,1 ,1 },
	{0xAC80D08 ,0x3F ,11 ,0 }, //params_cca_fsm_watchdog_en  AC80D08  5 ,0
	{0xac80ca0 ,0x1 ,25 ,25 }, // 0 , use sync end of ltf to decide frame end;
	{0xac80ca0 ,0x1 ,0 ,0 },  //  double check suc continue to run dsss;
	{0xAC80CCC ,0xfe ,9 ,0 },  //frame_end_cnt_for_rx_phy_ready_thr1/2/3/4, hex2dec('108')/20=13.2us, filter delay 1.1us, so rx delay 14.3us
	{0xAC80D00 ,0xfe ,9 ,0 },
	{0xAC80D00 ,0xfe ,25 ,16 },
	{0xAC80D04 ,0xfe ,9 ,0 },
	{0xac80cc4 ,0x6  ,23 ,16 }, // 40M_no_initial_cnt
	{0xAC901CC ,0xd  ,22 ,17 }, //ht_min_length
	{0xac901cc ,0x7f ,16 ,9 }, //he_duration_limit, the minimum he data bits
	{0xAC80450	,0x80  ,21 ,14 },	// sat_thr, 8*16+2=130, U(8,-2) 				
	{0xAC80454	,0x10	,5 ,0 },   // n_above_thr_high_20M			
	{0xAC80454	,0x10	,11 ,6 },	// n_above_thr_high_40M 
	{0xAC80454	,0x28	,17 ,12 },	// n_above_thr_low_20M		
	{0xAC80454	,0x28	,23 ,18 },	// n_above_thr_low_40M		
	{0xAC80458	,0xd8  ,7 ,0 },   // pwr_sat_high_thr_dB, 12*4+3 = 51dB 		
	{0xAC80458	,0xd0  ,15 ,8 },	// pwr_sat_low_thr_dB, 12*4=48dB		
	{0xAC80458	,0xcc  ,23 ,16 }, // pwr_sat_not_sat_thr_dB, 11*4 + 1 = 45dB
	{0xAC8045C	,0x1   ,1 ,0 },   // num_sat_buffer 					
	{0xAC8045C	,0x6   ,5 ,2 },   // n_sat_thr_20M						
	{0xAC8045C	,0xb   ,9 ,6 },   // n_sat_thr_40M						
	{0xAC8045C	,0x0   ,13 ,10 },	//continuous_not_sat_points
	{0xAC804BC ,0x0   ,0 ,0 }, // aagc_sat_condition_method
	{0xAC80D48 ,0x8 ,8 ,0 }, //iq_enable_cnt_mode_enable = 0, iq_enable_delay_cnt = 8;
	{0xacb8b34 ,0x1 ,0 ,0 }, // aagc init gain enable
	{0xAC880bc ,0x3c ,31 ,25 },  //aagc high gain value 
	{0xAC901CC ,0x0 ,29 ,29 }, //sig_b_over_16 unsupported
	{0xAC88230 ,0x875 ,11 ,0 }, //fs_over_fc =	dec2hex(round(20e6/2.422e9*2^18)), 2.422e9 Hz is channle 3 frequency;
	{0xAC80D18 ,0x140000 ,21 ,0 }, // cca watch dog cnt , dsss_rx_processing thr,  hex2dec('140000')/20e3=65.5ms, dsss max long packets is 64ms
};

#define BASIC_REG_SCRIPT_RX_NUM sizeof(BASIC_REG_SCRIPT_RX)/sizeof(BASIC_REG_SCRIPT_RX[0])


////load Tx basic reigster setting, mem etc
void ETF_PHY_Load_Basic_Reg_RxSetting(void)
{	
    RegSetting_t *pReg = NULL;
	int i;
    unsigned int RegNum = 0;
	
    pReg = BASIC_REG_SCRIPT_RX;
    RegNum = BASIC_REG_SCRIPT_RX_NUM;
	
    for(i=0;i<RegNum;i++)
    {
        ATBMPhyRegBitsSet(pReg->Addr, pReg->Val, (pReg->Msb + 1 - pReg->Lsb), pReg->Lsb);
        pReg++;
    }
}
ContTxParam_t ContTxParam;

void ETF_PHY_Start_Tx_Step1(ETF_HE_TX_CONFIG_REQ* pItem)
{
	ETF_HE_TX_CONFIG_REQ *pEtfHeTxConfigReq = (ETF_HE_TX_CONFIG_REQ *)pItem;
	ETF_PHY_TX_PARAM_T *pContTxMsgParam= &pEtfHeTxConfigReq->TxConfig;
	
	memset(&ContTxParam, 0, sizeof(ContTxParam_t));
	
	//ETF_PHY_Chip_Reset(); 
	//get current parameter setting from registers
	//ETF_PHY_Cont_Tx_Param_Get(&ContTxParam);
	//stop tx
	ETF_PHY_Cont_Tx_Stop(&ContTxParam);
	//translate user parameter(ETF_PHY_TX_PARAM_T) to driver parameter(ContTxParam_t)
	ETF_PHY_Cont_Tx_Param_Parse(pContTxMsgParam,&ContTxParam);
	//set all paramters except "ContTxParam.nt_contf"
	ETF_PHY_TxParamSet(&ContTxParam);
}
void ETF_PHY_Start_Tx_Step2(ETF_HE_TX_CONFIG_REQ* pItem)
{
	//load register script
	//ETF_PHY_Load_Basic_Reg_TxSetting();

	ETF_PHY_Cont_Tx_Start(&ContTxParam);
}
void ETF_PHY_Stop_Tx(ETF_HE_TX_CONFIG_REQ* pItem)
{
	//stop tx
	ETF_PHY_Cont_Tx_Stop(NULL);
}
void ETF_PHY_Start_Rx(ETF_HE_RX_CONFIG_REQ* pItem)
{
	ETF_HE_RX_CONFIG_REQ *pEtfHeRxConfigReq = (ETF_HE_RX_CONFIG_REQ *)pItem;
	ETF_PHY_RX_PARAM_T *pContRxMsgParam= &pEtfHeRxConfigReq->RxConfig;
	ContRxParam_t ContRxParam;

	//ETF_PHY_Chip_Reset(); 
	//stop rx
	//ETF_PHY_Cont_Rx_Stop(NULL);

	ETF_PHY_Cont_Rx_Param_Parse(pContRxMsgParam,&ContRxParam);
	//start rx
	ETF_PHY_Cont_Rx_Start(&ContRxParam);

	//load register script
	//ETF_PHY_Load_Basic_Reg_RxSetting();
	ATBMPhyRegBitsSet(0x0ACD0000, 1, 1, 16);//EnableRx        0000        16  ACD0000
}
void ETF_PHY_Stop_Rx(ETF_HE_RX_CONFIG_REQ* pItem)
{
	//stop rx
	ETF_PHY_Cont_Rx_Stop(NULL);
}
void ETF_PHY_TxRxParamInit(ETF_HE_TX_CONFIG_REQ* pItem)
{
	ETF_PHY_TX_PARAM_T *pEtfPhyTxParam = &pItem->TxConfig;
	
	pEtfPhyTxParam->FreqMHz=2422;				
	pEtfPhyTxParam->ChannelNum=7;
	pEtfPhyTxParam->WiFiMode=0;
	pEtfPhyTxParam->OFDMMode=2;
	pEtfPhyTxParam->BW=0;
	pEtfPhyTxParam->ChOffset=0;
	pEtfPhyTxParam->Rate=0;
	pEtfPhyTxParam->GIMode=0;
	pEtfPhyTxParam->PreambleMode=0;
	pEtfPhyTxParam->PSDULen=1024;
	pEtfPhyTxParam->PacketInterval=16;
	pEtfPhyTxParam->DigitalScaler=1;
	pEtfPhyTxParam->PacketNum=0;
	pEtfPhyTxParam->DataRateMbps=0;
	pEtfPhyTxParam->TxMode=0;
	pEtfPhyTxParam->InfiniteLongPacket=0;
	pEtfPhyTxParam->MPDUNum=1;
	pEtfPhyTxParam->MPDULen=0;
	pEtfPhyTxParam->Smoothing=0;
	pEtfPhyTxParam->Sounding=1;
	pEtfPhyTxParam->Aggregation=0;
	pEtfPhyTxParam->STBC=0;
	pEtfPhyTxParam->LTFNum=1;
	pEtfPhyTxParam->BeamFormed=0;
	pEtfPhyTxParam->Doppler=0;
	pEtfPhyTxParam->BurstLen=4090;
	pEtfPhyTxParam->TxopDuration=0;
	pEtfPhyTxParam->NoSigExtn=1;
	pEtfPhyTxParam->ServiceField=0;
	pEtfPhyTxParam->TxPower=0;
	pEtfPhyTxParam->TxStreams=0;
	pEtfPhyTxParam->TxAntennas=0;
	pEtfPhyTxParam->TxAbsPower=0;
	pEtfPhyTxParam->TxPowerModeSel=0;
	pEtfPhyTxParam->StartingStsNum=0;
	pEtfPhyTxParam->HELTFMode=0;
	pEtfPhyTxParam->HESigA2Reserved=0;
	pEtfPhyTxParam->SpatialReuse1=0;
	pEtfPhyTxParam->SpatialReuse2=0;
	pEtfPhyTxParam->SpatialReuse3=0;
	pEtfPhyTxParam->SpatialReuse4=0;
	pEtfPhyTxParam->TriggerResponding=0;
	pEtfPhyTxParam->TriggerMethod=0;
	pEtfPhyTxParam->BSSColor=0;
	pEtfPhyTxParam->UplinkFlag=1;
	pEtfPhyTxParam->ScramblerValueEn=0;
	pEtfPhyTxParam->ScramblerValue=0;
	pEtfPhyTxParam->LdpcExtrSysm=0;
	pEtfPhyTxParam->ReservedForMAC=0;
	pEtfPhyTxParam->CFO=0;
	pEtfPhyTxParam->PPM=0;
	pEtfPhyTxParam->DCM=0;
	pEtfPhyTxParam->Coding=0;
	pEtfPhyTxParam->Padding=0;
	pEtfPhyTxParam->AFactor=0;
	pEtfPhyTxParam->PEDisambiguity=0;
	pEtfPhyTxParam->MidamblePeriod=0;
	pEtfPhyTxParam->BeamChange=0;
	pEtfPhyTxParam->RuAllocation=0;
	pEtfPhyTxParam->DefaultRFBW=0;
	pEtfPhyTxParam->StationID0=1440;
}


int atbm_set_channel(struct atbm_common *hw_priv, u8 flag)
{
	int ret = -1;
	//set channel
	struct wsm_set_chantype arg = {
			.band = 0,			//0:2.4G,1:5G
			.flag = flag,		//no use
			.channelNumber = hw_priv->etf_channel, // channel number
			.channelType =  hw_priv->etf_channel_type,	// channel type
			};

	ret = wsm_set_chantype_func(hw_priv,&arg,0);
	return ret;
}



#define PRINT_VARIABLE(var)  {atbm_printk_always(#var"=%d\n", var);}
void ETFTxConfigShow(ETF_PHY_TX_PARAM_T *pEtfPhyTxParam)
{
	PRINT_VARIABLE(pEtfPhyTxParam->FreqMHz				);
	PRINT_VARIABLE(pEtfPhyTxParam->ChannelNum           );
	PRINT_VARIABLE(pEtfPhyTxParam->WiFiMode             );
	PRINT_VARIABLE(pEtfPhyTxParam->OFDMMode             );
	PRINT_VARIABLE(pEtfPhyTxParam->BW                   );
	PRINT_VARIABLE(pEtfPhyTxParam->ChOffset             );
	PRINT_VARIABLE(pEtfPhyTxParam->Rate                 );
	PRINT_VARIABLE(pEtfPhyTxParam->GIMode               );
	PRINT_VARIABLE(pEtfPhyTxParam->PreambleMode         );
	PRINT_VARIABLE(pEtfPhyTxParam->PSDULen              );
	PRINT_VARIABLE(pEtfPhyTxParam->PacketInterval       );
	PRINT_VARIABLE(pEtfPhyTxParam->DigitalScaler        );
	PRINT_VARIABLE(pEtfPhyTxParam->PacketNum            );
	PRINT_VARIABLE(pEtfPhyTxParam->DataRateMbps         );
	PRINT_VARIABLE(pEtfPhyTxParam->TxMode               );
	PRINT_VARIABLE(pEtfPhyTxParam->InfiniteLongPacket   );
	PRINT_VARIABLE(pEtfPhyTxParam->MPDUNum              );
	PRINT_VARIABLE(pEtfPhyTxParam->MPDULen              );
	PRINT_VARIABLE(pEtfPhyTxParam->Smoothing            );
	PRINT_VARIABLE(pEtfPhyTxParam->Sounding             );
	PRINT_VARIABLE(pEtfPhyTxParam->Aggregation          );
	PRINT_VARIABLE(pEtfPhyTxParam->STBC                 );
	PRINT_VARIABLE(pEtfPhyTxParam->LTFNum               );
	PRINT_VARIABLE(pEtfPhyTxParam->BeamFormed           );
	PRINT_VARIABLE(pEtfPhyTxParam->Doppler              );
	PRINT_VARIABLE(pEtfPhyTxParam->BurstLen             );
	PRINT_VARIABLE(pEtfPhyTxParam->TxopDuration         );
	PRINT_VARIABLE(pEtfPhyTxParam->NoSigExtn            );
	PRINT_VARIABLE(pEtfPhyTxParam->ServiceField         );
	PRINT_VARIABLE(pEtfPhyTxParam->TxPower              );
	PRINT_VARIABLE(pEtfPhyTxParam->TxStreams            );
	PRINT_VARIABLE(pEtfPhyTxParam->TxAntennas           );
	PRINT_VARIABLE(pEtfPhyTxParam->TxAbsPower           );
	PRINT_VARIABLE(pEtfPhyTxParam->TxPowerModeSel       );
	PRINT_VARIABLE(pEtfPhyTxParam->StartingStsNum       );
	PRINT_VARIABLE(pEtfPhyTxParam->HELTFMode            );
	PRINT_VARIABLE(pEtfPhyTxParam->HESigA2Reserved      );
	PRINT_VARIABLE(pEtfPhyTxParam->SpatialReuse1        );
	PRINT_VARIABLE(pEtfPhyTxParam->SpatialReuse2        );
	PRINT_VARIABLE(pEtfPhyTxParam->SpatialReuse3        );
	PRINT_VARIABLE(pEtfPhyTxParam->SpatialReuse4        );
	PRINT_VARIABLE(pEtfPhyTxParam->TriggerResponding    );
	PRINT_VARIABLE(pEtfPhyTxParam->TriggerMethod        );
	PRINT_VARIABLE(pEtfPhyTxParam->BSSColor             );
	PRINT_VARIABLE(pEtfPhyTxParam->UplinkFlag           );
	PRINT_VARIABLE(pEtfPhyTxParam->ScramblerValueEn     );
	PRINT_VARIABLE(pEtfPhyTxParam->ScramblerValue       );
	PRINT_VARIABLE(pEtfPhyTxParam->LdpcExtrSysm         );
	PRINT_VARIABLE(pEtfPhyTxParam->ReservedForMAC       );
	PRINT_VARIABLE(pEtfPhyTxParam->CFO                  );
	PRINT_VARIABLE(pEtfPhyTxParam->PPM                  );
	PRINT_VARIABLE(pEtfPhyTxParam->DCM                  );
	PRINT_VARIABLE(pEtfPhyTxParam->Coding               );
	PRINT_VARIABLE(pEtfPhyTxParam->Padding              );
	PRINT_VARIABLE(pEtfPhyTxParam->AFactor              );
	PRINT_VARIABLE(pEtfPhyTxParam->PEDisambiguity       );
	PRINT_VARIABLE(pEtfPhyTxParam->MidamblePeriod       );
	PRINT_VARIABLE(pEtfPhyTxParam->BeamChange           );
	PRINT_VARIABLE(pEtfPhyTxParam->RuAllocation         );
	PRINT_VARIABLE(pEtfPhyTxParam->DefaultRFBW          );
	PRINT_VARIABLE(pEtfPhyTxParam->StationID0           );
}

int WiFiMode;
int OFDMMode;
int ChBW;
int RateIndex;
int g_PacketInterval = 16;
void atbm_etf_tx_rx_param_config(ETF_PHY_TX_PARAM_T *pParamTxRx, int channel, int mode, int rate, int chBW, int chOff, int ldpc)
{
	pParamTxRx->ChannelNum = channel;
	//0:DSSS,else OFDM
	pParamTxRx->WiFiMode = mode>0?ATBM_WIFI_MODE_OFDM:ATBM_WIFI_MODE_DSSS;
	switch(mode){
	case 0:
		break;
	case 1://11g
		pParamTxRx->OFDMMode = ATBM_WIFI_OFDM_MD_LM;
		break;
	case 2://11n
		pParamTxRx->OFDMMode = ATBM_WIFI_OFDM_MD_MM;
		break;
	case 3://11ax HE-SU
		pParamTxRx->OFDMMode = ATBM_WIFI_OFDM_MD_HE_SU;
		pParamTxRx->GIMode = ATBM_WIFI_GILTF_0P8_2X;
		break;
	case 4://11ax HE_ER_SU
		pParamTxRx->OFDMMode = ATBM_WIFI_OFDM_MD_HE_ER_SU;
		pParamTxRx->GIMode = ATBM_WIFI_GILTF_0P8_2X;
		break;
	default:
		pParamTxRx->OFDMMode = ATBM_WIFI_OFDM_MD_MM;
		break;
	};
	pParamTxRx->BW = chBW;//0:20M;1:40M;
	pParamTxRx->ChOffset = chOff;//0:ZERO;1:10U;2:10L;
	pParamTxRx->Rate = rate;		
	if((ldpc) || (rate >= 10))
		pParamTxRx->Coding = 1;//HE-SU MCS11 coding: 0:BCC;1:LDPC

	pParamTxRx->HESigA2Reserved = 0x03;
		
}

void RxResetCount(void)
{
	// TODO: 鍦ㄦ娣诲姞鎺т欢閫氱煡澶勭悊绋嬪簭浠ｇ爜	
	HW_WRITE_REG_BIT(0x0AC88334, 8, 8, 1);//params_rst_rnn_evm      334     8   AC88334 0       REG_CFG ro_evm reset
	HW_WRITE_REG_BIT(0x0AC88334, 8, 8, 0);

	HW_WRITE_REG_BIT(0x0AC882F4, 2, 2,  1); //ch_len_small_flag_stat_clr
	HW_WRITE_REG_BIT(0x0AC882F4, 2, 2, 0);

	HW_WRITE_REG_BIT(0x0ACD0020, 0, 0, 1); //params_rst_frame_num        0020        0   ACD0020 0
	HW_WRITE_REG_BIT(0x0ACD0020, 0, 0, 0);

	HW_WRITE_REG_BIT(0x0AC90298, 0, 0,  1); //params_rst_ro_sounding_counter      0298        0:0 AC90298
	HW_WRITE_REG_BIT(0x0AC90298, 0, 0, 0);
}
extern u32 chipversion;
extern u8 ETF_bStartTx;
extern u8 ETF_bStartRx;
extern char ch_and_type[20];


extern u8 CodeStart;
extern  u8 CodeEnd;
extern u8 ucWriteEfuseFlag;
extern int Atbm_Test_Success;
extern int atbm_test_rx_cnt;
extern int txevm_total;

int g_EtfRxMode = 0;

void SingleToneDisable(void)
{
	//force tx disable
	HW_WRITE_REG_BIT(0xACB8004, 21, 20, 0); 
	HW_WRITE_REG_BIT(0xACB8004, 16, 16, 0); 
	HW_WRITE_REG_BIT(0xACB8004, 10, 10, 0);
	HW_WRITE_REG_BIT(0xACB8900, 3, 3, 0);

	HW_WRITE_REG_BIT(0xACB8998, 1, 0, 0);   //params_operative_mode 

}

u32 MyRand(void)
{
	u32 random_num = 0;
	u32 randseed = 0;	

#if (LINUX_VERSION_CODE > KERNEL_VERSION(5, 0, 0))
	randseed = ktime_get_seconds();
#else
	struct timex txc;
	do_gettimeofday(&(txc.time));
	//randseed = jiffies;
	randseed = txc.time.tv_sec;
#endif
	random_num = randseed * 1103515245 + 12345;
	return ((random_num/65536)%32768);
}

int MacStringToHex(char *mac, u8  *umac)
{
	int i = 0, j = 0;
	unsigned char d = 0;
	char ch = 0,buffer[12] = {0};

	if(mac)
		memcpy(buffer, mac, strlen(mac));

    for (i=0;i<12;i++)
    {
        ch = buffer[i];

        if (ch >= '0' && ch <= '9')
        {
            d = (d<<4) | (ch - '0');
        }
        else if (ch >= 'a' && ch <= 'f')
        {
            d = (d<<4) | (ch - 'a' + 10);
        }
        else if (ch >= 'A' && ch <= 'F')
        {
            d = (d<<4) | (ch - 'A' + 10);
        }
		if((i%2 == 1)){
			umac[j++] = d;
			d = 0;
		}
    }

    return 0;
}
extern int atbm_direct_read_reg_32(struct atbm_common *hw_priv, u32 addr, u32 *val);
//get chip version funciton
u32 GetChipVersion(struct atbm_common *hw_priv)
{	
#ifndef SPI_BUS
	u32 uiRegData;
	atbm_direct_read_reg_32(hw_priv, CHIP_VERSION_REG, &uiRegData);
	//hw_priv->sbus_ops->sbus_read_sync(hw_priv->sbus_priv,CHIP_VERSION_REG,&uiRegData,4);	
	
	return uiRegData;
#else
	return 0;
#endif
}

void etf_PT_test_config(char *param)
{
	int Freq = 0;
	int txEvm = 0;
	int rxEvm = 0;
	int rxEvmthreshold = 0;
	int txEvmthreshold = 0;
	int Txpwrmax = 0;
	int Txpwrmin = 0;
	int Rxpwrmax = 0;
	int Rxpwrmin = 0;
	int rssifilter = 0;
	int cableloss = 0;
	int default_dcxo = 0;
	int noFreqCali = 0;
	char mac[12] = {0};
	int dcxo_max_min = 0;
	char *ptr = NULL;
	char const *pos = NULL;
	char const *pos_end = NULL;
	int ret = 0;
	int i   = 0;
	int len = 0;
	int data_len = 0;
	
	memset(&etf_config, 0, sizeof(struct etf_test_config));
	

	if(strstr(param, "cfg:"))
	{
		atbm_printk_always("<USE CONFIG FILE>\n");
		atbm_printk_always("param:%s\n", param);
		
		ptr = param;
		ptr += strlen("cfg:");
		data_len = strlen(param);
		
		ptr[data_len] = 0;
		
		for(i = 0;i<data_len;i++){
			if(ptr[i] == ',')
				ptr[i] = ATBM_SPACE;
		}
		
		pos = atbm_skip_space(ptr,data_len+1);

		if(pos == NULL){
			goto exit;
		}

		pos = atbm_skip_space(pos,data_len+1-(pos-ptr));

		if(pos == NULL){
			goto exit;
		}

		len = data_len + 1 - (pos - ptr);

		if(len <= 0){
			goto exit;
		}

		pos_end = memchr(pos,ATBM_TAIL,len);
		
		if(pos_end != NULL)
			len = pos_end - pos+1;

		/*
		*parase Freq
		*/
		ATBM_WEXT_PROCESS_PARAMS(pos,len,Freq,atbm_accsii_to_int,false,exit,ret);
		/*
		*parase txEvm
		*/
		ATBM_WEXT_PROCESS_PARAMS(pos,len,txEvm,atbm_accsii_to_int,false,exit,ret);	
		/*
		*parase rxEvm
		*/
		ATBM_WEXT_PROCESS_PARAMS(pos,len,rxEvm,atbm_accsii_to_int,false,exit,ret);	
		/*
		*parase txEvmthreshold
		*/
		ATBM_WEXT_PROCESS_PARAMS(pos,len,txEvmthreshold,atbm_accsii_to_int,false,exit,ret);	
		/*
		*parase rxEvmthreshold
		*/
		ATBM_WEXT_PROCESS_PARAMS(pos,len,rxEvmthreshold,atbm_accsii_to_int,false,exit,ret);	
		/*
		*parase Txpwrmax
		*/
		ATBM_WEXT_PROCESS_PARAMS(pos,len,Txpwrmax,atbm_accsii_to_int,false,exit,ret);	
		/*
		*parase Txpwrmin
		*/
		ATBM_WEXT_PROCESS_PARAMS(pos,len,Txpwrmin,atbm_accsii_to_int,false,exit,ret);	
		/*
		*parase Rxpwrmax
		*/
		ATBM_WEXT_PROCESS_PARAMS(pos,len,Rxpwrmax,atbm_accsii_to_int,false,exit,ret);	
		/*
		*parase Rxpwrmin
		*/
		ATBM_WEXT_PROCESS_PARAMS(pos,len,Rxpwrmin,atbm_accsii_to_int,false,exit,ret);	
		/*
		*parase rssifilter
		*/
		ATBM_WEXT_PROCESS_PARAMS(pos,len,rssifilter,atbm_accsii_to_int,false,exit,ret);	
		/*
		*parase cableloss
		*/
		ATBM_WEXT_PROCESS_PARAMS(pos,len,cableloss,atbm_accsii_to_int,false,exit,ret);	
		/*
		*parase default_dcxo
		*/
		ATBM_WEXT_PROCESS_PARAMS(pos,len,default_dcxo,atbm_accsii_to_int,false,exit,ret);	
		/*
		*parase noFreqCali
		*/
		ATBM_WEXT_PROCESS_PARAMS(pos,len,noFreqCali,atbm_accsii_to_int,false,exit,ret);	
		/*
		*parase dcxo_max_min
		*/
		ATBM_WEXT_PROCESS_PARAMS(pos,len,dcxo_max_min,atbm_accsii_to_int,false,exit,ret);	
		
		if(pos)
		{
			memcpy(mac, pos, sizeof(mac));
		}
		/*
		sscanf(param, "cfg:%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%s", 
			&Freq, &txEvm, &rxEvm, &txEvmthreshold,&rxEvmthreshold,&Txpwrmax, 
			&Txpwrmin, &Rxpwrmax, &Rxpwrmin, &rssifilter, &cableloss, &default_dcxo,&noFreqCali, &dcxo_max_min, mac);
		*/
		etf_config.freq_ppm = Freq;
		etf_config.txevm = (txEvm?txEvm:65536); //txevm filter
		etf_config.rxevm = (rxEvm?rxEvm:65536); //rxevm filter
		etf_config.txevmthreshold = txEvmthreshold;
		etf_config.rxevmthreshold = rxEvmthreshold;
		etf_config.txpwrmax = Txpwrmax;
		etf_config.txpwrmin = Txpwrmin;
		etf_config.rxpwrmax = Rxpwrmax;
		etf_config.rxpwrmin = Rxpwrmin;
		etf_config.rssifilter = rssifilter;
		etf_config.cableloss = (cableloss?cableloss:30)*4;	
		etf_config.default_dcxo = default_dcxo;
		etf_config.noFfreqCaliFalg = noFreqCali;
		dcxo_max_min &= 0xffff;
		etf_config.dcxo_code_min = dcxo_max_min & 0xff;
		etf_config.dcxo_code_max = (dcxo_max_min >> 8) & 0xff;

		if(etf_config.dcxo_code_min < DCXO_CODE_MAX)
			CodeStart = etf_config.dcxo_code_min;
		else
			CodeStart = DCXO_CODE_MINI;
		if((etf_config.dcxo_code_max > DCXO_CODE_MINI) && (etf_config.dcxo_code_max <= DCXO_CODE_MAX))
			CodeEnd = etf_config.dcxo_code_max;
		else
			CodeEnd = DCXO_CODE_MAX;
		
		if(strlen(mac) == 12){
			etf_config.writemacflag = 1;
			MacStringToHex(mac, etf_config.writemac);
		}
	}
	else
	{
		etf_config.freq_ppm = 7000;
		etf_config.rxevm = (rxEvm?rxEvm:65536);
		etf_config.rssifilter = -100;
		etf_config.txevm = (txEvm?txEvm:65536);
		etf_config.txevmthreshold = 400;
		etf_config.rxevmthreshold = 400;
		etf_config.cableloss = 30*4;
		CodeStart = DCXO_CODE_MINI;
		CodeEnd = DCXO_CODE_MAX;
	}
exit:
	etf_config.featureid = MyRand();
	atbm_printk_always("featureid:%d\n", etf_config.featureid);
	atbm_printk_always("Freq:%d,txEvm:%d,rxEvm:%d,txevmthreshold:%d,rxevmthreshold:%d,Txpwrmax:%d,Txpwrmin:%d,Rxpwrmax:%d,Rxpwrmin:%d,rssifilter:%d,cableloss:%d,default_dcxo:%d,noFreqCali:%d",
		etf_config.freq_ppm,etf_config.txevm,etf_config.rxevm,etf_config.txevmthreshold,etf_config.rxevmthreshold,
		etf_config.txpwrmax,etf_config.txpwrmin,etf_config.rxpwrmax,
		etf_config.rxpwrmin,etf_config.rssifilter,etf_config.cableloss,etf_config.default_dcxo,
		etf_config.noFfreqCaliFalg);
	atbm_printk_always("dcxomin:%d,dcxomax:%d", etf_config.dcxo_code_min, etf_config.dcxo_code_max);
	if(strlen(mac) == 12){
		atbm_printk_always("WRITE MAC:%02X%02X%02X%02X%02X%02X\n", 
					etf_config.writemac[0],etf_config.writemac[1],etf_config.writemac[2],
					etf_config.writemac[3],etf_config.writemac[4],etf_config.writemac[5]);
		}
	atbm_printk_always("\n");
}

extern int wsm_start_tx_v2(struct atbm_common *hw_priv, struct ieee80211_vif *vif );
int atbm_internal_start_tx(struct atbm_common *hw_priv,start_tx_param_t *tx_param)
{
	int i = 0;
	int channel = 0;
	int rateIdx = 0;
	int rateMin = 0;
	int rateMax = 0;
	int mode = 0;//0
	int bw = 0;
	int chOff = 0;
	int ldpc = 0;
	int packetLen = 0;
	int precom = 0;
	//int len = 0;
	int ret = 0;
	int etf_v2 = 0;
	struct atbm_vif *vif;
	char *threshold_param = NULL;

	ETF_HE_TX_CONFIG_REQ  EtfConfig;


	if(ETF_bStartTx || ETF_bStartRx){
		
		if(ETF_bStartTx){
			atbm_internal_stop_tx(hw_priv);
			msleep(500);
		}else{
			atbm_printk_err("Error! already start_tx, please stop_rx first!\n");
			return 0;
		}
		
	}
	
	if(tx_param == NULL){
		atbm_printk_err("atbm_internal_start_tx : tx_param is NULL \n");
		return -EINVAL;
	}
	/*
	*parase channel
	*/
	channel = tx_param->channel;
	if((channel < 0) || (channel > 14)){
		atbm_printk_err("atbm_internal_start_tx : channel[%d] err\n",channel);
		return -EINVAL;
	}
	/*
	*parase mode
	*/
	mode = tx_param->mode;
	if((mode < 0) || (mode > 3)){
		atbm_printk_err("atbm_internal_start_tx : mode[%d] err\n",mode);
		return -EINVAL;
	}
	/*
	*parase rateIdx
	*/
	rateIdx = tx_param->rate_id;
	switch(mode){
		case 0://DSSS
			rateMin = 0;
			rateMax = 3;
			break;
		case 1://LM
		case 2://MM
			rateMin = 0;
			rateMax = 7;
			break;
		case 3://HE-SU
			rateMin = 0;
			rateMax = 11;
			break;
		case 4://HE-ER_SU
			rateMin = 0;
			rateMax = 2;
			break;
		default:
			break;
	};
	if((rateIdx < rateMin) || (rateIdx > rateMax)){
		atbm_printk_err("atbm_internal_start_tx : rateIdx[%d] err,rateMin[%d],rateMax[%d]\n",rateIdx,rateMin,rateMax);
		return -EINVAL;
	}
	/*
	*parase bw,0:20M,1:40M;
	*/
	bw = tx_param->bw;
	if(bw == 1 && 
		(hw_priv->chip_version == CRONUS_NO_HT40 || hw_priv->chip_version == CRONUS_NO_HT40_LDPC)){
		atbm_printk_err("atbm_internal_start_tx : CHIP NOT SUPPORT HT40\n");
		return -EINVAL;
	}
	if((bw < 0) || (bw > 1)){
		atbm_printk_err("atbm_internal_start_tx : bw[%d] err\n",bw);
		return -EINVAL;
	}

	if((mode == 4) && (bw == 1)){
		atbm_printk_err("atbm_internal_start_tx : bw[%d] mode[%d]err\n",bw,mode);
		return -EINVAL;
	}

	/*
	*parase chOff,0:ZERO
	*/
	chOff = tx_param->chOff;
	if(chOff != 0){
		atbm_printk_err("atbm_internal_start_tx : chOff[%d] err\n",chOff);
		return -EINVAL;
	}

	/*
	*parase LDPC,0:BBC;1:LDPC
	*/
	ldpc = tx_param->ldpc;
	if(ldpc == 1 && hw_priv->chip_version == CRONUS_NO_HT40_LDPC){
		atbm_printk_err("atbm_internal_start_tx : CHIP NOT SUPPORT LDPC\n");
		return -EINVAL;
	}
	if((ldpc < 0) || (ldpc > 1)){
		atbm_printk_err("atbm_internal_start_tx : ldpc[%d] err\n",ldpc);
		return -EINVAL;
	}

	if((mode > 2) && (bw) && (ldpc == 0)){
		atbm_printk_err("atbm_internal_start_tx : ldpc[%d],bw[%d],mode[%d] err\n",ldpc,bw,mode);
		return -EINVAL;
	}

	/*
	*parase packetLen
	*/
	packetLen = tx_param->pktlen;
	if((packetLen < 0)){
		atbm_printk_err("atbm_internal_start_tx : packetLen[%d] err\n",packetLen);
		return -EINVAL;
	}

	/*
	*parase precom;only channel1 20M used
	*/
	precom = tx_param->precom;
	if((precom < 0) || (precom > 4)){
		atbm_printk_err("atbm_internal_start_tx : precom[%d] err\n",precom);
		return -EINVAL;
	}

	//if((channel != 1) || (bw != 0))
		//precom = 0;
	
	threshold_param = tx_param->threshold_param;
	
	
	/**********************************************************************
	if etf_v2 =1 ,PRODUCT TEST with "golden WIFI". used scan send probe req
		: len == 99999 is PRODUCT TEST result need write efuse
		: len == 99998 is PRODUCT TEST result not write efuse
	else etf_v2 =0, RF test 锟斤�?used phy TX
	**********************************************************************/
	if(packetLen == 99999){
		ucWriteEfuseFlag = 1;
		etf_v2 = 1;	
		hw_priv->etf_len = 1000; 
	}
	else if(packetLen == 99998)
	{
		ucWriteEfuseFlag = 0;
		etf_v2 = 1;	
		hw_priv->etf_len = 1000; 
	}
	else if(packetLen == 0){
	//Prevent USB from being unplugged suddenly in product testing
	//11b 100% duty cycle
		packetLen = 1024;
	}
	
	/**********************************************************************
	if etf_v2 =1 ,PRODUCT TEST with "golden WIFI". used scan send probe req
	else etf_v2 =0, RF test used phy TX
	**********************************************************************/
	if(etf_v2)
	{	
		//etf_v2 will auto stop not need call atbm_ioctl_stop_tx
		hw_priv->etf_channel = channel;
		hw_priv->etf_channel_type = NL80211_CHAN_HT20;
		hw_priv->etf_rate = WSM_TRANSMIT_RATE_HT_65;
		hw_priv->etf_greedfiled = 0;
		
		atbm_for_each_vif(hw_priv,vif,i){
			if((vif != NULL)){
				atbm_printk_wext("Product Test\n");
				down(&hw_priv->scan.lock);
				mutex_lock(&hw_priv->conf_mutex);
				
				atbm_test_rx_cnt = 0;
				txevm_total = 0;
				hw_priv->bStartTx = 1;
				hw_priv->bStartTxWantCancel = 1;
				hw_priv->etf_test_v2 =1;
				etf_PT_test_config(threshold_param);
				if(chipversion == 0x49)
					GetChipCrystalType(hw_priv);
			
				if(wsm_start_tx_v2(hw_priv, vif->vif) != 0)
				{
					up(&hw_priv->scan.lock);
					atbm_printk_err("%s:%d,wsm_start_tx_v2 error\n", __func__, __LINE__);
				}
				
				mutex_unlock(&hw_priv->conf_mutex);
				break;
			}
		}
	}
	else {		
		atbm_printk_always("**ETF_PHY_Start_Tx***\n");
		
		ETF_PHY_TxRxParamInit(&EtfConfig);

		atbm_etf_tx_rx_param_config(&EtfConfig.TxConfig, channel, mode, rateIdx, bw, chOff, ldpc);
		EtfConfig.TxConfig.PSDULen = packetLen;
		EtfConfig.TxConfig.precompensation = precom;
		EtfConfig.TxConfig.PacketInterval = g_PacketInterval;
		hw_priv->etf_channel = channel;
		if(bw == 1)
		{
			//hw_priv->etf_channel = channel + 2;
			hw_priv->etf_channel_type = NL80211_CHAN_HT40MINUS;
		}
		else
		{
			//lmac:0:zero;1:invalid;2:10U;3:10L
			//hamc:0:zero;1:10U;2:10L
			hw_priv->etf_channel_type = chOff?(chOff+1):(chOff);
		}


		atbm_for_each_vif(hw_priv,vif,i){
			if((vif != NULL)){
				//down(&hw_priv->scan.lock);
				mutex_lock(&hw_priv->conf_mutex);
				ret = atbm_set_channel(hw_priv, BIT(WSM_SET_CHANTYPE_FLAGS__ETF_TEST_START));//set channel and start TPC
				if(ret != 0)
				{
					//up(&hw_priv->scan.lock);
					atbm_printk_err("atbm_set_channel err,ret:%d\n", ret);
					//goto exit;
				}
				mutex_unlock(&hw_priv->conf_mutex);
				break;
			}
		}
		

		if(precom == 1)
		{
			//lmac config rfpll reg 0x0acc0288 and reduce 1dB
			WARN_ON(wsm_write_mib(hw_priv, WSM_MIB_ID_SET_PRE_COMPENSATION,
						&precom, sizeof(precom), vif->if_id));
		}

		mutex_lock(&hw_priv->conf_mutex);				
		ETF_bStartTx = 1;
		mutex_unlock(&hw_priv->conf_mutex);

		//ETFTxConfigShow(&EtfConfig.TxConfig);
		
		ETF_PHY_Start_Tx_Step1(&EtfConfig);
		ETF_PHY_Start_Tx_Step2(&EtfConfig);

		WiFiMode = EtfConfig.TxConfig.WiFiMode;
		OFDMMode = EtfConfig.TxConfig.OFDMMode;
		ChBW = EtfConfig.TxConfig.BW;
		RateIndex = EtfConfig.TxConfig.Rate;
	}

	return 0;
}
	
int atbm_internal_stop_tx(struct atbm_common *hw_priv)
{
	struct atbm_vif *vif;
	int i = 0,ret = 0;

	
	if(0 == ETF_bStartTx){
		atbm_printk_err("please start start_tx first,then stop_tx\n");
		return -EINVAL;
	}

	

	hw_priv->etf_channel = 7;
	hw_priv->etf_channel_type = 0;

	atbm_for_each_vif(hw_priv,vif,i){
			if((vif != NULL)){
				mutex_lock(&hw_priv->conf_mutex);
				ret = atbm_set_channel(hw_priv, 0);//set channel and start TPC
				if(ret != 0)
				{
					atbm_printk_err("atbm_set_channel err,ret:%d\n", ret);
				}
				mutex_unlock(&hw_priv->conf_mutex);
				break;
			}
		}
	g_PacketInterval = 16;//default 16us
	if(ETF_bStartTx == 1)
	{
		ETF_PHY_Stop_Tx(NULL);
		atbm_printk_err("stop_tx\n");
	}
   	else if(ETF_bStartTx == 2)
	{
		SingleToneDisable();
		atbm_printk_err("stop singletone\n");
	}

	mutex_lock(&hw_priv->conf_mutex);
	ETF_bStartTx = 0;
	mutex_unlock(&hw_priv->conf_mutex);

	return ret;
}

int atbm_internal_start_rx(struct atbm_common *hw_priv,start_rx_param_t *rx_param)
{
	int i = 0;
	int ret = 0;
	int bw = 0;
	int chOff = 0;
	int channel = 0;
	int mode = 0;
	u8 ucDbgPrintOpenFlag = 1;
	struct atbm_vif *vif;
	ETF_HE_RX_CONFIG_REQ  EtfConfig;

	
	if(ETF_bStartTx || ETF_bStartRx){
		if(ETF_bStartRx){
			atbm_internal_stop_rx(hw_priv,NULL);
			msleep(500);
		}else{
			atbm_printk_err("Error! already start_rx, please stop_tx first!\n");
			return 0;
		}
	}
	/*
	*parase channel
	*/
	channel = rx_param->channel;
	if((channel < 0) || (channel > 14)){
		atbm_printk_err("atbm_internal_start_rx : channel[%d] err ! \n",channel);
		return -EINVAL;
	}
	/*
	*parase bw:0:20M;1:40M;2:RU242;3:RU106;
	*/
	bw = rx_param->bw;
	if(bw == 1 && 
		(hw_priv->chip_version == CRONUS_NO_HT40 || hw_priv->chip_version == CRONUS_NO_HT40_LDPC)){
		atbm_printk_err("atbm_internal_start_tx : CHIP NOT SUPPORT HT40\n");
		return -EINVAL;
	}
		
	if((bw < 0) || (bw > 1)){
		atbm_printk_err("atbm_internal_start_rx : bw[%d] err ! \n",bw);
		return -EINVAL;
	}
	/*
	*parase chOff:0:zero;1:10U;2:10L;
	*/
	chOff = rx_param->chOff;
	if((chOff < 0) || (chOff > 2)){
		atbm_printk_err("atbm_internal_start_rx : chOff[%d] err ! \n",chOff);
		return -EINVAL;
	}

	/*
	*parase mode:0:ofdm;1:dsss
	*/
	mode= rx_param->mode;
	g_EtfRxMode = rx_param->mode;
	if((mode < 0) || (mode > 2)){
		atbm_printk_err("atbm_internal_start_rx : mode[%d] err ! \n",mode);
		return -EINVAL;
	}

	if((bw == 0) && (chOff != 0))
	{
		atbm_printk_err("Invalid chOffset!chOff[0]\n");
		return -EINVAL;
	}

	if((bw == 1) && (chOff == 0))
	{
		atbm_printk_err("Invalid chOffset!chOff[1,2]\n");
		return -EINVAL;
	}



	
	//open lmac print
	atbm_for_each_vif(hw_priv,vif,i){
		if (vif != NULL)
		{
			WARN_ON(wsm_write_mib(hw_priv, WSM_MIB_ID_DBG_PRINT_TO_HOST,
				&ucDbgPrintOpenFlag, sizeof(ucDbgPrintOpenFlag), vif->if_id));
			break;
		}
	}

	//bw: 0:20M;1:40M_10U;2:40M_10L
	hw_priv->etf_channel = channel;
	//lmac:0:zero;1:invalid;2:10U;3:10L
	//hamc:0:zero;1:10U;2:10L
	hw_priv->etf_channel_type = chOff?(chOff+1):(chOff);

	hw_priv->etf_channel_type |= (0xa << 16);//use rx flag start rx status timer in lmac
	if(mode == 0)
		hw_priv->etf_channel_type |= (0xb << 24);//use rx flag start rx counter
	else if(mode == 1)
		hw_priv->etf_channel_type |= (0xc << 24);//use rx flag start rx counter
	atbm_for_each_vif(hw_priv,vif,i){
			if((vif != NULL)){
				//down(&hw_priv->scan.lock);
				mutex_lock(&hw_priv->conf_mutex);
				ret = atbm_set_channel(hw_priv, BIT(WSM_SET_CHANTYPE_FLAGS__ETF_TEST_START));//set channel and start TPC
				if(ret != 0)
				{
					//up(&hw_priv->scan.lock);
					atbm_printk_err("atbm_set_channel err,ret:%d\n", ret);
					//goto exit;
				}
				mutex_unlock(&hw_priv->conf_mutex);
				break;
			}
		}

	ETF_bStartRx = 1;
	
	ETF_PHY_TxRxParamInit((ETF_HE_TX_CONFIG_REQ*)&EtfConfig);
	EtfConfig.RxConfig.ChannelNum = channel;
	EtfConfig.RxConfig.BW = bw;
	EtfConfig.RxConfig.ChOffset = chOff;
	
	ETF_PHY_Start_Rx(&EtfConfig);

	return ret;

}

int atbm_internal_stop_rx(struct atbm_common *hw_priv,get_result_rx_data *rx_data)
{
	int i = 0;
	int ret = 0;
	u8 ucDbgPrintOpenFlag = 0;
	struct atbm_vif *vif;

	if((0 == ETF_bStartRx)){
		atbm_printk_err("please start start_rx first,then stop_rx\n");
		return -EINVAL;
	}

	//close lmac print
	atbm_for_each_vif(hw_priv,vif,i){
		if (vif != NULL)
		{
			WARN_ON(wsm_write_mib(hw_priv, WSM_MIB_ID_DBG_PRINT_TO_HOST,
				&ucDbgPrintOpenFlag, sizeof(ucDbgPrintOpenFlag), vif->if_id));
			break;
		}
	}

	atbm_for_each_vif(hw_priv,vif,i){
			if((vif != NULL)){
				//down(&hw_priv->scan.lock);
				mutex_lock(&hw_priv->conf_mutex);
				ret = atbm_set_channel(hw_priv, 0);//clear lmac etf flag and cancel rx status timer
				if(ret != 0)
				{
					//up(&hw_priv->scan.lock);
					atbm_printk_err("atbm_set_channel err,ret:%d\n", ret);
					//goto exit;
				}
				mutex_unlock(&hw_priv->conf_mutex);
				break;
			}
		}
	
	ETF_bStartRx = 0;


	{
		u32 rxTotal = 0;
		u32 rxSuccess = 0;
		u32 rxError = 0;

		if(g_EtfRxMode == 0)
		{
			//ofdm
			rxTotal = HW_READ_REG(0x0ACD002C);
			rxError = HW_READ_REG(0x0ACD0024);
		}
		else if(g_EtfRxMode == 1)
		{
			//dsss
			rxTotal = HW_READ_REG(0x0ACD012C);
			rxError = HW_READ_REG(0x0ACD0130);
		}
		else
		{
			rxTotal = HW_READ_REG(0x0ACD002C) + HW_READ_REG(0x0ACD012C);
			rxError = HW_READ_REG(0x0ACD0024) + HW_READ_REG(0x0ACD0130);
		}


		rxSuccess = rxTotal - rxError;
		if(rx_data){
			rx_data->rxError = rxError;
			rx_data->rxSuccess = rxSuccess;
		}
		atbm_printk_always("rxSuc:%d,Err:%d\n", rxSuccess,  rxError);
	}

	RxResetCount();//clear rx statstics
	ETF_PHY_Stop_Rx(NULL);

	return ret;
}

void etf_set_deltagain(struct efuse_headr efuse)
{
	int delta_gain = 0;

	delta_gain = efuse.delta_gain1==16?0:efuse.delta_gain1;
	delta_gain = _5bitTo32bit(delta_gain);
	HW_WRITE_REG_BIT(0xACB8B28, 6, 0, delta_gain);
	delta_gain = efuse.delta_gain2==16?0:efuse.delta_gain2;
	delta_gain = _5bitTo32bit(delta_gain);
	HW_WRITE_REG_BIT(0xACB8B28, 13, 7, delta_gain);
	delta_gain = efuse.delta_gain3==16?0:efuse.delta_gain3;
	delta_gain = _5bitTo32bit(delta_gain);
	HW_WRITE_REG_BIT(0xACB8B28, 20, 14, delta_gain);
}

void get_reg_deltagain(struct efuse_headr *efuse)
{
	int delta_gain = 0;
	if(efuse == NULL){
		atbm_printk_err("get_reg_deltagain : param is NULL");
		return;
	}
	
	delta_gain = HW_READ_REG_BIT(0xACB8B28, 6, 0);
	_32bitTo5bit(delta_gain);
	efuse->delta_gain1 = delta_gain;

	delta_gain = HW_READ_REG_BIT(0xACB8B28, 13, 7);
	_32bitTo5bit(delta_gain);
	efuse->delta_gain2 = delta_gain;

	delta_gain = HW_READ_REG_BIT(0xACB8B28, 20, 14);
	_32bitTo5bit(delta_gain);
	efuse->delta_gain3 = delta_gain;
}




u8 DSSS_Index[] = {0, 1};

u8 OFDM_LM_MM_Index[] = {2, 3, 4, 5, 6, 7, 8, 9, 10};

u8 OFDM_SU_ER_SU_Index[] = {11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22};

u8 OFDM_TB_Index[] = {24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35};

int Get_MCS_LUT_Offset_Index(int WiFiMode,int OFDMMode,int ChBW,int RateIndex)
{
	int offsetIndex = 0;

	if(WiFiMode == ATBM_WIFI_MODE_DSSS)
	{
		if(RateIndex < 2)
			offsetIndex = DSSS_Index[0];//1M/2M
		else
			offsetIndex = DSSS_Index[1];//5.5M/11M
	}
	else
	{
		if(OFDMMode == ATBM_WIFI_OFDM_MD_LM)
			offsetIndex = OFDM_LM_MM_Index[RateIndex];//6M - 54M
		else if(OFDMMode == ATBM_WIFI_OFDM_MD_MM)
		{
			if((RateIndex == 0) || (RateIndex == 8))
				offsetIndex = OFDM_LM_MM_Index[RateIndex];//mcs0/mcs32
			else
				offsetIndex = OFDM_LM_MM_Index[RateIndex+1];//mcs1~mcs7
		}
		else if(OFDMMode == ATBM_WIFI_OFDM_MD_HE_SU)
			offsetIndex = OFDM_SU_ER_SU_Index[RateIndex];//mcs0 - mcs11
		else if(OFDMMode == ATBM_WIFI_OFDM_MD_HE_ER_SU)
		{
			if(RateIndex < 3)
				offsetIndex = OFDM_SU_ER_SU_Index[RateIndex];//mcs0/mcs1/mcs2
			else
				atbm_printk_always("HE_ER_SU only support MCS0/MCS1/MCS2\n");
		}
		else if(OFDMMode == ATBM_WIFI_OFDM_MD_HE_TB)
			offsetIndex = OFDM_TB_Index[RateIndex];//mcs0 - mcs11
	}

	if(ChBW == ATBM_WIFI_BW_40M)
		offsetIndex += 36;

	return offsetIndex;
}




/***********************************************************
* Get register address of MCS LUT by the index of control on Tx UI
*
***********************************************************/
unsigned int Get_MCS_LUT_Addr_ByTxUICtrlIndex(int WiFiModeCtrlIndex,int OfdmModeCtrlIndex,int ChBwCtrlIndex,int RateCtrlIndex)
{
	int offsetIndex = 0;
	u32 mcsLUTAddr = 0xACBE010;//mcs LUT base addr

	offsetIndex = Get_MCS_LUT_Offset_Index(WiFiModeCtrlIndex,OfdmModeCtrlIndex,ChBwCtrlIndex,RateCtrlIndex);
	mcsLUTAddr += (offsetIndex << 2);
	return mcsLUTAddr;
}
unsigned int Get_MCS_LUT_Addr(int wifi_mode,int ofdm_mode, int ch_bw, int rate_index)
{
    return Get_MCS_LUT_Addr_ByTxUICtrlIndex(wifi_mode,ofdm_mode,ch_bw,rate_index);
}

void set_power_by_mode(int wifi_mode, int ofdm_mode, int bw, int rateIndex, int delfault_power, int power_delta, int powerTarFlag)
{
	u32 mcsLUTAddr = 0;
	int power_current = 0;
	int power_tar = 0;
	//int index = 0;

	mcsLUTAddr = Get_MCS_LUT_Addr(wifi_mode, ofdm_mode, bw, rateIndex);

	if(0 == powerTarFlag)
	{
		power_current = delfault_power;//HW_READ_REG_BIT(mcsLUTAddr, 7, 0);//read current power
		power_tar = power_current + power_delta;
	}
	else
		power_tar = power_delta;
	//atbm_printk_always("0x%x:%x\n", mcsLUTAddr, power_tar);
	
	//atbm_printk_always("mcsLUTAddr:0x%x:%d,%d,%d\n", mcsLUTAddr, delfault_power>>2, power_delta>>2, power_tar>>2);

	if(power_tar > 23*4)
		power_tar = 23*4;

	HW_WRITE_REG_BIT(mcsLUTAddr, 7, 0, (u32)power_tar);//function register
}
void set_power_by_bandwidth(int bw, u32 addrOffset, int delfault_power, int power_delta)
{
	//int i = 0;
	u32 basicAddr = 0;
	u32 mcsLUTAddr = 0;
	int power_current = 0;
	int power_tar = 0;

	if(bw == 0)
		basicAddr = 0xACBE010;//20M
	else
		basicAddr = 0xACBE0A0;//40M

	//for(i=0;i<36;i++)
	{
		mcsLUTAddr = basicAddr + addrOffset;
			
		power_current = delfault_power;//HW_READ_REG_BIT(mcsLUTAddr, 7, 0);//read current power
		power_tar = power_current + power_delta;

		if(power_tar > 23*4)
			power_tar = 23*4;

		//atbm_printk_always("mcsLUTAddr:0x%x:%d,%d,%d\n", mcsLUTAddr, delfault_power>>2, power_delta>>2, power_tar>>2);
		HW_WRITE_REG_BIT(mcsLUTAddr, 7, 0, (u32)power_tar);//function register
	}
	
}

void get_power_by_bandwidth(int bw)
{
	int i = 0;
	u32 basicAddr = 0;
	u32 mcsLUTAddr = 0;
	int power_tar = 0;

	if(bw == 0)
		basicAddr = 0xACBE010;//20M
	else
		basicAddr = 0xACBE0A0;//40M

	for(i=0;i<36;i++)
	{
		mcsLUTAddr = basicAddr + (i << 2);
			
		power_tar = HW_READ_REG_BIT(mcsLUTAddr, 7, 0);//read current power
		atbm_printk_always("0x%x:%x\n", mcsLUTAddr, power_tar);
	}
	
}
extern int rate_txpower_cfg[64];
int atbm_internat_cmd_get_cfg_txpower(struct atbm_common *hw_priv,char **results)
{
	int i = 0;
	//char *results = NULL;
	int total_len = 0,copy_len = 0;
	struct efuse_headr efuse = {0};
	int powerTarFlag = 0;
	//struct cfg_txpower_t configured_txpower;
	const char *ratebuf[] = {"b_1M_2M",
								"b_5_5M_11M",
								"g_6M_n_6_5M",
								"g_9M",
								"g_12M_n_13M",
								"g_18M_n_19_5M",
								"g_24M_n_26M",
								"g_36M_n_39M",
								"g_48M_n_52M",
								"g_54M_n_58_5M",
								"n_65M",
								"HE-SU_MCS0",
								"HE-SU_MCS1",
								"HE-SU_MCS2",
								"HE-SU_MCS3",
								"HE-SU_MCS4",
								"HE-SU_MCS5",
								"HE-SU_MCS6",
								"HE-SU_MCS7",
								"HE-SU_MCS8",
								"HE-SU_MCS9",
								"HE-SU_MCS10",
								"HE-SU_MCS11",};

	//memset(&configured_txpower,0, sizeof(configured_txpower));
	
	if(*results){
		atbm_printk_err("%s :results is not NULL!!!\n",__func__);
		return -1;
	}
	*results = atbm_kzalloc(1024,GFP_KERNEL);
	
	if(*results == NULL){
		atbm_printk_always("%s , no memory!! \n",__func__);
		return -ENOMEM;

	}
	powerTarFlag = rate_txpower_cfg[0];
	if(powerTarFlag){

		atbm_printk_cfg("set absolute power\n");
		copy_len = scnprintf(*results+total_len,1024-total_len,"set absolute power\n");
	}else{
		copy_len = scnprintf(*results+total_len,1024-total_len,"set delta power\n");
		atbm_printk_cfg("set delta power\n");
	}
	total_len += copy_len;

	for(i=1;i<=44;i++)
	{
		if(i <= 23){
			atbm_printk_cfg("%s=%d\n", ratebuf[i-1], rate_txpower_cfg[i]);
			copy_len = scnprintf(*results+total_len,1024-total_len,"%s=%d\n", ratebuf[i-1], rate_txpower_cfg[i]);
		}else{
			atbm_printk_cfg("%s_40M=%d\n", ratebuf[i-22], rate_txpower_cfg[i]);
			copy_len = scnprintf(*results+total_len,1024-total_len,"%s_40M=%d\n", ratebuf[i-22], rate_txpower_cfg[i]);
		
		}
		if(copy_len > 0)
			total_len += copy_len;
		else
			break;
	}

	if(hw_priv){
		get_reg_deltagain(&efuse);
		
		efuse.dcxo_trim = DCXOCodeRead(hw_priv);
		
		copy_len = scnprintf(*results+total_len,1024-total_len,
			"\n\ndelta_gain1:%d  delta_gain2:%d  delta_gain3:%d  dcxo:%d\n",
			efuse.delta_gain1,efuse.delta_gain2,efuse.delta_gain3,efuse.dcxo_trim);
			
		if(copy_len > 0)
			total_len += copy_len;
		else
			atbm_printk_always("get deltagain dcxo fail! \n");
	}else{
	
		copy_len = scnprintf(*results+total_len,1024-total_len,"\n\n hw_priv is NULL,efuse get fail!\n");
		if(copy_len > 0)
			total_len += copy_len;
		else
			atbm_printk_always("hw_priv is NULL,get deltagain dcxo fail! \n");
	}
	
	return total_len;
}
extern const u8 MCS_LUT_20M[36];
extern u8 MCS_LUT_20M_Modify[36] ;
extern s8 MCS_LUT_20M_delta_mode[36];
extern s8 MCS_LUT_20M_delta_bw[36];
extern const char *rate_name[36];
extern const u8 MCS_LUT_40M[36];
extern u8 MCS_LUT_40M_Modify[36] ;

extern s8 MCS_LUT_40M_delta_mode[36];
extern s8 MCS_LUT_40M_delta_bw[36];

int atbm_internat_cmd_get_txpower(struct atbm_common *hw_priv,char **results)
{
	//int ret = 0;
	int i = 0;
	int power_current = 0;
	u32 mcsLUTAddr = 0xACBE010;
	int delta_power = 0;
	int total_len = 0,copy_len = 0;

	
	if(results != NULL){
		if(*results){
			atbm_printk_err("%s :results is not NULL!!!\n",__func__);
			return -1;
		}
		*results = atbm_kzalloc(4096,GFP_KERNEL);
		if(*results == NULL){
			atbm_printk_always("%s , no memory!! \n",__func__);
			return -ENOMEM;
		}
	}
	
	//20M
	if(results){
		copy_len = scnprintf(*results+total_len,4096-total_len,"20M:\n");
		if(copy_len > 0)
			total_len += copy_len;
		else{
			atbm_printk_err("%s %d : overflow!!\n",__func__,__LINE__);
			return total_len;
		}
	}else
		atbm_printk_always("20M:\n");
		
	for(i=0;i<sizeof(MCS_LUT_20M)/sizeof(u8);i++)
	{
		power_current = HW_READ_REG_BIT(mcsLUTAddr+(i<<2), 7, 0);//read current power
		delta_power = power_current - MCS_LUT_20M[i];
		//
		if(results){
			copy_len = scnprintf(*results+total_len,4096-total_len,"%s:power:[%d]dBm mode:[%d]dB bw:[%d]dB\n", rate_name[i], MCS_LUT_20M_Modify[i]>>2, 
				MCS_LUT_20M_delta_mode[i]>>2, MCS_LUT_20M_delta_bw[i]>>2);
			if(copy_len > 0)
				total_len += copy_len;
			else{
				atbm_printk_err("%s %d : overflow!!\n",__func__,__LINE__);
				return total_len;
			}
		}else{
			atbm_printk_always("%s:power:[%d]dBm mode:[%d]dB bw:[%d]dB\n", rate_name[i], MCS_LUT_20M_Modify[i]>>2, 
			MCS_LUT_20M_delta_mode[i]>>2, MCS_LUT_20M_delta_bw[i]>>2);
		}
		
	}
	
	//40M
	//atbm_printk_always("\n40M:\n");
	if(results){
		copy_len = scnprintf(*results+total_len,4096-total_len,"\n40M:\n");
		if(copy_len > 0)
				total_len += copy_len;
		else{
			atbm_printk_err("%s %d : overflow!!\n",__func__,__LINE__);
			return total_len;
		}

	}else
		atbm_printk_always("\n40M:\n");
	for(i=2;i<sizeof(MCS_LUT_40M)/sizeof(u8);i++)
	{
		power_current = HW_READ_REG_BIT(mcsLUTAddr+0x90+(i<<2), 7, 0);//read current power
		delta_power = power_current - MCS_LUT_40M[i];
		//
		if(results){
			copy_len = scnprintf(*results+total_len,4096-total_len,"%s:power:[%d]dBm mode:[%d]dB bw:[%d]dB\n", rate_name[i], MCS_LUT_40M_Modify[i]>>2, 
				MCS_LUT_40M_delta_mode[i]>>2, MCS_LUT_40M_delta_bw[i]>>2);
			if(copy_len > 0)
				total_len += copy_len;
			else{
				atbm_printk_err("%s %d : overflow!!\n",__func__,__LINE__);
				return total_len;
			}
		}else{
			atbm_printk_always("%s:power:[%d]dBm mode:[%d]dB bw:[%d]dB\n", rate_name[i], MCS_LUT_40M_Modify[i]>>2, 
			MCS_LUT_40M_delta_mode[i]>>2, MCS_LUT_40M_delta_bw[i]>>2);
		}
	}

	
	return total_len;

}



#if 0//#ifdef CONFIG_ATBM_BLE
#include <linux/cdev.h>

#include <net/ndisc.h>

#define MAX_STATUS_SYNC_LSIT_CNT  		(32)


struct atbm_info {
	dev_t devid;
	struct cdev *my_cdev;
	struct class *my_class;
	struct device *my_device;
	struct atbm_common *hw_priv;
};

struct atbm_status_event {
	struct list_head link;
	struct ioctl_status_async status;
};

struct atbm_info atbm_info;
static struct fasync_struct *connect_async;
static spinlock_t s_status_queue_lock;
static struct list_head s_status_head;
static int s_cur_status_list_cnt = 0;
static u8 atbm_ioctl_data[2048];
extern char* ieee80211_ble_commb_ble_alloc_xmit(size_t len);
extern int ieee80211_ble_commb_xmit(struct ieee80211_local* local, u8* xmit, size_t xmit_len);
extern int ieee80211_ble_commb_start(struct ieee80211_local* local);
extern  int ieee80211_ble_commb_subscribe(struct ieee80211_local* local,
	int (*recv)(struct ieee80211_local* local, u8* recv, size_t recv_len, enum ieee80211_ble_msg_type msg_type));
int atbm_ble_dev_rx(struct ieee80211_local* local, u8* event_buffer, size_t event_len, enum ieee80211_ble_msg_type msg_type);
extern int ieee80211_ble_commb_unsubscribe(struct ieee80211_local* local);
extern int ieee80211_ble_commb_stop(struct ieee80211_local* local);
extern struct atbm_common* g_hw_priv;

#ifdef CONFIG_ATBM_BLE_ADV_COEXIST

int atbm_ioctl_ble_adv_coexit_start(u8* data)
{
	struct ioctl_ble_start* ble_start = (struct ioctl_ble_start*)data;
	struct wsm_ble_msg_coex_start ble_coex = { 0 };

	if ((ble_start->ble_adv == 0) && (ble_start->ble_scan == 0)) {
		atbm_printk_err("both adv and scan is close!\n");
		return -1;
	}

	if ((ble_start->ble_scan) && (ble_start->ble_scan_win == 0)) {
		atbm_printk_err("ble scan enable, but scan_win is 0!\n");
		return -1;
	}

	if ((ble_start->ble_adv_chan != 0) && (ble_start->ble_adv_chan >= 37)
		&& (ble_start->ble_adv_chan <= 39)) {
		ble_coex.chan_flag |= BIT(ble_start->ble_adv_chan - 37);
	}

	if ((ble_start->ble_scan_chan != 0) && (ble_start->ble_scan_chan >= 37)
		&& (ble_start->ble_scan_chan <= 39)) {
		ble_coex.chan_flag |= BIT(ble_start->ble_scan_chan - 37 + 3);
	}

	if (ble_start->ble_adv) {
		ble_coex.coex_flag |= BIT(0);
	}

	if (ble_start->ble_scan) {
		ble_coex.coex_flag |= BIT(1);
	}

	ble_coex.interval = ble_start->ble_interval;
	ble_coex.scan_win = ble_start->ble_scan_win;
	ble_coex.ble_id = BLE_MSG_COEXIST_START;
	atbm_printk_init("atbm_ioctl_ble_adv_coexit_start\n");
	return wsm_ble_msg_coexist_start(atbm_info.hw_priv, &ble_coex, 0);
}

int atbm_ioctl_ble_adv_coexit_stop(u8* data)
{
	struct wsm_ble_msg ble_coex = { 0 };
	ble_coex.ble_id = BLE_MSG_COEXIST_STOP;
	return wsm_ble_msg_coexist_stop(atbm_info.hw_priv, &ble_coex, 0);
}
int atbm_ioctl_ble_set_adv_data(u8* data)
{
	struct ioctl_ble_adv_data* adv_data = (struct ioctl_ble_adv_data*)data;
	struct wsm_ble_msg_adv_data ble_adv_data = { 0 };

	memcpy(&ble_adv_data.mac[0], adv_data, sizeof(struct ioctl_ble_adv_data));
	ble_adv_data.ble_id = BLE_MSG_SET_ADV_DATA;
	return wsm_ble_msg_set_adv_data(atbm_info.hw_priv, &ble_adv_data, 0);
}

int atbm_ioctl_ble_adv_resp_start(u8* data)
{
	struct ioctl_ble_adv_resp_start* ble_start = (struct ioctl_ble_adv_resp_start*)data;
	struct wsm_ble_msg_adv_resp_start ble_adv_resp_msg = { 0 };


	ble_adv_resp_msg.interval = ble_start->ble_interval;
	ble_adv_resp_msg.ble_id = BLE_MSG_ADV_RESP_MODE_START;
	return wsm_ble_msg_set_adv_data(atbm_info.hw_priv, (struct wsm_ble_msg_adv_data*)&ble_adv_resp_msg, 0);
}

int atbm_ioctl_ble_set_resp_data(u8* data)
{
	struct ioctl_ble_resp_data* resp_data = (struct ioctl_ble_resp_data*)data;
	struct wsm_ble_msg_resp_data ble_resp_data = { 0 };

	memcpy(&ble_resp_data.resp_data_len, resp_data, sizeof(struct ioctl_ble_resp_data));
	ble_resp_data.ble_id = BLE_MSG_SET_RESP_DATA;
	return wsm_ble_msg_set_adv_data(atbm_info.hw_priv, (struct wsm_ble_msg_adv_data*)&ble_resp_data, 0);
}
#endif//#ifdefCONFIG_ATBM_BLE_ADV_COEXISTstatic int atbm_ioctl_notify_add(u8 type, u8 driver_mode, u8 *event_buffer, u16 event_len)
#if 0
void* dump_mem(void * data, int len)
{
	int i, j;
	int size;
	u8 *tem = (u8 *)data ;
	u8 buf[64];

	size = len;
	while(size > 0){
		
		memset(buf, 0 ,sizeof(buf));
		j =  (size - 16) > 0? 16: size ;
		for(i = 0; i < j ; i++)
			sprintf(&buf[3*i], "%02x ", tem[i]);
		atbm_printk_err("%s \n",buf);
		size -= 16;
		tem +=  16;
	}
}
#endif
#ifdef BLUEDROID
#define QUEUE_SIZE 500
static wait_queue_head_t bt_read_wait;

static spinlock_t queue_lock;
static struct sk_buff *rx_skb_queue[QUEUE_SIZE];
static int rx_skb_queue_front = 0;
static int rx_skb_queue_rear = 0;

static void rx_enqueue(struct sk_buff *skb)
{
    spin_lock(&queue_lock);
    if (rx_skb_queue_front == (rx_skb_queue_rear + 1) % QUEUE_SIZE) {
        /*
         * If queue is full, current solution is to drop
         * the following entries.
         */
        printk("%s: Queue is full, entry will be dropped", __func__);
    } else {
        rx_skb_queue[rx_skb_queue_rear] = skb;
        rx_skb_queue_rear++;
        rx_skb_queue_rear %= QUEUE_SIZE;

    }
    spin_unlock(&queue_lock);
}

static struct sk_buff *rx_dequeue_try(unsigned int deq_len)
{
    struct sk_buff *skb;
    struct sk_buff *skb_copy;

    if (rx_skb_queue_front == rx_skb_queue_rear) {
        printk("%s: Queue is empty", __func__);
        return NULL;
    }

    skb = rx_skb_queue[rx_skb_queue_front];
    if (deq_len >= skb->len) {
        rx_skb_queue[rx_skb_queue_front] = NULL;
        rx_skb_queue_front++;
        rx_skb_queue_front %= QUEUE_SIZE;

        /*
         * Return skb addr to be dequeued, and the caller
         * should free the skb eventually.
         */
        return skb;
    } else {
        skb_copy = pskb_copy(skb, GFP_ATOMIC);
        skb_pull(skb, deq_len);
        /* Return its copy to be freed */
        return skb_copy;
    }
}

static inline int is_queue_empty(void)
{
    return (rx_skb_queue_front == rx_skb_queue_rear) ? 1 : 0;
}

static void rx_clear_queue(void)
{
    struct sk_buff *skb;
    spin_lock(&queue_lock);
    while(!is_queue_empty()) {
        skb = rx_skb_queue[rx_skb_queue_front];
        rx_skb_queue[rx_skb_queue_front] = NULL;
        rx_skb_queue_front++;
        rx_skb_queue_front %= QUEUE_SIZE;
        if (skb) {
            kfree_skb(skb);
        }
    }
    spin_unlock(&queue_lock);
}
#endif

static int atbm_ioctl_notify_add(u8 type, u8 driver_mode, u8 *event_buffer, u16 event_len)
{
	int first;
	struct atbm_status_event *event = NULL;

#ifdef BLUEDROID
	struct sk_buff *skb;
	uint8_t  pkt_type = 0x04;
	int  len;
	int i;

#define BT_SKB_RESERVE 8

	if (event_buffer == NULL)
		return -1;
		
	len = event_len - 4;
        //atbm_printk_err("%s: [%x] atbm rx.\n", __func__, type);
	 //dump_mem(event_buffer, event_len);
	 //printk("\n");

	if ((skb = alloc_skb(len + BT_SKB_RESERVE, GFP_ATOMIC))) 
        	skb_reserve(skb, BT_SKB_RESERVE);
        else
		return -ENOMEM;
	
    	memcpy(skb_put(skb, len), &event_buffer[4], len);

	rx_enqueue(skb);
	wake_up_interruptible(&bt_read_wait);

	return 1;
#endif

	if (atbm_info.hw_priv == NULL){
		atbm_printk_err("%s: atbm ioctl is not open.\n", __func__);
		return -1;
	}
	
	if (s_cur_status_list_cnt >= MAX_STATUS_SYNC_LSIT_CNT){
		atbm_printk_err("%s: status event list is full.\n", __func__);
		return 1;
	}

	if(event_len > MAX_SYNC_EVENT_BUFFER_LEN){
		atbm_printk_err("%s: event_len is overflow.\n", __func__);
		return -1;
	}

	event = atbm_kzalloc(sizeof(struct atbm_status_event), GFP_KERNEL);
	if(event == NULL){
		atbm_printk_err("%s: event atbm_kzalloc is null.\n", __func__);
		return -1;
	}

	if (event_buffer != NULL){
		memcpy(&(event->status.event_buffer), event_buffer, event_len);
	}
	event->status.type = type;
	event->status.driver_mode = driver_mode;

	spin_lock_bh(&s_status_queue_lock);
	first = list_empty(&s_status_head);
	list_add_tail(&event->link, &s_status_head);
	s_cur_status_list_cnt++;
	spin_unlock_bh(&s_status_queue_lock);
	
	if (1){
		return 1;//need async notify usr layer
	}
	else{
		return 0;//not need async notify usr layer
	}
}

void atbm_ioctl_ble_adv_rpt_async(u8 *event_buffer, u16 event_len)
{
	if (atbm_ioctl_notify_add(0, 0, event_buffer, event_len) > 0)
	{
		kill_fasync (&connect_async, SIGIO, POLL_IN);
	}
}

void atbm_ioctl_ble_conn_rpt_async(u8 *event_buffer, u16 event_len)
{
	if (atbm_ioctl_notify_add(1, 0, event_buffer, event_len) > 0)
	{
		kill_fasync (&connect_async, SIGIO, POLL_IN);
	}
}

int atbm_ioctl_ble_start(struct ieee80211_local* local,u8 *data)
{
#ifdef CONFIG_WIFI_BT_COMB
	printk("atbm_ioctl_ble_start\n");
	//ble full stack COMB
	ieee80211_ble_commb_start(local);
	ieee80211_ble_commb_subscribe(local, atbm_ble_dev_rx);
	return 0;
#else
//ble adv/scan comb
	return atbm_ioctl_ble_adv_coexit_start(data);
#endif
	}
	
int atbm_ioctl_ble_stop(struct ieee80211_local* local, u8* data)
{
#ifdef CONFIG_WIFI_BT_COMB
	printk("atbm_ioctl_ble_stop\n");
//ble full stack COMB
	ieee80211_ble_commb_unsubscribe(local);
	ieee80211_ble_commb_stop(local);
	return 0;
#else
//ble adv/scan comb
	return atbm_ioctl_ble_adv_coexit_stop(data);
#endif
	}
	
int atbm_ble_dev_rx(struct ieee80211_local* local, u8* event_buffer, size_t event_len, enum ieee80211_ble_msg_type msg_type)
{
	//printk("atbm_ble_dev_rx len %d\n", event_len);
	//frame_hexdump_wp("rx:", event_buffer, event_len);
	if (atbm_ioctl_notify_add(0, 0, event_buffer, event_len) > 0) {
		kill_fasync(&connect_async, SIGIO, POLL_IN);
	}
	return 0;
	}
void atbm_ble_dev_tx(uint8_t* buf)
{
#ifdef CONFIG_WIFI_BT_COMB
	//struct ble_hci_hif_pkt* tx_pkt;
	struct atbm_common* hw_priv = atbm_info.hw_priv;
	struct ieee80211_local* local = hw_to_local(hw_priv->hw);
	//struct platform_device* pble_dev = &local->ble_dev;
	char* xmit_buff;
	uint8_t* tx_pkt = &buf[2];
	u16 tx_len = *(u16*)buf;
	xmit_buff = NULL;

	//printk("atbm_ble_dev_tx %d\n", tx_len);
	rcu_read_lock();

	xmit_buff = ieee80211_ble_commb_ble_alloc_xmit( HCI_ACL_SHARE_SIZE);

	if (xmit_buff == NULL) {
		goto pkt_free;
}

	//printk("atbm_ble_dev_tx %p tx_pkt %p len %d\n", xmit_buff, tx_pkt, tx_len);
	memcpy(xmit_buff, tx_pkt, tx_len);
	ieee80211_ble_commb_xmit(local, xmit_buff, tx_len);

pkt_free:
	rcu_read_unlock();
	//ble_hci_trans_free_hif_pkt(tx_pkt);
	return;
#else
	printk("unsupport ble mode\n");
#endif //#ifdef CONFIG_WIFI_BT_COMB
}


static long atbm_unlocked_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	long ret = 0;
	struct atbm_common* hw_priv = atbm_info.hw_priv;
	struct ieee80211_local* local;
	if (atbm_info.hw_priv == NULL){
		atbm_printk_err("%s: atbm ioctl is not open.\n", __func__);
		return -1;
	}
	local = hw_to_local(hw_priv->hw);
	switch (cmd)
	{
		case ATBM_BLE_COEXIST_START:
			memset(atbm_ioctl_data, 0, sizeof(atbm_ioctl_data));
			if (0 != copy_from_user(atbm_ioctl_data, (struct at_cmd_direct *)arg, sizeof(atbm_ioctl_data)))
			{
				atbm_printk_err("%s: copy_from_user err.\n", __func__);
				ret = -1;
				break;
			}
			atbm_ioctl_ble_start(local,atbm_ioctl_data);
			break;
		case ATBM_BLE_COEXIST_STOP:
			memset(atbm_ioctl_data, 0, sizeof(atbm_ioctl_data));
			if (0 != copy_from_user(atbm_ioctl_data, (struct at_cmd_direct *)arg, sizeof(atbm_ioctl_data)))
			{
				atbm_printk_err("%s: copy_from_user err.\n", __func__);
				ret = -1;
				break;
			}
			atbm_ioctl_ble_stop(local,atbm_ioctl_data);
			break;
#ifdef CONFIG_ATBM_BLE_ADV_COEXIST
		case ATBM_BLE_SET_ADV_DATA:
			memset(atbm_ioctl_data, 0, sizeof(atbm_ioctl_data));
			if (0 != copy_from_user(atbm_ioctl_data, (struct at_cmd_direct *)arg, sizeof(atbm_ioctl_data)))
			{
				atbm_printk_err("%s: copy_from_user err.\n", __func__);
				ret = -1;
				break;
			}
			atbm_ioctl_ble_set_adv_data(atbm_ioctl_data);		
			break;

		case ATBM_BLE_ADV_RESP_MODE_START:
			memset(atbm_ioctl_data, 0, sizeof(atbm_ioctl_data));
			if (0 != copy_from_user(atbm_ioctl_data, (struct at_cmd_direct *)arg, sizeof(atbm_ioctl_data)))
			{
				atbm_printk_err("%s: copy_from_user err.\n", __func__);
				ret = -1;
				break;
			}
			atbm_ioctl_ble_adv_resp_start(atbm_ioctl_data);	
			break;
		case ATBM_BLE_SET_RESP_DATA:
			memset(atbm_ioctl_data, 0, sizeof(atbm_ioctl_data));
			if (0 != copy_from_user(atbm_ioctl_data, (struct at_cmd_direct *)arg, sizeof(atbm_ioctl_data)))
			{
				atbm_printk_err("%s: copy_from_user err.\n", __func__);
				ret = -1;
				break;
			}
			atbm_ioctl_ble_set_resp_data(atbm_ioctl_data);	
			break;
#endif  //#ifdef CONFIG_ATBM_BLE_ADV_COEXIST
		case ATBM_BLE_HIF_TXDATA:
			memset(atbm_ioctl_data, 0, sizeof(atbm_ioctl_data));
			if (0 != copy_from_user(atbm_ioctl_data, (struct at_cmd_direct*)arg, sizeof(atbm_ioctl_data))) {
				atbm_printk_err("%s: copy_from_user err.\n", __func__);
				ret = -1;
				break;
			}
			//frame_hexdump_wp("HIF_TXDATA ->", atbm_ioctl_data, 32);
			atbm_ble_dev_tx(atbm_ioctl_data);
			break;
		default:
			atbm_printk_err("%s cmd %d invalid.\n", __func__, cmd);
			ret = -1;
	}

	return ret;
}

static int atbm_ioctl_fasync(int fd, struct file *filp, int on)
{
	return fasync_helper(fd, filp, on, &connect_async);
}

static int atbm_ioctl_open(struct inode *inode, struct file *filp)
{
	int time_cnt = 100;
	struct atbm_common *hw_priv = NULL;

#ifdef BLUEDROID
	struct ieee80211_local* local;
	struct ioctl_ble_start iodata;
#endif
	while (NULL == (hw_priv=atbm_hw_priv_dereference()))
	{
		msleep(10);
		time_cnt--;
		if (time_cnt <= 0)
		{
			return -1;
		}
	}
	while (!hw_priv->init_done)
	{
		msleep(10);
		time_cnt--;
		if (time_cnt <= 0)
		{
			return -1;
		}
	}
	
	spin_lock_bh(&s_status_queue_lock);
	while (!list_empty(&s_status_head)) {
		struct atbm_status_event *event =
			list_first_entry(&s_status_head, struct atbm_status_event,
			link);
		list_del(&event->link);
		atbm_kfree(event);
		s_cur_status_list_cnt--;
	}
	spin_unlock_bh(&s_status_queue_lock);


#ifdef BLUEDROID
	local = hw_to_local(hw_priv->hw);

	iodata.ble_interval = 0x0800;
	iodata.ble_scan_win = 0x0010;
	iodata.ble_adv = 1;
	iodata.ble_scan = 0;
	iodata.ble_adv_chan = 39;
	iodata.ble_scan_chan = 38;

	atbm_printk_err("%s: ATBM_BLE_COEXIST_START.\n", __func__);
	memset(atbm_ioctl_data, 0, sizeof(atbm_ioctl_data));
	memcpy(atbm_ioctl_data, &iodata, sizeof(iodata));
	atbm_ioctl_ble_start(local,atbm_ioctl_data);

	rx_clear_queue();
#endif
	atbm_info.hw_priv = hw_priv;
	filp->private_data = &atbm_info;
	atbm_printk_debug("atbm_ioctl_open cost time: %d ms\n", 10*(100-time_cnt));
	return 0;
}

static int atbm_ioctl_release(struct inode *inode, struct file *filp)
{
	atbm_ioctl_fasync(-1, filp, 0);
	filp->private_data = NULL;
	return 0;
}

#ifdef BLUEDROID

static int extand_bredr_cmd(u8 * data)
{
#if 0
#endif 
return 0;
}

static ssize_t atbm_ioctl_write(struct file *filp, char __user *buff, size_t len, loff_t *off)
{
	
	//struct ble_hci_hif_pkt* tx_pkt;
	struct atbm_common* hw_priv = atbm_info.hw_priv;
	struct ieee80211_local* local = hw_to_local(hw_priv->hw);
	//struct platform_device* pble_dev = &local->ble_dev;
	char* xmit_buff;
	static u8 data[1024];
	uint8_t* tx_pkt;
	u16 tx_len;
	xmit_buff = NULL;
	int ret = 0;
	int i;
	static uint8_t pkt_type;
	memset(data, 0, sizeof(data));

	if (0 != copy_from_user(&data[3], (struct at_cmd_direct*)buff, len)) {
		atbm_printk_err("%s: copy_from_user err.\n", __func__);
		ret = -1;
		return ret;
	}

	if(len == 1){
		//atbm_printk_err("HIF_TXDATA type[%x] \n",data[3]);
		pkt_type = data[3];
		return len;
	}
	
	
	*(u16 *)data = len + 1; 
    	data[2] = pkt_type;

	tx_len = len + 3;  //  totel_len + pkt_type + len
	tx_pkt = data;

	//dump_mem(tx_pkt,tx_len);
	//printk("\n");

	if(!extand_bredr_cmd(data)){
		atbm_ble_dev_tx(data);
	}
	return len;
}
static ssize_t atbm_ioctl_read(struct file *filp, char __user *buff, size_t count, loff_t *off)
{

    //struct sk_buff *skb;
    ssize_t ret = 0;
    struct sk_buff *skb;
    int len;
    int i;
    uint16_t opcode;


    while (count) {
	ret = wait_event_interruptible(bt_read_wait, !is_queue_empty());
       	if (ret < 0)
	{
            atbm_printk_err("%s: wait event is signaled %d", __func__, (int)ret);
            break;
       	} 


       	skb = rx_dequeue_try(count);
       	if (skb) {
            	len = min_t(unsigned int, skb->len, count);
		if(len == 68){
			opcode = *(uint16_t *)&skb->data[1];
			atbm_printk_err("opcode [%x]:\n",opcode);
			if( opcode == 0x1002) 
				skb->data[24] = 0x10;

		}

		//dump_mem(skb->data, len);

            	ret = copy_to_user(buff, skb->data, len);
            	if (ret < 0)
                	atbm_printk_err("%s: Failed to put data to user space", __func__);
            	else
			ret = len;
			kfree_skb(skb);
			
        	break;
        }

    }

    return ret;
}

unsigned int atbm_ioctl_poll(struct file *filp, struct poll_table_struct *wait)
{
    unsigned int mask = 0;
    poll_wait(filp, &bt_read_wait , wait);
    if(!is_queue_empty()){
        mask |= POLLIN | POLLRDNORM;      //���������������ݿɻ��
    }
    return mask;
}
#else
static ssize_t atbm_ioctl_read(struct file *filp, char __user *buff, size_t len, loff_t *off)
{
	int ret = 0;
	struct atbm_status_event *event = NULL;

	if (atbm_info.hw_priv == NULL){
		atbm_printk_err("%s: atbm ioctl is not open.\n", __func__);
		return -1;
	}

	if (sizeof(struct ioctl_status_async) > len)
	{
		atbm_printk_err("%s: buff len is not enough.\n", __func__);
		return -1;
	}

	if (list_empty(&s_status_head))
	{
		atbm_printk_err("%s: status list is empty.\n", __func__);
		return -1;
	}

	spin_lock_bh(&s_status_queue_lock);
	event = list_first_entry(&s_status_head, struct atbm_status_event, link);
	if (event)
	{
		if (s_cur_status_list_cnt >= 2)
		{
			event->status.list_empty = 0;
		}
		else
		{
			event->status.list_empty = 1;
		}
		spin_unlock_bh(&s_status_queue_lock);
		ret = copy_to_user(buff, &event->status, sizeof(struct ioctl_status_async));
		spin_lock_bh(&s_status_queue_lock);
		if (ret)
		{
			atbm_printk_err("%s: copy_to_user err %d.\n", __func__, ret);
		}
		else
		{
			list_del(&event->link);
			atbm_kfree(event);
			s_cur_status_list_cnt--;
		}
	}
	else
	{
		ret = -1;
	}
	spin_unlock_bh(&s_status_queue_lock);
	
	if (ret)
	{
		return -1;
	}

	return sizeof(struct ioctl_status_async);
}
#endif

static struct file_operations atbm_ioctl_fops = {
    .owner = THIS_MODULE,
    .open = atbm_ioctl_open,
    .release = atbm_ioctl_release,
    .read = atbm_ioctl_read,
#ifdef BLUEDROID
	.write = atbm_ioctl_write,
	.poll = atbm_ioctl_poll,
#endif
    .unlocked_ioctl = atbm_unlocked_ioctl,
    .fasync = atbm_ioctl_fasync,
};

int atbm_ioctl_add(void)
{
	memset(&atbm_info, 0, sizeof(struct atbm_info));

	alloc_chrdev_region(&atbm_info.devid, 0, 1, "atbm_ioctl");

	atbm_info.my_cdev = cdev_alloc();
	cdev_init(atbm_info.my_cdev, &atbm_ioctl_fops);

	atbm_info.my_cdev->owner = THIS_MODULE;
	cdev_add(atbm_info.my_cdev, atbm_info.devid, 1);

	atbm_info.my_class = class_create(THIS_MODULE, "atbm_ioctl_class");
	atbm_info.my_device = device_create(atbm_info.my_class, NULL, atbm_info.devid, NULL, "atbm_ioctl");

#ifdef BLUEDROID
    	init_waitqueue_head(&bt_read_wait);
	spin_lock_init(&queue_lock);
#endif
	spin_lock_init(&s_status_queue_lock);
	INIT_LIST_HEAD(&s_status_head);

	atbm_printk_always("atbm_ioctl_add\n");
	return 0;
}

void atbm_ioctl_free(void)
{
	device_destroy(atbm_info.my_class, atbm_info.devid);
	class_destroy(atbm_info.my_class);
	cdev_del(atbm_info.my_cdev);
	unregister_chrdev_region(atbm_info.devid, 1);
	memset(&atbm_info, 0, sizeof(struct atbm_info));

	spin_lock_bh(&s_status_queue_lock);
	while (!list_empty(&s_status_head)) {
		struct atbm_status_event *event =
			list_first_entry(&s_status_head, struct atbm_status_event,
			link);
		list_del(&event->link);
		atbm_kfree(event);
		s_cur_status_list_cnt--;
	}
	spin_unlock_bh(&s_status_queue_lock);
}
#endif



