/*
 * Copyright (C) 2018-2020 AICSemi Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "reg_spi.h"
#include "reg_iomux.h"
#include "dma_generic.h"
#include "sysctrl_api.h"
#include "dbg.h"
#include "rtos_al.h"

/*
 ****************************************************************************************
 * This spi slave interface test is only for AIC8800MC and AIC8800M40.
 ****************************************************************************************
 */

#ifdef CFG_TEST_SPI_SLAVE
/*
 * MACROS
 ****************************************************************************************
 */
#define USER_PRINTF             dbg
#define SPI_MASTER_OR_SLAVE     (0)  // 1: master mode, 0: slave mode
#define SPI_CLK_FREQ            (1000000)
#define BUFFER_LENGTH           (16)

// spi_dma
#define SPI0_ISR_ENABLED        (1)
#define SPI0_DMA_ISR_ENABLED    (1)
#define SPI0_RXDMA_CH_IDX       DMA_CHANNEL_SPI0_RX
#define SPI0_TXDMA_CH_IDX       DMA_CHANNEL_SPI0_TX
#define SPI0_RXDMA_IRQ_IDX      DMA08_IRQn
#define SPI0_TXDMA_IRQ_IDX      DMA09_IRQn
#define spi0_rxdma_isr          DMA08_IRQHandler
#define spi0_txdma_isr          DMA09_IRQHandler
#define spi0_isr                SPI_IRQHandler

/*
 * GLOBAL CONSTANTS & VARIABLES
 ****************************************************************************************
 */
#if SPI0_ISR_ENABLED
rtos_semaphore spi0_tx_empty_sema;
rtos_semaphore spi0_rx_full_sema;
rtos_semaphore spi0_txend_sema;
rtos_semaphore spi0_rxend_sema;
#endif
#if SPI0_DMA_ISR_ENABLED
rtos_semaphore spi0_rxdma_sema;
rtos_semaphore spi0_txdma_sema;
#endif

unsigned int tx_buff[BUFFER_LENGTH];
unsigned int rx_buff[BUFFER_LENGTH];

/*
 * FUNCTIONS
 ****************************************************************************************
 */
/**
 ****************************************************************************************
 * @brief init spi, config iomux & clock div
 ****************************************************************************************
 */
void spi0_init(void)
{
    uint32_t pclk;
    #if SPI0_DMA_ISR_ENABLED
    int ret;
    #endif

    // clk en
    #if (PLF_AIC8800)
    cpusysctrl_pclkme_set(CSC_PCLKME_SPI_EN_BIT);
    cpusysctrl_oclkme_set(CSC_OCLKME_SPI_EN_BIT);
    cpusysctrl_hclkme_set(CSC_HCLKME_DMA_EN_BIT);
    #else
    cpusysctrl_pclkme_set(CPU_SYS_CTRL_PCLK_SPI_MANUAL_ENABLE);
    cpusysctrl_oclkme_set(CPU_SYS_CTRL_OTHERS_SPI_MANUAL_ENABLE);
    cpusysctrl_hclkme_set(CPU_SYS_CTRL_HCLK_DMA_MANUAL_ENABLE);
    #endif

    pclk = sysctrl_clock_get(SYS_PCLK);
    spi_dr_div_setf(pclk / 2 / SPI_CLK_FREQ - 1);  // pclk/(2*(div+1)) = freq

    #if !(SPI_MASTER_OR_SLAVE)
    uint32_t cnt = 5;
    for (int i=0; i<cnt; i++) {
        AIC_SPI0->CR[0] = ((1<<1) | (1<<0));
        AIC_SPI0->CR[0] = 0;
    }
    #endif

    // iomux
    #if (PLF_AIC8800)
    iomux_gpio_config_sel_setf(10, 3); // clk
    iomux_gpio_config_sel_setf(11, 3); // cs
    iomux_gpio_config_sel_setf(12, 3); // di
    iomux_gpio_config_sel_setf(13, 3); // do
    #elif (PLF_AIC8800MC)
    iomux_gpioa_config_sel_setf(10, 2); // clk
    iomux_gpioa_config_sel_setf(11, 2); // cs
    iomux_gpioa_config_sel_setf(12, 2); // di
    iomux_gpioa_config_sel_setf(13, 2); // do
    #elif (PLF_AIC8800M40)
    iomux_gpioa_config_sel_setf(10, 3); // clk
    iomux_gpioa_config_sel_setf(11, 3); // cs
    iomux_gpioa_config_sel_setf(12, 3); // di
    iomux_gpioa_config_sel_setf(13, 3); // do
    #endif

    #if SPI0_ISR_ENABLED
    //AIC_SPI0->IER |= (0x01UL << 8);  // txend: valid only when use DMA
    //AIC_SPI0->IER |= (0x01UL << 9);  // rxend: valid only when use DMA
    ret = rtos_semaphore_create(&spi0_tx_empty_sema, 1, 0);
    if (ret) {
        dbg("sema create fail: %d\r\n", ret);
    }
    ret = rtos_semaphore_create(&spi0_rx_full_sema, 1, 0);
    if (ret) {
        dbg("sema create fail: %d\r\n", ret);
    }
    ret = rtos_semaphore_create(&spi0_txend_sema, 1, 0);
    if (ret) {
        dbg("sema create fail: %d\r\n", ret);
    }
    ret = rtos_semaphore_create(&spi0_rxend_sema, 1, 0);
    if (ret) {
        dbg("sema create fail: %d\r\n", ret);
    }
    NVIC_SetPriority(SPI_IRQn, __NVIC_PRIO_LOWEST);
    NVIC_EnableIRQ(SPI_IRQn);
    #endif

    #if SPI0_DMA_ISR_ENABLED
    ret = rtos_semaphore_create(&spi0_txdma_sema, 1, 0);
    if (ret) {
        dbg("sema create fail: %d\r\n", ret);
    }
    ret = rtos_semaphore_create(&spi0_rxdma_sema, 1, 0);
    if (ret) {
        dbg("sema create fail: %d\r\n", ret);
    }
    NVIC_SetPriority(SPI0_RXDMA_IRQ_IDX, __NVIC_PRIO_LOWEST);
    NVIC_EnableIRQ(SPI0_RXDMA_IRQ_IDX);
    NVIC_SetPriority(SPI0_TXDMA_IRQ_IDX, __NVIC_PRIO_LOWEST);
    NVIC_EnableIRQ(SPI0_TXDMA_IRQ_IDX);
    #endif
}

void spi0_deinit(void)
{
    // iomux
    #if (PLF_AIC8800)
    iomux_gpio_config_sel_setf(10, 0); // clk
    iomux_gpio_config_sel_setf(11, 0); // cs
    iomux_gpio_config_sel_setf(12, 0); // di
    iomux_gpio_config_sel_setf(13, 0); // do
    #elif (PLF_AIC8800MC)
    iomux_gpioa_config_sel_setf(10, 0); // clk
    iomux_gpioa_config_sel_setf(11, 0); // cs
    iomux_gpioa_config_sel_setf(12, 0); // di
    iomux_gpioa_config_sel_setf(13, 0); // do
    #elif (PLF_AIC8800M40)
    iomux_gpioa_config_sel_setf(10, 0); // clk
    iomux_gpioa_config_sel_setf(11, 0); // cs
    iomux_gpioa_config_sel_setf(12, 0); // di
    iomux_gpioa_config_sel_setf(13, 0); // do
    #endif

    // clk rst
    cpusysctrl_pclkrs_set(CPU_SYS_CTRL_PRESETN_SPI_SET);
    cpusysctrl_pclkrc_set(CPU_SYS_CTRL_PRESETN_SPI_CLEAR);
    cpusysctrl_oclkrs_set(CPU_SYS_CTRL_RESETN_SPI_SET);
    cpusysctrl_oclkrc_set(CPU_SYS_CTRL_RESETN_SPI_CLEAR);

    // dma disable
    dma_ch_icsr_set(SPI0_RXDMA_CH_IDX, (dma_ch_icsr_get(SPI0_RXDMA_CH_IDX) | DMA_CH_TBL2_ICLR_BIT));
    dma_ch_ctlr_set(SPI0_RXDMA_CH_IDX, 0);
    dma_ch_icsr_set(SPI0_TXDMA_CH_IDX, (dma_ch_icsr_get(SPI0_TXDMA_CH_IDX) | DMA_CH_TBL2_ICLR_BIT));
    dma_ch_ctlr_set(SPI0_TXDMA_CH_IDX, 0);

    #if SPI0_ISR_ENABLED
    rtos_semaphore_delete(spi0_tx_empty_sema);
    rtos_semaphore_delete(spi0_rx_full_sema);
    rtos_semaphore_delete(spi0_txend_sema);
    rtos_semaphore_delete(spi0_rxend_sema);
    NVIC_DisableIRQ(SPI_IRQn);
    #endif

    #if SPI0_DMA_ISR_ENABLED
    rtos_semaphore_delete(spi0_txdma_sema);
    rtos_semaphore_delete(spi0_rxdma_sema);
    NVIC_DisableIRQ(SPI0_RXDMA_IRQ_IDX);
    NVIC_DisableIRQ(SPI0_TXDMA_IRQ_IDX);
    #endif
}

/**
 ****************************************************************************************
 * @brief spi write and read word, slave mode, don`t use DMA.
 * @Support: AIC8800MC/AIC8800M40
 ****************************************************************************************
 */
int spi0_slave_write_read_words(unsigned int *buff_wr, unsigned int word_cnt_wr, unsigned int *buff_rd, unsigned int word_cnt_rd)
{
    if (word_cnt_wr > 31 || word_cnt_rd > 31) {
        USER_PRINTF("spi-fifo len is 31.\n");
        return -1;
    }

    AIC_SPI0->CR[3] = (((word_cnt_rd)&31)<<0) | (((0)&31)<<8);
    AIC_SPI0->OFCR = (((word_cnt_wr)&31)<<0) | (((0)&31)<<8);
    AIC_SPI0->MR0 = (((0)&7)<<3) | (1<<11) | (1<<10) | (1<<0);
    AIC_SPI0->CR[0] = (((32)&31)<<2) | (1<<1);                          // transbit 32
    AIC_SPI0->CR[1] = (1<<6) | (((0x3)&3)<<12);                         // recv and send trans
    AIC_SPI0->CR[5] = (((0x100)&0xFFFF)<<0);
    AIC_SPI0->CR[2] |= (1<<5);
    AIC_SPI0->TCR = (1<<1);                                             // rx req start

    for (int i=0; i<word_cnt_wr; i++) {                                 // send data
        AIC_SPI0->IOR = buff_wr[i];
    }

    for (int i=0; i<word_cnt_rd; i++) {                                 // read data (block-mode)
        while ((AIC_SPI0->SR & (1<<5)) == (1<<5)) ;
        buff_rd[i] = AIC_SPI0->IOR;
    }

    AIC_SPI0->CR[0] = (0x0FUL << 8); // cs
    AIC_SPI0->CR[2] = 0x00UL;
    AIC_SPI0->TCR   = 0x00UL;

    return 0;
}

/**
 ****************************************************************************************
 * @brief spi write word, slave mode, don`t use DMA.
 * @Support: AIC8800MC/AIC8800M40
 ****************************************************************************************
 */
int spi0_slave_write_words(unsigned int *buff_wr, unsigned int word_cnt_wr)
{
    if (word_cnt_wr > 31) {
        USER_PRINTF("spi-fifo len is 31.\n");
        return -1;
    }

    AIC_SPI0->OFCR = (((word_cnt_wr)&31)<<0) | (((0)&31)<<8);
    AIC_SPI0->MR0 = (((0)&7)<<3) | (1<<11) | (1<<10) | (1<<0);
    AIC_SPI0->CR[0] = (((32)&31)<<2) | (1<<1);                         // transbit 32
    AIC_SPI0->CR[1] = (1<<6) | (((0x2)&3)<<12);                        // send trans
    AIC_SPI0->CR[5] = (((0x100)&0xFFFF)<<0);
    AIC_SPI0->CR[2] |= (1<<5);
    AIC_SPI0->TCR = (1<<1);                                            // tx req start

    for (int i=0; i<word_cnt_wr; i++) {
        AIC_SPI0->IOR = buff_wr[i];
    }

    AIC_SPI0->IER |= (1<<7);                                           // Tx fifo empty interrupt enable
    //dbg("AIC_SPI0->IER: %08x\n", AIC_SPI0->IER);

    #if SPI0_ISR_ENABLED
    rtos_semaphore_wait(spi0_tx_empty_sema, -1);
    #endif

    AIC_SPI0->CR[0] = (0x0FUL << 8);                                   // cs
    AIC_SPI0->CR[2] = 0x00UL;
    AIC_SPI0->TCR   = 0x00UL;

    return 0;
}

/**
 ****************************************************************************************
 * @brief spi read word, slave mode, don`t use DMA.
 * @Support: AIC8800MC/AIC8800M40
 ****************************************************************************************
 */
int spi0_slave_read_words(unsigned int *buff_rd, unsigned int word_cnt_rd)
{
    if (word_cnt_rd > 31) {
        USER_PRINTF("spi-fifo len is 31.\n");
        return -1;
    }

    AIC_SPI0->CR[3] = (((word_cnt_rd)&31)<<0) | (((0)&31)<<8);
    AIC_SPI0->MR0 = (((0)&7)<<3) | (1<<11) | (1<<10) | (1<<0);
    AIC_SPI0->CR[0] = (((32)&31)<<2) | (1<<1);                         // transbit 32
    AIC_SPI0->CR[1] = (1<<6) | (((0x1)&3)<<12);                        // recv trans
    AIC_SPI0->CR[5] = (((0x100)&0xFFFF)<<0);
    AIC_SPI0->CR[2] |= (1<<5);
    AIC_SPI0->TCR = (1<<0);                                            // rx req start

    AIC_SPI0->IER |= (1<<6);                                           // Rx fifo full interrupt enable
    //dbg("AIC_SPI0->IER: %08x\n", AIC_SPI0->IER);

    #if SPI0_ISR_ENABLED
    rtos_semaphore_wait(spi0_rx_full_sema, -1);
    #endif

    for (int i=0; i<word_cnt_rd; i++) {
        //while ((AIC_SPI0->SR & (1<<5)) == (1<<5)) ; // block
        buff_rd[i] = AIC_SPI0->IOR;
    }

    AIC_SPI0->CR[0] = (0x0FUL << 8); // cs
    AIC_SPI0->CR[2] = 0x00UL;
    AIC_SPI0->TCR   = 0x00UL;

    return 0;
}

/**
 ****************************************************************************************
 * @brief spi write and read, master and slave mode, use DMA.
 * @Support: AIC8800M40
 * @param: mode       1:MASTER, 0:SLAVE
 *         bit_mode   0:8bit, 1:16bit, 2:24bit, 3:32bit
 ****************************************************************************************
 */
int spi0_dma_write_read(unsigned int *buff_wr, unsigned int word_cnt_wr,
                              unsigned int *buff_rd, unsigned int word_cnt_rd, int mode, int bit_mode)
{
    if(!mode){
        uint32_t cnt = 5;
        for (int i = 0; i<cnt; i++) {
            AIC_SPI0->CR[0] = ((1<<1) | (1<<0));
            AIC_SPI0->CR[0] = 0;
        }
    }

    int ch;
    unsigned int bit_len, shift_len;
    AIC_SPI0->CR[1] = 0x00UL;

    bit_len = (bit_mode == 3) ? 32 : (bit_mode == 2) ? 24 : (bit_mode == 1) ? 16 : 8;
    shift_len= (bit_mode == 3) ? 2 : bit_mode;

    // dma channal for spi_tx
    ch = SPI0_TXDMA_CH_IDX;
    dma_erqcsr_set(REQ_CID_SPI_TX, ch);
    dma_ch_rqr_erqm_clrb(ch);
    dma_ch_rqr_erql_setb(ch);
    dma_ch_dar_set(ch, (unsigned int)(&AIC_SPI0->IOR));
    dma_ch_sar_set(ch, (unsigned int)buff_wr);

    if ((bit_len == 32) || (bit_len == 24)){
          dma_ch_tbl0cr_set(ch, ((1<<shift_len) | (REQ_TBL0 << DMA_CH_RQTYP_LSB) | (AHB_WORD << DMA_CH_DBUSU_LSB) |
                             (AHB_WORD << DMA_CH_SBUSU_LSB) | DMA_CH_CONSTDA_BIT));
    } else if (bit_len == 16){
          dma_ch_tbl0cr_set(ch, ((1<<shift_len) | (REQ_TBL0 << DMA_CH_RQTYP_LSB) | (AHB_HWORD << DMA_CH_DBUSU_LSB) |
                             (AHB_HWORD << DMA_CH_SBUSU_LSB) | DMA_CH_CONSTDA_BIT));
    } else if (bit_len == 8){
          dma_ch_tbl0cr_set(ch, ((1<<shift_len) | (REQ_TBL0 << DMA_CH_RQTYP_LSB) | (AHB_BYTE << DMA_CH_DBUSU_LSB) |
                             (AHB_BYTE << DMA_CH_SBUSU_LSB) | DMA_CH_CONSTDA_BIT));
    }

    dma_ch_tbl1cr_set(ch, word_cnt_wr<<shift_len);
    dma_ch_tbl2cr_set(ch, word_cnt_wr<<shift_len);
    dma_ch_tsr_set(ch, (((1<<shift_len) << DMA_CH_STRANSZ_LSB) | ((1<<shift_len) << DMA_CH_DTRANSZ_LSB)));
    dma_ch_wmar_set(ch, 0);
    dma_ch_wjar_set(ch, 0);
    dma_ch_lnar_set(ch, 0);
    dma_ch_tbl0sr_set(ch, ((0 << DMA_CH_STBL0SZ_LSB) | (0 << DMA_CH_DTBL0SZ_LSB)));
    dma_ch_tbl1ssr_set(ch, 0);
    dma_ch_tbl1dsr_set(ch, 0);
    #if SPI0_DMA_ISR_ENABLED
    dma_ch_icsr_set(ch, (DMA_CH_TBL2_IENA_BIT));
    #endif
    dma_ch_ctlr_set(ch, (DMA_CH_CHENA_BIT | (0x01UL << DMA_CH_BUSBU_LSB)));

    // dma channal for spi_rx
    ch = SPI0_RXDMA_CH_IDX;
    dma_erqcsr_set(REQ_CID_SPI_RX, ch);
    dma_ch_rqr_erqm_clrb(ch);
    dma_ch_rqr_erql_setb(ch);
    dma_ch_dar_set(ch, (unsigned int)buff_rd);
    dma_ch_sar_set(ch, (unsigned int)(&AIC_SPI0->IOR));
    if ((bit_len == 32) || (bit_len == 24)){
          dma_ch_tbl0cr_set(ch, ((1<<shift_len) | (REQ_TBL0 << DMA_CH_RQTYP_LSB) | (AHB_WORD << DMA_CH_DBUSU_LSB) |
                             (AHB_WORD << DMA_CH_SBUSU_LSB) | DMA_CH_CONSTSA_BIT));
    } else if (bit_len == 16){
          dma_ch_tbl0cr_set(ch, ((1<<shift_len) | (REQ_TBL0 << DMA_CH_RQTYP_LSB) | (AHB_HWORD << DMA_CH_DBUSU_LSB) |
                             (AHB_HWORD << DMA_CH_SBUSU_LSB) | DMA_CH_CONSTSA_BIT));
    } else if (bit_len == 8){
          dma_ch_tbl0cr_set(ch, ((1<<shift_len) | (REQ_TBL0 << DMA_CH_RQTYP_LSB) | (AHB_BYTE << DMA_CH_DBUSU_LSB) |
                             (AHB_BYTE << DMA_CH_SBUSU_LSB) | DMA_CH_CONSTSA_BIT));
    }

    dma_ch_tbl1cr_set(ch, word_cnt_rd<<shift_len);
    dma_ch_tbl2cr_set(ch, word_cnt_rd<<shift_len);
    dma_ch_tsr_set(ch, (((1<<shift_len) << DMA_CH_STRANSZ_LSB) | ((1<<shift_len) << DMA_CH_DTRANSZ_LSB)));
    dma_ch_wmar_set(ch, 0);
    dma_ch_wjar_set(ch, 0);
    dma_ch_lnar_set(ch, 0);
    dma_ch_tbl0sr_set(ch, ((0 << DMA_CH_STBL0SZ_LSB) | (0 << DMA_CH_DTBL0SZ_LSB)));
    dma_ch_tbl1ssr_set(ch, 0);
    dma_ch_tbl1dsr_set(ch, 0);
    #if SPI0_DMA_ISR_ENABLED
    dma_ch_icsr_set(ch, (DMA_CH_TBL2_IENA_BIT));
    #endif
    dma_ch_ctlr_set(ch, (DMA_CH_CHENA_BIT | (0x01UL << DMA_CH_BUSBU_LSB)));

    #if SPI0_ISR_ENABLED
    AIC_SPI0->IER = (0x01UL << 8) | (0x01UL << 9);
    #endif

    if (mode) { // master mode
        AIC_SPI0->OCR   = word_cnt_wr; // out data cnt
        AIC_SPI0->ICR   = word_cnt_rd; // in data cnt
        AIC_SPI0->OFCR  = (((30)&31)<<8);
        AIC_SPI0->CR[3] = ((0x02UL << 8) | (0x01UL << 0));
        AIC_SPI0->CR[1] = ((0x03UL << 12) | (0x01UL << 6)); // send and recv, cs, 3-wire
        AIC_SPI0->CR[0] = ((0x0EUL <<  8) | (0x01UL << 1) | (((bit_len)&31) << 2) ); // cs, bit count
        AIC_SPI0->MR0   = ((0x01UL << 11) | (0x00UL << 3)); // slave, generic mode
        AIC_SPI0->CR[2] = (0x01UL <<  6); // en dma
        AIC_SPI0->TCR   = (0x01UL << 1); // trans start
        AIC_SPI0->CR[5] = (((0x20)&0xFFFF)<<0);
    }
    else { // slave mode
        AIC_SPI0->OCR   = word_cnt_wr; // out data cnt
        AIC_SPI0->ICR   = word_cnt_rd; // in data cnt
        AIC_SPI0->OFCR  = (((30)&31)<<8);
        AIC_SPI0->CR[3] = (((0x1UL)&31)<<0) | (((0x2UL)&31)<<8);
        AIC_SPI0->CR[0] = (((bit_len)&31)<<2) | (1<<1);
        AIC_SPI0->MR0   = (((0x0)&7)<<3) | (0x01UL << 10)  | (1<<0);
        AIC_SPI0->CR[2] = (0x01UL << 5);
        AIC_SPI0->CR[1] = (((0x3UL)&3)<<12) | (0x01UL <<  6); // send and recv, cs, 3-wire
        AIC_SPI0->TCR   = (0x01UL << 1); // trans start
        AIC_SPI0->CR[2] |= (0x01UL <<  6); // en dma
    }

    #if SPI0_DMA_ISR_ENABLED
    rtos_semaphore_wait(spi0_rxdma_sema, -1);
    #else
    while (dma_ch_icsr_tbl2_irst_getb(ch) == 0);
    dma_ch_icsr_set(ch, (dma_ch_icsr_get(ch) | DMA_CH_TBL2_ICLR_BIT));
    #endif

    #if SPI0_ISR_ENABLED
    rtos_semaphore_wait(spi0_rxend_sema, -1);
    #endif

    while(AIC_SPI0->RESERVED2 == 0);
    //dbg("AIC_SPI0->RESERVED2: %x\n", AIC_SPI0->RESERVED2);

    if (mode) {
        AIC_SPI0->CR[2] = 0;
    } else {
        AIC_SPI0->CR[2] = (0x01UL << 5);
    }

    AIC_SPI0->CR[0] = (0x0FUL << 8); // cs
    AIC_SPI0->CR[1] = 0x00UL;
    AIC_SPI0->TCR   = 0x00UL;

    AIC_SPI0->MR0   |=(1<<11);
    cpusysctrl_oclkrs_set(0x800);
    cpusysctrl_oclkrc_set(0x800);
    AIC_SPI0->MR0   &=~(1<<11);
    return 0;
}

/**
 ****************************************************************************************
 * @brief spi write, slave mode, use DMA.
 * @Support: AIC8800M40
 * @param: bit_mode   0:8bit, 1:16bit, 2:24bit, 3:32bit
 ****************************************************************************************
 */
int spi0_slave_dma_write(unsigned int *buff_wr, unsigned int word_cnt_wr, int bit_mode)
{
    uint32_t cnt = 5;
    for (int i=0; i<cnt; i++) {
        AIC_SPI0->CR[0] = ((1<<1) | (1<<0));
        AIC_SPI0->CR[0] = 0;
    }

    unsigned int bit_len, shift_len;
    bit_len = (bit_mode == 3) ? 32 : (bit_mode == 2) ? 24 : (bit_mode == 1) ? 16 : 8;
    shift_len= (bit_mode == 3) ? 2: bit_mode;

    int ch;
    // dma channal for spi_tx
    ch = SPI0_TXDMA_CH_IDX;
    dma_erqcsr_set(REQ_CID_SPI_TX, ch);
    dma_ch_rqr_erqm_clrb(ch);
    dma_ch_rqr_erql_setb(ch);
    dma_ch_dar_set(ch, (unsigned int)(&AIC_SPI0->IOR));
    dma_ch_sar_set(ch, (unsigned int)buff_wr);
    if ((bit_len == 32) || (bit_len == 24)){
          dma_ch_tbl0cr_set(ch, ((1<<shift_len) | (REQ_TBL0 << DMA_CH_RQTYP_LSB) | (AHB_WORD << DMA_CH_DBUSU_LSB) |
                             (AHB_WORD << DMA_CH_SBUSU_LSB) | DMA_CH_CONSTDA_BIT));
    } else if (bit_len == 16){
          dma_ch_tbl0cr_set(ch, ((1<<shift_len) | (REQ_TBL0 << DMA_CH_RQTYP_LSB) | (AHB_HWORD << DMA_CH_DBUSU_LSB) |
                             (AHB_HWORD << DMA_CH_SBUSU_LSB) | DMA_CH_CONSTDA_BIT));
    } else if (bit_len == 8){
          dma_ch_tbl0cr_set(ch, ((1<<shift_len) | (REQ_TBL0 << DMA_CH_RQTYP_LSB) | (AHB_BYTE << DMA_CH_DBUSU_LSB) |
                             (AHB_BYTE << DMA_CH_SBUSU_LSB) | DMA_CH_CONSTDA_BIT));
    }
    //dma_ch_tbl0cr_set(ch, ((4) | (REQ_TBL0 << DMA_CH_RQTYP_LSB) | (AHB_WORD << DMA_CH_DBUSU_LSB) |
    //                         (AHB_WORD << DMA_CH_SBUSU_LSB) | DMA_CH_CONSTDA_BIT));
    dma_ch_tbl1cr_set(ch, word_cnt_wr<<shift_len);
    dma_ch_tbl2cr_set(ch, word_cnt_wr<<shift_len);
    dma_ch_tsr_set(ch, (((1<<shift_len) << DMA_CH_STRANSZ_LSB) | ((1<<shift_len) << DMA_CH_DTRANSZ_LSB)));
    dma_ch_wmar_set(ch, 0);
    dma_ch_wjar_set(ch, 0);
    dma_ch_lnar_set(ch, 0);
    dma_ch_tbl0sr_set(ch, ((0 << DMA_CH_STBL0SZ_LSB) | (0 << DMA_CH_DTBL0SZ_LSB)));
    dma_ch_tbl1ssr_set(ch, 0);
    dma_ch_tbl1dsr_set(ch, 0);
    #if SPI0_DMA_ISR_ENABLED
    dma_ch_icsr_set(ch, (DMA_CH_TBL2_IENA_BIT));
    #endif
    dma_ch_ctlr_set(ch, (DMA_CH_CHENA_BIT | (0x01UL << DMA_CH_BUSBU_LSB)));

    #if SPI0_ISR_ENABLED
    AIC_SPI0->IER = (0x01UL << 8);  // txend: valid only when use DMA
    #endif

    AIC_SPI0->OCR   = word_cnt_wr; // out data cnt
    AIC_SPI0->OFCR  = (((30)&31)<<8);
    AIC_SPI0->CR[0] = (((bit_len)&31)<<2) | (1<<1);                   // transbit 32
    AIC_SPI0->MR0   = (((0x0)&7)<<3) | (0x01UL << 10)  | (1<<0);
    AIC_SPI0->CR[2] = (0x01UL << 5);
    AIC_SPI0->CR[1] = (((0x2UL)&3)<<12) | (0x01UL <<  6);        // send trans
    AIC_SPI0->TCR   = (0x01UL << 1);                             // tx req start
    AIC_SPI0->CR[2] |= (0x01UL <<  6); // en dma

    uint32_t reg = 0xFFFFFFFF;
    while((reg & (1<<7)) == (1<<7)) {
        reg = AIC_SPI0->SR;
    }

    #if SPI0_DMA_ISR_ENABLED
    rtos_semaphore_wait(spi0_txdma_sema, -1);
    #else
    while (dma_ch_icsr_tbl2_irst_getb(ch) == 0);
    dma_ch_icsr_set(ch, (dma_ch_icsr_get(ch) | DMA_CH_TBL2_ICLR_BIT));
    #endif

    #if SPI0_ISR_ENABLED
    rtos_semaphore_wait(spi0_txend_sema, -1);
    #endif

    reg = 0;
    while((reg & (1<<0)) != (1<<0)) {
        reg = AIC_SPI0->RESERVED2;
    }

    AIC_SPI0->CR[0] = (0x0FUL << 8); // cs
    AIC_SPI0->CR[1] = 0x00UL;
    AIC_SPI0->CR[2] = (0x01UL << 5);
    AIC_SPI0->TCR   = 0x00UL;

    AIC_SPI0->MR0   |=(1<<11);
    cpusysctrl_oclkrs_set(0x800);
    cpusysctrl_oclkrc_set(0x800);
    AIC_SPI0->MR0   &=~(1<<11);
    return 0;
}

/**
 ****************************************************************************************
 * @brief spi read, slave mode, use DMA.
 * @Support: AIC8800M40
 * @param: bit_mode   0:8bit, 1:16bit, 2:24bit, 3:32bit
 ****************************************************************************************
 */
int spi0_slave_dma_read(unsigned int *buff_rd, unsigned int word_cnt_rd,int bit_mode)
{
    unsigned int bit_len, shift_len;
    bit_len = (bit_mode == 3) ? 32 : (bit_mode == 2) ? 24 : (bit_mode == 1) ? 16 : 8;
    shift_len = (bit_mode == 3) ? 2 : bit_mode;

    int ch;
    // dma channal for spi_rx
    ch = SPI0_RXDMA_CH_IDX;
    dma_erqcsr_set(REQ_CID_SPI_RX, ch);
    dma_ch_rqr_erqm_clrb(ch);
    dma_ch_rqr_erql_setb(ch);
    dma_ch_dar_set(ch, (unsigned int)buff_rd);
    dma_ch_sar_set(ch, (unsigned int)(&AIC_SPI0->IOR));
    if ((bit_len == 32) || (bit_len == 24)){
          dma_ch_tbl0cr_set(ch, ((1<<shift_len) | (REQ_TBL0 << DMA_CH_RQTYP_LSB) | (AHB_WORD << DMA_CH_DBUSU_LSB) |
                             (AHB_WORD << DMA_CH_SBUSU_LSB) | DMA_CH_CONSTSA_BIT));
    } else if (bit_len == 16){
          dma_ch_tbl0cr_set(ch, ((1<<shift_len) | (REQ_TBL0 << DMA_CH_RQTYP_LSB) | (AHB_HWORD << DMA_CH_DBUSU_LSB) |
                             (AHB_HWORD << DMA_CH_SBUSU_LSB) | DMA_CH_CONSTSA_BIT));
    } else if (bit_len == 8){
          dma_ch_tbl0cr_set(ch, ((1<<shift_len) | (REQ_TBL0 << DMA_CH_RQTYP_LSB) | (AHB_BYTE << DMA_CH_DBUSU_LSB) |
                             (AHB_BYTE << DMA_CH_SBUSU_LSB) | DMA_CH_CONSTSA_BIT));
    }
    //dma_ch_tbl0cr_set(ch, ((4) | (REQ_TBL0 << DMA_CH_RQTYP_LSB) | (AHB_WORD << DMA_CH_DBUSU_LSB) |
    //                         (AHB_WORD << DMA_CH_SBUSU_LSB) | DMA_CH_CONSTSA_BIT));
    dma_ch_tbl1cr_set(ch, word_cnt_rd<<shift_len);
    dma_ch_tbl2cr_set(ch, word_cnt_rd<<shift_len);
    dma_ch_tsr_set(ch, (((1<<shift_len) << DMA_CH_STRANSZ_LSB) | ((1<<shift_len) << DMA_CH_DTRANSZ_LSB)));
    dma_ch_wmar_set(ch, 0);
    dma_ch_wjar_set(ch, 0);
    dma_ch_lnar_set(ch, 0);
    dma_ch_tbl0sr_set(ch, ((0 << DMA_CH_STBL0SZ_LSB) | (0 << DMA_CH_DTBL0SZ_LSB)));
    dma_ch_tbl1ssr_set(ch, 0);
    dma_ch_tbl1dsr_set(ch, 0);
    #if SPI0_DMA_ISR_ENABLED
    dma_ch_icsr_set(ch, (DMA_CH_TBL2_IENA_BIT));
    #endif
    dma_ch_ctlr_set(ch, (DMA_CH_CHENA_BIT | (0x01UL << DMA_CH_BUSBU_LSB)));

    #if SPI0_ISR_ENABLED
    AIC_SPI0->IER = (0x01UL << 9);  // rxend: valid only when use DMA
    #endif

    AIC_SPI0->ICR   = word_cnt_rd; // in data cnt
    AIC_SPI0->CR[3] = (((0x1UL)&31)<<0) | (((0x2UL)&31)<<8);
    AIC_SPI0->CR[0] = (((bit_len)&31)<<2) | (1<<1);                   // transbit 32
    AIC_SPI0->MR0   = (((0x0)&7)<<3) | (0x01UL << 10)  | (1<<0);
    AIC_SPI0->CR[2] = (0x01UL << 5);
    AIC_SPI0->CR[1] = (((0x1UL)&3)<<12) | (0x01UL <<  6);        // recv trans
    AIC_SPI0->TCR   = (0x01UL << 0);                             // rx req start
    AIC_SPI0->CR[2] |= (0x01UL <<  6); // en dma

    #if SPI0_ISR_ENABLED
    rtos_semaphore_wait(spi0_rxend_sema, -1);
    #endif

    #if SPI0_DMA_ISR_ENABLED
    rtos_semaphore_wait(spi0_rxdma_sema, -1);
    #else
    while (dma_ch_icsr_tbl2_irst_getb(ch) == 0);
    dma_ch_icsr_set(ch, (dma_ch_icsr_get(ch) | DMA_CH_TBL2_ICLR_BIT));
    #endif

    uint32_t reg = 0;
    while((reg & (1<<0)) != (1<<0)) {
        reg = AIC_SPI0->RESERVED2;
    }

    AIC_SPI0->CR[0] = (0x0FUL << 8); // cs
    AIC_SPI0->CR[1] = 0x00UL;
    AIC_SPI0->CR[2] = (0x01UL << 5);
    AIC_SPI0->TCR   = 0x00UL;

    return 0;
}

/**
 ****************************************************************************************
 * @brief spi write and read word, master and slave mode, use DMA.
 * @Support: AIC8800M40
 ****************************************************************************************
 */
int spi0_dma_write_read_words(unsigned int *buff_wr, unsigned int word_cnt_wr,
                                       unsigned int *buff_rd, unsigned int word_cnt_rd, int mode)
{
    if(!mode){
        uint32_t cnt = 5;
        for (int i=0; i<cnt; i++) {
            AIC_SPI0->CR[0] = ((1<<1) | (1<<0));
            AIC_SPI0->CR[0] = 0;
        }
    }

    int ch;
    AIC_SPI0->CR[1] = 0x00UL;

    // dma channal for spi_tx
    ch = SPI0_TXDMA_CH_IDX;
    dma_erqcsr_set(REQ_CID_SPI_TX, ch);
    dma_ch_rqr_erqm_clrb(ch);
    dma_ch_rqr_erql_setb(ch);
    dma_ch_dar_set(ch, (unsigned int)(&AIC_SPI0->IOR));
    dma_ch_sar_set(ch, (unsigned int)buff_wr);
    dma_ch_tbl0cr_set(ch, ((4) | (REQ_TBL0 << DMA_CH_RQTYP_LSB) | (AHB_WORD << DMA_CH_DBUSU_LSB) |
                             (AHB_WORD << DMA_CH_SBUSU_LSB) | DMA_CH_CONSTDA_BIT));
    dma_ch_tbl1cr_set(ch, word_cnt_wr<<2);
    dma_ch_tbl2cr_set(ch, word_cnt_wr<<2);
    dma_ch_tsr_set(ch, ((4 << DMA_CH_STRANSZ_LSB) | (4 << DMA_CH_DTRANSZ_LSB)));
    dma_ch_wmar_set(ch, 0);
    dma_ch_wjar_set(ch, 0);
    dma_ch_lnar_set(ch, 0);
    dma_ch_tbl0sr_set(ch, ((0 << DMA_CH_STBL0SZ_LSB) | (0 << DMA_CH_DTBL0SZ_LSB)));
    dma_ch_tbl1ssr_set(ch, 0);
    dma_ch_tbl1dsr_set(ch, 0);
    #if SPI0_DMA_ISR_ENABLED
    dma_ch_icsr_set(ch, (DMA_CH_TBL2_IENA_BIT));
    #endif
    dma_ch_ctlr_set(ch, (DMA_CH_CHENA_BIT | (0x01UL << DMA_CH_BUSBU_LSB)));

    // dma channal for spi_rx
    ch = SPI0_RXDMA_CH_IDX;
    dma_erqcsr_set(REQ_CID_SPI_RX, ch);
    dma_ch_rqr_erqm_clrb(ch);
    dma_ch_rqr_erql_setb(ch);
    dma_ch_dar_set(ch, (unsigned int)buff_rd);
    dma_ch_sar_set(ch, (unsigned int)(&AIC_SPI0->IOR));
    dma_ch_tbl0cr_set(ch, ((4) | (REQ_TBL0 << DMA_CH_RQTYP_LSB) | (AHB_WORD << DMA_CH_DBUSU_LSB) |
                             (AHB_WORD << DMA_CH_SBUSU_LSB) | DMA_CH_CONSTSA_BIT));
    dma_ch_tbl1cr_set(ch, word_cnt_rd<<2);
    dma_ch_tbl2cr_set(ch, word_cnt_rd<<2);
    dma_ch_tsr_set(ch, ((4 << DMA_CH_STRANSZ_LSB) | (4 << DMA_CH_DTRANSZ_LSB)));
    dma_ch_wmar_set(ch, 0);
    dma_ch_wjar_set(ch, 0);
    dma_ch_lnar_set(ch, 0);
    dma_ch_tbl0sr_set(ch, ((0 << DMA_CH_STBL0SZ_LSB) | (0 << DMA_CH_DTBL0SZ_LSB)));
    dma_ch_tbl1ssr_set(ch, 0);
    dma_ch_tbl1dsr_set(ch, 0);
    #if SPI0_DMA_ISR_ENABLED
    dma_ch_icsr_set(ch, (DMA_CH_TBL2_IENA_BIT));
    #endif
    dma_ch_ctlr_set(ch, (DMA_CH_CHENA_BIT | (0x01UL << DMA_CH_BUSBU_LSB)));

    #if SPI0_ISR_ENABLED
    AIC_SPI0->IER = (0x01UL << 8) | (0x01UL << 9);
    #endif

    if (mode) { // master mode
        AIC_SPI0->OCR   = word_cnt_wr; // out data cnt
        AIC_SPI0->ICR   = word_cnt_rd; // in data cnt
        AIC_SPI0->OFCR  = (((30)&31)<<8);
        AIC_SPI0->CR[3] = ((0x02UL << 8) | (0x01UL << 0));
        AIC_SPI0->CR[1] = ((0x03UL << 12) | (0x01UL << 6)); // send and recv, cs, 3-wire
        AIC_SPI0->CR[0] = ((0x0EUL <<  8) | (0x01UL << 1) | (((32)&31) << 2) ); // cs, bit count
        AIC_SPI0->MR0   = ((0x01UL << 11) | (0x00UL << 3)); // slave, generic mode
        AIC_SPI0->CR[2] = (0x01UL <<  6); // en dma
        AIC_SPI0->TCR   = (0x01UL << 1); // trans start
        AIC_SPI0->CR[5] = (((0x20)&0xFFFF)<<0);
    }
    else { // slave mode
        AIC_SPI0->OCR   = word_cnt_wr; // out data cnt
        AIC_SPI0->ICR   = word_cnt_rd; // in data cnt
        AIC_SPI0->OFCR  = (((30)&31)<<8);
        AIC_SPI0->CR[3] = (((0x1UL)&31)<<0) | (((0x2UL)&31)<<8);
        AIC_SPI0->CR[0] = (((32)&31)<<2) | (1<<1);
        AIC_SPI0->MR0   = (((0x0)&7)<<3) | (0x01UL << 10)  | (1<<0);
        AIC_SPI0->CR[2] = (0x01UL << 5);
        AIC_SPI0->CR[1] = (((0x3UL)&3)<<12) | (0x01UL <<  6); // send and recv, cs, 3-wire
        AIC_SPI0->TCR   = (0x01UL << 1); // trans start
        AIC_SPI0->CR[2] |= (0x01UL <<  6); // en dma
    }

    #if SPI0_DMA_ISR_ENABLED
    rtos_semaphore_wait(spi0_rxdma_sema, -1);
    #else
    while (dma_ch_icsr_tbl2_irst_getb(ch) == 0);
    dma_ch_icsr_set(ch, (dma_ch_icsr_get(ch) | DMA_CH_TBL2_ICLR_BIT));
    #endif

    #if SPI0_ISR_ENABLED
    rtos_semaphore_wait(spi0_rxend_sema, -1);
    #endif

    while(AIC_SPI0->RESERVED2 == 0);
    //dbg("AIC_SPI0->RESERVED2: %x\n", AIC_SPI0->RESERVED2);

    if (mode) {
        AIC_SPI0->CR[2] = 0;
    } else {
        AIC_SPI0->CR[2] = (0x01UL << 5);
    }

    AIC_SPI0->CR[0] = (0x0FUL << 8); // cs
    AIC_SPI0->CR[1] = 0x00UL;
    AIC_SPI0->TCR   = 0x00UL;

    AIC_SPI0->MR0   |=(1<<11);
    cpusysctrl_oclkrs_set(0x800);
    cpusysctrl_oclkrc_set(0x800);
    AIC_SPI0->MR0   &=~(1<<11);
    return 0;
}

/**
 ****************************************************************************************
 * @brief spi write word, slave mode, use DMA.
 * @Support: AIC8800M40
 ****************************************************************************************
 */
int spi0_slave_dma_write_words(unsigned int *buff_wr, unsigned int word_cnt_wr)
{
    uint32_t cnt = 5;
    for (int i=0; i<cnt; i++) {
        AIC_SPI0->CR[0] = ((1<<1) | (1<<0));
        AIC_SPI0->CR[0] = 0;
    }

    int ch;
    // dma channal for spi_tx
    ch = SPI0_TXDMA_CH_IDX;
    dma_erqcsr_set(REQ_CID_SPI_TX, ch);
    dma_ch_rqr_erqm_clrb(ch);
    dma_ch_rqr_erql_setb(ch);
    dma_ch_dar_set(ch, (unsigned int)(&AIC_SPI0->IOR));
    dma_ch_sar_set(ch, (unsigned int)buff_wr);
    dma_ch_tbl0cr_set(ch, ((4) | (REQ_TBL0 << DMA_CH_RQTYP_LSB) | (AHB_WORD << DMA_CH_DBUSU_LSB) |
                             (AHB_WORD << DMA_CH_SBUSU_LSB) | DMA_CH_CONSTDA_BIT));
    dma_ch_tbl1cr_set(ch, word_cnt_wr<<2);
    dma_ch_tbl2cr_set(ch, word_cnt_wr<<2);
    dma_ch_tsr_set(ch, ((4 << DMA_CH_STRANSZ_LSB) | (4 << DMA_CH_DTRANSZ_LSB)));
    dma_ch_wmar_set(ch, 0);
    dma_ch_wjar_set(ch, 0);
    dma_ch_lnar_set(ch, 0);
    dma_ch_tbl0sr_set(ch, ((0 << DMA_CH_STBL0SZ_LSB) | (0 << DMA_CH_DTBL0SZ_LSB)));
    dma_ch_tbl1ssr_set(ch, 0);
    dma_ch_tbl1dsr_set(ch, 0);
    #if SPI0_DMA_ISR_ENABLED
    dma_ch_icsr_set(ch, (DMA_CH_TBL2_IENA_BIT));
    #endif
    dma_ch_ctlr_set(ch, (DMA_CH_CHENA_BIT | (0x01UL << DMA_CH_BUSBU_LSB)));

    #if SPI0_ISR_ENABLED
    AIC_SPI0->IER = (0x01UL << 8);  // txend: valid only when use DMA
    #endif

    AIC_SPI0->OCR   = word_cnt_wr; // out data cnt
    AIC_SPI0->OFCR  = (((30)&31)<<8);
    AIC_SPI0->CR[0] = (((32)&31)<<2) | (1<<1);                   // transbit 32
    AIC_SPI0->MR0   = (((0x0)&7)<<3) | (0x01UL << 10)  | (1<<0);
    AIC_SPI0->CR[2] = (0x01UL << 5);
    AIC_SPI0->CR[1] = (((0x2UL)&3)<<12) | (0x01UL <<  6);        // send trans
    AIC_SPI0->TCR   = (0x01UL << 1);                             // tx req start
    AIC_SPI0->CR[2] |= (0x01UL <<  6); // en dma

    uint32_t reg = 0xFFFFFFFF;
    while((reg & (1<<7)) == (1<<7)) {
        reg = AIC_SPI0->SR;
    }

    #if SPI0_DMA_ISR_ENABLED
    rtos_semaphore_wait(spi0_txdma_sema, -1);
    #else
    while (dma_ch_icsr_tbl2_irst_getb(ch) == 0);
    dma_ch_icsr_set(ch, (dma_ch_icsr_get(ch) | DMA_CH_TBL2_ICLR_BIT));
    #endif

    #if SPI0_ISR_ENABLED
    rtos_semaphore_wait(spi0_txend_sema, -1);
    #endif

    reg = 0;
    while((reg & (1<<0)) != (1<<0)) {
        reg = AIC_SPI0->RESERVED2;
    }

    AIC_SPI0->CR[0] = (0x0FUL << 8); // cs
    AIC_SPI0->CR[1] = 0x00UL;
    AIC_SPI0->CR[2] = (0x01UL << 5);
    AIC_SPI0->TCR   = 0x00UL;

    AIC_SPI0->MR0   |=(1<<11);
    cpusysctrl_oclkrs_set(0x800);
    cpusysctrl_oclkrc_set(0x800);
    AIC_SPI0->MR0   &=~(1<<11);
    return 0;
}

/**
 ****************************************************************************************
 * @brief spi read word, slave mode, use DMA.
 * @Support: AIC8800M40
 ****************************************************************************************
 */
int spi0_slave_dma_read_words(unsigned int *buff_rd, unsigned int word_cnt_rd)
{
    int ch;
    // dma channal for spi_rx
    ch = SPI0_RXDMA_CH_IDX;
    dma_erqcsr_set(REQ_CID_SPI_RX, ch);
    dma_ch_rqr_erqm_clrb(ch);
    dma_ch_rqr_erql_setb(ch);
    dma_ch_dar_set(ch, (unsigned int)buff_rd);
    dma_ch_sar_set(ch, (unsigned int)(&AIC_SPI0->IOR));
    dma_ch_tbl0cr_set(ch, ((4) | (REQ_TBL0 << DMA_CH_RQTYP_LSB) | (AHB_WORD << DMA_CH_DBUSU_LSB) |
                             (AHB_WORD << DMA_CH_SBUSU_LSB) | DMA_CH_CONSTSA_BIT));
    dma_ch_tbl1cr_set(ch, word_cnt_rd<<2);
    dma_ch_tbl2cr_set(ch, word_cnt_rd<<2);
    dma_ch_tsr_set(ch, ((4 << DMA_CH_STRANSZ_LSB) | (4 << DMA_CH_DTRANSZ_LSB)));
    dma_ch_wmar_set(ch, 0);
    dma_ch_wjar_set(ch, 0);
    dma_ch_lnar_set(ch, 0);
    dma_ch_tbl0sr_set(ch, ((0 << DMA_CH_STBL0SZ_LSB) | (0 << DMA_CH_DTBL0SZ_LSB)));
    dma_ch_tbl1ssr_set(ch, 0);
    dma_ch_tbl1dsr_set(ch, 0);
    #if SPI0_DMA_ISR_ENABLED
    dma_ch_icsr_set(ch, (DMA_CH_TBL2_IENA_BIT));
    #endif
    dma_ch_ctlr_set(ch, (DMA_CH_CHENA_BIT | (0x01UL << DMA_CH_BUSBU_LSB)));

    #if SPI0_ISR_ENABLED
    AIC_SPI0->IER = (0x01UL << 9);  // rxend: valid only when use DMA
    #endif

    AIC_SPI0->ICR   = word_cnt_rd; // in data cnt
    AIC_SPI0->CR[3] = (((0x1UL)&31)<<0) | (((0x2UL)&31)<<8);
    AIC_SPI0->CR[0] = (((32)&31)<<2) | (1<<1);                   // transbit 32
    AIC_SPI0->MR0   = (((0x0)&7)<<3) | (0x01UL << 10)  | (1<<0);
    AIC_SPI0->CR[2] = (0x01UL << 5);
    AIC_SPI0->CR[1] = (((0x1UL)&3)<<12) | (0x01UL <<  6);        // recv trans
    AIC_SPI0->TCR   = (0x01UL << 0);                             // rx req start
    AIC_SPI0->CR[2] |= (0x01UL <<  6); // en dma


    #if SPI0_ISR_ENABLED
    rtos_semaphore_wait(spi0_rxend_sema, -1);
    #endif

    #if SPI0_DMA_ISR_ENABLED
    rtos_semaphore_wait(spi0_rxdma_sema, -1);
    #else
    while (dma_ch_icsr_tbl2_irst_getb(ch) == 0);
    dma_ch_icsr_set(ch, (dma_ch_icsr_get(ch) | DMA_CH_TBL2_ICLR_BIT));
    #endif

    uint32_t reg = 0;
    while((reg & (1<<0)) != (1<<0)) {
        reg = AIC_SPI0->RESERVED2;
    }

    AIC_SPI0->CR[0] = (0x0FUL << 8); // cs
    AIC_SPI0->CR[1] = 0x00UL;
    AIC_SPI0->CR[2] = (0x01UL << 5);
    AIC_SPI0->TCR   = 0x00UL;

    return 0;
}

/**
 ****************************************************************************************
 * @brief spi write and read 16bit, master and slave mode, use DMA.
 * @Support: AIC8800M40
 ****************************************************************************************
 */
int spi0_dma_write_read_16bit(unsigned int *buff_wr, unsigned int wr_len,
                                       unsigned int *buff_rd, unsigned int rd_len, int mode)
{
{
    if(!mode){
        uint32_t cnt = 5;
        for (int i=0; i<cnt; i++) {
            AIC_SPI0->CR[0] = ((1<<1) | (1<<0));
            AIC_SPI0->CR[0] = 0;
        }
    }

    int ch;
    AIC_SPI0->CR[1] = 0x00UL;

    // dma channal for spi_tx
    ch = SPI0_TXDMA_CH_IDX;
    dma_erqcsr_set(REQ_CID_SPI_TX, ch);
    dma_ch_rqr_erqm_clrb(ch);
    dma_ch_rqr_erql_setb(ch);
    dma_ch_dar_set(ch, (unsigned int)(&AIC_SPI0->IOR));
    dma_ch_sar_set(ch, (unsigned int)buff_wr);
    dma_ch_tbl0cr_set(ch, ((2) | (REQ_TBL0 << DMA_CH_RQTYP_LSB) | (AHB_HWORD << DMA_CH_DBUSU_LSB) |
                             (AHB_HWORD << DMA_CH_SBUSU_LSB) | DMA_CH_CONSTDA_BIT));
    dma_ch_tbl1cr_set(ch, wr_len<<1);
    dma_ch_tbl2cr_set(ch, wr_len<<1);
    dma_ch_tsr_set(ch, ((2 << DMA_CH_STRANSZ_LSB) | (2 << DMA_CH_DTRANSZ_LSB)));
    dma_ch_wmar_set(ch, 0);
    dma_ch_wjar_set(ch, 0);
    dma_ch_lnar_set(ch, 0);
    dma_ch_tbl0sr_set(ch, ((0 << DMA_CH_STBL0SZ_LSB) | (0 << DMA_CH_DTBL0SZ_LSB)));
    dma_ch_tbl1ssr_set(ch, 0);
    dma_ch_tbl1dsr_set(ch, 0);
    #if SPI0_DMA_ISR_ENABLED
    dma_ch_icsr_set(ch, (DMA_CH_TBL2_IENA_BIT));
    #endif
    dma_ch_ctlr_set(ch, (DMA_CH_CHENA_BIT | (0x01UL << DMA_CH_BUSBU_LSB)));

    // dma channal for spi_rx
    ch = SPI0_RXDMA_CH_IDX;
    dma_erqcsr_set(REQ_CID_SPI_RX, ch);
    dma_ch_rqr_erqm_clrb(ch);
    dma_ch_rqr_erql_setb(ch);
    dma_ch_dar_set(ch, (unsigned int)buff_rd);
    dma_ch_sar_set(ch, (unsigned int)(&AIC_SPI0->IOR));
    dma_ch_tbl0cr_set(ch, ((2) | (REQ_TBL0 << DMA_CH_RQTYP_LSB) | (AHB_HWORD << DMA_CH_DBUSU_LSB) |
                             (AHB_HWORD << DMA_CH_SBUSU_LSB) | DMA_CH_CONSTSA_BIT));
    dma_ch_tbl1cr_set(ch, rd_len<<1);
    dma_ch_tbl2cr_set(ch, rd_len<<1);
    dma_ch_tsr_set(ch, ((2 << DMA_CH_STRANSZ_LSB) | (2 << DMA_CH_DTRANSZ_LSB)));
    dma_ch_wmar_set(ch, 0);
    dma_ch_wjar_set(ch, 0);
    dma_ch_lnar_set(ch, 0);
    dma_ch_tbl0sr_set(ch, ((0 << DMA_CH_STBL0SZ_LSB) | (0 << DMA_CH_DTBL0SZ_LSB)));
    dma_ch_tbl1ssr_set(ch, 0);
    dma_ch_tbl1dsr_set(ch, 0);
    #if SPI0_DMA_ISR_ENABLED
    dma_ch_icsr_set(ch, (DMA_CH_TBL2_IENA_BIT));
    #endif
    dma_ch_ctlr_set(ch, (DMA_CH_CHENA_BIT | (0x01UL << DMA_CH_BUSBU_LSB)));

    #if SPI0_ISR_ENABLED
    AIC_SPI0->IER = (0x01UL << 8) | (0x01UL << 9);
    #endif

    if (mode) { // master mode
        AIC_SPI0->OCR   = wr_len; // out data cnt
        AIC_SPI0->ICR   = rd_len; // in data cnt
        AIC_SPI0->OFCR  = (((30)&31)<<8);
        AIC_SPI0->CR[3] = ((0x02UL << 8) | (0x01UL << 0));
        AIC_SPI0->CR[1] = ((0x03UL << 12) | (0x01UL << 6)); // send and recv, cs, 3-wire
        AIC_SPI0->CR[0] = ((0x0EUL <<  8) | (0x01UL << 1) | (((16)&31) << 2) ); // cs, bit count
        AIC_SPI0->MR0   = ((0x01UL << 11) | (0x00UL << 3)); // slave, generic mode
        AIC_SPI0->CR[2] = (0x01UL <<  6); // en dma
        AIC_SPI0->TCR   = (0x01UL << 1); // trans start
        AIC_SPI0->CR[5] = (((0x20)&0xFFFF)<<0);
    }
    else { // slave mode
        AIC_SPI0->OCR   = wr_len; // out data cnt
        AIC_SPI0->ICR   = rd_len; // in data cnt
        AIC_SPI0->OFCR  = (((30)&31)<<8);
        AIC_SPI0->CR[3] = (((0x1UL)&31)<<0) | (((0x2UL)&31)<<8);
        AIC_SPI0->CR[0] = (((16)&31)<<2) | (1<<1);
        AIC_SPI0->MR0   = (((0x0)&7)<<3) | (0x01UL << 10)  | (1<<0);
        AIC_SPI0->CR[2] = (0x01UL << 5);
        AIC_SPI0->CR[1] = (((0x3UL)&3)<<12) | (0x01UL <<  6); // send and recv, cs, 3-wire
        AIC_SPI0->TCR   = (0x01UL << 1); // trans start

        AIC_SPI0->CR[2] |= (0x01UL <<  6); // en dma
    }

    #if SPI0_DMA_ISR_ENABLED
    rtos_semaphore_wait(spi0_rxdma_sema, -1);
    #else
    while (dma_ch_icsr_tbl2_irst_getb(ch) == 0);
    dma_ch_icsr_set(ch, (dma_ch_icsr_get(ch) | DMA_CH_TBL2_ICLR_BIT));
    #endif

    #if SPI0_ISR_ENABLED
    rtos_semaphore_wait(spi0_rxend_sema, -1);
    #endif

    while(AIC_SPI0->RESERVED2 == 0);
    //dbg("AIC_SPI0->RESERVED2: %x\n", AIC_SPI0->RESERVED2);

    if (mode) {
        AIC_SPI0->CR[2] = 0;
    } else {
        AIC_SPI0->CR[2] = (0x01UL << 5);
    }

    AIC_SPI0->CR[0] = (0x0FUL << 8); // cs
    AIC_SPI0->CR[1] = 0x00UL;
    AIC_SPI0->TCR   = 0x00UL;

    AIC_SPI0->MR0   |=(1<<11);
    cpusysctrl_oclkrs_set(0x800);
    cpusysctrl_oclkrc_set(0x800);
    AIC_SPI0->MR0   &=~(1<<11);
    return 0;
}

/**
 ****************************************************************************************
 * @brief spi write 16bit, slave mode, use DMA.
 * @Support: AIC8800M40
 ****************************************************************************************
 */
int spi0_slave_dma_write_16bit(unsigned int *buff_wr, unsigned int wr_len)
{
    uint32_t cnt = 5;
    for (int i=0; i<cnt; i++) {
        AIC_SPI0->CR[0] = ((1<<1) | (1<<0));
        AIC_SPI0->CR[0] = 0;
    }

    int ch;
    // dma channal for spi_tx
    ch = SPI0_TXDMA_CH_IDX;
    dma_erqcsr_set(REQ_CID_SPI_TX, ch);
    dma_ch_rqr_erqm_clrb(ch);
    dma_ch_rqr_erql_setb(ch);
    dma_ch_dar_set(ch, (unsigned int)(&AIC_SPI0->IOR));
    dma_ch_sar_set(ch, (unsigned int)buff_wr);
    dma_ch_tbl0cr_set(ch, ((2) | (REQ_TBL0 << DMA_CH_RQTYP_LSB) | (AHB_HWORD << DMA_CH_DBUSU_LSB) |
                             (AHB_HWORD << DMA_CH_SBUSU_LSB) | DMA_CH_CONSTDA_BIT));
    dma_ch_tbl1cr_set(ch, wr_len<<1);
    dma_ch_tbl2cr_set(ch, wr_len<<1);
    dma_ch_tsr_set(ch, ((2 << DMA_CH_STRANSZ_LSB) | (2 << DMA_CH_DTRANSZ_LSB)));
    dma_ch_wmar_set(ch, 0);
    dma_ch_wjar_set(ch, 0);
    dma_ch_lnar_set(ch, 0);
    dma_ch_tbl0sr_set(ch, ((0 << DMA_CH_STBL0SZ_LSB) | (0 << DMA_CH_DTBL0SZ_LSB)));
    dma_ch_tbl1ssr_set(ch, 0);
    dma_ch_tbl1dsr_set(ch, 0);
    #if SPI0_DMA_ISR_ENABLED
    dma_ch_icsr_set(ch, (DMA_CH_TBL2_IENA_BIT));
    #endif
    dma_ch_ctlr_set(ch, (DMA_CH_CHENA_BIT | (0x01UL << DMA_CH_BUSBU_LSB)));

    #if SPI0_ISR_ENABLED
    AIC_SPI0->IER = (0x01UL << 8);  // txend: valid only when use DMA
    #endif

    AIC_SPI0->OCR   = wr_len; // out data cnt
    AIC_SPI0->ICR   = wr_len; // out data cnt
    AIC_SPI0->OFCR  = (((30)&31)<<8);
    AIC_SPI0->CR[0] = (((16)&31)<<2) | (1<<1);                   // transbit 32
    AIC_SPI0->MR0   = (((0x0)&7)<<3) | (0x01UL << 10)  | (1<<0);
    AIC_SPI0->CR[2] = (0x01UL << 5);
    AIC_SPI0->CR[1] = (((0x2UL)&3)<<12) | (0x01UL <<  6);        // send trans
    AIC_SPI0->TCR   = (0x01UL << 1);                             // tx req start
    AIC_SPI0->CR[2] |= (0x01UL <<  6); // en dma

    uint32_t reg = 0xFFFFFFFF;
    while((reg & (1<<7)) == (1<<7)) {
        reg = AIC_SPI0->SR;
    }

    #if SPI0_DMA_ISR_ENABLED
    rtos_semaphore_wait(spi0_txdma_sema, -1);
    #else
    while (dma_ch_icsr_tbl2_irst_getb(ch) == 0);
    dma_ch_icsr_set(ch, (dma_ch_icsr_get(ch) | DMA_CH_TBL2_ICLR_BIT));
    #endif

    #if SPI0_ISR_ENABLED
    rtos_semaphore_wait(spi0_txend_sema, -1);
    #endif

    reg = 0;
    while((reg & (1<<0)) != (1<<0)) {
        reg = AIC_SPI0->RESERVED2;
    }

    AIC_SPI0->CR[0] = (0x0FUL << 8); // cs
    AIC_SPI0->CR[1] = 0x00UL;
    AIC_SPI0->CR[2] = (0x01UL << 5);
    AIC_SPI0->TCR   = 0x00UL;

    AIC_SPI0->MR0   |=(1<<11);
    cpusysctrl_oclkrs_set(0x800);
    cpusysctrl_oclkrc_set(0x800);
    AIC_SPI0->MR0   &=~(1<<11);
    return 0;
}

/**
 ****************************************************************************************
 * @brief spi read 16bit, slave mode, use DMA.
 * @Support: AIC8800M40
 ****************************************************************************************
 */
int spi0_slave_dma_read_16bit(unsigned int *buff_rd, unsigned int data_len)
{
    int ch;

    // dma channal for spi_rx
    ch = SPI0_RXDMA_CH_IDX;
    dma_erqcsr_set(REQ_CID_SPI_RX, ch);
    dma_ch_rqr_erqm_clrb(ch);
    dma_ch_rqr_erql_setb(ch);
    dma_ch_dar_set(ch, (unsigned int)buff_rd);
    dma_ch_sar_set(ch, (unsigned int)(&AIC_SPI0->IOR));
    dma_ch_tbl0cr_set(ch, ((2) | (REQ_TBL0 << DMA_CH_RQTYP_LSB) | (AHB_HWORD << DMA_CH_DBUSU_LSB) |
                             (AHB_HWORD << DMA_CH_SBUSU_LSB) | DMA_CH_CONSTSA_BIT));
    dma_ch_tbl1cr_set(ch, data_len<<1);
    dma_ch_tbl2cr_set(ch, data_len<<1);
    dma_ch_tsr_set(ch, ((2 << DMA_CH_STRANSZ_LSB) | (2 << DMA_CH_DTRANSZ_LSB)));
    dma_ch_wmar_set(ch, 0);
    dma_ch_wjar_set(ch, 0);
    dma_ch_lnar_set(ch, 0);
    dma_ch_tbl0sr_set(ch, ((0 << DMA_CH_STBL0SZ_LSB) | (0 << DMA_CH_DTBL0SZ_LSB)));
    dma_ch_tbl1ssr_set(ch, 0);
    dma_ch_tbl1dsr_set(ch, 0);
    #if SPI0_DMA_ISR_ENABLED
    dma_ch_icsr_set(ch, (DMA_CH_TBL2_IENA_BIT));
    #endif
    dma_ch_ctlr_set(ch, (DMA_CH_CHENA_BIT | (0x01UL << DMA_CH_BUSBU_LSB)));

    #if SPI0_ISR_ENABLED
    AIC_SPI0->IER = (0x01UL << 9);  // rxend: valid only when use DMA
    #endif

    AIC_SPI0->ICR   = data_len; // in data cnt
    AIC_SPI0->OCR   = data_len; // in data cnt
    AIC_SPI0->CR[3] = (((0x1UL)&31)<<0) | (((0x2UL)&31)<<8);
    AIC_SPI0->CR[0] = (((16)&31)<<2) | (1<<1);                   // transbit 32
    AIC_SPI0->MR0   = (((0x0)&7)<<3) | (0x01UL << 10)  | (1<<0);
    AIC_SPI0->CR[2] = (0x01UL << 5);
    AIC_SPI0->CR[1] = (((0x1UL)&3)<<12) | (0x01UL <<  6);        // recv trans
    AIC_SPI0->TCR   = (0x01UL << 0);                             // rx req start
    AIC_SPI0->CR[2] |= (0x01UL <<  6); // en dma

    #if SPI0_ISR_ENABLED
    rtos_semaphore_wait(spi0_rxend_sema, -1);
    #endif

    #if SPI0_DMA_ISR_ENABLED
    rtos_semaphore_wait(spi0_rxdma_sema, -1);
    #else
    while (dma_ch_icsr_tbl2_irst_getb(ch) == 0);
    dma_ch_icsr_set(ch, (dma_ch_icsr_get(ch) | DMA_CH_TBL2_ICLR_BIT));
    #endif

    uint32_t reg = 0;
    while((reg & (1<<0)) != (1<<0)) {
        reg = AIC_SPI0->RESERVED2;
    }

    AIC_SPI0->CR[0] = (0x0FUL << 8); // cs
    AIC_SPI0->CR[1] = 0x00UL;
    AIC_SPI0->CR[2] = (0x01UL << 5);
    AIC_SPI0->TCR   = 0x00UL;

    return 0;
}

#if SPI0_ISR_ENABLED
void spi0_isr(void)
{
    uint32_t reg;
    uint32_t int_st = AIC_SPI0->IMSR;
    //dbg("spi0_isr: %08x\n", int_st);
    AIC_SPI0->ICLR = AIC_SPI0->IRSR;  // int clr
    if (int_st & (0x01UL<<6)) {                       // Rx fifo full interrupt
        //AIC_SPI0->ICLR |= (0x01UL << 0); // int clr
        reg = AIC_SPI0->IER;
        AIC_SPI0->IER = reg & (~(0x01UL << 6));         // disable int
        rtos_semaphore_signal(spi0_rx_full_sema, true);
    }
    if (int_st & (0x01UL<<7)) {                  // Tx fifo empty interrupt
        //AIC_SPI0->ICLR |= (0x01UL << 3); // int clr
        reg = AIC_SPI0->IER;
        AIC_SPI0->IER = reg & (~(0x01UL << 7));         // disable int
        rtos_semaphore_signal(spi0_tx_empty_sema, true);
    }
    if (int_st & (0x01UL<<8)) {                  // txend interrupt
        rtos_semaphore_signal(spi0_txend_sema, true);
    }
    if (int_st & (0x01UL<<9)) {                  // rxend interrupt
        rtos_semaphore_signal(spi0_rxend_sema, true);
    }
}
#endif

#if SPI0_DMA_ISR_ENABLED
void spi0_rxdma_isr(void)
{
    const int ch = SPI0_RXDMA_CH_IDX;
    dma_ch_icsr_set(ch, (dma_ch_icsr_get(ch) | DMA_CH_TBL2_ICLR_BIT));
    dma_ch_ctlr_set(ch, 0);
    rtos_semaphore_signal(spi0_rxdma_sema, true);
}

void spi0_txdma_isr(void)
{
    const int ch = SPI0_TXDMA_CH_IDX;
    dma_ch_icsr_set(ch, (dma_ch_icsr_get(ch) | DMA_CH_TBL2_ICLR_BIT));
    dma_ch_ctlr_set(ch, 0);
    rtos_semaphore_signal(spi0_txdma_sema, true);
}
#endif

// master
int do_master_cmd(int argc, char * const argv[])
{
    #if PLF_AIC8800M40
    //spi0_dma_write_read_words(tx_buff, BUFFER_LENGTH, rx_buff, BUFFER_LENGTH, 1);
    spi0_dma_write_read_16bit(tx_buff, BUFFER_LENGTH, rx_buff, BUFFER_LENGTH, 1);
    #endif

    USER_PRINTF("\nmaster buf_rd:\n");
    for (int i = 0; i < BUFFER_LENGTH; i++) {
        USER_PRINTF(" %08x", rx_buff[i]);
        if ((i+1)%8 == 0 && i != 0)
            dbg("\n");
    }
    USER_PRINTF("\nspi master trans test done\n");
    return 0;
}

//slave
int do_slave_cmd(int argc, char * const argv[])
{
    #if PLF_AIC8800MC
    /*  No DMA write and read interface  */
    spi0_slave_write_read_words(tx_buff, BUFFER_LENGTH, rx_buff, BUFFER_LENGTH);
    //spi0_slave_write_words(tx_buff, BUFFER_LENGTH);
    //spi0_slave_read_words(rx_buff, BUFFER_LENGTH);
    #endif

    #if PLF_AIC8800M40
    /*  DMA word write and read interface  */
    //spi0_dma_write_read_words(tx_buff, BUFFER_LENGTH, rx_buff, BUFFER_LENGTH, 0);
    //spi0_slave_dma_write_words(tx_buff, BUFFER_LENGTH);
    //spi0_slave_dma_read_words(rx_buff, BUFFER_LENGTH);
    /*  DMA 16bit write and read interface  */
    spi0_dma_write_read_16bit(tx_buff, BUFFER_LENGTH, rx_buff, BUFFER_LENGTH, 0);
    //spi0_slave_dma_write_16bit(tx_buff, BUFFER_LENGTH);
    //spi0_slave_dma_read_16bit(rx_buff, BUFFER_LENGTH);
    /*  No DMA write and read interface  */
    //spi0_slave_write_words(tx_buff, BUFFER_LENGTH);
    //spi0_slave_read_words(rx_buff, BUFFER_LENGTH);
    #endif

    USER_PRINTF("\nslave buf_rd:\n");
    for (int i = 0; i < BUFFER_LENGTH; i++) {
        USER_PRINTF(" %08x", rx_buff[i]);
        if ((i+1)%8 == 0 && i != 0)
            dbg("\n");
    }
    USER_PRINTF("\nspi slave trans test done\n");
    return 0;
}

/**
 ****************************************************************************************
 * @brief test task implementation.
 ****************************************************************************************
 */
void spi_slave_test(void)
{
    USER_PRINTF("\nStart test case: spi trans\n\n");

    /* prepare data to write */
    USER_PRINTF("tx_buff:\n");
    for(int i=0; i<BUFFER_LENGTH; i++) {
        rx_buff[i] = 0;
        #if SPI_MASTER_OR_SLAVE
        tx_buff[i] = 3<<24 | 3<<16 | 3<<8 | (i&0xff);
        #else
        tx_buff[i] = 1<<24 | 1<<16 | 1<<8 | (i&0xff);
        #endif
        USER_PRINTF(" %08x", tx_buff[i]);
        if ((i+1)%8 == 0 && i != 0)
            dbg("\n");
    }

    /* add console */
    #if SPI_MASTER_OR_SLAVE
    dbg("\nSPI_MASTER_MODE\n");
    console_cmd_add("ms_cmd", "master test cmd", 1,  1, do_master_cmd);
    #else
    dbg("\nSPI_SLAVE_MODE\n");
    console_cmd_add("sl_cmd", "slave test cmd", 1,  1, do_slave_cmd);
    #endif

    spi0_init();
}

#endif /* CFG_TEST_SPI_SLAVE */
