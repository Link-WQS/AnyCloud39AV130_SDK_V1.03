#ifndef _SFCV2_H__
#define _SFCV2_H__

#if 1
/******************************* tiaoshi svt start *****************************************/
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

#define SFC_XIP_MODE_ERROR_STATUS         (0x1<<0)
#define SFC_XIP_ADDR_ERROR_STATUS         (0x1<<1)
#define SFC_TXFIFO_THRES_STATUS           (0x1<<2)
#define SFC_RXFIFO_THRES_STATUS           (0x1<<3)
#define SFC_TRANS_DONE_STATUS             (0x1<<4)
#define SFC_TRANS_IDLE_STATUS             (0x1<<5)
#define SFC_MDLU_INIT_DONE_STATUS         (0x1<<6)
#define SFC_MDLU_LOCK_ERROR_STATUS        (0x1<<7)
#define SFC_MDLU_LOCK_MODE_STATUS         (0x1<<8)
#define SFC_TRANS_ABORT_STATUS            (0x1<<9)

#define SFC_XIP_BAR0_ADDR                 (0x30000000)
#define SFC_XIP_BAR0_SIZE                 (8*1024*1024 - 256)
// #define SFC_XIP_BAR0_SIZE                 (8*1024*1024)
#define SFC_XIP_BAR0_FLASH_ADDR           (0)
#define SFC_XIP_BAR1_ADDR                 (0x30800000)
#define SFC_XIP_BAR1_SIZE                 (8*1024*1024)
#define SFC_XIP_BAR1_FLASH_ADDR           (8*1024*1024)
#define SFC_TXFIFO_SIZE					  (8)
#define SFC_RXFIFO_SIZE					  (8)
#define SFC_RAM_FIFO_SIZE                 (512)         // FIFO 大小
#define SFC_RAM_RX_FIFO_ADDR              (SHARE_RAM_BASE_ADDR+128)   // FIFO 地址
#define SFC_RAM_TX_FIFO_ADDR              (SHARE_RAM_BASE_ADDR+128+SFC_RAM_FIFO_SIZE)   // FIFO 地址

#define ERROR_DELAY                       (0x7ffffff)
#define ERROR(pause,string) \
    do {                  \
        printf_v2("Code location:%s: %s(%d)\n",__FILE__,__func__,__LINE__); \
        printf_v2("%s\n",string);                              \
        if(pause == AK_TRUE)\
            getch(); \
    }while(0)

typedef enum _SFC_TRANS_FORMAT
{
    CLK_MODE0,
    CLK_MODE3,
}T_SFC_TRANS_FORMAT;

typedef enum _SFC_SAMPLE_MODE
{
    STR_MODE,
    DTR_MODE,
}T_SFC_SAMPLE_MODE;

typedef enum _SFC_BUS_WIDTHS
{
    WIRE1,
    WIRE2,
    WIRE4,
    WIRE8,
}T_SFC_BUS_WIDTH;

typedef enum _SFC_WORK_MODE
{
    SFC_SWC_MODE,
    SFC_XIP_MODE,
}T_SFC_WORK_MODE;

typedef enum _SFC_INTR_WAIT
{
    WAIT_INTR,
    NO_WAIT_INTR,
}T_SFC_INTR_WAIT;

typedef enum _SFC_MATCH_MODE
{
    AND_MODE,
    OR_MODE,
}T_SFC_MATCH_MODE;

typedef enum _SFC_CMD_TYPE
{
    NO_POLLING_CMD,
    POLLING_CMD,
}T_SFC_CMD_TYPE;

typedef enum _SFC_CS_SELE
{
    SELE_CS0,
    SELE_CS1,
}T_SFC_CS_SELE;

typedef enum _SFC_TRANS_DIR
{
    READ_FLASH,
    WRITE_FLASH,
}T_SFC_TRANS_DIR;

typedef enum _SFC_TRANS_DATA_PATH
{
    CPU_MODE,
    BDMA_MODE,
    EDMA_MODE,
}T_SFC_TRANS_DATA_PATH;

typedef enum _SFC_TRANS_NUM
{
    TRANS_NUM1,
    TRANS_NUM2,
    TRANS_NUM3,
    TRANS_NUM4,
    TRANS_NUM5,
    TRANS_NUM6,
    TRANS_NUM7,
    TRANS_NUM8,
    TRANS_NUM9,
    TRANS_NUM10,
    TRANS_NUM11,
    TRANS_NUM12,
    TRANS_NUM13,
    TRANS_NUM14,
    TRANS_NUM15,
    TRANS_NUM_MAX = 4,
}T_SFC_TRANS_NUM;

typedef enum _SFC_MDLU_LOCK_MODE
{
    NON_STOP_LOCK,
    PERIOD_LOCK,
    SINGLE_LOCK,
}T_SFC_MDLU_LOCK_MODE;

typedef enum _SFC_ABORT_MODE
{
    TRANS_FINISH_ABORT,
    TRANS_PROCESS_ABORT,
}T_SFC_ABORT_MODE;

typedef struct _SFC_XIP_INITI_PARAM
{
    unsigned int system_addr;
    unsigned int flash_addr;
    unsigned int bar_size;
    T_SFC_CS_SELE cs_sele;
}T_SFC_XIP_INITI_PARAM;

typedef struct _SFC_INITI_PARAM
{
    unsigned int txfifo_thres;
    unsigned int rxfifo_thres;
    unsigned int clk_div;
    bool flash_parallel;
    bool hyper_bus;
    T_SFC_WORK_MODE work_mode;
    T_SFC_CS_SELE cs_sele;
    T_SFC_SAMPLE_MODE sample_mode;
    T_SFC_TRANS_FORMAT trans_format;
    T_SFC_TRANS_DATA_PATH data_path;
}T_SFC_INITI_PARAM;

typedef struct _SFC_SPI_PHASE_PARAM
{
    bool enable;
    unsigned int data;
    unsigned int cnt;
    T_SFC_BUS_WIDTH bus_wire;
    T_SFC_SAMPLE_MODE spl_mode;
}T_SFC_SPI_PHASE_PARAM;

typedef struct _SFC_SPI_PHASE_CFG
{
    T_SFC_SPI_PHASE_PARAM inst_phase;
    T_SFC_SPI_PHASE_PARAM addr_phase;
    T_SFC_SPI_PHASE_PARAM mode_phase;
    T_SFC_SPI_PHASE_PARAM wait_phase;
    T_SFC_SPI_PHASE_PARAM data_phase;
}T_SFC_SPI_PHASE_CFG;

typedef struct _SFC_TRANS_PARAM
{
    unsigned int trans_gap;//计时单位SFC_PHY_CLK
    bool crypto_en;
    bool rxds_en;
    bool data_reg_nofifo;
    T_SFC_CS_SELE cs_sele;
    T_SFC_CMD_TYPE cmd_type;
    T_SFC_TRANS_DIR trans_dir;
}T_SFC_TRANS_PARAM;

/*********************flash start*****************************/
#define FLASH_CMD_RESET_EN                     (0x66)
#define FLASH_CMD_RESET                        (0x99)
#define FLASH_CMD_WREN                         (0x06)
#define FLASH_CMD_WRDI                         (0x04)
#define FLASH_CMD_WREN_VSR                     (0x50)
#define FLASH_CMD_READ_SFDP                    (0x5A)

#define FLASH_CMD_READ_ID                      (0x9f)
#define FLASH_CMD_RDSR1                        (0x05)
#define FLASH_CMD_RDSR2                        (0x35)
#define FLASH_CMD_RDSR3                        (0x15)
#define FLASH_CMD_READ_FLAG_SR                 (0x70)
#define FLASH_CMD_READ_NVCR                    (0xB5)
#define FLASH_CMD_READ_VCR                     (0x85)

#define FLASH_CMD_WRSR1                        (0x01)
#define FLASH_CMD_WRSR2                        (0x31)
#define FLASH_CMD_WRSR3                        (0x11)
#define FLASH_CMD_WRITE_NVCR                   (0xB1)
#define FLASH_CMD_WRITE_VCR                    (0x81)

#define FLASH_CMD_READ                         (0x03)
#define FLASH_CMD_FAST_READ                    (0x0B)
#define FLASH_CMD_DTR_FAST_READ                (0x0D)
#define FLASH_CMD_DUAL_FAST_READ               (0x3B)
#define FLASH_CMD_DUAL_IO_FAST_READ            (0xBB)
#define FLASH_CMD_DUAL_IO_DTR_FAST_READ        (0xBD)
#define FLASH_CMD_QUAD_FAST_READ               (0x6B)
#define FLASH_CMD_QUAD_IO_FAST_READ            (0xEB)
#define FLASH_CMD_QUAD_IO_WORD_FAST_READ       (0xE7)
#define FLASH_CMD_QUAD_IO_DTR_FAST_READ        (0xED)
#define FLASH_CMD_OCTAL_FAST_READ              (0x8B)
#define FLASH_CMD_OCTAL_IO_FAST_READ           (0xCB)
#define FLASH_CMD_OCTAL_IO_DTR_FAST_READ       (0xFD)
#define FLASH_CMD_CONTINUE_READ_RESET          (0xFF)

#define FLASH_CMD_PP                           (0x02)
#define FLASH_CMD_QUAD_PP                      (0x32)
#define FLASH_CMD_OCTAL_PP                     (0x82)

#define FLASH_CMD_SE                           (0x20)
#define FLASH_CMD_64K_BE                       (0xD8)
#define FLASH_CMD_CE                           (0x60)

#define FLASH_CMD_ESR                          (0x44)  //Erase Security Registers
#define FLASH_CMD_PSR                          (0x42)  //Program Security Registers
#define FLASH_CMD_RSR                          (0x48)  //Read Security Registers
#define FLASH_CMD_QPI                          (0x38)

#define NO_USE                                 (0xffffffff)
#define PAGE_SIZE                              (256)
#define SECTOR_SIZE                            (4*1024)
#define BLOCK_SIZE                             (64*1024)
#define FLASH_CHIP_SIZE                        (32*1024*1024)

typedef enum _SPIFLASH_CMD
{
    CMD_CODE,
    CMD_BUS,
    ADDR_BUS,
    DATA_BUS,
    DUMMY_CNT,
    DTR_DUMMY_CNT,
    MODE_DATA,
}T_FLASH_CMD;

typedef struct _FLASH_PARAM
{
    unsigned int addr_mode;
    unsigned int trans_gap;
    bool crypto_en;
    bool rxds_en;
    T_SFC_BUS_WIDTH bus_wire;
    T_SFC_CS_SELE cs_sele;
    T_SFC_TRANS_DATA_PATH data_path;
    T_SFC_SAMPLE_MODE sample_mode;
}T_FLASH_PARAM;

void sfcv2_set_parm(unsigned int max_hz);
/*********************flash end*****************************/
/******************************* tiaoshi svt end *****************************************/
#endif
#endif
