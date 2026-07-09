/**
* #FILENAME: spi_nand.h
* #BRIEF    spi_nand.h
* Copyright (C) 2015 Anyka (ShenZhen) Micro-Electronic Technology Co., LTD
* #AUTHOR ZOU TIANXIANG
* #DATE 2015-03-25
* #VERSION 1.0
* #REF Please refer to��

*********************************************************/


#ifndef _SPI_NAND_H_
#define _SPI_NAND_H_

#include "AK_types.h"

//ÿ��PAGE��SIZE
#define SPI_NAND_PAGE_SIZE              2048
//1��BLOCK��������ҳ��
#define SPI_NAND_PAGE_CNT_PER_BLOCK     64
//ÿ��BLOCK��SIZE
#define SPI_NAND_BLOCK_SIZE             (SPI_NAND_PAGE_SIZE*SPI_NAND_PAGE_CNT_PER_BLOCK)




//Sfc CLK 
#define SFC_IO_CLK_50MHZ        50000000
#define SFC_IO_CLK_40MHZ        40000000
#define SFC_IO_CLK_30MHZ        30000000
#define SFC_IO_CLK_10MHZ        10000000



typedef enum
{
    PROGRAM_X1_MODE = 0,
    PROGRAM_X4_MODE
}T_PROGRAM_MODE;


typedef enum
{
    READ_X1_MODE = 0,
    READ_X2_MODE,   
    READ_X4_MODE
}T_READ_MODE;


/**
  Function:       T_VOID spi_nand_reset()
  Description:   spi nand reset
  Input:           
  Output:         
  Return:         
  Date      2015-03-25
  Author:         Zou Tianxiang
**/
void spi_nand_reset();



/**
  Function:       T_VOID spi_nand_rdid()
  Description:   spi nand read id
  Input:           
  Output:         
  Return:         
  Date      2015-03-24
  Author:         Zou Tianxiang
**/
void spi_nand_rdid(T_U8 *data);



/**
  Function:       T_VOID spi_nand_get_features_status()
  Description:   spi nand get_features_status
  Input:           
  Output:         
  Return:         
  Date      2015-03-24
  Author:         Zou Tianxiang
**/
T_U8 spi_nand_get_features_status();



/**
  Function:       T_VOID spi_nand_get_features_feature()
  Description:   spi nand get_features_featue
  Input:           
  Output:         
  Return:         
  Date      2015-03-24
  Author:         Zou Tianxiang
**/
T_U8 spi_nand_get_features_feature();



/**
  Function:       T_VOID spi_nand_get_features_protection()
  Description:   spi nand get_features_protection
  Input:           
  Output:         
  Return:         
  Date      2015-03-24
  Author:         Zou Tianxiang
**/
T_U8 spi_nand_get_features_protection();




/**
  Function:       T_VOID spi_nand_set_features_feature()
  Description:   spi nand set_features feature
  Input:           
  Output:         
  Return:         
  Date      2015-03-28
  Author:         Zou Tianxiang
**/
T_VOID spi_nand_set_features_feature(T_U8 val);





/**
  Function:       T_VOID spi_nand_set_features_protection()
  Description:   spi nand set_features protection
  Input:           
  Output:         
  Return:         
  Date      2015-03-28
  Author:         Zou Tianxiang
**/
T_VOID spi_nand_set_features_protection(T_U8 val);







/**
  Function:       T_BOOL spi_nand_erase_block(T_U32 block_num);
  Description:   spi nand erase block 
  Input:           T_U32 data
  Output:         
  Return:         
  Date      2015-03-25
  Author:         Zou Tianxiang
**/
T_BOOL spi_nand_erase_block(T_U32 block_num);





/**
  Function:       T_BOOL spi_nand_afd04gws_page_program()
  Description:   afd04gws page program  
  Input:          T_U32 page_addr, T_U8 *write_buf, T_U32 write_count, T_PROGRAM_MODE mode
  Output:         
  Return:         AK_TRUE,  AK_FALSE
  Date      2015-03-25
  Author:         Zou Tianxiang
**/
T_BOOL spi_nand_page_program(T_U32 page_addr, T_U8 *write_buf, T_U32 write_count, T_PROGRAM_MODE mode);



/**
  Function:      T_BOOL spi_nand_afd04gws_page_read()
  Description:  afd04gws page read
  Input:           T_U32 data
  Output:         
  Return:         AK_TRUE,  AK_FALSE
  Date        2015-03-25
  Author:         Zou Tianxiang
**/
T_BOOL spi_nand_page_read(T_U32 page_addr, T_U8 *read_buf, T_U32 read_count, T_READ_MODE mode);


T_BOOL ak_spinand_load_page_op(T_U32 page_addr);
T_BOOL ak_spinand_read_from_cache_op( T_U8 *read_buf, T_U32 read_count, T_READ_MODE mode);







#endif



