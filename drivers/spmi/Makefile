# DOTY MT6890 PMIF/AEE compatibility include
ccflags-y += -I$(srctree)/drivers/misc/mediatek/include
# SPDX-License-Identifier: GPL-2.0-only
#
# Makefile for kernel SPMI framework.
#
obj-$(CONFIG_SPMI)	+= spmi.o

obj-$(CONFIG_SPMI_MSM_PMIC_ARB)	+= spmi-pmic-arb.o

# DOTY-VENDOR-BOOT-PARITY MTK_PMIF BEGIN
obj-$(CONFIG_MTK_PMIF)  += mtk-pmif.o mtk-dbg.o
# DOTY-VENDOR-BOOT-PARITY MTK_PMIF END
