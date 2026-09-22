# SPDX-License-Identifier: GPL-2.0-only
#
# Makefile for the Mediatek SoCs built-in ethernet macs
#

obj-$(CONFIG_NET_MEDIATEK_SOC)                 += mtk_eth.o
mtk_eth-y := mtk_eth_soc.o mtk_sgmii.o mtk_eth_path.o

# DOTY FULL PARITY v3 NET_MEDIATEK_DBG
obj-$(CONFIG_NET_MEDIATEK_DBG)			+= mtk_eth_dbg.o

# DOTY MT6890 parent Kbuild closure v10: MediaTek Ethernet -> SGMII bring-up provider
obj-$(CONFIG_NET_MEDIATEK_SOC) += mtk_sgmii_pwr.o
