# SPDX-License-Identifier: GPL-2.0
# platform driver

# DOTY-MTK-USIP-CCCI-INCLUDE-LINUX54
ccflags-y += -I$(srctree)/drivers/misc/mediatek/include/mt-plat
snd-soc-mtk-common-objs := mtk-afe-platform-driver.o mtk-afe-fe-dai.o
obj-$(CONFIG_SND_SOC_MEDIATEK) += snd-soc-mtk-common.o

obj-$(CONFIG_SND_SOC_MTK_BTCVSD) += mtk-btcvsd.o

# DOTY-AW1000-STAGE3: rule restored from local 4.19 donor
obj-$(CONFIG_SND_SOC_MTK_SMART_PHONE) += snd-soc-mtk-sp-common.o

# DOTY-AW1000-STAGE3: rule restored from local 4.19 donor
obj-$(CONFIG_SND_SOC_MTK_SRAM) += snd-soc-mtk-sram-manager.o

# DOTY-MTK-AUDIO-COMMON-LINUX54: snd-soc-mtk-sp-common composite rule
snd-soc-mtk-sp-common-objs := mtk-sp-pcm-ops.o mtk-afe-debug.o mtk-usip.o mtk-sp-spk-amp.o mtk-sp-ext-amp.o

# DOTY-MTK-AUDIO-COMMON-LINUX54: snd-soc-mtk-sram-manager composite rule
snd-soc-mtk-sram-manager-objs := mtk-sram-manager.o
