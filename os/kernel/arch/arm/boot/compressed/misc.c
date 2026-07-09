/*
 * misc.c
 * 
 * This is a collection of several routines from gzip-1.0.3 
 * adapted for Linux.
 *
 * malloc by Hannu Savolainen 1993 and Matthias Urlichs 1994
 *
 * Modified for ARM Linux by Russell King
 *
 * Nicolas Pitre <nico@visuaide.com>  1999/04/14 :
 *  For this code to run directly from Flash, all constant variables must
 *  be marked with 'const' and all other variables initialized at run-time 
 *  only.  This way all non constant variables will end up in the bss segment,
 *  which should point to addresses in RAM and cleared to 0 on start.
 *  This allows for a much quicker boot time.
 */

unsigned int __machine_arch_type;

#include <linux/compiler.h>	/* for inline */
#include <linux/types.h>
#include <linux/linkage.h>

static void putstr(const char *ptr);
extern void error(char *x);

#include CONFIG_UNCOMPRESS_INCLUDE

#ifdef CONFIG_DEBUG_ICEDCC

#if defined(CONFIG_CPU_V6) || defined(CONFIG_CPU_V6K) || defined(CONFIG_CPU_V7)

static void icedcc_putc(int ch)
{
	int status, i = 0x4000000;

	do {
		if (--i < 0)
			return;

		asm volatile ("mrc p14, 0, %0, c0, c1, 0" : "=r" (status));
	} while (status & (1 << 29));

	asm("mcr p14, 0, %0, c0, c5, 0" : : "r" (ch));
}


#elif defined(CONFIG_CPU_XSCALE)

static void icedcc_putc(int ch)
{
	int status, i = 0x4000000;

	do {
		if (--i < 0)
			return;

		asm volatile ("mrc p14, 0, %0, c14, c0, 0" : "=r" (status));
	} while (status & (1 << 28));

	asm("mcr p14, 0, %0, c8, c0, 0" : : "r" (ch));
}

#else

static void icedcc_putc(int ch)
{
	int status, i = 0x4000000;

	do {
		if (--i < 0)
			return;

		asm volatile ("mrc p14, 0, %0, c0, c0, 0" : "=r" (status));
	} while (status & 2);

	asm("mcr p14, 0, %0, c1, c0, 0" : : "r" (ch));
}

#endif

#define putch(ch)	icedcc_putc(ch)
#endif

static void putstr(const char *ptr)
{
	char c;

	while ((c = *ptr++) != '\0') {
		if (c == '\n')
			putch('\r');
		putch(c);
	}

	flush();
}

/*
 * gzip declarations
 */
extern char input_data[];
extern char input_data_end[];

unsigned char *output_data;

unsigned long free_mem_ptr;
unsigned long free_mem_end_ptr;

#ifndef arch_error
#define arch_error(x)
#endif

void error(char *x)
{
	arch_error(x);

	putstr("\n\n");
	putstr(x);
	putstr("\n\n -- System halted");

	while(1);	/* Halt */
}

asmlinkage void __div0(void)
{
	error("Attempting division by 0!");
}

unsigned long __stack_chk_guard;

void __stack_chk_guard_setup(void)
{
	__stack_chk_guard = 0x000a0dff;
}

void __stack_chk_fail(void)
{
	error("stack-protector: Kernel stack is corrupted\n");
}

#ifdef CONFIG_AK_PM_TIME_TEST
void pr_int(unsigned int x)
{
    int i = 0;
    char c;
    putch('0');
    putch('x');
    for (i = 0; i < sizeof(x) * 2; i++) {
        c = (x >> 28) & 0xf;
        if (c >= 10)
            c = 'A' + c - 10;
        else
            c = '0' + c;
        putch(c);
        x = x << 4;
    }
}

#define MODULE_CONF_BASE_ADDR   0x20000000
#define TIMER_BASE_ADDR (MODULE_CONF_BASE_ADDR + 0x1110000)
/*use timer 1*/
#define TICK_TIMER      (1)
#define TICK_TIMER_BASE (TIMER_BASE_ADDR + (TICK_TIMER * 0x1000))

#define RET_TIMER_VALUE        (0x04)
#endif

extern int do_decompress(u8 *input, int len, u8 *output, void (*error)(char *x));


void
decompress_kernel(unsigned long output_start, unsigned long free_mem_ptr_p,
		unsigned long free_mem_ptr_end_p,
		int arch_id)
{
	int ret;

	__stack_chk_guard_setup();

	output_data		= (unsigned char *)output_start;
	free_mem_ptr		= free_mem_ptr_p;
	free_mem_end_ptr	= free_mem_ptr_end_p;
	__machine_arch_type	= arch_id;

	arch_decomp_setup();

#if !(defined(CONFIG_MACH_KM01A) || defined(CONFIG_MACH_AK3918AV130))
	putstr("Uncompressing Linux...");
#else
    #ifdef CONFIG_AK_PM_TIME_TEST
        putstr("uncompress tc:");
        pr_int(REG32(TICK_TIMER_BASE+RET_TIMER_VALUE));
        putstr("\n");
    #endif
	//putstr("...");
#endif
	ret = do_decompress(input_data, input_data_end - input_data,
			    output_data, error);
	if (ret)
		error("decompressor returned an error");
	else

#if !(defined(CONFIG_MACH_KM01A) || defined(CONFIG_MACH_AK3918AV130))
		putstr(" done, booting the kernel.\n");
#else
        #ifdef CONFIG_AK_PM_TIME_TEST
            putstr("done tc:");
            pr_int(REG32(TICK_TIMER_BASE+RET_TIMER_VALUE));
            putstr("\n");
        #endif
		putstr("booting kernel\n");
#endif
}
