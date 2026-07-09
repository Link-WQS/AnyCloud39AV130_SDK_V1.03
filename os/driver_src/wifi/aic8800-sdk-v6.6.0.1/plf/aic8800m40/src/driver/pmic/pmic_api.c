/**
 ****************************************************************************************
 *
 * @file pmic_api.c
 *
 * @brief PMIC API functions
 *
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "pmic_api.h"
#include "ll.h"
#include "dbg.h"
#include "system.h"
#include "sysctrl_api.h"
#include "pwrkey_api.h"
#include "rom_api.h"
#include "reg_anareg.h"
#include "dual_dcdc_rf_ctrl.h"
#include "msadc_api.h"

#define PMIC_DEBUG_PRINTF(fmt, ...)         do {} while(0) //dbg(fmt, ##__VA_ARGS__)//

void PMU_RTC0_IRQHandler(void)
{
    uint32_t reg_val, int_st_masked;
    reg_val = psim_read((unsigned int)(&AIC_PMU_RTC0->pmu_rtc0_cfg4));
    int_st_masked = (reg_val & PMU_RTC0_RTC_RG_INT_MASK_STATUS_MASK) >> PMU_RTC0_RTC_RG_INT_MASK_STATUS_LSB;
    if (int_st_masked & (0x01UL << PMIC_PWRKEY_POS_IRQn)) {
        pwrkey_irq_handler(PWRKEY_IRQ_EVENT_KEYDOWN);
        psim_mask_write((unsigned int)(&AIC_PMU_RTC0->pmu_rtc0_cfg4),
            PMU_RTC0_RTC_RG_INT_CLR1, PMU_RTC0_RTC_RG_INT_CLR1);
    } else if (int_st_masked & (0x01UL << PMIC_PWRKEY_NEG_IRQn)) {
        pwrkey_irq_handler(PWRKEY_IRQ_EVENT_KEYUP);
        psim_mask_write((unsigned int)(&AIC_PMU_RTC0->pmu_rtc0_cfg4),
            PMU_RTC0_RTC_RG_INT_CLR2, PMU_RTC0_RTC_RG_INT_CLR2);
    }
}

void pmic_pmu_rtc0_irq_enable(uint32_t int_en)
{
    int_en = (int_en << PMU_RTC0_RTC_RG_INT_MASK_LSB) & PMU_RTC0_RTC_RG_INT_MASK_MASK;
    if (int_en) {
        int_en |= (0x01UL << 0); // rtc_rg_int_en
        psim_mask_write((unsigned int)(&AIC_PMU_RTC0->pmu_rtc0_cfg4), int_en, int_en);
        pmic_pmu_rtc0_irq_enable_local();
    }
}

void pmic_pmu_rtc0_irq_enable_local(void)
{
    if (!NVIC_GetEnableIRQ(PMU_RTC0_IRQn)) {
        NVIC_SetPriority(PMU_RTC0_IRQn, __NVIC_PRIO_LOWEST);
        NVIC_EnableIRQ(PMU_RTC0_IRQn);
    }
}

int pmic_powerkey_enable(unsigned int reb_sec)
{
    int ret = 0;
    #if 0
    uint32_t reg_val, int_st_raw;
    reg_val = psim_read((unsigned int)(&AIC_PMU_RTC0->pmu_rtc0_cfg4));
    int_st_raw = (reg_val & PMU_RTC0_RTC_RG_INT_RAW_STATUS_MASK) >> PMU_RTC0_RTC_RG_INT_RAW_STATUS_LSB;
    #endif
    pmic_pmu_rtc0_irq_enable((0x01UL << PMIC_PWRKEY_POS_IRQn) | (0x01UL << PMIC_PWRKEY_NEG_IRQn));
    return ret;
}

void pmic_powerkey_devmode_enable(void)
{
    psim_mask_write((unsigned int)(&AIC_PMU_RTC0->pmu_rtc0_cfg0),
                    (PMU_RTC0_RTC_RG_FORCE_PBINT_NEG_PWROFF),
                    (PMU_RTC0_RTC_RG_FORCE_PBINT_NEG_PWROFF));
}

void pmic_chip_shutdown(void)
{
    PMIC_DEBUG_PRINTF("Shutdown...\r\n");
    ///TODO:
    psim_mask_write((unsigned int)(&AIC_PMU_RTC1->pmu_pd_ctrl1),
                    (PMU_RTC1_RTC_RG_PWR_OFF_SEQ_EN),
                    (PMU_RTC1_RTC_RG_PWR_OFF_SEQ_EN));
    __DSB();
    __ISB();
    __WFI();
    while (1);
}

__RAMTEXT void pmic_chip_shutdown_ram(void)
{
    ROM_PsimMaskWrite((unsigned int)(&AIC_PMU_RTC1->pmu_pd_ctrl1),
                    (PMU_RTC1_RTC_RG_PWR_OFF_SEQ_EN),
                    (PMU_RTC1_RTC_RG_PWR_OFF_SEQ_EN));
    while (1);
}

void pmic_chip_reboot(unsigned int dly_ticks)
{
    PMIC_DEBUG_PRINTF("Reboot:%x\n", dly_ticks);
    psim_mask_write((unsigned int)(&AIC_PMU_RTC0->pmu_rtc0_cfg3),
        (PMU_RTC0_RTC_RG_WDG_RSTN_REPOWER_EN | PMU_RTC0_RTC_RG_WDG_RSTN_PD_EN | 0),
        (PMU_RTC0_RTC_RG_WDG_RSTN_REPOWER_EN | PMU_RTC0_RTC_RG_WDG_RSTN_PD_EN |
        PMU_RTC0_RTC_RG_WDG_RSTN_POWER_OFF_RTC0_EN));
    psim_mask_write((unsigned int)(&AIC_PMU_RTC0->pmu_rtc0_cfg0),
        PMU_RTC0_PMU_RTC0_RG_UPDATE, PMU_RTC0_PMU_RTC0_RG_UPDATE);
    psim_write((unsigned int)(&AIC_WDT4->LOAD), dly_ticks);
    psim_mask_write((unsigned int)(&AIC_WDT4->CTRL), WDG_WDG_RUN, WDG_WDG_RUN);
    psim_write((unsigned int)(&AIC_PMU_RTC0->rtc_rg_reserved1), 0); // reset rtc epoch time
    __DSB();
    __ISB();
    __WFI();
    while (1);
}

void pmic_wdt_init(unsigned int seconds)
{
    unsigned int ticks = seconds << 15; // with 32768Hz freq
    psim_mask_write((unsigned int)(&AIC_PMU_RTC0->pmu_rtc0_cfg3),
        (PMU_RTC0_RTC_RG_WDG_RSTN_REPOWER_EN | PMU_RTC0_RTC_RG_WDG_RSTN_PD_EN | 0),
        (PMU_RTC0_RTC_RG_WDG_RSTN_REPOWER_EN | PMU_RTC0_RTC_RG_WDG_RSTN_PD_EN |
        PMU_RTC0_RTC_RG_WDG_RSTN_POWER_OFF_RTC0_EN));
    psim_mask_write((unsigned int)(&AIC_PMU_RTC0->pmu_rtc0_cfg0),
        PMU_RTC0_PMU_RTC0_RG_UPDATE, PMU_RTC0_PMU_RTC0_RG_UPDATE);
    psim_write((unsigned int)(&AIC_PMU_RTC0->pmu_rtc0_wdg4_cnt_ini),
        PMU_RTC0_RTC_RG_WDG4_CNT_INI(ticks));
    psim_write((unsigned int)(&AIC_WDT4->LOAD), ticks);
    psim_mask_write((unsigned int)(&AIC_WDT4->CTRL), WDG_WDG_RUN, WDG_WDG_RUN);
}

void pmic_wdt_kick(void)
{
    unsigned int ticks = psim_read((unsigned int)(&AIC_WDT4->LOAD));
    psim_write((unsigned int)(&AIC_WDT4->LOAD), ticks);
}

void pmic_wdt_pause(void)
{
    psim_mask_write((unsigned int)(&AIC_WDT4->CTRL), 0, WDG_WDG_RUN);
}

void pmic_wdt_continue(void)
{
    psim_mask_write((unsigned int)(&AIC_WDT4->CTRL), WDG_WDG_RUN, WDG_WDG_RUN);
}

unsigned int pmic_xtal_cap_get(void)
{
    unsigned int reg_val = psim_read((unsigned int)(&AIC_PMU_RTC1->pmu_clk_ctrl1));
    return (reg_val & PMU_RTC1_RTC_RG_XTAL_CAP_NOR_BIT_MASK) >> PMU_RTC1_RTC_RG_XTAL_CAP_NOR_BIT_LSB;
}

/**
 * work with cm_rf_cm_xtal_cap_fine_bit_setf
 */
void pmic_xtal_cap_set(unsigned int xtal_cap)
{
    psim_mask_write((unsigned int)(&AIC_PMU_RTC1->pmu_clk_ctrl1),
        (PMU_RTC1_RTC_RG_XTAL_CAP_NOR_BIT(xtal_cap)),
        (PMU_RTC1_RTC_RG_XTAL_CAP_NOR_BIT_MASK));
}

void pmic_dcdc_rf_set(POWER_MODE_LEVEL_T level)
{
}

void pmic_ldo_vrtc08_set(POWER_MODE_LEVEL_T level)
{
}

void pmic_ldo_vcore08_set(POWER_MODE_LEVEL_T level)
{
}

__RAMTEXT void pmic_vflash_init(int vflash_sel)
{
    if (vflash_sel == VFLASH_SEL_HIGH_VOLT) {
        // set vflash to 3.3V, use rom code
        ROM_PsimMaskWrite((unsigned int)(&AIC_PMU_RTC1->pmu_ldo_vflash_ctrl),
            (PMU_RTC1_RTC_RG_LDO_VFLASH_VBIT_NOR_TUNE(0xE)), // set vflash: 0xC=3.1V, 0xE=3.3V
            (PMU_RTC1_RTC_RG_LDO_VFLASH_VBIT_NOR_TUNE_MASK));
        AIC_ANAREG1->flash_ctrl &= ~ANALOG_REG1_RF_CFG_ANA_FLS_MS;
    } else {
        // set vflash to 1.8V, use rom code
        ROM_PsimMaskWrite((unsigned int)(&AIC_PMU_RTC1->pmu_ldo_vflash_ctrl),
            (PMU_RTC1_RTC_RG_LDO_VFLASH_VBIT_NOR_TUNE(0x5)), // set vflash=1.8V
            (PMU_RTC1_RTC_RG_LDO_VFLASH_VBIT_NOR_TUNE_MASK));
        AIC_ANAREG1->flash_ctrl |= ANALOG_REG1_RF_CFG_ANA_FLS_MS;
    }
    // delay after flash ms bit changed
    {
        int idx;
        for (idx = 0; idx < 64; idx++) {
            __NOP(); __NOP();
        }
    }
}

void pmic_pmu_init(void)
{
    // 0x70001040, ldo_vrtc08 drop
    psim_mask_write((unsigned int)(&AIC_PMU_RTC1->pmu_ldo_vrtc08_ctrl),
                    (PMU_RTC1_RTC_RG_LDO_VRTC08_VOSEL_NOR_TUNE(0x15)),
                    (PMU_RTC1_RTC_RG_LDO_VRTC08_VOSEL_NOR_TUNE_MASK));
    // 0x7000103C, ldo_vcore08 drop
    psim_mask_write((unsigned int)(&AIC_PMU_RTC1->pmu_ldo_vcore08_ctrl),
                    (PMU_RTC1_RTC_RG_LDO_VCORE08_VOSEL_NOR_TUNE(0x11) |
                     PMU_RTC1_RTC_RG_LDO_VCORE08_DVFS_STEP_VOL(0x1) |
                     PMU_RTC1_RTC_RG_LDO_VCORE08_DVFS_STEP_EN |
                     PMU_RTC1_RTC_RG_LDO_VCORE08_DVFS_STEP_DELAY(0x2)),
                    (PMU_RTC1_RTC_RG_LDO_VCORE08_VOSEL_NOR_TUNE_MASK |
                     PMU_RTC1_RTC_RG_LDO_VCORE08_DVFS_STEP_VOL_MASK |
                     PMU_RTC1_RTC_RG_LDO_VCORE08_DVFS_STEP_EN |
                     PMU_RTC1_RTC_RG_LDO_VCORE08_DVFS_STEP_DELAY_MASK));
    // 0x700010A0
    psim_mask_write((unsigned int)(&AIC_PMU_RTC1->pmu_dcdc_core_ctrl3),
                    (PMU_RTC1_RTC_RG_DCDC_CORE_NOVERLAP_ANA_PW |
                     PMU_RTC1_RTC_RG_DCDC_CORE_SW_2_RF),
                    (PMU_RTC1_RTC_RG_DCDC_CORE_NOVERLAP_ANA_PW |
                     PMU_RTC1_RTC_RG_DCDC_CORE_SW_2_RF));
    // 0x70002144
    psim_mask_write((unsigned int)(&AIC_DCDC_RFCTRL->dcdc_ctrl_cfg5),
                    (DUAL_DCDC_RF_CTRL_RG_DCDC_DCM_CLK_INT_DN_TH(0xFF)),
                    (DUAL_DCDC_RF_CTRL_RG_DCDC_DCM_CLK_INT_DN_TH_MASK));
    // 0x70001030
    psim_mask_write((unsigned int)(&AIC_PMU_RTC1->pmu_dcdc_rf_ctrl1),
                    (PMU_RTC1_RTC_RG_DCDC_RF_PULSE_TIME(0x0)),
                    (PMU_RTC1_RTC_RG_DCDC_RF_PULSE_TIME_MASK));
    // 0x70001034
    psim_mask_write((unsigned int)(&AIC_PMU_RTC1->pmu_dcdc_rf_ctrl2),
                    (PMU_RTC1_RTC_RG_DCDC_RF_NOVERLAP_BIT(0x0)),
                    (PMU_RTC1_RTC_RG_DCDC_RF_NOVERLAP_BIT_MASK));
    // 0x700021D0 bit5=1@wf_bt_pu, bit5=0@wf_bt_pd
    psim_mask_write((unsigned int)(&AIC_DCDC_RFCTRL->dcdc_ctrl_cfg40),
                    (DUAL_DCDC_RF_CTRL_RG_DCDC_MODE_FLAG_FRC_VALUE),
                    (DUAL_DCDC_RF_CTRL_RG_DCDC_MODE_FLAG_FRC_VALUE));
    // 0x70001038
    psim_mask_write((unsigned int)(&AIC_PMU_RTC1->pmu_dcdc_rf_ctrl3),
                    (PMU_RTC1_RTC_RG_DCDC_RF_NOVERLAP_ANA_PW),
                    (PMU_RTC1_RTC_RG_DCDC_RF_NOVERLAP_ANA_PW));
    // 0x7000102C
    psim_mask_write((unsigned int)(&AIC_PMU_RTC1->pmu_dcdc_rf_ctrl0),
                    (PMU_RTC1_RTC_RG_DCDC_RF_BIT_VFB_DECAP(0x7)),
                    (PMU_RTC1_RTC_RG_DCDC_RF_BIT_VFB_DECAP_MASK));
    // 0x7000216C
    psim_mask_write((unsigned int)(&AIC_DCDC_RFCTRL->dcdc_ctrl_cfg15),
                    (DUAL_DCDC_RF_CTRL_RG_DCDC_NLG_SCALE3(0x1) |
                    DUAL_DCDC_RF_CTRL_RG_DCDC_NLG_SCALE2(0x2) |
                    DUAL_DCDC_RF_CTRL_RG_DCDC_NLG_SCALE1(0x2)),
                    (DUAL_DCDC_RF_CTRL_RG_DCDC_NLG_SCALE3_MASK |
                    DUAL_DCDC_RF_CTRL_RG_DCDC_NLG_SCALE2_MASK |
                    DUAL_DCDC_RF_CTRL_RG_DCDC_NLG_SCALE1_MASK));
    // 0x700010A8
    psim_mask_write((unsigned int)(&AIC_PMU_RTC1->pmu_clk_ctrl4),
                    (PMU_RTC1_RTC_RG_CM_XDBLR_DLL_MODE_EN |
                     PMU_RTC1_RTC_RG_CM_XDBLR_DUTY_EN),
                    (PMU_RTC1_RTC_RG_CM_XDBLR_DLL_MODE_EN |
                     PMU_RTC1_RTC_RG_CM_XDBLR_DUTY_EN));
    // 0x70001024
    psim_mask_write((unsigned int)(&AIC_PMU_RTC1->pmu_clk_ctrl3),
                    (PMU_RTC1_RTC_RG_XTAL_IBIAS_NOR_BIT(0x0A)),
                    (PMU_RTC1_RTC_RG_XTAL_IBIAS_NOR_BIT_MASK));
    // 0x70001084
    psim_mask_write((unsigned int)(&AIC_PMU_RTC1->pmu_cm_xtal_cfg1),
                    (PMU_RTC1_RTC_RG_CM_XTAL_REG_VBIT(0x7) |
                    0 | //xtal_reg_bypass
                    0 | //xtal_rf_on_reg_bypass
                     PMU_RTC1_RTC_RG_CM_XTAL_AVDD_SEL),
                    (PMU_RTC1_RTC_RG_CM_XTAL_REG_VBIT_MASK |
                     PMU_RTC1_RTC_RG_CM_XTAL_REG_BYPASS |
                     PMU_RTC1_RTC_RG_CM_XTAL_RF_ON_REG_BYPASS |
                     PMU_RTC1_RTC_RG_CM_XTAL_AVDD_SEL));
    // 0x70001044
    psim_mask_write((unsigned int)(&AIC_PMU_RTC1->pmu_other_ldo_ctrl0),
                    (PMU_RTC1_RTC_RG_LDO_AVDD18_VBIT_NOR_TUNE(0x4)),
                    (PMU_RTC1_RTC_RG_LDO_AVDD18_VBIT_NOR_TUNE_MASK));
    // 0x700021C4
    psim_mask_write((unsigned int)(&AIC_DCDC_RFCTRL->dcdc_ctrl_cfg37),
                    (DUAL_DCDC_RF_CTRL_RG_DCDC_GAIN3_LIMIT_VTH(0x10)),
                    (DUAL_DCDC_RF_CTRL_RG_DCDC_GAIN3_LIMIT_VTH_MASK));
    // 0x70001028
    psim_mask_write((unsigned int)(&AIC_PMU_RTC1->pmu_rstn_ctrl),
                    (PMU_RTC1_RTC_RG_VBIT_UVLO(0x0)),
                    (PMU_RTC1_RTC_RG_VBIT_UVLO_MASK));
    // 0x700021CC
    psim_mask_write((unsigned int)(&AIC_DCDC_RFCTRL->dcdc_ctrl_cfg39),
                    (0), // nmos_pmos_sw_mode
                    (DUAL_DCDC_RF_CTRL_RG_DCDC_EN_NMOS_PMOS_SW_MODE));

    // 0x700021DC
    psim_mask_write((unsigned int)(&AIC_DCDC_RFCTRL->dcdc_ctrl_cfg43),
                    (DUAL_DCDC_RF_CTRL_RG_DCDC_DPWM_FRC_ON_DEEPSLEEP),
                    (DUAL_DCDC_RF_CTRL_RG_DCDC_DPWM_FRC_ON_DEEPSLEEP));

    // 0x70002190
    psim_mask_write((unsigned int)(&AIC_DCDC_RFCTRL->dcdc_ctrl_cfg24),
                    (DUAL_DCDC_RF_CTRL_RG_DCDC_HYS_PFM_GAIN3_DEEPSLEEP(0x8)),
                    (DUAL_DCDC_RF_CTRL_RG_DCDC_HYS_PFM_GAIN3_DEEPSLEEP_MASK));
    // 0x70001000
    psim_mask_write((unsigned int)(&AIC_PMU_RTC1->pmu_pd_ctrl0),
                    (((syscfg_predefined.sys_vrf_sel == VRF_SEL_DCDC_MODE) ? PMU_RTC1_RTC_RG_DCDC_LDO_RF_MODE_SEL : 0)),
                    (PMU_RTC1_RTC_RG_DCDC_LDO_RF_MODE_SEL));
}

void pmic_lpmode_init(void)
{
    uint32_t vrf13_vbit_ds_sw, vrtc1_ds_sw, vcore08_ds_sw;

    // 0x70001040
    vrtc1_ds_sw = (msadc_cal_res.cal_res_flags & CAL_RES_VRTC1_LP_REG_VALID) ? msadc_cal_res.vrtc1_lp_reg : 0x8;
    psim_mask_write((unsigned int)(&AIC_PMU_RTC1->pmu_ldo_vrtc08_ctrl),
        (PMU_RTC1_RTC_RG_LDO_VRTC08_VOSEL_DS_SW(vrtc1_ds_sw) |
        PMU_RTC1_RTC_RG_LDO_VRTC08_DVFS_STEP_VOL(0x2) |
        PMU_RTC1_RTC_RG_LDO_VRTC08_DVFS_STEP_EN),
        (PMU_RTC1_RTC_RG_LDO_VRTC08_VOSEL_DS_SW_MASK |
        PMU_RTC1_RTC_RG_LDO_VRTC08_DVFS_STEP_VOL_MASK |
        PMU_RTC1_RTC_RG_LDO_VRTC08_DVFS_STEP_EN));
    // 0x7000103C
    vcore08_ds_sw = (syscfg_predefined.pmic_vcore_drop_en) ? vrtc1_ds_sw : 0xC;
    psim_mask_write((unsigned int)(&AIC_PMU_RTC1->pmu_ldo_vcore08_ctrl),
        (PMU_RTC1_RTC_RG_LDO_VCORE08_VOSEL_DS_SW(vcore08_ds_sw)),
        (PMU_RTC1_RTC_RG_LDO_VCORE08_VOSEL_DS_SW_MASK));
    // 0x7000102C
    psim_mask_write((unsigned int)(&AIC_PMU_RTC1->pmu_dcdc_rf_ctrl0),
        (PMU_RTC1_RTC_RG_DCDC_RF_DVFS_STEP_VOL(0x1) |
        PMU_RTC1_RTC_RG_DCDC_RF_DVFS_STEP_EN |
        PMU_RTC1_RTC_RG_DCDC_RF_DVFS_STEP_DELAY(0x0)),
        (PMU_RTC1_RTC_RG_DCDC_RF_DVFS_STEP_VOL_MASK |
        PMU_RTC1_RTC_RG_DCDC_RF_DVFS_STEP_EN |
        PMU_RTC1_RTC_RG_DCDC_RF_DVFS_STEP_DELAY_MASK));
    // 0x70001098
    //psim_mask_write((unsigned int)(&AIC_PMU_RTC1->pmu_dcdc_core_ctrl1),
    //    PMU_RTC1_RTC_RG_DCDC_CORE_VBIT_NOR_RF(0x16),
    //    PMU_RTC1_RTC_RG_DCDC_CORE_VBIT_NOR_RF_MASK);
    // 0x70001030
    vrf13_vbit_ds_sw = (msadc_cal_res.cal_res_flags & CAL_RES_AVDD13_LP_VREF_VALID) ? msadc_cal_res.avdd13_lp_vref : 0x6;
    psim_mask_write((unsigned int)(&AIC_PMU_RTC1->pmu_dcdc_rf_ctrl1),
        (PMU_RTC1_RTC_RG_DCDC_RF_VBIT_DS_SW(vrf13_vbit_ds_sw) |
        PMU_RTC1_RTC_RG_DCDC_RF_VBIT_VOUT_VALLEY_DELTA(0x2)),
        (PMU_RTC1_RTC_RG_DCDC_RF_VBIT_DS_SW_MASK |
        PMU_RTC1_RTC_RG_DCDC_RF_VBIT_VOUT_VALLEY_DELTA_MASK));
    // 0x70001020
    psim_mask_write((unsigned int)(&AIC_PMU_RTC1->pmu_clk_ctrl2),
        PMU_RTC1_RTC_RG_XTAL_CAP_LP_BIT(0x0),
        PMU_RTC1_RTC_RG_XTAL_CAP_LP_BIT_MASK);
    // 0x7000101C
    //psim_mask_write((unsigned int)(&AIC_PMU_RTC1->pmu_clk_ctrl1),
    //    (PMU_RTC1_RTC_RG_XTAL_IBIAS_LP_BIT(0x0) |
    //    PMU_RTC1_RTC_RG_CM_XTAL_IREF_LP_BIT(0x0)),
    //    (PMU_RTC1_RTC_RG_XTAL_IBIAS_LP_BIT_MASK |
    //    PMU_RTC1_RTC_RG_CM_XTAL_IREF_LP_BIT_MASK));
    // 0x70001000
    //psim_mask_write((unsigned int)(&AIC_PMU_RTC1->pmu_pd_ctrl0),
    //    PMU_RTC1_RTC_RG_PMU_CHOPP_BG_PU_LPVREF,
    //    PMU_RTC1_RTC_RG_PMU_CHOPP_BG_PU_LPVREF);
    // 0x70001084
    psim_mask_write((unsigned int)(&AIC_PMU_RTC1->pmu_cm_xtal_cfg1),
        (PMU_RTC1_RTC_RG_XTAL_IBIAS_LP_BIT(0x4) |
        PMU_RTC1_RTC_RG_CM_XTAL_IREF_LP_BIT(0x0)),
        (PMU_RTC1_RTC_RG_XTAL_IBIAS_LP_BIT_MASK |
        PMU_RTC1_RTC_RG_CM_XTAL_IREF_LP_BIT_MASK));
    // 0x70001008
    psim_mask_write((unsigned int)(&AIC_PMU_RTC1->pmu_seq_num0),
        (PMU_RTC1_RTC_RG_LDO_AVDD18_PU_SEQ_NUM(0x1F)),
        (PMU_RTC1_RTC_RG_LDO_AVDD18_PU_SEQ_NUM_MASK));
    if (ChipIdGet(2) >= 4) { // >= U04
        uint32_t rsvd_bit;
        if (syscfg_predefined.sys_hib_lowramret_en) {
            rsvd_bit = (0<<4) | (0<<2); // hram volt drop, lram power down
        } else {
            rsvd_bit = (0<<4) | (1<<2); // hram volt drop, lram volt drop
        }
        // 0x70001088
        psim_mask_write((unsigned int)(&AIC_PMU_RTC1->pmu_cm_xtal_cfg2),
            (PMU_RTC1_RTC_RG_PMIC_RSVD_BIT(rsvd_bit)),
            (PMU_RTC1_RTC_RG_PMIC_RSVD_BIT((1<<4) | (1<<2))));
    }
}

void pmic_lpo_256k_calib_init(void)
{
    //stdio_uart_puts("pmic_lpo_256k_calib_init\r\n");
    unsigned volatile int rdata;
    // 40m
    psim_write((unsigned int)(&AIC_PMU_RTC1->pmu_clk_div_128k),
        PMU_RTC1_RTC_RG_CLK_DIV_128K_CNT_FRC(4816) | PMU_RTC1_RTC_RG_CLK_DIV_128K_CNT_INT(76));
    psim_mask_write((unsigned int)(&AIC_PMU_RTC1->pmu_psm_cfg0),
        PMU_RTC1_RTC_RG_PSM_DIV_DEMON_FACTOR(312500),PMU_RTC1_RTC_RG_PSM_DIV_DEMON_FACTOR(0x7FFFF));
    // psm cal initial
    psim_mask_write((unsigned int)(&AIC_PMU_RTC1->pmu_psm_cfg0),
        PMU_RTC1_RTC_RG_PSM_CAL_EN, PMU_RTC1_RTC_RG_PSM_CAL_EN);
    psim_write((unsigned int)(&AIC_PMU_RTC1->pmu_psm_cfg2),
        PMU_RTC1_RTC_RG_PSM_DEEP_SLEEP_CNT_INTERVAL_TH(2) | PMU_RTC1_RTC_RG_PSM_128K_CAL_CNT_P(7+1) |
        PMU_RTC1_RTC_RG_PSM_128K_CAL_CNT_N(0+1) | PMU_RTC1_RTC_RG_PSM_CAL_PRE_CNT_TH(2));
    psim_mask_write((unsigned int)(&AIC_PMU_RTC1->pmu_psm_cfg0),
        PMU_RTC1_RTC_RG_PSM_REG_UPDATE, PMU_RTC1_RTC_RG_PSM_REG_UPDATE);
    psim_mask_write((unsigned int)(&AIC_PMU_RTC1->pmu_psm_cfg0),
        0, PMU_RTC1_RTC_RG_PSM_REG_UPDATE);
    do {
        rdata = psim_read((unsigned int)(&AIC_PMU_RTC1->pmu_psm_cfg6));
    } while ((rdata & PMU_RTC1_PSM_REG_UPDATE_PULSE_VLD) != PMU_RTC1_PSM_REG_UPDATE_PULSE_VLD);
}

void pmic_lpo_256k_calib_start(void)
{
    // psm cal en
    psim_mask_write((unsigned int)(&AIC_PMU_RTC1->pmu_psm_cfg0),
        PMU_RTC1_RTC_RG_PSM_SW_CAL_EN, PMU_RTC1_RTC_RG_PSM_SW_CAL_EN);
    psim_mask_write((unsigned int)(&AIC_PMU_RTC1->pmu_psm_cfg0),
        0, PMU_RTC1_RTC_RG_PSM_SW_CAL_EN);
}

void pmic_lpo_256k_calib_done_check(void)
{
    unsigned volatile int rdata;
    //stdio_uart_puts("pmic_lpo_256k_calib_done_check\r\n");
    do {
        rdata = psim_read((unsigned int)(&AIC_PMU_RTC1->pmu_int_irq));
    } while ((rdata & PMU_RTC1_RTC_PSM_CAL_DONE_RAW_STATUS) != PMU_RTC1_RTC_PSM_CAL_DONE_RAW_STATUS);
    psim_mask_write((unsigned int)(&AIC_PMU_RTC1->pmu_int_irq),
        PMU_RTC1_RTC_RG_PSM_CAL_DONE_STATUS_CLR, PMU_RTC1_RTC_RG_PSM_CAL_DONE_STATUS_CLR);
    //stdio_uart_puts("check end\r\n");
}

void pmic_frc_ccm_en(int en)
{
    // 0x700021D0 bit5=1@wf_bt_pu, bit5=0@wf_bt_pd
    psim_mask_write((unsigned int)(&AIC_DCDC_RFCTRL->dcdc_ctrl_cfg40),
        ((en) ? DUAL_DCDC_RF_CTRL_RG_DCDC_MODE_FLAG_FRC_EN : 0),
        (DUAL_DCDC_RF_CTRL_RG_DCDC_MODE_FLAG_FRC_EN));
}

static void pmic_slplvl_active_config(void)
{
    // dcdc_rf drop in HYS mode
   // psim_mask_write((unsigned int)(&aic1000liteRtcCore->rtc_rg_pwr_cfg),
   //     AIC1000LITE_RTC_CORE_RTC_RG_DCDC_RF_VBIT_DET_VO_DS(0xf), AIC1000LITE_RTC_CORE_RTC_RG_DCDC_RF_VBIT_DET_VO_DS(0xf));
}

static void pmic_slplvl_light_sleep_config(void)
{
    //if (syscfg_predefined.pmic_lp_clk_sel == LPCLK_SEL_RC_256K) {
    //} else
    {
        psim_write((unsigned int)(&AIC_PMU_RTC1->pmu_slp_ctrl1),
              0x0
            //| PMU_RTC1_RTC_RG_RESET_B_SLP_EN                //0
            | PMU_RTC1_RTC_RG_SLP_RTC_PCLK_GATE_EN          //1
            | PMU_RTC1_RTC_RG_SLP_CORE_CLK_FAST_SEL_EN      //2
            | PMU_RTC1_RTC_RG_XTAL_SLP_LP_EN                //3
            //| PMU_RTC1_RTC_RG_GATE_TO_DBB_SLP_EN            //4
            //| PMU_RTC1_RTC_RG_LDO_PA_SLP_DROP_EN            //5
            //| PMU_RTC1_RTC_RG_LDO_VRTC08_DVFS_SLP_EN        //6
            //| PMU_RTC1_RTC_RG_LDO_DCDC_VCORE08_DVFS_SLP_EN  //7
            //| PMU_RTC1_RTC_RG_LDO_AVDD18_SLP_LP_EN          //8
            //| PMU_RTC1_RTC_RG_LDO_AVDD18_SLP_DROP_EN        //9
            | PMU_RTC1_RTC_RG_DCDC_CORE_HYS_PFM_MODE_SLP_EN //10
            | PMU_RTC1_RTC_RG_DCDC_RF_HYS_PFM_MODE_SLP_EN   //11
            //| PMU_RTC1_RTC_RG_DCDC_RF_DVFS_SLP_EN           //12
            | PMU_RTC1_RTC_RG_DCDC_CORE_DEEP_SLP_SLP_EN     //13
            //| PMU_RTC1_RTC_RG_LDO_PA_SLP_LP_EN              //14
            | PMU_RTC1_RTC_RG_LPO26M_SLP_DIS_EN             //15
            | PMU_RTC1_RTC_RG_RC_256K_SLP_DIS_EN            //16
            | PMU_RTC1_RTC_RG_CLK_26M_SLP_DIS_EN            //17
            //| PMU_RTC1_RTC_RG_CLK_COMP_SLP_DIS_EN           //18
            | PMU_RTC1_RTC_RG_CLK_BUF_SLP_DIS_EN            //19
            //| PMU_RTC1_RTC_RG_CLK_6P5M_SLP_DIS_EN           //20
            | PMU_RTC1_RTC_RG_DCDC_RF_CLK_SEL_SLP_EN        //21
            | PMU_RTC1_RTC_RG_DCDC_RF_DEEP_SLP_SLP_EN       //22
            | PMU_RTC1_RTC_RG_CLK_52M_SLP_DIS_EN            //23
            | PMU_RTC1_RTC_RG_CLK_26M_DLY_SLP_DIS_EN        //24
            //| PMU_RTC1_RTC_RG_LDO_VFLASH_SLP_LP_EN          //25
            //| PMU_RTC1_RTC_RG_LDO_VFLASH_SLP_DROP_EN        //26
            //| PMU_RTC1_RTC_RG_XTAL_AVDD_SEL_SLP_EN          //27
            //| PMU_RTC1_RTC_RG_XTAL_CLK_DIV4_SEL_SLP_EN      //28
            | PMU_RTC1_RTC_RG_DCDC_CORE_CLK_SEL_SLP_EN      //29
            //| PMU_RTC1_RTC_RG_LDO_VCORE08_SLP_LP_EN         //30
            | PMU_RTC1_RTC_RG_CHOPP_BG_SW_LPVREF_LP_EN      //31
            );
        psim_write((unsigned int)(&AIC_PMU_RTC1->pmu_slp_ctrl0),
              0x0
            | PMU_RTC1_RTC_RG_SLP_LDO_PD_EN                 //0
            //| PMU_RTC1_RTC_RG_RTC_SLP_PMU_RTC0_PD_EN        //1
            //| PMU_RTC1_RTC_RG_RTC_SLP_PMU_RTC1_PD_EN        //2
            //| PMU_RTC1_RTC_RG_LDO_PA_SLP_PD_EN              //3
            //| PMU_RTC1_RTC_RG_REG18_ULP_SLP_PU_EN           //4
            //| PMU_RTC1_RTC_RG_LDO_DCDC_VCORE08_SLP_PD_EN    //5
            //| PMU_RTC1_RTC_RG_LDO_AVDD18_SLP_PD_EN          //6
            //| PMU_RTC1_RTC_RG_LDO_VRTC08_SLP_PD_EN          //7
            //| PMU_RTC1_RTC_RG_CORE_ISO_SLP_EN               //8
            //| PMU_RTC1_RTC_RG_COREON_SLP_PD_EN              //9
            //| PMU_RTC1_RTC_RG_XTAL_SLP_PD_EN                //10
            | PMU_RTC1_RTC_RG_LPO26M_SLP_PD_EN              //11
            | PMU_RTC1_RTC_RG_RC_256K_SLP_PD_EN             //12
            | PMU_RTC1_RTC_RG_LDO_VFLASH_SLP_PD_EN          //13
            //| PMU_RTC1_RTC_RG_GPIO_SEL_LDO18_SLP_EN         //14
            | PMU_RTC1_RTC_RG_DCDC_CORE_ADC_SLP_PD_EN       //15
            | PMU_RTC1_RTC_RG_DCDC_RF_ADC_SLP_PD_EN         //16
            //| PMU_RTC1_RTC_RG_CHOPP_BG_SLP_PD_EN            //17
            //| PMU_RTC1_RTC_RG_CM_XTAL_SLP_REG_BUF_BYPASS    //18
            | PMU_RTC1_RTC_RG_XDBLR_SLP_PD_EN               //19
            //| PMU_RTC1_RTC_RG_LDO_VCORE08_AWAKE_PU_EN       //20
            //| PMU_RTC1_RTC_RG_AON13_SW2V13_SLP_EN           //21
            //| PMU_RTC1_RTC_RG_LDO_AVDD18_ULPMODE_SLP_EN     //22
            //| PMU_RTC1_RTC_RG_DEEP_SLEEP_AWAKE_EN(0x0)      //23-26
            //| PMU_RTC1_RTC_RG_XTAL_SLP_HOLD_EN              //27
            );
        // 0x70002190
        psim_mask_write((unsigned int)(&AIC_DCDC_RFCTRL->dcdc_ctrl_cfg24),
                        (DUAL_DCDC_RF_CTRL_RG_DCDC_HYS_PFM_GAIN3_DEEPSLEEP(0x20)),
                        (DUAL_DCDC_RF_CTRL_RG_DCDC_HYS_PFM_GAIN3_DEEPSLEEP_MASK));
    }
}

static void pmic_slplvl_deep_sleep_config(void)
{
    uint32_t reg_val0, reg_val1, vio_bits0, vio_bits1, vcore_bits0, vcore_bits1;
    reg_val0 = psim_read((unsigned int)(&AIC_PMU_RTC1->pmu_slp_ctrl0));
    reg_val1 = psim_read((unsigned int)(&AIC_PMU_RTC1->pmu_slp_ctrl1));
    // check gpio wakeup source
    if (pwrctrl_cpusys_awake_src_getf() & ((0x01UL << 4) | (0x01UL << 5) | (0x01UL << 9))) {
        vio_bits0 = reg_val0 &
            (PMU_RTC1_RTC_RG_REG18_ULP_SLP_PU_EN
            | PMU_RTC1_RTC_RG_LDO_AVDD18_SLP_PD_EN
            | PMU_RTC1_RTC_RG_LDO_AVDD18_ULPMODE_SLP_EN
            | PMU_RTC1_RTC_RG_LDO_PA_SLP_PD_EN);
        vio_bits1 = reg_val1 &
            (PMU_RTC1_RTC_RG_LDO_AVDD18_SLP_LP_EN
            | PMU_RTC1_RTC_RG_LDO_PA_SLP_LP_EN);
    } else { // default val
        if (syscfg_predefined.pmic_vio_slp_pd_en) {
            vio_bits0 = (0x0
                //| PMU_RTC1_RTC_RG_REG18_ULP_SLP_PU_EN
                | PMU_RTC1_RTC_RG_LDO_AVDD18_SLP_PD_EN
                //| PMU_RTC1_RTC_RG_LDO_AVDD18_ULPMODE_SLP_EN
                | PMU_RTC1_RTC_RG_LDO_PA_SLP_PD_EN);
            vio_bits1 = (0x0
                //| PMU_RTC1_RTC_RG_LDO_AVDD18_SLP_LP_EN
                //| PMU_RTC1_RTC_RG_LDO_PA_SLP_LP_EN
                );
        } else {
            if (syscfg_predefined.sys_vio_sel == VIO_SEL_1V8_LDO_AVDD18) {
                // ldo_pa pd, ldo_avdd18 use reg18 ulp mode
                vio_bits0 =
                    ( PMU_RTC1_RTC_RG_REG18_ULP_SLP_PU_EN
                    | PMU_RTC1_RTC_RG_LDO_AVDD18_SLP_PD_EN
                    | PMU_RTC1_RTC_RG_LDO_AVDD18_ULPMODE_SLP_EN
                    | PMU_RTC1_RTC_RG_LDO_PA_SLP_PD_EN);
                vio_bits1 = ( 0x0
                    //| PMU_RTC1_RTC_RG_LDO_AVDD18_SLP_LP_EN
                    //| PMU_RTC1_RTC_RG_LDO_PA_SLP_LP_EN
                    );
            } else if (syscfg_predefined.sys_vio_sel == VIO_SEL_3V3_LDO_PA) {
                // ldo_pa lp, ldo_avdd18 pd
                vio_bits0 = (0x0
                    //| PMU_RTC1_RTC_RG_REG18_ULP_SLP_PU_EN
                    | PMU_RTC1_RTC_RG_LDO_AVDD18_SLP_PD_EN
                    //| PMU_RTC1_RTC_RG_LDO_AVDD18_ULPMODE_SLP_EN
                    //| PMU_RTC1_RTC_RG_LDO_PA_SLP_PD_EN
                    );
                vio_bits1 = (0x0
                    //| PMU_RTC1_RTC_RG_LDO_AVDD18_SLP_LP_EN
                    | PMU_RTC1_RTC_RG_LDO_PA_SLP_LP_EN);
            } else {
                vio_bits0 = (0x0
                    //| PMU_RTC1_RTC_RG_REG18_ULP_SLP_PU_EN
                    | PMU_RTC1_RTC_RG_LDO_AVDD18_SLP_PD_EN
                    //| PMU_RTC1_RTC_RG_LDO_AVDD18_ULPMODE_SLP_EN
                    | PMU_RTC1_RTC_RG_LDO_PA_SLP_PD_EN);
                vio_bits1 = (0x0
                    //| PMU_RTC1_RTC_RG_LDO_AVDD18_SLP_LP_EN
                    //| PMU_RTC1_RTC_RG_LDO_PA_SLP_LP_EN
                    );
            }
        }
    }
    if (syscfg_predefined.pmic_vcore_drop_en) {
        vcore_bits0 = (0x0
            //| PMU_RTC1_RTC_RG_LDO_DCDC_VCORE08_SLP_PD_EN    //5
            );
        vcore_bits1 = (0x0
            | PMU_RTC1_RTC_RG_LDO_DCDC_VCORE08_DVFS_SLP_EN  //7
            | PMU_RTC1_RTC_RG_LDO_VCORE08_SLP_LP_EN         //30
            );
    } else {
        vcore_bits0 = (0x0
            | PMU_RTC1_RTC_RG_LDO_DCDC_VCORE08_SLP_PD_EN    //5
            );
        vcore_bits1 = (0x0
            //| PMU_RTC1_RTC_RG_LDO_DCDC_VCORE08_DVFS_SLP_EN  //7
            //| PMU_RTC1_RTC_RG_LDO_VCORE08_SLP_LP_EN         //30
            );
    }
    PMIC_DEBUG_PRINTF("deep_sleep_config pmic_lp_clk_sel:%x\n", syscfg_predefined.pmic_lp_clk_sel);
    if (syscfg_predefined.pmic_lp_clk_sel == LPCLK_SEL_RC_256K) {
        //deep sleep 4  ///vrtc drop, dcdc rf drop, vcore off,xtal pd, lpo256k mode
        // [0x70001014] = 0x81FE9846
        psim_write((unsigned int)(&AIC_PMU_RTC1->pmu_slp_ctrl1),
              vio_bits1
            | vcore_bits1
            //| PMU_RTC1_RTC_RG_RESET_B_SLP_EN                //0
            | PMU_RTC1_RTC_RG_SLP_RTC_PCLK_GATE_EN          //1
            | PMU_RTC1_RTC_RG_SLP_CORE_CLK_FAST_SEL_EN      //2
            //| PMU_RTC1_RTC_RG_XTAL_SLP_LP_EN                //3
            //| PMU_RTC1_RTC_RG_GATE_TO_DBB_SLP_EN            //4
            //| PMU_RTC1_RTC_RG_LDO_PA_SLP_DROP_EN            //5
            | PMU_RTC1_RTC_RG_LDO_VRTC08_DVFS_SLP_EN        //6
            //| PMU_RTC1_RTC_RG_LDO_DCDC_VCORE08_DVFS_SLP_EN  //7
            //| PMU_RTC1_RTC_RG_LDO_AVDD18_SLP_LP_EN          //8
            //| PMU_RTC1_RTC_RG_LDO_AVDD18_SLP_DROP_EN        //9
            //| PMU_RTC1_RTC_RG_DCDC_CORE_HYS_PFM_MODE_SLP_EN //10
            | PMU_RTC1_RTC_RG_DCDC_RF_HYS_PFM_MODE_SLP_EN   //11
            | PMU_RTC1_RTC_RG_DCDC_RF_DVFS_SLP_EN           //12
            //| PMU_RTC1_RTC_RG_DCDC_CORE_DEEP_SLP_SLP_EN     //13
            //| PMU_RTC1_RTC_RG_LDO_PA_SLP_LP_EN              //14
            | PMU_RTC1_RTC_RG_LPO26M_SLP_DIS_EN             //15
            //| PMU_RTC1_RTC_RG_RC_256K_SLP_DIS_EN            //16
            | PMU_RTC1_RTC_RG_CLK_26M_SLP_DIS_EN            //17
            | PMU_RTC1_RTC_RG_CLK_COMP_SLP_DIS_EN           //18
            | PMU_RTC1_RTC_RG_CLK_BUF_SLP_DIS_EN            //19
            | PMU_RTC1_RTC_RG_CLK_6P5M_SLP_DIS_EN           //20
            | PMU_RTC1_RTC_RG_DCDC_RF_CLK_SEL_SLP_EN        //21
            | PMU_RTC1_RTC_RG_DCDC_RF_DEEP_SLP_SLP_EN       //22
            | PMU_RTC1_RTC_RG_CLK_52M_SLP_DIS_EN            //23
            | PMU_RTC1_RTC_RG_CLK_26M_DLY_SLP_DIS_EN        //24
            //| PMU_RTC1_RTC_RG_LDO_VFLASH_SLP_LP_EN          //25
            //| PMU_RTC1_RTC_RG_LDO_VFLASH_SLP_DROP_EN        //26
            //| PMU_RTC1_RTC_RG_XTAL_AVDD_SEL_SLP_EN          //27
            //| PMU_RTC1_RTC_RG_XTAL_CLK_DIV4_SEL_SLP_EN      //28
            //| PMU_RTC1_RTC_RG_DCDC_CORE_CLK_SEL_SLP_EN      //29
            //| PMU_RTC1_RTC_RG_LDO_VCORE08_SLP_LP_EN         //30
            | PMU_RTC1_RTC_RG_CHOPP_BG_SW_LPVREF_LP_EN      //31
            );
        // [0x70001010] = 0x002BEF69
        psim_write((unsigned int)(&AIC_PMU_RTC1->pmu_slp_ctrl0),
              vio_bits0
            | vcore_bits0
            | PMU_RTC1_RTC_RG_SLP_LDO_PD_EN                 //0
            //| PMU_RTC1_RTC_RG_RTC_SLP_PMU_RTC0_PD_EN        //1
            //| PMU_RTC1_RTC_RG_RTC_SLP_PMU_RTC1_PD_EN        //2
            //| PMU_RTC1_RTC_RG_LDO_PA_SLP_PD_EN              //3
            //| PMU_RTC1_RTC_RG_REG18_ULP_SLP_PU_EN           //4
            //| PMU_RTC1_RTC_RG_LDO_DCDC_VCORE08_SLP_PD_EN    //5
            //| PMU_RTC1_RTC_RG_LDO_AVDD18_SLP_PD_EN          //6
            //| PMU_RTC1_RTC_RG_LDO_VRTC08_SLP_PD_EN          //7
            | PMU_RTC1_RTC_RG_CORE_ISO_SLP_EN               //8
            | PMU_RTC1_RTC_RG_COREON_SLP_PD_EN              //9
            | PMU_RTC1_RTC_RG_XTAL_SLP_PD_EN                //10
            | PMU_RTC1_RTC_RG_LPO26M_SLP_PD_EN              //11
            //| PMU_RTC1_RTC_RG_RC_256K_SLP_PD_EN             //12
            | PMU_RTC1_RTC_RG_LDO_VFLASH_SLP_PD_EN          //13
            | PMU_RTC1_RTC_RG_GPIO_SEL_LDO18_SLP_EN         //14
            | PMU_RTC1_RTC_RG_DCDC_CORE_ADC_SLP_PD_EN       //15
            | PMU_RTC1_RTC_RG_DCDC_RF_ADC_SLP_PD_EN         //16
            | PMU_RTC1_RTC_RG_CHOPP_BG_SLP_PD_EN            //17
            //| PMU_RTC1_RTC_RG_CM_XTAL_SLP_REG_BUF_BYPASS    //18
            | PMU_RTC1_RTC_RG_XDBLR_SLP_PD_EN               //19
            //| PMU_RTC1_RTC_RG_LDO_VCORE08_AWAKE_PU_EN       //20
            | PMU_RTC1_RTC_RG_AON13_SW2V13_SLP_EN           //21
            //| PMU_RTC1_RTC_RG_LDO_AVDD18_ULPMODE_SLP_EN     //22
            //| PMU_RTC1_RTC_RG_DEEP_SLEEP_AWAKE_EN(0x0)      //23-26
            //| PMU_RTC1_RTC_RG_XTAL_SLP_HOLD_EN              //27
            | PMU_RTC1_RTC_RG_GPIO_VIO13_LP_SLP_EN          //28
            );
    } else {
        //deep sleep 5  ///vrtc drop, dcdc rf drop, vcore off,xtal lp, lpo26m mode
        // [0x70001014] = 0x81EB984E
        psim_write((unsigned int)(&AIC_PMU_RTC1->pmu_slp_ctrl1),
              vio_bits1
            | vcore_bits1
            //| PMU_RTC1_RTC_RG_RESET_B_SLP_EN                //0
            | PMU_RTC1_RTC_RG_SLP_RTC_PCLK_GATE_EN          //1
            | PMU_RTC1_RTC_RG_SLP_CORE_CLK_FAST_SEL_EN      //2
            | PMU_RTC1_RTC_RG_XTAL_SLP_LP_EN                //3
            //| PMU_RTC1_RTC_RG_GATE_TO_DBB_SLP_EN            //4
            //| PMU_RTC1_RTC_RG_LDO_PA_SLP_DROP_EN            //5
            | PMU_RTC1_RTC_RG_LDO_VRTC08_DVFS_SLP_EN        //6
            //| PMU_RTC1_RTC_RG_LDO_DCDC_VCORE08_DVFS_SLP_EN  //7
            //| PMU_RTC1_RTC_RG_LDO_AVDD18_SLP_LP_EN          //8
            //| PMU_RTC1_RTC_RG_LDO_AVDD18_SLP_DROP_EN        //9
            //| PMU_RTC1_RTC_RG_DCDC_CORE_HYS_PFM_MODE_SLP_EN //10
            | PMU_RTC1_RTC_RG_DCDC_RF_HYS_PFM_MODE_SLP_EN   //11
            | PMU_RTC1_RTC_RG_DCDC_RF_DVFS_SLP_EN           //12
            //| PMU_RTC1_RTC_RG_DCDC_CORE_DEEP_SLP_SLP_EN     //13
            //| PMU_RTC1_RTC_RG_LDO_PA_SLP_LP_EN              //14
            | PMU_RTC1_RTC_RG_LPO26M_SLP_DIS_EN             //15
            | PMU_RTC1_RTC_RG_RC_256K_SLP_DIS_EN            //16
            | PMU_RTC1_RTC_RG_CLK_26M_SLP_DIS_EN            //17
            //| PMU_RTC1_RTC_RG_CLK_COMP_SLP_DIS_EN           //18
            | PMU_RTC1_RTC_RG_CLK_BUF_SLP_DIS_EN            //19
            //| PMU_RTC1_RTC_RG_CLK_6P5M_SLP_DIS_EN           //20
            | PMU_RTC1_RTC_RG_DCDC_RF_CLK_SEL_SLP_EN        //21
            | PMU_RTC1_RTC_RG_DCDC_RF_DEEP_SLP_SLP_EN       //22
            | PMU_RTC1_RTC_RG_CLK_52M_SLP_DIS_EN            //23
            | PMU_RTC1_RTC_RG_CLK_26M_DLY_SLP_DIS_EN        //24
            //| PMU_RTC1_RTC_RG_LDO_VFLASH_SLP_LP_EN          //25
            //| PMU_RTC1_RTC_RG_LDO_VFLASH_SLP_DROP_EN        //26
            //| PMU_RTC1_RTC_RG_XTAL_AVDD_SEL_SLP_EN          //27
            //| PMU_RTC1_RTC_RG_XTAL_CLK_DIV4_SEL_SLP_EN      //28
            //| PMU_RTC1_RTC_RG_DCDC_CORE_CLK_SEL_SLP_EN      //29
            //| PMU_RTC1_RTC_RG_LDO_VCORE08_SLP_LP_EN         //30
            | PMU_RTC1_RTC_RG_CHOPP_BG_SW_LPVREF_LP_EN      //31
            );
        // [0x70001010] = 0x082BF869
        psim_write((unsigned int)(&AIC_PMU_RTC1->pmu_slp_ctrl0),
              vio_bits0
            | vcore_bits0
            | PMU_RTC1_RTC_RG_SLP_LDO_PD_EN                 //0
            //| PMU_RTC1_RTC_RG_RTC_SLP_PMU_RTC0_PD_EN        //1
            //| PMU_RTC1_RTC_RG_RTC_SLP_PMU_RTC1_PD_EN        //2
            //| PMU_RTC1_RTC_RG_LDO_PA_SLP_PD_EN              //3
            //| PMU_RTC1_RTC_RG_REG18_ULP_SLP_PU_EN           //4
            //| PMU_RTC1_RTC_RG_LDO_DCDC_VCORE08_SLP_PD_EN    //5
            //| PMU_RTC1_RTC_RG_LDO_AVDD18_SLP_PD_EN          //6
            //| PMU_RTC1_RTC_RG_LDO_VRTC08_SLP_PD_EN          //7
            //| PMU_RTC1_RTC_RG_CORE_ISO_SLP_EN               //8
            //| PMU_RTC1_RTC_RG_COREON_SLP_PD_EN              //9
            //| PMU_RTC1_RTC_RG_XTAL_SLP_PD_EN                //10
            | PMU_RTC1_RTC_RG_LPO26M_SLP_PD_EN              //11
            | PMU_RTC1_RTC_RG_RC_256K_SLP_PD_EN             //12
            | PMU_RTC1_RTC_RG_LDO_VFLASH_SLP_PD_EN          //13
            | PMU_RTC1_RTC_RG_GPIO_SEL_LDO18_SLP_EN         //14
            | PMU_RTC1_RTC_RG_DCDC_CORE_ADC_SLP_PD_EN       //15
            | PMU_RTC1_RTC_RG_DCDC_RF_ADC_SLP_PD_EN         //16
            | PMU_RTC1_RTC_RG_CHOPP_BG_SLP_PD_EN            //17
            //| PMU_RTC1_RTC_RG_CM_XTAL_SLP_REG_BUF_BYPASS    //18
            | PMU_RTC1_RTC_RG_XDBLR_SLP_PD_EN               //19
            //| PMU_RTC1_RTC_RG_LDO_VCORE08_AWAKE_PU_EN       //20
            | PMU_RTC1_RTC_RG_AON13_SW2V13_SLP_EN           //21
            //| PMU_RTC1_RTC_RG_LDO_AVDD18_ULPMODE_SLP_EN     //22
            //| PMU_RTC1_RTC_RG_DEEP_SLEEP_AWAKE_EN(0x0)      //23-26
            | PMU_RTC1_RTC_RG_XTAL_SLP_HOLD_EN              //27
            | PMU_RTC1_RTC_RG_GPIO_VIO13_LP_SLP_EN          //28
            );
    }
    // 0x70002190
    psim_mask_write((unsigned int)(&AIC_DCDC_RFCTRL->dcdc_ctrl_cfg24),
        (DUAL_DCDC_RF_CTRL_RG_DCDC_HYS_PFM_GAIN3_DEEPSLEEP(0x18)),
        (DUAL_DCDC_RF_CTRL_RG_DCDC_HYS_PFM_GAIN3_DEEPSLEEP_MASK));

    PMIC_DEBUG_PRINTF("[%p]=%08x\n",(&AIC_PMU_RTC1->pmu_slp_ctrl1),psim_read((unsigned int)(&AIC_PMU_RTC1->pmu_slp_ctrl1)));
    PMIC_DEBUG_PRINTF("[%p]=%08x\n",(&AIC_PMU_RTC1->pmu_slp_ctrl0),psim_read((unsigned int)(&AIC_PMU_RTC1->pmu_slp_ctrl0)));
    PMIC_DEBUG_PRINTF("[%p]=%08x\n",(&AIC_PMU_RTC1->pmu_pd_ctrl0),psim_read((unsigned int)(&AIC_PMU_RTC1->pmu_pd_ctrl0)));
    PMIC_DEBUG_PRINTF("[%p]=%08x\n",(&AIC_PMU_RTC1->pmu_ldo_vcore08_ctrl),psim_read((unsigned int)(&AIC_PMU_RTC1->pmu_ldo_vcore08_ctrl)));
}

static void pmic_slplvl_hibernate_config(void)
{
    uint32_t reg_val0, reg_val1, vio_bits0, vio_bits1;
    reg_val0 = psim_read((unsigned int)(&AIC_PMU_RTC1->pmu_slp_ctrl0));
    reg_val1 = psim_read((unsigned int)(&AIC_PMU_RTC1->pmu_slp_ctrl1));
    // check gpio wakeup source
    if (pwrctrl_cpusys_awake_src_getf() & ((0x01UL << 4) | (0x01UL << 5) | (0x01UL << 9))) {
        vio_bits0 = reg_val0 &
            (PMU_RTC1_RTC_RG_REG18_ULP_SLP_PU_EN
            | PMU_RTC1_RTC_RG_LDO_AVDD18_SLP_PD_EN
            | PMU_RTC1_RTC_RG_LDO_AVDD18_ULPMODE_SLP_EN
            | PMU_RTC1_RTC_RG_LDO_PA_SLP_PD_EN);
        vio_bits1 = reg_val1 &
            (PMU_RTC1_RTC_RG_LDO_AVDD18_SLP_LP_EN
            | PMU_RTC1_RTC_RG_LDO_PA_SLP_LP_EN);
    } else { // default val
        vio_bits0 = (0x0
            //| PMU_RTC1_RTC_RG_REG18_ULP_SLP_PU_EN
            | PMU_RTC1_RTC_RG_LDO_AVDD18_SLP_PD_EN
            //| PMU_RTC1_RTC_RG_LDO_AVDD18_ULPMODE_SLP_EN
            | PMU_RTC1_RTC_RG_LDO_PA_SLP_PD_EN);
        vio_bits1 = (0x0
            //| PMU_RTC1_RTC_RG_LDO_AVDD18_SLP_LP_EN
            //| PMU_RTC1_RTC_RG_LDO_PA_SLP_LP_EN
            );
    }
    PMIC_DEBUG_PRINTF("hibernate_config pmic_lp_clk_sel:%x\n", syscfg_predefined.pmic_lp_clk_sel);
    if (syscfg_predefined.pmic_lp_clk_sel == LPCLK_SEL_RC_256K) {
        //deep sleep 4  ///vrtc drop, dcdc rf drop, vcore off,xtal pd, lpo256k mode
        // [0x70001014] = 0x81FE9846
        psim_write((unsigned int)(&AIC_PMU_RTC1->pmu_slp_ctrl1),
              vio_bits1
            //| PMU_RTC1_RTC_RG_RESET_B_SLP_EN                //0
            | PMU_RTC1_RTC_RG_SLP_RTC_PCLK_GATE_EN          //1
            | PMU_RTC1_RTC_RG_SLP_CORE_CLK_FAST_SEL_EN      //2
            //| PMU_RTC1_RTC_RG_XTAL_SLP_LP_EN                //3
            //| PMU_RTC1_RTC_RG_GATE_TO_DBB_SLP_EN            //4
            //| PMU_RTC1_RTC_RG_LDO_PA_SLP_DROP_EN            //5
            | PMU_RTC1_RTC_RG_LDO_VRTC08_DVFS_SLP_EN        //6
            //| PMU_RTC1_RTC_RG_LDO_DCDC_VCORE08_DVFS_SLP_EN  //7
            //| PMU_RTC1_RTC_RG_LDO_AVDD18_SLP_LP_EN          //8
            //| PMU_RTC1_RTC_RG_LDO_AVDD18_SLP_DROP_EN        //9
            //| PMU_RTC1_RTC_RG_DCDC_CORE_HYS_PFM_MODE_SLP_EN //10
            | PMU_RTC1_RTC_RG_DCDC_RF_HYS_PFM_MODE_SLP_EN   //11
            | PMU_RTC1_RTC_RG_DCDC_RF_DVFS_SLP_EN           //12
            //| PMU_RTC1_RTC_RG_DCDC_CORE_DEEP_SLP_SLP_EN     //13
            //| PMU_RTC1_RTC_RG_LDO_PA_SLP_LP_EN              //14
            | PMU_RTC1_RTC_RG_LPO26M_SLP_DIS_EN             //15
            //| PMU_RTC1_RTC_RG_RC_256K_SLP_DIS_EN            //16
            | PMU_RTC1_RTC_RG_CLK_26M_SLP_DIS_EN            //17
            | PMU_RTC1_RTC_RG_CLK_COMP_SLP_DIS_EN           //18
            | PMU_RTC1_RTC_RG_CLK_BUF_SLP_DIS_EN            //19
            | PMU_RTC1_RTC_RG_CLK_6P5M_SLP_DIS_EN           //20
            | PMU_RTC1_RTC_RG_DCDC_RF_CLK_SEL_SLP_EN        //21
            | PMU_RTC1_RTC_RG_DCDC_RF_DEEP_SLP_SLP_EN       //22
            | PMU_RTC1_RTC_RG_CLK_52M_SLP_DIS_EN            //23
            | PMU_RTC1_RTC_RG_CLK_26M_DLY_SLP_DIS_EN        //24
            //| PMU_RTC1_RTC_RG_LDO_VFLASH_SLP_LP_EN          //25
            //| PMU_RTC1_RTC_RG_LDO_VFLASH_SLP_DROP_EN        //26
            //| PMU_RTC1_RTC_RG_XTAL_AVDD_SEL_SLP_EN          //27
            //| PMU_RTC1_RTC_RG_XTAL_CLK_DIV4_SEL_SLP_EN      //28
            //| PMU_RTC1_RTC_RG_DCDC_CORE_CLK_SEL_SLP_EN      //29
            //| PMU_RTC1_RTC_RG_LDO_VCORE08_SLP_LP_EN         //30
            | PMU_RTC1_RTC_RG_CHOPP_BG_SW_LPVREF_LP_EN      //31
            );
        // [0x70001010] = 0x002BEF69
        psim_write((unsigned int)(&AIC_PMU_RTC1->pmu_slp_ctrl0),
              vio_bits0
            | PMU_RTC1_RTC_RG_SLP_LDO_PD_EN                 //0
            //| PMU_RTC1_RTC_RG_RTC_SLP_PMU_RTC0_PD_EN        //1
            //| PMU_RTC1_RTC_RG_RTC_SLP_PMU_RTC1_PD_EN        //2
            //| PMU_RTC1_RTC_RG_LDO_PA_SLP_PD_EN              //3
            //| PMU_RTC1_RTC_RG_REG18_ULP_SLP_PU_EN           //4
            | PMU_RTC1_RTC_RG_LDO_DCDC_VCORE08_SLP_PD_EN    //5
            //| PMU_RTC1_RTC_RG_LDO_AVDD18_SLP_PD_EN          //6
            //| PMU_RTC1_RTC_RG_LDO_VRTC08_SLP_PD_EN          //7
            | PMU_RTC1_RTC_RG_CORE_ISO_SLP_EN               //8
            | PMU_RTC1_RTC_RG_COREON_SLP_PD_EN              //9
            | PMU_RTC1_RTC_RG_XTAL_SLP_PD_EN                //10
            | PMU_RTC1_RTC_RG_LPO26M_SLP_PD_EN              //11
            //| PMU_RTC1_RTC_RG_RC_256K_SLP_PD_EN             //12
            | PMU_RTC1_RTC_RG_LDO_VFLASH_SLP_PD_EN          //13
            | PMU_RTC1_RTC_RG_GPIO_SEL_LDO18_SLP_EN         //14
            | PMU_RTC1_RTC_RG_DCDC_CORE_ADC_SLP_PD_EN       //15
            | PMU_RTC1_RTC_RG_DCDC_RF_ADC_SLP_PD_EN         //16
            | PMU_RTC1_RTC_RG_CHOPP_BG_SLP_PD_EN            //17
            //| PMU_RTC1_RTC_RG_CM_XTAL_SLP_REG_BUF_BYPASS    //18
            | PMU_RTC1_RTC_RG_XDBLR_SLP_PD_EN               //19
            //| PMU_RTC1_RTC_RG_LDO_VCORE08_AWAKE_PU_EN       //20
            | PMU_RTC1_RTC_RG_AON13_SW2V13_SLP_EN           //21
            //| PMU_RTC1_RTC_RG_LDO_AVDD18_ULPMODE_SLP_EN     //22
            //| PMU_RTC1_RTC_RG_DEEP_SLEEP_AWAKE_EN(0x0)      //23-26
            //| PMU_RTC1_RTC_RG_XTAL_SLP_HOLD_EN              //27
            );
    } else {
        //deep sleep 5  ///vrtc drop, dcdc rf drop, vcore off,xtal lp, lpo26m mode
        // [0x70001014] = 0x81EB984E
        psim_write((unsigned int)(&AIC_PMU_RTC1->pmu_slp_ctrl1),
              vio_bits1
            //| PMU_RTC1_RTC_RG_RESET_B_SLP_EN                //0
            | PMU_RTC1_RTC_RG_SLP_RTC_PCLK_GATE_EN          //1
            | PMU_RTC1_RTC_RG_SLP_CORE_CLK_FAST_SEL_EN      //2
            | PMU_RTC1_RTC_RG_XTAL_SLP_LP_EN                //3
            //| PMU_RTC1_RTC_RG_GATE_TO_DBB_SLP_EN            //4
            //| PMU_RTC1_RTC_RG_LDO_PA_SLP_DROP_EN            //5
            | PMU_RTC1_RTC_RG_LDO_VRTC08_DVFS_SLP_EN        //6
            //| PMU_RTC1_RTC_RG_LDO_DCDC_VCORE08_DVFS_SLP_EN  //7
            //| PMU_RTC1_RTC_RG_LDO_AVDD18_SLP_LP_EN          //8
            //| PMU_RTC1_RTC_RG_LDO_AVDD18_SLP_DROP_EN        //9
            //| PMU_RTC1_RTC_RG_DCDC_CORE_HYS_PFM_MODE_SLP_EN //10
            | PMU_RTC1_RTC_RG_DCDC_RF_HYS_PFM_MODE_SLP_EN   //11
            | PMU_RTC1_RTC_RG_DCDC_RF_DVFS_SLP_EN           //12
            //| PMU_RTC1_RTC_RG_DCDC_CORE_DEEP_SLP_SLP_EN     //13
            //| PMU_RTC1_RTC_RG_LDO_PA_SLP_LP_EN              //14
            | PMU_RTC1_RTC_RG_LPO26M_SLP_DIS_EN             //15
            | PMU_RTC1_RTC_RG_RC_256K_SLP_DIS_EN            //16
            | PMU_RTC1_RTC_RG_CLK_26M_SLP_DIS_EN            //17
            //| PMU_RTC1_RTC_RG_CLK_COMP_SLP_DIS_EN           //18
            | PMU_RTC1_RTC_RG_CLK_BUF_SLP_DIS_EN            //19
            //| PMU_RTC1_RTC_RG_CLK_6P5M_SLP_DIS_EN           //20
            | PMU_RTC1_RTC_RG_DCDC_RF_CLK_SEL_SLP_EN        //21
            | PMU_RTC1_RTC_RG_DCDC_RF_DEEP_SLP_SLP_EN       //22
            | PMU_RTC1_RTC_RG_CLK_52M_SLP_DIS_EN            //23
            | PMU_RTC1_RTC_RG_CLK_26M_DLY_SLP_DIS_EN        //24
            //| PMU_RTC1_RTC_RG_LDO_VFLASH_SLP_LP_EN          //25
            //| PMU_RTC1_RTC_RG_LDO_VFLASH_SLP_DROP_EN        //26
            //| PMU_RTC1_RTC_RG_XTAL_AVDD_SEL_SLP_EN          //27
            //| PMU_RTC1_RTC_RG_XTAL_CLK_DIV4_SEL_SLP_EN      //28
            //| PMU_RTC1_RTC_RG_DCDC_CORE_CLK_SEL_SLP_EN      //29
            //| PMU_RTC1_RTC_RG_LDO_VCORE08_SLP_LP_EN         //30
            | PMU_RTC1_RTC_RG_CHOPP_BG_SW_LPVREF_LP_EN      //31
            );
        // [0x70001010] = 0x082BF869
        psim_write((unsigned int)(&AIC_PMU_RTC1->pmu_slp_ctrl0),
              vio_bits0
            | PMU_RTC1_RTC_RG_SLP_LDO_PD_EN                 //0
            //| PMU_RTC1_RTC_RG_RTC_SLP_PMU_RTC0_PD_EN        //1
            //| PMU_RTC1_RTC_RG_RTC_SLP_PMU_RTC1_PD_EN        //2
            //| PMU_RTC1_RTC_RG_LDO_PA_SLP_PD_EN              //3
            //| PMU_RTC1_RTC_RG_REG18_ULP_SLP_PU_EN           //4
            | PMU_RTC1_RTC_RG_LDO_DCDC_VCORE08_SLP_PD_EN    //5
            //| PMU_RTC1_RTC_RG_LDO_AVDD18_SLP_PD_EN          //6
            //| PMU_RTC1_RTC_RG_LDO_VRTC08_SLP_PD_EN          //7
            //| PMU_RTC1_RTC_RG_CORE_ISO_SLP_EN               //8
            //| PMU_RTC1_RTC_RG_COREON_SLP_PD_EN              //9
            //| PMU_RTC1_RTC_RG_XTAL_SLP_PD_EN                //10
            | PMU_RTC1_RTC_RG_LPO26M_SLP_PD_EN              //11
            | PMU_RTC1_RTC_RG_RC_256K_SLP_PD_EN             //12
            | PMU_RTC1_RTC_RG_LDO_VFLASH_SLP_PD_EN          //13
            | PMU_RTC1_RTC_RG_GPIO_SEL_LDO18_SLP_EN         //14
            | PMU_RTC1_RTC_RG_DCDC_CORE_ADC_SLP_PD_EN       //15
            | PMU_RTC1_RTC_RG_DCDC_RF_ADC_SLP_PD_EN         //16
            | PMU_RTC1_RTC_RG_CHOPP_BG_SLP_PD_EN            //17
            //| PMU_RTC1_RTC_RG_CM_XTAL_SLP_REG_BUF_BYPASS    //18
            | PMU_RTC1_RTC_RG_XDBLR_SLP_PD_EN               //19
            //| PMU_RTC1_RTC_RG_LDO_VCORE08_AWAKE_PU_EN       //20
            | PMU_RTC1_RTC_RG_AON13_SW2V13_SLP_EN           //21
            //| PMU_RTC1_RTC_RG_LDO_AVDD18_ULPMODE_SLP_EN     //22
            //| PMU_RTC1_RTC_RG_DEEP_SLEEP_AWAKE_EN(0x0)      //23-26
            | PMU_RTC1_RTC_RG_XTAL_SLP_HOLD_EN              //27
            );
    }

    PMIC_DEBUG_PRINTF("[%p]=%08x\n",(&AIC_PMU_RTC1->pmu_slp_ctrl1),psim_read((unsigned int)(&AIC_PMU_RTC1->pmu_slp_ctrl1)));
    PMIC_DEBUG_PRINTF("[%p]=%08x\n",(&AIC_PMU_RTC1->pmu_slp_ctrl0),psim_read((unsigned int)(&AIC_PMU_RTC1->pmu_slp_ctrl0)));
    PMIC_DEBUG_PRINTF("[%p]=%08x\n",(&AIC_PMU_RTC1->pmu_pd_ctrl0),psim_read((unsigned int)(&AIC_PMU_RTC1->pmu_pd_ctrl0)));
}

void pmic_gpio_wakeup_config(int en)
{
    unsigned int reg_val, vio_bits0 = 0, vio_bits1 = 0;
    reg_val = psim_read((unsigned int)(&AIC_PMU_RTC1->pmu_slp_ctrl0));
    if (en && !syscfg_predefined.pmic_vio_slp_pd_en) {
        if (syscfg_predefined.sys_vio_sel == VIO_SEL_1V8_LDO_AVDD18) {
            if (!(reg_val & PMU_RTC1_RTC_RG_REG18_ULP_SLP_PU_EN)) {
                // ldo_pa pd, ldo_avdd18 use reg18 ulp mode
                vio_bits0 =
                    ( PMU_RTC1_RTC_RG_REG18_ULP_SLP_PU_EN
                    | PMU_RTC1_RTC_RG_LDO_AVDD18_SLP_PD_EN
                    | PMU_RTC1_RTC_RG_LDO_AVDD18_ULPMODE_SLP_EN
                    | PMU_RTC1_RTC_RG_LDO_PA_SLP_PD_EN);
                vio_bits1 = ( 0x0
                    //| PMU_RTC1_RTC_RG_LDO_AVDD18_SLP_LP_EN
                    //| PMU_RTC1_RTC_RG_LDO_PA_SLP_LP_EN
                    );
            }
        } else if (syscfg_predefined.sys_vio_sel == VIO_SEL_3V3_LDO_PA) {
            if ((reg_val & (PMU_RTC1_RTC_RG_LDO_PA_SLP_PD_EN | PMU_RTC1_RTC_RG_LDO_AVDD18_SLP_PD_EN))
                != PMU_RTC1_RTC_RG_LDO_AVDD18_SLP_PD_EN) {
                // ldo_pa lp, ldo_avdd18 pd
                vio_bits0 = (0x0
                    //| PMU_RTC1_RTC_RG_REG18_ULP_SLP_PU_EN
                    | PMU_RTC1_RTC_RG_LDO_AVDD18_SLP_PD_EN
                    //| PMU_RTC1_RTC_RG_LDO_AVDD18_ULPMODE_SLP_EN
                    //| PMU_RTC1_RTC_RG_LDO_PA_SLP_PD_EN
                    );
                vio_bits1 = (0x0
                    //| PMU_RTC1_RTC_RG_LDO_AVDD18_SLP_LP_EN
                    | PMU_RTC1_RTC_RG_LDO_PA_SLP_LP_EN);
            }
        }
    } else {
        // ldo_pa pd, ldo_avdd18 pd
        vio_bits0 = (0x0
            //| PMU_RTC1_RTC_RG_REG18_ULP_SLP_PU_EN
            | PMU_RTC1_RTC_RG_LDO_AVDD18_SLP_PD_EN
            //| PMU_RTC1_RTC_RG_LDO_AVDD18_ULPMODE_SLP_EN
            | PMU_RTC1_RTC_RG_LDO_PA_SLP_PD_EN);
        vio_bits1 = (0x0
            //| PMU_RTC1_RTC_RG_LDO_AVDD18_SLP_LP_EN
            //| PMU_RTC1_RTC_RG_LDO_PA_SLP_LP_EN
            );
    }
    if (vio_bits0) {
        psim_mask_write((unsigned int)(&AIC_PMU_RTC1->pmu_slp_ctrl0),
            vio_bits0,
            (PMU_RTC1_RTC_RG_REG18_ULP_SLP_PU_EN
            | PMU_RTC1_RTC_RG_LDO_AVDD18_SLP_PD_EN
            | PMU_RTC1_RTC_RG_LDO_AVDD18_ULPMODE_SLP_EN
            | PMU_RTC1_RTC_RG_LDO_PA_SLP_PD_EN));
        psim_mask_write((unsigned int)(&AIC_PMU_RTC1->pmu_slp_ctrl1),
            vio_bits1,
            (PMU_RTC1_RTC_RG_LDO_AVDD18_SLP_LP_EN
            | PMU_RTC1_RTC_RG_LDO_PA_SLP_LP_EN));
    }
}

__RAMTEXT void pmic_gpio_hold_config(int hold_en)
{
    if (hold_en) {
        // set GPIOA+GPIOB hold
        ROM_PsimMaskWrite((unsigned int)(&AIC_PMU_RTC0->pmu_rtc0_cfg4),
            (PMU_RTC0_RTC_RG_GPIOA_HOLD(0x7) |
             PMU_RTC0_RTC_RG_GPIOB_HOLD),
            (PMU_RTC0_RTC_RG_GPIOA_HOLD_MASK |
             PMU_RTC0_RTC_RG_GPIOB_HOLD));
    } else {
        // clr GPIOA+GPIOB hold
        ROM_PsimMaskWrite((unsigned int)(&AIC_PMU_RTC0->pmu_rtc0_cfg4),
            (0 | // gpioa_hold
             0), // gpiob_hold
            (PMU_RTC0_RTC_RG_GPIOA_HOLD_MASK |
             PMU_RTC0_RTC_RG_GPIOB_HOLD));
    }
}

uint8_t pmic_gpio_hold_get(void)
{
    uint8_t hold_val;
    uint32_t reg_val;
    // read GPIOA+GPIOB hold
    reg_val = psim_read((unsigned int)(&AIC_PMU_RTC0->pmu_rtc0_cfg4));
    hold_val = (uint8_t)((reg_val &
        (PMU_RTC0_RTC_RG_GPIOA_HOLD_MASK |
         PMU_RTC0_RTC_RG_GPIOB_HOLD)) >>
         PMU_RTC0_RTC_RG_GPIOA_HOLD_LSB);
    return hold_val;
}

void pmic_gpio_hold_set(uint8_t hold_bits)
{
    uint32_t reg_val = ((uint32_t)hold_bits << PMU_RTC0_RTC_RG_GPIOA_HOLD_LSB) &
        (PMU_RTC0_RTC_RG_GPIOA_HOLD_MASK |
         PMU_RTC0_RTC_RG_GPIOB_HOLD);
    // write GPIOA+GPIOB hold
    psim_mask_write((unsigned int)(&AIC_PMU_RTC0->pmu_rtc0_cfg4),
        (reg_val),
        (PMU_RTC0_RTC_RG_GPIOA_HOLD_MASK |
         PMU_RTC0_RTC_RG_GPIOB_HOLD));
}

void pmic_sleep_level_set(POWER_MODE_LEVEL_T level)
{
    critical_section_start();
    switch (level) {
    case PM_LEVEL_ACTIVE: {
        pmic_slplvl_active_config();
        break;
    }
    case PM_LEVEL_LIGHT_SLEEP: {
        pmic_slplvl_light_sleep_config();
        break;
    }
    case PM_LEVEL_DEEP_SLEEP: {
        pmic_slplvl_deep_sleep_config();
        break;
    }
    case PM_LEVEL_HIBERNATE: {
        pmic_slplvl_hibernate_config();
        break;
    }
    default:
        break;
    }
    critical_section_end();
}

uint8_t pmic_por_source_get(void)
{
    uint8_t por_src;
    uint32_t reg_val;
    reg_val = psim_read((unsigned int)(&AIC_PMU_RTC0->pmu_rtc0_cfg7));
    por_src = (uint8_t)((reg_val & PMU_RTC0_RTC_RG_PWR_ON_SRC_STATUS_MASK) >> PMU_RTC0_RTC_RG_PWR_ON_SRC_STATUS_LSB);
    return por_src;
}

void pmic_bor_config(uint32_t bor_vbit, int bor_repower)
{
    //bor enable
    PMIC_MEM_MASK_WRITE((unsigned int)(&AIC_PMU_RTC0->pmu_rtc0_cfg1),
                         (PMU_RTC0_RTC_RG_BOR_EN | PMU_RTC0_RTC_RG_BOR_HIGH_DEGLITCH_EN | PMU_RTC0_RTC_RG_BOR_HIGH_DEGLITCH_TH(0x10) | PMU_RTC0_RTC_RG_BOR_PWROFF_EN | ((bor_repower) ? (PMU_RTC0_RTC_RG_BOR_REPOWER_EN) : 0)),
                         (PMU_RTC0_RTC_RG_BOR_EN | PMU_RTC0_RTC_RG_BOR_HIGH_DEGLITCH_EN | PMU_RTC0_RTC_RG_BOR_HIGH_DEGLITCH_TH_MASK | PMU_RTC0_RTC_RG_BOR_PWROFF_EN | PMU_RTC0_RTC_RG_BOR_REPOWER_EN));
    PMIC_MEM_MASK_WRITE((unsigned int)(&AIC_PMU_RTC0->pmu_rtc0_cfg0),PMU_RTC0_PMU_RTC0_RG_UPDATE,PMU_RTC0_PMU_RTC0_RG_UPDATE);
    for(int i=0;i<10000;i++)
    {
       __NOP();
    }
    //bor out enable
    PMIC_MEM_MASK_WRITE((unsigned int)(&AIC_PMU_RTC1->pmu_rstn_ctrl), (PMU_RTC1_RTC_RG_BOR_VBIT(bor_vbit) | PMU_RTC1_RTC_RG_BOR_ENABLE), (PMU_RTC1_RTC_RG_BOR_ENABLE | PMU_RTC1_RTC_RG_BOR_VBIT_MASK));
}

void pmic_boot_check(void)
{
    uint8_t por_src = pmic_por_source_get();
    if (por_src & PMIC_POR_SRC_WDG_PWR_ON) {
        // clear gpio hold after pmic wdt reboot
        if (pmic_gpio_hold_get()) {
            pmic_gpio_hold_set(0x0);
        }
    }
}

int pmic_rtc_epoch_time_get(unsigned int *sec, unsigned int *usec)
{
    if (sec && usec) {
        *sec  = psim_read((unsigned int)(&AIC_PMU_RTC0->rtc_rg_reserved1));
        *usec = psim_read((unsigned int)(&AIC_PMU_RTC1->pmu_psm_cfg5));
    } else {
        return -1;
    }
    return 0;
}

void pmic_rtc_epoch_time_set(unsigned int sec, unsigned int usec)
{
    psim_write((unsigned int)(&AIC_PMU_RTC0->rtc_rg_reserved1), sec); // keep value while sysctrl_chip_reboot, cleared while pmic_chip_reboot
    psim_write((unsigned int)(&AIC_PMU_RTC1->pmu_psm_cfg5), usec);
}
