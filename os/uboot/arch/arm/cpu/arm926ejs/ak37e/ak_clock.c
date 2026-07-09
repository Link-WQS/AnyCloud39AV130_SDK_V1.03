#include <asm/io.h>
#include <asm/arch-ak37e/ak_cpu.h>
#include <common.h>

#define PLL_CLK_MIN	180

unsigned long clk_get_core_pll_freq(void)
{
	unsigned long m, n, od;
	unsigned long pll_cfg_val;
	unsigned long core_pll_freq;
	
    pll_cfg_val = inl(CLK_ASIC_PLL_CTRL) ;
	m = (pll_cfg_val & 0xff);
	n = ((pll_cfg_val & 0xf00)>>8);
	od = ((pll_cfg_val & 0x3000)>>12); 

	core_pll_freq = (m * 24 * 1000000) / (n *  (1<<od));

	 return core_pll_freq;
}

unsigned long get_vclk(void)
{
	unsigned long vclk_sel;
	unsigned long core_pll_freq;
	unsigned long vclk_freq;

	vclk_sel =  ((inl(CLK_ASIC_PLL_CTRL) >> 17) & 0x3);
	
	core_pll_freq = clk_get_core_pll_freq();
	vclk_freq = (core_pll_freq / (2*(vclk_sel+1)));

	return vclk_freq;
}

unsigned long get_asic_freq(void)
{
	unsigned long regval;
	unsigned long div;

	regval = readl(CLK_ASIC_PLL_CTRL);
	div = regval & (1 << 24);
	if (div == 0)
		regval =  get_vclk();
	else
		regval = (get_vclk() >> 1);

	return regval;
}

static void ak_check_pll1_clk(void)
{
#define CPU_PLL_OD_CFG_MASK             (0x3)
#define CPU_PLL_OD_CFG_SHIFT            (12)
#define CPU_PLL_N_CFG_MASK              (0xF)
#define CPU_PLL_N_CFG_SHIFT             (8)
#define CPU_PLL_M_CFG_MASK              (0xFF)
#define CPU_PLL_M_CFG_SHIFT             (0)

#define PLL1_MAX_RATE                   (800000000)
#define JCLK_MAX_RATE                   (800000000)
#define HCLK_MAX_RATE                   (440000000)
#define DPHY_CLK_MAX_RATE               (440000000)

    u32 regval, dphyclk, hclk, dclk, cpu_pll_clk;
    u32 pll_m, pll_n, pll_od;

    regval = __raw_readl(CPU_PLL_CHANNEL_CTRL_REG);
    pll_od = (regval >> CPU_PLL_OD_CFG_SHIFT) & CPU_PLL_OD_CFG_MASK;
    pll_n = (regval >> CPU_PLL_N_CFG_SHIFT) & CPU_PLL_N_CFG_MASK;
    pll_m = (regval >> CPU_PLL_M_CFG_SHIFT) & CPU_PLL_M_CFG_MASK;

    cpu_pll_clk = 24 * pll_m /(pll_n * (1 << pll_od));

    cpu_pll_clk = cpu_pll_clk * 1000000;
    if (cpu_pll_clk > PLL1_MAX_RATE) {
        printf("Can't set PLL1 rate %d, Max rate: %d! halt!!!\n", cpu_pll_clk, PLL1_MAX_RATE);
        while (1);
    }

    if (regval & (1UL << 30))
    {
        //ddr2 work 2x mode
        //dphyclk=cpu pll, hclk=dclk=1/2 cpu pll
        dphyclk = cpu_pll_clk;
        hclk = cpu_pll_clk / 2;
        dclk = hclk;
    }
    else
    {
        //ddr2 work 1x mode
        //dphy clock mode define by cpu_5x_sel
        if (regval & (1UL << 26))
        {
            //cpu work 5x mode
            //jclk = cpu pll, hclk=dclk=dphyclk=1/5 cpu pll
            dphyclk = cpu_pll_clk / 5;
            hclk = dphyclk;
            dclk = hclk;
        }
        else
        {
            //cpu work 2x mode
            //jclk = cpu pll, hclk=dclk=dphyclk=cpu pll even div clk
            u32 pll_even_div = (regval >> 15) & 0x03;
            u32 pll_even_clk = cpu_pll_clk / (2 * (pll_even_div + 1));
            dphyclk = pll_even_clk;
            hclk = pll_even_clk;
            dclk = hclk;
        }
    }

    if (dphyclk > DPHY_CLK_MAX_RATE) {
        printf("Can't set dphy clk rate %u, Max rate: %u! halt!!!\n", dphyclk, DPHY_CLK_MAX_RATE);
        while (1);
    }
    if (hclk > HCLK_MAX_RATE) {
        printf("Can't set hclk rate %u, Max rate: %u! halt!!!\n", hclk, HCLK_MAX_RATE);
        while (1);
    }
    if (dclk > HCLK_MAX_RATE) {
        printf("Can't set dclk rate %u, Max rate: %u! halt!!!\n", dclk, HCLK_MAX_RATE);
        while (1);
    }

}

#if defined(CONFIG_DISPLAY_CPUINFO)

#define CPU_PLL_OD_CFG_MASK							(0x3)
#define CPU_PLL_OD_CFG_SHIFT						(12)
#define CPU_PLL_N_CFG_MASK							(0xF)
#define CPU_PLL_N_CFG_SHIFT							(8)
#define CPU_PLL_M_CFG_MASK							(0xFF)
#define CPU_PLL_M_CFG_SHIFT							(0)


static unsigned long ak_get_cpu_pll_clk(void)
{
	u32 pll_m, pll_n, pll_od;
	u32 cpu_pll_clk;
	u32 regval;

	regval = __raw_readl(CPU_PLL_CHANNEL_CTRL_REG);
	pll_od = (regval >> CPU_PLL_OD_CFG_SHIFT) & CPU_PLL_OD_CFG_MASK;
	pll_n = (regval >> CPU_PLL_N_CFG_SHIFT) & CPU_PLL_N_CFG_MASK;
	pll_m = (regval >> CPU_PLL_M_CFG_SHIFT) & CPU_PLL_M_CFG_MASK;

	cpu_pll_clk = 24 * pll_m /(pll_n * (1 << pll_od));

	return cpu_pll_clk;
}

int print_cpuinfo(void)
{
	u32 cpurev = ak_get_cpu_pll_clk();

	printf("CPU: %dMHz\n",cpurev);

    ak_check_pll1_clk();
    return 0;
}
#endif


int get_clocks(void)
{
	return 0;
}


