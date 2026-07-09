#include <linux/linkage.h>
#include <asm/assembler.h>
#include <asm/pgtable.h>
#include <asm/memory.h>
#include "ak_pm.h"

#define FPGA_PLAT_PHYS_OFFSET		0x80000000

.align
.text
ENTRY(ak_pm_resume_in_mem)
	adr	r5, __pm_resume_data	@ __pm_resume_data为虚拟地址，加载在r5
	/**
	 * @ r1保存内核虚拟地址
	 * @ r2保存内核物理地址
	 * @ r3保存数据保存区域的物理起始地址
	 */
	ldmia	r5, {r1, r2, r3}	@ 从__pm_resume_data中获取数据
	
	sub	r4, r2, r1		@r2 - r1	@计算物理地址与虚拟地址的差值

	add	r3, r3, r4
	ldr r8, [r3]				@ 获取保存resume data的物理地址地址

	adr lr, 2f					@ return (PIC) address			

	ldr	r10, [r8, #PM_PROCINFO_LIST_OFFSET]
	ldr	r12, [r8, #PM_PROCINFO_INITFUNC_OFFSET]	// r12 保存proc   info init的地址
	add	r12, r12, r10
	ret r12

2:	adr	r3, __pm_mmu_data
	ldmia	r3, {r5, r6}
	mov r13, r6				@ 获取开启mmu后跳转的__pm_mmap_switched虚拟地址	

	add r3, r8, #PM_IDMAP_MMU_OFFSET
	ldmia	r3, {r0, r1, r2}
	mcr	p15, 0, r1, c2, c0, 0		@ load page table pointer

	add r3, r8, #PM_MMU_OFFSET		@ 获取休眠前IDMAP MMU配置的值
	ldmia	r3, {r0, r1, r2}
	mcr	p15, 0, r2, c3, c0, 0		@ load domain access register
	add	r5, r5, r4					@ 获取__pm_turn_mmu_on的物理地址
	mov	pc, r5
ENDPROC(ak_pm_resume_in_mem)
__pm_resume_data:
	.long PAGE_OFFSET
	.long PLAT_PHYS_OFFSET
	.long __pm_resume_data_pbase
__pm_mmu_data:
	.long __pm_turn_mmu_on
	.long __pm_mmap_switched

ENTRY(ak_pm_resume_in_mem_sz)
	.word .-ak_pm_resume_in_mem

__pm_mmap_switched:
	ldr	r8, =__pm_resume_data_vbase
	ldr r8, [r8]

	@add r0, r8, #PM_CPSR_OFFSET
	@ldr r1, [r0]
	@msr CPSR_c, r1

	add r0, r8, #PM_REGS_OFFSET
	ldmia r0!, {r3 - r14}
	bx lr
ENDPROC(__pm_mmap_switched)

ENTRY(ak_pm_save_data_in_mem)
	mov	ip, #0
	add r0, r0, #PM_MMU_OFFSET
	mrc	p15, 0, ip, c1, c0
	str ip, [r0], #4
	mrc	p15, 0, ip, c2, c0
	str ip, [r0], #4
	mrc	p15, 0, ip, c3, c0
	str ip, [r0], #4
	ret lr
ENDPROC(ak_pm_save_data_in_mem)

	.align	5
	.pushsection	.idmap.text, "ax"
ENTRY(__pm_turn_mmu_on)		// 开启mmu代码
	mov	r0, r0
	instr_sync
	mcr	p15, 0, r0, c1, c0, 0
	mrc	p15, 0, r3, c0, c0, 0
	instr_sync
	mov	r3, r3
	mov	r3, r13
	ret	r3
ENDPROC(__pm_turn_mmu_on)
	.popsection

	.pushsection .data
	.globl __pm_resume_data_pbase
	.globl __pm_resume_data_vbase

__pm_resume_data_pbase:
	.long 0
__pm_resume_data_vbase:
	.long 0
	.popsection

	.align	5
	.pushsection	.idmap.text, "ax"

   //.extern ak_close_mmu_gpio_on_s
   //.extern ak_close_mmu_gpio_on_s_c

ENTRY(ak_pm_suspend_in_mem)

	mov r4, #0
   	mrc p15, 0, r4, c1, c0, 0	@DisableICache DisableDCache DisableMMU
   	bic r4, r4, #0x0003
   	bic r4, r4, #0x1000
   	mcr p15, 0, r4, c1, c0, 0

	mov	ip, #0
	add r2, r0, #PM_IDMAP_MMU_OFFSET
	mrc	p15, 0, ip, c1, c0
	str ip, [r2], #4
	mrc	p15, 0, ip, c2, c0
	str ip, [r2], #4
	mrc	p15, 0, ip, c3, c0
	str ip, [r2], #4

	add r2, r0, #PM_CPSR_OFFSET
	mrs r1, CPSR

	add r2, r0, #PM_REGS_OFFSET
	/* Save registers on resume mem */
	stmia r2!, {r3 - r14}		// r1: resume data mem base

#if 0
   /*点灯验证start，可以点亮*/
   
	LDR r1, =0x08000198    
	LDR r2, [r1]           
	BIC r2, r2, #0x70000000
	STR r2, [r1]
	
	LDR r1, =0x201700008   
	LDR r2, [r1]           
	ORR r2, r2, #0x40      
	STR r2, [r1]           
	LDR r1, =0x2017001c    
	LDR r2, [r1]           
	ORR r2, r2, #0x40      
	STR r2, [r1]  

  /*点灯验证end*/
#endif

#if 0
   /*采用调用c函数可以点亮*/
   LDR sp, =0x82D80300  /* 将堆栈指针设置为0x82D80300 */
       /* 调用C函数 */
    BL ak_close_mmu_gpio_on_s     /* 调用C函数 */

    /* 停止程序 */
    B .                  /* 挂起程序，防止退出 */
#endif

#if 0
   /*采用调用c函数,含参数可以点亮*/
   LDR sp, =0x82D80300  /* 将堆栈指针设置为0x82D80300 */
    MOV r0, #1          // 参数1
       /* 调用C函数 */
    BL ak_close_mmu_gpio_on_s_c     /* 调用C函数 */

    /* 停止程序 */
    B .                  /* 挂起程序，防止退出 */
#endif

#if 1
    LDR sp, =0x81400300  /* 将堆栈指针设置为0x82D80300 */
    MOV r0, #3          // 参数1,3表示mem
    MOV r1, #0          // 参数2,1表示reboot,所以默认要置0
	LDR r6, =0x48000000
	BLX r6
#endif
  
ENDPROC(ak_pm_suspend_in_mem)
	.popsection
