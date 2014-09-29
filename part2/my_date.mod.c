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

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x98fc5a6, __VMLINUX_SYMBOL_STR(module_layout) },
	{ 0x62978a8, __VMLINUX_SYMBOL_STR(single_release) },
	{ 0xe0f4d96e, __VMLINUX_SYMBOL_STR(seq_read) },
	{ 0x2b14b8a3, __VMLINUX_SYMBOL_STR(remove_proc_entry) },
	{ 0x50eedeb8, __VMLINUX_SYMBOL_STR(printk) },
	{ 0x232537bd, __VMLINUX_SYMBOL_STR(proc_create_data) },
	{ 0xfd485ec8, __VMLINUX_SYMBOL_STR(seq_printf) },
	{ 0x92a9c60c, __VMLINUX_SYMBOL_STR(time_to_tm) },
	{ 0x34184afe, __VMLINUX_SYMBOL_STR(current_kernel_time) },
	{ 0x7ebfdbb6, __VMLINUX_SYMBOL_STR(single_open) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";

