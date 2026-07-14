#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/spi/spi.h>
#include <linux/types.h>
#include <linux/ide.h>
#include <linux/errno.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
 
#include "RC522_SPI.h"
/***************************************************************
文件名 : spi_rc522_drive.c
作者 : jiaozhu
版本 : V1.0
描述 : RFID-RC522 设备驱动文件
其他 : 无
日志 : 初版 V1.0 2023/2/16
***************************************************************/
 
 
/*------------------字符设备内容----------------------*/
#define RC522_NAME	"spi_rc522"
#define RC522_CNT	(1)
 
static unsigned char card_type[2];		// 卡片类型
static unsigned char card_id[4];		// 卡片id
static unsigned char card_auth_mode = 0x60;		// 密码验证类型
static unsigned char card_cipher[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};		// 卡片块密码
 
struct rc522_dev_s {
	struct spi_device *spi; 			// spi 设备
	dev_t devid; 						// 设备号
	struct cdev cdev; 					// cdev
	struct class *class; 				// 类
	struct device *device; 				// 设备
	struct device_node *node;			// 设备节点
};
 
/* 声明 SPI 操作函数 */
static s32 spi_write_regs(struct spi_device *spi, u8 reg, u8 *buf, u8 len);
static int spi_read_regs(struct spi_device *spi, u8 reg, void *buf, int len);
 
 
/**
 * @brief 向 rc522 设备的寄存器中写入 8 位数据
 * 
 * @param rc522_dev rc522 设备
 * @param reg 寄存器地址
 * @param val 写入的值
 * @return 返回执行的结果
 */
static int rc522_write_reg8(struct rc522_dev_s *rc522_dev, u8 reg, u8 value)
{
	u8 buf = value;
	return spi_write_regs(rc522_dev->spi, (reg << 1) & 0x7E, &buf, 1);
}
 
/**
 * @brief 从 rc522 设备的寄存器中读取 8 位数据
 * 
 * @param rc522_dev rc522 设备
 * @param reg 寄存器地址
 * @param buf 读取的缓冲区
 * @return 返回执行的结果
 */
static int rc522_read_reg8(struct rc522_dev_s *rc522_dev, u8 reg, u8 *buf)
{
	return spi_read_regs(rc522_dev->spi, (reg << 1) & 0x7E, buf, 1);
}
 
/**
 * @brief 置RC522寄存器位
 * 
 * @param rc522_dev rc522 设备
 * @param reg 寄存器地址
 * @param mask 置位值
 * @return 返回执行的结果
 */
static int rc522_set_bit_mask(struct rc522_dev_s *rc522_dev, u8 reg, u8 mask)  
{
	int res = 0;
    u8 tmp = 0x0;
    res = rc522_read_reg8(rc522_dev, reg, &tmp);
	if (0 != res)
	{
		return MI_NOTAGERR;
	}
    rc522_write_reg8(rc522_dev, reg, tmp | mask);  // set bit mask
	return MI_OK;
}
 
/**
 * @brief 清RC522寄存器位
 * 
 * @param rc522_dev rc522 设备
 * @param reg 寄存器地址
 * @param mask 清位值
 * @return 返回执行的结果
 */
static int rc522_clear_bit_mask(struct rc522_dev_s *rc522_dev, u8 reg, u8 mask)
{
	int res = 0;
    u8 tmp = 0x0;
    res = rc522_read_reg8(rc522_dev, reg, &tmp);
	if (0 != res)
	{
		return MI_NOTAGERR;
	}
    rc522_write_reg8(rc522_dev, reg, tmp & ~mask);  // set bit mask
	return MI_OK;
}
 
 
/**
 * @brief 用 RC522 计算 CRC16 函数
 * 
 * @param rc522_dev rc522 设备
 * @param pIndata 需要计算的数据
 * @param len 数据长度
 * @param pOutData CRC 计算结果
 * @return 返回执行的结果
 */
static int rc522_calulate_crc(struct rc522_dev_s *rc522_dev, u8 *pIndata, u8 len, u8 *pOutData)
{
	u8 i,n;
	int res = 0;
	rc522_clear_bit_mask(rc522_dev, DivIrqReg, 0x04);
	rc522_write_reg8(rc522_dev, CommandReg, PCD_IDLE);
	rc522_set_bit_mask(rc522_dev, FIFOLevelReg, 0x80);
	for (i=0; i<len; i++)
	{   
		rc522_write_reg8(rc522_dev, FIFODataReg, *(pIndata+i));
	}
	rc522_write_reg8(rc522_dev, CommandReg, PCD_CALCCRC);
	i = 0xFF;
	do 
	{
		res = rc522_read_reg8(rc522_dev, DivIrqReg, &n);
		i--;
	}
	while ((i != 0) && !( n & 0x04));
	res |= rc522_read_reg8(rc522_dev, CRCResultRegL, &pOutData[0]);
	res |= rc522_read_reg8(rc522_dev, CRCResultRegM, &pOutData[1]);
	return res;
}
 
/**
 * @brief 通过RC522和ISO14443卡通讯
 * 
 * @param rc522_dev rc522 设备
 * @param Command RC522 命令字
 * @param pInData 通过 RC522 发送到卡片的数据
 * @param InLenByte 发送数据的字节长度
 * @param pOutData 接收到的卡片返回数据
 * @param pOutLenBit 返回数据的位长度
 * @return 返回执行的结果
 */
static int rc522_com_card(struct rc522_dev_s *rc522_dev, u8 Command, u8 *pInData, u8 InLenByte, u8 *pOutData, u32 *pOutLenBit)
{
	int status = MI_ERR;
	u8 irqEn   = 0x00;
	u8 waitFor = 0x00;
	u8 lastBits;
	u8 n;
	u32 i;
	switch (Command)
	{
		case PCD_AUTHENT:
			irqEn   = 0x12;
			waitFor = 0x10;
			break;
		case PCD_TRANSCEIVE:
			irqEn   = 0x77;
	    	waitFor = 0x30;
	    	break;
	   	default:
	     	break;
	}
 
	rc522_write_reg8(rc522_dev, ComIEnReg, irqEn|0x80);
	rc522_clear_bit_mask(rc522_dev, ComIrqReg, 0x80);
	rc522_write_reg8(rc522_dev, CommandReg, PCD_IDLE);
	rc522_set_bit_mask(rc522_dev, FIFOLevelReg, 0x80);
	
	for (i = 0; i < InLenByte; i++)
	{   
		rc522_write_reg8(rc522_dev, FIFODataReg, pInData[i]);
	}
	rc522_write_reg8(rc522_dev, CommandReg, Command);
	
	 
	if (Command == PCD_TRANSCEIVE)
	{    
		rc522_set_bit_mask(rc522_dev, BitFramingReg, 0x80);
	}
	 
	/* 根据时钟频率调整，操作 M1 卡最大等待时间25ms */
	i = 2000;
	do 
	{
		status = rc522_read_reg8(rc522_dev, ComIrqReg, &n);
		i--;
	}
	while ((i != 0) && !(n & 0x01) && !(n & waitFor));
	rc522_clear_bit_mask(rc522_dev, BitFramingReg, 0x80);
	      
	if (i !=0 )
	{    
		status = rc522_read_reg8(rc522_dev, ErrorReg, &n);
	 	if(!(n & 0x1B))
	   	{
	      	status = MI_OK;
	      	if (n & irqEn & 0x01)
	       	{   
				status = MI_NOTAGERR;
			}
	       	if (Command == PCD_TRANSCEIVE)
	      	{
				status = rc522_read_reg8(rc522_dev, FIFOLevelReg, &n);
				status = rc522_read_reg8(rc522_dev, ControlReg, &lastBits);
				lastBits = lastBits & 0x07;
	        	if (lastBits)
	        	{   
					*pOutLenBit = (n-1)*8 + lastBits;   }
	         	else
	         	{   
					*pOutLenBit = n * 8;
				}
	           	if (n == 0)
	          	{   
					n = 1;
				}
	        	if (n > MAXRLEN)
	           	{   
					n = MAXRLEN;
				}
	           	for (i=0; i<n; i++)
	          	{   
					status = rc522_read_reg8(rc522_dev, FIFODataReg, &pOutData[i]);
				}
	      	}
	 	}
	  	else
	   	{   
			status = MI_ERR;
		}
	}
 
	rc522_set_bit_mask(rc522_dev, ControlReg, 0x80);
	rc522_write_reg8(rc522_dev, CommandReg, PCD_IDLE);
	return status;
}
 
/**
 * @brief 关闭天线
 * 
 * @param rc522_dev rc522 设备
 * @return 返回执行的结果
 */
static int rc522_antenna_off(struct rc522_dev_s *rc522_dev)
{
    return rc522_clear_bit_mask(rc522_dev, TxControlReg, 0x03);
}
 
/**
 * @brief 开启天线
 * 
 * @param rc522_dev rc522 设备, 每次启动或关闭天险发射之间应至少有1ms的间隔
 * @return 返回执行的结果
 */
static int rc522_antenna_on(struct rc522_dev_s *rc522_dev)
{
    u8 tmp = 0x0;
    tmp = rc522_read_reg8(rc522_dev, TxControlReg, &tmp);
    if (!(tmp & 0x03))
    {
        return rc522_set_bit_mask(rc522_dev, TxControlReg, 0x03);
    }
	return MI_OK;
}
 
/**
 * @brief 设置 RC522 的工作方式
 * 
 * @param rc522_dev rc522 设备
 * @param type 工作模式，新增模式时，建议通过枚举
 * @return 返回执行的结果
 */
static int rc522_config_iso_type(struct rc522_dev_s *rc522_dev, u8 type)
{
	int res = MI_OK;
 
	switch (type)
	{
		case 1:
			res = rc522_clear_bit_mask(rc522_dev, Status2Reg, 0x08);
			res |= rc522_write_reg8(rc522_dev, ModeReg,0x3D);
			res |= rc522_write_reg8(rc522_dev, TxSelReg,0x10);
			res |= rc522_write_reg8(rc522_dev, RxSelReg,0x86);
			res |= rc522_write_reg8(rc522_dev, RFCfgReg,0x7F);
			res |= rc522_write_reg8(rc522_dev, TReloadRegL,30);
			res |= rc522_write_reg8(rc522_dev, TReloadRegH,0);
			res |= rc522_write_reg8(rc522_dev, TModeReg,0x8D);
			res |= rc522_write_reg8(rc522_dev, TPrescalerReg,0x3E);
			msleep (1);
			res |= rc522_antenna_on(rc522_dev);
			break;
		
		default:
			res = MI_NOTAGERR;
			break;
	}
   
	return res;
}
 
/**
 * @brief 复位RC522
 * 
 * @param rc522_dev rc522 设备
 * @return 返回执行的结果
 */
static int rc522_reset_dev(struct rc522_dev_s *rc522_dev)
{
	int ret = MI_OK;
	/* RC522 启动并复位 */
	ret = rc522_write_reg8(rc522_dev, CommandReg, PCD_RESETPHASE);
	ret |= rc522_write_reg8(rc522_dev, ModeReg, 0x3D);
	ret |= rc522_write_reg8(rc522_dev, TReloadRegL, 30);
	ret |= rc522_write_reg8(rc522_dev, TReloadRegH, 0);
	ret |= rc522_write_reg8(rc522_dev, TModeReg, 0x8D);
	ret |= rc522_write_reg8(rc522_dev, TPrescalerReg, 0x3E);
	ret |= rc522_write_reg8(rc522_dev, TxAutoReg, 0x40);
	return MI_OK;
}
 
/**
 * @brief RC522 寻卡
 * 
 * @param rc522_dev rc522 设备
 * @param req_code 寻卡方式, 
 * 				0x52 = 寻感应区内所有符合14443A标准的卡
 * 				0x26 = 寻未进入休眠状态的卡
 * @param pTagType 卡片类型代码
 * 				0x4400 = Mifare_UltraLight
 * 				0x0400 = Mifare_One(S50)
 * 				0x0200 = Mifare_One(S70)
 * 				0x0800 = Mifare_Pro(X)
 * 				0x4403 = Mifare_DESFire
 * @return 返回执行的结果
 */
static int rc522_request_card(struct rc522_dev_s *rc522_dev, u8 req_code, u8 *pTagType)
{
	int status;  
	unsigned int  unLen;
	unsigned char ucComMF522Buf[MAXRLEN];
 
	rc522_clear_bit_mask(rc522_dev, Status2Reg, 0x08);
	rc522_write_reg8(rc522_dev, BitFramingReg, 0x07);
	rc522_set_bit_mask(rc522_dev, TxControlReg, 0x03);
 
	ucComMF522Buf[0] = req_code;
 
	status = rc522_com_card(rc522_dev, PCD_TRANSCEIVE, ucComMF522Buf, 1, ucComMF522Buf, &unLen);
	if ((status == MI_OK) && (unLen == 0x10))
	{    
   		*pTagType     = ucComMF522Buf[0];
   		*(pTagType+1) = ucComMF522Buf[1];
   	}
   	else
   	{
		status = MI_ERR;
	}
 
	return status;
}
 
/**
 * @brief RC522 防冲撞
 * 
 * @param rc522_dev rc522 设备
 * @param pSnr 卡片序列号，4字节
 * @return 返回执行的结果
 */
static int rcc_anticoll_card(struct rc522_dev_s *rc522_dev, u8 *pSnr)
{
	int status;
	unsigned char i, snr_check=0;
	unsigned int  unLen;
	unsigned char ucComMF522Buf[MAXRLEN]; 
 
	rc522_clear_bit_mask(rc522_dev, Status2Reg, 0x08);
	rc522_write_reg8(rc522_dev, BitFramingReg, 0x00);
	rc522_clear_bit_mask(rc522_dev, CollReg, 0x80);
 
	ucComMF522Buf[0] = PICC_ANTICOLL1;
	ucComMF522Buf[1] = 0x20;
 
	status = rc522_com_card(rc522_dev, PCD_TRANSCEIVE, ucComMF522Buf, 2, ucComMF522Buf, &unLen);
	if (status == MI_OK)
	{
		for (i=0; i<4; i++)
		{   
			*(pSnr+i)  = ucComMF522Buf[i];
			snr_check ^= ucComMF522Buf[i];
		}
		if (snr_check != ucComMF522Buf[i])
		{   status = MI_ERR;    }
	}
 
	rc522_set_bit_mask(rc522_dev, CollReg, 0x80);
	return status;
}
 
/**
 * @brief RC522 选定卡片
 * 
 * @param rc522_dev rc522 设备
 * @param pSnr 卡片序列号，4字节
 * @return 返回执行的结果
 */
static int rc522_select_card(struct rc522_dev_s *rc522_dev, u8 *pSnr)
{
    char status;
    unsigned char i;
    unsigned int  unLen;
    unsigned char ucComMF522Buf[MAXRLEN]; 
    
    ucComMF522Buf[0] = PICC_ANTICOLL1;
    ucComMF522Buf[1] = 0x70;
    ucComMF522Buf[6] = 0;
    for (i=0; i<4; i++)
    {
    	ucComMF522Buf[i+2] = *(pSnr+i);
    	ucComMF522Buf[6]  ^= *(pSnr+i);
    }
    rc522_calulate_crc(rc522_dev, ucComMF522Buf, 7, &ucComMF522Buf[7]);
  
    rc522_clear_bit_mask(rc522_dev, Status2Reg, 0x08);
 
    status = rc522_com_card(rc522_dev, PCD_TRANSCEIVE, ucComMF522Buf, 9, ucComMF522Buf, &unLen);
    if ((status == MI_OK) && (unLen == 0x18))
    {   
		status = MI_OK;
	}
    else
    {   
		status = MI_ERR;
	}
    return status;
}
     
/**
 * @brief RC522 验证卡片密码
 * 
 * @param rc522_dev rc522 设备
 * @param auth_mode 密码验证模式，0x60 = 验证A密钥，0x61 = 验证B密钥
 * @param addr 块地址
 * @param pKey 密码
 * @param pSnr 卡片序列号，4字节
 * @return 返回执行的结果
 */
static int rc522_auth_state(struct rc522_dev_s *rc522_dev, u8 auth_mode, u8 addr, u8 *pKey, u8 *pSnr)
{
    int status;
    unsigned int  unLen;
    unsigned char i,ucComMF522Buf[MAXRLEN]; 
	u8 temp;
 
    ucComMF522Buf[0] = auth_mode;
    ucComMF522Buf[1] = addr;
    for (i=0; i<6; i++)
    {    
		ucComMF522Buf[i+2] = *(pKey+i);
	}
    for (i=0; i<6; i++)
    {    
		ucComMF522Buf[i+8] = *(pSnr+i);
	}
    
    status = rc522_com_card(rc522_dev, PCD_AUTHENT, ucComMF522Buf, 12, ucComMF522Buf, &unLen);
	rc522_read_reg8(rc522_dev, Status2Reg, &temp);
    if ((status != MI_OK) || (!(temp & 0x08)))
    {   
		status = MI_ERR;
	}
    
    return status;
}
 
/**
 * @brief 读取 RC522 卡的一块数据
 * 
 * @param rc522_dev rc522 设备
 * @param addr 块地址
 * @param pData 读出的数据，16字节
 * @return 返回执行的结果
 */
static int rc522_read_card(struct rc522_dev_s *rc522_dev, u8 addr, u8 *pData)
{
	char status;
	unsigned int  unLen;
	unsigned char i, ucComMF522Buf[MAXRLEN]; 
 
	ucComMF522Buf[0] = PICC_READ;
	ucComMF522Buf[1] = addr;
	rc522_calulate_crc(rc522_dev, ucComMF522Buf, 2 , &ucComMF522Buf[2]);
 
	status = rc522_com_card(rc522_dev, PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, &unLen);
	if ((status == MI_OK) && (unLen == 0x90))
	{
		for (i=0; i<16; i++)
		{    
			*(pData+i) = ucComMF522Buf[i];
		}
	}
	else
	{   
		status = MI_ERR;
	}
	return status;
}
 
/**
 * @brief 写入 RC522 卡的一块数据
 * 
 * @param rc522_dev rc522 设备
 * @param addr 块地址
 * @param pData 读出的数据，16字节
 * @return 返回执行的结果
 */
static int rc522_write_card(struct rc522_dev_s *rc522_dev, u8 addr, u8 *pData)
{
    char status;
    unsigned int unLen;
    unsigned char i, ucComMF522Buf[MAXRLEN];
    ucComMF522Buf[0] = PICC_WRITE;
    ucComMF522Buf[1] = addr;
 
    rc522_calulate_crc(rc522_dev, ucComMF522Buf, 2, &ucComMF522Buf[2]);
    status = rc522_com_card(rc522_dev, PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, &unLen);
    if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
    {
        status = MI_ERR;
    }
    if (status == MI_OK)
    {
        for (i = 0; i < 16; i++)
        {
            ucComMF522Buf[i] = *(pData + i);
        }
        rc522_calulate_crc(rc522_dev, ucComMF522Buf, 16, &ucComMF522Buf[16]);
 
        status = rc522_com_card(rc522_dev, PCD_TRANSCEIVE, ucComMF522Buf, 18, ucComMF522Buf, &unLen);
        if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
        {
            status = MI_ERR;
        }
    }
    return status;
}
 
 
/**
 * @brief RC522 命令卡片进入休眠状态
 * 
 * @param rc522_dev rc522 设备
 * @return 返回执行的结果
 */
static int rc522_halt_card(struct rc522_dev_s *rc522_dev)
{
    char status;
    unsigned int  unLen;
    unsigned char ucComMF522Buf[MAXRLEN]; 
 
    ucComMF522Buf[0] = PICC_HALT;
    ucComMF522Buf[1] = 0;
    rc522_calulate_crc(rc522_dev, ucComMF522Buf, 2, &ucComMF522Buf[2]);
 
    status = rc522_com_card(rc522_dev, PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, &unLen);
	if ((status != MI_OK))
	{
		return MI_ERR;
	}
    return MI_OK;
}
 
 
/**
 * @brief 向 spi 设备中写入多个寄存器数据
 * 
 * @param spi spi 设备
 * @param reg 要写入的寄存器首地址
 * @param buf 要写入的数据缓冲区
 * @param len 要写入的数据长度
 * @return 返回执行结果
 */
static s32 spi_write_regs(struct spi_device *spi, u8 reg, u8 *buf, u8 len)
{
	int ret = -1;
	unsigned char *txdata;
	struct spi_message msg;
	struct spi_transfer *trf;
 
	/* 申请内存*/
	trf = kzalloc(sizeof(struct spi_transfer), GFP_KERNEL);
	if(!trf) {
		return -ENOMEM;
	}
 
	txdata = kzalloc(sizeof(char)+len, GFP_KERNEL);
	if(!txdata) {
		goto out1;
	}
 
	/* 一共发送 len+1 个字节的数据，第一个字节为寄存器首地址，len 为要写入的寄存器的集合，*/
	*txdata = reg & ~0x80; /* 写数据的时候首寄存器地址 bit8 要清零 */
	memcpy(txdata+1, buf, len); /* 把 len 个数据拷贝到 txdata 里 */
	trf->tx_buf = txdata; /* 要发送的数据 */
	trf->len = len+1; /* trf->len = 发送的长度+读取的长度 */
	spi_message_init(&msg); /* 初始化 spi_message */
	spi_message_add_tail(trf, &msg);/*添加到 spi_message 队列 */
	ret = spi_sync(spi, &msg); /* 同步发送 */
	if(ret) {
		goto out2;
	}
 
out2:
	kfree(txdata); /* 释放内存 */
out1:
	kfree(trf); /* 释放内存 */
	return ret;
 
}
 
/**
 * @brief 读取 spi 的多个寄存器数据
 * 
 * @param spi spi 设备
 * @param reg 要读取的寄存器首地址
 * @param buf 要读取的数据缓冲区
 * @param len 要读取的数据长度
 * @return 返回执行结果
 */
static int spi_read_regs(struct spi_device *spi, u8 reg, void *buf, int len)
{
 
	int ret = -1;
	unsigned char txdata[1];
	unsigned char * rxdata;
	struct spi_message msg;
	struct spi_transfer *trf;
 
	/* 申请内存*/
	trf = kzalloc(sizeof(struct spi_transfer), GFP_KERNEL);
	if(!trf) {
		return -ENOMEM;
	}
 
	/* 申请内存 */
	rxdata = kzalloc(sizeof(char) * len, GFP_KERNEL);
	if(!rxdata) {
		goto out1;
	}
 
	/* 一共发送 len+1 个字节的数据，第一个字节为寄存器首地址，一共要读取 len 个字节长度的数据，*/
	txdata[0] = reg | 0x80; /* 写数据的时候首寄存器地址 bit8 要置 1 */ 
	trf->tx_buf = txdata; /* 要发送的数据 */
 
	trf->rx_buf = rxdata; /* 要读取的数据 */
	trf->len = len+1; /* trf->len = 发送的长度+读取的长度 */
	spi_message_init(&msg); /* 初始化 spi_message */
	spi_message_add_tail(trf, &msg);/* 将 spi_transfer 添加到 spi_message*/
	ret = spi_sync(spi, &msg); /* 同步发送 */
	if(ret) {
		goto out2;
	}
 
	memcpy(buf , rxdata+1, len); /* 只需要读取的数据 */
 
out2:
	kfree(rxdata); /* 释放内存 */
out1: 
	kfree(trf); /* 释放内存 */
 
	return ret;
}
 
/**
 * @brief 向 spi 设备中同时读写多个寄存器数据
 * 
 * @param spi spi 设备
 * @param reg 要写入的寄存器首地址
 * @param write_buf 要写入的数据缓冲区
 * @param read_buf 要读取的数据缓冲区
 * @param len 要写入的数据长度
 * @return 返回执行结果
 */
static s32 spi_read_write_regs(struct spi_device *spi, u8 reg, u8 *write_buf, u8 *read_buf, u8 len)
{
	int ret = -1;
	unsigned char *txdata;
	struct spi_message msg;
	struct spi_transfer *trf;
	unsigned char * rxdata;
 
	/* 申请内存*/
	trf = kzalloc(sizeof(struct spi_transfer), GFP_KERNEL);
	if(!trf) {
		return -ENOMEM;
	}
 
	txdata = kzalloc(sizeof(char)+len, GFP_KERNEL);
	if(!txdata) {
		goto out1;
	}
 
	/* 申请内存 */
	rxdata = kzalloc(sizeof(char) * len, GFP_KERNEL);
	if(!rxdata) {
		goto out1;
	}
 
	/* 一共发送 len+1 个字节的数据，第一个字节为寄存器首地址，len 为要写入的寄存器的集合，*/
	*txdata = reg & ~0x80;
	memcpy(txdata+1, write_buf, len); /* 把 len 个数据拷贝到 txdata 里 */
	trf->tx_buf = txdata; /* 要发送的数据 */
	trf->rx_buf = rxdata; /* 要读取的数据 */
 
	trf->len = len+1; /* trf->len = 发送的长度+读取的长度 */
	spi_message_init(&msg); /* 初始化 spi_message */
	spi_message_add_tail(trf, &msg);/*添加到 spi_message 队列 */
	ret = spi_sync(spi, &msg); /* 同步发送 */
	if(ret) {
		goto out2;
	}
 
	memcpy(read_buf , rxdata+1, len); /* 只需要读取的数据 */
 
out2:
	kfree(txdata); /* 释放内存 */
	kfree(rxdata); /* 释放内存 */
out1:
	kfree(trf); /* 释放内存 */
	return ret;
}
 
/**
 * @brief 打开设备
 * 
 * @param inode 传递给驱动的 inode
 * @param filp 设备文件，file 结构体有个叫做 private_data 的成员变量
 * 一般在 open 的时候将 private_data 指向设备结构体。
 * @return 0 成功;其他 失败
 */
static int rc522_open(struct inode *inode, struct file *filp)
{
	u8 value[5];
	u8 buf[5] = {0x11, 0x22, 0x33, 0x44, 0x55};
	int res = -1;
 
	struct cdev *cdev = filp->f_path.dentry->d_inode->i_cdev;
	struct rc522_dev_s *rc522_dev = container_of(cdev, struct rc522_dev_s, cdev);
	filp->private_data = rc522_dev;
 
	// pr_info("rc522_open\n");
	/* 复位 RC522 */
	res = rc522_reset_dev(rc522_dev);
	/* 关闭天线 */
	res |= rc522_antenna_off(rc522_dev);
	msleep (1);
	/* 打开天线，天线操作之间需要间隔 1ms */
	res |= rc522_antenna_on(rc522_dev);
	/* 设置 RC522 的工作模式*/
	res |= rc522_config_iso_type(rc522_dev, 1);
 
	if (MI_OK != res)
	{
		return MI_NOTAGERR;
	}
 
    return MI_OK;
 
	rc522_write_reg8(rc522_dev, 0x05, 0xFF);
 
	/* 验证 spi 是否正常工作 */
	mutex_lock(&rc522_dev->spi->dev.mutex);
	spi_read_write_regs(rc522_dev->spi, 0x01, buf, value, 5);
	mutex_unlock(&rc522_dev->spi->dev.mutex);
	pr_info("spi read value is: %x	%x	%x	%x	%x\n", value[0], value[1], value[2], value[3], value[4]);
}
 
/**
 * @brief 从设备读取数据
 * 
 * @param filp 要打开的设备文件(文件描述符)
 * @param buf 返回给用户空间的数据缓冲区
 * @param cnt 要读取的数据长度
 * @param offt 相对于文件首地址的偏移
 * @return 0 成功;其他 失败
 */
static ssize_t rc522_read(struct file *filp, char __user *buf, size_t cnt, loff_t *offt)
{
	int res = 0;
	unsigned char card_data[16];
	int read_position = *offt/16;			// 用户空间读取的位置
	struct rc522_dev_s *rc522_dev = filp->private_data;
 
	/* RC522 只有16个扇区，每个扇区4个块，总共64块 */
	if (read_position > 66)
	{
		return MI_NOTAGERR;
	}
 
	/* 寻卡 */
	if (64 == read_position)
	{
		res = rc522_request_card(rc522_dev, 0x26, card_type);
		if (MI_OK != res)
		{
			return MI_NOTAGERR;
		}
 
		/* 将卡片 id 拷贝到用户空间中 */
		return copy_to_user(buf, &card_type, cnt);
	}
	
	/* 防冲撞，读取卡的ID */
	if (65 == read_position)
	{
		res = rcc_anticoll_card(rc522_dev, card_id);
		if (MI_OK != res)
		{
			return MI_NOTAGERR;
		}
 
		return copy_to_user(buf, card_id, cnt);
	}
 
	/* 读取卡片密码 */
	if (66 == read_position)
	{
		return copy_to_user(buf, card_cipher, cnt);
	}
 
	/* 验证卡片密码 */
	res = rc522_auth_state(rc522_dev, card_auth_mode, read_position, card_cipher, card_id);
	if (MI_OK != res)
	{
		// pr_info("Verification card password setting error when reading\n");
		return MI_NOTAGERR;
	}
 
	/* 读取指定块中的数据 */
	memset(card_data, 0, sizeof(card_data));
	res = rc522_read_card(rc522_dev, read_position, card_data);
	if (MI_OK != res)
	{
		// pr_info("Failed to read card\n");
		return MI_NOTAGERR;
	}
 
	return copy_to_user(buf, card_data, cnt);
}
 
/**
 * @brief 向设备写数据
 * 
 * @param filp 设备文件，表示打开的文件描述符
 * @param buf 要写给设备写入的数据
 * @param cnt 要写入的数据长度
 * @param offt 相对于文件首地址的偏移
 * @return 写入的字节数，如果为负值，表示写入失败
*/
static ssize_t rc522_write(struct file *filp, const char __user *buf, size_t cnt, loff_t *offt)
{
    int res = 0;
	unsigned char temp = 0;
	unsigned char card_data[16] = {0};
	struct rc522_dev_s *rc522_dev = filp->private_data;
	int write_position = *offt/16;			// 用户空间读取的位置
 
	/* RC522 只有16个扇区，每个扇区4个块，总共64块 */
	if (write_position > 66)
	{
		return MI_NOTAGERR;
	}
 
	/* 设置密码验证方式 */
	if (64 == write_position)
	{
		res = copy_from_user(&temp, buf, 1);
		if (MI_OK != res)
		{
			return MI_NOTAGERR;
		}
		
		if (temp)
		{
			/* 验证 B 密钥 */
			card_auth_mode = 0x61;
		}
		else
		{
			/* 验证 A 密钥 */
			card_auth_mode = 0x60;
		}
		return MI_OK;
	}
 
	/* 选择卡片 */
	if (65 == write_position)
	{
		if (cnt > sizeof(card_id))
		{
			res = copy_from_user(card_id, buf, sizeof(card_id));
		}
		else
		{
			res = copy_from_user(card_id, buf, cnt);
		}
 
		if (MI_OK != res)
		{
			return MI_NOTAGERR;
		}
		
		/* 选择卡片 */
		res = rc522_select_card(rc522_dev, card_id);
		if (MI_OK != res)
		{
			// pr_info("Failed to select card when reading\n");
			return MI_NOTAGERR;
		}
		return MI_OK;
	}
 
	/* 设置卡片密码 */
	if (66 == write_position)
	{
		if (cnt > sizeof(card_cipher))
		{
			return copy_from_user(card_cipher, buf, sizeof(card_cipher));
		}
		return copy_from_user(card_cipher, buf, cnt);
	}
 
	/* 验证卡片密码 */
	res = rc522_auth_state(rc522_dev, card_auth_mode, write_position, card_cipher, card_id);
	if (MI_OK != res)
	{
		pr_info("Verification card password setting error when writing\n");
		return MI_NOTAGERR;
	}
 
	/* 向指定块中写数据 */
	memset(card_data, write_position, sizeof(card_data));
	if (cnt > sizeof(card_data))
	{
		res = copy_from_user(card_data, buf, sizeof(card_data));
	}
	else
	{
		res = copy_from_user(card_data, buf, cnt);
	}
	if (MI_OK != res)
	{
		return MI_NOTAGERR;
	}
	
    return rc522_write_card(rc522_dev, 6, card_data);
}
 
/**
 * @brief 关闭/释放设备
 * 
 * @param filp 要关闭的设备文件(文件描述符)
 * @return 0 成功;其他 失败
*/
static int rc522_release(struct inode *inode, struct file *filp)
{
	int res = MI_OK;
	struct rc522_dev_s *rc522_dev = filp->private_data;
	// pr_info("rc522_release\n");
 
	/* 复位 RC522 */
	res = rc522_reset_dev(rc522_dev);
	if (MI_OK != res)
	{
		return MI_NOTAGERR;
	}
 
	/* 卡片进入休眠 */
    return rc522_halt_card(rc522_dev);
}
 
/**
 * @brief 修改文件读写的偏移位置
 * 
 * @param filp 要关闭的设备文件(文件描述符)
 * @param loff_t 偏移位置
 * @param whence 文件位置
 * @return 0 成功;其他 失败
*/
loff_t file_llseek (struct file *filp, loff_t offset, int whence)
{
	loff_t new_pos; 				//新偏移量
	loff_t old_pos = filp->f_pos; 	//旧偏移量
 
	// pr_info("file llseek !\n");
	switch(whence){
	case SEEK_SET:
		new_pos = offset;
		break;
	case SEEK_CUR:
		new_pos = old_pos + offset;
		break;
	case SEEK_END:
		new_pos = RC522_MAX_OFFSET + offset;
		break;
	default:
		printk("error: Unknow whence !\n");
		return - EINVAL;
	}
	
	/* 偏移量的合法检查 */
	if(new_pos < 0 || new_pos > RC522_MAX_OFFSET){ 	
		printk("error: Set offset error !\n");
		return - EINVAL;
	}
 
	filp->f_pos = new_pos;
	// printk("The new pos = %lld and offset = %lld!\n", new_pos, offset);
	
	return new_pos; 					//正确返回新的偏移量	
 }
 
/* 设备操作函数结构体 */
static struct file_operations rc522_ops = {
    .owner = THIS_MODULE, 
    .open = rc522_open,
    .read = rc522_read,
    .write = rc522_write,
    .release = rc522_release,
	.llseek = file_llseek,
};
 
/**
 * @brief spi 驱动的 probe 函数，当驱动与设备匹配以后此函数就会执行
 * @param client spi 设备
 * @param id spi 设备 ID
 * @return 0，成功;其他负值,失败
*/
static int rc522_probe(struct spi_device *spi)
{
    int ret = -1; 							// 保存错误状态码
	struct rc522_dev_s *rc522_dev;			// 设备数据结构体
 
	/*---------------------注册字符设备驱动-----------------*/ 
	
	/* 驱动与总线设备匹配成功 */
	pr_info("\t  %s match successed  \r\n", spi->modalias);
	// dev_info(&spi->dev, "match successed\n");
 
	/* 申请内存并与 client->dev 进行绑定。*/
	/* 在 probe 函数中使用时，当设备驱动被卸载，该内存被自动释放，也可使用 devm_kfree() 函数直接释放 */
	rc522_dev = devm_kzalloc(&spi->dev, sizeof(*rc522_dev), GFP_KERNEL);
	if(!rc522_dev)
	{
		pr_err("Failed to request memory \r\n");
		return -ENOMEM;
	}
	/* 1、创建设备号 */
	/* 采用动态分配的方式，获取设备编号，次设备号为0 */
	/* 设备名称为 SPI_NAME，可通过命令 cat /proc/devices 查看 */
	/* RC522_CNT 为1，只申请一个设备编号 */
	ret = alloc_chrdev_region(&rc522_dev->devid, 0, RC522_CNT, RC522_NAME);
	if (ret < 0)
	{
		pr_err("%s Couldn't alloc_chrdev_region, ret = %d \r\n", RC522_NAME, ret);
		return -ENOMEM;
	}
 
	/* 2、初始化 cdev */
	/* 关联字符设备结构体 cdev 与文件操作结构体 file_operations */
	rc522_dev->cdev.owner = THIS_MODULE;
	cdev_init(&rc522_dev->cdev, &rc522_ops);
 
	/* 3、添加一个 cdev */
	/* 添加设备至cdev_map散列表中 */
	ret = cdev_add(&rc522_dev->cdev, rc522_dev->devid, RC522_CNT);
	if (ret < 0)
	{
		pr_err("fail to add cdev \r\n");
		goto del_unregister;
	}
 
	/* 4、创建类 */
	rc522_dev->class = class_create(THIS_MODULE, RC522_NAME);
	if (IS_ERR(rc522_dev->class)) 
	{
		pr_err("Failed to create device class \r\n");
		goto del_cdev;
	}
 
	/* 5、创建设备,设备名是 RC522_NAME */
	/*创建设备 RC522_NAME 指定设备名，*/
	rc522_dev->device = device_create(rc522_dev->class, NULL, rc522_dev->devid, NULL, RC522_NAME);
	if (IS_ERR(rc522_dev->device)) {
		goto destroy_class;
	}
	rc522_dev->spi = spi;
 
	/*初始化 rc522_device */
	spi->mode = SPI_MODE_0; /*MODE0，CPOL=0，CPHA=0*/
	spi_setup(spi);
 
	/* 保存 rc522_dev 结构体 */
	spi_set_drvdata(spi, rc522_dev);
 
	return 0;
 
destroy_class:
	device_destroy(rc522_dev->class, rc522_dev->devid);
del_cdev:
	cdev_del(&rc522_dev->cdev);
del_unregister:
	unregister_chrdev_region(rc522_dev->devid, RC522_CNT);
	return -EIO;
}
 
/**
 * @brief spi 驱动的 remove 函数，移除 spi 驱动的时候此函数会执行
 * @param client spi 设备
 * @return 0，成功;其他负值,失败
*/
static int rc522_remove(struct spi_device *spi)
{
	struct rc522_dev_s *rc522_dev = spi_get_drvdata(spi);
 
	/*---------------------注销字符设备驱动-----------------*/
 
	/* 1、删除 cdev */
	cdev_del(&rc522_dev->cdev);
	/* 2、注销设备号 */
	unregister_chrdev_region(rc522_dev->devid, RC522_CNT);
	/* 3、注销设备 */
	device_destroy(rc522_dev->class, rc522_dev->devid);
	/* 4、注销类 */
	class_destroy(rc522_dev->class);
	return 0;
}
 
/* 传统匹配方式 ID 列表 */
static const struct spi_device_id gtp_device_id[] = {
	{"rfid,rfid_rc522", 0},
	{}
};
 
/* 设备树匹配表 */
static const struct of_device_id rc522_of_match_table[] = {
	{.compatible = "rfid,rfid_rc522"},
	{/* sentinel */}
};
 
/* SPI 驱动结构体 */
static struct spi_driver rc522_driver = {
	.probe = rc522_probe,
	.remove = rc522_remove,
	.id_table = gtp_device_id,
	.driver = {
		.name = "rfid,rfid_rc522",
		.owner = THIS_MODULE,
		.of_match_table = rc522_of_match_table,
	},
};
 
/**
 * @brief 驱动入口函数
 * @return 0，成功;其他负值,失败
*/
static int __init rc522_driver_init(void)
{
	int ret;
	pr_info("spi_driver_init\n");
	ret = spi_register_driver(&rc522_driver);
	return ret;
}
 
/**
 * @brief 驱动出口函数
 * @return 0，成功;其他负值,失败
*/
static void __exit rc522_driver_exit(void)
{
	pr_info("spi_driver_exit\n");
	spi_unregister_driver(&rc522_driver);
}
 
 
/* 将上面两个函数指定为驱动的入口和出口函数 */
module_init(rc522_driver_init);
module_exit(rc522_driver_exit);
 
/* LICENSE 和作者信息 */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("JIAOZHU");
MODULE_INFO(intree, "Y");
 
