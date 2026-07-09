#include <asm/io.h>
#include <asm/arch-ak3918av130/ak_cpu.h>
#include <common.h>

unsigned long clk_get_core_pll_freq(void)
{
	unsigned long m = 0, n = 0, od = 0;
	unsigned long pll_cfg_val = 0;
	unsigned long pll_freq = 0;
	
	pll_cfg_val = inl(CORE_PLL_CHANNEL_CTRL_REG) ;
	m = (pll_cfg_val & 0x1FFF)+1;
	n = ((pll_cfg_val >> 13) & 0x3F)+1;
	od = ((pll_cfg_val >> 19) & 0xF)+1;
	
	pll_freq = (m * 24 * 1000) / (n * od);


	 return pll_freq * 1000;
}


unsigned long get_asic_freq(void)
{
	unsigned long vclk_sel = 0;
	unsigned long core_pll_freq  = 0;
	unsigned long vclk_freq = 0;

	vclk_sel =  ((inl(CORE_PLL_CHANNEL_CTRL_REG) >> 23) & 0x3);
	core_pll_freq = clk_get_core_pll_freq();
	vclk_freq = core_pll_freq / (2*(vclk_sel + 1));
	
//	if(inl(CORE_PLL_CHANNEL_CTRL_REG) & (1<<25))
//		vclk_freq = vclk_freq>>1;

	return vclk_freq;  //asic freq = Gclk

}


#if defined(CONFIG_DISPLAY_CPUINFO)

#define CPU_PLL_OD_CFG_MASK							(0xF)
#define CPU_PLL_OD_CFG_SHIFT						(19)
#define CPU_PLL_N_CFG_MASK							(0x3F)
#define CPU_PLL_N_CFG_SHIFT							(13)
#define CPU_PLL_M_CFG_MASK							(0x1FFF)
#define CPU_PLL_M_CFG_SHIFT							(0)

#define PLL1_MAX_FREQ					1200000000
#define HCLK_DCLK_DDR2_MAX_FREQ			600000000
#define CPU_MAX_FREQ					1200000000

static unsigned long ak_get_cpu_pll_clk(void)
{
	u32 pll_m, pll_n, pll_od;
	u32 cpu_pll_clk;
	u32 regval;

	regval = __raw_readl(CPU_PLL_CHANNEL_CTRL_REG);
	pll_od = ((regval >> CPU_PLL_OD_CFG_SHIFT) & CPU_PLL_OD_CFG_MASK) + 1;
	pll_n = ((regval >> CPU_PLL_N_CFG_SHIFT) & CPU_PLL_N_CFG_MASK) + 1;
	pll_m = ((regval >> CPU_PLL_M_CFG_SHIFT) & CPU_PLL_M_CFG_MASK) + 1;

	cpu_pll_clk = 24 * pll_m /(pll_n * pll_od);

	if(inl(CPU_PLL_CHANNEL_CTRL_REG) & (1<<31))
		cpu_pll_clk = cpu_pll_clk>>1;

	return cpu_pll_clk;
}

void ak3918av130_check_clock(void)
{
	u32 regval, pll_nf, pll_nr, pll_od, freq;
	u32 pll1_freq, hclk_dclk_ddr2_freq, cpu_freq;

	regval = __raw_readl(CPU_PLL_CHANNEL_CTRL_REG);
	pll_nf = (regval & 0x1fff) + 1;
	pll_nr = ((regval >> 13) & 0x3f) + 1;
	pll_od = ((regval >> 19) & 0xf) + 1;

	freq = 24000 * (pll_nf) / (pll_nr) / (pll_od);
	pll1_freq = freq * 1000;

	hclk_dclk_ddr2_freq = pll1_freq / 2;
	cpu_freq = regval & (1<<31) ? pll1_freq / 2 : pll1_freq;

	if (pll1_freq > PLL1_MAX_FREQ) {
		printf("pll1 exceeds the max freq %d, pll1 freq %d\n", PLL1_MAX_FREQ, pll1_freq);
		while(1);
	}

	if (hclk_dclk_ddr2_freq > HCLK_DCLK_DDR2_MAX_FREQ) {
		printf("ddr2 exceeds the max freq %d, ddr freq %d\n",
				HCLK_DCLK_DDR2_MAX_FREQ, hclk_dclk_ddr2_freq);
		while(1);
	}

	if (cpu_freq > CPU_MAX_FREQ) {
		printf("cpu exceeds the max freq %d, cpu freq %d\n", CPU_MAX_FREQ, cpu_freq);
		while(1);
	}
}

int print_cpuinfo(void)
{
	u32 cpurev = ak_get_cpu_pll_clk();

	printf("CPU: %dMHz\n",cpurev);
	ak3918av130_check_clock();
	return 0;
}
#endif


int get_clocks(void)
{
	return 0;
}


