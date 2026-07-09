// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

/* #define	DEBUG	*/

#include <common.h>
#include <autoboot.h>
#include <cli.h>
#include <console.h>
#include <env.h>
#include <version.h>

/*
 * Board-specific Platform code can reimplement show_boot_progress () if needed
 */
__weak void show_boot_progress(int val) {}

static void run_preboot_environment_command(void)
{
	char *p;

	p = env_get("preboot");
	if (p != NULL) {
		int prev = 0;

		if (IS_ENABLED(CONFIG_AUTOBOOT_KEYED))
			prev = disable_ctrlc(1); /* disable Ctrl-C checking */

		run_command_list(p, -1, 0);

		if (IS_ENABLED(CONFIG_AUTOBOOT_KEYED))
			disable_ctrlc(prev);	/* restore Ctrl-C checking */
	}
}

#if defined(CONFIG_AK_AUTOUPGRADE)

#define UPGRADE_BOOTDELAY           10
#define UPGRADE_FAILURE_MAX         3

__weak int autoupgrade_firmware(void)
{
    printf("\n###### __weak autoupgrade_firmware. fixme\n\n");
    return 0;
}

static void ak_autoupgrade(void)
{
    int ret = 0;
    int upgrade_failure = env_get_ulong("upgrade_failure", 10, 0);

    ret = autoupgrade_firmware();

    if (ret != 0) {
        upgrade_failure++;
        printf("upgrade_failure[%d]. max[%d]\n", upgrade_failure, UPGRADE_FAILURE_MAX);
        if (upgrade_failure < UPGRADE_FAILURE_MAX) {
            //保存环境变量
            env_set_ulong("upgrade_failure", upgrade_failure);
            env_save();
            env_set_ulong("bootdelay", UPGRADE_BOOTDELAY);
            return ;
        }
        printf("upgrade_failure[%d] >= max[%d], clear flags and status\n", upgrade_failure, UPGRADE_FAILURE_MAX);
    }

    env_set_ulong("upgrade_flag", 0);
    env_set_ulong("upgrade_failure", 0);
    env_save();
    run_command("printenv", 0);
#if 0
    env_set_ulong("bootdelay", UPGRADE_BOOTDELAY);
#else
    run_command("reset", 0);
#endif
}

#endif

/* We come here after U-Boot is initialised and ready to process commands */
void main_loop(void)
{
	const char *s;

	bootstage_mark_name(BOOTSTAGE_ID_MAIN_LOOP, "main_loop");

	if (IS_ENABLED(CONFIG_VERSION_VARIABLE))
		env_set("ver", version_string);  /* set version variable */

	cli_init();

	if (IS_ENABLED(CONFIG_USE_PREBOOT))
		run_preboot_environment_command();

	if (IS_ENABLED(CONFIG_UPDATE_TFTP))
		update_tftp(0UL, NULL, NULL);

#if defined(CONFIG_AK_AUTOUPGRADE)
    ak_autoupgrade();
#endif
	s = bootdelay_process();
	if (cli_process_fdt(&s))
		cli_secure_boot_cmd(s);

	autoboot_command(s);

	cli_loop();
	panic("No CLI available");
}
