# SPDX-License-Identifier: GPL-2.0-only
obj-$(CONFIG_MTK_CMDQ) += mtk-cmdq-helper.o
obj-$(CONFIG_MTK_INFRACFG) += mtk-infracfg.o
obj-$(CONFIG_MTK_PMIC_WRAP) += mtk-pmic-wrap.o
obj-$(CONFIG_MTK_SCPSYS) += mtk-scpsys-ext.o mtk-scpsys.o

# Ported from MT6890 Linux 4.19
obj-$(CONFIG_MTK_MBOX) += mtk-mbox.o

# Ported from MT6890 Linux 4.19
obj-$(CONFIG_MTK_IPI) += mtk_tinysys_ipi.o

# DOTY MT6890 Stage-2B vendor parity
obj-$(CONFIG_MTK_SCPSYS_BRINGUP) += mtk-scpsys-bringup.o

# DOTY MT6890 Stage-2B vendor parity
obj-$(CONFIG_MTK_LPM) += mtk-lpm.o

# DOTY MT6890 Stage-2C-A6 vendor DVFSRC provider
obj-$(CONFIG_MTK_DVFSRC) += mtk-dvfsrc.o mtk-dvfsrc-start.o

# DOTY MT6890 parent Kbuild closure v10: drivers/soc/mediatek -> DEVAPC provider
obj-$(CONFIG_MTK_DEVAPC) += devapc/
