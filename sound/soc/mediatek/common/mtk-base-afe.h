/* SPDX-License-Identifier: GPL-2.0 */
/*
 * mtk-base-afe.h  --  Mediatek base afe structure
 *
 * Copyright (c) 2016 MediaTek Inc.
 * Author: Garlic Tseng <garlic.tseng@mediatek.com>
 */

#ifndef _MTK_BASE_AFE_H_
#define _MTK_BASE_AFE_H_

#define MTK_STREAM_NUM (SNDRV_PCM_STREAM_LAST + 1)

struct mtk_base_memif_data {
	int id;
	const char *name;
	int reg_ofs_base;
	int reg_ofs_cur;
	int fs_reg;
	int fs_shift;
	int fs_maskbit;
	int mono_reg;
	int mono_shift;
	int enable_reg;
	int enable_shift;
	int hd_reg;
	int hd_align_reg;
	int hd_shift;
	int hd_align_mshift;
	int msb_reg;
	int msb_shift;
	int agent_disable_reg;
	int agent_disable_shift;

	/* DOTY-MT6880-STRUCT-SYNC-mtk_base_memif_data-BEGIN */
	/* Exact member declarations restored from vendor Linux 4.19. */
	/* reg_ofs_end */
	int reg_ofs_end;
	/* reg_ofs_base_msb */
	int reg_ofs_base_msb;
	/* reg_ofs_cur_msb */
	int reg_ofs_cur_msb;
	/* reg_ofs_end_msb */
	int reg_ofs_end_msb;
	/* pbuf_reg */
	int pbuf_reg;
	/* pbuf_mask_shift */
	int pbuf_mask_shift;
	/* pbuf_shift */
	int pbuf_shift;
	/* minlen_reg */
	int minlen_reg;
	/* minlen_mask_shift */
	int minlen_mask_shift;
	/* minlen_shift */
	int minlen_shift;
	/* mono_invert */
	int mono_invert;
	/* DOTY-MT6880-STRUCT-SYNC-mtk_base_memif_data-END */
};

struct mtk_base_irq_data {
	int id;
	int irq_cnt_reg;
	int irq_cnt_shift;
	int irq_cnt_maskbit;
	int irq_fs_reg;
	int irq_fs_shift;
	int irq_fs_maskbit;
	int irq_en_reg;
	int irq_en_shift;
	int irq_clr_reg;
	int irq_clr_shift;

	/* DOTY-MT6880-STRUCT-SYNC-mtk_base_irq_data-BEGIN */
	/* Exact member declarations restored from vendor Linux 4.19. */
	/* irq_ap_en_reg */
	int irq_ap_en_reg;
	/* irq_ap_en_shift */
	int irq_ap_en_shift;
	/* irq_scp_en_reg */
	int irq_scp_en_reg;
	/* irq_scp_en_shift */
	int irq_scp_en_shift;
	/* DOTY-MT6880-STRUCT-SYNC-mtk_base_irq_data-END */
};

struct device;
struct list_head;
struct mtk_base_afe_memif;
struct mtk_base_afe_irq;
struct mtk_base_afe_dai;
struct regmap;
struct snd_pcm_substream;
struct snd_soc_dai;

struct mtk_base_afe {
	void __iomem *base_addr;
	struct device *dev;
	struct regmap *regmap;
	struct mutex irq_alloc_lock; /* dynamic alloc irq lock */

	unsigned int const *reg_back_up_list;
	unsigned int *reg_back_up;
	unsigned int reg_back_up_list_num;

	int (*runtime_suspend)(struct device *dev);
	int (*runtime_resume)(struct device *dev);
	bool suspended;

	struct mtk_base_afe_memif *memif;
	int memif_size;
	struct mtk_base_afe_irq *irqs;
	int irqs_size;

	struct list_head sub_dais;
	struct snd_soc_dai_driver *dai_drivers;
	unsigned int num_dai_drivers;

	const struct snd_pcm_hardware *mtk_afe_hardware;
	int (*memif_fs)(struct snd_pcm_substream *substream,
			unsigned int rate);
	int (*irq_fs)(struct snd_pcm_substream *substream,
		      unsigned int rate);

	void *platform_priv;

	/* DOTY-MTK-AFE-DEBUG-COMPAT-BEGIN */
	/* Exact member declarations restored from vendor Linux 4.19. */
	/* debug_cmds */
	const struct mtk_afe_debug_cmd *debug_cmds;
	/* DOTY-MTK-AFE-DEBUG-COMPAT-END */

	/* DOTY-MT6880-STRUCT-SYNC-mtk_base_afe-BEGIN */
	/* Exact member declarations restored from vendor Linux 4.19. */
	/* sram */
	void *sram;
	/* get_dai_fs */
	int (*get_dai_fs)(struct mtk_base_afe *afe,
			  int dai_id, unsigned int rate);
	/* get_memif_pbuf_size */
	int (*get_memif_pbuf_size)(struct snd_pcm_substream *substream);
	/* request_dram_resource */
	int (*request_dram_resource)(struct device *dev);
	/* release_dram_resource */
	int (*release_dram_resource)(struct device *dev);
	/* debugfs */
	struct dentry *debugfs;
	/* DOTY-MT6880-STRUCT-SYNC-mtk_base_afe-END */
};

struct mtk_base_afe_memif {
	unsigned int phys_buf_addr;
	int buffer_size;
	struct snd_pcm_substream *substream;
	const struct mtk_base_memif_data *data;
	int irq_usage;
	int const_irq;

	/* DOTY-MT6880-STRUCT-SYNC-mtk_base_afe_memif-BEGIN */
	/* Exact member declarations restored from vendor Linux 4.19. */
	/* ack */
	int (*ack)(struct snd_pcm_substream *substream);
	/* ack_enable */
	bool ack_enable;
	/* use_dram_only */
	int use_dram_only;
	/* DOTY-MT6880-STRUCT-SYNC-mtk_base_afe_memif-END */

	/* DOTY-MTK-MEMIF-DMA-LINUX54-BEGIN */
	unsigned char *dma_area;
	dma_addr_t dma_addr;
	size_t dma_bytes;
	/* DOTY-MTK-MEMIF-DMA-LINUX54-END */
};

struct mtk_base_afe_irq {
	const struct mtk_base_irq_data *irq_data;
	int irq_occupyed;
};

struct mtk_base_afe_dai {
	struct snd_soc_dai_driver *dai_drivers;
	unsigned int num_dai_drivers;

	const struct snd_kcontrol_new *controls;
	unsigned int num_controls;
	const struct snd_soc_dapm_widget *dapm_widgets;
	unsigned int num_dapm_widgets;
	const struct snd_soc_dapm_route *dapm_routes;
	unsigned int num_dapm_routes;

	struct list_head list;
};

#endif

