/*
 * Copyright (C) 2013 Anyka.
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#include <common.h>
#include <spi.h>
#include <malloc.h>
#include <asm/io.h>
//#include <asm/errno.h>

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

#if  defined(CONFIG_KM01A_CODE)
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

#include <dm.h>
#include <errno.h>
#include <fdtdec.h>
#include <clk.h>



#include "ak_spi.h"


DECLARE_GLOBAL_DATA_PTR;


#define SPI_CS_ACTIVE       (1)
#define SPI_CS_INACTIVE     (0)


#define AKSPI_DATA_WIRE(mode)   \
    (((mode & SPI_XFER_4DATAWIRE) == SPI_XFER_4DATAWIRE) ?  \
    AKSPI_4DATAWIRE:((mode & SPI_XFER_2DATAWIRE) == SPI_XFER_2DATAWIRE) ?  \
    AKSPI_2DATAWIRE : AKSPI_1DATAWIRE)

extern unsigned long get_asic_freq(void);

extern unsigned long clk_get_core_pll_freq(void);



/**
 * @brief from slave struct get ak_spi_slave struct.
 *
 * from container_of get ak_spi_slave struct
 * @author CaoDonghua
 * @date 2016-10-20
 * @param[in] slave spi slave handler.
 * @return int struct ak_spi_slave
 * @retval return ak_spi_slave handle on success
 * @retval return other if failed
 */
static inline struct ak_spi_slave *to_ak_spi(struct spi_slave *slave)
{
    return container_of(slave, struct ak_spi_slave, slave);
}


/**
 * @brief software reset spi system module.
 *
 * software reset spi system module for clean spi controller status
 * @author CaoDonghua
 * @date 2016-10-20
 * @param[in] spi ak_spi_slave handler.
 * @return void
 * @retval none
 */
static void spi_reset(struct ak_spi_slave *spi)
{
    //struct spi_slave *slave = &spi->slave;
    ak_soft_reset(AK39_SRESET_SPI1);//ak_soft_reset(SPI_RESET_NUM(slave));
}


/**
 * @brief spi init controller and .
 *
 * software reset spi system module for clean spi controller status
 * @author CaoDonghua
 * @date 2016-10-20
 * @param[in] spi ak_spi_slave handler.
 * @return void
 * @retval none
 */
void spi_init(void)
{
    /* do nothing */
}


/**
 * @brief spi flash cs controll .
 *
 * software force spi controller pull high or low cs signal
 * @author CaoDonghua
 * @date 2016-10-20
 * @param[in] spi ak_spi_slave handler.
 * @param[in] value spi cs inactive or active
 * @return void
 * @retval none
 */
static void ak_spi_chipsel(struct ak_spi_slave *spi, int value)
{
    unsigned long spcon;

    /* change the chipselect state and the state of the spi engine clock */
    switch (value) {
        case SPI_CS_INACTIVE:
            debug("spi cs inactive.\n");
            spcon = readl(spi->regs + AK_SPICON);
            spcon &= ~FORCE_CS;
            __raw_writel(spcon, spi->regs + AK_SPICON);
            break;
        case SPI_CS_ACTIVE:
            debug("spi cs active.\n");
            spcon = readl(spi->regs + AK_SPICON);
            spcon |= FORCE_CS; //by Shaohua
            __raw_writel(spcon, spi->regs + AK_SPICON);
            break;
        default:
            break;
    }
}

/**
 * @brief wait for spi cnt transmit count to zero .
 *
 * wait for spi cnt transmit count to zero for judge transmit finish
 * @author CaoDonghua
 * @date 2016-10-20
 * @param[in] spi ak_spi_slave handler.
 * @param[in] timeout wait time out count
 * @return int 0 wait success, else failed -Ebusy
 * @retval 0 wait finish success
 * @retval -Ebusy wait finish timeout failed
 */
static inline int wait_for_spi_cnt_to_zero(struct ak_spi_slave *spi,
                                                    u32 timeout)
{
    do {
        if (readl(spi->regs + AK_SPICNT) == 0) {
            break;
        }

        udelay(1);
    }while(timeout--);

    return (timeout > 0) ? 0 : -EBUSY;
}

/**
 * @brief init spi controller and interface.
 *
 * init spi controller and interface.
 * @author CaoDonghua
 * @date 2016-10-20
 * @param[in] spi ak_spi_slave handler.
 * @return int 0 return on success, else failed
 * @retval 0 on success
 * @retval else on failed
 */
#ifdef CONFIG_DM_SPI
#if defined(CONFIG_3918AV100_CODE) || defined(CONFIG_3918EV300L_CODE)
static int ak_spi_hw_init(struct ak_spi_slave *spi)
{
    /* software reset spi comtroller*/
    spi_reset(spi);


    //printf("ak_spi_hw_init spi->regs:%lx\r\n", spi->regs);

    //config spi 0 controll mode
    //NOTE: H3D PG
    REG32(spi->regs+ASPEN_SPI_CTRL) = (SPI_MASTER_V | SPI_CTRL_ENABLE |
                                       SPI_MODE0_V | SPI_CTRL_RX_REJECT);


    /* config to spi controller to normal mode */
    REG32(spi->regs+ASPEN_SPI_WORK_MODE) |= 0x1;

    while((REG32(spi->regs+ASPEN_SPI_WORK_MODE) & 0x1)  == 0)
    {;}

    return 0;

}
#endif

#if defined(CONFIG_37_D_CODE) || defined(CONFIG_37_E_CODE)
static int ak_spi_hw_init(struct ak_spi_slave *spi)
{
    /* software reset spi comtroller*/
    spi_reset(spi);

    //config spi 0 controll mode
    //NOTE: H3D PG
    REG32(SPI1_CTRL_REG) = (SPI_MASTER_V | SPI_CTRL_ENABLE |
                            SPI_MODE0_V | SPI_CTRL_RX_REJECT);


    /* config to spi controller to normal mode */
    REG32(SPI1_WORK_MODE) |= 0x1;

    while((REG32(SPI1_WORK_MODE) & 0x1)  == 0)
    {;}

    return 0;
}
#endif

#else

#ifdef CONFIG_39EV33X_CODE
void spi_sharepin_cfg(struct spi_slave *spi)
{
    //unsigned long regval;

    /* SHARE PIN CONFIG */
    if(spi->bus == AKSPI_BUS_NUM1) {
        T_U32 reg_value;
        #if 1
        reg_value = REG32(SHARE_PIN_CFG0_REG);
        reg_value &= ~(1<<25);
        reg_value |= (1<<25);
        REG32(SHARE_PIN_CFG0_REG) = reg_value;

        reg_value = REG32(SHARE_PIN_CFG3_REG);
        reg_value &= ~0x3E03;
        reg_value |= 0x3E03;
        REG32(SHARE_PIN_CFG3_REG) = reg_value;
        #else
        REG32(SHARE_PIN_CFG0_REG) |= (0x0<<25);
        REG32(SHARE_PIN_CFG3_REG) |= (0x3 | (0x3f<<14)); // check h3 ok
        #endif
    }
}
#endif

#if (defined(CONFIG_3918AV100_CODE) || defined(CONFIG_3918EV300L_CODE) || defined(CONFIG_KM01A_CODE) || defined(CONFIG_3918AV130_CODE))
void spi_sharepin_cfg(struct spi_slave *spi)
{
    //unsigned long regval;

    /* SHARE PIN CONFIG */
    if(spi->bus == AKSPI_BUS_NUM1) {
        REG32(SHARE_PIN_CFG4_REG) &= (~(0x2492441));
#if defined(CONFIG_KM01A_CODE) || defined(CONFIG_3918AV130_CODE)
        REG32(0x00000000);
#endif

        REG32(SHARE_PIN_CFG4_REG) |= 0x2492441;
#if defined(CONFIG_KM01A_CODE) || defined(CONFIG_3918AV130_CODE)
           REG32(0x00000000);
#endif

    }
}
#endif



#ifdef CONFIG_37_E_CODE
void spi_sharepin_cfg(struct spi_slave *spi)
{
    //unsigned long regval;

    /* SHARE PIN CONFIG */
    if(spi->bus == AKSPI_BUS_NUM1) {
        REG32(SHARE_PIN_CFG1_REG) &= (~(0x9249));
        REG32(SHARE_PIN_CFG1_REG) |= 0x9249;
    }
}
#endif

#ifdef CONFIG_37_D_CODE

/**
 * @brief set spi interface sharepin.
 *
 * set spi interface sharepin cfg
 * @author CaoDonghua
 * @date 2016-10-20
 * @param[in] spi ak_spi_slave handler.
 * @return void
 * @retval none
 * @notes because spi boot only support spi0 interface
 */
void spi_sharepin_cfg(struct spi_slave *spi)
{
    //unsigned long regval;

    /* SHARE PIN CONFIG */
    if(spi->bus == AKSPI_BUS_NUM1) {
        REG32(SHARE_PIN_CFG4_REG) &= (~(0x3f));
        REG32(SHARE_PIN_CFG3_REG) &= (~(0xfUL<<28));
    }
}
#endif

#ifdef CONFIG_39EV200_CODE
void spi_sharepin_cfg(struct spi_slave *spi)
{
    //unsigned long regval;

    /* SHARE PIN CONFIG */
    if(spi->bus == AKSPI_BUS_NUM1) {
        T_U32 reg_value;
#ifdef CONFIG_SPI_FLASH_PIN_37_40
        reg_value = REG32(SHARE_PIN_CFG0_REG);
        reg_value &= ~(1<<25);
        REG32(SHARE_PIN_CFG0_REG) = reg_value;

        reg_value = REG32(SHARE_PIN_CFG3_REG);
        reg_value |= (0x3)|(0x3F<<14);
        REG32(SHARE_PIN_CFG3_REG) = reg_value;
#else
        reg_value = REG32(SHARE_PIN_CFG0_REG);
        reg_value |= (1<<25);
        REG32(SHARE_PIN_CFG0_REG) = reg_value;

        reg_value = REG32(SHARE_PIN_CFG3_REG);
        reg_value |= (0x3)|(0x3F<<8);
        REG32(SHARE_PIN_CFG3_REG) = reg_value;
#endif
    }
}
#endif

static unsigned long spi_buses[AKSPI_MAX_BUS_NUM] = {
    SPI0_BASE_ADDR,
    SPI1_BASE_ADDR,
};

int spi_claim_bus(struct spi_slave *slave)
{
    //spi_cs_activate(slave);
    return 0;
}
void spi_release_bus(struct spi_slave *slave)
{
    //spi_cs_deactivate(slave);
}


static void spi_clock_enable(struct ak_spi_slave *spi)
{

    unsigned long regval;
    unsigned long en_bit;

    en_bit = (spi->slave.bus == AKSPI_BUS_NUM1) ?
                SPI1_CLOCK_EN_BIT : SPI2_CLOCK_EN_BIT;
    regval = readl(CLOCK_CTRL_REG);
    regval &= ~en_bit;
    __raw_writel(regval, CLOCK_CTRL_REG);
#if defined(CONFIG_KM01A_CODE) || defined(CONFIG_3918AV130_CODE)
               REG32(0x00000000);
#endif


}

extern unsigned long get_asic_pll_clk(void);
static int ak_spi_hw_init(struct ak_spi_slave *spi)
{
    unsigned int typical_hz = 0;
    unsigned int div = 0;
    unsigned long clk = 0;
    unsigned int hz;
#if !defined(CONFIG_39EV33X_CODE) && !defined(CONFIG_39EV200_CODE)
    unsigned int reg_val;
#endif

    /* software reset spi comtroller*/
    spi_reset(spi);

    /* clock enable and set interface sharepin */
    spi_clock_enable(spi);
    spi_sharepin_cfg(&spi->slave);

    if (!typical_hz) {
        typical_hz = spi->max_hz;
    }


#if defined(CONFIG_39EV33X_CODE) || defined(CONFIG_39EV200_CODE)
    clk = get_asic_freq();
#endif

#ifdef CONFIG_37_D_CODE
    clk = get_asic_pll_clk()*1000000;
#endif

#if (defined(CONFIG_3918AV100_CODE) || defined(CONFIG_37_E_CODE) || \
    defined(CONFIG_3918EV300L_CODE) || defined(CONFIG_KM01A_CODE) || defined(CONFIG_3918AV130_CODE))
    clk = clk_get_core_pll_freq();
#endif

    div = clk / (typical_hz*2) - 1;
    //printf("clk:%ld, typical_hz:%d, div:%d\r\n", clk, typical_hz, div);
    if (div > 255) {
        div = 255;
    }

    if (div < 0) {
        div = 0;
    }

    hz = clk /((div+1)*2);

    /* when got clock greater than wanted clock, increase divider */
    if ((hz - typical_hz) > 0) {
        div++;
    }
#if 0
    printf("pre-scaler=%d (wanted %dMhz, got %luMhz)\n",
            div, typical_hz/(1000000),
            (clk / (2 * (div + 1)))/(1000000));
#endif
    spi->freq = hz;

#if defined(CONFIG_39EV33X_CODE) || defined(CONFIG_39EV200_CODE)
    unsigned long spicon;
    /* init spi mode change, config again */
    spicon = DFT_CON;
    spicon &= ~(AK_SPICON_CPHA|AK_SPICON_CPOL);
    if ((spi->mode & SPI_CPHA) == SPI_CPHA) {
        spicon |= AK_SPICON_CPHA;
        //debug("yes SPI_CPHA set mode:0x%x \n", spi->mode);
    }else {
        //debug("no  SPI_CPHA set mode:0x%x \n", spi->mode);
    }

    if ((spi->mode & SPI_CPOL) == SPI_CPOL) {
        spicon |= AK_SPICON_CPOL;
        //debug("yes SPI_CPOL set mode:0x%x \n", spi->mode);
    }else {
        //debug("no  SPI_CPOL set mode:0x%x \n", spi->mode);
    }

    spicon &= ~AK_SPICON_CLKDIV;
    spicon |= div << 8;

    /* program defaults into the registers */
    __raw_writel(spicon, spi->regs + AK_SPICON);

    __raw_writel(0, spi->regs + AK_SPIINT);
#endif

    //debug("spi hz is %lu, div is %u.\n", spi->freq, div);
#if (defined(CONFIG_3918AV100_CODE) || defined(CONFIG_3918EV300L_CODE) || defined(CONFIG_KM01A_CODE) || defined(CONFIG_3918AV130_CODE)) 
    /* disable spi_hsclk & reset spi_hsclk */
    reg_val = REG32(RESET_CTRL_REG);
    reg_val |= (0x1UL<<31);
    REG32(RESET_CTRL_REG) = reg_val;
#if defined(CONFIG_KM01A_CODE) || defined(CONFIG_3918AV130_CODE)
           REG32(0x00000000);
#endif


    reg_val = REG32(SDADC_SPI0_SDDAC_CTRL_REG);
    reg_val &= ~((0x1<<18)|(0xFF<<10));
    REG32(SDADC_SPI0_SDDAC_CTRL_REG) = reg_val;
#if defined(CONFIG_KM01A_CODE) || defined(CONFIG_3918AV130_CODE)
           REG32(0x00000000);
#endif

    /* config spi clk divider, support 80MHZ */
    reg_val = REG32(SDADC_SPI0_SDDAC_CTRL_REG);
    reg_val |= ((div<<10) | (0x1<<19));
    REG32(SDADC_SPI0_SDDAC_CTRL_REG) = reg_val;
#if defined(CONFIG_KM01A_CODE) || defined(CONFIG_3918AV130_CODE)
           REG32(0x00000000);
#endif

    //wait spi clk div finish
    while((REG32(SDADC_SPI0_SDDAC_CTRL_REG) & (0x1<<19)))
    {;}

    //enable spi_hsclk
    REG32(SDADC_SPI0_SDDAC_CTRL_REG) |= (0x1<<18);
#if defined(CONFIG_KM01A_CODE) || defined(CONFIG_3918AV130_CODE)
           REG32(0x00000000);
#endif

    //release spi_hsclk
    REG32(RESET_CTRL_REG) &= ~(0x1UL<<31);
#if defined(CONFIG_KM01A_CODE) || defined(CONFIG_3918AV130_CODE)
           REG32(0x00000000);
#endif

    //config spi 0 controll mode
    REG32(spi->regs + ASPEN_SPI_CTRL) = (SPI_MASTER_V | SPI_CTRL_ENABLE |
                                        SPI_MODE0_V | SPI_CTRL_RX_REJECT);

        /* config to spi controller to normal mode */
    REG32(spi->regs + ASPEN_SPI_WORK_MODE) |= 0x1;

    while((REG32(spi->regs + ASPEN_SPI_WORK_MODE) & 0x1)  == 0)
    {;}
#endif

#if (defined(CONFIG_37_D_CODE) || defined(CONFIG_37_E_CODE))
    /* disable spi_hsclk & reset spi_hsclk */
    reg_val = REG32(HS_SPI_CLK_CFG_REG);
    reg_val &= ~0x3ff;
    reg_val |= (0x1UL<<31);
    REG32(HS_SPI_CLK_CFG_REG) = reg_val;

    /* config spi clk divider, support 80MHZ */
    reg_val = REG32(HS_SPI_CLK_CFG_REG);
    reg_val &= ~0x3ff;
    reg_val |= (div | (0x1<<9));
    REG32(HS_SPI_CLK_CFG_REG) = reg_val;

    /* wait spi clk div finish */
    while((REG32(HS_SPI_CLK_CFG_REG) & (0x1<<9)))
    {;}

    /* enable spi_hsclk */
    REG32(HS_SPI_CLK_CFG_REG) |= (0x1<<8);

    /* release spi_hsclk */
    REG32(HS_SPI_CLK_CFG_REG) &= ~(0x1UL<<31);

    //config spi 0 controll mode
    //NOTE: H3D
    REG32(SPI1_CTRL_REG) = (SPI_MASTER_V | SPI_CTRL_ENABLE |
                            SPI_MODE0_V | SPI_CTRL_RX_REJECT);


    /* config to spi controller to normal mode */
    REG32(SPI1_WORK_MODE) |= 0x1;

    while((REG32(SPI1_WORK_MODE) & 0x1)  == 0)
    {;}

#endif

    return 0;
}
/*end of ak_spi_hw_init*/

/**
 * @brief init spi slave device and register.
 *
 * init spi slave device and register.
 * @author CaoDonghua
 * @date 2016-10-20
 * @param[in] bus spi interface num.
 * @param[in] cs  spi chip device select
 * @param[in] max_hz max transmit speed
 * @param[in] mode time sequence mode
 * @return struct spi_slave spi_slave handle return on success, else failed
 * @retval spi_slave handle on success
 * @retval else on failed
 */
struct spi_slave *spi_setup_slave(unsigned int bus, unsigned int cs,
                  unsigned int max_hz, unsigned int mode)
{
    struct ak_spi_slave *spi;

    debug("bus:%d, cs:%d, max_hz:%d, mode:%d.\n",
            bus, cs, max_hz, mode);

    debug("start setup the spi flash.\n");
    if (bus >= AKSPI_MAX_BUS_NUM) {
        printf("SPI error: unsupported bus %u. \
            Supported busses 0 - 1\n", bus);
        return NULL;
    }

    if (max_hz > AKSPI_MAX_FREQ) {
        printf("SPI error: unsupported frequency %u Hz. \
            Max frequency is %d/1000000 Mhz\n", max_hz, AKSPI_MAX_FREQ);
        return NULL;
    }

    spi = spi_alloc_slave(struct ak_spi_slave, bus, cs);
    if (!spi) {
        printf("SPI error: malloc of SPI structure failed\n");
        return NULL;
    }

    spi->regs = spi_buses[bus];
    spi->max_hz = max_hz;
    spi->freq = 0;
    spi->mode = mode;
    spi->slave.max_hz = max_hz;
    if (bus > 0)
        ak_spi_hw_init(spi);

    return &spi->slave;
}

/**
 * @brief init spi slave device and register.
 *
 * init spi slave device and register.
 * @author CaoDonghua
 * @date 2016-10-20
 * @param[in] slave slave handle.
 * @return void
 * @retval none
 */
void spi_free_slave(struct spi_slave *slave)
{
    struct ak_spi_slave *spi = to_ak_spi(slave);

    free(spi);
}

#endif

/**
 * @brief spi cs activate cfg.
 *
 * spi cs active cfg.
 * @author CaoDonghua
 * @date 2016-10-20
 * @param[in] slave slave handle.
 * @return void
 * @retval none
 */
void spi_cs_activate(struct spi_slave *slave)
{
    struct ak_spi_slave *spi = to_ak_spi(slave);

    ak_spi_chipsel(spi, SPI_CS_ACTIVE);
}

/**
 * @brief spi cs deactivate cfg.
 *
 * spi cs active cfg.
 * @author CaoDonghua
 * @date 2016-10-20
 * @param[in] slave slave handle.
 * @return void
 * @retval none
 */
void spi_cs_deactivate(struct spi_slave *slave)
{
    struct ak_spi_slave *spi = to_ak_spi(slave);

    ak_spi_chipsel(spi, SPI_CS_INACTIVE);
}

/**
 * @brief start spi tx or rx data action cfg .
 *
 * cfg spi controller to start spi tx or rx data action.
 * @author CaoDonghua
 * @date 2016-10-20
 * @param[in] spi ak_spi_slave handle.
 * @param[in] dir tx or rx direction
 * @param[in] flags bus width
 * @return void
 * @retval none
 */
static void ak_spi_start_txrx(struct ak_spi_slave *spi, int dir, int flags)
{
    u32 reg_value;

    debug("the spi transfer mode is %s.\n", (dir == SPI_DIR_TXRX) ?
                "txrx" : (dir == SPI_DIR_RX)? "rx":"tx");

    reg_value = readl(spi->regs + AK_SPICON);
    switch(dir) {
        case SPI_DIR_TX:
            reg_value &= ~AK_SPICON_TGDM;
            reg_value |= AK_SPICON_ARRM;
            break;
        case SPI_DIR_RX:
            reg_value |= AK_SPICON_TGDM;
            reg_value &= ~AK_SPICON_ARRM;
            break;
        case SPI_DIR_TXRX:
            reg_value &= ~AK_SPICON_TGDM;
            reg_value &= ~AK_SPICON_ARRM;
            break;
        default:
            break;
    }

    debug("xfer use for %s wire mode(flags:%d).\n",
            (flags & SPI_XFER_4DATAWIRE) ?"4":
            ((flags & SPI_XFER_2DATAWIRE) ? "2":"1"), flags);

    /* configure the data wire */
    reg_value &= ~AK_SPICON_WIRE;
    reg_value |= AKSPI_DATA_WIRE(flags);
    __raw_writel(reg_value, spi->regs + AK_SPICON);

    if(dir == SPI_DIR_TX)
        __udelay(50);//add for spinand write data error
}


/**
* @brief  configure the master register when stop transfer data.
*
* configure the master register when stop transfer data
* @author      CaoDonghua
* @date        2016-10-20
* @param[in]  *spi :ak spi master dev.
* @param[in]   dir  : read or write direction.
* @return      void
* @retval none
*/
static void ak_spi_stop_txrx(struct ak_spi_slave *spi, int dir)
{
    u32 reg_value;

    reg_value = readl(spi->regs + AK_SPICON);
    reg_value &= ~AK_SPICON_WIRE;
    __raw_writel(reg_value, spi->regs + AK_SPICON);
}



/**
* @brief  spi tx transfer as unit byte
*
* spi tx transfer as unit byte
* @author      CaoDonghua
* @date        2016-10-20
* @param[in]   *spi ak_spi_slave handle
* @param[in]   len  transmit byte
* @return      unsigned int transmit value
* @retval transmit byte value
*/
static unsigned int spi_txbyte(struct ak_spi_slave *spi, int len)
{
    u32 val = 0;
    int i = 0;

    while (i < len) {
        val |= (spi->tx[spi->count+i] << i*8);
        i++;
    }

    return val;
}

/**
* @brief  spi tx transfer as unit word
*
* spi tx transfer as unit word
* @author      CaoDonghua
* @date        2016-10-20
* @param[in]   *spi
* @return      unsigned int
* @retval transmit word value
*/
static unsigned int spi_txdword(struct ak_spi_slave *spi)
{
    u32 val = 0;
    int l = 0;

    l = (spi->len - spi->count) > 4 ? 4 : (spi->len - spi->count);
    val = spi_txbyte(spi, l);
    spi->count += l;
    // debug("[%08x] ", val);

    return val;
}

/**
* @brief  spi rx transfer as unit byte
*
* spi tx transfer as unit byte
* @author      CaoDonghua
* @date        2016-10-20
* @param[in] *spi  ak_spi_slave handle
* @param[in] val rx value
* @param[in] len rx count
* @return  void
* @retval none
*/
static void spi_rxbyte(struct ak_spi_slave *spi, unsigned int val, int len)
{
    int i = 0;

    while (i < len) {
        spi->rx[spi->count + i] = (val >> i*8) & 0xff;
        i++;
    }
}

/**
* @brief  spi rx transfer as unit word
*
* spi rx transfer as unit word
* @author      CaoDonghua
* @date        2016-10-20
* @param[in] *spi  ak_spi_slave handle
* @param[in] val rx word value
* @return void
* @retval none
*/
static void spi_rxdword(struct ak_spi_slave *spi, unsigned int val)
{
    int l = 0;

    l = (spi->len - spi->count) > 4 ? 4 : (spi->len - spi->count);
    spi_rxbyte(spi, val, l);
    spi->count += l;
    // debug("[%08x] ", val);
}


/**
* @brief  spi transmit remain data len
*
* spi transmit remain data len
* @author      CaoDonghua
* @date        2016-10-20
* @param[in] *spi  ak_spi_slave handle
* @return u32 remain data len
* @retval remain data len
*/
static u32 spi_remain_datalen(struct ak_spi_slave *spi)
{
    return (spi->len - spi->count);
}


/**
* @brief  spi write transmit with CPU mode
*
* send message function with CPU mode and polling mode
* @author      CaoDonghua
* @date        2016-10-20
* @param[in] *spi ak_spi_slave handle
* @param[in] *buf tx data buffer
* @param[in] count tx len
* @return  int spi->count on success, else on failed
* @retval return spi->count on success
* @retval else negative data on failed
*/
static int spi_pio_write(struct ak_spi_slave *spi,
                        unsigned char *buf, int count)
{
    u32 status;
    u32 to = 0;

    //  debug("ak spi write by cpu mode\n");
    if (count > 64*1024) {
        printf("too much to be send...\n");
        return -EINVAL;
    }

    /* set data count, and the the master will rise clk */
    __raw_writel(count, spi->regs + AK_SPICNT);

    while(spi_remain_datalen(spi) > 0) {
        status = readl(spi->regs + AK_SPISTA);
        if ((status & AK_SPISTA_TXHEMP) == AK_SPISTA_TXHEMP) {
            __raw_writel(spi_txdword(spi), spi->regs + AK_SPIOUT);
        }else {
            if (to++ > 10*1000000) {
                printf("master write data timeout.\n");
                return spi->count;
            }
        }
    }

    /* wait transfer finish */
    while(1) {
        status = readl(spi->regs + AK_SPISTA);
        if ((status & AK_SPISTA_TRANSF) == AK_SPISTA_TRANSF) {
            break;
        }

        if (to++ > 10 * 1000000) {
            printf("wait for write data finish timeout..\n");
            return spi->count;
        }
    }

    if (spi_remain_datalen(spi) > 0) {
        printf("write wasn't finished.\n");
    }

    return spi->count;
}

/**
* @brief spi read receive with CPU mode
*
* receiving message function with CPU mode and polling mode
* @author      CaoDonghua
* @date        2016-10-20
* @param[in] *spi ak_spi_slave handle
* @param[in] *buf rx data buffer
* @param[in] count rx count
* @return      int  spi->count on success, else other on failed
* @retval return spi->count on success
* @retval else negative data on failed
*/
static int spi_pio_read(struct ak_spi_slave *spi, unsigned char *buf, int count)
{
    u32 status;
    u32 to=0;

    //debug("ak spi read by cpu mode\n");
    if (count >= 64*1024) {
        printf("too much to be read...\n");
        return -EINVAL;
    }

    /* set data count, and the the master will rise clk */
    __raw_writel(count, spi->regs + AK_SPICNT);

    while(1) {
        status = readl(spi->regs + AK_SPISTA);
        if((status & AK_SPISTA_TRANSF) == AK_SPISTA_TRANSF) {
            if(status & AK_SPISTA_RXFULL) {
                spi_rxdword(spi, readl(spi->regs + AK_SPIIN));
                spi_rxdword(spi, readl(spi->regs + AK_SPIIN));
            }else if (status & AK_SPISTA_RXHFULL) {
                spi_rxdword(spi, readl(spi->regs + AK_SPIIN));
            }

            if (spi_remain_datalen(spi) > 0) {
                spi_rxdword(spi, readl(spi->regs + AK_SPIIN));
            }
            break;
        } else {
            if((status & AK_SPISTA_RXHFULL) == AK_SPISTA_RXHFULL) {
                spi_rxdword(spi, readl(spi->regs + AK_SPIIN));
            }
            else {
                if (to++ > 10 * 1000000) {
                    debug("master read timeout.\n");
                    return spi->count;
                }
            }
        }
    }

    if (spi_remain_datalen(spi) > 0) {
        printf("read wasn't finished.\n");
    }

    return spi->count;
}


int ak_spi_read_cpu(struct spi_slave *slave,
            unsigned int len, u8 *rxp, unsigned long flags)
{
    int retlen = 0;
    u32 count = len;
    struct ak_spi_slave *spi = to_ak_spi(slave);

    spi->rx = rxp;
    //debug("start the spi pio read len:0x%x.\n", len);
    ak_spi_start_txrx(spi, SPI_DIR_RX, flags);

    while(count > 0) {
        spi->count = 0;
        spi->len = (count > MAX_XFER_LEN) ? MAX_XFER_LEN : count;

        retlen = spi_pio_read(spi, spi->rx, spi->len);
        if (unlikely(retlen < 0)) {
            printf("spi master transfer data error!\n");
            goto txrx_ret;
        }

        spi->rx += retlen;
        count -= retlen;

    }
    ak_spi_stop_txrx(spi, SPI_DIR_RX);

    //debug("finish the spi pio read.\n");

txrx_ret:
    return (retlen<0) ? retlen : len;
}

int ak_spi_write_cpu(struct spi_slave *slave,
                unsigned int len, const u8 *txp, unsigned long flags)
{

    int retlen = 0;
    u32 count = len;
    struct ak_spi_slave *spi = to_ak_spi(slave);

    spi->tx = (unsigned char*)txp;
    //debug("start the spi pio write.len:%d, data:0x%x\n", len, txp[0]);
    //debug("data[0]:0x%x,data0:0x%x, data123:0x%x%x%x .\n",
    //                  len, txp[0], txp[1],txp[2],txp[3]);
    ak_spi_start_txrx(spi, SPI_DIR_TX, flags);

    while(count > 0) {
        spi->count = 0;
        spi->len = (count > MAX_XFER_LEN) ? MAX_XFER_LEN : count;

        retlen = spi_pio_write(spi, spi->tx, spi->len);
        if (unlikely(retlen < 0)) {
            printf("spi master transfer data error!\n");
            goto txrx_ret;
        }

        spi->tx += retlen;
        count -= retlen;
    }
    //debug("finish the spi pio write.\n");

    ak_spi_stop_txrx(spi, SPI_DIR_TX);
txrx_ret:
    return (retlen<0) ? retlen : len;
}




#if defined(CONFIG_SPI_XFER_CPU)


/**
* @brief spi read data with whole process
*
* spi read data with whole process
* @author      CaoDonghua
* @date        2016-10-20
* @param[in] *slave slave handle
* @param[in] len rx data len
* @param[in] *rxp rx buffer pointer
* @param[in] flags bus width flags
* @return      int  len on success, else retlen on failed
* @retval return len on success
* @retval else retlen data on failed
*/
int ak_spi_read(struct spi_slave *slave,
            unsigned int len, u8 *rxp, unsigned long flags)
{
    int retlen;
    u32 count = len;
    struct ak_spi_slave *spi = to_ak_spi(slave);

    spi->rx = rxp;
    //debug("start the spi pio read len:0x%x.\n", len);
    ak_spi_start_txrx(spi, SPI_DIR_RX, flags);

    while(count > 0) {
        spi->count = 0;
        spi->len = (count > MAX_XFER_LEN) ? MAX_XFER_LEN : count;

        retlen = spi_pio_read(spi, spi->rx, spi->len);
        if (unlikely(retlen < 0)) {
            printf("spi master transfer data error!\n");
            goto txrx_ret;
        }

        spi->rx += retlen;
        count -= retlen;

    }
    ak_spi_stop_txrx(spi, SPI_DIR_RX);

    //debug("finish the spi pio read.\n");

txrx_ret:
    return (retlen<0) ? retlen : len;
}


/**
* @brief spi write data with whole process
*
* spi write data with whole process
* @author      CaoDonghua
* @date        2016-10-20
* @param[in] *slave slave handle
* @param[in] len tx data len
* @param[in] *txp tx buffer pointer
* @param[in] flags bus width flags
* @return      int  len on success, else retlen on failed
* @retval return len on success
* @retval else retlen data on failed
*/
int ak_spi_write(struct spi_slave *slave,
            unsigned int len, const u8 *txp, unsigned long flags)
{

    int retlen;
    u32 count = len;
    struct ak_spi_slave *spi = to_ak_spi(slave);

    spi->tx = (unsigned char*)txp;
    //debug("start the spi pio write.len:%d, data:0x%x\n", len, txp[0]);
    //debug("data[0]:0x%x,data0:0x%x, data123:0x%x%x%x .\n",
    //              len, txp[0], txp[1],txp[2],txp[3]);
    ak_spi_start_txrx(spi, SPI_DIR_TX, flags);

    while(count > 0) {
        spi->count = 0;
        spi->len = (count > MAX_XFER_LEN) ? MAX_XFER_LEN : count;

        retlen = spi_pio_write(spi, spi->tx, spi->len);
        if (unlikely(retlen < 0)) {
            printf("spi master transfer data error!\n");
            goto txrx_ret;
        }

        spi->tx += retlen;
        count -= retlen;
    }
    //debug("finish the spi pio write.\n");

    ak_spi_stop_txrx(spi, SPI_DIR_TX);
txrx_ret:
    return (retlen<0) ? retlen : len;
}


#else //#elif defined(CONFIG_SPI_XFER_DMA)

int ak_spi_write_internal(struct ak_spi_slave *spi, const u8 *buf,
        unsigned int count)
{
    int ret = 0;
    bool is_dma = false;
    u32 val;

    val = AK_SPIEXTX_BUFEN|AK_SPIEXTX_DMAEN;
    __raw_writel(val, spi->regs + AK_SPIEXTX);
    __raw_writel(count, spi->regs + AK_SPICNT);

    //dbg_dumpdata("write", buf, count);
    /*use for dma mode: greater than 256 bytes, align 4bytes of buf addr,
        align 64 bytes of data count*/
    if((count < 256) || ((unsigned long)buf & 0x3) || (count & (64 - 1))) {
        l2_combuf_cpu((unsigned long)buf, spi->l2buf_tid, count, MEM2BUF);

    } else {
        //start l2 dma transmit
        l2_combuf_dma((u32)buf, spi->l2buf_tid, count, MEM2BUF, AK_FALSE);
        is_dma = true;
    }

    if (is_dma && (l2_combuf_wait_dma_finish(spi->l2buf_tid) == AK_FALSE))  {
        printf("%s: l2_combuf_wait_dma_finish failed!\n", __func__);
        ret = -EINVAL;
        goto xfer_fail;
    }

    ret = wait_for_spi_cnt_to_zero(spi, TRANS_TIMEOUT);
    if(ret) {
        printf("%s: wait_for_spi_cnt_to_zero failed!\n", __func__);
        ret = -EINVAL;
        goto xfer_fail;
    }

    ret = count;
xfer_fail:
    //disable l2 dma
    __raw_writel(0, spi->regs + AK_SPIEXTX);
    l2_clr_status(spi->l2buf_tid);
    return ret;
}

int ak_spi_read_internal(struct ak_spi_slave *spi, u8 *buf,
        unsigned int count)
{
    int ret = 0;
    bool is_dma = false;
    u32 val;
    u32 loop;

    //prepare spi read
    val = AK_SPIEXTX_BUFEN|AK_SPIEXTX_DMAEN;
    __raw_writel(val, spi->regs + AK_SPIEXRX);
    __raw_writel(count, spi->regs + AK_SPICNT);

    if(count < 256 || ((unsigned long)buf & 0x3) || (count & (64 - 1))) {
     #if 0
     __udelay (20);
     l2_combuf_cpu((unsigned long)buf, spi->l2buf_rid, count, BUF2MEM);
     #else
     loop = count/64;

        if(loop){
            l2_combuf_cpu((unsigned long)buf, spi->l2buf_rid, loop*64, BUF2MEM);
        }
        if(count%64){
            /*wait for spi count register value to zero.*/
            ret = wait_for_spi_cnt_to_zero(spi, TRANS_TIMEOUT);
            if(ret) {
                printf("%s: wait for spi count to zero failed!\n", __func__);
                ret = -EINVAL;
                goto xfer_fail;
            }

            l2_combuf_cpu_frac((unsigned long)buf,
                        spi->l2buf_rid, loop, count%64, BUF2MEM);
        }
    #endif
    }
    else {
        //start L2 dma
        l2_combuf_dma((u32)buf, spi->l2buf_rid, count, BUF2MEM, AK_FALSE);
        is_dma = true;
    }

    //wait L2 dma finish, if need frac dma,start frac dma
    if (is_dma && l2_combuf_wait_dma_finish(spi->l2buf_rid) ==  AK_FALSE)   {
        ret = -EINVAL;
        goto xfer_fail;
    }

    /*wait for spi count register value to zero.*/
    ret = wait_for_spi_cnt_to_zero(spi, TRANS_TIMEOUT);
    if(ret) {
        printf("%s: wait for spi count to zero failed!\n", __func__);
        ret = -EINVAL;
        goto xfer_fail;
    }

//  dbg_dumpdata("read", buf, count);
    ret = count;
xfer_fail:
    //disable l2 dma
    __raw_writel(0, spi->regs + AK_SPIEXRX);
    l2_clr_status(spi->l2buf_rid);
    return ret;
}

int ak_spi_write(struct spi_slave *slave, unsigned int len, const u8 *txp,
            unsigned long flags)
{
    int ret = 0;
    u32 retlen = 0;
    u32 l;
    u32 count = 0;
    struct ak_spi_slave *spi = to_ak_spi(slave);

    debug("start the spi dma transfer.\n");

    ak_spi_start_txrx(spi, SPI_DIR_TX, flags);
    //alloc L2 buffer
    //spi->l2buf_tid = l2_alloc(SPI_L2_TXADDR(slave));
    spi->l2buf_tid = l2_alloc(ADDR_SPI1_TX);

    if (unlikely(BUF_NULL == spi->l2buf_tid)) {
        printf("%s: l2_alloc failed!\n", __func__);
        return -EBUSY;
    }
    l2_clr_status(spi->l2buf_tid);

    while(len > 0) {
        l = (len > MAX_XFER_LEN) ? MAX_XFER_LEN : len;

        retlen = ak_spi_write_internal(spi, txp + count, l);
        if(unlikely(retlen < 0)) {
            printf("spi master read data error!\n");
            ret = -EBUSY;
            goto txrx_ret;
        }
        count += retlen;
        len -= retlen;
    }

    ak_spi_stop_txrx(spi, SPI_DIR_TX);
    debug("finish the spi dma transfer.\n");
txrx_ret:
    if(spi->l2buf_tid != BUF_NULL)
        l2_free(ADDR_SPI1_TX);//l2_free(SPI_L2_TXADDR(slave));

    return ret ? ret : count;
}

int ak_spi_read(struct spi_slave *slave, unsigned int len, u8 *rxp,
           unsigned long flags)
{
    int ret = 0;
    u32 retlen;
    u32 l;
    u32 count = 0;
    struct ak_spi_slave *spi = to_ak_spi(slave);

    debug("start the spi dma transfer.\n");

    ak_spi_start_txrx(spi, SPI_DIR_RX, flags);
    //alloc L2 buffer
    //spi->l2buf_rid = l2_alloc(SPI_L2_RXADDR(slave));
    spi->l2buf_rid = l2_alloc(ADDR_SPI1_RX);

    if (unlikely(BUF_NULL == spi->l2buf_rid))
    {
        return -EBUSY;
    }
    l2_clr_status(spi->l2buf_rid);

    while(len > 0) {
        l = (len > MAX_XFER_LEN) ? MAX_XFER_LEN : len;

        retlen = ak_spi_read_internal(spi, rxp + count, l);
        if(unlikely(retlen < 0)) {
            printf("spi master read data error!\n");
            ret = -EBUSY;
            goto txrx_ret;
        }
        count += retlen;
        len -= retlen;
    }

    ak_spi_stop_txrx(spi, SPI_DIR_RX);
    debug("finish the spi dma transfer.\n");
txrx_ret:
    if(spi->l2buf_rid != BUF_NULL)
        l2_free(ADDR_SPI1_RX);//l2_free(SPI_L2_RXADDR(slave));

    return ret ? ret : count;
}

#endif


/**
* @brief spi write and read data with whole process
*
* spi write and read  data with whole process
* @author      CaoDonghua
* @date        2016-10-20
* @param[in] *slave slave handle
* @param[in] len tx data len
* @param[in] *txp tx buffer pointer
* @param[in] *rxp rx buffer pointer
* @param[in] flags bus width flags
* @return      int  always return 0
* @retval always return 0
*/
int ak_spi_txrx(struct spi_slave *slave, unsigned int len,
                    const u8 *txp, u8 *rxp, unsigned long flags)
{
    printf("sorry, not support duplex mode now.\n");
    return 0;
}



/**
* @brief spi transmit data with whole process
*
* spi transmit data with whole process
* @author      CaoDonghua
* @date        2016-10-20
* @param[in] *slave slave handle
* @param[in] bitlen transmit data bit len
* @param[in] *dout tx buffer pointer
* @param[in] *din rx buffer pointer
* @param[in] flags bus width flags
* @return      int  0 on success, else other on failed
* @retval 0 on success
* @retval else other on failed
*/
#ifdef CONFIG_DM_SPI
int spi_xfer_ex(struct spi_slave *slave, unsigned int bitlen,
         const void *dout, void *din, unsigned long flags)
#else
int spi_xfer(struct spi_slave *slave, unsigned int bitlen,
         const void *dout, void *din, unsigned long flags)
#endif
{
    unsigned int    len;
    const u8    *txp = dout;
    u8      *rxp = din;
    int ret = -1;

    if (bitlen % 8) {
        return -1;
    }
    len = bitlen / 8;

    if (flags & SPI_XFER_BEGIN) {
        spi_cs_activate(slave);
        //debug("spi_cs_activate.\n");
    }

    /* data transmit default use 1 wire while send cmd */
    if ((dout != NULL) && (din != NULL)) {
        //debug("ak_spi_txrx\n");
        ret = ak_spi_txrx(slave, len, txp, rxp, flags);

    }else if (dout != NULL) {
        //printf("ak_spi_write\n");
        if(len < 512 || ((unsigned long)txp & 0x3))
            ret = ak_spi_write_cpu(slave, len, txp, flags);
        else
            ret = ak_spi_write(slave, len, txp, flags);

    }else if (din != NULL) {
        if(len < 512 || ((unsigned long)txp & 0x3))
            ret = ak_spi_read_cpu(slave, len, rxp, flags);
        else
            ret = ak_spi_read(slave, len, rxp, flags);
    }

    if (flags & SPI_XFER_END) {
        spi_cs_deactivate(slave);
        //debug("spi_cs_deactivate\n");
    }

    return (ret < 0) ? ret:0;
}

/**
* @brief spi cs check if valid or not
*
* spi cs check if valid or not
* @author      CaoDonghua
* @date        2016-10-20
* @param[in] bus spi bus num
* @param[in] cs spi chip select
* @return      int  always return 1
* @retval 1 always return 1
*/
#if 0
int spi_cs_is_valid(unsigned int bus, unsigned int cs)
{
    return 1;
}
#endif
////////////////////////////////////////


#ifdef CONFIG_DM_SPI


static int ak_spi_probe(struct udevice *dev)
{
    struct ak_spi_slave *aks = dev_get_platdata(dev);
    int node = dev_of_offset(dev);
    const void *blob = gd->fdt_blob;
    struct clk clk;
    int ret;

    aks->slave.dev= (struct udevice *)&dev;
    aks->regs = devfdt_get_addr(dev);
    if (aks->regs == FDT_ADDR_T_NONE)
        return -ENODEV;
    aks->max_hz = fdtdec_get_int(blob, node, "spi-max-frequency",
                      20000000);
    debug("aks->max_hz:%ld\r\n", aks->max_hz);
    aks->freq = 0;
    aks->mode = 0;

    //config clk enable
    ret = clk_get_by_index(dev, 0, &clk);
    if (ret){
        printf("clk_get_by_index 0 fail:%d\r\n", ret);
        return ret;
    }
    ret = clk_enable(&clk);
    if (ret){
        printf("clk_enable fail:%d\r\n", ret);
        return ret;
    }
#if !defined(CONFIG_39EV33X_CODE) && !defined(CONFIG_39EV200_CODE)
    ulong rate;
    //config spi clk div // for fast spi
    ret = clk_get_by_index(dev, 1, &clk);
    if (ret){
        printf("clk_get_by_index 1 fail:%d\r\n", ret);
        return ret;
    }
    rate = clk_set_rate(&clk, aks->max_hz);
    debug("rate:%ld\r\n", rate);

#if !defined(CONFIG_KM01A_CODE) && !defined(CONFIG_3918AV130_CODE)
    ak_spi_hw_init(aks);
#else
    spi_reset(aks);
#endif
#else

    unsigned long core_clk = 0;
    unsigned int div = 0;
    unsigned long hz;
    unsigned long spicon;

    spi_reset(aks);

    core_clk = clk_get_rate(&clk);

    hz = core_clk /((div+1)*2);
    while (hz > aks->max_hz) {
        div++;
        hz = core_clk /((div+1)*2);
    }
    aks->freq = hz;

    debug("spi hz is %lu %lu, div is %u.\n", core_clk, aks->freq, div);

    /* init spi mode change, config again */
    spicon = DFT_CON;
    spicon &= ~AK_SPICON_CLKDIV;
    spicon |= div << 8;

    /* program defaults into the registers */
    __raw_writel(spicon, aks->regs + AK_SPICON);
    __raw_writel(0, aks->regs + AK_SPIINT);
#endif

    return 0;
}

static int ak_spi_xfer(struct udevice *dev, unsigned int bitlen,
        const void *dout, void *din, unsigned long flags)
{
    struct udevice *bus = dev->parent;
    struct ak_spi_slave *aks = dev_get_platdata(bus);

    return spi_xfer_ex(&aks->slave, bitlen, dout, din, flags);
}

static int ak_spi_claim_bus(struct udevice *dev)
{
    //struct ak_spi_slave *aks = dev_get_platdata(dev);
    //struct dm_spi_slave_platdata *slave_plat = dev_get_parent_platdata(dev);

    //aks->dev = dev;
    //printf("ak_spi_claim_bus\r\n");

    return 0;//mxc_spi_claim_bus_internal(aks, slave_plat->cs);
}

static int ak_spi_release_bus(struct udevice *dev)
{
    //printf("ak_spi_release_bus\r\n");
    return 0;
}

static int ak_spi_set_speed(struct udevice *dev, uint speed)
{
    //printf("ak_spi_set_speed\r\n");
    /* Nothing to do */
    return 0;
}

static int ak_spi_set_mode(struct udevice *dev, uint mode)
{
    unsigned long spicon;
    struct udevice *bus = dev->parent;
    struct ak_spi_slave *aks = dev_get_platdata(bus);

    spicon = readl(aks->regs + AK_SPICON);

    spicon &= ~(AK_SPICON_CPHA|AK_SPICON_CPOL);
    if ((mode & SPI_CPHA) == SPI_CPHA) {
        spicon |= AK_SPICON_CPHA;
        debug("yes SPI_CPHA set mode:0x%x \n", mode);
    }else {
        debug("no  SPI_CPHA set mode:0x%x \n", mode);
    }

    if ((mode & SPI_CPOL) == SPI_CPOL) {
        spicon |= AK_SPICON_CPOL;
        debug("yes SPI_CPOL set mode:0x%x \n", mode);
    }else {
        debug("no  SPI_CPOL set mode:0x%x \n", mode);
    }

    __raw_writel(spicon, aks->regs + AK_SPICON);

    //printf("ak_spi_set_mode\r\n");
    return 0;
}

static const struct dm_spi_ops ak_spi_ops = {
    .claim_bus  = ak_spi_claim_bus,
    .release_bus    = ak_spi_release_bus,
    .xfer       = ak_spi_xfer,
    .set_speed  = ak_spi_set_speed,
    .set_mode   = ak_spi_set_mode,
};

static const struct udevice_id ak_spi_ids[] = {
    { .compatible = "anyka,ak37e-spi0" },
    { .compatible = "anyka,ak37d-spi0" },
    { .compatible = "anyka,ak39ev330-spi0" },
    { .compatible = "anyka,ak3918av100-spi0" },
    { .compatible = "anyka,ak39ev200-spi0" },
    { .compatible = "anyka,ak3918ev300l-spi0" },
    { }
};

U_BOOT_DRIVER(ak_spi) = {
    .name   = "ak_spi",
    .id = UCLASS_SPI,
    .of_match = ak_spi_ids,
    .ops    = &ak_spi_ops,
    .platdata_auto_alloc_size = sizeof(struct ak_spi_slave),
    .probe  = ak_spi_probe,
};

#endif

