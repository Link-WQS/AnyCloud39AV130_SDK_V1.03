#if 1
#include <spi.h>
#include "ak_sfcv2.h"
/*************************************** svt start **********************************************/
#ifdef CONFIG_39EV33X_CODE
#include <asm/arch-ak39ev33x/ak_cpu.h>
#include <asm/arch-ak39ev33x/ak_l2buf.h>
#include <asm/arch-ak39ev33x/ak_module_reset.h>
#endif

#ifdef CONFIG_37_D_CODE
#include <asm/arch-ak37d/ak_cpu.h>
#include <asm/arch-ak37d/ak_l2buf.h>
#include <asm/arch-ak37d/ak_module_reset.h>
#endif

#ifdef CONFIG_37_E_CODE
#include <asm/arch-ak37e/ak_cpu.h>
#include <asm/arch-ak37e/ak_l2buf.h>
#include <asm/arch-ak37e/ak_module_reset.h>
#endif

#if defined(CONFIG_3918AV100_CODE)
#include <asm/arch-ak3918av100/ak_cpu.h>
#include <asm/arch-ak3918av100/ak_l2buf.h>
#include <asm/arch-ak3918av100/ak_module_reset.h>
#endif

#if defined(CONFIG_3918AV130_CODE)
#include <asm/arch-ak3918av130/ak_cpu.h>
#include <asm/arch-ak3918av130/ak_l2buf.h>
#include <asm/arch-ak3918av130/ak_module_reset.h>
#endif

#if defined(CONFIG_KM01A_CODE)
#include <asm/arch-km01a/ak_cpu.h>
#include <asm/arch-km01a/ak_l2buf.h>
#include <asm/arch-km01a/ak_module_reset.h>
#endif

#ifdef CONFIG_3918EV300L_CODE
#include <asm/arch-ak3918ev300l/ak_cpu.h>
#include <asm/arch-ak3918ev300l/ak_l2buf.h>
#include <asm/arch-ak3918ev300l/ak_module_reset.h>
#endif

#ifdef CONFIG_39EV200_CODE
#include <asm/arch-ak39ev200/ak_cpu.h>
#include <asm/arch-ak39ev200/ak_l2buf.h>
#include <asm/arch-ak39ev200/ak_module_reset.h>
#endif

extern unsigned long clk_get_core_pll_freq(void);

T_FLASH_PARAM flash_param;

static unsigned int flash_cmd[][7] =
{
    //cmd code                         cmd    addr    data    dummy_cnt  dtr_dummy_cnt mode
    {FLASH_CMD_RDSR1,                  WIRE1, NO_USE, WIRE1,  NO_USE,    8,            NO_USE},
    {FLASH_CMD_RDSR2,                  WIRE1, NO_USE, WIRE1,  NO_USE,    8,            NO_USE},
    {FLASH_CMD_RDSR3,                  WIRE1, NO_USE, WIRE1,  NO_USE,    8,            NO_USE},
    {FLASH_CMD_READ_FLAG_SR,           WIRE1, NO_USE, WIRE1,  NO_USE,    8,            NO_USE},
    {FLASH_CMD_READ_ID,                WIRE1, NO_USE, WIRE1,  NO_USE,    8,            NO_USE},
    {FLASH_CMD_READ_NVCR,              WIRE1, WIRE1,  WIRE1,  8,         8,            NO_USE},
    {FLASH_CMD_READ_VCR,               WIRE1, WIRE1,  WIRE1,  8,         8,            NO_USE},
    {FLASH_CMD_READ_SFDP,              WIRE1, WIRE1, WIRE1,   8,         7,            NO_USE},

    {FLASH_CMD_QPI,                    WIRE1, NO_USE, NO_USE, NO_USE,    NO_USE,       NO_USE},
    {FLASH_CMD_RESET_EN,               WIRE1, NO_USE, NO_USE, NO_USE,    NO_USE,       NO_USE},
    {FLASH_CMD_RESET,                  WIRE1, NO_USE, NO_USE, NO_USE,    NO_USE,       NO_USE},
    {FLASH_CMD_WREN,                   WIRE1, NO_USE, NO_USE, NO_USE,    NO_USE,       NO_USE},
    {FLASH_CMD_WRDI,                   WIRE1, NO_USE, NO_USE, NO_USE,    NO_USE,       NO_USE},
    {FLASH_CMD_WREN_VSR,               WIRE1, NO_USE, NO_USE, NO_USE,    NO_USE,       NO_USE},
    {FLASH_CMD_WRSR1,                  WIRE1, NO_USE, WIRE1,  NO_USE,    NO_USE,       NO_USE},
    {FLASH_CMD_WRSR2,                  WIRE1, NO_USE, WIRE1,  NO_USE,    NO_USE,       NO_USE},
    {FLASH_CMD_WRSR3,                  WIRE1, NO_USE, WIRE1,  NO_USE,    NO_USE,       NO_USE},
    {FLASH_CMD_WRITE_NVCR,             WIRE1, WIRE1,  WIRE1,  NO_USE,    NO_USE,       NO_USE},
    {FLASH_CMD_WRITE_VCR,              WIRE1, WIRE1,  WIRE1,  NO_USE,    NO_USE,       NO_USE},

    {FLASH_CMD_SE,                     WIRE1, WIRE1,  NO_USE, NO_USE,    NO_USE,       NO_USE},
    {FLASH_CMD_64K_BE,                 WIRE1, WIRE1,  NO_USE, NO_USE,    NO_USE,       NO_USE},
    {FLASH_CMD_CE,                     WIRE1, NO_USE, NO_USE, NO_USE,    NO_USE,       NO_USE},

    {FLASH_CMD_READ,                   WIRE1, WIRE1,  WIRE1,  NO_USE,    NO_USE,       NO_USE},
    {FLASH_CMD_FAST_READ,              WIRE1, WIRE1,  WIRE1,  8,         16,           NO_USE},
    {FLASH_CMD_DTR_FAST_READ,          WIRE1, WIRE1,  WIRE1,  6,         6,            NO_USE},
    {FLASH_CMD_DUAL_FAST_READ,         WIRE1, WIRE1,  WIRE2,  8,         8,            NO_USE},
    {FLASH_CMD_DUAL_IO_FAST_READ,      WIRE1, WIRE2,  WIRE2,  2,         2,            0xa0},
    {FLASH_CMD_DUAL_IO_DTR_FAST_READ,  WIRE1, WIRE2,  WIRE2,  4,         4,            0xa0},
    {FLASH_CMD_QUAD_FAST_READ,         WIRE1, WIRE1,  WIRE4,  8,         16,           NO_USE},
    {FLASH_CMD_QUAD_IO_FAST_READ,      WIRE1, WIRE4,  WIRE4,  14,        15,           0xa0},
    {FLASH_CMD_QUAD_IO_DTR_FAST_READ,  WIRE1, WIRE4,  WIRE4,  7,        7,           0x00},
    {FLASH_CMD_OCTAL_FAST_READ,        WIRE1, WIRE1,  WIRE8,  8,         16,           NO_USE},
    {FLASH_CMD_OCTAL_IO_FAST_READ,     WIRE1, WIRE8,  WIRE8,  15,        15,           0xa0},
    {FLASH_CMD_OCTAL_IO_DTR_FAST_READ, WIRE1, WIRE8,  WIRE8,  15,        15,           0xa0},

    {FLASH_CMD_PP,                     WIRE1, WIRE1,  WIRE1,  NO_USE,    NO_USE,       NO_USE},
    {FLASH_CMD_QUAD_PP,                WIRE1, WIRE1,  WIRE4,  NO_USE,    NO_USE,       NO_USE},
    {FLASH_CMD_OCTAL_PP,               WIRE1, WIRE1,  WIRE8,  NO_USE,    NO_USE,       NO_USE},
};

int tiaoshi_flag = 0;
void sfc_set_spi_trans_phase(T_SFC_SPI_PHASE_CFG *phase_cfg, unsigned int cmd_bank_num);
void sfc_intr_enable(unsigned int intr_signal);

/**
 * @brief  norflash_cmd_analysis
 * @param  *phase_cfg
 * @param  sample_mode
 * @param  *cmd
 * @param  trans_addr
 * @return
 * @note   解析spi传输各个phase的参数，八线DTR模式inst，address，mode，data数据量需为2的倍数
 */
static void norflash_cmd_analysis(T_SFC_SPI_PHASE_CFG *phase_cfg, unsigned char flash_cmd_code, unsigned int flash_addr, unsigned int data_cnt)
{
    unsigned int *cmd = NULL;
    unsigned int tmp = STR_MODE;

    for(int i = 0; i < sizeof(flash_cmd)/sizeof(flash_cmd[0]); ++i)
    {
        if(flash_cmd[i][CMD_CODE] == flash_cmd_code)
        {
            cmd = flash_cmd[i];
        }
    }

    phase_cfg->inst_phase.enable = true;
    phase_cfg->inst_phase.bus_wire = cmd[CMD_BUS];
    phase_cfg->inst_phase.spl_mode = STR_MODE;
    phase_cfg->inst_phase.cnt = 1;
    phase_cfg->inst_phase.data = cmd[CMD_CODE];

    if(flash_cmd_code == FLASH_CMD_QUAD_IO_DTR_FAST_READ || flash_cmd_code == FLASH_CMD_DTR_FAST_READ\
    || flash_cmd_code == FLASH_CMD_DUAL_IO_DTR_FAST_READ || flash_cmd_code == FLASH_CMD_OCTAL_IO_DTR_FAST_READ)
    {
        tmp = flash_param.sample_mode;
        flash_param.sample_mode = DTR_MODE;
    }

    if(cmd[ADDR_BUS] != NO_USE)
    {
        phase_cfg->addr_phase.enable = true;
        phase_cfg->addr_phase.bus_wire = cmd[ADDR_BUS];  //dummy,mode和addr一样
        phase_cfg->addr_phase.spl_mode = flash_param.sample_mode;  //dummy,mode和addr一样
        phase_cfg->addr_phase.cnt = flash_param.addr_mode;
        phase_cfg->addr_phase.data = flash_addr;
    }
    else
        phase_cfg->addr_phase.enable = false;

    if(cmd[MODE_DATA] != NO_USE)
    {
        phase_cfg->mode_phase.enable = true;
        phase_cfg->mode_phase.bus_wire = cmd[ADDR_BUS]; //dummy,mode和addr一样
        phase_cfg->mode_phase.spl_mode = flash_param.sample_mode;  //dummy,mode和addr一样
        phase_cfg->mode_phase.cnt = 1;
        phase_cfg->mode_phase.data = cmd[MODE_DATA];
        if(flash_param.sample_mode == DTR_MODE && flash_param.bus_wire == WIRE8)
        {
            phase_cfg->mode_phase.cnt = 2;
            phase_cfg->mode_phase.data = cmd[MODE_DATA] << 8;
        }
    }
    else
        phase_cfg->mode_phase.enable = false;

    if(cmd[DUMMY_CNT] != NO_USE && flash_param.sample_mode == STR_MODE)
    {
        phase_cfg->wait_phase.enable = true;
        phase_cfg->wait_phase.bus_wire = cmd[ADDR_BUS]; //dummy,mode和addr一样
        phase_cfg->wait_phase.spl_mode = flash_param.sample_mode;  //dummy,mode和addr一样
        phase_cfg->wait_phase.cnt = cmd[DUMMY_CNT];
    }
    else if(cmd[DTR_DUMMY_CNT] != NO_USE && flash_param.sample_mode == DTR_MODE)
    {
        phase_cfg->wait_phase.enable = true;
        phase_cfg->wait_phase.bus_wire = cmd[ADDR_BUS]; //dummy,mode和addr一样
        phase_cfg->wait_phase.spl_mode = flash_param.sample_mode;  //dummy,mode和addr一样
        phase_cfg->wait_phase.cnt = cmd[DTR_DUMMY_CNT];
    }
    else
        phase_cfg->wait_phase.enable = false;

    if(cmd[DATA_BUS] != NO_USE)
    {
        phase_cfg->data_phase.enable = true;
        phase_cfg->data_phase.bus_wire = cmd[DATA_BUS];
        phase_cfg->data_phase.spl_mode = flash_param.sample_mode;
        phase_cfg->data_phase.cnt = data_cnt;
        if(flash_param.sample_mode == DTR_MODE && flash_param.bus_wire == WIRE8)
            phase_cfg->data_phase.cnt = ((data_cnt+1)>>1)<<1;
    }
    else
        phase_cfg->data_phase.enable = false;

    if(flash_cmd_code == FLASH_CMD_QUAD_IO_DTR_FAST_READ || flash_cmd_code == FLASH_CMD_DTR_FAST_READ\
    || flash_cmd_code == FLASH_CMD_DUAL_IO_DTR_FAST_READ || flash_cmd_code == FLASH_CMD_OCTAL_IO_DTR_FAST_READ)
        flash_param.sample_mode = tmp;

    //GD flash DTR模式所有phase bus均为flash data IO数量
    if(flash_param.sample_mode == DTR_MODE)
    {
        phase_cfg->inst_phase.bus_wire = flash_param.bus_wire;
        phase_cfg->addr_phase.bus_wire = flash_param.bus_wire;
        phase_cfg->mode_phase.bus_wire = flash_param.bus_wire;
        phase_cfg->wait_phase.bus_wire = flash_param.bus_wire;
        phase_cfg->data_phase.bus_wire = flash_param.bus_wire;
    }
}

/**
 * @brief  sfc_spi_trans_cfg
 * @param  *trans_param //结构体数组
 * @param  *phase_cfg
 * @param  cmd_bank_sum
 * @return
 * @note   spi传输配置
 */
void sfc_spi_trans_cfg(T_SFC_TRANS_PARAM *trans_param, T_SFC_SPI_PHASE_CFG *phase_cfg, unsigned int cmd_bank_num)
{
    unsigned int reg_val;

    sfc_set_spi_trans_phase(phase_cfg, cmd_bank_num);

    reg_val = REG32(SFC_INTERFACE_CFG_REG(cmd_bank_num));
    reg_val &= ~((0x3 << 27) | (0x3 << 20) | (0x7 << 14));
    reg_val |= ((trans_param->data_reg_nofifo << 28)\
    | (trans_param->cmd_type << 27) | (trans_param->cs_sele << 20)\
    | (trans_param->crypto_en << 16) | (trans_param->rxds_en << 15)\
    | (trans_param->trans_dir << 14));
    REG32(SFC_INTERFACE_CFG_REG(cmd_bank_num)) = reg_val;

    REG32(SFC_INTERVAL_CFG_REG(cmd_bank_num)) = (trans_param->trans_gap);

//    printf("interface reg = %x\n",REG32(SFC_INTERFACE_CFG_REG(cmd_bank_num)));
}

/**
 * @brief  sfc_trans_start
 * @param  data_path
 * @param  cmd_bank_sum
 * @return
 * @note   spi传输开始使能
 */
void sfc_trans_start(T_SFC_TRANS_DATA_PATH data_path, unsigned int cmd_bank_sum)
{
    unsigned int reg_val;

    reg_val = REG32(SFC_TRANSFER_ENABLE_CFG_REG);
    reg_val &= ~(0xf);
    reg_val |= cmd_bank_sum;    //真实次数为配置值+1
    REG32(SFC_TRANSFER_ENABLE_CFG_REG) = reg_val;

    if(data_path == BDMA_MODE || data_path == EDMA_MODE)
        REG32(SFC_TRANSFER_ENABLE_CFG_REG) |= (0x1 << 30);
    else
        REG32(SFC_TRANSFER_ENABLE_CFG_REG) &= ~(0x1 << 30);

    REG32(SFC_TRANSFER_ENABLE_CFG_REG) |= (0x1 << 31);   //启动传输

//    printf("SFC_TRANSFER_ENABLE_CFG_REG = %x\n",REG32(SFC_TRANSFER_ENABLE_CFG_REG));
}

/**
 * @brief  sfc_cmd_polling_cfg
 * @param  match_data //轮询目标值
 * @param  match_mask //mask bit，0：不屏蔽，1：屏蔽
 * @param  match_mode //and模式：未屏蔽所有bit与目标一致，or模式：未屏蔽有一个bit与目标一致
 * @param  cmd_bank_num
 * @return
 * @note   硬件轮询命令配置
 */
void sfc_cmd_polling_cfg(unsigned int match_data, unsigned int match_mask, unsigned int match_mode, unsigned int cmd_bank_num)
{
    REG32(SFC_POLLING_DATA_REG(cmd_bank_num)) = match_data;
    REG32(SFC_POLLING_MASK_REG(cmd_bank_num)) = match_mask;
    REG32(SFC_INTERFACE_CFG_REG(cmd_bank_num)) &= ~(0x1 << 26);
    REG32(SFC_INTERFACE_CFG_REG(cmd_bank_num)) |= (match_mode << 26);
}
#if 0
void timer_start_new(void)
{
    //关闭timer
    REG32(0x21111000+ 0x08) = 0;

    //清中断
    REG32(0x21111000+ 0x0C) = 0x1;

    //设置分频为24分频，即1us为单位
    REG32(0x21111000+ 0x14) = 12;

    //计数值设定
    REG32(0x21111000+ 0) = 0xffffffff;


    //启动timer + load 入新值
    REG32(0x21111000+ 0x08) = (1 | (0x1<<3));
}

unsigned int timer_finish_new(void)
{
    unsigned int reg_val;
    unsigned int consuming_time;

    //read timer cnt reg
    reg_val = REG32(0x21111000+ 0x04);

    //计算消耗的时间. 单位: us
    consuming_time = (0xffffffff - reg_val) / 1;

    return consuming_time;

}

/**
 * @brief  Data Contrast
 * @param  buf
 * @param  con
 * @param  size
 * @return
 * @note   数组与固定数值对比
 */
bool data_contrast(unsigned char *buf, unsigned char con, unsigned int size)
{
    unsigned int i,j;
    for (i=0; i<size; i++)
    {
        if (buf[i] != con)
        {
            printf("\n!!!!!!!!!!!!!!data error!!!!!!!!!!!!!\n");
            if(i >= 8)
            {
                for(i -= 4,j = 0; j < (size >= 12 ? 8:size); j++,i++)
                {
                    printf("error:data_buf[%d]= 0x%x,0x%x\n", i, buf[i],con);
                }
            }
            else
            {
                for(i = 0; i < (size >= 8 ? 8:size); i++)
                {
                    printf("error:data_buf[%d]= 0x%x,0x%x\n", i, buf[i],con);
                }
            }

            for(; i < size; i++)
            {
                printf("error:data_buf[%d]= 0x%x,0x%x\n", i, buf[i],con);
                // if('e' == getch())
                    return false;
            }
            // if('e' == getch())
                return false;
        }
    }
    return true;
}
#endif
/**
 * @brief sfc pinmux
 * @author yang jianxiong
 * @date 2023-9-12
*/
void sfc_pinmux_config(void)
{
#define PIN_CONTROL_REG4 0x08000188
    unsigned int regv;

    //config share pin, funcmux_reg4, XGPIO 42銆乆GPIO 43銆乆GPIO 44銆乆PGIO 45銆乆GPIO 46銆乆GPIO 47
#if defined(CONFIG_3918AV130_CODE)
    regv = inl(PIN_CONTROL_REG4);
    regv &= ~((0x3 << 6) | (0x3 << 9) | (0x3 << 12) | (0x3 << 15) | (0x3 << 18) | (0x3 << 21));
    regv |= ((0x2 << 6) | (0x2 << 9) | (0x2 << 12) | (0x2 << 15) | (0x2 << 18) | (0x2 << 21));
    outl(regv, PIN_CONTROL_REG4);
#else
    regv = inl(PIN_CONTROL_REG4);
    regv = 0;
    regv |= ((0x2 << 9) | (0x2 << 12) | (0x2 << 15) | (0x2 << 18) | (0x2 << 21) | (0x2 << 24));
    outl(regv, PIN_CONTROL_REG4);
#endif
    // regv = inl(0x08000288);
    // regv &= ~(0x3f << 10);
    // regv |= (0x3f << 10);
    // outl(regv, 0x08000288);

    // printf("REG-0x08000188 %.8x\n", inl(0x08000188));
    // printf("REG-0x080001AC %.8x\n", inl(0x080001AC));
    // printf("REG-0x080001D4 %.8x\n", inl(0x080001D4));
    // printf("REG-0x080001C4 %.8x\n", inl(0x080001C4));
    // printf("REG-0x08000268 %.8x\n", inl(0x08000268));
    // printf("REG-0x08000278 %.8x\n", inl(0x08000278));
    // printf("REG-0x08000288 %.8x\n", inl(0x08000288));
}

/**
 * @brief sfc clk rst config
 * @author yang jianxiong
 * @date 2023-9-12
*/
void sfc_clk_rst_config(void)
{
    unsigned long reg_val;
#define RESET_SFC_HSCLK                     31
#define RESET_SFC                           5
#define CLOCK_CTRL_SPI0                     (1<<5)

    //1. 关闭SFC gclk gate
    reg_val = inl(CLOCK_CTRL_REG);
    reg_val |= CLOCK_CTRL_SPI0;
    outl(reg_val, CLOCK_CTRL_REG);

    //2. 由于有gclk和phyclk两个时钟域，因此必须两个都复位
    reg_val = inl(RESET_CTRL_REG);
    reg_val |= ((0x1UL<<RESET_SFC_HSCLK) | (0x1UL<<RESET_SFC));
    outl(reg_val, RESET_CTRL_REG);

    //3. 设置SFC_PHY_CLK分频比,这里的步骤:
    //关sfcphy clk  ->  HOLD 复位sfcphy clk -> 变频-> 开sfc_phyclk  -> 放开复位sfcphy clk

    //关sfc_phyclk
    reg_val = inl(SDADC_SPI0_SDDAC_CTRL_REG);
    reg_val &= ~(0x1<<18);
    outl(reg_val, SDADC_SPI0_SDDAC_CTRL_REG);

    //HOLD住sfc_phyclk复位
    reg_val = inl(RESET_CTRL_REG);
    reg_val |= (0x1UL<<RESET_SFC_HSCLK);
    outl(reg_val, RESET_CTRL_REG);

    //配置分频
    reg_val = inl(SDADC_SPI0_SDDAC_CTRL_REG);
    reg_val &= ~(0xFF<<10);
    reg_val |= ((clk_get_core_pll_freq()/200000000 - 1)<<10);
    outl(reg_val, SDADC_SPI0_SDDAC_CTRL_REG);

    reg_val = inl(SDADC_SPI0_SDDAC_CTRL_REG);
    reg_val |= (0x1<<19);
    outl(reg_val, SDADC_SPI0_SDDAC_CTRL_REG);

    //等待变频完成
    while((inl(SDADC_SPI0_SDDAC_CTRL_REG) & (0x1<<19)))
    {;}

    //开sfc_phyclk
    reg_val = inl(SDADC_SPI0_SDDAC_CTRL_REG);
    reg_val |= (0x1<<18);
    outl(reg_val, SDADC_SPI0_SDDAC_CTRL_REG);

    //4. 打开SFC gclk gate
    reg_val = inl(CLOCK_CTRL_REG);
    reg_val &= ~CLOCK_CTRL_SPI0;
    outl(reg_val, CLOCK_CTRL_REG);

    //放开sfc_phyclk复位
    reg_val = inl(RESET_CTRL_REG);
    reg_val &= ~((0x1UL<<RESET_SFC_HSCLK) | (0x1UL<<RESET_SFC));
    outl(reg_val, RESET_CTRL_REG);
}

/**
 * @brief  sfc_bdma_ahb_axi_cfg
 * @param
 * @return
 * @note   内置dma配置
 */
void sfc_bdma_ahb_axi_cfg(void)
{
    unsigned int awqos = 0;
    unsigned int arqos = 0;
    unsigned int awid = 0;
    unsigned int arid = 0;
    unsigned int reg_val = 0;

    reg_val |= ((awqos << 20) | (arqos << 16) | (awid << 8) | (arid << 0));
    //REG32(SFC_DMA_CFG_REG0) = reg_val;
    outl(reg_val,SFC_DMA_CFG_REG0);

    //暂时使用默认值
    reg_val = REG32(SFC_DMA_CFG_REG1);
    //REG32(SFC_DMA_CFG_REG1) = reg_val;
    outl(reg_val,SFC_DMA_CFG_REG1);
    
}
#if 0
/**
 * @brief  sfc_xip_addr_size_cfg
 * @param  *initi_param
 * @return
 * @note   xip地址容量配置
 */
void sfc_xip_addr_size_cfg(T_SFC_XIP_INITI_PARAM *initi_param, unsigned int bar_num)
{
    unsigned int reg_val = 0;

    //配置系统映射地址和选择flash器件
    reg_val |= ((initi_param->system_addr&0xfffff000) | (0x1 << 9) | (initi_param->cs_sele));
    REG32(SFC_XIP_MODE_ADDR_REG(bar_num)) = reg_val;

    //配置系统映射容量,要求配置容量为Cache Line Size的整数倍减去1
    REG32(SFC_XIP_MODE_SIZE_REG(bar_num)) = (initi_param->bar_size & 0xfffffff) - 1;

    //配置flash地址
    REG32(SFC_XIP_MODE_FLASH_ADDR_REG(bar_num)) = initi_param->flash_addr;

    sfc_intr_enable(SFC_XIP_MODE_ERROR_STATUS | SFC_XIP_ADDR_ERROR_STATUS);
//    printf("xip mode addr reg = %x\n", REG32(SFC_XIP_MODE_ADDR_REG(bar_num)));
}
#endif
#if 0
/**
 * @brief  sfc_print_all_reg
 * @param
 * @return
 * @note   打印静态配置寄存器
 */
void sfc_print_static_reg(void)
{
    printf("\nSFC_XIP_MODE_ADDR0(0x%x) = %lx\n", SFC_XIP_MODE_ADDR_REG(0), REG32(SFC_XIP_MODE_ADDR_REG(0)));
    printf("SFC_XIP_MODE_SIZE0(0x%x) = %lx\n", SFC_XIP_MODE_SIZE_REG(0), REG32(SFC_XIP_MODE_SIZE_REG(0)));
    printf("SFC_XIP_MODE_FLASH_ADDR0(0x%x) = %lx\n", SFC_XIP_MODE_FLASH_ADDR_REG(0), REG32(SFC_XIP_MODE_FLASH_ADDR_REG(0)));
    printf("SFC_XIP_MODE_ADDR1(0x%x) = %lx\n", SFC_XIP_MODE_ADDR_REG(1), REG32(SFC_XIP_MODE_ADDR_REG(1)));
    printf("SFC_XIP_MODE_SIZE1(0x%x) = %lx\n", SFC_XIP_MODE_SIZE_REG(1), REG32(SFC_XIP_MODE_SIZE_REG(1)));
    printf("SFC_XIP_MODE_FLASH_ADDR1(0x%x) = %lx\n", SFC_XIP_MODE_FLASH_ADDR_REG(1), REG32(SFC_XIP_MODE_FLASH_ADDR_REG(1)));
    printf("SFC_FIFO_THRES_STATUS_REG(0x%x) = %lx\n",SFC_FIFO_THRES_STATUS_REG, REG32(SFC_FIFO_THRES_STATUS_REG));
    printf("SFC_STATUS_REG(0x%x) = %lx\n",SFC_STATUS_REG, REG32(SFC_STATUS_REG));
    printf("SFC_INTR_ENABLE_REG(0x%x) = %lx\n",SFC_INTR_ENABLE_REG, REG32(SFC_INTR_ENABLE_REG));
    printf("SFC_WORK_MODE_REG(0x%x) = %lx\n",SFC_WORK_MODE_REG, REG32(SFC_WORK_MODE_REG));
    printf("SFC_CS_TIMING_REG(0x%x) = %lx\n",SFC_CS_TIMING_REG, REG32(SFC_CS_TIMING_REG));
    printf("SFC_BAUD_RATE_REG(0x%x) = %lx\n",SFC_BAUD_RATE_REG, REG32(SFC_BAUD_RATE_REG));
    printf("SFC_WAIT_PHASE_REG(0x%x) = %lx\n",SFC_WAIT_PHASE_REG, REG32(SFC_WAIT_PHASE_REG));
    printf("SFC_XIP_TIMEOUT_REG(0x%x) = %lx\n",SFC_XIP_TIMEOUT_REG, REG32(SFC_XIP_TIMEOUT_REG));
    printf("SFC_RXDS_CFG_REG0(0x%x) = %lx\n",SFC_RXDS_CFG_REG0, REG32(SFC_RXDS_CFG_REG0));
    printf("SFC_RXDS_CFG_REG1(0x%x) = %lx\n",SFC_RXDS_CFG_REG1, REG32(SFC_RXDS_CFG_REG1));
    printf("SFC_RXDS_CFG_REG2(0x%x) = %lx\n",SFC_RXDS_CFG_REG2, REG32(SFC_RXDS_CFG_REG2));
    printf("SFC_DMA_CFG_REG0(0x%x) = %lx\n",SFC_DMA_CFG_REG0, REG32(SFC_DMA_CFG_REG0));
    printf("SFC_DMA_CFG_REG1(0x%x) = %lx\n",SFC_DMA_CFG_REG1, REG32(SFC_DMA_CFG_REG1));
    printf("SFC_DATA_ENCRY_KEY_REG0(0x%x) = %lx\n",SFC_DATA_ENCRY_KEY_REG0, REG32(SFC_DATA_ENCRY_KEY_REG0));
    printf("SFC_DATA_ENCRY_KEY_REG1(0x%x) = %lx\n",SFC_DATA_ENCRY_KEY_REG1, REG32(SFC_DATA_ENCRY_KEY_REG1));
    printf("SFC_DATA_ENCRY_KEY_REG2(0x%x) = %lx\n",SFC_DATA_ENCRY_KEY_REG2, REG32(SFC_DATA_ENCRY_KEY_REG2));
    printf("SFC_DATA_ENCRY_KEY_REG3(0x%x) = %lx\n",SFC_DATA_ENCRY_KEY_REG3, REG32(SFC_DATA_ENCRY_KEY_REG3));
    printf("SFC_DATA_ENCRY_ROUND_REG(0x%x) = %lx\n",SFC_DATA_ENCRY_ROUND_REG, REG32(SFC_DATA_ENCRY_ROUND_REG));
    printf("SFC_TRANSFER_ENABLE_CFG_REG(0x%x) = %lx\n",SFC_TRANSFER_ENABLE_CFG_REG, REG32(SFC_TRANSFER_ENABLE_CFG_REG));
    printf("SFC_DATA_FIFO_REG(0x%x) = %lx\n",SFC_DATA_FIFO_REG, REG32(SFC_DATA_FIFO_REG));
    printf("SFC_DMA_START_ADDR_REG(0x%x) = %lx\n",SFC_DMA_START_ADDR_REG, REG32(SFC_DMA_START_ADDR_REG));
    printf("SFC_DMA_LENGTH_REG(0x%x) = %lx\n",SFC_DMA_LENGTH_REG, REG32(SFC_DMA_LENGTH_REG));
    printf("SFC_TRANSFRE_ABORT_CFG_REG(0x%x) = %lx\n", SFC_TRANSFRE_ABORT_CFG_REG, REG32(SFC_TRANSFRE_ABORT_CFG_REG));
}

/**
 * @brief  sfc_print_all_reg
 * @param
 * @return
 * @note   打印静态配置寄存器
 */
void l2_print_static_reg(unsigned int buf_num)
{
    printf("\nL2_REG_DMA_ADDR_%d(0x%x) = %lx\n", buf_num, 0x20140000+0x4*buf_num, REG32(0x20140000+0x4*buf_num));
    printf("\nL2_REG_DMA_CNT_%d(0x%x) = %lx\n", buf_num, 0x20140040+0x4*buf_num, REG32(0x20140040+0x4*buf_num));
    printf("\nL2_REG0(0x%x) = %lx\n", 0x20140080, REG32(0x20140080));
    printf("\nL2_REG1(0x%x) = %lx\n", 0x20140084, REG32(0x20140084));
    printf("\nL2_REG2(0x%x) = %lx\n", 0x20140088, REG32(0x20140088));
    printf("\nL2_REG3(0x%x) = %lx\n", 0x2014008c, REG32(0x2014008c));
    printf("\nL2_REG4(0x%x) = %lx\n", 0x20140090, REG32(0x20140090));
    printf("\nL2_REG5(0x%x) = %lx\n", 0x20140094, REG32(0x20140094));
    printf("\nL2_REG6(0x%x) = %lx\n", 0x20140098, REG32(0x20140098));
    printf("\nL2_REG7(0x%x) = %lx\n", 0x2014009c, REG32(0x2014009c));
    printf("\nL2_REG8(0x%x) = %lx\n", 0x201400a0, REG32(0x201400a0));
    printf("\nL2_REG9(0x%x) = %lx\n", 0x201400a4, REG32(0x201400a4));
    printf("\nL2_REG10(0x%x) = %lx\n", 0x201400a8, REG32(0x201400a8));
}
#endif
void l2_dma_buf1(void *addr, unsigned int size, unsigned int dir)
{
#define READ_L2         0
#define WRITE_L2        1
#define	L2_REG_BASE_ADDR_0	(0x20140000)
#define	L2_REG_BUF_ASSIGN0	(L2_REG_BASE_ADDR_0+0x90)
#define	L2_REG_DMA_ADDR_0		(L2_REG_BASE_ADDR_0+0x00)
#define	L2_REG_DMA_CNT_0		(L2_REG_BASE_ADDR_0+0x40)
#define	L2_REG_DMA_REQ_0		(L2_REG_BASE_ADDR_0+0x80)
#define	L2_REG_CBUF_CFG_0		(L2_REG_BASE_ADDR_0+0x88)
#define L2_DMA_EN			(1 << 0)
//buf1
#define EN_BUF_1_DMA		(1 << 1)
#define WRITE_BUF_1			(1 << 9)
#define READ_BUF_1			(0 << 9)
#define EN_BUF_1			(1 << 17)
#define CLR_BUF_1_FLAG		(1 << 25)
#define DMA_REQ_BUF_1		(1 << 25)
    unsigned int smloop;
    unsigned int bgloop;

    //DMA Address of Buf 0
    *(volatile unsigned int *) (L2_REG_DMA_ADDR_0 + 0x04) = (unsigned int)addr;

    if (size > 4096)
    {
        smloop = 64;
        bgloop = size/(4*1024)-1;
        //DMA Cnt of Buf 0
        //256 byte(trans size)/64 byte, bit[7:0], max trans size: 0xff*16= 4080 bytes
        *(volatile unsigned int *) (L2_REG_DMA_CNT_0 + 0x04) = 0x0 | (smloop) | (bgloop << 16);
    }
    else
    {
        //DMA Cnt of Buf 0
        //256 byte(trans size)/64 byte, bit[7:0], max trans size: 0xff*16= 4080 bytes
        *(volatile unsigned int *) (L2_REG_DMA_CNT_0 + 0x04) = 0x0 | (size/64);
    }

    //Validate the Buf 0
    if (dir == WRITE_L2) {//write
        *(volatile unsigned int *) (L2_REG_CBUF_CFG_0) = 0x0 | EN_BUF_1_DMA | WRITE_BUF_1 | EN_BUF_1 | CLR_BUF_1_FLAG;
    } else {//read
        *(volatile unsigned int *) (L2_REG_CBUF_CFG_0) = 0x0 | EN_BUF_1_DMA | READ_BUF_1 | EN_BUF_1 | CLR_BUF_1_FLAG;
    }

    //Base Address of SFC Buffer
    //*(volatile unsigned int *) (L2_REG_BUF_ASSIGN0) = 0x0 | (0x1 << 21) | (0x1 << 24);
    *(volatile unsigned int *) (L2_REG_BUF_ASSIGN0) = 0x1;

    //Start the DMA transfer to program the flash
    *(volatile unsigned int *) (L2_REG_DMA_REQ_0) = 0x0 | L2_DMA_EN | DMA_REQ_BUF_1;
}
#if 0
void l2_dma_bufx(void *addr, unsigned int size, unsigned int dir, unsigned int buf_num)
{
    unsigned int smloop;
    unsigned int bgloop;

    //DMA Address of Buf 0
    *(volatile unsigned int *) (L2_REG_DMA_ADDR_0 + 0x04*buf_num) = (unsigned int) addr;

    if (size > 1024)
    {
        smloop = 64;
        bgloop = size/(4*1024)-1;
        //DMA Cnt of Buf buf_num
        //256 byte(trans size)/64 byte, bit[7:0], max trans size: 0xff*16= 4080 bytes
        *(volatile unsigned int *) (L2_REG_DMA_CNT_0 + 0x04*buf_num) = 0x0 | (smloop) | (bgloop << 16);
    }
    else
    {
        //DMA Cnt of Buf buf_num
        //256 byte(trans size)/64 byte, bit[7:0], max trans size: 0xff*16= 4080 bytes
        *(volatile unsigned int *) (L2_REG_DMA_CNT_0 + 0x04*buf_num) = 0x0 | (size/64);
    }

    //Validate the Buf buf_num
    if (dir == WRITE_L2) {//write
        *(volatile unsigned int *) (L2_REG_CBUF_CFG_0) = 0x0 | (1 << buf_num) | (1 << (8 + buf_num)) | (1 << (16 + buf_num)) | (1 << (24 + buf_num));
    } else {//read
        *(volatile unsigned int *) (L2_REG_CBUF_CFG_0) = 0x0 | (1 << buf_num) | (0 << (8 + buf_num)) | (1 << (16 + buf_num)) | (1 << (24 + buf_num));
    }

    //Base Address of SFC Buffer
    //*(volatile unsigned int *) (L2_REG_BUF_ASSIGN0) = 0x0 | (0x1 << 21) | (0x1 << 24);
    *(volatile unsigned int *) (L2_REG_BUF_ASSIGN0) = buf_num;

    //Start the DMA transfer to program the flash
    *(volatile unsigned int *) (L2_REG_DMA_REQ_0) = 0x0 | (1 << 0) | (1 << (24 + buf_num));
}
#endif
/**
 * @brief  norflash_trans_param
 * @param  *flash_trans_param
 * @return
 * @note   flash传输参数配置
 */
void norflash_trans_param(T_FLASH_PARAM *flash_trans_param)
{
    memcpy(&flash_param, flash_trans_param, sizeof(flash_param));
}

/**
 * @brief  sfc interrupt check
 * @param  intr_enable
 * @param  intr_wait_sele
 * @return
 * @note   检查对应中断是否发生
 */
bool sfc_intr_check(unsigned int intr_singal, T_SFC_INTR_WAIT intr_wait_sele)
{
    unsigned int cnt = ERROR_DELAY;

    while(1)
    {
        /*if((sfc_intr_flag & intr_singal) == intr_singal)
            break;*/
        if((REG32(SFC_STATUS_REG) & intr_singal) == intr_singal)
        {
            REG32(SFC_STATUS_REG) |= intr_singal;
            break;
        }
        if(intr_wait_sele == NO_WAIT_INTR)
            return false;
        if(--cnt <= 0)
        {
            printf("wait too long for an interrupt!!!");
            printf("input 'e' to exit,no input to continue wait.\n");
            cnt = ERROR_DELAY;
        }
        // if('e' == getch_no_wait())
        //     return false;
    }
    // sfc_clean_intr_flag(intr_singal);//tiaoshi
    return true;
}

/**
 * @brief  spi intr enable
 * @param  intr_signal
 * @return
 * @note   中断使能
 */
void sfc_intr_enable(unsigned int intr_signal)
{
    unsigned int reg_val;

    reg_val = REG32(SFC_INTR_ENABLE_REG);
    reg_val |= intr_signal;
    REG32(SFC_INTR_ENABLE_REG) = reg_val;
}

/**
 * @brief  spi_intr_disable
 * @param  intr_signal
 * @return
 * @note   中断禁能
 */
void sfc_intr_disable(unsigned int intr_signal)
{
    unsigned int reg_val;

    reg_val = REG32(SFC_INTR_ENABLE_REG);
    reg_val &= ~intr_signal;
    REG32(SFC_INTR_ENABLE_REG) = reg_val;
}

/**
 * @brief  spi all intr disable
 * @param
 * @return
 * @note   关闭所有中断
 */
void sfc_all_intr_disable(void)
{
    // sfc_intr_flag = 0;//tiaoshi
    REG32(SFC_INTR_ENABLE_REG) = 0;
}

/**
 * @brief  sfc_fifo_clear
 * @param
 * @return
 * @note   复位sfc控制器的FIFO指针
 */
void sfc_fifo_clear(void)
{
    REG32(SFC_WORK_MODE_REG) |= (0x1 << 4);
    while(REG32(SFC_WORK_MODE_REG)&(0x1<<4));//等待FIFO指针复位
}

/**
 * @brief  sfc_sdlu_cfg
 * @param  samp_delay
 * @param  sdlu_delay
 * @return
 * @note   sdlu延时配置
 */
void sfc_sdlu_cfg(unsigned int samp_delay, unsigned int sdlu_delay)
{
    unsigned int reg_val;

    reg_val = REG32(SFC_RXDS_CFG_REG2);
    reg_val &= ~((0xf<<28) | (0xff));
    reg_val |= ((sdlu_delay) | (samp_delay)<<28);
    REG32(SFC_RXDS_CFG_REG2) = reg_val;
}

/**
 * @brief  sfc_sclk_div_cfg
 * @param  div
 * @return
 * @note   spi sclk 分频配置
 */
void sfc_sclk_div_cfg(unsigned int div)
{
    unsigned int reg_val;

    reg_val = REG32(SFC_BAUD_RATE_REG);
    reg_val &= ~(0x3ff);
    reg_val |= (div & 0x3ff) - 1;//SCLK分频
    REG32(SFC_BAUD_RATE_REG) = reg_val;

    //更改SCLK频率，MDLU/SDLU需要重新更新。
//    sfc_mdlu_cfg();   //当前FPGA版本不支持MDLU/SDLU
}

/**
 * @brief  sfc_work_mode_cfg
 * @param  mode_cfg
 * @return
 * @note   xip，swc模式配置
 */
void sfc_work_mode_cfg(T_SFC_WORK_MODE mode_cfg)
{
    unsigned int cnt = ERROR_DELAY;

    if(mode_cfg == SFC_SWC_MODE)
    {
        REG32(SFC_WORK_MODE_REG) |= (0x1 << 0);    //停止xip访问请求
        while(!(REG32(SFC_WORK_MODE_REG) & 0x2))    //等待当前xip传输停止
        {
            if(--cnt <= 0)
                printf("wait for xip transmission stop too long\n");
        }
        REG32(SFC_WORK_MODE_REG) &= ~(0x1 << 2);    //SWC模式
    }
    else if(mode_cfg == SFC_XIP_MODE)
    {
        if((REG32(SFC_STATUS_REG) & SFC_TRANS_IDLE_STATUS))
        {
            REG32(SFC_WORK_MODE_REG) |= (0x1 << 2);        //xip模式
            REG32(SFC_WORK_MODE_REG) &= ~(0x1 << 0);    //禁能XIP hold,XIP hold为xip模式总开关，需放在最后
        }
        else
            printf( "spi transfer idle status error\n");
    }
    else
        printf( "func param error\n");

//    printf("SFC_WORK_MODE_REG = %x\n",REG32(SFC_WORK_MODE_REG));
}

/**
 * @brief  sfc_dual_flash_parallel_en
 * @param  sele
 * @return
 * @note   并联flash使能
 */
void sfc_dual_flash_parallel_en(bool sele)
{
    if(sele == true)
        REG32(SFC_WORK_MODE_REG) |= (0x1 << 6);    //并联flash使能
    else if(sele == false)
        REG32(SFC_WORK_MODE_REG) &= ~(0x1 << 6);    //并联flash禁能
    else
        printf("func param error");
}

/**
 * @brief  sfc_fifo_thres_cfg
 * @param  rx_thres
 * @param  tx_thres
 * @return
 * @note   FIFO阈值配置
 */
void sfc_fifo_thres_cfg(unsigned int rx_thres, unsigned int tx_thres)
{
	unsigned int reg_val;

	reg_val = REG32(SFC_FIFO_THRES_STATUS_REG);
	reg_val &= ~(0xffff);
	reg_val |= (tx_thres & 0xff) | ((rx_thres & 0xff) << 8);
	REG32(SFC_FIFO_THRES_STATUS_REG) = reg_val;
}

/**
 * @brief  sfc_get_fifo_thres
 * @param  *rx_thres
 * @param  *tx_thres
 * @return
 * @note   FIFO阈值配置
 */
void sfc_get_fifo_thres(unsigned int *rx_thres, unsigned int *tx_thres)
{
    unsigned int reg_val;

    reg_val = REG32(SFC_FIFO_THRES_STATUS_REG);
    *tx_thres = reg_val & 0xff;
    *rx_thres = (reg_val >> 8) & 0xff;
}

/**
 * @brief  sfc_hyper_bus_en
 * @param  sele
 * @return
 * @note   spi bus和hyper bus切换
 */
void sfc_hyper_bus_en(bool sele)
{
    if(sele == true)
    {
        REG32(SFC_BAUD_RATE_REG) &= ~(0x1 << 30);    //sclk反相使能
//        REG32(SFC_BAUD_RATE_REG) |= (0x1 << 30);    //sclk反相禁能
//        REG32(SFC_WORK_MODE_REG) |= (0x1 << 5);    //hyper bus使能
    }
    else
    {

        REG32(SFC_BAUD_RATE_REG) |= (0x1 << 30);    //sclk反相禁能
//        REG32(SFC_BAUD_RATE_REG) &= ~(0x1 << 30);    //sclk反相使能
//        REG32(SFC_WORK_MODE_REG) &= ~(0x1 << 5);       //hyper bus禁能
    }
//    printf("sfc baud rate reg = %x\n", REG32(SFC_BAUD_RATE_REG));
}

/**
 * @brief  sfc_spi_trans_format_cfg
 * @param  sele
 * @return
 * @note   mode0，mode3配置
 */
void sfc_spi_trans_format_cfg(T_SFC_TRANS_FORMAT sele)
{
    if(sele == CLK_MODE0)
        REG32(SFC_BAUD_RATE_REG) &= ~(0x1 << 31);
    else if(sele == CLK_MODE3)
        REG32(SFC_BAUD_RATE_REG) |= (0x1 << 31);
    else
        printf( "func param error");
}

/**
 * @brief  sfc_spi_cs_timing_cfg
 * @param  start_cfg
 * @param  end_cfg
 * @return
 * @note   cs timing配置，计时单位SFC_PHY_CLK
 */
void sfc_spi_cs_timing_cfg(unsigned int start_cfg, unsigned int end_cfg)
{
    unsigned int reg_val;

    reg_val = REG32(SFC_CS_TIMING_REG);
    reg_val &= ~(0xff);
    reg_val |= (((start_cfg-1) & 0xf) | (((end_cfg-1) & 0xf) << 4));
    REG32(SFC_CS_TIMING_REG) = reg_val;
//    printf("cs timing reg = %x\n", REG32(SFC_CS_TIMING_REG));
}

/**
 * @brief  sfc_initi
 * @param  *initi_param
 * @return
 * @note   sfc初始化函数
 */
void sfc_initi(T_SFC_INITI_PARAM *initi_param)
{
	//unsigned int reg_val;
    //printf("%s %d\n", __func__, __LINE__);
    sfc_sclk_div_cfg(initi_param->clk_div);
    sfc_fifo_thres_cfg(initi_param->rxfifo_thres,initi_param->txfifo_thres);
	sfc_work_mode_cfg(initi_param->work_mode);
	sfc_dual_flash_parallel_en(initi_param->flash_parallel);
	sfc_hyper_bus_en(initi_param->hyper_bus);
	sfc_spi_trans_format_cfg(initi_param->trans_format);
	sfc_spi_cs_timing_cfg(10, 10);  //注意根据器件datasheet配置
    sfc_all_intr_disable();

    sfc_sdlu_cfg(1, 0);

//    sfc_mdlu_cfg();    //FPGA版本暂不支持
    //sfc_bdma_ahb_axi_cfg();//tiaoshi
    // sfc_intr_initi();//tiaoshi
}

/**
 * @brief  sfc_trans_attribute
 * @param  data_reg_nofifo
 * @param  trans_dir
 * @param  trans_num
 * @return
 * @note
 */
static bool sfc_trans_attribute(bool data_reg_nofifo, T_SFC_TRANS_DIR trans_dir, T_SFC_TRANS_NUM trans_num)
{
    unsigned int reg_val;

    reg_val = REG32(SFC_INTERFACE_CFG_REG(trans_num));

    //无data phase和polling cmd不用处理数据
    if(((reg_val >> 27) & 0x1) == 1 || ((reg_val >> 10) & 0x1) == 0)
        return false;

    if(((reg_val >> 28) & 0x1) == data_reg_nofifo)
    {
        if(((reg_val >> 14) & 0x1) == trans_dir)
            return true;
    }

    return false;
}

/**
 * @brief  sfc_get_trans_data_cnt
 * @param  trans_num
 * @return
 * @note
 */
static unsigned int sfc_get_trans_data_cnt(T_SFC_TRANS_NUM trans_num)
{
    unsigned int data_cnt;

    data_cnt = ((REG32(SFC_PHASE_LENGTH_CFG_REG(trans_num)) >> 6) & 0x3ffffff) + 1;

    if(REG32(SFC_WORK_MODE_REG) & (0x1 << 6))
        data_cnt *= 2;

    return data_cnt;
}

/**
 * @brief  sfc_set_spi_trans_phase
 * @param  *phase_cfg
 * @param  cmd_bank
 * @return
 * @note   spi传输各个phase配置， Wait Phase单位SCLK
 */
void sfc_set_spi_trans_phase(T_SFC_SPI_PHASE_CFG *phase_cfg, unsigned int cmd_bank_num)
{
    unsigned int reg_val0;
    unsigned int reg_val1;

    reg_val0 = REG32(SFC_INTERFACE_CFG_REG(cmd_bank_num));
    reg_val1 = REG32(SFC_PHASE_LENGTH_CFG_REG(cmd_bank_num));

    if(phase_cfg->inst_phase.enable == true)
    {
        REG32(SFC_INST_PHASE_REG(cmd_bank_num)) = phase_cfg->inst_phase.data;
        // if(tiaoshi_flag == 1){
        //     tiaoshi_flag = 2;
        //     int ii;
        //     for(ii = 0; ii < 9; ii++)
        //         printf("tiaoshi REG32(SFC_INST_PHASE_REG(%d)) = 0x%x\r\n",ii , REG32(SFC_INST_PHASE_REG(ii)));
        // }
        reg_val0 &= ~((0x1 << 11) | (0x3 << 0));
        reg_val1 &= ~(0x3 << 0);
        reg_val0 |= (phase_cfg->inst_phase.spl_mode << 11) | (phase_cfg->inst_phase.enable << 6) | \
        (phase_cfg->inst_phase.bus_wire << 0);
        reg_val1 |= ((phase_cfg->inst_phase.cnt - 1) << 0);
    }
    else if(phase_cfg->inst_phase.enable == false)
    {
        REG32(SFC_INST_PHASE_REG(cmd_bank_num)) = 0;
        reg_val0 &= ~((0x1 << 11) | (0x1 << 6) | (0x3 << 0));
        reg_val1 &= ~(0x3 << 0);
    }

    if(phase_cfg->addr_phase.enable == true)
    {
        REG32(SFC_ADDR_PHASE_REG(cmd_bank_num)) = phase_cfg->addr_phase.data;
        reg_val0 &= ~((0x1 << 12) | (0x3 << 2));
        reg_val1 &= ~(0x3 << 2);
        reg_val0 |= (phase_cfg->addr_phase.enable << 7);
        reg_val0 |= (phase_cfg->addr_phase.spl_mode << 12) | (phase_cfg->addr_phase.bus_wire << 2);
        reg_val1 |= ((phase_cfg->addr_phase.cnt - 1) << 2);
    }
    else if(phase_cfg->addr_phase.enable == false)
    {
        REG32(SFC_ADDR_PHASE_REG(cmd_bank_num)) = 0;
        reg_val0 &= ~(0x1 << 7);
        reg_val1 &= ~(0x3 << 2);
    }

    if(phase_cfg->mode_phase.enable == true)
    {
        REG32(SFC_MODE_PHASE_REG(cmd_bank_num)) = phase_cfg->mode_phase.data;
        reg_val0 &= ~((0x1 << 12) | (0x3 << 2));
        reg_val1 &= ~(0x3 << 4);
        reg_val0 |= (phase_cfg->mode_phase.enable << 8);
        reg_val0 |= (phase_cfg->mode_phase.spl_mode << 12) | (phase_cfg->mode_phase.bus_wire << 2);
        reg_val1 |= ((phase_cfg->mode_phase.cnt - 1) << 4);
    }
    else if(phase_cfg->mode_phase.enable == false)
    {
        REG32(SFC_MODE_PHASE_REG(cmd_bank_num)) = 0;
        reg_val0 &= ~(0x1 << 8);
        reg_val1 &= ~(0x3 << 4);
    }

    if(phase_cfg->wait_phase.enable == true)
    {
        REG32(SFC_WAIT_PHASE_REG) = phase_cfg->wait_phase.cnt - 1;
        reg_val0 &= ~((0x1 << 12) | (0x3 << 2));
        reg_val0 |= (phase_cfg->wait_phase.enable << 9);
        reg_val0 |= (phase_cfg->wait_phase.spl_mode << 12) | (phase_cfg->wait_phase.bus_wire << 2);
    }
    else if(phase_cfg->wait_phase.enable == false)
    {
        reg_val0 &= ~(0x1 << 9);
        REG32(SFC_WAIT_PHASE_REG) = 0;
    }

    if(phase_cfg->addr_phase.enable == false && phase_cfg->mode_phase.enable == false && \
    phase_cfg->wait_phase.enable == false)
    {
        reg_val0 &= ~((0x1 << 12) | (0x3 << 2));
    }

    if(phase_cfg->data_phase.enable == true)
    {
        reg_val0 &= ~((0x1 << 13) | (0x3 << 4));
        reg_val1 &= ~(0x3ffffff << 6);
        reg_val0 |= (phase_cfg->data_phase.spl_mode << 13) | (phase_cfg->data_phase.enable << 10) | \
        (phase_cfg->data_phase.bus_wire << 4);

        //并联flash寄存器data cnt配置为总发送数据量的一半
        if(REG32(SFC_WORK_MODE_REG)&(0x1<<6))
            reg_val1 |= (((phase_cfg->data_phase.cnt/2) - 1) << 6);
        else
            reg_val1 |= ((phase_cfg->data_phase.cnt - 1) << 6);
    }
    else if(phase_cfg->data_phase.enable == false)
    {
        reg_val0 &= ~((0x1 << 13) | (0x1 << 10) | (0x3 << 4));
        reg_val1 &= ~(0x3ffffff << 6);
    }

    REG32(SFC_INTERFACE_CFG_REG(cmd_bank_num)) = reg_val0;
    REG32(SFC_PHASE_LENGTH_CFG_REG(cmd_bank_num)) = reg_val1;
//    printf("interface(num:%d) = %x\n",cmd_bank_num, REG32(SFC_INTERFACE_CFG_REG(cmd_bank_num)));
//    printf("length(num:%d) = %x\n",cmd_bank_num, REG32(SFC_PHASE_LENGTH_CFG_REG(cmd_bank_num)));
}

#if 0
static unsigned int data_buf[16] = {0};
/**
 * @brief  data_buf_initi
 * @param  **p
 * @return
 * @note   buffer指针初始化
 */
static void data_buf_initi(unsigned char **p)
{
	unsigned int i;

	for(i = 0; i < sizeof(data_buf)/4; i++)
	{
		data_buf[i] = 0;
		p[i] = (unsigned char*)&data_buf[i];
	}
}
#endif

void dump_cmdbank(unsigned int base, unsigned int cmdnum)
{
	int i;
	unsigned int inst, addr, mode, data, match_mask, match_data, interface, length, interval;

	for(i = 0; i < cmdnum; i++) {
		inst = REG32(SFC_INST_PHASE_REG(i));
		addr = REG32(SFC_ADDR_PHASE_REG(i));
		mode = REG32(SFC_MODE_PHASE_REG(i));
		data = REG32(SFC_DATA_PHASE_REG(i));

		match_mask = REG32(SFC_POLLING_MASK_REG(i));
		match_data = REG32(SFC_POLLING_DATA_REG(i));
        interface  = REG32(SFC_INTERFACE_CFG_REG(i));
		length     = REG32(SFC_PHASE_LENGTH_CFG_REG(i));
		interval   = REG32(SFC_INTERVAL_CFG_REG(i));

		printf("cmdbank[%d] - inst:0x%x\n", i, inst);
		printf("cmdbank[%d] - addr:0x%x\n", i, addr);
		printf("cmdbank[%d] - mode:0x%x\n", i, mode);
		printf("cmdbank[%d] - data:0x%x\n", i, data);

		printf("cmdbank[%d] - match_mask:0x%x\n", i, match_mask);
		printf("cmdbank[%d] - match_data:0x%x\n", i, match_data);
		printf("cmdbank[%d] - interface:0x%x\n", i, interface);
		printf("cmdbank[%d] - length:0x%x\n", i, length);
		printf("cmdbank[%d] - interval:0x%x\n", i, interval);
		printf("---------------------------------------\r\n");
	}
}

/**
 * @brief  spi write data through the register
 * @param  *trans_param
 * @param  **p_buf
 * @param  cmd_bank_sum
 * @return
 * @note   CPU模式传输数据处理函数
 */
void sfc_spi_write_read_cpu(unsigned char **p_buf, unsigned int cmd_bank_sum)
{
    unsigned int i, j, l;
    unsigned int reg_val = 0;
    unsigned int thres_cnt;
    unsigned int frac_thres_cnt;
    unsigned int tmp = false;
    unsigned int rx_thres;
    unsigned int tx_thres;

    sfc_fifo_clear();
    sfc_get_fifo_thres(&rx_thres, &tx_thres);
//    sfc_intr_enable(SFC_TRANS_DONE_STATUS);
   // dump_cmdbank(1, cmd_bank_sum+1); //tiaoshi
    for(i = 0; i <= cmd_bank_sum; i++)
    {
        //不使用FIFO的data phase需要先放进寄存器
        if(sfc_trans_attribute(true, WRITE_FLASH, i)){
            // printf("tiaoshi $$$$$$$$$ fun:%s line:%d i = %d\n", __func__, __LINE__, i);
            REG32(SFC_DATA_PHASE_REG(i)) = (p_buf[i][0] | p_buf[i][1]<<8 | p_buf[i][2]<<16 | p_buf[i][3]<<24);
        }
    }
    sfc_trans_start(CPU_MODE, cmd_bank_sum);

    for(i = 0; i <= cmd_bank_sum; i++)
    {
        if(sfc_trans_attribute(false, WRITE_FLASH, i))
        {
            thres_cnt = sfc_get_trans_data_cnt(i) / ((SFC_TXFIFO_SIZE - tx_thres) * 4);
            frac_thres_cnt = sfc_get_trans_data_cnt(i) % ((SFC_TXFIFO_SIZE - tx_thres) * 4);
            // printf("tiaoshi ########## fun:%s line:%d i = %d thres_cnt = %d frac_thres_cnt = %d\n", __func__, __LINE__, i, thres_cnt, frac_thres_cnt);
            for(j = 0; j < thres_cnt; j++)
            {
                while(!(REG32(SFC_STATUS_REG) & SFC_TXFIFO_THRES_STATUS));    //等待FIFO阈值
                for(l = 0; l < SFC_TXFIFO_SIZE - tx_thres; l++)
                {
                    REG32(SFC_DATA_FIFO_REG) = (p_buf[i][0] | p_buf[i][1]<<8 | p_buf[i][2]<<16 | p_buf[i][3]<<24);
                    p_buf[i] += 4;
                }
            }

            if(frac_thres_cnt)
            {
                while(!(REG32(SFC_STATUS_REG) & SFC_TXFIFO_THRES_STATUS));    //等待FIFO阈值
                for(l = 0; l < frac_thres_cnt / 4; l++)
                {
                    REG32(SFC_DATA_FIFO_REG) = (p_buf[i][0] | p_buf[i][1]<<8 | p_buf[i][2]<<16 | p_buf[i][3]<<24);
                    p_buf[i] += 4;
                }
                for(j = 0; j < frac_thres_cnt % 4; j++)
                {
                    reg_val |= (*(p_buf[i]) << (j * 8));
                    p_buf[i]++;
                }
                if(frac_thres_cnt % 4 != 0)
                    REG32(SFC_DATA_FIFO_REG) = reg_val;
            }
        }
        else if(sfc_trans_attribute(false, READ_FLASH, i))
        {
            thres_cnt = sfc_get_trans_data_cnt(i) / (rx_thres * 4);
            frac_thres_cnt = sfc_get_trans_data_cnt(i) % (rx_thres * 4);
            for(j = 0; j < thres_cnt; j++)
            {
                while(!(REG32(SFC_STATUS_REG) & SFC_RXFIFO_THRES_STATUS));    //等待FIFO阈值
                for(l = 0; l < rx_thres; l++)
                {
                    reg_val = REG32(SFC_DATA_FIFO_REG);
                    memcpy(p_buf[i], &reg_val, sizeof(reg_val));
                    p_buf[i] += 4;
                }
            }

            if(frac_thres_cnt)
            {
                tmp = true;
                sfc_intr_check(SFC_TRANS_DONE_STATUS, WAIT_INTR);
                for(l = 0; l < frac_thres_cnt / 4; l++)
                {
                    reg_val = REG32(SFC_DATA_FIFO_REG);
                    memcpy(p_buf[i], &reg_val, sizeof(reg_val));
                    p_buf[i] += 4;
                }

                reg_val = REG32(SFC_DATA_FIFO_REG);
                for(j = 0; j < frac_thres_cnt % 4; j++)
                {
                    (*(p_buf[i]) = reg_val >> (j * 8));
                    p_buf[i]++;
                }
            }
        }
    }

    //等待SCMT传输结束再读取数据
    if(tmp == false)
        sfc_intr_check(SFC_TRANS_DONE_STATUS, WAIT_INTR);

    for(i = 0; i <= cmd_bank_sum; i++)
    {
        if(sfc_trans_attribute(true, READ_FLASH, i))
        {
            reg_val = REG32(SFC_DATA_PHASE_REG(i));
            memcpy(p_buf[i], &reg_val, sizeof(reg_val));
        }
    }
//    sfc_intr_disable(SFC_TRANS_DONE_STATUS);
}

/**
 * @brief  spi write data through the dma
 * @param  *trans_param
 * @param  *p_src_buf
 * @param  cmd_bank_sum
 * @return
 * @note   BDMA模式传输数据处理函数
 */
void sfc_spi_write_read_bdma(unsigned char **p_buf, unsigned int cmd_bank_sum)
{
    unsigned int i;
    unsigned int reg_val;
    // printf("tiaoshi fun:%s line:%d\n", __func__, __LINE__);
    sfc_fifo_clear();
    sfc_intr_enable(SFC_TRANS_DONE_STATUS);
    // printf("tiaoshi fun:%s line:%d\n", __func__, __LINE__);
    for(i = 0; i <= cmd_bank_sum; i++)
    {
        //不使用FIFO的data phase需要先放进寄存器
        if(sfc_trans_attribute(true, WRITE_FLASH, i))
            REG32(SFC_DATA_PHASE_REG(i)) = (p_buf[i][0] | p_buf[i][1] << 8 | p_buf[i][2] << 16 | p_buf[i][3] << 24);

        if(sfc_trans_attribute(false, WRITE_FLASH, i))
        {
            REG32(SFC_TRANSFER_ENABLE_CFG_REG) |= (0x1 << 29);  //dma下载方向
            REG32(SFC_DMA_START_ADDR_REG) = (unsigned long) (p_buf[i]+0x40000000);  //需配置为DDR实际地址
            REG32(SFC_DMA_LENGTH_REG) = sfc_get_trans_data_cnt(i);
        }
        else if(sfc_trans_attribute(false, READ_FLASH, i))
        {
            REG32(SFC_TRANSFER_ENABLE_CFG_REG) &= ~(0x1 << 29);  //dma上传方向
            REG32(SFC_DMA_START_ADDR_REG) = (unsigned long) (p_buf[i]+0x40000000);  //需配置为DDR实际地址
            REG32(SFC_DMA_LENGTH_REG) = sfc_get_trans_data_cnt(i);
        }
    }
    // printf("tiaoshi fun:%s line:%d\n", __func__, __LINE__);
    sfc_trans_start(BDMA_MODE, cmd_bank_sum);    //spi启动传输
    for(i = 0; i <= cmd_bank_sum; i++)
    {
        if(sfc_trans_attribute(false, READ_FLASH, i) || sfc_trans_attribute(false, WRITE_FLASH, i))
        {
            while((REG32(SFC_TRANSFER_ENABLE_CFG_REG) & (0x1 << 30))); //等待传输完成
        }
    }
    // printf("tiaoshi fun:%s line:%d\n", __func__, __LINE__);
    sfc_intr_check(SFC_TRANS_DONE_STATUS, WAIT_INTR);
    for(i = 0; i <= cmd_bank_sum; i++)
    {
        if(sfc_trans_attribute(true, READ_FLASH, i))
        {
            reg_val = REG32(SFC_DATA_PHASE_REG(i));
            memcpy(p_buf[i], &reg_val, sizeof(reg_val));
        }
    }
    // printf("tiaoshi fun:%s line:%d\n", __func__, __LINE__);
    sfc_intr_disable(SFC_TRANS_DONE_STATUS);
    // printf("tiaoshi fun:%s line:%d\n", __func__, __LINE__);
}

/**
 * @brief  spi write data through the dma
 * @param  *trans_param
 * @param  *p_src_buf
 * @param  cmd_bank_sum
 * @return
 * @note   EDMA模式传输数据处理函数
 */
void sfc_spi_write_read_edma(unsigned char **p_buf, unsigned int cmd_bank_sum)
{
    unsigned int i;
    unsigned int reg_val;

    sfc_fifo_clear();
    // sfc_intr_enable(SFC_TRANS_DONE_STATUS);

    for(i = 0; i <= cmd_bank_sum; i++)
    {
        //不使用FIFO的data phase需要先放进寄存器
        if(sfc_trans_attribute(true, WRITE_FLASH, i))
            REG32(SFC_DATA_PHASE_REG(i)) = (p_buf[i][0] | p_buf[i][1]<<8 | p_buf[i][2]<<16 | p_buf[i][3]<<24);

        if(sfc_trans_attribute(false, WRITE_FLASH, i))
        {
            if((unsigned long)p_buf[i]%4 != 0)
            {
                printf("buffer addr = %p\n",p_buf[i]);
                printf("data buffer address not 4byte align\n");
            }
            REG32(SFC_TRANSFER_ENABLE_CFG_REG) |= (0x1 << 29);  //dma下载方向
            REG32(SFC_DMA_LENGTH_REG) = ((sfc_get_trans_data_cnt(i) + 3) >> 2) << 2;    //data count需为4的倍数
            #if !CONFIG_IS_ENABLED(SYS_DCACHE_OFF)
            flush_dcache_range((unsigned long)p_buf[i],
                    (unsigned long)(p_buf[i] + (((sfc_get_trans_data_cnt(i) + 3) >> 2) << 2)));
            #endif
            l2_dma_buf1(p_buf[i], ((sfc_get_trans_data_cnt(i) + 3) >> 2) << 2, WRITE_L2);
            // l2_dma_bufx(p_buf[i], ((sfc_get_trans_data_cnt(i) + 3) >> 2) << 2, WRITE_L2, 1);
        }
        else if(sfc_trans_attribute(false, READ_FLASH, i))
        {
            if((unsigned long)p_buf[i]%4 != 0)
            {
                printf("buffer addr = %p\n",p_buf[i]);
                printf("data buffer address not 4byte align\n");
            }
            REG32(SFC_TRANSFER_ENABLE_CFG_REG) &= ~(0x1 << 29);  //dma上传方向
            REG32(SFC_DMA_LENGTH_REG) = ((sfc_get_trans_data_cnt(i) + 3) >> 2) << 2;    //data count需为4的倍数
            #if !CONFIG_IS_ENABLED(SYS_DCACHE_OFF)
            invalidate_dcache_range((unsigned long)p_buf[i],
                    (unsigned long)(p_buf[i] + (((sfc_get_trans_data_cnt(i) + 3) >> 2) << 2)));
            #endif
            l2_dma_buf1(p_buf[i], ((sfc_get_trans_data_cnt(i) + 3) >> 2) << 2, READ_L2);
            // l2_dma_bufx(p_buf[i], ((sfc_get_trans_data_cnt(i) + 3) >> 2) << 2, READ_L2, 2);
        }
    }
//    timer_start_new();
    sfc_trans_start(EDMA_MODE, cmd_bank_sum);    //spi启动传输
    /*for(i = 0; i <= cmd_bank_sum; i++)
    {
        if(sfc_trans_attribute(false, WRITE_FLASH, i))
        {
        }
        else if(sfc_trans_attribute(false, READ_FLASH, i))
        {
        }
    }*/

    sfc_intr_check(SFC_TRANS_DONE_STATUS, WAIT_INTR);
//    printf("flash transmission cost time:%dus\r\n",timer_finish_new());
    for(i = 0; i <= cmd_bank_sum; i++)
    {
        if(sfc_trans_attribute(true, READ_FLASH, i))
        {
            reg_val = REG32(SFC_DATA_PHASE_REG(i));
            memcpy(p_buf[i], &reg_val, sizeof(reg_val));
        }
    }
    //Disable th L2 Buffer
    *(volatile unsigned int *) (0x20140000 + 0x80) = 0x0;
    // sfc_intr_disable(SFC_TRANS_DONE_STATUS);
}

/**
 * @brief  sfc_spi_write_read_data
 * @param  *trans_param
 * @param  *p_src_buf
 * @param  data_path
 * @param  cmd_bank_sum
 * @return
 * @note   传输数据处理函数
 */
void sfc_spi_write_read_data(unsigned char **p_buf, T_SFC_TRANS_DATA_PATH data_path, unsigned int cmd_bank_sum)
{
    unsigned int i;

    for(i = 0; i <= cmd_bank_sum; i++)
    {
        if(sfc_trans_attribute(false, READ_FLASH, i) || sfc_trans_attribute(false, WRITE_FLASH, i))
            if(sfc_get_trans_data_cnt(i) > 0x3ffffff)
                printf("transfer data count too max");
    }

    if(data_path == CPU_MODE){
        sfc_spi_write_read_cpu(p_buf, cmd_bank_sum);
    }
    else if(data_path == BDMA_MODE){
        sfc_spi_write_read_bdma(p_buf, cmd_bank_sum);
    }
    else if(data_path == EDMA_MODE){
        sfc_spi_write_read_edma(p_buf, cmd_bank_sum);

    }

}
#if 0
/**
 * @brief  norflash_write_volatile_cfg_reg
 * @param  reg_addr
 * @param  reg_val
 * @return
 * @note   写flash易失性寄存器
 */
void norflash_write_volatile_cfg_reg(unsigned char reg_val, unsigned int reg_addr)
{
    unsigned int tmp = reg_val;
    unsigned char *p_data_phase[TRANS_NUM_MAX] = {NULL};
    T_SFC_SPI_PHASE_CFG phase_cfg;
    T_SFC_TRANS_PARAM trans_param;

    memset(&phase_cfg,0,sizeof(phase_cfg));

    trans_param.trans_gap = flash_param.trans_gap;    /*两次传输之间的间隔时间，单位PHY_CLK*/
    trans_param.crypto_en = false;    /*加密使能*/
    trans_param.rxds_en = flash_param.rxds_en;    /*RXDS使能*/
    trans_param.cs_sele = flash_param.cs_sele;    /*CS PIN选择*/
    trans_param.data_reg_nofifo = true;    /*数据收发是否使用FIFO*/
    trans_param.cmd_type = NO_POLLING_CMD;    /*命令类型，轮询命令或非轮询命令*/
    trans_param.trans_dir = WRITE_FLASH;    /*spi传输方向*/

    //SCMT第1次传输的配置
    norflash_cmd_analysis(&phase_cfg, FLASH_CMD_WREN, 0, 0);
    sfc_spi_trans_cfg(&trans_param, &phase_cfg, TRANS_NUM1);

    //SCMT第2次传输的配置
    norflash_cmd_analysis(&phase_cfg, FLASH_CMD_WRITE_VCR, reg_addr, 1);
    sfc_spi_trans_cfg(&trans_param, &phase_cfg, TRANS_NUM2);

    //SCMT第3次传输的配置
    if(reg_val == 0xff && reg_addr == 5)
        flash_param.addr_mode = 3;
    else if(reg_addr == 5)
        flash_param.addr_mode = 4;

    if(reg_val == 0xe7 && reg_addr == 0)
        flash_param.sample_mode = DTR_MODE;
    else if(reg_addr == 0)
        flash_param.sample_mode = STR_MODE;

    trans_param.cmd_type = POLLING_CMD;
    trans_param.trans_dir = READ_FLASH;
    sfc_cmd_polling_cfg(0,0xfffffffe,AND_MODE,TRANS_NUM3);
    norflash_cmd_analysis(&phase_cfg, FLASH_CMD_RDSR1, 0, 1);
    sfc_spi_trans_cfg(&trans_param, &phase_cfg, TRANS_NUM3);

    //SCMT启动传输
    p_data_phase[TRANS_NUM2] = (unsigned char*)&tmp;
    sfc_spi_write_read_data(p_data_phase, flash_param.data_path, TRANS_NUM3);
}
#endif
/**
 * @brief  norflash_write_status_cfg_reg
 * @param  reg_addr
 * @param  reg_val
 * @return
 * @note   写flash状态寄存器
 */
void norflash_write_status_cfg_reg(unsigned char* reg_val, unsigned int count, unsigned int cmd_code)
{
    // unsigned int tmp = reg_val;
    unsigned char *p_data_phase[TRANS_NUM_MAX] = {NULL};
    T_SFC_SPI_PHASE_CFG phase_cfg;
    T_SFC_TRANS_PARAM trans_param;

    memset(&phase_cfg,0,sizeof(phase_cfg));

    trans_param.trans_gap = flash_param.trans_gap;    /*两次传输之间的间隔时间，单位PHY_CLK*/
    trans_param.crypto_en = false;    /*加密使能*/
    trans_param.rxds_en = flash_param.rxds_en;    /*RXDS使能*/
    trans_param.cs_sele = flash_param.cs_sele;    /*CS PIN选择*/
    trans_param.data_reg_nofifo = true;    /*数据收发是否使用FIFO*/
    trans_param.cmd_type = NO_POLLING_CMD;    /*命令类型，轮询命令或非轮询命令*/
    trans_param.trans_dir = WRITE_FLASH;    /*spi传输方向*/

    norflash_cmd_analysis(&phase_cfg, cmd_code, 0, count);
    sfc_spi_trans_cfg(&trans_param, &phase_cfg, TRANS_NUM1);

    trans_param.cmd_type = POLLING_CMD;
    trans_param.trans_dir = READ_FLASH;
    norflash_cmd_analysis(&phase_cfg, FLASH_CMD_RDSR1, 0, 1);
    sfc_spi_trans_cfg(&trans_param, &phase_cfg, TRANS_NUM2);
    sfc_cmd_polling_cfg(0,0xfffffffe,AND_MODE,TRANS_NUM2);

    p_data_phase[TRANS_NUM1] = reg_val;
    sfc_spi_write_read_data(p_data_phase, flash_param.data_path, TRANS_NUM2);
}
#if 0
/**
 * @brief  spi flash stop continue read
 * @param  address
 * @param  des_buf
 * @param  count
 * @param  flash_cmd_code
 * @return
 * @note   flash停止continue read模式
 */
void norflash_stop_continue_read(unsigned char flash_cmd_code)
{
    unsigned char *p_data_phase[TRANS_NUM_MAX] = {NULL};
    unsigned int tmp;
    T_SFC_SPI_PHASE_CFG phase_cfg;
    T_SFC_TRANS_PARAM trans_param;

    memset(&phase_cfg,0,sizeof(phase_cfg));

    trans_param.trans_gap = flash_param.trans_gap;    /*两次传输之间的间隔时间，单位PHY_CLK*/
    trans_param.crypto_en = false;    /*加密使能*/
    trans_param.rxds_en = flash_param.rxds_en;    /*RXDS使能*/
    trans_param.cs_sele = flash_param.cs_sele;    /*CS PIN选择*/
    trans_param.data_reg_nofifo = true;    /*数据收发是否使用FIFO*/
    trans_param.cmd_type = NO_POLLING_CMD;    /*命令类型，轮询命令或非轮询命令*/
    trans_param.trans_dir = WRITE_FLASH;    /*spi传输方向*/

    //SCMT第1次传输的配置
    trans_param.trans_dir = READ_FLASH;
    norflash_cmd_analysis(&phase_cfg, flash_cmd_code, 0, 4);
    phase_cfg.inst_phase.enable = false;//continue read not inst phase
    phase_cfg.mode_phase.data = 0;//mode data = 0退出continue read模式
    sfc_spi_trans_cfg(&trans_param, &phase_cfg, TRANS_NUM1);

    //SCMT启动传输
    p_data_phase[TRANS_NUM1] = (unsigned char*)&tmp;
    sfc_spi_write_read_data(p_data_phase, flash_param.data_path, TRANS_NUM1);
}
#endif
/**
 * @brief  norflash_read_status_cfg_reg
 * @param
 * @return  status value
 * @note   读flash非易失性寄存器
 */
unsigned int norflash_read_status_cfg_reg(unsigned int count, unsigned int cmd_code)
{
    unsigned int reg_val;
    unsigned char *p_data_phase[TRANS_NUM_MAX] = {NULL};
    T_SFC_SPI_PHASE_CFG phase_cfg;
    T_SFC_TRANS_PARAM trans_param;

    memset(&phase_cfg,0,sizeof(phase_cfg));

    trans_param.trans_gap = flash_param.trans_gap;    /*两次传输之间的间隔时间，单位PHY_CLK*/
    trans_param.crypto_en = false;    /*加密使能*/
    trans_param.rxds_en = flash_param.rxds_en;    /*RXDS使能*/
    trans_param.cs_sele = flash_param.cs_sele;    /*CS PIN选择*/
    trans_param.data_reg_nofifo = true;    /*数据收发是否使用FIFO*/
    trans_param.cmd_type = NO_POLLING_CMD;    /*命令类型，轮询命令或非轮询命令*/
    trans_param.trans_dir = WRITE_FLASH;    /*spi传输方向*/

    trans_param.trans_dir = READ_FLASH;
    norflash_cmd_analysis(&phase_cfg, cmd_code, 0, count);
    sfc_spi_trans_cfg(&trans_param, &phase_cfg, TRANS_NUM1);

    p_data_phase[TRANS_NUM1] = (unsigned char*)&reg_val;
    sfc_spi_write_read_data(p_data_phase, flash_param.data_path, TRANS_NUM1);

    return reg_val;
}
#if 0
/**
 * @brief  spi flash get id
 * @param
 * @return
 * @note   获取flash id
 */
unsigned int norflash_getid(void)
{
    unsigned int flash_id;
    unsigned char *p_data_phase[TRANS_NUM_MAX] = {NULL};
    T_SFC_SPI_PHASE_CFG phase_cfg;
    T_SFC_TRANS_PARAM trans_param;

    memset(&phase_cfg,0,sizeof(phase_cfg));

    trans_param.trans_gap = flash_param.trans_gap;
    trans_param.crypto_en = false;
    trans_param.rxds_en = flash_param.rxds_en;
    trans_param.cs_sele = flash_param.cs_sele;
    trans_param.data_reg_nofifo = true;
    trans_param.cmd_type = NO_POLLING_CMD;
    trans_param.trans_dir = WRITE_FLASH;

    trans_param.trans_dir = READ_FLASH;
    norflash_cmd_analysis(&phase_cfg, FLASH_CMD_READ_ID, 0, 3);
    sfc_spi_trans_cfg(&trans_param, &phase_cfg, TRANS_NUM1);

    p_data_phase[TRANS_NUM1] = (unsigned char*)&flash_id;
    sfc_spi_write_read_data(p_data_phase, flash_param.data_path, TRANS_NUM1);

    return flash_id;
}
#endif
/**
 * @brief  spi flash sector erase
 * @param  address
 * @return
 * @note   flash扇擦除
 */
int norflash_sector_erase(unsigned int address)
{
    unsigned char *p_data_phase[TRANS_NUM_MAX] = {NULL};
    T_SFC_SPI_PHASE_CFG phase_cfg;
    T_SFC_TRANS_PARAM trans_param;

    memset(&phase_cfg,0,sizeof(phase_cfg));

    trans_param.trans_gap = flash_param.trans_gap;
    trans_param.crypto_en = false;
    trans_param.rxds_en = flash_param.rxds_en;
    trans_param.cs_sele = flash_param.cs_sele;
    trans_param.data_reg_nofifo = true;
    trans_param.cmd_type = NO_POLLING_CMD;
    trans_param.trans_dir = WRITE_FLASH;

    norflash_cmd_analysis(&phase_cfg, FLASH_CMD_SE, address, 0);
    sfc_spi_trans_cfg(&trans_param, &phase_cfg, TRANS_NUM1);

    trans_param.cmd_type = POLLING_CMD;
    trans_param.trans_dir = READ_FLASH;
    norflash_cmd_analysis(&phase_cfg, FLASH_CMD_RDSR1, 0, 1);
    sfc_spi_trans_cfg(&trans_param, &phase_cfg, TRANS_NUM2);
    sfc_cmd_polling_cfg(0,0xfffffffe,AND_MODE,TRANS_NUM2);

    sfc_spi_write_read_data(p_data_phase, flash_param.data_path, TRANS_NUM2);

    return 0;
}
#if 0
/**
 * @brief  spi flash block erase
 * @param  address
 * @return
 * @note   flash块擦除
 */
void norflash_block_erase(unsigned int address)
{
    unsigned char *p_data_phase[TRANS_NUM_MAX] = {NULL};
    T_SFC_SPI_PHASE_CFG phase_cfg;
    T_SFC_TRANS_PARAM trans_param;
    // printf("tiaoshi norflash_block_erase address = %d\n", address);
    // encryption_flag = false;
    memset(&phase_cfg,0,sizeof(phase_cfg));

    trans_param.trans_gap = flash_param.trans_gap;
    trans_param.crypto_en = false;
    trans_param.rxds_en = flash_param.rxds_en;
    trans_param.cs_sele = flash_param.cs_sele;
    trans_param.data_reg_nofifo = true;
    trans_param.cmd_type = NO_POLLING_CMD;
    trans_param.trans_dir = WRITE_FLASH;

    norflash_cmd_analysis(&phase_cfg, FLASH_CMD_WREN, 0, 0);
    sfc_spi_trans_cfg(&trans_param, &phase_cfg, TRANS_NUM1);

    norflash_cmd_analysis(&phase_cfg, FLASH_CMD_64K_BE, address, 0);
    sfc_spi_trans_cfg(&trans_param, &phase_cfg, TRANS_NUM2);

    trans_param.cmd_type = POLLING_CMD;
    sfc_cmd_polling_cfg(0,0xfffffffe,AND_MODE,TRANS_NUM3);
    trans_param.trans_dir = READ_FLASH;
    norflash_cmd_analysis(&phase_cfg, FLASH_CMD_RDSR1, 0, 1);
    sfc_spi_trans_cfg(&trans_param, &phase_cfg, TRANS_NUM3);

    sfc_spi_write_read_data(p_data_phase, flash_param.data_path, TRANS_NUM3);
}
#endif
/**
 * @brief  nor flash Page Program
 * @param  unsigned int address
 * @param  unsigned char *src_buf
 * @param  unsigned int count
 * @return void:
 * @note:  flash一线页编程
 */
void norflash_page_program(unsigned int address, unsigned char *src_buf, unsigned int count)
{
    unsigned char *p_data_phase[TRANS_NUM_MAX] = {NULL};
    T_SFC_SPI_PHASE_CFG phase_cfg;
    T_SFC_TRANS_PARAM trans_param;

    memset(&phase_cfg,0,sizeof(phase_cfg));

    trans_param.trans_gap = flash_param.trans_gap;
    trans_param.crypto_en = false;
    trans_param.rxds_en = flash_param.rxds_en;
    trans_param.cs_sele = flash_param.cs_sele;
    trans_param.data_reg_nofifo = true;
    trans_param.cmd_type = NO_POLLING_CMD;
    trans_param.trans_dir = WRITE_FLASH;

    trans_param.crypto_en = flash_param.crypto_en;
    trans_param.data_reg_nofifo = false;

    norflash_cmd_analysis(&phase_cfg, FLASH_CMD_PP, address, count);
    sfc_spi_trans_cfg(&trans_param, &phase_cfg, TRANS_NUM1);

    trans_param.crypto_en = false;
    trans_param.cmd_type = POLLING_CMD;
    trans_param.trans_dir = READ_FLASH;
    trans_param.data_reg_nofifo = true;
    sfc_cmd_polling_cfg(0,0xfffffffe, AND_MODE, TRANS_NUM2);
    norflash_cmd_analysis(&phase_cfg, FLASH_CMD_RDSR1, 0, 1);
    sfc_spi_trans_cfg(&trans_param, &phase_cfg, TRANS_NUM2);

    p_data_phase[TRANS_NUM1] = src_buf;
    sfc_spi_write_read_data(p_data_phase, flash_param.data_path, TRANS_NUM2);
}

/**
 * @brief  spi flash quad page program
 * @param  src_buf
 * @param  count
 * @param  address
 * @return
 * @note   flash四线页编程
 */
void norflash_quad_page_program(unsigned int address, unsigned char *src_buf, unsigned int count)
{
    unsigned char *p_data_phase[TRANS_NUM_MAX] = {NULL};
    T_SFC_SPI_PHASE_CFG phase_cfg;
    T_SFC_TRANS_PARAM trans_param;

    memset(&phase_cfg,0,sizeof(phase_cfg));

    trans_param.trans_gap = flash_param.trans_gap;
    trans_param.crypto_en = false;
    trans_param.rxds_en = flash_param.rxds_en;
    trans_param.cs_sele = flash_param.cs_sele;
    trans_param.data_reg_nofifo = true;
    trans_param.cmd_type = NO_POLLING_CMD;
    trans_param.trans_dir = WRITE_FLASH;

    trans_param.crypto_en = flash_param.crypto_en;
    trans_param.data_reg_nofifo = false;

    norflash_cmd_analysis(&phase_cfg, FLASH_CMD_QUAD_PP, address, count);
    sfc_spi_trans_cfg(&trans_param, &phase_cfg, TRANS_NUM1);

    trans_param.crypto_en = false;
    trans_param.cmd_type = POLLING_CMD;
    trans_param.trans_dir = READ_FLASH;
    trans_param.data_reg_nofifo = true;
    sfc_cmd_polling_cfg(0,0xfffffffe, AND_MODE, TRANS_NUM2);
    norflash_cmd_analysis(&phase_cfg, FLASH_CMD_RDSR1, 0, 1);
    sfc_spi_trans_cfg(&trans_param, &phase_cfg, TRANS_NUM2);

    p_data_phase[TRANS_NUM1] = src_buf;
    sfc_spi_write_read_data(p_data_phase, flash_param.data_path, TRANS_NUM2);
}

/**
 * @brief  spi flash octla page program
 * @param  src_buf
 * @param  count
 * @param  address
 * @return
 * @note   flash八线页编程
 */
void norflash_octla_page_program(unsigned int address, unsigned char *src_buf, unsigned int count)
{
    unsigned char *p_data_phase[TRANS_NUM_MAX] = {NULL};
    T_SFC_SPI_PHASE_CFG phase_cfg;
    T_SFC_TRANS_PARAM trans_param;

    memset(&phase_cfg,0,sizeof(phase_cfg));

    trans_param.trans_gap = flash_param.trans_gap;
    trans_param.crypto_en = false;
    trans_param.rxds_en = flash_param.rxds_en;
    trans_param.cs_sele = flash_param.cs_sele;
    trans_param.data_reg_nofifo = true;
    trans_param.cmd_type = NO_POLLING_CMD;
    trans_param.trans_dir = WRITE_FLASH;

    trans_param.crypto_en = flash_param.crypto_en;
    trans_param.data_reg_nofifo = false;

    norflash_cmd_analysis(&phase_cfg, FLASH_CMD_OCTAL_PP, address, count);
    sfc_spi_trans_cfg(&trans_param, &phase_cfg, TRANS_NUM1);

    p_data_phase[TRANS_NUM1] = src_buf;
    sfc_spi_write_read_data(p_data_phase, flash_param.data_path, TRANS_NUM1);
}

/**
 * @brief  nor flash Read Data Bytes
 * @param  unsigned int address
 * @param  unsigned char *des_buf
 * @param  unsigned int count
 * @param  T_SPIFLASH_CMD cmd_data
 * @return void:
 * @note:  flash读数据
 */
void _norflash_read_data(unsigned int address , unsigned char *des_buf, unsigned int count, unsigned char flash_cmd_code)
{
    unsigned char *p_data_phase[TRANS_NUM_MAX] = {NULL};
    T_SFC_SPI_PHASE_CFG phase_cfg;
    T_SFC_TRANS_PARAM trans_param;

    //printf("%s flash_cmd_code 0x%x address 0x%x count %d\n", __func__, flash_cmd_code, address, count);
    // memset(des_buf,0,count);
    memset(&phase_cfg,0,sizeof(phase_cfg));

    trans_param.trans_gap = flash_param.trans_gap;
    trans_param.crypto_en = false;
    trans_param.rxds_en = flash_param.rxds_en;
    trans_param.cs_sele = flash_param.cs_sele;
    trans_param.data_reg_nofifo = true;
    trans_param.cmd_type = NO_POLLING_CMD;
    trans_param.trans_dir = WRITE_FLASH;


    trans_param.data_reg_nofifo = false;
    trans_param.trans_dir = READ_FLASH;
    norflash_cmd_analysis(&phase_cfg, flash_cmd_code, address, count);
    sfc_spi_trans_cfg(&trans_param, &phase_cfg, TRANS_NUM1);

    p_data_phase[TRANS_NUM1] = des_buf;
    sfc_spi_write_read_data(p_data_phase, flash_param.data_path, TRANS_NUM1);
}

#define L2_DMA_MAX_SIZE (1*1024*1024)

void norflash_read_data(unsigned int address , unsigned char *des_buf, unsigned int count, unsigned char flash_cmd_code)
{
    //int i;
    unsigned int offset = 0, count1; // count_old = count;
    T_SFC_TRANS_DATA_PATH data_path1;

    if (flash_param.data_path == CPU_MODE) {
        _norflash_read_data(address+offset, des_buf+offset, count, flash_cmd_code);
    } else if (flash_param.data_path == EDMA_MODE) {
        while (count >= L2_DMA_MAX_SIZE) {
            _norflash_read_data(address+offset, des_buf+offset, L2_DMA_MAX_SIZE, flash_cmd_code);
            offset += L2_DMA_MAX_SIZE;
            count -= L2_DMA_MAX_SIZE;
        }
        if (count >= 4096) {
            count1 = count - count % 4096;
            _norflash_read_data(address+offset, des_buf+offset, count1, flash_cmd_code);
            offset += count1;
            count -= count1;
        }
        if (count >= 64) {
            count1 = count - count % 64;
            _norflash_read_data(address+offset, des_buf+offset, count1, flash_cmd_code);
            offset += count1;
            count -= count1;
        }
        if (count) {
            data_path1 = flash_param.data_path;
            flash_param.data_path = CPU_MODE;
            _norflash_read_data(address+offset, des_buf+offset, count, flash_cmd_code);
            flash_param.data_path = data_path1;
        }
    }
}

/**
 * @brief  spi flash program data
 * @param  count
 * @param  address
 * @param  *src_buf
 * @param  wire_mode
 * @return
 * @note   flash编程数据
 */
void norflash_program_data(unsigned int address, unsigned char *src_buf, unsigned int count, unsigned int wire_mode)
{
    T_SFC_TRANS_DATA_PATH data_path;
    unsigned int page_addr = address%PAGE_SIZE;
    unsigned int tran_cnt;
    unsigned int i = 0;
    unsigned int count1;
    unsigned int frac_count;

    // printf("tiaoshi norflash_program_data address = %d count:%d wire_mode:%d\n", address, count, wire_mode);
    if(page_addr + count <= PAGE_SIZE)
    {
        count1 = count - count % 64;
        frac_count = count % 64;
        if(wire_mode == WIRE8)
            norflash_octla_page_program(address, src_buf, count1);
        else if(wire_mode == WIRE4)
            norflash_quad_page_program(address, src_buf, count1);
        else if(wire_mode == WIRE1)
            norflash_page_program(address, src_buf, count1);

        if (frac_count) {
            data_path = flash_param.data_path;
            flash_param.data_path = CPU_MODE;
            if(wire_mode == WIRE8)
                norflash_octla_page_program(address, src_buf, frac_count);
            else if(wire_mode == WIRE4)
                norflash_quad_page_program(address, src_buf, frac_count);
            else if(wire_mode == WIRE1)
                norflash_page_program(address, src_buf, frac_count);
            flash_param.data_path = data_path;
        }
    }
    else
    {
        do
        {
            if(i == 0)
            {
                tran_cnt = PAGE_SIZE-page_addr;
                count1 = tran_cnt - tran_cnt % 64;
                frac_count = tran_cnt % 64;
                // printf("flash addr = 0x%x,buf addr = 0x%x, cnt = %d\n",address + i, src_buf + i, tran_cnt);
                if(wire_mode == WIRE8)
                    norflash_octla_page_program(address + i, src_buf + i, count1);
                else if(wire_mode == WIRE4)
                    norflash_quad_page_program(address + i, src_buf + i, count1);
                else if(wire_mode == WIRE1)
                    norflash_page_program(address + i, src_buf + i, count1);
                i += count1;
                if (frac_count) {
                    data_path = flash_param.data_path;
                    flash_param.data_path = CPU_MODE;
                    if(wire_mode == WIRE8)
                        norflash_octla_page_program(address + i, src_buf + i, frac_count);
                    else if(wire_mode == WIRE4)
                        norflash_quad_page_program(address + i, src_buf + i, frac_count);
                    else if(wire_mode == WIRE1)
                        norflash_page_program(address + i, src_buf + i, frac_count);
                    flash_param.data_path = data_path;
                    i += frac_count;
                }
            }

            tran_cnt = count - i;
            if(tran_cnt > PAGE_SIZE)
                tran_cnt = PAGE_SIZE;
            // printf("flash addr = 0x%x,buf addr = 0x%x, cnt = %d\n",address + i, src_buf + i, tran_cnt);
            count1 = tran_cnt - tran_cnt % 64;
            frac_count = tran_cnt % 64;
            if(wire_mode == WIRE8)
                norflash_octla_page_program(address + i, src_buf + i, count1);
            else if(wire_mode == WIRE4)
                norflash_quad_page_program(address + i, src_buf + i, count1);
            else if(wire_mode == WIRE1)
                norflash_page_program(address + i, src_buf + i, count1);
            i += count1;
            if (frac_count) {
                data_path = flash_param.data_path;
                flash_param.data_path = CPU_MODE;
                if(wire_mode == WIRE8)
                    norflash_octla_page_program(address + i, src_buf + i, frac_count);
                else if(wire_mode == WIRE4)
                    norflash_quad_page_program(address + i, src_buf + i, frac_count);
                else if(wire_mode == WIRE1)
                    norflash_page_program(address + i, src_buf + i, frac_count);
                flash_param.data_path = data_path;
                i += frac_count;
            }
        }while(i < count);
    }
}

void sfcv2_set_parm(unsigned int max_hz)
{
    //tiaoshi begin
    //int ii = 0;
    sfc_pinmux_config();
    sfc_clk_rst_config();
    /*
    printf("TOP_REG_CLK_CTRL0 = %x\n",REG32(0x08000004));
    printf("TOP_REG_CLK_CTRL1 = %x\n",REG32(0x08000008));
    printf("TOP_REG_CLK_CTRL2 = %x\n",REG32(0x0800000c));
    printf("TOP_REG_CLK_CTRL7 = %x\n",REG32(0x08000020));
    */
    T_FLASH_PARAM flash_trans_param;
    T_SFC_INITI_PARAM initi_param;
    flash_trans_param.sample_mode = STR_MODE;
    //flash_trans_param.data_path = CPU_MODE;
    //flash_trans_param.data_path = BDMA_MODE;
    flash_trans_param.data_path = EDMA_MODE;
    flash_trans_param.cs_sele = SELE_CS0;
    flash_trans_param.bus_wire = WIRE4;
    // flash_trans_param.bus_wire = WIRE1;
    flash_trans_param.crypto_en = false;
    flash_trans_param.addr_mode = 3;
    initi_param.trans_format = CLK_MODE0;
#if defined(CONFIG_3918AV130_CODE)
    //H322LS SFC 100M存在读数据异常, 临时改为66M
    if (max_hz < 20000000)
        max_hz = 20000000;
    if (max_hz > 100000000)
        max_hz = 100000000;

    initi_param.clk_div = 200000000 / max_hz;
#else
    initi_param.clk_div = 2;
#endif
    // printf("max_hz = %d, sclk div = %d,sclk = %dHz\n", max_hz, initi_param.clk_div, (200000000 / (initi_param.clk_div)));
    initi_param.work_mode = SFC_SWC_MODE;
    initi_param.flash_parallel = false;
    initi_param.hyper_bus = false;
    initi_param.rxfifo_thres = 5;
    initi_param.txfifo_thres = 4;
    flash_trans_param.trans_gap = 0;
    flash_trans_param.rxds_en = false;
    sfc_initi(&initi_param);
    norflash_trans_param(&flash_trans_param);
    //printf("test 8 norflash id = 0x%x\n", norflash_getid());
}
#endif
