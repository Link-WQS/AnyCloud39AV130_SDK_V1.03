/*
 * (C) Copyright 2010
 * Texas Instruments, <www.ti.com>
 * Syed Mohammed Khasim <khasim@ti.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation's version 2 of
 * the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef _AK_MCI_H
#define _AK_MCI_H

#define SDIO_INTRCTR_REG        0x000
#define MCI_CLK_REG             0x004
#define MCI_ARGUMENT_REG        0x008
#define MCI_COMMAND_REG         0x00c
#define MCI_RESPCMD_REG         0x010
#define MCI_RESPONSE0_REG       0x014
#define MCI_RESPONSE1_REG       0x018
#define MCI_RESPONSE2_REG       0x01c
#define MCI_RESPONSE3_REG       0x020
#define MCI_DATATIMER_REG       0x024
#define MCI_DATALENGTH_REG      0x028
#define MCI_DATACTRL_REG        0x02c
#define MCI_DATACNT_REG         0x030
#define MCI_STATUS_REG          0x034
#define MCI_MASK_REG            0x038
#define MCI_DMACTRL_REG         0x03c
#define MCI_FIFO_REG            0x040

#if defined(CONFIG_37_D_CODE) || defined(CONFIG_37_E_CODE)
#define MCI_PLUGINOFF_REG0          (0x08000080) 
#define MCI_PLUGINOFF_REG1          (0x08000084)
#endif

#if defined(CONFIG_39EV33X_CODE) || defined(CONFIG_3918AV100_CODE) || \
    defined(CONFIG_3918EV300L_CODE)
#define MCI_PLUGINOFF_REG0          (0x080000EC)   
#define MCI_PLUGINOFF_REG1          (0x080000f0)   
#endif

#if defined(CONFIG_KM01A_CODE) || defined(CONFIG_3918AV130_CODE)
#define MCI_PLUGINOFF_REG           (0x044)
#endif

#ifdef CONFIG_39EV200_CODE
#define MCI_PLUGINOFF_REG0          (0x080000EC)
#define MCI_PLUGINOFF_REG1          (0x080000f0)
#endif

/************** MCI_POWER_CONTROL   0x000 **************/
#define MCI_DELAY_CYCLE(x)   (((x) & 0xf) << 9)

/************** MCI_CLK_REG         0x004 **************/
#define MMC_CLK_DIVL(x)     ((x) & 0xff)
#define MMC_CLK_DIVH(x)     (((x) & 0xff) << 8)
#define MCI_CLK_ENABLE      (1 << 16)
#define MCI_CLK_PWRSAVE     (1 << 17)
#if defined(CONFIG_KM01A_CODE) || defined(CONFIG_3918AV130_CODE)
#define MCI_CLK_BYPASS      (1 << 18)
#define MCI_HS_MODE         (1 << 19)
#else
#define MCI_FALL_TRIGGER    (1 << 19)
#endif
#define MCI_ENABLE          (1 << 20)
#if defined(CONFIG_KM01A_CODE) || defined(CONFIG_3918AV130_CODE)
#define MCI_PWR_FIX         (1 << 21)
#define MCI_SPEED_MODE_FIX  (1 << 22)
#define MCI_MULREAD_STOP_EN (1 << 23)
#endif

/************** MCI_COMMAND_REG     0x00c **************/
#define MCI_CPSM_ENABLE     (1 << 0)
#define MCI_CPSM_CMD(x)     (((x) & 0x3f) << 1)
#define MCI_CPSM_RESPONSE   (1 << 7)
#define MCI_CPSM_LONGRSP    (1 << 8)
#define MCI_CPSM_PENDING    (1 << 9)
#define MCI_CPSM_RSPCRC_NOCHK   (1 << 10)
#define MCI_CPSM_WITHDATA       (1 << 11)
#define MCI_CPSM_CMD12_IOABORT  (1 << 13)
#define MCI_RESTORE_SDIOIRQ     (1 << 14)
#define MCI_CPSM_RSPWAITCFG     (16)

/************** MCI_DATATIMER_REG   0x024 **************/
#define TRANS_DATA_TIMEOUT   0x800000 

/************** MCI_DATACTRL_REG    0x02c **************/
#define MCI_DPSM_ENABLE     (1 << 0)
#define MCI_DPSM_DIRECTION  (1 << 1)
#define MCI_DPSM_STREAM     (1 << 2)
#define MCI_DPSM_BUSMODE(x)             (((x) & 0x3) << 3)
#define MCI_DPSM_BLOCKSIZE(x)           (((x) & 0xfff) << 16)
#define MCI_DPSM_ABORT_EN_MULTIBLOCK    (1 << 6)

/************** MCI_STATUS_REG      0x034 **************/
#define MCI_RESPCRCFAIL         (1 << 0)
#define MCI_DATACRCFAIL         (1 << 1)
#define MCI_RESPTIMEOUT         (1 << 2)
#define MCI_DATATIMEOUT         (1 << 3)
#define MCI_RESPEND             (1 << 4)
#define MCI_CMDSENT             (1 << 5)
#define MCI_DATAEND             (1 << 6)
#define MCI_DATABLOCKEND        (1 << 7)
#define MCI_STARTBIT_ERR        (1 << 8)
#define MCI_CMDACTIVE           (1 << 9)
#define MCI_TXACTIVE            (1 << 10)
#define MCI_RXACTIVE            (1 << 11)
#define MCI_FIFOFULL            (1 << 12)
#define MCI_FIFOEMPTY           (1 << 13)
#define MCI_FIFOHALFFULL        (1 << 14)
#define MCI_FIFOHALFEMPTY       (1 << 15)
#define MCI_DATATRANS_FINISH    (1 << 16)
#define MCI_SDIOINT             (1 << 17)
#if defined(CONFIG_KM01A_CODE) || defined(CONFIG_3918AV130_CODE)
#define MCI_PIINT               (1 << 20)
#define MCI_POINT               (1 << 21)
#define MCI_PIOSTA              (1 << 22)
#define MCI_DATA0POS            (1 << 23)
#define MCI_DATA0NEG            (1 << 24)
#define MCI_CARDNOBUSY          (1 << 25)
#define MCI_CMDRSPEND_DLY       (1 << 26)
#define MCI_CMDSENT_DLY         (1 << 27)
#endif
/************** MCI_MASK_REG        0x038 **************/
#define MCI_RESPCRCFAILMASK (1 << 0)
#define MCI_DATACRCFAILMASK (1 << 1)
#define MCI_RESPTIMEOUTMASK (1 << 2)
#define MCI_DATATIMEOUTMASK (1 << 3)
#define MCI_RESPENDMASK     (1 << 4)
#define MCI_CMDSENTMASK     (1 << 5)
#define MCI_DATAENDMASK     (1 << 6)
#define MCI_DATABLOCKENDMASK    (1 << 7)
#define MCI_STARTBIT_ERRMASK    (1 << 8)
#define MCI_CMDACTIVEMASK   (1 << 9)
#define MCI_TXACTIVEMASK    (1 << 10)
#define MCI_RXACTIVEMASK    (1 << 11)
#define MCI_FIFOFULLMASK    (1 << 12)
#define MCI_FIFOEMPTYMASK   (1 << 13)
#define MCI_FIFOHALFFULLMASK    (1 << 14)
#define MCI_FIFOHALFEMPTYMASK   (1 << 15)
#define MCI_DATATRANS_FINISHMASK    (1 << 16)
#define MCI_SDIOINTMASK             (1 << 17)

#define MCI_CMDIRQMASKS \
    (MCI_CMDSENTMASK|MCI_RESPENDMASK|       \
     MCI_RESPCRCFAILMASK|MCI_RESPTIMEOUTMASK)

#define MCI_DATAIRQMASKS \
    (MCI_DATAEND|       \
     MCI_DATACRCFAILMASK|MCI_DATATIMEOUTMASK)


/************** MCI_DMACTRL_REG     0x03c **************/
#define MCI_DMA_BUFEN       (1 << 0)
#define MCI_DMA_ADDR(x)     (((x) & 0x7fff) << 1)
#define MCI_DMA_EN          (1 << 16)
#define MCI_DMA_SIZE(x)     (((x) & 0x7fff) << 17)


/************** MCI_PLUGINOFF_REG0  MCI_PLUGINOFF_REG1 **************/
#define MCI_PLUGOFF_INT_OFFSET      0
#define MCI0_PLUG_STATUS_OFFSET     2
#define MCI_PLUGIN_INT_OFFSET       3
#define MCI_PLUGOFF_STA_OFFSET      8
#define MCI_PLUGIN_STA_OFFSET       11
#define MCI_PLUG_ACTIVE_OFFSET      16
#define MCI_PLUG_STATUS_OFFSET      19


/***** macro for MCI_PLUGINOFF_REG0 *****/
#define  MCI0_D3_DETECT_ENABLE_OFFSET       11
#define  MCI0_CLK_DETECT_ENABLE_OFFSET      8



/************** other **************/
/* H3B/H3D/Sundance4 using auto detect sdio irq*/
#define SDIO_INTR_AUTO_DETECT

enum akmci_detect_mode {
    AKMCI_PLUGIN_ALWAY,
    AKMCI_DETECT_MODE_CLK,
    AKMCI_DETECT_MODE_GPIO,
    AKMCI_DETECT_MODE_D3,
};
    
#define MMC_BUS_WIDTH_1     1
#define MMC_BUS_WIDTH_4     4
#define MMC_BUS_WIDTH_8     8

#define SD0_SOFTRESET           (0x1 << 1)
#define SD1_SOFTRESET           (0x1 << 2)
#define SD2_SOFTRESET           (0x1 << 19)

#define CARD_UNPLUGED           0
#define CARD_PLUGED             1
#define CARD_POWER_ENABLE       1
#define CARD_POWER_DISABLE      0

#define SD_DEFAULT_VOLTAGE          (0x00FF8000)

#endif /* _AK_MCI_H */
