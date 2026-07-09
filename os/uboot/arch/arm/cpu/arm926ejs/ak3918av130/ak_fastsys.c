#include <asm/arch-ak3918av130/ak_cpu.h>
#include <common.h>
#include <command.h>
#include <image.h>

#define SLAVE_CPU_JUMP_ADDR        0x480014FC
// slave cpu clock & reset config
#define SLAVE_CPU_CLK_RST_CFG_REG       (CHIP_CONF_BASE_ADDR + 0x00000028)

//SLAVE CPU AHB HANT
#define CPU_S_AHB_HANT_BASE_ADDR          0x21103000
#define CPU_S_AHB_HANT_CTRL_REG         (CPU_S_AHB_HANT_BASE_ADDR+0x04)

void run_slave_cpu(u32 run_addr, u32 size)
{
    flush_dcache_range(run_addr, run_addr + size);
    REG32(SLAVE_CPU_JUMP_ADDR) = run_addr;

    REG32(SLAVE_CPU_CLK_RST_CFG_REG) &= (~(0x1<<31));

    REG32(SLAVE_CPU_CLK_RST_CFG_REG) &= (~(0x1<<30));

    REG32(CPU_S_AHB_HANT_CTRL_REG) |= (0x1<<0);
}

static int do_go_fastsys(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    ulong addr, size = 0x300000;

    addr = simple_strtoul(argv[1], NULL, 16) + sizeof(image_header_t);
    printf("go fastsys addr 0x%x\n", (unsigned int)addr);
    if (!addr)
        return -1;
    run_slave_cpu(addr, size);
    return 0;
}

U_BOOT_CMD(
    go_fastsys, CONFIG_SYS_MAXARGS, 1, do_go_fastsys,
    "Run fastsys",
    ""
);
