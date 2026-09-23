# SPDX-License-Identifier: GPL-2.0-only

# DOTY MT6890: AEE headers needed by CONFIG_LOG_TOO_MUCH_WARNING printk port
ccflags-$(CONFIG_LOG_TOO_MUCH_WARNING) += -I$(srctree)/drivers/misc/mediatek/include
obj-y	= printk.o
obj-$(CONFIG_PRINTK)	+= printk_safe.o
obj-$(CONFIG_A11Y_BRAILLE_CONSOLE)	+= braille.o
