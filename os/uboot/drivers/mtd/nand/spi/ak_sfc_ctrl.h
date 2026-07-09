/**
* @FILENAME: sfc_ctrl.h
* @BRIEF SPI 控制器头文件
* Copyright (C) 2024 Anyka (ShenZhen) Micro-Electronic Technology Co., LTD
* @AUTHOR ZouTianxiang
* @DATA 2024-04-28
* @VERSION 1.0
* @REF please refer to...
*/


#ifndef _SFC_CONTROLLER_H_
#define _SFC_CONTROLLER_H_

#include "AK_types.h"


#define SFC_ADDRESS                       (0x20120000)
#define SFC_FIFO_THRES_STATUS_REG         (0x0c0+SFC_ADDRESS)
#define SFC_STATUS_REG                    (0x0d0+SFC_ADDRESS)
#define SFC_INTR_ENABLE_REG               (0x0d4+SFC_ADDRESS)
#define SFC_WORK_MODE_REG                 (0x100+SFC_ADDRESS)
#define SFC_CS_TIMING_REG                 (0x104+SFC_ADDRESS)
#define SFC_BAUD_RATE_REG                 (0x108+SFC_ADDRESS)  //此寄存器的相邻写操作应间隔1us及以上时间
#define SFC_WAIT_PHASE_REG                (0x10c+SFC_ADDRESS)
#define SFC_XIP_TIMEOUT_REG               (0x110+SFC_ADDRESS)
#define SFC_RXDS_CFG_REG0                 (0x114+SFC_ADDRESS)
#define SFC_RXDS_CFG_REG1                 (0x118+SFC_ADDRESS)
#define SFC_RXDS_CFG_REG2                 (0x11c+SFC_ADDRESS)
#define SFC_DMA_CFG_REG0                  (0x120+SFC_ADDRESS)
#define SFC_DMA_CFG_REG1                  (0x124+SFC_ADDRESS)
#define SFC_DATA_ENCRY_KEY_REG0           (0x130+SFC_ADDRESS)
#define SFC_DATA_ENCRY_KEY_REG1           (0x134+SFC_ADDRESS)
#define SFC_DATA_ENCRY_KEY_REG2           (0x138+SFC_ADDRESS)
#define SFC_DATA_ENCRY_KEY_REG3           (0x13c+SFC_ADDRESS)
#define SFC_DATA_ENCRY_ROUND_REG          (0x140+SFC_ADDRESS)
#define SFC_TRANSFER_ENABLE_CFG_REG       (0x180+SFC_ADDRESS)
#define SFC_DATA_FIFO_REG                 (0x184+SFC_ADDRESS)
#define SFC_DMA_START_ADDR_REG            (0x188+SFC_ADDRESS)
#define SFC_DMA_LENGTH_REG                (0x190+SFC_ADDRESS)
#define SFC_TRANSFRE_ABORT_CFG_REG        (0x19c+SFC_ADDRESS)

#define SFC_XIP_MODE_ADDR_REG(num)        (0x40+SFC_ADDRESS+num*16)
#define SFC_XIP_MODE_SIZE_REG(num)        (0x48+SFC_ADDRESS+num*16)
#define SFC_XIP_MODE_FLASH_ADDR_REG(num)  (0x4c+SFC_ADDRESS+num*16)
#define SFC_INST_PHASE_REG(num)           (0x200+SFC_ADDRESS+num*0x40)
#define SFC_ADDR_PHASE_REG(num)           (0x204+SFC_ADDRESS+num*0x40)
#define SFC_MODE_PHASE_REG(num)           (0x208+SFC_ADDRESS+num*0x40)
#define SFC_DATA_PHASE_REG(num)           (0x20c+SFC_ADDRESS+num*0x40)
#define SFC_POLLING_MASK_REG(num)         (0x218+SFC_ADDRESS+num*0x40)
#define SFC_POLLING_DATA_REG(num)         (0x21c+SFC_ADDRESS+num*0x40)
#define SFC_INTERFACE_CFG_REG(num)        (0x220+SFC_ADDRESS+num*0x40)
#define SFC_PHASE_LENGTH_CFG_REG(num)     (0x224+SFC_ADDRESS+num*0x40)
#define SFC_INTERVAL_CFG_REG(num)         (0x22c+SFC_ADDRESS+num*0x40)


//硬件配置的TX, RX FIFO深度
#define SFC_RX_FIFO_DEPTH                 8
#define SFC_TX_FIFO_DEPTH                 8





//SPI 数据传输线宽
typedef enum _SFC_BUS_WIDTH
{
    SFC_BUS_1_WIRE = 0,
    SFC_BUS_2_WIRE,
    SFC_BUS_4_WIRE,
    SFC_BUS_8_WIRE,
}T_SFC_BUS_WIDTH;




//phase 传输线宽设置
#define SFC_INST_BUSWIDTH_CFG(bus_width)        ((bus_width)<<0x0)     //命令阶段线宽
#define SFC_ADDR_MODE_BUSWIDTH_CFG(bus_width)   ((bus_width)<<0x2)     //地址& 模式阶段线宽
#define SFC_DATA_BUSWIDTH_CFG(bus_width)        ((bus_width)<<0x4)     //数据阶段线宽








//SPI  传输阶段使能
typedef enum _SFC_PHASE_ENA
{
    SFC_INST_PHASE_ENA = (0x1<<6),
    SFC_ADDR_PHASE_ENA = (0x1<<7),
    SFC_MODE_PHASE_ENA = (0x1<<8),
    SFC_WAIT_PHASE_ENA = (0x1<<9),
    SFC_DATA_PHASE_ENA = (0x1<<10),
}T_SFC_PHASE_ENA;



//SPI  传输双沿使能
typedef enum _SFC_DTR_ENA
{
    SFC_INST_DTR_ENA = (0x1<<11),
    SFC_ADDR_MODE_DTR_ENA = (0x1<<12),
    SFC_DATA_DTR_ENA = (0x1<<13),
}T_SFC_DTR_ENA;



//SPI  传输方向
typedef enum _SFC_DATA_DIR
{
    SFC_DATA_DIR_RX = (0x0<<14),
    SFC_DATA_DIR_TX = (0x1<<14),
}T_SFC_DATA_DIR;



//SPI  传输RXDS使能
typedef enum _SFC_RXDS
{
    SFC_RXDS_DIS = (0x0<<15),
    SFC_RXDS_ENA = (0x1<<15),
}T_SFC_RXDS;



//SPI  传输加密使能
typedef enum _SFC_CRYPTO
{
    SFC_CRYPTO_DIS = (0x0<<16),
    SFC_CRYPTO_ENA = (0x1<<16),
}T_SFC_CRYPTO;




//SPI  传输CS选择
typedef enum _SFC_CS_SEL
{
    SFC_CS0_SEL = (0x0<<20),
    SFC_CS1_SEL = (0x1<<20),
}T_SFC_CS_SEL;



//SPI  轮询状态比对模式
typedef enum _SFC_POLLING_COMPARE_MODE
{
    SFC_POLLING_AND_MODE = (0x0<<26),
    SFC_POLLING_OR_MODE = (0x1<<26),
}T_SFC_POLLING_COMPARE_MODE;



//SPI  轮询使能
typedef enum _SFC_POLLING_ENA
{
    SFC_POLLING_DIS = (0x0<<27),
    SFC_POLLING_ENA = (0x1<<27),
}T_SFC_POLLING_ENA;





//SPI  数据传输阶段存放数据的地方
typedef enum _SFC_DATA_PHASE_TARGET
{
    SFC_DATA_PHASE_FIFO = (0x0<<28),    //存放在FIFO
    SFC_DATA_PHASE_REG = (0x1<<28),     //存放在寄存器
}T_SFC_DATA_PHASE_TARGET;



//phase 传输长度设置
#define SFC_INST_LEN_CFG(len)           ((len-1)<<0x0)          //命令长度
#define SFC_ADDR_LEN_CFG(len)           ((len-1)<<0x2)          //地址长度
#define SFC_MODE_LEN_CFG(len)           ((len-1)<<0x4)          //模式长度
#define SFC_DATA_LEN_CFG(len)           ((len-1)<<0x6)          //数据长度


//dummy cycle 设置
#define SFC_DUMMY_CYCLE_CFG(cycle_num)  ((cycle_num-1)<<0x0)     //命令长度


//TX 阈值设置
#define SFC_TX_THRESHOLD_CFG(tx_threshold_val)  (tx_threshold_val<<0x0)     
//RX 阈值设置
#define SFC_RX_THRESHOLD_CFG(rx_threshold_val)  (rx_threshold_val<<0x8)    



#endif

