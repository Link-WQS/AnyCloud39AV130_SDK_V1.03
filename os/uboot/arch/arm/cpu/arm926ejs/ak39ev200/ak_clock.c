#include <asm/io.h>
#include <asm/arch-ak39ev200/ak_cpu.h>
#include <common.h>

#define PLL_CLK_MIN	180

// cdh:check ok
unsigned long get_asic_pll_clk(void)
{
	unsigned long pll_m, pll_n, pll_od;
	unsigned long asic_pll_clk;
	unsigned long regval;

	regval = readl(CLK_ASIC_PLL_CTRL);
	pll_od = (regval & (0x3 << 12)) >> 12;
	pll_n = (regval & (0xf << 8)) >> 8;
	pll_m = regval & 0xff;

	asic_pll_clk = (12 * pll_m)/(pll_n * (1 << pll_od));

	if ((pll_od >= 1) && ((pll_n >= 2) && (pll_n <= 6)) 
		&& ((pll_m >= 84) && (pll_m <= 254)))
		return asic_pll_clk;

	return 0;
}

// cdh:check ok
unsigned long get_vclk(void)
{
	unsigned long regval;
	unsigned long div;
	
	regval = readl(CLK_ASIC_PLL_CTRL);
	div = (regval & (0x7 << 17)) >> 17;
	if (div == 0)
		regval = get_asic_pll_clk() >> 1;
	else
		regval = get_asic_pll_clk() >> div;

	return regval;
}

// cdh:check ok
unsigned long get_asic_freq(void)
{
	unsigned long regval;
	unsigned long div;

	regval = readl(CLK_ASIC_PLL_CTRL);
	div = regval & (1 << 24); // cdh:check vclk
	if (div == 0)
		regval =  get_vclk() * 1000000;
	else
		regval = (get_vclk() >> 1) * 1000000;

	return regval;
}

static void ak39ev200_check_asic_clock(void)
{
#define AISC_PLL_MAX_RATE                (400000000)
#define VCLK_MAX_RATE1                   (125000000)
#define VCLK_MAX_RATE2                   (200000000)
    unsigned long rate;

    rate = get_asic_pll_clk();
    if (rate > AISC_PLL_MAX_RATE) {
        printf("Can't set ASIC PLL rate %lu, Max rate: %d! halt!!!\n", rate, AISC_PLL_MAX_RATE);
        while (1);
    }
    rate = get_vclk();
    if (readl(CLK_ASIC_PLL_CTRL) & (1 << 24)) {
        if (rate > VCLK_MAX_RATE1) { 
            printf("Can't set vclk rate %lu, Max rate1: %d! halt!!!\n", rate, VCLK_MAX_RATE1);
            while (1);
        }
    } else {
        if (rate > VCLK_MAX_RATE2) { 
            printf("Can't set vclk rate %lu, Max rate2: %d! halt!!!\n", rate, VCLK_MAX_RATE2);
            while (1);
        }
    }
}

static void ak39ev200_check_cpu_clock(void)
{
#define CPU_PLL_MAX_RATE                (400000000)
#define CPU_DCLK_MAX_RATE               (200000000)
#define CPU_HCLK_MAX_RATE               (200000000)
#define CPU_CLK_MAX_RATE                (400000000)
#define AHB_CLK_MAX_RATE                (200000000)
#define MEM_CLK_MAX_RATE                (200000000)

    u32 regval, cpu_pll_clk, cpu_dclk, cpu_hclk, cpu_clk, ahb_clk, mem_clk;
    u32 pll_m, pll_n, pll_od;
    u8 div[8] = {2, 2, 4, 8, 16, 32, 64, 128};

    regval = __raw_readl(CPU_PLL_CHANNEL_CTRL_REG);
    pll_od = (regval >> 12) & 0x3;
    pll_n = (regval >> 8) & 0xF;
    pll_m = (regval) & 0xff;

    cpu_pll_clk = 12 * pll_m /(pll_n * (1 << pll_od));

    cpu_pll_clk = cpu_pll_clk * 1000000;
    if (cpu_pll_clk > CPU_PLL_MAX_RATE) {
        printf("Can't set CPU PLL rate %d, Max rate: %d! halt!!!\n", cpu_pll_clk, CPU_PLL_MAX_RATE);
        while (1);
    }
    cpu_hclk = cpu_pll_clk / div[((regval >> 17) & 0x7)];
    if (cpu_hclk > CPU_HCLK_MAX_RATE) {
        printf("Can't set CPU HCLK rate %d, Max rate: %d! halt!!!\n", cpu_hclk, CPU_HCLK_MAX_RATE);
        while (1);
    }
    cpu_dclk = cpu_pll_clk / div[((regval >> 20) & 0x7)];
    if (cpu_dclk > CPU_DCLK_MAX_RATE) {
        printf("Can't set CPU DCLK rate %d, Max rate: %d! halt!!!\n", cpu_dclk, CPU_DCLK_MAX_RATE);
        while (1);
    }
    if ((!(regval & (1UL << 26)) && (regval & (1UL << 24))) ||
        (!(regval & (1UL << 24)) && (regval & (1UL << 26)))) {
        cpu_clk = cpu_pll_clk;
        ahb_clk = cpu_hclk;
        mem_clk = cpu_dclk;
    }

    if (!(regval & (1UL << 24)) && !(regval & (1UL << 26))) {
        cpu_clk = cpu_pll_clk;
        ahb_clk = cpu_pll_clk / 3;
        mem_clk = cpu_pll_clk / 3;
    }

    if (cpu_clk > CPU_CLK_MAX_RATE) {
        printf("Can't set cpu clk rate %u, Max rate: %d! halt!!!\n", cpu_clk, CPU_CLK_MAX_RATE);
        while (1);
    }
    if (ahb_clk > AHB_CLK_MAX_RATE) {
        printf("Can't set ahb clk rate %u, Max rate: %d! halt!!!\n", ahb_clk, AHB_CLK_MAX_RATE);
        while (1);
    }
    if (mem_clk > MEM_CLK_MAX_RATE) {
        printf("Can't set mem clk rate %u, Max rate: %d! halt!!!\n", mem_clk, MEM_CLK_MAX_RATE);
        while (1);
    }
}

/**
 * ak39ev200_check_clock - check ak39ev200 chips clock
 */
void ak39ev200_check_clock(void)
{
    ak39ev200_check_cpu_clock();
    ak39ev200_check_asic_clock();
}

#if defined(CONFIG_DISPLAY_CPUINFO)
int print_cpuinfo(void)
{
	u32 cpurev = get_asic_pll_clk();

	printf("ASIC PLL: %dMHz\n",cpurev);
    ak39ev200_check_clock();
	return 0;
}
#endif


int get_clocks(void)
{
	return 0;
}


