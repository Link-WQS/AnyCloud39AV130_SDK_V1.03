/**
* #FILENAME: spi_ctrl_drv.c
* #BRIEF    spi controller driver
* Copyright (C) 2014 Anyka (ShenZhen) Micro-Electronic Technology Co., LTD
* #AUTHOR ZOU TIANXIANG
* #DATE 2014-05-29
* #VERSION 1.0
* #REF Please refer to��

*********************************************************/


#include "ak_sfc_ctrl.h" 
#include "ak_clk_tree_drv.h"
#include "ak_common_function.h"


#if 1





/**
* @Function:     T_VOID sfc_trans_start()
* @Input:          none          
* @Output:       none
* @Return:       none
* @DATA:         2024.4.29
* @Author:       Zou Tianxiang
* @Note:          �������䲢�ȴ�����
**/
static T_VOID sfc_trans_start() 
{
    
    //��������(���������䣬����λ����)
    REG32(SFC_TRANSFER_ENABLE_CFG_REG) |= (0x1 << 31);  
}

/**
* @Function:      T_VOID sfc_trans_wait_done()
* @Input:           none
* @Output:         none
* @Return:         none
* @DATA:          2024.4.29
* @Author:        Zou Tianxiang
* @Note:           �ȴ�����
**/
static T_VOID sfc_trans_wait_done()
{
    //�ȴ�done
    while(1)
    {
        if((REG32(SFC_STATUS_REG) & (0x1<<4)) == (0x1<<4))
        {
            REG32(SFC_STATUS_REG) |= (0x1 << 4);
            break;
        }
    }
}



/**
* @Function:      T_VOID sfc_cpu_trans_start_and_wait_finish()
* @Input:           none
* @Output:         none
* @Return:         none
* @DATA:          2024.4.28
* @Author:        Zou Tianxiang
* @Note:           �������䲢�ȴ�����,ȫ��ʹ��CPU��ʽ����
**/
T_VOID sfc_cpu_trans_start_and_wait_finish() 
{
    
    //��������+ CPU��ʽ
    REG32(SFC_TRANSFER_ENABLE_CFG_REG) = (0x1 << 31);  

    //�ȴ�done
    while(1)
    {
        //printf("REG32(SFC_STATUS_REG) = %x\n", REG32(SFC_STATUS_REG));
        if((REG32(SFC_STATUS_REG) & (0x1<<4)) == (0x1<<4))
        {
            REG32(SFC_STATUS_REG) |= (0x1 << 4);
            break;
        }
    }
}



/**
* @BRIEF sfc_tx_fifo_idle_cnt
* @AUTHOR zoutianxiang
* @DATE 2024-5-15
* @PARAM    
* @RETURN   
* @RETVAL   
* @NOTE     spi tx fifo �ж��ٿ��пռ�.  ��λ: word
*/
__inline T_U32 sfc_tx_fifo_idle_cnt()
{
    T_U32 tx_idle_fifo_cnt;
    
    //����TX FIFO�п�д�Ŀռ�
    tx_idle_fifo_cnt = ((REG32(SFC_FIFO_THRES_STATUS_REG) >> 16) & 0xff);

    return tx_idle_fifo_cnt;
}






/**
* @BRIEF sfc_tx_fifo_is_full
* @AUTHOR zoutianxiang
* @DATE 2024-5-15
* @PARAM    
* @RETURN   
* @RETVAL   AK_TRUE  AK_FALSE
* @NOTE     spi tx fifo ȷ���Ƿ���
*/
T_BOOL sfc_tx_fifo_is_full()
{
    //���û��FIFO��д�򷵻�TRUE,��ʾ��
    if(sfc_tx_fifo_idle_cnt() == 0)
    {
        return AK_TRUE;
    }
    else
    {
        return AK_FALSE;
    }
}







/**
* @BRIEF sfc_tx_fifo_empty
* @AUTHOR zoutianxiang
* @DATE 2024-5-17
* @PARAM    
* @RETURN   
* @RETVAL   
* @NOTE     spi tx fifo Ϊ��
*/
T_BOOL sfc_tx_fifo_empty()
{
    //TX FIFO ������������FIFO���
    if(sfc_tx_fifo_idle_cnt() == SFC_TX_FIFO_DEPTH)
    {
        return AK_TRUE;
    }
    else
    {
        return AK_FALSE;
    }
    
}





/**
* @BRIEF sfc_send_dat
* @AUTHOR zoutianxiang
* @DATE 2012-12-06
* @DATE 2024-5-17  for H322-D
* @PARAM    dat_buf, count
* @RETURN   
* @RETVAL   
* @NOTE     
*/
T_VOID sfc_send_dat(T_U8* dat_buf, T_U32 count)
{
    T_U32 tx_cnt;
    T_U32 fifo_idle_cnt;
    T_U32 cnt_4B;
    T_U32 cnt_4B_left;
    T_U32 i;
    T_U32 reg_val;
    

    //����һ������WORD
    cnt_4B = count/4;

    //����word�������������
    cnt_4B_left = count%4;
    
    tx_cnt = 0;
    

    //TX��RX ��ֵ������Ϊ4WORD(16B) ά��Ĭ��ֵ(ʵ����sfc_send_dat������û�õ�)
    //REG32(SFC_FIFO_THRES_STATUS_REG) = (SFC_TX_THRESHOLD_CFG(4) | SFC_RX_THRESHOLD_CFG(4));

    //����CPUģʽ+ ֻ��1��CMD����
    REG32(SFC_TRANSFER_ENABLE_CFG_REG) = 0;  

    //���FIFO
    REG32(SFC_WORK_MODE_REG) |= (0x1<<4);
    while(1)
    {
        if((REG32(SFC_WORK_MODE_REG) & (0x1<<4)) == 0)
        {
            break;
        }
    }

    //��������
    sfc_trans_start();

    
    while(1)
    {
        //�õ�TX FIFO�Ŀ�������
        fifo_idle_cnt = sfc_tx_fifo_idle_cnt();

        //���û�пռ�
        if(fifo_idle_cnt == 0)
        {
            continue;
        }


        //���Ҫ���͵����ݴ���fifo������
        if(cnt_4B >=  fifo_idle_cnt)
        {
            //������FIFO ����ȫ��д��
            for(i = 0; i < fifo_idle_cnt; i++)
            {
                REG32(SFC_DATA_FIFO_REG) = ( ((dat_buf[tx_cnt+0]) << 0)  | \
                                             ((dat_buf[tx_cnt+1]) << 8)  | \
                                             ((dat_buf[tx_cnt+2]) << 16) | \
                                             ((dat_buf[tx_cnt+3]) << 24) );
                tx_cnt += 4;
            }

            cnt_4B -= fifo_idle_cnt; 
        }

        //���Ҫ���͵�����С��fifo������
        else
        {
            //������FIFO ����ȫ��д��
            for(i = 0; i < cnt_4B; i++)
            {
                REG32(SFC_DATA_FIFO_REG) = ( ((dat_buf[tx_cnt+0]) << 0)  | \
                                             ((dat_buf[tx_cnt+1]) << 8)  | \
                                             ((dat_buf[tx_cnt+2]) << 16) | \
                                             ((dat_buf[tx_cnt+3]) << 24) );
                tx_cnt += 4;
            }
            
            cnt_4B = 0;
        }

        //���word�Ѵ���������˳�ѭ��
        if(cnt_4B == 0)
        {
            break;
        }
            
    }

    //�����ʣ��<4B ������
    if(cnt_4B_left != 0)
    {
        reg_val = 0;
        
        for( i = 0; i < cnt_4B_left; i++)
        {
            reg_val |= ( (dat_buf[tx_cnt+i]) << (i<<3));
        }

        //�ȴ�FIFO�ճ�
        while(sfc_tx_fifo_is_full() == AK_TRUE)
        {;}

        REG32(SFC_DATA_FIFO_REG) = reg_val;
    }


    //��TX FIFO��ȫ�����˳�
    while(1)
    {
        if(sfc_tx_fifo_empty() == AK_TRUE)
        {
            break;
        }
    }
    
    //�ȴ��������
    sfc_trans_wait_done();
    
}


/**
* @BRIEF sfc_rece_dat
* @AUTHOR zoutianxiang
* @DATE 2012-12-06
* @PARAM    dat_buf, count
* @RETURN   
* @RETVAL   
* @NOTE     
*/
T_VOID sfc_rece_dat(T_U8* dat_buf, T_U32 count)
{
    T_U32 cnt_16B;        //���ж��ٸ�16B ����
    T_U32 cnt_16B_left;   //��16B����������ʣ���������
    T_U32 last_left;      //����4 �ֽڶ��������
    T_U32 fifo_val;
    T_U32 i;
    T_U32 read_cnt;

    //���ж��ٸ�16B ����
    cnt_16B = count >> 4;

    //��16�����⻹ʣ�������
    cnt_16B_left = count & 0xf;

    //���ʣ���4B���������
    last_left = count & 0x3;


    //TX��RX ��ֵ������Ϊ4WORD(16B) ά��Ĭ��ֵ
    REG32(SFC_FIFO_THRES_STATUS_REG) = (SFC_TX_THRESHOLD_CFG(4) | SFC_RX_THRESHOLD_CFG(4));

    //����CPUģʽ+ ֻ��1��CMD����
    REG32(SFC_TRANSFER_ENABLE_CFG_REG) = 0;  

    //���FIFO
    REG32(SFC_WORK_MODE_REG) |= (0x1<<4);
    while(1)
    {
        if((REG32(SFC_WORK_MODE_REG) & (0x1<<4)) == 0)
        {
            break;
        }
    }

    //��������
    sfc_trans_start();

    //����������
    read_cnt= 0;
    
    while(1)
    {
        
        //�ж��Ƿ񵽴﷧ֵ
        if((REG32(SFC_STATUS_REG) & (0x1<<3)) == (0x1<<3))
        {
            //����4��word
            for(i = 0; i < 4; i++)
            {
                //ÿ�ζ�4��BYTE
                fifo_val = REG32(SFC_DATA_FIFO_REG);
                dat_buf[read_cnt + 0] = (T_U8)(fifo_val & 0xff);
                dat_buf[read_cnt + 1] = (T_U8)((fifo_val>>8) & 0xff);
                dat_buf[read_cnt + 2] = (T_U8)((fifo_val>>16) & 0xff);
                dat_buf[read_cnt + 3] = (T_U8)((fifo_val>>24) & 0xff);
                read_cnt += 4;
            }
            cnt_16B--;
       }
           
       if(cnt_16B == 0)
       {
           break;
       }
    }

    //�ȴ��������
    sfc_trans_wait_done();

    //ʣ�����ݶ�ȡ
    if(cnt_16B_left != 0)
    {
        //����ʣ��4�ֽڶ��������
        for(i = 0; i < (cnt_16B_left>>2); i++)
        {
            fifo_val = REG32(SFC_DATA_FIFO_REG);
            dat_buf[read_cnt + 0] = (T_U8)(fifo_val & 0xff);
            dat_buf[read_cnt + 1] = (T_U8)((fifo_val>>8) & 0xff);
            dat_buf[read_cnt + 2] = (T_U8)((fifo_val>>16) & 0xff);
            dat_buf[read_cnt + 3] = (T_U8)((fifo_val>>24) & 0xff);
            read_cnt += 4;
        }

        //�������ʣ��4B�����������
        if(last_left!= 0)
        {
           //�������FIFOֵ
           fifo_val = REG32(SFC_DATA_FIFO_REG);
           for(i = 0; i < last_left; i++)
           {
              dat_buf[read_cnt + i] = (T_U8)((fifo_val>>(i*8)) & 0xff);
           }
           read_cnt += last_left;
        }
    }

}



#endif



/**
* #BRIEF  spi share pin select 
* #AUTHOR ZouTianxiang
* #DATE 2022-11-30
* #RETURN
* #RETVAL
*/
#define CHIP_CONF_BASE_ADDR  0x08000000 // chip configurations

#define PAD_DRV_CFG2_REG (CHIP_CONF_BASE_ADDR + 0x000001AC)

T_VOID  sfc_share_pin_cfg()
{
    T_U32 reg_val;

    //����SHARE PIN
    reg_val = REG32(SHARE_PIN_CFG4_REG);
    reg_val &= (~(0x3ffff<<9));
    reg_val |= ( (0x2<<9) | (0x2<<12) | (0x2<<15) | (0x2<<18) | (0x2<<21) | (0x2<<24) );
    REG32(SHARE_PIN_CFG4_REG) = reg_val;
    CLEAR_AHB_WRITE_BUF;

    reg_val = REG32(PAD_DRV_CFG2_REG);
    reg_val &= (~(0xFFFF00000<<20));
    reg_val |= (0xFFFF00000<<20);
    REG32(PAD_DRV_CFG2_REG) = reg_val;

    
    //��д���棬�����ʱ��Ч����
    CLEAR_AHB_WRITE_BUF;
}




/**
  Function:       sfc_ctrl_master_init(T_U32 sfc_clk_freq)
  Description:    initial the spi ctrl
  Input:          role: master or slaver                 
  Output:         none
  Return:         none
  Date:            2024-05-15
  NOTE:           SPI  PORT CLK = (core_pll_clock/(sfv_phyclk_div_num_cfg+1) ) /2 
  Author:         Zou Tianxiang
**/
T_VOID sfc_ctrl_master_init(T_U32 sfc_clk_freq)
{
    T_U32 pll_clk;
    T_U32 clk_div;
    T_U32 reg_val;

    //�õ�pll0 clkʱ��Ƶ��
    pll_clk = ak_clk_get_core_pll_freq();
    //printf("pll_clk = %d hz\n", pll_clk);
  
    //�̶�����SFC �ӿ�ʱ����SFC PHYCLK��һ�룬
    //���sfc_phyclk_div�������¼���
    //FPGA ��PLL ��sfcphy div ֱ����PHYCLK = 60mhz  ������������������ã���Ӱ��FPGA����
    clk_div = pll_clk/(sfc_clk_freq * 2);
    //printf("sfc phyclk_div = %d\n", clk_div);

    
    //�ر�SFC gclk gate
    clk_close_peripheral_module_gate(CLK_SFC);
      
    //������gclk��phyclk����ʱ������˱�����������λ
    clk_hold_reset_peripheral_module(CLK_SFC);


    //����SFC_PHY_CLK��Ƶ��,����Ĳ���:
    //��sfcphy clk  ->  HOLD ��λsfcphy clk -> ��Ƶ-> ��sfc_phyclk  -> �ſ���λsfcphy clk
    clk_set_sfc_phyclk_div(clk_div);


    //��SFC gclk gate
    clk_open_peripheral_module_gate(CLK_SFC);

     //�ſ�SFC GCLK��λ
    clk_release_reset_peripheral_module(CLK_SFC);


    //��AHB д����    
    CLEAR_AHB_WRITE_BUF;


//SFC ����������
    //CS�����ҵ�PHYCLK����Ϊ�˿�ʱ�ӵ������������CLK���ǰ1 PHY CLK ����
    //CS��CLK����������ٶ���ʱ1 PHY CLK ������
    //ά��Ĭ��ֵ
    REG32(SFC_CS_TIMING_REG) = ((0x0<<0) | (0x0<<2));

    //����SWCģʽ (ά��Ĭ��ֵ)
    REG32(SFC_WORK_MODE_REG) &= (~(0x3<<2));

    //���� SPI MODE0 & �˿�ʱ��Ƶ��ΪPHY CLK 2��Ƶ������ά��Ĭ��ֵ
    //SCLK = PHY_CLK/(cfg_spi_baud_rate+1)
    //ά��Ĭ��ֵ�����˲���Ҫд��仰 
    REG32(SFC_BAUD_RATE_REG) = ((0x1<<29) | 1);


    //����SHARE PIN
    sfc_share_pin_cfg();
    #if 0
    #endif

}


















