/*http://172.21.10.40:8080/icproduct/snowbirdc/SVT/**
* #FILENAME: spi_nand_AFD04GWS.c
* #BRIEF    spi nand flash AFD04GWS driver
* Copyright (C) 2015 Anyka (ShenZhen) Micro-Electronic Technology Co., LTD
* #AUTHOR ZOU TIANXIANG
* #DATE 2015-03-24
* #VERSION 1.0
* #REF Please refer to��

**/

#include "ak_spi_nand.h"
#include "ak_sfc_ctrl.h"
#include "console.h"
#include <common.h>


#define SPI_NAND_AFD04GWS_WREN			        	0x06		//WRITE ENABLE
#define SPI_NAND_AFD04GWS_WRDI			        	0x04		//WRITE DISABLE	
#define SPI_NAND_AFD04GWS_GET_FEATURES	        	0x0F		//GET FEATURES
#define SPI_NAND_AFD04GWS_SET_FEATURE	        	0x1F		//SET FEATURES
#define SPI_NAND_AFD04GWS_PAGE_READ		        	0x13		//PAGE READ(TO CACHE)
#define SPI_NAND_AFD04GWS_READ_FROM_CACHE        	0x03		//READ FROM CACHE
#define SPI_NAND_AFD04GWS_READ_FROM_CACHE_X2     	0x3B		//READ FROM CACHE X2
#define SPI_NAND_AFD04GWS_READ_FROM_CACHE_X4     	0x6B		//READ FROM CACHE X4
#define SPI_NAND_AFD04GWS_READ_FROM_CACHE_DUAL   	0xBB		//READ FROM CACHE DUAL IO
#define SPI_NAND_AFD04GWS_READ_FROM_CACHE_QUAD   	0xEB		//READ FROM CACHE QUAD IO
#define SPI_NAND_AFD04GWS_RDID                   	0x9f		//READ ID
#define SPI_NAND_AFD04GWS_PROGRAM_LOAD           	0x02		//PROGRAM LOAD
#define SPI_NAND_AFD04GWS_PROGRAM_LOAD_X4        	0x32		//PROGRAM LOAD X4
#define SPI_NAND_AFD04GWS_PROGRAM_EXCUTE         	0x10		//PROGRAM EXCUTE
#define SPI_NAND_AFD04GWS_PROGRAM_LOAD_DATA      	0x84		//PROGRAM LOAD RANDOM DATA
#define SPI_NAND_AFD04GWS_PROGRAM_LOAD_DATA_X4   	0xC4		//PROGRAM LOAD RANDOM DATA X4
#define SPI_NAND_AFD04GWS_PROGRAM_LOAD_QUAD_IO   	0x72		//PROGRAM LOAD RANDOM DATA QUAD IO
#define SPI_NAND_AFD04GWS_BLOCK_ERASE				0xD8		//PROGRAM LOAD RANDOM DATA QUAD IO
#define SPI_NAND_AFD04GWS_RESET						0xFF		//RESET



#define	SPI_NAND_STATUS_OIP_BIT						(0x1<<0)	//OPERATION IN PROCESS
#define	SPI_NAND_STATUS_WEL_BIT						(0x1<<1)	//WRITE ENABLE BIT
#define	SPI_NAND_STATUS_EFAIL_BIT					(0x1<<2)	//ERASE FAILED BIT
#define	SPI_NAND_STATUS_PFAIL_BIT					(0x1<<3)	//ERASE FAILED BIT
#define	SPI_NAND_STATUS_ECC_PASS_BIT				(0x0<<4)	//ECC PASS BIT
#define	SPI_NAND_STATUS_ECC_REPARE_BIT				(0x1<<4)	//ECC CAN REPARE BIT
#define	SPI_NAND_STATUS_ECC_FAILED_BIT				(0x2<<4)	//ECC FAILED
#define	SPI_NAND_STATUS_ECC_REPARE1_BIT				(0x3<<4)	//ECC CAN REPARE  8 Bit




typedef enum
{
	SPI_NAND_AFD04GWS_STATUS_ADDR = 0xC0,
	SPI_NAND_AFD04GWS_FEATURE_ADDR = 0xB0,
	SPI_NAND_AFD04GWS_PROTECTION_ADDR = 0xA0,
}T_SPI_NAND_AFD04GWS_FEATURES;




static void spi_nand_afd04gws_wait_status_oip();
static T_BOOL spi_nand_afd04gws_check_erase_ok();







/**
  Function:       T_VOID spi_nand_reset()
  Description:   spi nand reset
  Input:           
  Output:         
  Return:         
  Date 		2015-03-25
  Author:         Zou Tianxiang
**/
void spi_nand_reset()
{
    //1��FF����
    REG32(SFC_INST_PHASE_REG(0)) = SPI_NAND_AFD04GWS_RESET;
    //ֻʹ��1�߷�����
    REG32(SFC_INTERFACE_CFG_REG(0)) = (SFC_INST_BUSWIDTH_CFG(SFC_BUS_1_WIRE) | SFC_INST_PHASE_ENA \
                                        | SFC_DATA_DIR_TX);
    //�����1
    REG32(SFC_PHASE_LENGTH_CFG_REG(0)) = SFC_INST_LEN_CFG(1);
    
    //dummy  CYCLE����(û��Wait phase, ��������)
    //REG32(SFC_WAIT_PHASE_REG) = 0;
    
    //�������䲢�ȴ�����
    sfc_cpu_trans_start_and_wait_finish();

    // �ȴ�OIP(Operation in progress) ����
    spi_nand_afd04gws_wait_status_oip();
    
}





/**
  Function:       T_VOID spi_nand_rdid()
  Description:   spi nand read id
  Input:           
  Output:         
  Return:         
  Date 		2015-03-24
  Author:         Zou Tianxiang
**/
void spi_nand_rdid(T_U8 *data)
{
    T_U32 reg_val;
    T_U8 manufacturer_id;
    T_U8 device_id;
       
    //��������
    REG32(SFC_INST_PHASE_REG(0)) = SPI_NAND_AFD04GWS_RDID;

    //��ַ����
    REG32(SFC_ADDR_PHASE_REG(0)) = 0;

    //��Ҫ�����ַ�� ���ݾ�ʹ��1�߷�,RX
    REG32(SFC_INTERFACE_CFG_REG(0)) = (SFC_INST_BUSWIDTH_CFG(SFC_BUS_1_WIRE) | SFC_INST_PHASE_ENA \
                                 | SFC_ADDR_MODE_BUSWIDTH_CFG(SFC_BUS_1_WIRE) | SFC_ADDR_PHASE_ENA \
                                 | SFC_DATA_BUSWIDTH_CFG(SFC_BUS_1_WIRE) | SFC_DATA_PHASE_ENA \
                                 | SFC_DATA_DIR_RX);

    //�����1 cycle + ��ַ����1 cycle  + DATA ����1 cycle 
    REG32(SFC_PHASE_LENGTH_CFG_REG(0)) = (SFC_INST_LEN_CFG(1) | SFC_ADDR_LEN_CFG(1) | SFC_DATA_LEN_CFG(2));

    //dummy  CYCLE����(û��Wait phase, ��������)
    //REG32(SFC_WAIT_PHASE_REG) = 0;

    //���FIFO
    REG32(SFC_WORK_MODE_REG) |= (0x1<<4);
    while(1)
    {
        if((REG32(SFC_WORK_MODE_REG) & (0x1<<4)) == 0)
        {
            break;
        }
    }

    //�������䲢�ȴ�����
    sfc_cpu_trans_start_and_wait_finish();

    //��FIFO����
    reg_val = REG32(SFC_DATA_FIFO_REG);

    //ȡ�� manufacturer_id��device_id
    manufacturer_id = reg_val & 0xff;
    device_id = (reg_val>>8) & 0xff;
    //printf("manufacturer_id = %x, device_id0 = %x\n", manufacturer_id, device_id);
    data[0]=manufacturer_id;
    data[1]=device_id;
    
}



/**
  Function:       T_VOID spi_nand_afd04gws_wren()
  Description:   spi nand afd04gws send wren
  Input:           
  Output:         
  Return:         
  Date 		2015-03-24
  Author:         Zou Tianxiang
**/
static void spi_nand_afd04gws_wren()
{

    T_U8 cmd;

    //WREN
    cmd = SPI_NAND_AFD04GWS_WREN;

    //��������
    REG32(SFC_INST_PHASE_REG(0)) = cmd;

    //��Ҫ����ʹ��1�߷�
    //�ĵ���˵����û��DATA PHYASEʱ��������BIT14
    REG32(SFC_INTERFACE_CFG_REG(0)) = (SFC_INST_BUSWIDTH_CFG(SFC_BUS_1_WIRE) | SFC_INST_PHASE_ENA \
                                       | SFC_DATA_DIR_TX );

    //�����1 cycle 
    REG32(SFC_PHASE_LENGTH_CFG_REG(0)) = SFC_INST_LEN_CFG(1);

    //dummy  CYCLE����(û��Wait phase, ��������)
    //REG32(SFC_WAIT_PHASE_REG) = 0;
    
    //�������䲢�ȴ�����
    sfc_cpu_trans_start_and_wait_finish();

}




/**
  Function:       T_U8spi_nand_get_features_status()
  Description:   spi nand afd04gws get_features_status
  Input:           
  Output:         
  Return:         
  Date 		2015-03-24
  Author:         Zou Tianxiang
**/
T_U8 spi_nand_get_features_status()
{
    T_U8 status;
    
    //��������
    REG32(SFC_INST_PHASE_REG(0)) = SPI_NAND_AFD04GWS_GET_FEATURES;

    //��ַ����
    REG32(SFC_ADDR_PHASE_REG(0)) = SPI_NAND_AFD04GWS_STATUS_ADDR;

    //��Ҫ�����ַ�� ���ݾ�ʹ��1�߷�,RX
    REG32(SFC_INTERFACE_CFG_REG(0)) = (SFC_INST_BUSWIDTH_CFG(SFC_BUS_1_WIRE) | SFC_INST_PHASE_ENA \
                                  | SFC_ADDR_MODE_BUSWIDTH_CFG(SFC_BUS_1_WIRE) | SFC_ADDR_PHASE_ENA \
                                  | SFC_DATA_BUSWIDTH_CFG(SFC_BUS_1_WIRE) | SFC_DATA_PHASE_ENA \
                                  | SFC_DATA_DIR_RX);

    //�����1 cycle + ��ַ����1 cycle  + DATA ����1 cycle 
    REG32(SFC_PHASE_LENGTH_CFG_REG(0)) = (SFC_INST_LEN_CFG(1) | SFC_ADDR_LEN_CFG(1) | SFC_DATA_LEN_CFG(1));

    //dummy  CYCLE����(û��Wait phase, ��������)
    //REG32(SFC_WAIT_PHASE_REG) = 0;

    //���FIFO
    REG32(SFC_WORK_MODE_REG) |= (0x1<<4);
    while(1)
    {
        if((REG32(SFC_WORK_MODE_REG) & (0x1<<4)) == 0)
        {
            break;
        }
    }
    
    //�������䲢�ȴ�����
    sfc_cpu_trans_start_and_wait_finish();

    //��FIFO����
    status = (REG32(SFC_DATA_FIFO_REG) & 0xff);

    return status;

}


/**
  Function:       T_U8 spi_nand_get_features_feature()
  Description:   spi nand get_features_featue
  Input:           
  Output:         
  Return:         
  Date 		2015-03-24
  Author:         Zou Tianxiang
**/
T_U8 spi_nand_get_features_feature()
{
	T_U8 status;
    
    //��������
    REG32(SFC_INST_PHASE_REG(0)) = SPI_NAND_AFD04GWS_GET_FEATURES;

    //��ַ����
    REG32(SFC_ADDR_PHASE_REG(0)) = SPI_NAND_AFD04GWS_FEATURE_ADDR;

    //��Ҫ�����ַ�� ���ݾ�ʹ��1�߷�,RX
    REG32(SFC_INTERFACE_CFG_REG(0)) = (SFC_INST_BUSWIDTH_CFG(SFC_BUS_1_WIRE) | SFC_INST_PHASE_ENA \
                                  | SFC_ADDR_MODE_BUSWIDTH_CFG(SFC_BUS_1_WIRE) | SFC_ADDR_PHASE_ENA \
                                  | SFC_DATA_BUSWIDTH_CFG(SFC_BUS_1_WIRE) | SFC_DATA_PHASE_ENA \
                                  | SFC_DATA_DIR_RX);

    //�����1 cycle + ��ַ����1 cycle  + DATA ����1 cycle 
    REG32(SFC_PHASE_LENGTH_CFG_REG(0)) = (SFC_INST_LEN_CFG(1) | SFC_ADDR_LEN_CFG(1) | SFC_DATA_LEN_CFG(1));

    //dummy  CYCLE����(û��Wait phase, ��������)
    //REG32(SFC_WAIT_PHASE_REG) = 0;

    //���FIFO
    REG32(SFC_WORK_MODE_REG) |= (0x1<<4);
    while(1)
    {
        if((REG32(SFC_WORK_MODE_REG) & (0x1<<4)) == 0)
        {
            break;
        }
    }
    
    //�������䲢�ȴ�����
    sfc_cpu_trans_start_and_wait_finish();

    //��FIFO����
    status = (REG32(SFC_DATA_FIFO_REG) & 0xff);

    return status;

}



/**
  Function:       T_U8 spi_nand_get_features_protection()
  Description:   spi nand get_features_protection
  Input:           
  Output:         
  Return:         
  Date 		2015-03-24
  Author:         Zou Tianxiang
**/
T_U8 spi_nand_get_features_protection()
{
	T_U8 status;
    
    //��������
    REG32(SFC_INST_PHASE_REG(0)) = SPI_NAND_AFD04GWS_GET_FEATURES;

    //��ַ����
    REG32(SFC_ADDR_PHASE_REG(0)) = SPI_NAND_AFD04GWS_PROTECTION_ADDR;

    //��Ҫ�����ַ�� ���ݾ�ʹ��1�߷�,RX
    REG32(SFC_INTERFACE_CFG_REG(0)) = (SFC_INST_BUSWIDTH_CFG(SFC_BUS_1_WIRE) | SFC_INST_PHASE_ENA \
                                  | SFC_ADDR_MODE_BUSWIDTH_CFG(SFC_BUS_1_WIRE) | SFC_ADDR_PHASE_ENA \
                                  | SFC_DATA_BUSWIDTH_CFG(SFC_BUS_1_WIRE) | SFC_DATA_PHASE_ENA \
                                  | SFC_DATA_DIR_RX);

    //�����1 cycle + ��ַ����1 cycle  + DATA ����1 cycle 
    REG32(SFC_PHASE_LENGTH_CFG_REG(0)) = (SFC_INST_LEN_CFG(1) | SFC_ADDR_LEN_CFG(1) | SFC_DATA_LEN_CFG(1));

    //dummy  CYCLE����(û��Wait phase, ��������)
    //REG32(SFC_WAIT_PHASE_REG) = 0;

    //���FIFO
    REG32(SFC_WORK_MODE_REG) |= (0x1<<4);
    while(1)
    {
        if((REG32(SFC_WORK_MODE_REG) & (0x1<<4)) == 0)
        {
            break;
        }
    }
    
    //�������䲢�ȴ�����
    sfc_cpu_trans_start_and_wait_finish();

    //��FIFO����
    status = (REG32(SFC_DATA_FIFO_REG) & 0xff);

    return status;

}





/**
  Function:       T_VOID spi_nand_set_features_feature()
  Description:   spi nand set_features
  Input:           
  Output:         
  Return:         
  Date 		2015-03-28
  Author:         Zou Tianxiang
**/
T_VOID spi_nand_set_features_feature(T_U8 val)
{
    //��������
    REG32(SFC_INST_PHASE_REG(0)) = SPI_NAND_AFD04GWS_SET_FEATURE;

    //��ַ����
    REG32(SFC_ADDR_PHASE_REG(0)) = SPI_NAND_AFD04GWS_FEATURE_ADDR;

    //��Ҫ�����ַ�� ���ݾ�ʹ��1�߷�,TX DATA
    REG32(SFC_INTERFACE_CFG_REG(0)) = (SFC_INST_BUSWIDTH_CFG(SFC_BUS_1_WIRE) | SFC_INST_PHASE_ENA \
                                  | SFC_ADDR_MODE_BUSWIDTH_CFG(SFC_BUS_1_WIRE) | SFC_ADDR_PHASE_ENA \
                                  | SFC_DATA_BUSWIDTH_CFG(SFC_BUS_1_WIRE) | SFC_DATA_PHASE_ENA \
                                  | SFC_DATA_DIR_TX);

    //�����1 cycle + ��ַ����1 cycle  + DATA ����1 cycle 
    REG32(SFC_PHASE_LENGTH_CFG_REG(0)) = (SFC_INST_LEN_CFG(1) | SFC_ADDR_LEN_CFG(1) | SFC_DATA_LEN_CFG(1));

    //dummy  CYCLE����(û��Wait phase, ��������)
    //REG32(SFC_WAIT_PHASE_REG) = 0;

    //���FIFO
    REG32(SFC_WORK_MODE_REG) |= (0x1<<4);
    while(1)
    {
        if((REG32(SFC_WORK_MODE_REG) & (0x1<<4)) == 0)
        {
            break;
        }
    }

    //����TX FIFO
    REG32(SFC_DATA_FIFO_REG) = ((T_U32)val & 0xff);
    
    //�������䲢�ȴ�����
    sfc_cpu_trans_start_and_wait_finish();
    
}




/**
  Function:       T_VOID spi_nand_set_features_protection()
  Description:   
  Input:           T_U8 val
  Output:         
  Return:         
  Date 		2015-03-28
  Author:         Zou Tianxiang
**/
T_VOID spi_nand_set_features_protection(T_U8 val)
{
    //��������
    REG32(SFC_INST_PHASE_REG(0)) = SPI_NAND_AFD04GWS_SET_FEATURE;

    //��ַ����
    REG32(SFC_ADDR_PHASE_REG(0)) = SPI_NAND_AFD04GWS_PROTECTION_ADDR;

    //��Ҫ�����ַ�� ���ݾ�ʹ��1�߷�,TX DATA
    REG32(SFC_INTERFACE_CFG_REG(0)) = (SFC_INST_BUSWIDTH_CFG(SFC_BUS_1_WIRE) | SFC_INST_PHASE_ENA \
                                  | SFC_ADDR_MODE_BUSWIDTH_CFG(SFC_BUS_1_WIRE) | SFC_ADDR_PHASE_ENA \
                                  | SFC_DATA_BUSWIDTH_CFG(SFC_BUS_1_WIRE) | SFC_DATA_PHASE_ENA \
                                  | SFC_DATA_DIR_TX);

    //�����1 cycle + ��ַ����1 cycle  + DATA ����1 cycle 
    REG32(SFC_PHASE_LENGTH_CFG_REG(0)) = (SFC_INST_LEN_CFG(1) | SFC_ADDR_LEN_CFG(1) | SFC_DATA_LEN_CFG(1));

    //dummy  CYCLE����(û��Wait phase, ��������)
    //REG32(SFC_WAIT_PHASE_REG) = 0;

    //���FIFO
    REG32(SFC_WORK_MODE_REG) |= (0x1<<4);
    while(1)
    {
        if((REG32(SFC_WORK_MODE_REG) & (0x1<<4)) == 0)
        {
            break;
        }
    }

    //����TX FIFO
    REG32(SFC_DATA_FIFO_REG) = ((T_U32)val & 0xff);
    
    //�������䲢�ȴ�����
    sfc_cpu_trans_start_and_wait_finish();

}




/**
  Function:      static T_VOID spi_nand_afd04gws_wait_status_oip()
  Description:  spi afd04gws wait status oip bit
  Input:           
  Output:         
  Return:         
  Date 		  2015-03-24
  Author:         Zou Tianxiang
  Note:            �ȴ�OIP(Operation in progress) ����
**/
static void spi_nand_afd04gws_wait_status_oip()
{
	T_U8 status;

	while(1)
	{
		status = spi_nand_get_features_status();
		//printf("status = %x\n", status);
		if( (status & SPI_NAND_STATUS_OIP_BIT) == 0)
		{
			break;
		}
	}
}






/**
  Function:       T_VOID spi_nand_erase_block()
  Description:   spi nand erase block 
  Input:           T_U32 data
  Output:         
  Return:         
  Date 		2015-03-25
  Author:         Zou Tianxiang
**/
T_BOOL spi_nand_erase_block(T_U32 block_num)
{
	T_U32 addr;
	T_BOOL ret;

	addr = block_num * 64;

    //��WREN
	spi_nand_afd04gws_wren();
    
    //��������
    REG32(SFC_INST_PHASE_REG(0)) = SPI_NAND_AFD04GWS_BLOCK_ERASE;

    //��ַ����
    REG32(SFC_ADDR_PHASE_REG(0)) = addr;

    //��Ҫ�����ַ�� ��ʹ��1�߷�
    REG32(SFC_INTERFACE_CFG_REG(0)) = (SFC_INST_BUSWIDTH_CFG(SFC_BUS_1_WIRE) | SFC_INST_PHASE_ENA \
                                  | SFC_ADDR_MODE_BUSWIDTH_CFG(SFC_BUS_1_WIRE) | SFC_ADDR_PHASE_ENA \
                                  | SFC_DATA_DIR_TX );

    //�����1 cycle + ��ַ���� cycle  
    REG32(SFC_PHASE_LENGTH_CFG_REG(0)) = (SFC_INST_LEN_CFG(1) | SFC_ADDR_LEN_CFG(3) );

    //dummy  CYCLE����(û��Wait phase, ��������)
    //REG32(SFC_WAIT_PHASE_REG) = 0;

    //�������䲢�ȴ�����
    sfc_cpu_trans_start_and_wait_finish();

	ret = spi_nand_afd04gws_check_erase_ok();
	
	return ret;
	
		
}



/**
  Function:       static T_VOID spi_nand_afd04gws_check_erase()
  Description:   spi nand afd04gws erase block 
  Input:           T_U32 data
  Output:         
  Return:         
  Date 		2015-03-25
  Author:         Zou Tianxiang
**/
static T_BOOL spi_nand_afd04gws_check_erase_ok()
{
	T_U8 status;

	while(1)
	{
		status = spi_nand_get_features_status();
		//printf("status = %x\n", status);
		if( (status & SPI_NAND_STATUS_OIP_BIT) == 0)
		{
			break;
		}
	}
	if(status & SPI_NAND_STATUS_EFAIL_BIT)
	{
		return AK_FALSE;
	}

	return AK_TRUE;
}




/**
  Function:       T_BOOL spi_nand_page_program()
  Description:   page program  
  Input:          T_U32 page_addr, T_U8 *write_buf, T_U32 write_count, T_PROGRAM_MODE mode
  Output:         
  Return:         AK_TRUE,  AK_FALSE
  Date 		2015-03-25
  Author:         Zou Tianxiang
**/
T_BOOL spi_nand_page_program(T_U32 page_addr, T_U8 *write_buf, T_U32 write_count, T_PROGRAM_MODE mode)
{
	T_U8 status;
    T_U8 cmd;
    T_U8 bus_width;

	//write enable
	spi_nand_afd04gws_wren();	//WINBOND, POWERCHIP , MICRON,  MACRONIX �⼸����˾��WRITE ENABLE������Ͷ��Ƿ�����ǰ��


    if( mode == PROGRAM_X1_MODE)
	{
	    cmd = SPI_NAND_AFD04GWS_PROGRAM_LOAD;
        bus_width = SFC_BUS_1_WIRE; 
	}
	else if( mode == PROGRAM_X4_MODE)
	{
    	//SEND PROGRAM LOAD X 4
		cmd = SPI_NAND_AFD04GWS_PROGRAM_LOAD_X4;
        bus_width = SFC_BUS_4_WIRE; 
	}

    
    //����ֵ����
    REG32(SFC_INST_PHASE_REG(0)) = cmd;

    //��ֵַ����
    REG32(SFC_ADDR_PHASE_REG(0)) = 0;

    //����/��ַ/����PHASE ��λ�����ã���������
    REG32(SFC_INTERFACE_CFG_REG(0)) = (SFC_INST_BUSWIDTH_CFG(SFC_BUS_1_WIRE) | SFC_INST_PHASE_ENA \
                                    | SFC_ADDR_MODE_BUSWIDTH_CFG(SFC_BUS_1_WIRE) | SFC_ADDR_PHASE_ENA \
                                    | SFC_DATA_BUSWIDTH_CFG(bus_width) | SFC_DATA_PHASE_ENA \
                                    | SFC_DATA_DIR_TX);
    
    //PHASE��������:  �����1 + ��ַ���� + ���ݳ���
    REG32(SFC_PHASE_LENGTH_CFG_REG(0)) = (SFC_INST_LEN_CFG(1) | SFC_ADDR_LEN_CFG(2) | \
                                          SFC_DATA_LEN_CFG(write_count)) ;
    
    //dummy  CYCLE����(û��Wait phase, ��������)
    //REG32(SFC_WAIT_PHASE_REG) = 0;

    //��FIFO��������
    sfc_send_dat(write_buf, write_count);


//-----------------------------------------------------------------------
    //��������
    REG32(SFC_INST_PHASE_REG(0)) = SPI_NAND_AFD04GWS_PROGRAM_EXCUTE;

    //��ַ����
    REG32(SFC_ADDR_PHASE_REG(0)) = page_addr;

    //��Ҫ�����ַ�� ��ʹ��1�߷�
    REG32(SFC_INTERFACE_CFG_REG(0)) = (SFC_INST_BUSWIDTH_CFG(SFC_BUS_1_WIRE) | SFC_INST_PHASE_ENA \
                                  | SFC_ADDR_MODE_BUSWIDTH_CFG(SFC_BUS_1_WIRE) | SFC_ADDR_PHASE_ENA \
                                  | SFC_DATA_DIR_TX );

    //�����1 cycle + ��ַ���� cycle  
    REG32(SFC_PHASE_LENGTH_CFG_REG(0)) = (SFC_INST_LEN_CFG(1) | SFC_ADDR_LEN_CFG(3) );

    //dummy  CYCLE����(û��Wait phase, ��������)
    //REG32(SFC_WAIT_PHASE_REG) = 0;

    //�������䲢�ȴ�����
    sfc_cpu_trans_start_and_wait_finish();


   // delay_us(1);        //delay tCS
    udelay(1);    

    //�ȴ�OIP����
    while(1)
    {
        status = spi_nand_get_features_status();
        //printf("status = %x\n", status);                  
        if( (status & SPI_NAND_STATUS_OIP_BIT) == 0)
        {
            break;
        }
    }

    if( status & SPI_NAND_STATUS_PFAIL_BIT )
	{
		return AK_FALSE;	
	}
	
    return AK_TRUE;

 
}


T_BOOL ak_spinand_load_page_op(T_U32 page_addr)
{
    T_U32   spi_nand_status;
    T_U32   return_val;
    T_U8    cmd;
    T_U8    bus_width;
    T_U8    status;

    return_val = AK_FALSE;
    

////////////////////////////////////// COM PAGE READ ////////////////////////////////////////////////
    //��PAGE ��CACHE ����
    //��������
    REG32(SFC_INST_PHASE_REG(0)) = SPI_NAND_AFD04GWS_PAGE_READ;

    //��ַ����
    REG32(SFC_ADDR_PHASE_REG(0)) = page_addr;

    //��Ҫ�����ַ�� ��ʹ��1�߷�
    REG32(SFC_INTERFACE_CFG_REG(0)) = (SFC_INST_BUSWIDTH_CFG(SFC_BUS_1_WIRE) | SFC_INST_PHASE_ENA \
                                       | SFC_ADDR_MODE_BUSWIDTH_CFG(SFC_BUS_1_WIRE) | SFC_ADDR_PHASE_ENA \
                                       | SFC_DATA_DIR_TX);

    //�����1 cycle + ��ַ����3 cycle(��ַ+ dummy)
    REG32(SFC_PHASE_LENGTH_CFG_REG(0)) = (SFC_INST_LEN_CFG(1) | SFC_ADDR_LEN_CFG(3));

    //dummy  CYCLE����(û��Wait phase, ��������)
    //REG32(SFC_WAIT_PHASE_REG) = 0;

    sfc_cpu_trans_start_and_wait_finish();

    return AK_TRUE;

}





T_BOOL ak_spinand_read_from_cache_op( T_U8 *read_buf, T_U32 read_count, T_READ_MODE mode)
{
    T_U32   spi_nand_status;
    T_U32   return_val;
    T_U8    cmd;
    T_U8    bus_width;
    T_U8    status;

    return_val = AK_FALSE;



////////////////////////////////////// �������� ////////////////////////////////////////
    if(mode == READ_X1_MODE)
    {
        //SEND READ FROM CACHE X 1
        cmd = SPI_NAND_AFD04GWS_READ_FROM_CACHE; 
        bus_width = SFC_BUS_1_WIRE;
    }

    else if(mode == READ_X2_MODE)
    {
        //SEND READ FROM CACHE X 2
        cmd = SPI_NAND_AFD04GWS_READ_FROM_CACHE_X2; 
        bus_width = SFC_BUS_2_WIRE;
    }

    else if(mode == READ_X4_MODE)
    {
        //SEND READ FROM CACHE X 4
        cmd = SPI_NAND_AFD04GWS_READ_FROM_CACHE_X4;
        bus_width = SFC_BUS_4_WIRE;
    }

    
    //��������
    REG32(SFC_INST_PHASE_REG(0)) = cmd;

    //��ַ����
    REG32(SFC_ADDR_PHASE_REG(0)) = 0;

    //��Ҫ�����ַ�� ���ݾ�ʹ��1�߷�
    REG32(SFC_INTERFACE_CFG_REG(0)) = (SFC_INST_BUSWIDTH_CFG(SFC_BUS_1_WIRE) | SFC_INST_PHASE_ENA \
                                  | SFC_ADDR_MODE_BUSWIDTH_CFG(SFC_BUS_1_WIRE) | SFC_ADDR_PHASE_ENA \
                                  | SFC_DATA_BUSWIDTH_CFG(bus_width) | SFC_DATA_PHASE_ENA \
                                  | SFC_DATA_DIR_RX);

    //�����1 cycle + ��ַ����3 cycle (��DUMMY����ADDR��) + DATA ����1 cycle 
    REG32(SFC_PHASE_LENGTH_CFG_REG(0)) = (SFC_INST_LEN_CFG(1) | SFC_ADDR_LEN_CFG(3) | SFC_DATA_LEN_CFG(read_count));

    //dummy  CYCLE����(û��Wait phase, ��������)
    //REG32(SFC_WAIT_PHASE_REG) = 0;

    //��������
    sfc_rece_dat(read_buf, read_count);

    return AK_TRUE;


}



T_BOOL ak_oob_spi_nand_afd04gws_page_read(T_U32 page_addr, T_U8 *read_buf, T_U32 read_count, T_READ_MODE mode)
{
    T_U32   spi_nand_status;
    T_U32   return_val;
    T_U8    cmd;
    T_U8    bus_width;
    T_U8    status;

    return_val = AK_FALSE;
    

////////////////////////////////////// COM PAGE READ ////////////////////////////////////////////////
    //��PAGE ��CACHE ����
    //��������
    REG32(SFC_INST_PHASE_REG(0)) = SPI_NAND_AFD04GWS_PAGE_READ;

    //��ַ����
    REG32(SFC_ADDR_PHASE_REG(0)) = page_addr;

    //��Ҫ�����ַ�� ��ʹ��1�߷�
    REG32(SFC_INTERFACE_CFG_REG(0)) = (SFC_INST_BUSWIDTH_CFG(SFC_BUS_1_WIRE) | SFC_INST_PHASE_ENA \
                                       | SFC_ADDR_MODE_BUSWIDTH_CFG(SFC_BUS_1_WIRE) | SFC_ADDR_PHASE_ENA \
                                       | SFC_DATA_DIR_TX);

    //�����1 cycle + ��ַ����3 cycle(��ַ+ dummy)
    REG32(SFC_PHASE_LENGTH_CFG_REG(0)) = (SFC_INST_LEN_CFG(1) | SFC_ADDR_LEN_CFG(3));

    //dummy  CYCLE����(û��Wait phase, ��������)
    //REG32(SFC_WAIT_PHASE_REG) = 0;
    
    //�������䲢�ȴ�����
    sfc_cpu_trans_start_and_wait_finish();

     udelay(1);		//delay tCS

    while(1)
	{
		status = spi_nand_get_features_status();		//GET STATUS
		//printf("status is %x\n", status);
		if( (status & SPI_NAND_STATUS_OIP_BIT) == 0)
		{
			break;
		}
	}

	if( (status & (0x3<<4))== SPI_NAND_STATUS_ECC_FAILED_BIT)
	{
		printf("ecc failed, status is %x\n", status);
		return AK_FALSE;									
	}


////////////////////////////////////// �������� ////////////////////////////////////////
    if(mode == READ_X1_MODE)
    {
        //SEND READ FROM CACHE X 1
        cmd = SPI_NAND_AFD04GWS_READ_FROM_CACHE; 
        bus_width = SFC_BUS_1_WIRE;
    }

    else if(mode == READ_X2_MODE)
    {
        //SEND READ FROM CACHE X 2
        cmd = SPI_NAND_AFD04GWS_READ_FROM_CACHE_X2; 
        bus_width = SFC_BUS_2_WIRE;
    }

    else if(mode == READ_X4_MODE)
    {
        //SEND READ FROM CACHE X 4
        cmd = SPI_NAND_AFD04GWS_READ_FROM_CACHE_X4;
        bus_width = SFC_BUS_4_WIRE;
    }

    
    //��������
    REG32(SFC_INST_PHASE_REG(0)) = cmd;

    //��ַ����
    REG32(SFC_ADDR_PHASE_REG(0)) = 0;

    //��Ҫ�����ַ�� ���ݾ�ʹ��1�߷�
    REG32(SFC_INTERFACE_CFG_REG(0)) = (SFC_INST_BUSWIDTH_CFG(SFC_BUS_1_WIRE) | SFC_INST_PHASE_ENA \
                                  | SFC_ADDR_MODE_BUSWIDTH_CFG(SFC_BUS_1_WIRE) | SFC_ADDR_PHASE_ENA \
                                  | SFC_DATA_BUSWIDTH_CFG(bus_width) | SFC_DATA_PHASE_ENA \
                                  | SFC_DATA_DIR_RX);

    //�����1 cycle + ��ַ����3 cycle (��DUMMY����ADDR��) + DATA ����1 cycle 
    REG32(SFC_PHASE_LENGTH_CFG_REG(0)) = (SFC_INST_LEN_CFG(1) | SFC_ADDR_LEN_CFG(3) | SFC_DATA_LEN_CFG(read_count));

    //dummy  CYCLE����(û��Wait phase, ��������)
    //REG32(SFC_WAIT_PHASE_REG) = 0;

    //��������
    sfc_rece_dat(read_buf, read_count);

    return AK_TRUE;


}



/**
  Function:      T_BOOL spi_nand_page_read()
  Description:  page read
  Input:           T_U32 data
  Output:         
  Return:         AK_TRUE,  AK_FALSE
  Date 		  2015-03-25
  Author:         Zou Tianxiang
**/
T_BOOL spi_nand_page_read(T_U32 page_addr, T_U8 *read_buf, T_U32 read_count, T_READ_MODE mode)
{
    T_U32   spi_nand_status;
    T_U32   return_val;
    T_U8    cmd;
    T_U8    bus_width;
    T_U8    status;

    return_val = AK_FALSE;
    

////////////////////////////////////// COM PAGE READ ////////////////////////////////////////////////
    //��PAGE ��CACHE ����
    //��������
    REG32(SFC_INST_PHASE_REG(0)) = SPI_NAND_AFD04GWS_PAGE_READ;

    //��ַ����
    REG32(SFC_ADDR_PHASE_REG(0)) = page_addr;

    //��Ҫ�����ַ�� ��ʹ��1�߷�
    REG32(SFC_INTERFACE_CFG_REG(0)) = (SFC_INST_BUSWIDTH_CFG(SFC_BUS_1_WIRE) | SFC_INST_PHASE_ENA \
                                       | SFC_ADDR_MODE_BUSWIDTH_CFG(SFC_BUS_1_WIRE) | SFC_ADDR_PHASE_ENA \
                                       | SFC_DATA_DIR_TX);

    //�����1 cycle + ��ַ����3 cycle(��ַ+ dummy)
    REG32(SFC_PHASE_LENGTH_CFG_REG(0)) = (SFC_INST_LEN_CFG(1) | SFC_ADDR_LEN_CFG(3));

    //dummy  CYCLE����(û��Wait phase, ��������)
    //REG32(SFC_WAIT_PHASE_REG) = 0;
    
    //�������䲢�ȴ�����
    sfc_cpu_trans_start_and_wait_finish();

     udelay(1);		//delay tCS

    while(1)
	{
		status = spi_nand_get_features_status();		//GET STATUS
		//printf("status is %x\n", status);
		if( (status & SPI_NAND_STATUS_OIP_BIT) == 0)
		{
			break;
		}
	}

	if( (status & (0x3<<4))== SPI_NAND_STATUS_ECC_FAILED_BIT)
	{
		printf("ecc failed, status is %x\n", status);
		return AK_FALSE;									
	}


////////////////////////////////////// �������� ////////////////////////////////////////
    if(mode == READ_X1_MODE)
    {
        //SEND READ FROM CACHE X 1
        cmd = SPI_NAND_AFD04GWS_READ_FROM_CACHE; 
        bus_width = SFC_BUS_1_WIRE;
    }

    else if(mode == READ_X2_MODE)
    {
        //SEND READ FROM CACHE X 2
        cmd = SPI_NAND_AFD04GWS_READ_FROM_CACHE_X2; 
        bus_width = SFC_BUS_2_WIRE;
    }

    else if(mode == READ_X4_MODE)
    {
        //SEND READ FROM CACHE X 4
        cmd = SPI_NAND_AFD04GWS_READ_FROM_CACHE_X4;
        bus_width = SFC_BUS_4_WIRE;
    }

    
    //��������
    REG32(SFC_INST_PHASE_REG(0)) = cmd;

    //��ַ����
    REG32(SFC_ADDR_PHASE_REG(0)) = 0;

    //��Ҫ�����ַ�� ���ݾ�ʹ��1�߷�
    REG32(SFC_INTERFACE_CFG_REG(0)) = (SFC_INST_BUSWIDTH_CFG(SFC_BUS_1_WIRE) | SFC_INST_PHASE_ENA \
                                  | SFC_ADDR_MODE_BUSWIDTH_CFG(SFC_BUS_1_WIRE) | SFC_ADDR_PHASE_ENA \
                                  | SFC_DATA_BUSWIDTH_CFG(bus_width) | SFC_DATA_PHASE_ENA \
                                  | SFC_DATA_DIR_RX);

    //�����1 cycle + ��ַ����3 cycle (��DUMMY����ADDR��) + DATA ����1 cycle 
    REG32(SFC_PHASE_LENGTH_CFG_REG(0)) = (SFC_INST_LEN_CFG(1) | SFC_ADDR_LEN_CFG(3) | SFC_DATA_LEN_CFG(read_count));

    //dummy  CYCLE����(û��Wait phase, ��������)
    //REG32(SFC_WAIT_PHASE_REG) = 0;

    //��������
    sfc_rece_dat(read_buf, read_count);

    return AK_TRUE;


}





