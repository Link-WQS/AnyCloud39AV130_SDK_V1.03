#include <asm/io.h>
#include <asm/arch-ak3918ev300l/ak_cpu.h>
#include <common.h>

unsigned long clk_get_core_pll_freq(void)
{
	unsigned long m = 0, n = 0, od = 0;
	unsigned long pll_cfg_val = 0;
	unsigned long pll_freq = 0;
	
    pll_cfg_val = inl(CORE_PLL_CHANNEL_CTRL_REG) ;
	m = (pll_cfg_val & 0x1FFF)+1;
	n = ((pll_cfg_val & 0x7E000)>>13)+1;
	od = ((pll_cfg_val & 0x780000)>>19)+1; 
	
	pll_freq = (m * 24 * 1000000) / (n * od);


	 return pll_freq;
}


unsigned long get_asic_freq(void)
{
	unsigned long vclk_sel = 0;
	unsigned long core_pll_freq  = 0;
	unsigned long vclk_freq = 0;

	vclk_sel =  ((inl(CORE_PLL_CHANNEL_CTRL_REG) >> 23) & 0x3);
	core_pll_freq = clk_get_core_pll_freq();
	vclk_freq = core_pll_freq / (2*(vclk_sel + 1));
	
	if(inl(CORE_PLL_CHANNEL_CTRL_REG) & (1<<25))
		vclk_freq = vclk_freq>>1;

	return vclk_freq;  //asic freq = Gclk

}


#if defined(CONFIG_DISPLAY_CPUINFO)

#define CPU_PLL_OD_CFG_MASK							(0xF)
#define CPU_PLL_OD_CFG_SHIFT						(19)
#define CPU_PLL_N_CFG_MASK							(0x3F)
#define CPU_PLL_N_CFG_SHIFT							(13)
#define CPU_PLL_M_CFG_MASK							(0x1FFF)
#define CPU_PLL_M_CFG_SHIFT							(0)


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

int print_cpuinfo(void)
{
	u32 cpurev = ak_get_cpu_pll_clk();

	printf("CPU: %dMHz\n",cpurev);
	return 0;
}
#endif


int get_clocks(void)
{
	return 0;
}


