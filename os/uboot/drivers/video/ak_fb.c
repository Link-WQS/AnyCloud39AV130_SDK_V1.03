/*
 * Copyright (C) 2012 Samsung Electronics
 *
 * Author: InKi Dae <inki.dae@samsung.com>
 * Author: Donghwa Lee <dh09.lee@samsung.com>
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#include <asm/config.h>
#include <common.h>
#include <lcd.h>
#include <fdtdec.h>
#include <asm/io.h>
#include <errno.h>
#include "ak_fb.h"
#include <ubi_uboot.h>
#include <asm-generic/gpio.h>
#include <dm.h>
#include <video.h>
#include <bmp_layout.h>

#define AKFB_DEBUG  pr_debug

#ifdef CONFIG_37_E_CODE
#include <asm/arch-ak37e/ak_cpu.h>
#endif
#ifdef CONFIG_37_D_CODE
#include <asm/arch-ak37d/ak_cpu.h>
#endif


DECLARE_GLOBAL_DATA_PTR;

struct ak_fb *fb_dev;
enum PANNEL_IF_TYPE{
    RGB = 0,
    MIPI,
};

#ifdef CONFIG_37_D_CODE
unsigned long get_peri_pll_freq(void)
{
    unsigned long pll_m, pll_n, pll_od;
    unsigned long peril_pll_clk;
    unsigned long regval;

    regval = readl(PERIL_PLL_CHANNEL_CTRL_REG);
    pll_od = (regval & (0x3 << 12)) >> 12;
    pll_n = (regval & (0xf << 8)) >> 8;
    pll_m = regval & 0xff;

    peril_pll_clk = (12 * pll_m)/(pll_n * (1 << pll_od));

    if ((pll_od >= 1) && ((pll_n >= 1) && (pll_n <= 12))
        && ((pll_m >= 84) && (pll_m <= 254)))
        return peril_pll_clk;

    return 0;

}
#endif

static s32 fdtdec_get_valoffset_int(const void *blob, int node,
    const char *prop_name, int valoffset,s32 default_val)
{
    const s32 *cell;
    int len;

    AKFB_DEBUG("%s: %s\n", __func__, prop_name);
    cell = fdt_getprop(blob, node, prop_name, &len);
    if (cell && len >= sizeof(s32) && (valoffset < len/sizeof(s32))) {
        s32 val = fdt32_to_cpu(cell[valoffset]);
        return val;
    }
    AKFB_DEBUG("%s,line:%d (not found)\n", __func__, __LINE__);
    return default_val;
}

/*
 * function: mipi_clk_and_pad_setting
 * params: struct ak_fb *fb_dev
 */
static void mipi_clk_and_pad_setting(struct ak_fb *fb_dev)
{
    u32 regval;
    struct lcdc_dsi_info *dsi_info = fb_dev->dsi_info;

    /* lane pad swap setting 0x6a100000, check ok */
    regval = __raw_readl(AK_LCD_DSI_LANE_SWAP);
    regval &= ~((0x7<<17)|(0x7<<20)|(0x7<<23)|(0x7<<26)|(0x7UL<<29));
    regval |= (((dsi_info->txd0)
        |(dsi_info->txd1<<3)
        |(dsi_info->txd2<<6)
        |(dsi_info->txd3<<9)
        |(dsi_info->txd4<<12))<<17);
    __raw_writel(regval, AK_LCD_DSI_LANE_SWAP);

    /* enable mipi dsi clk 0xf000b, */
    regval = __raw_readl(AK_CLK_GATE_CTRL0);
    regval &= ~(0x1UL<<31);
    __raw_writel(regval, AK_CLK_GATE_CTRL0);

    /* reset mipi dsi controller clk */
    regval = __raw_readl(AK_SW_RESET_CTRL0);
    regval |= (0x1UL<<31);
    __raw_writel(regval, AK_SW_RESET_CTRL0);
    mdelay(1);
    regval = __raw_readl(AK_SW_RESET_CTRL0);
    regval &= ~(0x1UL<<31);
    __raw_writel(regval, AK_SW_RESET_CTRL0);
    mdelay(10);

    /* reset pclk and hold pclk 0x4000c700, k 0x4008c702 check ok */
    regval = __raw_readl(AK_LCD_PCLK_CTRL);
    regval |= (0x1<<30);
    __raw_writel(regval, AK_LCD_PCLK_CTRL);

    /*
     * DSI Controller & DPHY clock gate enable 0x40000000,
     * k 0x400000b8 check ok 
     */
    regval = __raw_readl(AK_LCD_DSI_CLK_CFG);
    regval &= ~(0x1UL<<31);
    __raw_writel(regval, AK_LCD_DSI_CLK_CFG);

    /* DSI DPHY Reset, check ok  */
    regval = __raw_readl(AK_LCD_DSI_CLK_CFG);
    regval &= ~(0x1<<30);
    __raw_writel(regval, AK_LCD_DSI_CLK_CFG);
#if 0
    mdelay(1);
    regval = __raw_readl(AK_LCD_DSI_CLK_CFG);
    regval |= (0x1<<30);
    __raw_writel(regval, AK_LCD_DSI_CLK_CFG);
#endif

    /*
     * DSI Controller TxByteClkHS,
     * DSI Controller & dphy TxClkEsc reset, check ok
     */
    regval = __raw_readl(AK_LCD_DSI_CLK_CFG);
    regval |= (0x3<<28);
    __raw_writel(regval, AK_LCD_DSI_CLK_CFG);
    mdelay(1);
    regval = __raw_readl(AK_LCD_DSI_CLK_CFG);
    regval &= ~(0x3<<28);
    __raw_writel(regval, AK_LCD_DSI_CLK_CFG);

#if 0
    /* check if the DPHY is ready, check ok   */
    do {
    } while (!(__raw_readl(fb_dev->lcdc_reg + AK_DPI_STATUS_REG) & (0x1<<2)));
#endif


    AKFB_DEBUG("%s, reg:0x%x, val:0x%x\n", __func__, AK_LCD_DSI_LANE_SWAP,
        __raw_readl(AK_LCD_DSI_LANE_SWAP));
    AKFB_DEBUG("%s, reg:0x%x, val:0x%x\n", __func__, AK_CLK_GATE_SF_RESET_CTRL,
        __raw_readl(AK_CLK_GATE_SF_RESET_CTRL));
    AKFB_DEBUG("%s, reg:0x%x, val:0x%x\n", __func__, AK_CLK_GATE_SF_RESET_CTRL,
        __raw_readl(AK_CLK_GATE_SF_RESET_CTRL));
    AKFB_DEBUG("%s, reg:0x%x, val:0x%x\n", __func__, AK_LCD_PCLK_CTRL,
        __raw_readl(AK_LCD_PCLK_CTRL));
    AKFB_DEBUG("%s, reg:0x%x, val:0x%x\n", __func__, AK_LCD_DSI_CLK_CFG,
        __raw_readl(AK_LCD_DSI_CLK_CFG));

}/* end of func */

/* 
 * check ok
 */
static void mipi_dsi_cfg(struct ak_fb *fb_dev)
{
    struct lcdc_dsi_info *dsi_info = fb_dev->dsi_info;

    /* mipi dsi config reg setting 3, check ok */
    __raw_writel(dsi_info->num_lane, fb_dev->dsi_reg + AK_DSI_NUM_LANES);

    /* mipi dsi config reg setting 0 , check ok */
    __raw_writel(dsi_info->noncontinuous_clk,
        fb_dev->dsi_reg + AK_DSI_NONCONTINUOUS_CLK);

    /* mipi dsi config reg setting 1 , check ok */
    __raw_writel(dsi_info->t_pre, fb_dev->dsi_reg + AK_DSI_T_PRE);

    /* mipi dsi config reg setting 1 , check ok */
    __raw_writel(dsi_info->t_post, fb_dev->dsi_reg + AK_DSI_T_POST);

    /* mipi dsi config reg setting 1 , check ok */
    __raw_writel(dsi_info->tx_gap, fb_dev->dsi_reg + AK_DSI_TX_GAP);

    /* mipi dsi config reg setting 0 , check ok */
    __raw_writel(dsi_info->autoinsert_eotp,
        fb_dev->dsi_reg + AK_DSI_AUTOINSERT_EOTP);

    /* mipi dsi config reg setting 0xFFFFFF , check ok */
    __raw_writel(dsi_info->htx_to_count,
        fb_dev->dsi_reg + AK_DSI_HTX_TO_COUNT);

    /* mipi dsi config reg setting 0xFFFFFF , check ok */
    __raw_writel(dsi_info->lrx_to_count,
        fb_dev->dsi_reg + AK_DSI_LRX_H_TO_COUNT);

    /* mipi dsi config reg setting 0xFFFFFF , check ok */
    __raw_writel(dsi_info->bta_to_count,
        fb_dev->dsi_reg + AK_DSI_BTA_H_TO_COUNT);

    /* mipi dsi config reg setting 0xc8 , check ok */
    __raw_writel(dsi_info->t_wakeup,
        fb_dev->dsi_reg + AK_DSI_T_WAKEUP);

    /* mipi dsi config reg setting 0x320 , check ok */
    __raw_writel(dsi_info->pix_payload_size,
        fb_dev->dsi_reg + AK_DSI_PIX_PAYPLOAD_SIZE);

    /* mipi dsi config reg setting 0x200 , check ok */
    __raw_writel(dsi_info->pix_fifo_level,
        fb_dev->dsi_reg + AK_DSI_PIX_FIFO_SEND_LEVEL);

    /* mipi dsi config reg setting 0x5 , check ok */
    __raw_writel(dsi_info->if_color_coding,
        fb_dev->dsi_reg + AK_DSI_IF_COLOR_CODING);

    /* mipi dsi config reg setting 0x3 , check ok */
    __raw_writel(dsi_info->pix_format, fb_dev->dsi_reg + AK_DSI_PIX_FORMAT);

    /* mipi dsi config reg setting 0x0 , check ok */
    __raw_writel(dsi_info->vsync_pol, fb_dev->dsi_reg + AK_DSI_VSYNC_POL);

    /* mipi dsi config reg setting 0x0 , check ok */
    __raw_writel(dsi_info->hsync_pol, fb_dev->dsi_reg + AK_DSI_HSYNC_POL);

    /* mipi dsi config reg setting 0x2 , check ok */
    __raw_writel(dsi_info->video_mode, fb_dev->dsi_reg + AK_DSI_VIDEO_MODE);

    /* re-cacult hfp, hbp, hsa */
    /* mipi dsi config reg setting 0xB5 , check ok */
    __raw_writel(dsi_info->hfp, fb_dev->dsi_reg + AK_DSI_HFP);

    /* mipi dsi config reg setting 0x5A , check ok */
    __raw_writel(dsi_info->hbp, fb_dev->dsi_reg + AK_DSI_HBP);

    /* mipi dsi config reg setting 0x5A , check ok */
    __raw_writel(dsi_info->hsa, fb_dev->dsi_reg + AK_DSI_HSA);

    /* mipi dsi config reg setting 0x0 , check ok */
    __raw_writel(dsi_info->mult_pkts_en, fb_dev->dsi_reg + AK_DSI_MULT_PKTS_EN);

    /* re-cacult vbp, vfp */
    /* mipi dsi config reg setting 0x8 , check ok */
    __raw_writel(dsi_info->vbp, fb_dev->dsi_reg + AK_DSI_VBP);

    /* mipi dsi config reg setting 0x8 , check ok */
    __raw_writel(dsi_info->vfp, fb_dev->dsi_reg + AK_DSI_VFP);

    /* mipi dsi config reg setting 0x1 , check ok */
    __raw_writel(dsi_info->bllp_mode, fb_dev->dsi_reg + AK_DSI_BLLP_MODE);

    /* mipi dsi config reg setting 0x0 , check ok */
    __raw_writel(dsi_info->use_null_pkt_bllp,
        fb_dev->dsi_reg + AK_DSI_USE_NULL_PKT_BLLP);

    /* mipi dsi config reg setting 0x500 , check ok */
    __raw_writel(dsi_info->vactive, fb_dev->dsi_reg + AK_DSI_VACTIVE);

    /* mipi dsi config reg setting 0x0 , check ok */
    __raw_writel(dsi_info->vc, fb_dev->dsi_reg + AK_DSI_VC);

    AKFB_DEBUG("%s, reg:0x%x, val:0x%x\n", __func__, AK_DSI_NUM_LANES,
        __raw_readl(fb_dev->dsi_reg + AK_DSI_NUM_LANES));
    AKFB_DEBUG("%s, reg:0x%x, val:0x%x\n", __func__, AK_DSI_NONCONTINUOUS_CLK,
        __raw_readl(fb_dev->dsi_reg + AK_DSI_NONCONTINUOUS_CLK));
    AKFB_DEBUG("%s, reg:0x%x, val:0x%x\n", __func__, AK_DSI_T_PRE,
        __raw_readl(fb_dev->dsi_reg + AK_DSI_T_PRE));
    AKFB_DEBUG("%s, reg:0x%x, val:0x%x\n", __func__, AK_DSI_T_POST,
        __raw_readl(fb_dev->dsi_reg + AK_DSI_T_POST));
    AKFB_DEBUG("%s, reg:0x%x, val:0x%x\n", __func__, AK_DSI_TX_GAP,
        __raw_readl(fb_dev->dsi_reg + AK_DSI_TX_GAP));
    AKFB_DEBUG("%s, reg:0x%x, val:0x%x\n", __func__, AK_DSI_AUTOINSERT_EOTP,
        __raw_readl(fb_dev->dsi_reg + AK_DSI_AUTOINSERT_EOTP));
    AKFB_DEBUG("%s, reg:0x%x, val:0x%x\n", __func__, AK_DSI_HTX_TO_COUNT,
        __raw_readl(fb_dev->dsi_reg + AK_DSI_HTX_TO_COUNT));
    AKFB_DEBUG("%s, reg:0x%x, val:0x%x\n", __func__, AK_DSI_LRX_H_TO_COUNT,
        __raw_readl(fb_dev->dsi_reg + AK_DSI_LRX_H_TO_COUNT));
    AKFB_DEBUG("%s, reg:0x%x, val:0x%x\n", __func__, AK_DSI_BTA_H_TO_COUNT,
        __raw_readl(fb_dev->dsi_reg + AK_DSI_BTA_H_TO_COUNT));
    AKFB_DEBUG("%s, reg:0x%x, val:0x%x\n", __func__, AK_DSI_T_WAKEUP,
        __raw_readl(fb_dev->dsi_reg + AK_DSI_T_WAKEUP));
    AKFB_DEBUG("%s, reg:0x%x, val:0x%x\n", __func__, AK_DSI_PIX_PAYPLOAD_SIZE,
        __raw_readl(fb_dev->dsi_reg + AK_DSI_PIX_PAYPLOAD_SIZE));
    AKFB_DEBUG("%s, reg:0x%x, val:0x%x\n", __func__, AK_DSI_PIX_FIFO_SEND_LEVEL,
        __raw_readl(fb_dev->dsi_reg + AK_DSI_PIX_FIFO_SEND_LEVEL));
    AKFB_DEBUG("%s, reg:0x%x, val:0x%x\n", __func__, AK_DSI_IF_COLOR_CODING,
        __raw_readl(fb_dev->dsi_reg + AK_DSI_IF_COLOR_CODING));
    AKFB_DEBUG("%s, reg:0x%x, val:0x%x\n", __func__, AK_DSI_PIX_FORMAT,
        __raw_readl(fb_dev->dsi_reg + AK_DSI_PIX_FORMAT));
    AKFB_DEBUG("%s, reg:0x%x, val:0x%x\n", __func__, AK_DSI_VSYNC_POL,
        __raw_readl(fb_dev->dsi_reg + AK_DSI_VSYNC_POL));
    AKFB_DEBUG("%s, reg:0x%x, val:0x%x\n", __func__, AK_DSI_HSYNC_POL,
        __raw_readl(fb_dev->dsi_reg + AK_DSI_HSYNC_POL));
    AKFB_DEBUG("%s, reg:0x%x, val:0x%x\n", __func__, AK_DSI_VIDEO_MODE,
        __raw_readl(fb_dev->dsi_reg + AK_DSI_VIDEO_MODE));
    AKFB_DEBUG("%s, reg:0x%x, val:0x%x\n", __func__, AK_DSI_HFP,
        __raw_readl(fb_dev->dsi_reg + AK_DSI_HFP));
    AKFB_DEBUG("%s, reg:0x%x, val:0x%x\n", __func__, AK_DSI_HBP,
        __raw_readl(fb_dev->dsi_reg + AK_DSI_HBP));
    AKFB_DEBUG("%s, reg:0x%x, val:0x%x\n", __func__, AK_DSI_HSA,
        __raw_readl(fb_dev->dsi_reg + AK_DSI_HSA));
    AKFB_DEBUG("%s, reg:0x%x, val:0x%x\n", __func__, AK_DSI_MULT_PKTS_EN,
        __raw_readl(fb_dev->dsi_reg + AK_DSI_MULT_PKTS_EN));
    AKFB_DEBUG("%s, reg:0x%x, val:0x%x\n", __func__, AK_DSI_VBP,
        __raw_readl(fb_dev->dsi_reg + AK_DSI_VBP));
    AKFB_DEBUG("%s, reg:0x%x, val:0x%x\n", __func__, AK_DSI_VFP,
        __raw_readl(fb_dev->dsi_reg + AK_DSI_VFP));
    AKFB_DEBUG("%s, reg:0x%x, val:0x%x\n", __func__, AK_DSI_BLLP_MODE,
        __raw_readl(fb_dev->dsi_reg + AK_DSI_BLLP_MODE));
    AKFB_DEBUG("%s, reg:0x%x, val:0x%x\n", __func__, AK_DSI_USE_NULL_PKT_BLLP,
        __raw_readl(fb_dev->dsi_reg + AK_DSI_USE_NULL_PKT_BLLP));
    AKFB_DEBUG("%s, reg:0x%x, val:0x%x\n", __func__, AK_DSI_VACTIVE,
        __raw_readl(fb_dev->dsi_reg + AK_DSI_VACTIVE));
    AKFB_DEBUG("%s, reg:0x%x, val:0x%x\n", __func__, AK_DSI_VC,
        __raw_readl(fb_dev->dsi_reg + AK_DSI_VC));

}/* end of func */


/*
 * function: mipi_panel_cmd_setting
 * param: struct ak_fb *fb_dev
 */
static void mipi_panel_cmd_setting(struct ak_fb *fb_dev)
{
    int num_inits;
    u32 config,delay = 0;
    u32 payload_type = 0;
    u32 short_data;
    u8 data_type = 0,remainder,quotient;
    u8 long_packet[256];
    u8 short_packet[2];
    u8 *p;
    u32 regval = 0;
    int i,j;
    int len = 0;
    unsigned int node;
    const void *blob = gd->fdt_blob;

    node = fb_dev->lcd_node;
    fdt_getprop(blob, node, "panel-init-list", &len);
    num_inits = len ? (len / 4) : 0;
    AKFB_DEBUG("%s, len:%d, num_inits:%d\n", __func__, len, num_inits);

    /* check if the DPHY is ready , check ok */
    do {
    } while (!(__raw_readl(fb_dev->lcdc_reg + AK_DPI_STATUS_REG) & (1<<2)));

    for(i = 0; i < num_inits; ) {
        for(j = 0;; j++) {
            config = fdtdec_get_valoffset_int(blob, node, "panel-init-list",
                i+j, 0);
            AKFB_DEBUG("index:i=%d, j=%d, (i+j)=%d, config:%x\n", i,j, i+j,
                config);

            if (j == 0)
                /* delay parameter */
                delay = config;
            else if (j == 1) {
                /* data type */
                data_type = (u8) config;
                AKFB_DEBUG("data_type = %x\n", data_type);
                /* 
                 * 0x03 Generic Short WRITE, no parameters Short
                 * 0x13  Generic Short WRITE, 1 parameter Short
                 * 0x23 Generic Short WRITE, 2 parameters Short
                 * 0x05 DCS Short WRITE, no parameters Short
                 * 0x15 DCS Short WRITE, 1 parameter Short
                 * 0x29 Generic Long Write Long
                 * 0x39 DCS Long Write/write_LUT Command Packet Long
                 */
                if (data_type == 0x03 || data_type == 0x13 || data_type == 0x23
                        || data_type == 0x05 || data_type == 0x15) {
                    /* configure short packet cmd */
                    payload_type = 1;
                    // short packet cmd
                    regval = fb_dev->dsi_info->cmd_type|(0x1<<1);
                } else if (data_type == 0x29 || data_type == 0x39) {
                    /* configure long packet cmd  */
                    payload_type = 0;
                    // long packet cmd
                    regval = fb_dev->dsi_info->cmd_type|(0x0<<1);
                }
                __raw_writel(regval, fb_dev->lcdc_reg + AK_DPI_CONFIG_REG);
            }else {
                /*zzp, 2020/5/27 translating to char(1 bytes) here. */
                if (config != 0xFFF) {
                    if (payload_type == 1)
                        short_packet[j - 2] = (unsigned char)config;

                    if (payload_type == 0)
                        long_packet[j - 2] = (unsigned char)config;
                }

                /*zzp, 2020/5/27 start to send packet. */
                if(config == 0xFFF){
                    /* sending short packet */
                    if (payload_type == 1) {
                        regval = (data_type|(fb_dev->dsi_info->vc<<6)|
                            (short_packet[0]<<8)|(short_packet[1]<<16));
                        __raw_writel(regval,
                            fb_dev->lcdc_reg + AK_PACKET_HEAD_REG);

                        /* check if the current packet tx done */
                        do {
                        } while (!(__raw_readl(fb_dev->lcdc_reg + 
                            AK_DPI_STATUS_REG) & 0x1));
                        mdelay(delay);
                    }

                    /* sending long packet  */
                    if (payload_type == 0) {
                        p = long_packet;
                        /*
                         * write long packet data to payload register.
                         * 4bytes every time.
                         */
                        for(quotient=0;quotient<(j-2)/4;quotient++){
                            short_data = *(unsigned long *)p;
                            p += 4;
                            __raw_writel(short_data, fb_dev->lcdc_reg +
                                AK_TX_PAYLOAD_REG);
                        }

                        remainder = (j-2)%4;
                        if(remainder !=0){
                            short_data = 0;
                            memcpy(&short_data,p,remainder);
                            __raw_writel(short_data, fb_dev->lcdc_reg +
                                AK_TX_PAYLOAD_REG);
                        }

                        regval = (data_type|(fb_dev->dsi_info->vc<<6)|
                            ((j-2)<<8));
                        __raw_writel(regval, fb_dev->lcdc_reg +
                            AK_PACKET_HEAD_REG);
                        /* check if the current packet tx done */
                        do {
                        } while (!(__raw_readl(fb_dev->lcdc_reg +
                            AK_DPI_STATUS_REG) & 0x1));
                        mdelay(delay);
                    }
                }
            }
            if(config == 0xFFF)
                break;
        }
        i += (j + 1);
    }
    AKFB_DEBUG("panel-init-list end...\n");
}/* end of func */

/*
 * @BRIEF       config rgb panel info
 * @PARAM[in]   struct ak_fb *fb_dev
 * @RETURN          void
 * @RETVAL:
 */
static void rgb_panel_info_cfg(struct ak_fb *fb_dev)
{
    struct lcdc_panel_info *panel_info = fb_dev->panel_info;
    struct lcdc_rgb_info *rgb_info = fb_dev->rgb_info;
    u32 regval;
    u32 thlen, tvlen;

    /* top register config and disable rgb and osd channel */
    regval = __raw_readl(fb_dev->lcdc_reg + AK_LCD_TOP_CONFIG);
    regval &= ~((0x1)|(0xF<<3)|(0x1<<11)|(0x3FF<<14)|(0xFFUL<<24));
    regval|= (((!panel_info->pclk_pol)<<4)|(0x2<<5)
        |((!panel_info->rgb_seq)<<11)
        |(rgb_info->alarm_full<<14)
        |(rgb_info->alarm_empty<<24));
    __raw_writel(regval, fb_dev->lcdc_reg + AK_LCD_TOP_CONFIG);

    /* lcd rgb general info parameter config */
    regval = __raw_readl(fb_dev->lcdc_reg + AK_LCD_RGB_GEN_INFO);
    regval = ((panel_info->vogate_pol)
        |(panel_info->vsync_pol<<1)
        |(panel_info->hsync_pol<<2)
        |(rgb_info->neg_blank_level<<3)
        |(rgb_info->pos_blank_level<<11)
        |(rgb_info->blank_sel<<19)
        |(rgb_info->dma_mode<<20)
        |(panel_info->bus_width<<21)
        |(rgb_info->tpg_mode<<23)
        |(rgb_info->rgb_even_seq<<24)
        |(rgb_info->rgb_odd_seq<<27)
        |(rgb_info->rgb_seq_sel<<30)
        |(rgb_info->dummy_seq<<31));
    __raw_writel(regval, fb_dev->lcdc_reg + AK_LCD_RGB_GEN_INFO);

    /*disable vpage function*/
    regval = __raw_readl(fb_dev->lcdc_reg + AK_LCD_RGB_CTRL);
    regval &= ~(0x1<<30);
    __raw_writel(regval, fb_dev->lcdc_reg + AK_LCD_RGB_CTRL);


    /* set rgb bg color */
    regval = __raw_readl(fb_dev->lcdc_reg + AK_RGB_BACKGROUND);
    regval &= ~(0xFFFFFF);
    regval |= (fb_dev->rgb_info->bg_color);
    __raw_writel(regval, fb_dev->lcdc_reg + AK_RGB_BACKGROUND);


    /* rgb timing setting */
    regval = __raw_readl(fb_dev->lcdc_reg + AK_RGB_INTERFACE1);
    regval &= ~(0xFFFFFF);
    regval |= ((panel_info->tvpw)
        |(panel_info->thpw<<12));
    __raw_writel(regval, fb_dev->lcdc_reg + AK_RGB_INTERFACE1);

    regval = __raw_readl(fb_dev->lcdc_reg + AK_RGB_INTERFACE2);
    regval &= ~(0xFFFFFF);
    regval |= ((panel_info->thd)
        |(panel_info->thb<<12));
    __raw_writel(regval, fb_dev->lcdc_reg + AK_RGB_INTERFACE2);

    thlen = panel_info->thpw
        + panel_info->thb
        + panel_info->thd
        + panel_info->thf;
    regval = __raw_readl(fb_dev->lcdc_reg + AK_RGB_INTERFACE3);
    regval &= ~(0xFFFFFF);
    regval |= ((thlen)
        |(panel_info->thf<<13));
    __raw_writel(regval, fb_dev->lcdc_reg + AK_RGB_INTERFACE3);

    regval = __raw_readl(fb_dev->lcdc_reg + AK_RGB_INTERFACE4);
    regval &= ~(0xFFF);
    regval |= (panel_info->tvb);
    __raw_writel(regval, fb_dev->lcdc_reg + AK_RGB_INTERFACE4);

    regval = __raw_readl(fb_dev->lcdc_reg + AK_RGB_INTERFACE5);
    regval &= ~(0xFFF);
    regval |= (panel_info->tvf);
    __raw_writel(regval, fb_dev->lcdc_reg + AK_RGB_INTERFACE5);

    regval = __raw_readl(fb_dev->lcdc_reg + AK_RGB_INTERFACE6);
    regval &= ~((0x1FFF)|(0x1FFF<<14));
    regval |= ((rgb_info->v_unit)
        |(rgb_info->vh_delay_cycle<<1)
        |(rgb_info->vh_delay_en<<14)
        |(panel_info->tvd<<15));
    __raw_writel(regval, fb_dev->lcdc_reg + AK_RGB_INTERFACE6);

    if(rgb_info->v_unit == 0) {
        tvlen = panel_info->tvpw
            + panel_info->tvb
            + panel_info->tvd
            + panel_info->tvf;
    } else {
        tvlen = panel_info->tvb
            + panel_info->tvd
            + panel_info->tvf;
    }
    regval = __raw_readl(fb_dev->lcdc_reg + AK_RGB_INTERFACE7);
    regval &= ~(0x1FFF);
    regval |= (tvlen);
    __raw_writel(regval, fb_dev->lcdc_reg + AK_RGB_INTERFACE7);

    /* set panel size */
    regval = __raw_readl(fb_dev->lcdc_reg + AK_PANEL_SIZE);
    regval &= ~(0x3FFFFF);
    regval |= ((panel_info->height)
        |(panel_info->width<<11));
    __raw_writel(regval, fb_dev->lcdc_reg + AK_PANEL_SIZE);

    /* lcd sw ctrl settings */
    rgb_info->alert_line = panel_info->height - rgb_info->alert_line;
    regval = __raw_readl(fb_dev->lcdc_reg + AK_LCD_SW_CTRL);
    regval &= ~((0xFFF));
    regval |= ((rgb_info->alert_line)|(0x1<<11));
    __raw_writel(regval, fb_dev->lcdc_reg + AK_LCD_SW_CTRL);

    /* pclk domain reset, div setting and enable */
    lcdc_set_reg(1, AK_LCD_PCLK_CTRL, 30, 30);
    lcdc_set_reg(0, AK_LCD_PCLK_CTRL, 30, 30);

    regval = __raw_readl(fb_dev->lcdc_reg + AK_LCD_PCLK);
    regval &= ~(0x1FF);
    regval |= (0x1)|(panel_info->pclk_div<<1)|(0x1<<8);
    __raw_writel(regval, fb_dev->lcdc_reg + AK_LCD_PCLK);

#ifdef CONFIG_37_E_CODE
    if (fb_dev->power_pin > 0) {
                AKFB_DEBUG("%s, line:%d\n", __func__, __LINE__);
                gpio_direction_output(fb_dev->power_pin, 0);
    }
#endif

    /*
    * LCD backlight control
    */
    if (fb_dev->backlight_pin > 0) {
        /* enable open backlight, output high */
        gpio_direction_output(fb_dev->backlight_pin, 1);
        mdelay(1);
    }

    AKFB_DEBUG("%s, reg:0x%x, val:0x%x\n", __func__, AK_LCD_TOP_CONFIG,
        __raw_readl(fb_dev->lcdc_reg + AK_LCD_TOP_CONFIG));
    AKFB_DEBUG("%s, reg:0x%x, val:0x%x\n", __func__, AK_LCD_RGB_GEN_INFO,
        __raw_readl(fb_dev->lcdc_reg + AK_LCD_RGB_GEN_INFO));
    AKFB_DEBUG("%s, reg:0x%x, val:0x%x\n", __func__, AK_LCD_RGB_CTRL,
        __raw_readl(fb_dev->lcdc_reg + AK_LCD_RGB_CTRL));
    AKFB_DEBUG("%s, reg:0x%x, val:0x%x\n", __func__, AK_RGB_BACKGROUND,
        __raw_readl(fb_dev->lcdc_reg + AK_RGB_BACKGROUND));
    AKFB_DEBUG("%s, reg:0x%x, val:0x%x\n", __func__, AK_RGB_INTERFACE1,
        __raw_readl(fb_dev->lcdc_reg + AK_RGB_INTERFACE1));
    AKFB_DEBUG("%s, reg:0x%x, val:0x%x\n", __func__, AK_RGB_INTERFACE2,
        __raw_readl(fb_dev->lcdc_reg + AK_RGB_INTERFACE2));
    AKFB_DEBUG("%s, reg:0x%x, val:0x%x\n", __func__, AK_RGB_INTERFACE3,
        __raw_readl(fb_dev->lcdc_reg + AK_RGB_INTERFACE3));
    AKFB_DEBUG("%s, reg:0x%x, val:0x%x\n", __func__, AK_RGB_INTERFACE4,
        __raw_readl(fb_dev->lcdc_reg + AK_RGB_INTERFACE4));
    AKFB_DEBUG("%s, reg:0x%x, val:0x%x\n", __func__, AK_RGB_INTERFACE5,
        __raw_readl(fb_dev->lcdc_reg + AK_RGB_INTERFACE5));
    AKFB_DEBUG("%s, reg:0x%x, val:0x%x\n", __func__, AK_RGB_INTERFACE6,
        __raw_readl(fb_dev->lcdc_reg + AK_RGB_INTERFACE6));
    AKFB_DEBUG("%s, reg:0x%x, val:0x%x\n", __func__, AK_RGB_INTERFACE7,
        __raw_readl(fb_dev->lcdc_reg +AK_RGB_INTERFACE7));
    AKFB_DEBUG("%s, reg:0x%x, val:0x%x\n", __func__, AK_PANEL_SIZE,
        __raw_readl(fb_dev->lcdc_reg + AK_PANEL_SIZE));
    AKFB_DEBUG("%s, reg:0x%x, val:0x%x\n", __func__, AK_LCD_SW_CTRL,
        __raw_readl(fb_dev->lcdc_reg + AK_LCD_SW_CTRL));
    AKFB_DEBUG("%s, reg:0x%x, val:0x%x\n", __func__, AK_LCD_PCLK_CTRL,
        __raw_readl(fb_dev->lcdc_reg + AK_LCD_PCLK_CTRL));
    AKFB_DEBUG("%s, reg:0x%x, val:0x%x\n", __func__, AK_LCD_PCLK,
        __raw_readl(fb_dev->lcdc_reg +AK_LCD_PCLK));
}/* end of func */


/*
 * @BRIEF       get asic pll clk
 * asic_pll_clk = 24 * M/(N*2^OD)
 * @PARAM[in]: void
 * @RETURN: unsigned long
 * @RETVAL: asic_pll_clk
 */
unsigned long ak_get_asic_pll_clk(void)
{
    u32 pll_m, pll_n, pll_od;
    u32 asic_pll_clk;
    u32 regval;

    regval = __raw_readl(CLK_ASIC_PLL_CTRL);
    pll_od = (regval >> 12) & 0x3;
    pll_n = (regval >> 8) & 0xF;
    pll_m = (regval >> 0) & 0xFF;

    asic_pll_clk = 24 * pll_m /(pll_n * (1 << pll_od));

    return asic_pll_clk;
}

/*
 * @BRIEF       init mipi panel
 * @PARAM[in]: struct ak_fb *fb_dev
 * @RETURN: void
 * @RETVAL:
 */
static void mipi_panel_init(struct ak_fb *fb_dev)
{
    struct lcdc_panel_info *panel_info = fb_dev->panel_info;
    struct lcdc_dsi_info *dsi_info = fb_dev->dsi_info;
    unsigned long peri_rate;
    u32 pclk,factor;
    u32 regval;

    pr_debug("%s, line:%d\n", __func__, __LINE__);

    gpio_direction_output(fb_dev->backlight_pin, 0);
    mdelay(20);
    gpio_direction_output(fb_dev->power_pin, 0);

    if (dsi_info->mult_pkts_en == 0){
        dsi_info->pix_payload_size = panel_info->width;
    }
    else if (dsi_info->mult_pkts_en == 1){
        dsi_info->pix_payload_size = panel_info->width/2;
    }
    dsi_info->vactive = panel_info->height;
    mipi_clk_and_pad_setting(fb_dev);

    if (dsi_info->if_color_coding <= 2)
        panel_info->bus_width = 1;
    else if (dsi_info->if_color_coding <= 4)
        panel_info->bus_width = 2;
    else if (dsi_info->if_color_coding <= 5)
        panel_info->bus_width = 3;
    else
        panel_info->bus_width = 3;
    AKFB_DEBUG("%s, line:%d, bus_width:%d\n", __func__, __LINE__,
        panel_info->bus_width);
#ifdef CONFIG_37_D_CODE
    peri_rate = get_peri_pll_freq();
    pr_err("peri_rate=%lu\n", peri_rate);
    if(peri_rate != 600) {
        peri_rate = 600;
        pr_err("err, peril pll is not 600 MHz!\n");
    }
#endif
#ifdef CONFIG_37_E_CODE
        peri_rate = ak_get_asic_pll_clk();
        pr_err("peri_rate=%lu\n", peri_rate);
        if(peri_rate != 500) {
            peri_rate = 500;
            pr_err("err, peril pll is not 500 MHz, set to 500 MHz\n");
        }
#endif
    AKFB_DEBUG("%s_%d,peri_rate =%lu, panel_info->pclk_div=%d\n", __func__,
        __LINE__,peri_rate, panel_info->pclk_div);
    pclk = peri_rate/(panel_info->pclk_div + 1);

    factor = (1000*8*pclk)/((dsi_info->num_lane+1)*dsi_info->dsi_clk);

    /* get RGB timing config from dts */
    panel_info->pclk_pol = 0;
    panel_info->vsync_pol = dsi_info->vsync_pol?0:1;
    panel_info->hsync_pol = dsi_info->hsync_pol?0:1;
    panel_info->thpw = dsi_info->hsa;
    panel_info->thb = dsi_info->hbp;
    panel_info->thd = panel_info->width;
    panel_info->thf = dsi_info->hfp;
    panel_info->tvpw = dsi_info->vsa;
    panel_info->tvb = dsi_info->vbp;
    panel_info->tvd = panel_info->height;
    panel_info->tvf = dsi_info->vfp;

    /* calcuate mipi dsi timing setting */
    dsi_info->hsa = panel_info->thpw*1000/factor;
    dsi_info->hbp = panel_info->thb*1000/factor;
    dsi_info->hfp = panel_info->thf*1000/factor;
    dsi_info->vbp = panel_info->tvb;
    dsi_info->vfp = panel_info->tvf;

    mipi_dsi_cfg(fb_dev);

    /* set mipi high speed clk u 0x40000092, k 0x400000b8 */
    regval = __raw_readl(AK_LCD_DSI_CLK_CFG);
    regval &= ~(0x1FF);
    if (dsi_info->dsi_clk <= 160)
        regval |= ((dsi_info->dsi_clk-80)*8/10); // 1.25*
    else if (dsi_info->dsi_clk <= 320)
        regval |= ((dsi_info->dsi_clk)*2/5); // 2.5*
    else
        regval |= ((dsi_info->dsi_clk)/5 + 64); // 5*
    __raw_writel(regval, AK_LCD_DSI_CLK_CFG);
    AKFB_DEBUG("mipi clk reg value :%x\n", regval);

    regval = __raw_readl(AK_LCD_DSI_CLK_CFG);
    regval |= (0x1<<30);
    __raw_writel(regval, AK_LCD_DSI_CLK_CFG);

    /* check if the DPHY is ready, check ok   */
    do {
    } while (!(__raw_readl(fb_dev->lcdc_reg + AK_DPI_STATUS_REG) & (0x1<<2)));

    /* reset mipi lcd panel */
    if (fb_dev->reset_pin > 0) {
        gpio_direction_output(fb_dev->reset_pin, 1);
        mdelay(1);
        gpio_direction_output(fb_dev->reset_pin, 0);
        mdelay(5);
        gpio_direction_output(fb_dev->reset_pin, 1);
        mdelay(25);
    }

    mipi_panel_cmd_setting(fb_dev);
    rgb_panel_info_cfg(fb_dev);

}/* end of func */

/*
 * @BRIEF       init rgb panel
 * @PARAM[in]: struct ak_fb *fb_dev
 * @RETURN: void
 * @RETVAL:
 */
static void rgb_panel_init(struct ak_fb *fb_dev)
{
    struct lcdc_panel_info *panel_info = fb_dev->panel_info;

    panel_info->thd = panel_info->width;
    panel_info->tvd = panel_info->height;

    /* reset rgb lcd panel */
    if (fb_dev->reset_pin > 0) {
        gpio_direction_output(fb_dev->reset_pin, 1);

        mdelay(1);
        gpio_direction_output(fb_dev->reset_pin, 0);
        mdelay(10);
        gpio_direction_output(fb_dev->reset_pin, 1);
        mdelay(10);
    }

    rgb_panel_info_cfg(fb_dev);
}

/*
 * @BRIEF       config lcd control info
 * @PARAM[in]: struct ak_fb *fb_dev
 * @RETURN: void
 * @RETVAL:
 */
static void lcdc_info_cfg(struct ak_fb *fb_dev)
{
    if(fb_dev->panel_info->rgb_if == DISP_IF_RGB){
        rgb_panel_init(fb_dev);
    }else if(fb_dev->panel_info->rgb_if == DISP_IF_DSI){
            mipi_panel_init(fb_dev);
    }else if(fb_dev->panel_info->rgb_if == DISP_IF_MPU){
        /* add special init for mpu panel */
        /* todo */
    }
}


/* 
 * set rgb display for logo display
 *
 * rgb channel config and rgb go
 */
static void lcdc_rgb_display(struct ak_fb *fb_dev)
{
    u32 regval;
    /*
     * input data format is determined by bit[8] and bit[13] of this register.
     * 00:input data format is 16bits (RGB565 or BGR565)
     * 01:input data format is 24bits (RGB888 or BGR888)
     * 10 or 11:input data format is 32bits (ARGB888 ABGR888 RGBA888 BGRA888)
     * bit[12] input data seq, 0:RGB,1:BGR
     * bit[7] inout a location, 0: A front, 1, A back
     */
    regval = __raw_readl(fb_dev->lcdc_reg + AK_LCD_TOP_CONFIG);
    regval &= ~((0x3<<7)|(0x3<<12));
    regval |= ((fb_dev->rgb_input->a_location<<7)
        |(fb_dev->rgb_input->fmt1<<8)
        |(fb_dev->rgb_input->rgb_seq<<12)
        |(fb_dev->rgb_input->fmt0<<13));
    __raw_writel(regval, fb_dev->lcdc_reg + AK_LCD_TOP_CONFIG);

    /* config RGB input size */
    regval = __raw_readl(fb_dev->lcdc_reg + AK_RGB_SIZE);
    regval &= ~(0x3FFFFF);
    regval|= ((fb_dev->rgb_input->height)
        |(fb_dev->rgb_input->width<<11));
    __raw_writel(regval, fb_dev->lcdc_reg + AK_RGB_SIZE);

    /* config RGB input offset */
    regval = __raw_readl(fb_dev->lcdc_reg + AK_RGB_OFFSET);
    regval &= ~(0x3FFFFF);
    regval|= ((fb_dev->rgb_input->v_offset)
        |(fb_dev->rgb_input->h_offset<<11));
    __raw_writel(regval, fb_dev->lcdc_reg + AK_RGB_OFFSET);

    /* config RGB dma address */
    ///fb_dev->panel_info->paddr = 0x03d02000;
    fb_dev->panel_info->paddr_offset = 0x0;
    regval = __raw_readl(fb_dev->lcdc_reg + AK_LCD_RGB_CTRL);
    regval &= ~(0x3FFFFFFF);
    regval|= ((fb_dev->panel_info->paddr + fb_dev->panel_info->paddr_offset)&
        0x3FFFFFFF);
    __raw_writel(regval, fb_dev->lcdc_reg + AK_LCD_RGB_CTRL);

    /* enable rgb channel */
    regval = __raw_readl(fb_dev->lcdc_reg + AK_LCD_TOP_CONFIG);
    regval |= (0x1<<3);
    __raw_writel(regval, fb_dev->lcdc_reg + AK_LCD_TOP_CONFIG);

    /* set sw ctrl en */
    regval = __raw_readl(fb_dev->lcdc_reg + AK_LCD_SW_CTRL);
    regval|= (0x1<<12);
    __raw_writel(regval, fb_dev->lcdc_reg + AK_LCD_SW_CTRL);

    /* rgb go */
    if (fb_dev->lcd_go == False)
    {
        if (fb_dev->panel_info->rgb_if == DISP_IF_MPU) {
            __raw_writel(0x1<<3, fb_dev->lcdc_reg + AK_LCD_GO);
            fb_dev->lcd_go = True;
        } else {
            __raw_writel(0x1<<2, fb_dev->lcdc_reg + AK_LCD_GO);
            fb_dev->lcd_go = True;
        }
    }
    AKFB_DEBUG("%s, reg:0x%x, val:0x%x\n", __func__, AK_LCD_TOP_CONFIG,
        __raw_readl(fb_dev->lcdc_reg + AK_LCD_TOP_CONFIG));
    AKFB_DEBUG("%s, reg:0x%x, val:0x%x\n", __func__, AK_RGB_SIZE,
        __raw_readl(fb_dev->lcdc_reg + AK_RGB_SIZE));
    AKFB_DEBUG("%s, reg:0x%x, val:0x%x\n", __func__, AK_RGB_OFFSET,
        __raw_readl(fb_dev->lcdc_reg + AK_RGB_OFFSET));
    AKFB_DEBUG("%s, reg:0x%x, val:0x%x\n", __func__, AK_LCD_RGB_CTRL,
        __raw_readl(fb_dev->lcdc_reg + AK_LCD_RGB_CTRL));
    AKFB_DEBUG("panel_info->width=%d\n",fb_dev->panel_info->width);
    AKFB_DEBUG("panel_info->height=%d\n",fb_dev->panel_info->height);
    AKFB_DEBUG("rgb_info->width=%d\n",fb_dev->rgb_input->width);
    AKFB_DEBUG("rgb_info->height=%d\n",fb_dev->rgb_input->height);
    AKFB_DEBUG("rgb_info->v_offset=%d\n",fb_dev->rgb_input->v_offset);
    AKFB_DEBUG("rgb_info->h_offset=%d\n",fb_dev->rgb_input->h_offset);
    AKFB_DEBUG("panel_info->paddr=0x%x, \n",fb_dev->panel_info->paddr);
    AKFB_DEBUG("panel_info->paddr_offset=0x%x, \n",
        fb_dev->panel_info->paddr_offset);
    AKFB_DEBUG("%s, reg:0x%x, val:0x%x\n", __func__, AK_LCD_TOP_CONFIG,
        __raw_readl(fb_dev->lcdc_reg + AK_LCD_TOP_CONFIG));
    AKFB_DEBUG("%s, reg:0x%x, val:0x%x\n", __func__, AK_LCD_SW_CTRL,
        __raw_readl(fb_dev->lcdc_reg + AK_LCD_SW_CTRL));
    AKFB_DEBUG("%s, reg:0x%x, val:0x%x\n", __func__, AK_LCD_GO,
        __raw_readl(fb_dev->lcdc_reg + AK_LCD_GO));

}/* end of func */


static void lcd_panel_on(struct ak_fb *fb_dev)
{
#if 0
#ifdef CONFIG_37_E_CODE
        /*
        * disable the backlight
        * open LCD panel power
        */
        gpio_direction_output(fb_dev->backlight_pin, 0);
        mdelay(1);
        if (fb_dev->power_pin > 0) {
            debug("%s, line:%d\n", __func__, __LINE__);
            gpio_direction_output(fb_dev->power_pin, 0);
        }
        mdelay(1);
#endif
#endif


    /* lcd logo display */
    lcdc_rgb_display(fb_dev);

#ifdef CONFIG_37_E_CODE
    /*
    * waiting for several frames reflush then enable LCD backlight
    */
    mdelay(95);
    gpio_direction_output(fb_dev->backlight_pin, 1);
#endif


}

#ifndef CONFIG_DM_VIDEO
/*
 * board_init_f(arch/arm/lib/board.c) calls lcd_setmem() which needs
 * panel_info.vl_col, panel_info.vl_row and panel_info.vl_bpix to reserve
 * FB memory at a very early stage, i.e even before
 * is called. So, we are forced to statically assign it.
 *
 * 10 inch lcd(1280x800), 24bit
 */
vidinfo_t panel_info  = {
    .vl_col = LCD_MAX_WIDTH,
    .vl_row = LCD_MAX_HEIGHT,
    .vl_width = LCD_MAX_WIDTH,
    .vl_height = LCD_MAX_HEIGHT,
    .vl_bpix = LCD_COLOR24,  // priv
};

/*
 * @BRIEF       init lcd control 
 * @PARAM[in]: void *lcdbase
 * @RETURN: void
 * @RETVAL:
 */
void lcd_ctrl_init(void *lcdbase)
{
    u32 regval;
    u32 ret = 0;
    struct lcdc_rgb_input_info *rgb_input;
    struct lcdc_rgb_info *rgb_info;
    struct lcdc_osd_info *osd_info;
    struct lcdc_panel_info *panel_info;

    /*
    * malloc fb some struct space
    */
    rgb_input = kzalloc(sizeof(struct lcdc_rgb_input_info), GFP_KERNEL);
    rgb_info = kzalloc(sizeof(struct lcdc_rgb_info), GFP_KERNEL);
    osd_info = kzalloc(sizeof(struct lcdc_osd_info), GFP_KERNEL);
    panel_info = kzalloc(sizeof(struct lcdc_panel_info), GFP_KERNEL);
    if ((!rgb_input)||(!rgb_info)||(!osd_info)||(!panel_info)) {
        pr_err("alloc lcdc info buffer error\n");
        ret = -ENOMEM;
        return ;
    }

    /*
    * malloc fb_dev struct space
    */
    fb_dev = malloc(sizeof(struct ak_fb));
    if(!fb_dev){
        pr_err("alloc lcdc info buffer2 error\n");
        ret = -ENOMEM;
        return ;
    }

    fb_dev->rgb_input = rgb_input;
    fb_dev->rgb_info = rgb_info;
    fb_dev->osd_info = osd_info;
    fb_dev->panel_info = panel_info;
    fb_dev->panel_info->paddr = (dma_addr_t)(lcdbase - 0x80000000);
    //pr_debug("panel_info->paddr=0x%x, \n",fb_dev->panel_info->paddr);

    /*
    * enable lcd controller clk
    */
    regval = __raw_readl(AK_CLK_GATE_CTRL0);
    regval &= ~(0x1<<22);
    __raw_writel(regval, AK_CLK_GATE_CTRL0);

    /*
    * reset lcd controller clk
    */
    regval = __raw_readl(AK_SW_RESET_CTRL0);
    regval |= (0x1<<22);
    __raw_writel(regval, AK_SW_RESET_CTRL0);
    mdelay(1);
    regval = __raw_readl(AK_SW_RESET_CTRL0);
    regval &= ~(0x1<<22);
    __raw_writel(regval, AK_SW_RESET_CTRL0);
    mdelay(10);

    /*
    * init fb dev fdt base address
    */
    fb_dev->fdt_blob = gd->fdt_blob;
    ret = ak_lcd_parameter_parse_dt1(gd->fdt_blob, fb_dev);
    if(ret){
        pr_err("err, get dtb and parse lcd parameter!\n");
        ret = -ENOMEM;
        return ;
    }

    lcdc_info_cfg(fb_dev);
}

/*
 * @BRIEF       enable lcd
 * @PARAM[in]: void
 * @RETURN: void
 * @RETVAL:
 */
void lcd_enable(void)
{
    /* copy logo file here */
    //memcpy(panel_info->vaddr, boot_logo, sizeof(boot_logo));
    fb_dev->fb_id = 0;
    fb_dev->panel_info->paddr_offset = 0;

    /* set the display offset */
    fb_dev->rgb_input->h_offset =
        (fb_dev->panel_info->width-fb_dev->rgb_input->width)/2;
    fb_dev->rgb_input->h_offset -= fb_dev->rgb_input->h_offset%2;
    fb_dev->rgb_input->v_offset =
        (fb_dev->panel_info->height-fb_dev->rgb_input->height)/2;
    fb_dev->rgb_input->v_offset -= fb_dev->rgb_input->v_offset%2;

    /* set rgb display for logo display */
    lcd_panel_on(fb_dev);
}

/* dummy function */
void lcd_setcolreg(ushort regno, ushort red, ushort green, ushort blue)
{
    return;
}

#else
enum {
    /* Maximum LCD size we support */
    LCD_MAX_WIDTH       = 1920,
    LCD_MAX_HEIGHT      = 1080,
    /* actually support 24bit,so set LCD_MAX_LOG2_BPP=VIDEO_BPP32*/
    LCD_MAX_LOG2_BPP    = VIDEO_BPP32,
};

/*
 * @BRIEF       enable lcd
 * @PARAM[in]: void
 * @RETURN: void
 * @RETVAL:
 */
void lcd_enable(struct ak_fb *fb_dev)
{
    /*  logo file  */
    fb_dev->fb_id = 0;
    fb_dev->panel_info->paddr_offset = 0;

    /* set the display offset */
    fb_dev->rgb_input->h_offset =
        (fb_dev->panel_info->width-fb_dev->rgb_input->width)/2;
    fb_dev->rgb_input->h_offset -= fb_dev->rgb_input->h_offset%2;
    fb_dev->rgb_input->v_offset =
        (fb_dev->panel_info->height-fb_dev->rgb_input->height)/2;
    fb_dev->rgb_input->v_offset -= fb_dev->rgb_input->v_offset%2;

    /* set rgb display for logo display */
    lcd_panel_on(fb_dev);
}

static int ak_fb_probe(struct udevice *dev)
{
    struct ak_fb *fb_dev = dev_get_priv(dev);
    int ret = 0;
    u32 regval;
    /*
    * enable lcd controller clk
    */
    regval = __raw_readl(AK_CLK_GATE_CTRL0);
    regval &= ~(0x1<<22);
    __raw_writel(regval, AK_CLK_GATE_CTRL0);

    /*
    * reset lcd controller clk
    */
    regval = __raw_readl(AK_SW_RESET_CTRL0);
    regval |= (0x1<<22);
    __raw_writel(regval, AK_SW_RESET_CTRL0);
    mdelay(1);
    regval = __raw_readl(AK_SW_RESET_CTRL0);
    regval &= ~(0x1<<22);
    __raw_writel(regval, AK_SW_RESET_CTRL0);
    mdelay(10);

    fb_dev->panel_info->paddr = (dma_addr_t)(gd->video_bottom - 0x80000000);

    AKFB_DEBUG("gd->video_bottom =0x%lx, gd->video_top=0x%lx , \
        gd->fb_base=0x%lx\n",
        gd->video_bottom, gd->video_top, gd->fb_base);
    AKFB_DEBUG("panel_info->paddr=0x%x, \n",fb_dev->panel_info->paddr);

    lcdc_info_cfg(fb_dev);

    lcd_enable(fb_dev);

    AKFB_DEBUG("ak_fb_probe return %d\n", ret);
    return ret;
}

/*
 * function: get lcdc rgb config from dts
 */
static void get_lcdc_rgb_cfg(struct ak_fb *fb_dev, const void *blob,
    int parent)
{
    fb_dev->rgb_input->width =
        fdtdec_get_int(blob, parent, "lcd-logo-width", 0);
    fb_dev->rgb_input->height =
        fdtdec_get_int(blob, parent, "lcd-logo-height", 0);
    fb_dev->rgb_input->fmt0 =
        fdtdec_get_int(blob, parent, "lcd-logo-fmt0", 0);
    fb_dev->rgb_input->fmt1 =
        fdtdec_get_int(blob, parent, "lcd-logo-fmt1", 0);
    fb_dev->rgb_input->rgb_seq =
        fdtdec_get_int(blob, parent, "lcd-logo-rgb-seq", 0);
    fb_dev->rgb_input->a_location =
        fdtdec_get_int(blob, parent, "lcd-logo-a-location", 0);
    fb_dev->rgb_info->alarm_empty =
        fdtdec_get_int(blob, parent, "lcd-alarm-empty", 0);
    fb_dev->rgb_info->alarm_full =
        fdtdec_get_int(blob, parent, "lcd-alarm-full", 0);
    fb_dev->rgb_info->dma_mode =
        fdtdec_get_int(blob, parent, "lcd-dma-mode", 0);
    fb_dev->rgb_info->dummy_seq =
        fdtdec_get_int(blob, parent, "lcd-dummy-seq", 0);
    fb_dev->rgb_info->rgb_seq_sel =
        fdtdec_get_int(blob, parent, "lcd-rgb-seq-sel", 0);
    fb_dev->rgb_info->rgb_odd_seq =
        fdtdec_get_int(blob, parent, "lcd-rgb-odd-seq", 0);
    fb_dev->rgb_info->rgb_even_seq =
        fdtdec_get_int(blob, parent, "lcd-rgb-even-seq", 0);
    fb_dev->rgb_info->tpg_mode =
        fdtdec_get_int(blob, parent, "lcd-rgb-tpg-sel", 0);
    fb_dev->rgb_info->vpage_en =
        fdtdec_get_int(blob, parent, "lcd-vpage-en", 0);
    fb_dev->rgb_info->blank_sel =
        fdtdec_get_int(blob, parent, "lcd-blank-sel", 0);
    fb_dev->rgb_info->pos_blank_level =
        fdtdec_get_int(blob, parent, "lcd-pos-blank-level", 0);
    fb_dev->rgb_info->neg_blank_level =
        fdtdec_get_int(blob, parent, "lcd-neg-blank-level", 0);
    fb_dev->rgb_info->bg_color =
        fdtdec_get_int(blob, parent, "lcd-rgb-bg-color", 0);
    fb_dev->rgb_info->vh_delay_en =
        fdtdec_get_int(blob, parent, "lcd-vh-delay-en", 0);
    fb_dev->rgb_info->vh_delay_cycle =
        fdtdec_get_int(blob, parent, "lcd-vh-delay-cycle", 0);
    fb_dev->rgb_info->v_unit =
        fdtdec_get_int(blob, parent, "lcd-v-unit", 0);
    fb_dev->rgb_info->alert_line =
        fdtdec_get_int(blob, parent, "lcd-alert-line", 0);

    AKFB_DEBUG("%s, line:%d, get device node OK, fb_node:%d!!!\n",
        __func__, __LINE__, parent);
    AKFB_DEBUG("%s, line:%d, get device reset pin:%d, OK!!!\n",
        __func__, __LINE__, fb_dev->reset_pin);
    AKFB_DEBUG("%s, line:%d, get device fb_type:%d, OK!!!\n",
        __func__, __LINE__, fb_dev->fb_type);
    AKFB_DEBUG("%s, line:%d, get device rgb_input->width:%d, OK!!!\n",
        __func__, __LINE__, fb_dev->rgb_input->width);
    AKFB_DEBUG("%s, line:%d, get device rgb_input->height:%d, OK!!!\n",
        __func__, __LINE__, fb_dev->rgb_input->height);
    AKFB_DEBUG("%s, line:%d, get device rgb_input->fmt0:%d, OK!!!\n",
        __func__, __LINE__, fb_dev->rgb_input->fmt0);
    AKFB_DEBUG("%s, line:%d, get device rgb_input->fmt1:%d, OK!!!\n",
        __func__, __LINE__, fb_dev->rgb_input->fmt1);
    AKFB_DEBUG("%s, line:%d, get device rgb_input->rgb_seq:%d, OK!!!\n",
        __func__, __LINE__, fb_dev->rgb_input->rgb_seq);
    AKFB_DEBUG("%s, line:%d, get device rgb_input->a_location:%d, OK!!!\n",
        __func__, __LINE__, fb_dev->rgb_input->a_location);
    AKFB_DEBUG("%s, line:%d, get device rgb_info->alarm_empty:%d, OK!!!\n",
        __func__, __LINE__, fb_dev->rgb_info->alarm_empty);
    AKFB_DEBUG("%s, line:%d, get device rgb_info->alarm_full:%d, OK!!!\n",
        __func__, __LINE__, fb_dev->rgb_info->alarm_full);
    AKFB_DEBUG("%s, line:%d, get device rgb_info->dma_mode:%d, OK!!!\n",
        __func__, __LINE__, fb_dev->rgb_info->dma_mode);
    AKFB_DEBUG("%s, line:%d, get device rgb_info->dummy_seq:%d, OK!!!\n",
        __func__, __LINE__, fb_dev->rgb_info->dummy_seq);
    AKFB_DEBUG("%s, line:%d, get device rgb_info->rgb_seq_sel:%d, OK!!!\n",
        __func__, __LINE__, fb_dev->rgb_info->rgb_seq_sel);
    AKFB_DEBUG("%s, line:%d, get device rgb_info->rgb_odd_seq:%d, OK!!!\n",
        __func__, __LINE__, fb_dev->rgb_info->rgb_odd_seq);
    AKFB_DEBUG("%s, line:%d, get device rgb_info->rgb_even_seq:%d, OK!!!\n",
        __func__, __LINE__, fb_dev->rgb_info->rgb_even_seq);
    AKFB_DEBUG("%s, line:%d, get device rgb_info->tpg_mode:%d, OK!!!\n",
        __func__, __LINE__, fb_dev->rgb_info->tpg_mode);
    AKFB_DEBUG("%s, line:%d, get device rgb_info->vpage_en:%d, OK!!!\n",
        __func__, __LINE__, fb_dev->rgb_info->vpage_en);
    AKFB_DEBUG("%s, line:%d, get device rgb_info->blank_sel:%d, OK!!!\n",
        __func__, __LINE__, fb_dev->rgb_info->blank_sel);
    AKFB_DEBUG("%s, line:%d, get device rgb_info->pos_blank_level:%d, OK!!!\n",
        __func__, __LINE__, fb_dev->rgb_info->pos_blank_level);
    AKFB_DEBUG("%s, line:%d, get device rgb_info->neg_blank_level:%d, OK!!!\n",
        __func__, __LINE__, fb_dev->rgb_info->neg_blank_level);
    AKFB_DEBUG("%s, line:%d, get device rgb_info->bg_color:%d, OK!!!\n",
        __func__, __LINE__, fb_dev->rgb_info->bg_color);
    AKFB_DEBUG("%s, line:%d, get device rgb_info->vh_delay_en:%d, OK!!!\n",
        __func__, __LINE__, fb_dev->rgb_info->vh_delay_en);
    AKFB_DEBUG("%s, line:%d, get device rgb_info->vh_delay_cycle:%d, OK!!!\n",
        __func__, __LINE__, fb_dev->rgb_info->vh_delay_cycle);
    AKFB_DEBUG("%s, line:%d, get device rgb_info->v_unit:%d, OK!!!\n",
        __func__, __LINE__, fb_dev->rgb_info->v_unit);
    AKFB_DEBUG("%s, line:%d, get device rgb_info->alert_line:%d, OK!!!\n",
        __func__, __LINE__, fb_dev->rgb_info->alert_line);
    AKFB_DEBUG("%s, line:%d, get panel_info->name:%s, OK!!!\n",
        __func__, __LINE__, fb_dev->panel_info->name);
    AKFB_DEBUG("%s, line:%d, get panel_info->rgb_if:%s, OK!!!\n",
        __func__, __LINE__, fb_dev->panel_info->rgb_if?"MIPI":"RGB");

}/* end of func */

/*
 * function: init rgb pannel timing from dts.
 */
static void init_rgb_panel_timing(struct ak_fb *fb_dev, const void *blob,
    int panel_node)
{
    fb_dev->panel_info->rgb_seq =
        fdtdec_get_int(blob, panel_node, "panel-rgb-seq", 0);
    fb_dev->panel_info->width =
        fdtdec_get_int(blob, panel_node, "panel-width", 0);
    fb_dev->panel_info->height =
        fdtdec_get_int(blob, panel_node, "panel-height", 0);
    fb_dev->panel_info->pclk_div =
        fdtdec_get_int(blob, panel_node, "panel-pclk-div", 0);
    fb_dev->panel_info->thpw =
        fdtdec_get_int(blob, panel_node, "panel-thpw", 0);
    fb_dev->panel_info->thb =
        fdtdec_get_int(blob, panel_node, "panel-thb", 0);
    fb_dev->panel_info->thf =
        fdtdec_get_int(blob, panel_node, "panel-thf", 0);
    fb_dev->panel_info->tvpw =
        fdtdec_get_int(blob, panel_node, "panel-tvpw", 0);
    fb_dev->panel_info->tvb =
        fdtdec_get_int(blob, panel_node, "panel-tvb", 0);
    fb_dev->panel_info->tvf =
        fdtdec_get_int(blob, panel_node, "panel-tvf", 0);
    fb_dev->panel_info->pclk_pol =
        fdtdec_get_int(blob, panel_node, "panel-pclk-pol", 0);
    fb_dev->panel_info->hsync_pol =
        fdtdec_get_int(blob, panel_node, "panel-hsync-pol", 0);
    fb_dev->panel_info->vsync_pol =
        fdtdec_get_int(blob, panel_node, "panel-vsync-pol", 0);
    fb_dev->panel_info->vogate_pol =
        fdtdec_get_int(blob, panel_node, "panel-vogate-pol", 0);
    fb_dev->panel_info->bus_width =
        fdtdec_get_int(blob, panel_node, "panel-bus-width", 0);
    AKFB_DEBUG("%s,line:%d, fdt_addr:0x%p\n", __func__, __LINE__, blob);
    AKFB_DEBUG("%s, line:%d, get panel_info->rgb_seq:%d, OK!!!\n",
        __func__, __LINE__, fb_dev->panel_info->rgb_seq);
    AKFB_DEBUG("%s, line:%d, get panel_info->width:%d, OK!!!\n",
        __func__, __LINE__, fb_dev->panel_info->width);
    AKFB_DEBUG("%s, line:%d, get panel_info->height:%d, OK!!!\n",
        __func__, __LINE__, fb_dev->panel_info->height);
    AKFB_DEBUG("%s, line:%d, get panel_info->pclk_div:%d, OK!!!\n",
        __func__, __LINE__, fb_dev->panel_info->pclk_div);
    AKFB_DEBUG("%s, line:%d, get panel_info->thpw:%d, OK!!!\n",
        __func__, __LINE__, fb_dev->panel_info->thpw);
    AKFB_DEBUG("%s, line:%d, get panel_info->thb:%d, OK!!!\n",
        __func__, __LINE__, fb_dev->panel_info->thb);
    AKFB_DEBUG("%s, line:%d, get panel_info->thf:%d, OK!!!\n",
        __func__, __LINE__, fb_dev->panel_info->thf);
    AKFB_DEBUG("%s, line:%d, get panel_info->tvpw:%d, OK!!!\n",
        __func__, __LINE__, fb_dev->panel_info->tvpw);
    AKFB_DEBUG("%s, line:%d, get panel_info->tvb:%d, OK!!!\n",
        __func__, __LINE__, fb_dev->panel_info->tvb);
    AKFB_DEBUG("%s, line:%d, get panel_info->tvf:%d, OK!!!\n",
        __func__, __LINE__, fb_dev->panel_info->tvf);
    AKFB_DEBUG("%s, line:%d, get panel_info->pclk_pol:%d, OK!!!\n",
        __func__, __LINE__, fb_dev->panel_info->pclk_pol);
    AKFB_DEBUG("%s, line:%d, get panel_info->hsync_pol:%d, OK!!!\n",
        __func__, __LINE__, fb_dev->panel_info->hsync_pol);
    AKFB_DEBUG("%s, line:%d, get panel_info->vsync_pol:%d, OK!!!\n",
        __func__, __LINE__, fb_dev->panel_info->vsync_pol);
    AKFB_DEBUG("%s, line:%d, get panel_info->vogate_pol:%d, OK!!!\n",
        __func__, __LINE__, fb_dev->panel_info->vogate_pol);
    AKFB_DEBUG("%s, line:%d, get panel_info->bus_width:%d, OK!!!\n",
        __func__, __LINE__, fb_dev->panel_info->bus_width);
}

/*
 * function: init mipi pannel timing from dts.
 */
static void init_mipi_panel_timing(struct ak_fb *fb_dev, const void *blob,
    int panel_node)
{
    fb_dev->panel_info->rgb_seq =
        fdtdec_get_int(blob, panel_node, "panel-rgb-seq", 0);
    fb_dev->panel_info->width =
        fdtdec_get_int(blob, panel_node, "panel-width", 0);
    fb_dev->panel_info->height =
        fdtdec_get_int(blob, panel_node, "panel-height", 0);
    fb_dev->panel_info->pclk_div =
        fdtdec_get_int(blob, panel_node, "panel-pclk-div", 0);
    fb_dev->dsi_info->num_lane =
        fdtdec_get_int(blob, panel_node, "panel-dsi-num-lane", 0);
    fb_dev->dsi_info->txd0 =
        fdtdec_get_int(blob, panel_node, "panel-dsi-txd0", 0);
    fb_dev->dsi_info->txd1 =
        fdtdec_get_int(blob, panel_node, "panel-dsi-txd1", 0);
    fb_dev->dsi_info->txd2 =
        fdtdec_get_int(blob, panel_node, "panel-dsi-txd2", 0);
    fb_dev->dsi_info->txd3 =
        fdtdec_get_int(blob, panel_node, "panel-dsi-txd3", 0);
    fb_dev->dsi_info->txd4 =
        fdtdec_get_int(blob, panel_node, "panel-dsi-txd4", 0);
    fb_dev->dsi_info->noncontinuous_clk =
        fdtdec_get_int(blob, panel_node, "panel-dsi-noncontinuous-clk", 0);
    fb_dev->dsi_info->t_pre =
        fdtdec_get_int(blob, panel_node, "panel-dsi-t-pre", 0);
    fb_dev->dsi_info->t_post =
        fdtdec_get_int(blob, panel_node, "panel-dsi-t-post", 0);
    fb_dev->dsi_info->tx_gap =
        fdtdec_get_int(blob, panel_node, "panel-dsi-tx-gap", 0);
    fb_dev->dsi_info->autoinsert_eotp =
        fdtdec_get_int(blob, panel_node, "panel-dsi-autoinsert-eotp", 0);
    fb_dev->dsi_info->htx_to_count =
        fdtdec_get_int(blob, panel_node, "panel-dsi-htx-to-count", 0);
    fb_dev->dsi_info->lrx_to_count =
        fdtdec_get_int(blob, panel_node, "panel-dsi-lrx-to-count", 0);
    fb_dev->dsi_info->bta_to_count =
        fdtdec_get_int(blob, panel_node, "panel-dsi-bta-to-count", 0);
    fb_dev->dsi_info->t_wakeup =
        fdtdec_get_int(blob, panel_node, "panel-dsi-t-wakeup", 0);
    fb_dev->dsi_info->pix_fifo_level =
        fdtdec_get_int(blob, panel_node,
        "panel-dsi-pix-fifo-send-level", 0);
    fb_dev->dsi_info->if_color_coding =
        fdtdec_get_int(blob, panel_node, "panel-dsi-if-color-coding", 0);
    fb_dev->dsi_info->pix_format =
        fdtdec_get_int(blob, panel_node, "panel-dsi-pix-format", 0);
    fb_dev->dsi_info->vsync_pol =
        fdtdec_get_int(blob, panel_node, "panel-dsi-vsync-pol", 0);
    fb_dev->dsi_info->hsync_pol =
        fdtdec_get_int(blob, panel_node, "panel-dsi-hsync-pol", 0);
    fb_dev->dsi_info->video_mode =
        fdtdec_get_int(blob, panel_node, "panel-dsi-video-mode", 0);
    fb_dev->dsi_info->hfp =
        fdtdec_get_int(blob, panel_node, "panel-dsi-hfp", 0);
    fb_dev->dsi_info->hbp =
        fdtdec_get_int(blob, panel_node, "panel-dsi-hbp", 0);
    fb_dev->dsi_info->hsa =
        fdtdec_get_int(blob, panel_node, "panel-dsi-hsa", 0);
    fb_dev->dsi_info->mult_pkts_en =
        fdtdec_get_int(blob, panel_node, "panel-dsi-mult-pkts-en", 0);
    fb_dev->dsi_info->vbp =
        fdtdec_get_int(blob, panel_node, "panel-dsi-vbp", 0);
    fb_dev->dsi_info->vfp =
        fdtdec_get_int(blob, panel_node, "panel-dsi-vfp", 0);
    fb_dev->dsi_info->vsa =
        fdtdec_get_int(blob, panel_node, "panel-dsi-vsa", 0);
    fb_dev->dsi_info->bllp_mode =
        fdtdec_get_int(blob, panel_node, "panel-dsi-bllp-mode", 0);
    fb_dev->dsi_info->use_null_pkt_bllp =
        fdtdec_get_int(blob, panel_node, "panel-dsi-use-null-pkt-bllp", 0);
    fb_dev->dsi_info->vc =
        fdtdec_get_int(blob, panel_node, "panel-dsi-vc", 0);
    fb_dev->dsi_info->cmd_type =
        fdtdec_get_int(blob, panel_node, "panel-dsi-cmd-type", 0);
    fb_dev->dsi_info->dsi_clk =
        fdtdec_get_int(blob, panel_node, "panel-dsi-clk", 0);
    AKFB_DEBUG("%s,line:%d, fdt_addr:0x%p\n", __func__, __LINE__, blob);
    AKFB_DEBUG("%s, line:%d, get panel_info->rgb_seq:%d, OK!!!\n",
        __func__, __LINE__, fb_dev->panel_info->rgb_seq);
    AKFB_DEBUG("%s, line:%d, get panel_info->width:%d, OK!!!\n",
        __func__, __LINE__, fb_dev->panel_info->width);
    AKFB_DEBUG("%s, line:%d, get panel_info->height:%d, OK!!!\n",
        __func__, __LINE__, fb_dev->panel_info->height);
    AKFB_DEBUG("%s, line:%d, get panel_info->pclk_div:%d, OK!!!\n",
        __func__, __LINE__, fb_dev->panel_info->pclk_div);
    AKFB_DEBUG("%s, line:%d, get dsi_info->num_lane:%d, OK!!!\n",
        __func__, __LINE__, fb_dev->dsi_info->num_lane);
    AKFB_DEBUG("%s, line:%d, get dsi_info->txd0:%d, OK!!!\n",
        __func__, __LINE__, fb_dev->dsi_info->txd0);
    AKFB_DEBUG("%s, line:%d, get dsi_info->txd1:%d, OK!!!\n",
        __func__, __LINE__, fb_dev->dsi_info->txd1);
    AKFB_DEBUG("%s, line:%d, get dsi_info->txd2:%d, OK!!!\n",
        __func__, __LINE__, fb_dev->dsi_info->txd2);
    AKFB_DEBUG("%s, line:%d, get dsi_info->txd3:%d, OK!!!\n",
        __func__, __LINE__, fb_dev->dsi_info->txd3);
    AKFB_DEBUG("%s, line:%d, get dsi_info->txd4:%d, OK!!!\n",
        __func__, __LINE__, fb_dev->dsi_info->txd4);
    AKFB_DEBUG("%s, line:%d, get dsi_info->noncontinuous_clk:%d, OK!!!\n",
        __func__, __LINE__, fb_dev->dsi_info->noncontinuous_clk);
    AKFB_DEBUG("%s, line:%d, get dsi_info->t_pre:%d, OK!!!\n",
        __func__, __LINE__, fb_dev->dsi_info->t_pre);
    AKFB_DEBUG("%s, line:%d, get dsi_info->t_post:%d, OK!!!\n",
        __func__, __LINE__, fb_dev->dsi_info->t_post);
    AKFB_DEBUG("%s, line:%d, get dsi_info->tx_gap:%d, OK!!!\n",
        __func__, __LINE__, fb_dev->dsi_info->tx_gap);
    AKFB_DEBUG("%s, line:%d, get dsi_info->autoinsert_eotp:%d, OK!!!\n",
        __func__, __LINE__, fb_dev->dsi_info->autoinsert_eotp);
    AKFB_DEBUG("%s, line:%d, get dsi_info->htx_to_count:%d, OK!!!\n",
        __func__, __LINE__, fb_dev->dsi_info->htx_to_count);
    AKFB_DEBUG("%s, line:%d, get dsi_info->lrx_to_count:%d, OK!!!\n",
        __func__, __LINE__, fb_dev->dsi_info->lrx_to_count);
    AKFB_DEBUG("%s, line:%d, get dsi_info->bta_to_count:%d, OK!!!\n",
        __func__, __LINE__, fb_dev->dsi_info->bta_to_count);
    AKFB_DEBUG("%s, line:%d, get dsi_info->t_wakeup:%d, OK!!!\n",
        __func__, __LINE__, fb_dev->dsi_info->t_wakeup);
    AKFB_DEBUG("%s, line:%d, get dsi_info->pix_fifo_level:%d, OK!!!\n",
        __func__, __LINE__, fb_dev->dsi_info->pix_fifo_level);
    AKFB_DEBUG("%s, line:%d, get dsi_info->if_color_coding:%d, OK!!!\n",
        __func__, __LINE__, fb_dev->dsi_info->if_color_coding);
    AKFB_DEBUG("%s, line:%d, get dsi_info->pix_format:%d, OK!!!\n",
        __func__, __LINE__, fb_dev->dsi_info->pix_format);
    AKFB_DEBUG("%s, line:%d, get dsi_info->vsync_pol:%d, OK!!!\n",
        __func__, __LINE__, fb_dev->dsi_info->vsync_pol);
    AKFB_DEBUG("%s, line:%d, get dsi_info->hsync_pol:%d, OK!!!\n",
        __func__, __LINE__, fb_dev->dsi_info->hsync_pol);
    AKFB_DEBUG("%s, line:%d, get dsi_info->video_mode:%d, OK!!!\n",
        __func__, __LINE__, fb_dev->dsi_info->video_mode);
    AKFB_DEBUG("%s, line:%d, get dsi_info->hfp:%d, OK!!!\n",
        __func__, __LINE__, fb_dev->dsi_info->hfp);
    AKFB_DEBUG("%s, line:%d, get dsi_info->hbp:%d, OK!!!\n",
        __func__, __LINE__, fb_dev->dsi_info->hbp);
    AKFB_DEBUG("%s, line:%d, get dsi_info->hsa:%d, OK!!!\n",
        __func__, __LINE__, fb_dev->dsi_info->hsa);
    AKFB_DEBUG("%s, line:%d, get dsi_info->mult_pkts_en:%d, OK!!!\n",
        __func__, __LINE__, fb_dev->dsi_info->mult_pkts_en);
    AKFB_DEBUG("%s, line:%d, get dsi_info->vbp:%d, OK!!!\n",
        __func__, __LINE__, fb_dev->dsi_info->vbp);
    AKFB_DEBUG("%s, line:%d, get dsi_info->vfp:%d, OK!!!\n",
        __func__, __LINE__, fb_dev->dsi_info->vfp);
    AKFB_DEBUG("%s, line:%d, get dsi_info->vsa:%d, OK!!!\n",
        __func__, __LINE__, fb_dev->dsi_info->vsa);
    AKFB_DEBUG("%s, line:%d, get dsi_info->bllp_mode:%d, OK!!!\n",
        __func__, __LINE__, fb_dev->dsi_info->bllp_mode);
    AKFB_DEBUG("%s, line:%d, get dsi_info->use_null_pkt_bllp:%d, OK!!!\n",
        __func__, __LINE__, fb_dev->dsi_info->use_null_pkt_bllp);
    AKFB_DEBUG("%s, line:%d, get dsi_info->vc:%d, OK!!!\n",
        __func__, __LINE__, fb_dev->dsi_info->vc);
    AKFB_DEBUG("%s, line:%d, get dsi_info->cmd_type:%d, OK!!!\n",
        __func__, __LINE__, fb_dev->dsi_info->cmd_type);
    AKFB_DEBUG("%s, line:%d, get dsi_info->dsi_clk:%d, OK!!!\n",
        __func__, __LINE__, fb_dev->dsi_info->dsi_clk);
}/* end of func */

/* ak_fb_fdtdec_decode_display_timing */
static int ak_fb_fdtdec_decode_disp_timing(const void *blob,
    int parent, struct ak_fb *fb_dev)
{
    int panel_node, len;
    const char *nodep;
    int ret = 0;

    /* get subnode with  status = "okay";*/
    for (panel_node = fdt_first_subnode(blob, parent);
        panel_node > 0;panel_node = fdt_next_subnode(blob, panel_node)){
        nodep = fdt_getprop(blob, panel_node, "status", &len);
        if(strcmp(nodep, "okay") == 0)
            break;
    }
    /* now panel_node got, save it */
    fb_dev->lcd_node = panel_node;

    fb_dev->panel_info->name =
        fdt_getprop(blob, panel_node, "compatible", &len);
    fb_dev->panel_info->rgb_if =
        fdtdec_get_int(blob, panel_node, "panel-if", 0);
    AKFB_DEBUG("%s, line:%d, get panel_info->name:%s, panel_node:0x%x OK!!!\n",
        __func__, __LINE__, fb_dev->panel_info->name, panel_node);
    AKFB_DEBUG("%s, line:%d, get panel_info->rgb_if:%s, OK!!!\n", __func__,
        __LINE__, fb_dev->panel_info->rgb_if?"MIPI":"RGB");

    fb_dev->reset_pin =
        fdtdec_get_valoffset_int(blob, parent, "reset-pins", 1, 0);
    if (!fb_dev->reset_pin) {
        pr_err("could not get %s pannel device reset pin\n",
            fb_dev->panel_info->rgb_if?"MIPI":"RGB");
        ret = 1;
        return ret;
    }

    ret = gpio_request(fb_dev->reset_pin, "lcd-reset-pins");
    if (ret) {
        pr_err("%s %d, gpio request lcd-reset-pins:%d fail\n",
                __func__, __LINE__, fb_dev->reset_pin);
    }

    AKFB_DEBUG("%s, line:%d, %s pannel if = %d!\n", __func__, __LINE__,
        fb_dev->panel_info->rgb_if?"MIPI":"RGB",fb_dev->panel_info->rgb_if);
#ifdef CONFIG_37_E_CODE
    fb_dev->power_pin =
        fdtdec_get_valoffset_int(blob, parent, "pwr-gpio", 1, 0);
    if (!fb_dev->power_pin) {
        pr_err("could not get lcd device power pin\n");
        ret = 1;
        return ret;
    }

    ret = gpio_request(fb_dev->power_pin, "lcd-pwr-gpio");
    if (ret) {
        pr_err("%s %d, gpio request lcd-pwr-gpio:%d fail\n",
                __func__, __LINE__, fb_dev->power_pin);
    }

    fb_dev->backlight_pin =
        fdtdec_get_valoffset_int(blob, parent, "backlight-gpio", 1, 0);
    if (!fb_dev->backlight_pin) {
        pr_err("could not get lcd device power pin\n");
        ret = 1;
        return ret;
    }

    ret = gpio_request(fb_dev->backlight_pin, "lcd-backlight-gpio");
    if (ret) {
        pr_err("%s %d, gpio request lcd-backlight-gpio:%d fail\n",
                __func__, __LINE__, fb_dev->backlight_pin);
    }

#endif
    fb_dev->lcd_go = False;
    fb_dev->par_change = False;
    /* get lcd fb type, 0:single buffer, 1:double buffer */
    fb_dev->fb_type = fdtdec_get_int(blob, parent, "lcd-fb-type", 0);
    /* get lcdc rgb config info from dts */
    get_lcdc_rgb_cfg(fb_dev, blob, parent);

    /* panel-if: 0 for RGB, 1 for MIPI, 2 for MPU */
    if(fb_dev->panel_info->rgb_if == 0 ){
        /* rgb_panel_init timing*/
        init_rgb_panel_timing(fb_dev, blob, panel_node);
    } else if (fb_dev->panel_info->rgb_if == 1) {
        /* mipi_panel_init timing  */
        init_mipi_panel_timing(fb_dev, blob, panel_node);
    }
    return ret;
}/* end of func */


static int ak_fb_ofdata_to_platdata(struct udevice *dev)
{
    struct ak_fb *fb_dev = dev_get_priv(dev);
    const void *blob = gd->fdt_blob;

    struct lcdc_rgb_input_info *rgb_input;
    struct lcdc_rgb_info *rgb_info;
    struct lcdc_osd_info *osd_info;
    struct lcdc_panel_info *panel_info;
    struct lcdc_dsi_info *dsi_info;
    /*
    * malloc fb some struct space
    */
    rgb_input = kzalloc(sizeof(struct lcdc_rgb_input_info), GFP_KERNEL);
    rgb_info = kzalloc(sizeof(struct lcdc_rgb_info), GFP_KERNEL);
    osd_info = kzalloc(sizeof(struct lcdc_osd_info), GFP_KERNEL);
    panel_info = kzalloc(sizeof(struct lcdc_panel_info), GFP_KERNEL);
    dsi_info = kzalloc(sizeof(struct lcdc_dsi_info), GFP_KERNEL);
    if ((!rgb_input)||(!rgb_info)||(!osd_info)||(!panel_info)||(!dsi_info)) {
        pr_err("alloc lcdc info buffer error\n");
        return -ENOMEM;;
    }

    fb_dev->rgb_input = rgb_input;
    fb_dev->rgb_info = rgb_info;
    fb_dev->osd_info = osd_info;
    fb_dev->panel_info = panel_info;
    fb_dev->dsi_info = dsi_info;

    /* register region remap*/
    fb_dev->lcdc_reg =  dev_remap_addr_index(dev,0);
    fb_dev->dsi_reg =  dev_remap_addr_index(dev,1);
    if ((!fb_dev->lcdc_reg) || (!fb_dev->dsi_reg)){
        pr_err("%s: No display controller address\n", __func__);
        return -EINVAL;
    }
    AKFB_DEBUG("%s, line:%d, fb_dev->lcdc_reg = %p\n", __func__, __LINE__,
        fb_dev->lcdc_reg);
    AKFB_DEBUG("%s, line:%d, fb_dev->dsi_reg  = %p\n", __func__, __LINE__,
        fb_dev->dsi_reg);

    /* decoder display timing*/
    if (ak_fb_fdtdec_decode_disp_timing(blob, dev_of_offset(dev), fb_dev)) {
        pr_err("%s: Failed to decode display timing\n", __func__);
        return -EINVAL;
    }

    return 0;
}

static int ak_fb_bind(struct udevice *dev)
{
    struct video_uc_platdata *uc_plat = dev_get_uclass_platdata(dev);

    uc_plat->size = LCD_MAX_WIDTH * LCD_MAX_HEIGHT *(24) / 8;
    uc_plat->align = 4096;

    AKFB_DEBUG("%s: Frame buffer size %x\n", __func__, uc_plat->size);
    return 0;
}


static const struct udevice_id ak_lcdc_match[] = {
    { .compatible = "anyka,ak37d-lcd-ctrl", },
    { .compatible = "anyka,ak37e-lcd-ctrl", },
    {}
};

U_BOOT_DRIVER(ak_fb) = {
    .name   = "akfb",
    .id = UCLASS_VIDEO,
    .of_match = ak_lcdc_match,
    .bind   = ak_fb_bind,
    .probe  = ak_fb_probe,
    .ofdata_to_platdata = ak_fb_ofdata_to_platdata,
    .priv_auto_alloc_size = sizeof(struct ak_fb),
    .flags  = DM_FLAG_PRE_RELOC,
};
#endif // CONFIG_DM_VIDEO
