#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

__visible struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=usbcore,bluetooth";

MODULE_ALIAS("usb:vA69Cp8801d*dc*dsc*dp*icE0isc01ip01in*");
MODULE_ALIAS("usb:vA69Cp8D81d*dc*dsc*dp*icE0isc01ip01in*");
MODULE_ALIAS("usb:vA69Cp88DCd*dc*dsc*dp*icE0isc01ip01in*");

MODULE_INFO(srcversion, "EBEA80423A80576EA99E83F");
