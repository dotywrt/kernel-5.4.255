# SPDX-License-Identifier: GPL-2.0
ccflags-y += -I$(srctree)/drivers/misc/mediatek/include

obj-$(CONFIG_MTK_EMI)	+= emi-cen.o
obj-$(CONFIG_MTK_EMI)  += emi-mpu.o
obj-$(CONFIG_MTK_EMI)  += emi-dbg.o
obj-$(CONFIG_MTK_EMI)  += emi-isu.o
obj-$(CONFIG_MTK_EMI)  += emi-mpu-test.o
obj-$(CONFIG_MTK_DRAMC)    += mtk_dramc.o
mtk_dramc-objs	+= dramc.o dramc_bin.o

# MT6890 Linux 5.4: final built-in EMI MPU provider required by MEDMCU
obj-y += emi-mpu.o
