// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2019 MediaTek Inc.
 * Author: Eason Yen <eason.yen@mediatek.com>
 */

#include <linux/module.h>
#include <linux/pm_runtime.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>

#include "../common/mtk-afe-platform-driver.h"
#include "../common/mtk-sp-spk-amp.h"

#include "mt6880-afe-common.h"
#include "mt6880-afe-clk.h"
#include "mt6880-afe-gpio.h"
/* #include "../../codecs/mt6359.h" */

/*
 * if need additional control for the ext spk amp that is connected
 * after Lineout Buffer / HP Buffer on the codec, put the control in
 * mt6880_mt6359_spk_amp_event()
 */
#define EXT_SPK_AMP_W_NAME "Ext_Speaker_Amp"

static const char *const mt6880_spk_type_str[] = {MTK_SPK_NOT_SMARTPA_STR,
						  MTK_SPK_RICHTEK_RT5509_STR,
						  MTK_SPK_MEDIATEK_MT6660_STR,
						  MTK_SPK_NUVOTON_NAU88C22_STR
						  };
static const char *const mt6880_spk_i2s_type_str[] = {MTK_SPK_I2S_0_STR,
						      MTK_SPK_I2S_1_STR,
						      MTK_SPK_I2S_2_STR,
						      MTK_SPK_I2S_3_STR,
						      MTK_SPK_I2S_5_STR};

static const struct soc_enum mt6880_spk_type_enum[] = {
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(mt6880_spk_type_str),
			    mt6880_spk_type_str),
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(mt6880_spk_i2s_type_str),
			    mt6880_spk_i2s_type_str),
};

/* Use speaker : MTK_SPK_MEDIATEK_MT6660 on i2s0 and i2s3 */
static int mt6880_spk_type_get(struct snd_kcontrol *kcontrol,
			       struct snd_ctl_elem_value *ucontrol)
{
	int idx = mtk_spk_get_type();

	pr_debug("%s() = %d\n", __func__, idx);
	ucontrol->value.integer.value[0] = idx;
	return 0;
}

static int mt6880_spk_i2s_out_type_get(struct snd_kcontrol *kcontrol,
				       struct snd_ctl_elem_value *ucontrol)
{
	int idx = mtk_spk_get_i2s_out_type();

	pr_debug("%s() = %d\n", __func__, idx);
	ucontrol->value.integer.value[0] = idx;
	return 0;
}

static int mt6880_spk_i2s_in_type_get(struct snd_kcontrol *kcontrol,
				      struct snd_ctl_elem_value *ucontrol)
{
	int idx = mtk_spk_get_i2s_in_type();

	pr_debug("%s() = %d\n", __func__, idx);
	ucontrol->value.integer.value[0] = idx;
	return 0;
}

static int mt6880_mt6359_spk_amp_event(struct snd_soc_dapm_widget *w,
				       struct snd_kcontrol *kcontrol,
				       int event)
{
	struct snd_soc_dapm_context *dapm = w->dapm;
	struct snd_soc_card *card = dapm->card;

	dev_info(card->dev, "%s(), event %d\n", __func__, event);

	switch (event) {
	case SND_SOC_DAPM_POST_PMU:
		/* spk amp on control */
		break;
	case SND_SOC_DAPM_PRE_PMD:
		/* spk amp off control */
		break;
	default:
		break;
	}

	return 0;
};

static const struct snd_soc_dapm_widget mt6880_mt6359_widgets[] = {
	SND_SOC_DAPM_SPK(EXT_SPK_AMP_W_NAME, mt6880_mt6359_spk_amp_event),
};

static const struct snd_soc_dapm_route mt6880_mt6359_routes[] = {
};

static const struct snd_kcontrol_new mt6880_mt6359_controls[] = {
	SOC_DAPM_PIN_SWITCH(EXT_SPK_AMP_W_NAME),
	SOC_ENUM_EXT("MTK_SPK_TYPE_GET", mt6880_spk_type_enum[0],
		     mt6880_spk_type_get, NULL),
	SOC_ENUM_EXT("MTK_SPK_I2S_OUT_TYPE_GET", mt6880_spk_type_enum[1],
		     mt6880_spk_i2s_out_type_get, NULL),
	SOC_ENUM_EXT("MTK_SPK_I2S_IN_TYPE_GET", mt6880_spk_type_enum[1],
		     mt6880_spk_i2s_in_type_get, NULL),
};

/*
 * define mtk_spk_i2s_mck node in dts when need mclk,
 * BE i2s need assign snd_soc_ops = mt6880_mt6359_i2s_ops
 */
static int mt6880_mt6359_i2s_hw_params(struct snd_pcm_substream *substream,
				       struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	unsigned int rate = params_rate(params);
	unsigned int mclk_fs_ratio = 256;
	unsigned int mclk_fs = rate * mclk_fs_ratio;
	unsigned int cpu_dai_fmt, codec_dai_fmt;
	int ret = 0;

	cpu_dai_fmt = SND_SOC_DAIFMT_I2S;
	codec_dai_fmt = SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_CBS_CFS;

	ret = snd_soc_dai_set_fmt(rtd->cpu_dai, cpu_dai_fmt);
	if (ret != 0 && ret != -ENOTSUPP)
		return ret;

	ret = snd_soc_dai_set_fmt(rtd->codec_dai, codec_dai_fmt);
	if (ret != 0 && ret != -ENOTSUPP)
		return ret;

	return snd_soc_dai_set_sysclk(rtd->cpu_dai, 0,
				      mclk_fs, SND_SOC_CLOCK_OUT);
}
/*end: modify by laizhenhao for I2S*/


static const struct snd_soc_ops mt6880_mt6359_i2s_ops = {
	.hw_params = mt6880_mt6359_i2s_hw_params,
};
#if 0
static int mt6880_mt6359_mtkaif_calibration(struct snd_soc_pcm_runtime *rtd)
{
	struct snd_soc_component *component =
		snd_soc_rtdcom_lookup(rtd, AFE_PCM_NAME);
	struct mtk_base_afe *afe = snd_soc_component_get_drvdata(component);
	struct mt6880_afe_private *afe_priv = afe->platform_priv;
	struct snd_soc_component *codec_component =
		snd_soc_rtdcom_lookup(rtd, CODEC_MT6359_NAME);

	int phase;
	unsigned int monitor;
	int test_done_1, test_done_2, test_done_3;
	int cycle_1, cycle_2, cycle_3;
	int prev_cycle_1, prev_cycle_2, prev_cycle_3;
	int counter;

	dev_info(afe->dev, "%s(), start\n", __func__);

	pm_runtime_get_sync(afe->dev);
	mt6880_afe_gpio_request(afe, true, MT6880_DAI_ADDA, 1);
	mt6880_afe_gpio_request(afe, true, MT6880_DAI_ADDA, 0);
	mt6880_afe_gpio_request(afe, true, MT6880_DAI_ADDA_CH34, 1);
	mt6880_afe_gpio_request(afe, true, MT6880_DAI_ADDA_CH34, 0);

	mt6359_mtkaif_calibration_enable(codec_component);

	/* set clock protocol 2 */
	regmap_update_bits(afe->regmap, AFE_AUD_PAD_TOP, 0xff, 0x38);
	regmap_update_bits(afe->regmap, AFE_AUD_PAD_TOP, 0xff, 0x39);

	/* set test type to synchronizer pulse */
	regmap_update_bits(afe_priv->topckgen, CKSYS_AUD_TOP_CFG,
			   0xffff, 0x4);

	afe_priv->mtkaif_calibration_num_phase = 42;	/* mt6359: 0 ~ 42 */
	afe_priv->mtkaif_calibration_ok = true;
	afe_priv->mtkaif_chosen_phase[0] = -1;
	afe_priv->mtkaif_chosen_phase[1] = -1;
	afe_priv->mtkaif_chosen_phase[2] = -1;

	for (phase = 0;
	     phase <= afe_priv->mtkaif_calibration_num_phase &&
	     afe_priv->mtkaif_calibration_ok;
	     phase++) {
		mt6359_set_mtkaif_calibration_phase(codec_component,
						    phase, phase, phase);

		regmap_update_bits(afe_priv->topckgen, CKSYS_AUD_TOP_CFG,
				   0x1, 0x1);

		test_done_1 = 0;
		test_done_2 = 0;
		test_done_3 = 0;
		cycle_1 = -1;
		cycle_2 = -1;
		cycle_3 = -1;
		counter = 0;
		while (test_done_1 == 0 ||
		       test_done_2 == 0 ||
		       test_done_3 == 0) {
			regmap_read(afe_priv->topckgen, CKSYS_AUD_TOP_MON,
				    &monitor);

			test_done_1 = (monitor >> 28) & 0x1;
			test_done_2 = (monitor >> 29) & 0x1;
			test_done_3 = (monitor >> 30) & 0x1;
			if (test_done_1 == 1)
				cycle_1 = monitor & 0xf;

			if (test_done_2 == 1)
				cycle_2 = (monitor >> 4) & 0xf;

			if (test_done_3 == 1)
				cycle_3 = (monitor >> 8) & 0xf;

			/* handle if never test done */
			if (++counter > 10000) {
				dev_err(afe->dev, "%s(), test fail, cycle_1 %d, cycle_2 %d, cycle_3 %d, monitor 0x%x\n",
					__func__,
					cycle_1, cycle_2, cycle_3, monitor);
				afe_priv->mtkaif_calibration_ok = false;
				break;
			}
		}

		if (phase == 0) {
			prev_cycle_1 = cycle_1;
			prev_cycle_2 = cycle_2;
			prev_cycle_3 = cycle_3;
		}

		if (cycle_1 != prev_cycle_1 &&
		    afe_priv->mtkaif_chosen_phase[0] < 0) {
			afe_priv->mtkaif_chosen_phase[0] = phase - 1;
			afe_priv->mtkaif_phase_cycle[0] = prev_cycle_1;
		}

		if (cycle_2 != prev_cycle_2 &&
		    afe_priv->mtkaif_chosen_phase[1] < 0) {
			afe_priv->mtkaif_chosen_phase[1] = phase - 1;
			afe_priv->mtkaif_phase_cycle[1] = prev_cycle_2;
		}

		if (cycle_3 != prev_cycle_3 &&
		    afe_priv->mtkaif_chosen_phase[2] < 0) {
			afe_priv->mtkaif_chosen_phase[2] = phase - 1;
			afe_priv->mtkaif_phase_cycle[2] = prev_cycle_3;
		}

		regmap_update_bits(afe_priv->topckgen, CKSYS_AUD_TOP_CFG,
				   0x1, 0x0);

		if (afe_priv->mtkaif_chosen_phase[0] >= 0 &&
		    afe_priv->mtkaif_chosen_phase[1] >= 0 &&
		    afe_priv->mtkaif_chosen_phase[2] >= 0)
			break;
	}

	if (afe_priv->mtkaif_chosen_phase[0] < 0 ||
	    afe_priv->mtkaif_chosen_phase[1] < 0 ||
	    afe_priv->mtkaif_chosen_phase[2] < 0)
		afe_priv->mtkaif_calibration_ok = false;

	if (!afe_priv->mtkaif_calibration_ok)
		mt6359_set_mtkaif_calibration_phase(codec_component,
						    0, 0, 0);
	else
		mt6359_set_mtkaif_calibration_phase(codec_component,
			afe_priv->mtkaif_chosen_phase[0],
			afe_priv->mtkaif_chosen_phase[1],
			afe_priv->mtkaif_chosen_phase[2]);

	/* disable rx fifo */
	regmap_update_bits(afe->regmap, AFE_AUD_PAD_TOP, 0xff, 0x38);

	mt6359_mtkaif_calibration_disable(codec_component);

	mt6880_afe_gpio_request(afe, false, MT6880_DAI_ADDA, 1);
	mt6880_afe_gpio_request(afe, false, MT6880_DAI_ADDA, 0);
	mt6880_afe_gpio_request(afe, false, MT6880_DAI_ADDA_CH34, 1);
	mt6880_afe_gpio_request(afe, false, MT6880_DAI_ADDA_CH34, 0);
	pm_runtime_put(afe->dev);

	dev_info(afe->dev, "%s(), end, calibration ok %d\n",
		 __func__,
		 afe_priv->mtkaif_calibration_ok);

	return 0;
}
#endif
#if 0
static int mt6880_mt6359_init(struct snd_soc_pcm_runtime *rtd)
{

	struct snd_soc_component *component =
		snd_soc_rtdcom_lookup(rtd, AFE_PCM_NAME);
	struct mtk_base_afe *afe = snd_soc_component_get_drvdata(component);
	struct mt6880_afe_private *afe_priv = afe->platform_priv;
	struct snd_soc_component *codec_component =
		snd_soc_rtdcom_lookup(rtd, CODEC_MT6359_NAME);
	struct snd_soc_dapm_context *dapm = &rtd->card->dapm;
	/* set mtkaif protocol */
	mt6359_set_mtkaif_protocol(codec_component,
				   MT6359_MTKAIF_PROTOCOL_2_CLK_P2);
	afe_priv->mtkaif_protocol = MTKAIF_PROTOCOL_2_CLK_P2;

	/* mtkaif calibration */
	if (afe_priv->mtkaif_protocol == MTKAIF_PROTOCOL_2_CLK_P2)
		mt6880_mt6359_mtkaif_calibration(rtd);

	/* disable ext amp connection */
	snd_soc_dapm_disable_pin(dapm, EXT_SPK_AMP_W_NAME);

	return 0;
}
#endif

#if 0
static int mt6880_i2s_hw_params_fixup(struct snd_soc_pcm_runtime *rtd,
				      struct snd_pcm_hw_params *params)
{
	dev_info(rtd->dev, "%s(), fix format to 32bit\n", __func__);

	/* fix BE i2s format to 32bit, clean param mask first */
	snd_mask_reset_range(hw_param_mask(params, SNDRV_PCM_HW_PARAM_FORMAT),
			     0, SNDRV_PCM_FORMAT_LAST);

	params_set_format(params, SNDRV_PCM_FORMAT_S32_LE);
	return 0;
}
#endif
					  
/*begin: modify by laizhenhao for I2S*/
/* DOTY-BULK-DAILINK-LINUX54:mt6880_mt6359_dai_links:BEGIN */
/* Endpoint representation converted for Linux 5.4; non-endpoint vendor initializer text preserved. */
static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_000_cpus[] = {
	{
		.dai_name = "DL1",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_000_codecs[] = {
	{
		.name = "snd-soc-dummy",
		.dai_name = "snd-soc-dummy-dai",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_000_platforms[] = {
	{
		0,
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_001_cpus[] = {
	{
		.dai_name = "DL12",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_001_codecs[] = {
	{
		.name = "snd-soc-dummy",
		.dai_name = "snd-soc-dummy-dai",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_001_platforms[] = {
	{
		0,
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_002_cpus[] = {
	{
		.dai_name = "DL2",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_002_codecs[] = {
	{
		.name = "snd-soc-dummy",
		.dai_name = "snd-soc-dummy-dai",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_002_platforms[] = {
	{
		0,
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_003_cpus[] = {
	{
		.dai_name = "DL3",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_003_codecs[] = {
	{
		.name = "snd-soc-dummy",
		.dai_name = "snd-soc-dummy-dai",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_003_platforms[] = {
	{
		0,
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_004_cpus[] = {
	{
		.dai_name = "DL4",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_004_codecs[] = {
	{
		.name = "snd-soc-dummy",
		.dai_name = "snd-soc-dummy-dai",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_004_platforms[] = {
	{
		0,
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_005_cpus[] = {
	{
		.dai_name = "UL1",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_005_codecs[] = {
	{
		.name = "snd-soc-dummy",
		.dai_name = "snd-soc-dummy-dai",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_005_platforms[] = {
	{
		0,
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_006_cpus[] = {
	{
		.dai_name = "UL2",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_006_codecs[] = {
	{
		.name = "snd-soc-dummy",
		.dai_name = "snd-soc-dummy-dai",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_006_platforms[] = {
	{
		0,
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_007_cpus[] = {
	{
		.dai_name = "UL3",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_007_codecs[] = {
	{
		.name = "snd-soc-dummy",
		.dai_name = "snd-soc-dummy-dai",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_007_platforms[] = {
	{
		0,
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_008_cpus[] = {
	{
		.dai_name = "UL4",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_008_codecs[] = {
	{
		.name = "snd-soc-dummy",
		.dai_name = "snd-soc-dummy-dai",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_008_platforms[] = {
	{
		0,
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_009_cpus[] = {
	{
		.dai_name = "UL5",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_009_codecs[] = {
	{
		.name = "snd-soc-dummy",
		.dai_name = "snd-soc-dummy-dai",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_009_platforms[] = {
	{
		0,
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_010_cpus[] = {
	{
		.dai_name = "UL6",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_010_codecs[] = {
	{
		.name = "snd-soc-dummy",
		.dai_name = "snd-soc-dummy-dai",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_010_platforms[] = {
	{
		0,
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_011_cpus[] = {
	{
		.dai_name = "UL7",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_011_codecs[] = {
	{
		.name = "snd-soc-dummy",
		.dai_name = "snd-soc-dummy-dai",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_011_platforms[] = {
	{
		0,
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_012_cpus[] = {
	{
		.dai_name = "UL_MONO_1",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_012_codecs[] = {
	{
		.name = "snd-soc-dummy",
		.dai_name = "snd-soc-dummy-dai",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_012_platforms[] = {
	{
		0,
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_013_cpus[] = {
	{
		.dai_name = "UL_MONO_2",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_013_codecs[] = {
	{
		.name = "snd-soc-dummy",
		.dai_name = "snd-soc-dummy-dai",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_013_platforms[] = {
	{
		0,
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_014_cpus[] = {
	{
		.dai_name = "UL_MONO_3",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_014_codecs[] = {
	{
		.name = "snd-soc-dummy",
		.dai_name = "snd-soc-dummy-dai",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_014_platforms[] = {
	{
		0,
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_015_cpus[] = {
	{
		.dai_name = "HDMI",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_015_codecs[] = {
	{
		.name = "snd-soc-dummy",
		.dai_name = "snd-soc-dummy-dai",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_015_platforms[] = {
	{
		0,
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_016_cpus[] = {
	{
		.dai_name = "Hostless LPBK DAI",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_016_codecs[] = {
	{
		.name = "snd-soc-dummy",
		.dai_name = "snd-soc-dummy-dai",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_016_platforms[] = {
	{
		0,
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_017_cpus[] = {
	{
		.dai_name = "Hostless FM DAI",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_017_codecs[] = {
	{
		.name = "snd-soc-dummy",
		.dai_name = "snd-soc-dummy-dai",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_017_platforms[] = {
	{
		0,
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_018_cpus[] = {
	{
		.dai_name = "Hostless Speech DAI",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_018_codecs[] = {
	{
		.name = "snd-soc-dummy",
		.dai_name = "snd-soc-dummy-dai",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_018_platforms[] = {
	{
		0,
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_019_cpus[] = {
	{
		.dai_name = "Hostless_Sph_Echo_Ref_DAI",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_019_codecs[] = {
	{
		.name = "snd-soc-dummy",
		.dai_name = "snd-soc-dummy-dai",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_019_platforms[] = {
	{
		0,
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_020_cpus[] = {
	{
		.dai_name = "Hostless_Spk_Init_DAI",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_020_codecs[] = {
	{
		.name = "snd-soc-dummy",
		.dai_name = "snd-soc-dummy-dai",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_020_platforms[] = {
	{
		0,
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_021_cpus[] = {
	{
		.dai_name = "Hostless_ADDA_DL_I2S_OUT DAI",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_021_codecs[] = {
	{
		.name = "snd-soc-dummy",
		.dai_name = "snd-soc-dummy-dai",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_021_platforms[] = {
	{
		0,
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_022_cpus[] = {
	{
		.dai_name = "Hostless_SRC_1_DAI",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_022_codecs[] = {
	{
		.name = "snd-soc-dummy",
		.dai_name = "snd-soc-dummy-dai",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_022_platforms[] = {
	{
		0,
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_023_cpus[] = {
	{
		.dai_name = "ADDA",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_023_codecs[] = {
	{
		.dai_name = "mt6359-snd-codec-aif1",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_023_platforms[] = {
	{
		0,
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_024_cpus[] = {
	{
		.dai_name = "ADDA_CH34",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_024_codecs[] = {
	{
		.dai_name = "mt6359-snd-codec-aif2",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_024_platforms[] = {
	{
		0,
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_025_cpus[] = {
	{
		.dai_name = "AP_DMIC",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_025_codecs[] = {
	{
		.name = "snd-soc-dummy",
		.dai_name = "snd-soc-dummy-dai",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_025_platforms[] = {
	{
		0,
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_026_cpus[] = {
	{
		.dai_name = "AP_DMIC_CH34",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_026_codecs[] = {
	{
		.name = "snd-soc-dummy",
		.dai_name = "snd-soc-dummy-dai",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_026_platforms[] = {
	{
		0,
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_027_cpus[] = {
	{
		.dai_name = "I2S3",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_027_codecs[] = {
	{
		.dai_name = "mt6660-aif",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_027_platforms[] = {
	{
		0,
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_028_cpus[] = {
	{
		.dai_name = "I2S0",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_028_codecs[] = {
	{
		.dai_name = "mt6660-aif",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_028_platforms[] = {
	{
		0,
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_029_cpus[] = {
	{
		.dai_name = "External Codec DL",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_029_codecs[] = {
	{
		.name = "snd-soc-dummy",
		.dai_name = "snd-soc-dummy-dai",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_029_platforms[] = {
	{
		0,
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_030_cpus[] = {
	{
		.dai_name = "External Codec UL",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_030_codecs[] = {
	{
		.name = "snd-soc-dummy",
		.dai_name = "snd-soc-dummy-dai",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_030_platforms[] = {
	{
		0,
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_031_cpus[] = {
	{
		.dai_name = "I2S0",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_031_codecs[] = {
	{
		.name = "nau88c22.0-001a",
		.dai_name = "nau88c22-hifi",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_031_platforms[] = {
	{
		0,
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_032_cpus[] = {
	{
		.dai_name = "I2S1",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_032_codecs[] = {
	{
		.name = "snd-soc-dummy",
		.dai_name = "snd-soc-dummy-dai",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_032_platforms[] = {
	{
		0,
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_033_cpus[] = {
	{
		.dai_name = "I2S2",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_033_codecs[] = {
	{
		.name = "snd-soc-dummy",
		.dai_name = "snd-soc-dummy-dai",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_033_platforms[] = {
	{
		0,
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_034_cpus[] = {
	{
		.dai_name = "I2S3",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_034_codecs[] = {
	{
		.name = "nau88c22.0-001a",
		.dai_name = "nau88c22-hifi",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_034_platforms[] = {
	{
		0,
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_035_cpus[] = {
	{
		.dai_name = "I2S4",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_035_codecs[] = {
	{
		.name = "snd-soc-dummy",
		.dai_name = "snd-soc-dummy-dai",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_035_platforms[] = {
	{
		0,
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_036_cpus[] = {
	{
		.dai_name = "I2S5",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_036_codecs[] = {
	{
		.name = "snd-soc-dummy",
		.dai_name = "snd-soc-dummy-dai",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_036_platforms[] = {
	{
		0,
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_037_cpus[] = {
	{
		.dai_name = "I2S6",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_037_codecs[] = {
	{
		.name = "snd-soc-dummy",
		.dai_name = "snd-soc-dummy-dai",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_037_platforms[] = {
	{
		0,
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_038_cpus[] = {
	{
		.dai_name = "HW Gain 1",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_038_codecs[] = {
	{
		.name = "snd-soc-dummy",
		.dai_name = "snd-soc-dummy-dai",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_038_platforms[] = {
	{
		0,
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_039_cpus[] = {
	{
		.dai_name = "HW Gain 2",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_039_codecs[] = {
	{
		.name = "snd-soc-dummy",
		.dai_name = "snd-soc-dummy-dai",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_039_platforms[] = {
	{
		0,
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_040_cpus[] = {
	{
		.dai_name = "HW_SRC_1",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_040_codecs[] = {
	{
		.name = "snd-soc-dummy",
		.dai_name = "snd-soc-dummy-dai",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_040_platforms[] = {
	{
		0,
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_041_cpus[] = {
	{
		.dai_name = "HW_SRC_2",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_041_codecs[] = {
	{
		.name = "snd-soc-dummy",
		.dai_name = "snd-soc-dummy-dai",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_041_platforms[] = {
	{
		0,
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_042_cpus[] = {
	{
		.dai_name = "CONNSYS_I2S",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_042_codecs[] = {
	{
		.name = "snd-soc-dummy",
		.dai_name = "snd-soc-dummy-dai",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_042_platforms[] = {
	{
		0,
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_043_cpus[] = {
	{
		.dai_name = "PCM 1",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_043_codecs[] = {
	{
		.name = "snd-soc-dummy",
		.dai_name = "snd-soc-dummy-dai",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_043_platforms[] = {
	{
		0,
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_044_cpus[] = {
	{
		.dai_name = "PCM 2",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_044_codecs[] = {
	{
		.name = "snd-soc-dummy",
		.dai_name = "snd-soc-dummy-dai",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_044_platforms[] = {
	{
		0,
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_045_cpus[] = {
	{
		.dai_name = "TDM",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_045_codecs[] = {
	{
		.name = "snd-soc-dummy",
		.dai_name = "snd-soc-dummy-dai",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_045_platforms[] = {
	{
		0,
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_046_cpus[] = {
	{
		.dai_name = "ETDM",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_046_codecs[] = {
	{
		.name = "spi3.0",
		.dai_name = "proslic_spi-aif",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_046_platforms[] = {
	{
		0,
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_047_cpus[] = {
	{
		.dai_name = "Hostless_UL1 DAI",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_047_codecs[] = {
	{
		.name = "snd-soc-dummy",
		.dai_name = "snd-soc-dummy-dai",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_047_platforms[] = {
	{
		0,
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_048_cpus[] = {
	{
		.dai_name = "Hostless_UL2 DAI",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_048_codecs[] = {
	{
		.name = "snd-soc-dummy",
		.dai_name = "snd-soc-dummy-dai",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_048_platforms[] = {
	{
		0,
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_049_cpus[] = {
	{
		.dai_name = "Hostless_UL3 DAI",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_049_codecs[] = {
	{
		.name = "snd-soc-dummy",
		.dai_name = "snd-soc-dummy-dai",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_049_platforms[] = {
	{
		0,
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_050_cpus[] = {
	{
		.dai_name = "Hostless_UL6 DAI",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_050_codecs[] = {
	{
		.name = "snd-soc-dummy",
		.dai_name = "snd-soc-dummy-dai",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_050_platforms[] = {
	{
		0,
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_051_cpus[] = {
	{
		.dai_name = "snd-soc-dummy-dai",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_051_codecs[] = {
	{
		.name = "snd-soc-dummy",
		.dai_name = "snd-soc-dummy-dai",
	},
};

static struct snd_soc_dai_link_component doty_mt6880_mt6359_dai_links_051_platforms[] = {
	{
		.name = "18050000.mtk-btcvsd-snd",
	},
};
/* DOTY-BULK-DAILINK-LINUX54:mt6880_mt6359_dai_links:END */

static struct snd_soc_dai_link mt6880_mt6359_dai_links[] = {
	/* Front End DAI links */
	{
	.cpus = doty_mt6880_mt6359_dai_links_000_cpus,
	.num_cpus = 1,
	.codecs = doty_mt6880_mt6359_dai_links_000_codecs,
	.num_codecs = 1,
	.platforms = doty_mt6880_mt6359_dai_links_000_platforms,
	.num_platforms = 1,
		.name = "Playback_1",
		.stream_name = "Playback_1",
		                      
		                              
		                                      
		.trigger = {SND_SOC_DPCM_TRIGGER_PRE,
			    SND_SOC_DPCM_TRIGGER_PRE},
		.dynamic = 1,
		.dpcm_playback = 1,
	},
	{
	.cpus = doty_mt6880_mt6359_dai_links_001_cpus,
	.num_cpus = 1,
	.codecs = doty_mt6880_mt6359_dai_links_001_codecs,
	.num_codecs = 1,
	.platforms = doty_mt6880_mt6359_dai_links_001_platforms,
	.num_platforms = 1,
		.name = "Playback_12",
		.stream_name = "Playback_12",
		                       
		                              
		                                      
		.trigger = {SND_SOC_DPCM_TRIGGER_PRE,
			    SND_SOC_DPCM_TRIGGER_PRE},
		.dynamic = 1,
		.dpcm_playback = 1,
	},
	{
	.cpus = doty_mt6880_mt6359_dai_links_002_cpus,
	.num_cpus = 1,
	.codecs = doty_mt6880_mt6359_dai_links_002_codecs,
	.num_codecs = 1,
	.platforms = doty_mt6880_mt6359_dai_links_002_platforms,
	.num_platforms = 1,
		.name = "Playback_2",
		.stream_name = "Playback_2",
		                      
		                              
		                                      
		.trigger = {SND_SOC_DPCM_TRIGGER_PRE,
			    SND_SOC_DPCM_TRIGGER_PRE},
		.dynamic = 1,
		.dpcm_playback = 1,
	},
	{
	.cpus = doty_mt6880_mt6359_dai_links_003_cpus,
	.num_cpus = 1,
	.codecs = doty_mt6880_mt6359_dai_links_003_codecs,
	.num_codecs = 1,
	.platforms = doty_mt6880_mt6359_dai_links_003_platforms,
	.num_platforms = 1,
		.name = "Playback_3",
		.stream_name = "Playback_3",
		                      
		                              
		                                      
		.trigger = {SND_SOC_DPCM_TRIGGER_PRE,
			    SND_SOC_DPCM_TRIGGER_PRE},
		.dynamic = 1,
		.dpcm_playback = 1,
	},
	{
	.cpus = doty_mt6880_mt6359_dai_links_004_cpus,
	.num_cpus = 1,
	.codecs = doty_mt6880_mt6359_dai_links_004_codecs,
	.num_codecs = 1,
	.platforms = doty_mt6880_mt6359_dai_links_004_platforms,
	.num_platforms = 1,
		.name = "Playback_4",
		.stream_name = "Playback_4",
		                      
		                              
		                                      
		.trigger = {SND_SOC_DPCM_TRIGGER_PRE,
			    SND_SOC_DPCM_TRIGGER_PRE},
		.dynamic = 1,
		.dpcm_playback = 1,
	},
	{
	.cpus = doty_mt6880_mt6359_dai_links_005_cpus,
	.num_cpus = 1,
	.codecs = doty_mt6880_mt6359_dai_links_005_codecs,
	.num_codecs = 1,
	.platforms = doty_mt6880_mt6359_dai_links_005_platforms,
	.num_platforms = 1,
		.name = "Capture_1",
		.stream_name = "Capture_1",
		                      
		                              
		                                      
		.trigger = {SND_SOC_DPCM_TRIGGER_PRE,
			    SND_SOC_DPCM_TRIGGER_PRE},
		.dynamic = 1,
		.dpcm_capture = 1,
	},
	{
	.cpus = doty_mt6880_mt6359_dai_links_006_cpus,
	.num_cpus = 1,
	.codecs = doty_mt6880_mt6359_dai_links_006_codecs,
	.num_codecs = 1,
	.platforms = doty_mt6880_mt6359_dai_links_006_platforms,
	.num_platforms = 1,
		.name = "Capture_2",
		.stream_name = "Capture_2",
		                      
		                              
		                                      
		.trigger = {SND_SOC_DPCM_TRIGGER_PRE,
			    SND_SOC_DPCM_TRIGGER_PRE},
		.dynamic = 1,
		.dpcm_capture = 1,
	},
	{
	.cpus = doty_mt6880_mt6359_dai_links_007_cpus,
	.num_cpus = 1,
	.codecs = doty_mt6880_mt6359_dai_links_007_codecs,
	.num_codecs = 1,
	.platforms = doty_mt6880_mt6359_dai_links_007_platforms,
	.num_platforms = 1,
		.name = "Capture_3",
		.stream_name = "Capture_3",
		                      
		                              
		                                      
		.trigger = {SND_SOC_DPCM_TRIGGER_PRE,
			    SND_SOC_DPCM_TRIGGER_PRE},
		.dynamic = 1,
		.dpcm_capture = 1,
	},
	{
	.cpus = doty_mt6880_mt6359_dai_links_008_cpus,
	.num_cpus = 1,
	.codecs = doty_mt6880_mt6359_dai_links_008_codecs,
	.num_codecs = 1,
	.platforms = doty_mt6880_mt6359_dai_links_008_platforms,
	.num_platforms = 1,
		.name = "Capture_4",
		.stream_name = "Capture_4",
		                      
		                              
		                                      
		.trigger = {SND_SOC_DPCM_TRIGGER_PRE,
			    SND_SOC_DPCM_TRIGGER_PRE},
		.dynamic = 1,
		.dpcm_capture = 1,
	},
	{
	.cpus = doty_mt6880_mt6359_dai_links_009_cpus,
	.num_cpus = 1,
	.codecs = doty_mt6880_mt6359_dai_links_009_codecs,
	.num_codecs = 1,
	.platforms = doty_mt6880_mt6359_dai_links_009_platforms,
	.num_platforms = 1,
		.name = "Capture_5",
		.stream_name = "Capture_5",
		                      
		                              
		                                      
		.trigger = {SND_SOC_DPCM_TRIGGER_PRE,
			    SND_SOC_DPCM_TRIGGER_PRE},
		.dynamic = 1,
		.dpcm_capture = 1,
	},
	{
	.cpus = doty_mt6880_mt6359_dai_links_010_cpus,
	.num_cpus = 1,
	.codecs = doty_mt6880_mt6359_dai_links_010_codecs,
	.num_codecs = 1,
	.platforms = doty_mt6880_mt6359_dai_links_010_platforms,
	.num_platforms = 1,
		.name = "Capture_6",
		.stream_name = "Capture_6",
		                      
		                              
		                                      
		.trigger = {SND_SOC_DPCM_TRIGGER_PRE,
			    SND_SOC_DPCM_TRIGGER_PRE},
		.dynamic = 1,
		.dpcm_capture = 1,
	},
	{
	.cpus = doty_mt6880_mt6359_dai_links_011_cpus,
	.num_cpus = 1,
	.codecs = doty_mt6880_mt6359_dai_links_011_codecs,
	.num_codecs = 1,
	.platforms = doty_mt6880_mt6359_dai_links_011_platforms,
	.num_platforms = 1,
		.name = "Capture_7",
		.stream_name = "Capture_7",
		                      
		                              
		                                      
		.trigger = {SND_SOC_DPCM_TRIGGER_PRE,
			    SND_SOC_DPCM_TRIGGER_PRE},
		.dynamic = 1,
		.dpcm_capture = 1,
	},
	{
	.cpus = doty_mt6880_mt6359_dai_links_012_cpus,
	.num_cpus = 1,
	.codecs = doty_mt6880_mt6359_dai_links_012_codecs,
	.num_codecs = 1,
	.platforms = doty_mt6880_mt6359_dai_links_012_platforms,
	.num_platforms = 1,
		.name = "Capture_Mono_1",
		.stream_name = "Capture_Mono_1",
		                            
		                              
		                                      
		.trigger = {SND_SOC_DPCM_TRIGGER_PRE,
			    SND_SOC_DPCM_TRIGGER_PRE},
		.dynamic = 1,
		.dpcm_capture = 1,
	},
	{
	.cpus = doty_mt6880_mt6359_dai_links_013_cpus,
	.num_cpus = 1,
	.codecs = doty_mt6880_mt6359_dai_links_013_codecs,
	.num_codecs = 1,
	.platforms = doty_mt6880_mt6359_dai_links_013_platforms,
	.num_platforms = 1,
		.name = "Capture_Mono_2",
		.stream_name = "Capture_Mono_2",
		                            
		                              
		                                      
		.trigger = {SND_SOC_DPCM_TRIGGER_PRE,
			    SND_SOC_DPCM_TRIGGER_PRE},
		.dynamic = 1,
		.dpcm_capture = 1,
	},
	{
	.cpus = doty_mt6880_mt6359_dai_links_014_cpus,
	.num_cpus = 1,
	.codecs = doty_mt6880_mt6359_dai_links_014_codecs,
	.num_codecs = 1,
	.platforms = doty_mt6880_mt6359_dai_links_014_platforms,
	.num_platforms = 1,
		.name = "Capture_Mono_3",
		.stream_name = "Capture_Mono_3",
		                            
		                              
		                                      
		.trigger = {SND_SOC_DPCM_TRIGGER_PRE,
			    SND_SOC_DPCM_TRIGGER_PRE},
		.dynamic = 1,
		.dpcm_capture = 1,
	},
	{
	.cpus = doty_mt6880_mt6359_dai_links_015_cpus,
	.num_cpus = 1,
	.codecs = doty_mt6880_mt6359_dai_links_015_codecs,
	.num_codecs = 1,
	.platforms = doty_mt6880_mt6359_dai_links_015_platforms,
	.num_platforms = 1,
		.name = "Playback_HDMI",
		.stream_name = "Playback_HDMI",
		                       
		                              
		                                      
		.trigger = {SND_SOC_DPCM_TRIGGER_PRE,
			    SND_SOC_DPCM_TRIGGER_PRE},
		.dynamic = 1,
		.dpcm_playback = 1,
	},
	{
	.cpus = doty_mt6880_mt6359_dai_links_016_cpus,
	.num_cpus = 1,
	.codecs = doty_mt6880_mt6359_dai_links_016_codecs,
	.num_codecs = 1,
	.platforms = doty_mt6880_mt6359_dai_links_016_platforms,
	.num_platforms = 1,
		.name = "Hostless_LPBK",
		.stream_name = "Hostless_LPBK",
		                                    
		                              
		                                      
		.trigger = {SND_SOC_DPCM_TRIGGER_PRE,
			    SND_SOC_DPCM_TRIGGER_PRE},
		.dynamic = 1,
		.dpcm_playback = 1,
		.dpcm_capture = 1,
		.ignore_suspend = 1,
	},
	{
	.cpus = doty_mt6880_mt6359_dai_links_017_cpus,
	.num_cpus = 1,
	.codecs = doty_mt6880_mt6359_dai_links_017_codecs,
	.num_codecs = 1,
	.platforms = doty_mt6880_mt6359_dai_links_017_platforms,
	.num_platforms = 1,
		.name = "Hostless_FM",
		.stream_name = "Hostless_FM",
		                                  
		                              
		                                      
		.trigger = {SND_SOC_DPCM_TRIGGER_PRE,
			    SND_SOC_DPCM_TRIGGER_PRE},
		.dynamic = 1,
		.dpcm_playback = 1,
		.dpcm_capture = 1,
		.ignore_suspend = 1,
	},
	{
	.cpus = doty_mt6880_mt6359_dai_links_018_cpus,
	.num_cpus = 1,
	.codecs = doty_mt6880_mt6359_dai_links_018_codecs,
	.num_codecs = 1,
	.platforms = doty_mt6880_mt6359_dai_links_018_platforms,
	.num_platforms = 1,
		.name = "Hostless_Speech",
		.stream_name = "Hostless_Speech",
		                                      
		                              
		                                      
		.trigger = {SND_SOC_DPCM_TRIGGER_PRE,
			    SND_SOC_DPCM_TRIGGER_PRE},
		.dynamic = 1,
		.dpcm_playback = 1,
		.dpcm_capture = 1,
		.ignore_suspend = 1,
	},
	{
	.cpus = doty_mt6880_mt6359_dai_links_019_cpus,
	.num_cpus = 1,
	.codecs = doty_mt6880_mt6359_dai_links_019_codecs,
	.num_codecs = 1,
	.platforms = doty_mt6880_mt6359_dai_links_019_platforms,
	.num_platforms = 1,
		.name = "Hostless_Sph_Echo_Ref",
		.stream_name = "Hostless_Sph_Echo_Ref",
		                                            
		                              
		                                      
		.trigger = {SND_SOC_DPCM_TRIGGER_PRE,
			    SND_SOC_DPCM_TRIGGER_PRE},
		.dynamic = 1,
		.dpcm_playback = 1,
		.dpcm_capture = 1,
		.ignore_suspend = 1,
	},
	{
	.cpus = doty_mt6880_mt6359_dai_links_020_cpus,
	.num_cpus = 1,
	.codecs = doty_mt6880_mt6359_dai_links_020_codecs,
	.num_codecs = 1,
	.platforms = doty_mt6880_mt6359_dai_links_020_platforms,
	.num_platforms = 1,
		.name = "Hostless_Spk_Init",
		.stream_name = "Hostless_Spk_Init",
		                                        
		                              
		                                      
		.trigger = {SND_SOC_DPCM_TRIGGER_PRE,
			    SND_SOC_DPCM_TRIGGER_PRE},
		.dynamic = 1,
		.dpcm_playback = 1,
		.dpcm_capture = 1,
		.ignore_suspend = 1,
	},
	{
	.cpus = doty_mt6880_mt6359_dai_links_021_cpus,
	.num_cpus = 1,
	.codecs = doty_mt6880_mt6359_dai_links_021_codecs,
	.num_codecs = 1,
	.platforms = doty_mt6880_mt6359_dai_links_021_platforms,
	.num_platforms = 1,
		.name = "Hostless_ADDA_DL_I2S_OUT",
		.stream_name = "Hostless_ADDA_DL_I2S_OUT",
		                                               
		                              
		                                      
		.trigger = {SND_SOC_DPCM_TRIGGER_PRE,
			    SND_SOC_DPCM_TRIGGER_PRE},
		.dynamic = 1,
		.dpcm_playback = 1,
		.ignore_suspend = 1,
	},
	{
	.cpus = doty_mt6880_mt6359_dai_links_022_cpus,
	.num_cpus = 1,
	.codecs = doty_mt6880_mt6359_dai_links_022_codecs,
	.num_codecs = 1,
	.platforms = doty_mt6880_mt6359_dai_links_022_platforms,
	.num_platforms = 1,
		.name = "Hostless_SRC_1",
		.stream_name = "Hostless_SRC_1",
		                                     
		                              
		                                      
		.trigger = {SND_SOC_DPCM_TRIGGER_PRE,
			    SND_SOC_DPCM_TRIGGER_PRE},
		.dynamic = 1,
		.dpcm_playback = 1,
		.dpcm_capture = 1,
		.ignore_suspend = 1,
	},
	/* Back End DAI links */
#if 0
	{
	.cpus = doty_mt6880_mt6359_dai_links_023_cpus,
	.num_cpus = 1,
	.codecs = doty_mt6880_mt6359_dai_links_023_codecs,
	.num_codecs = 1,
	.platforms = doty_mt6880_mt6359_dai_links_023_platforms,
	.num_platforms = 1,
		.name = "Primary Codec",
		                       
		                                          
		.no_pcm = 1,
		.dpcm_playback = 1,
		.dpcm_capture = 1,
		.ignore_suspend = 1,
		.init = mt6880_mt6359_init,
	},
	{
	.cpus = doty_mt6880_mt6359_dai_links_024_cpus,
	.num_cpus = 1,
	.codecs = doty_mt6880_mt6359_dai_links_024_codecs,
	.num_codecs = 1,
	.platforms = doty_mt6880_mt6359_dai_links_024_platforms,
	.num_platforms = 1,
		.name = "Primary Codec CH34",
		                            
		                                          
		.no_pcm = 1,
		.dpcm_playback = 1,
		.dpcm_capture = 1,
		.ignore_suspend = 1,
	},
#endif
	{
	.cpus = doty_mt6880_mt6359_dai_links_025_cpus,
	.num_cpus = 1,
	.codecs = doty_mt6880_mt6359_dai_links_025_codecs,
	.num_codecs = 1,
	.platforms = doty_mt6880_mt6359_dai_links_025_platforms,
	.num_platforms = 1,
		.name = "AP_DMIC",
		                          
		                              
		                                      
		.no_pcm = 1,
		.dpcm_capture = 1,
		.ignore_suspend = 1,
	},
	{
	.cpus = doty_mt6880_mt6359_dai_links_026_cpus,
	.num_cpus = 1,
	.codecs = doty_mt6880_mt6359_dai_links_026_codecs,
	.num_codecs = 1,
	.platforms = doty_mt6880_mt6359_dai_links_026_platforms,
	.num_platforms = 1,
		.name = "AP_DMIC_CH34",
		                               
		                              
		                                      
		.no_pcm = 1,
		.dpcm_capture = 1,
		.ignore_suspend = 1,
	},
			
#if 0
	{
	.cpus = doty_mt6880_mt6359_dai_links_027_cpus,
	.num_cpus = 1,
	.codecs = doty_mt6880_mt6359_dai_links_027_codecs,
	.num_codecs = 1,
	.platforms = doty_mt6880_mt6359_dai_links_027_platforms,
	.num_platforms = 1,
		.name = "Speaker Codec",
		                       
		                               
		.dai_fmt = SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_CBS_CFS
			| SND_SOC_DAIFMT_GATED,
		.ops = &mt6880_mt6359_i2s_ops,
		.no_pcm = 1,
		.dpcm_playback = 1,
		.ignore_suspend = 1,
		.ignore_pmdown_time = 1,
		.be_hw_params_fixup = mt6880_i2s_hw_params_fixup,
	},
	{
	.cpus = doty_mt6880_mt6359_dai_links_028_cpus,
	.num_cpus = 1,
	.codecs = doty_mt6880_mt6359_dai_links_028_codecs,
	.num_codecs = 1,
	.platforms = doty_mt6880_mt6359_dai_links_028_platforms,
	.num_platforms = 1,
		.name = "Speaker Codec Ref",
		                       
		                               
		.dai_fmt = SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_CBS_CFS
			| SND_SOC_DAIFMT_GATED,
		.ops = &mt6880_mt6359_i2s_ops,
		.no_pcm = 1,
		.dpcm_capture = 1,
		.ignore_suspend = 1,
		.ignore_pmdown_time = 1,
		.be_hw_params_fixup = mt6880_i2s_hw_params_fixup,
	},
	{
	.cpus = doty_mt6880_mt6359_dai_links_029_cpus,
	.num_cpus = 1,
	.codecs = doty_mt6880_mt6359_dai_links_029_codecs,
	.num_codecs = 1,
	.platforms = doty_mt6880_mt6359_dai_links_029_platforms,
	.num_platforms = 1,
		.name = "External Codec DL",
		                                    
		                              
		                                      
		.no_pcm = 1,
		.dpcm_playback = 1,
		.ignore_suspend = 1,
		.ignore_pmdown_time = 1,
	},
	{
	.cpus = doty_mt6880_mt6359_dai_links_030_cpus,
	.num_cpus = 1,
	.codecs = doty_mt6880_mt6359_dai_links_030_codecs,
	.num_codecs = 1,
	.platforms = doty_mt6880_mt6359_dai_links_030_platforms,
	.num_platforms = 1,
		.name = "External Codec UL",
		                                    
		                              
		                                      
		.no_pcm = 1,
		.dpcm_capture = 1,
		.ignore_suspend = 1,
		.ignore_pmdown_time = 1,
	},
#endif
	{
	.cpus = doty_mt6880_mt6359_dai_links_031_cpus,
	.num_cpus = 1,
	.codecs = doty_mt6880_mt6359_dai_links_031_codecs,
	.num_codecs = 1,
	.platforms = doty_mt6880_mt6359_dai_links_031_platforms,
	.num_platforms = 1,
		.name = "I2S0",
		                       
		                                  
		                                
		.no_pcm = 1,
		.dpcm_capture = 1,
		.ignore_suspend = 1,
		.dai_fmt = SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_CBS_CFS
			| SND_SOC_DAIFMT_GATED,
		//.be_hw_params_fixup = mt6880_i2s_hw_params_fixup,
		.ops = &mt6880_mt6359_i2s_ops,
	},
	{
	.cpus = doty_mt6880_mt6359_dai_links_032_cpus,
	.num_cpus = 1,
	.codecs = doty_mt6880_mt6359_dai_links_032_codecs,
	.num_codecs = 1,
	.platforms = doty_mt6880_mt6359_dai_links_032_platforms,
	.num_platforms = 1,
		.name = "I2S1",
		                       
		                                      
		                              
		.no_pcm = 1,
		.dpcm_playback = 1,
		.ignore_suspend = 1,
		//.be_hw_params_fixup = mt6880_i2s_hw_params_fixup,
	},
	{
	.cpus = doty_mt6880_mt6359_dai_links_033_cpus,
	.num_cpus = 1,
	.codecs = doty_mt6880_mt6359_dai_links_033_codecs,
	.num_codecs = 1,
	.platforms = doty_mt6880_mt6359_dai_links_033_platforms,
	.num_platforms = 1,
		.name = "I2S2",
		                       
		                                      
		                              
		.no_pcm = 1,
		.dpcm_capture = 1,
		.ignore_suspend = 1,
		//.be_hw_params_fixup = mt6880_i2s_hw_params_fixup,
	},
	{
	.cpus = doty_mt6880_mt6359_dai_links_034_cpus,
	.num_cpus = 1,
	.codecs = doty_mt6880_mt6359_dai_links_034_codecs,
	.num_codecs = 1,
	.platforms = doty_mt6880_mt6359_dai_links_034_platforms,
	.num_platforms = 1,
		.name = "I2S3",
		                       
		                                  
		                                
		.no_pcm = 1,
		.dpcm_playback = 1,
		.ignore_suspend = 1,
		.dai_fmt = SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_CBS_CFS
			| SND_SOC_DAIFMT_GATED,
		//.be_hw_params_fixup = mt6880_i2s_hw_params_fixup,
		.ops = &mt6880_mt6359_i2s_ops,
	},
	{
	.cpus = doty_mt6880_mt6359_dai_links_035_cpus,
	.num_cpus = 1,
	.codecs = doty_mt6880_mt6359_dai_links_035_codecs,
	.num_codecs = 1,
	.platforms = doty_mt6880_mt6359_dai_links_035_platforms,
	.num_platforms = 1,
		.name = "I2S4",
		                       
		                                      
		                              
		.no_pcm = 1,
		.dpcm_capture = 1,
		.ignore_suspend = 1,
	},
	{
	.cpus = doty_mt6880_mt6359_dai_links_036_cpus,
	.num_cpus = 1,
	.codecs = doty_mt6880_mt6359_dai_links_036_codecs,
	.num_codecs = 1,
	.platforms = doty_mt6880_mt6359_dai_links_036_platforms,
	.num_platforms = 1,
		.name = "I2S5",
		                       
		                                      
		                              
		.no_pcm = 1,
		.dpcm_playback = 1,
		.ignore_suspend = 1,
	},	
	{
	.cpus = doty_mt6880_mt6359_dai_links_037_cpus,
	.num_cpus = 1,
	.codecs = doty_mt6880_mt6359_dai_links_037_codecs,
	.num_codecs = 1,
	.platforms = doty_mt6880_mt6359_dai_links_037_platforms,
	.num_platforms = 1,
		.name = "I2S6",
		                       
		                                      
		                              
		.no_pcm = 1,
		.dpcm_capture = 1,
		.ignore_suspend = 1,
	},
	{
	.cpus = doty_mt6880_mt6359_dai_links_038_cpus,
	.num_cpus = 1,
	.codecs = doty_mt6880_mt6359_dai_links_038_codecs,
	.num_codecs = 1,
	.platforms = doty_mt6880_mt6359_dai_links_038_platforms,
	.num_platforms = 1,
		.name = "HW Gain 1",
		                            
		                              
		                                      
		.no_pcm = 1,
		.dpcm_playback = 1,
		.dpcm_capture = 1,
		.ignore_suspend = 1,
	},
	{
	.cpus = doty_mt6880_mt6359_dai_links_039_cpus,
	.num_cpus = 1,
	.codecs = doty_mt6880_mt6359_dai_links_039_codecs,
	.num_codecs = 1,
	.platforms = doty_mt6880_mt6359_dai_links_039_platforms,
	.num_platforms = 1,
		.name = "HW Gain 2",
		                            
		                              
		                                      
		.no_pcm = 1,
		.dpcm_playback = 1,
		.dpcm_capture = 1,
		.ignore_suspend = 1,
	},
	{
	.cpus = doty_mt6880_mt6359_dai_links_040_cpus,
	.num_cpus = 1,
	.codecs = doty_mt6880_mt6359_dai_links_040_codecs,
	.num_codecs = 1,
	.platforms = doty_mt6880_mt6359_dai_links_040_platforms,
	.num_platforms = 1,
		.name = "HW_SRC_1",
		                           
		                              
		                                      
		.no_pcm = 1,
		.dpcm_playback = 1,
		.dpcm_capture = 1,
		.ignore_suspend = 1,
	},
	{
	.cpus = doty_mt6880_mt6359_dai_links_041_cpus,
	.num_cpus = 1,
	.codecs = doty_mt6880_mt6359_dai_links_041_codecs,
	.num_codecs = 1,
	.platforms = doty_mt6880_mt6359_dai_links_041_platforms,
	.num_platforms = 1,
		.name = "HW_SRC_2",
		                           
		                              
		                                      
		.no_pcm = 1,
		.dpcm_playback = 1,
		.dpcm_capture = 1,
		.ignore_suspend = 1,
	},
	{
	.cpus = doty_mt6880_mt6359_dai_links_042_cpus,
	.num_cpus = 1,
	.codecs = doty_mt6880_mt6359_dai_links_042_codecs,
	.num_codecs = 1,
	.platforms = doty_mt6880_mt6359_dai_links_042_platforms,
	.num_platforms = 1,
		.name = "CONNSYS_I2S",
		                              
		                              
		                                      
		.no_pcm = 1,
		.dpcm_capture = 1,
		.ignore_suspend = 1,
	},
	{
	.cpus = doty_mt6880_mt6359_dai_links_043_cpus,
	.num_cpus = 1,
	.codecs = doty_mt6880_mt6359_dai_links_043_codecs,
	.num_codecs = 1,
	.platforms = doty_mt6880_mt6359_dai_links_043_platforms,
	.num_platforms = 1,
		.name = "PCM 1",
		                        
		                              
		                                      
		.no_pcm = 1,
		.dpcm_playback = 1,
		.dpcm_capture = 1,
		.ignore_suspend = 1,
	},
	{
	.cpus = doty_mt6880_mt6359_dai_links_044_cpus,
	.num_cpus = 1,
	.codecs = doty_mt6880_mt6359_dai_links_044_codecs,
	.num_codecs = 1,
	.platforms = doty_mt6880_mt6359_dai_links_044_platforms,
	.num_platforms = 1,
		.name = "PCM 2",
		                        
		                              
		                                      
		.no_pcm = 1,
		.dpcm_playback = 1,
		.dpcm_capture = 1,
		.ignore_suspend = 1,
	},
	{
	.cpus = doty_mt6880_mt6359_dai_links_045_cpus,
	.num_cpus = 1,
	.codecs = doty_mt6880_mt6359_dai_links_045_codecs,
	.num_codecs = 1,
	.platforms = doty_mt6880_mt6359_dai_links_045_platforms,
	.num_platforms = 1,
		.name = "TDM",
		                      
		                              
		                                      
		.no_pcm = 1,
		.dpcm_playback = 1,
		.ignore_suspend = 1,
	},
	{
	.cpus = doty_mt6880_mt6359_dai_links_046_cpus,
	.num_cpus = 1,
	.codecs = doty_mt6880_mt6359_dai_links_046_codecs,
	.num_codecs = 1,
	.platforms = doty_mt6880_mt6359_dai_links_046_platforms,
	.num_platforms = 1,
		.name = "ETDM",
		                       
		                       
		                                    
		.no_pcm = 1,
		.dpcm_playback = 1,
		.dpcm_capture = 1,
		.ignore_suspend = 1,
	},

	/* dummy BE for ul memif to record from dl memif */
	{
	.cpus = doty_mt6880_mt6359_dai_links_047_cpus,
	.num_cpus = 1,
	.codecs = doty_mt6880_mt6359_dai_links_047_codecs,
	.num_codecs = 1,
	.platforms = doty_mt6880_mt6359_dai_links_047_platforms,
	.num_platforms = 1,
		.name = "Hostless_UL1",
		                                   
		                                      
		                              
		.no_pcm = 1,
		.dpcm_capture = 1,
		.ignore_suspend = 1,
	},
	{
	.cpus = doty_mt6880_mt6359_dai_links_048_cpus,
	.num_cpus = 1,
	.codecs = doty_mt6880_mt6359_dai_links_048_codecs,
	.num_codecs = 1,
	.platforms = doty_mt6880_mt6359_dai_links_048_platforms,
	.num_platforms = 1,
		.name = "Hostless_UL2",
		                                   
		                                      
		                              
		.no_pcm = 1,
		.dpcm_capture = 1,
		.ignore_suspend = 1,
	},
	{
	.cpus = doty_mt6880_mt6359_dai_links_049_cpus,
	.num_cpus = 1,
	.codecs = doty_mt6880_mt6359_dai_links_049_codecs,
	.num_codecs = 1,
	.platforms = doty_mt6880_mt6359_dai_links_049_platforms,
	.num_platforms = 1,
		.name = "Hostless_UL3",
		                                   
		                                      
		                              
		.no_pcm = 1,
		.dpcm_capture = 1,
		.ignore_suspend = 1,
	},
	{
	.cpus = doty_mt6880_mt6359_dai_links_050_cpus,
	.num_cpus = 1,
	.codecs = doty_mt6880_mt6359_dai_links_050_codecs,
	.num_codecs = 1,
	.platforms = doty_mt6880_mt6359_dai_links_050_platforms,
	.num_platforms = 1,
		.name = "Hostless_UL6",
		                                   
		                                      
		                              
		.no_pcm = 1,
		.dpcm_capture = 1,
		.ignore_suspend = 1,
	},
	/* BTCVSD */
#ifdef CONFIG_SND_SOC_MTK_BTCVSD
	{
	.cpus = doty_mt6880_mt6359_dai_links_051_cpus,
	.num_cpus = 1,
	.codecs = doty_mt6880_mt6359_dai_links_051_codecs,
	.num_codecs = 1,
	.platforms = doty_mt6880_mt6359_dai_links_051_platforms,
	.num_platforms = 1,
		.name = "BTCVSD",
		.stream_name = "BTCVSD",
		                                      
		                                            
		                                      
		                              
	},
#endif
};
/*end: modify by laizhenhao for I2S*/

static struct snd_soc_card mt6880_mt6359_soc_card = {
	.name = "mt6880-mt6359",
	.owner = THIS_MODULE,
	.dai_link = mt6880_mt6359_dai_links,
	.num_links = ARRAY_SIZE(mt6880_mt6359_dai_links),

	.controls = mt6880_mt6359_controls,
	.num_controls = ARRAY_SIZE(mt6880_mt6359_controls),
	.dapm_widgets = mt6880_mt6359_widgets,
	.num_dapm_widgets = ARRAY_SIZE(mt6880_mt6359_widgets),
	.dapm_routes = mt6880_mt6359_routes,
	.num_dapm_routes = ARRAY_SIZE(mt6880_mt6359_routes),
};

static int mt6880_mt6359_dev_probe(struct platform_device *pdev)
{
	struct snd_soc_card *card = &mt6880_mt6359_soc_card;
	struct device_node *platform_node;
#if 0
	struct device_node *codec_node, *spk_codec_node;
	struct device_node *spk_codec_node;
#endif
	struct device_node *ext_codec_node;

	int ret;
	int i;

	dev_info(&pdev->dev, "%s(), start\n", __func__);

/*begin: modify audio codec function,by laizhenhao,2021.05.31*/
#if 1
	ret = mtk_spk_update_dai_link(card, pdev, &mt6880_mt6359_i2s_ops);
		if (ret) {
			dev_err(&pdev->dev, "%s(), mtk_spk_update_dai_link error\n",
				__func__);
			return -EINVAL;
	}
#endif
/*end: modify audio codec function,by laizhenhao,2021.05.31*/
	platform_node = of_parse_phandle(pdev->dev.of_node,
					 "mediatek,platform", 0);
	if (!platform_node) {
		dev_err(&pdev->dev, "Property 'platform' missing or invalid\n");
		return -EINVAL;
	}

	for (i = 0; i < card->num_links; i++) {
		if (mt6880_mt6359_dai_links[i].platforms[0].name)
			continue;
		mt6880_mt6359_dai_links[i].platforms[0].of_node = platform_node;
	}

	card->dev = &pdev->dev;
	ext_codec_node = of_parse_phandle(pdev->dev.of_node,
				      "mediatek,ext-codec", 0);
	if (!ext_codec_node) {
		dev_err(&pdev->dev,
			"Property 'ext-codec' missing or invalid\n");
		return -EINVAL;
	}
#if 0
	codec_node = of_parse_phandle(pdev->dev.of_node,
				      "mediatek,audio-codec", 0);
	if (!codec_node) {
		dev_err(&pdev->dev,
			"Property 'audio-codec' missing or invalid\n");
		return -EINVAL;
	}

	spk_codec_node = of_parse_phandle(pdev->dev.of_node,
					  "mediatek,speaker-codec", 0);
	if (!spk_codec_node) {
		dev_err(&pdev->dev,
			"Property 'speaker-codec' missing or invalid\n");
		return -EINVAL;
	}
	
#endif

	for (i = 0; i < card->num_links; i++) {
		if (mt6880_mt6359_dai_links[i].codecs[0].name)
			continue;

		if (!strcmp(mt6880_mt6359_dai_links[i].name, "ETDM"))
			mt6880_mt6359_dai_links[i].codecs[0].of_node =
				ext_codec_node;
	}



	ret = devm_snd_soc_register_card(&pdev->dev, card);
	if (ret)
		dev_err(&pdev->dev, "%s snd_soc_register_card fail %d\n",
			__func__, ret);

	dev_info(&pdev->dev, "%s(), ret %d\n", __func__, ret);
	return ret;
}

#ifdef CONFIG_OF
static const struct of_device_id mt6880_mt6359_dt_match[] = {
	{.compatible = "mediatek,mt6880-mt6359-sound",},
	{}
};
#endif

static const struct dev_pm_ops mt6880_mt6359_pm_ops = {
	.poweroff = snd_soc_poweroff,
	.restore = snd_soc_resume,
};

static struct platform_driver mt6880_mt6359_driver = {
	.driver = {
		.name = "mt6880-mt6359",
#ifdef CONFIG_OF
		.of_match_table = mt6880_mt6359_dt_match,
#endif
		.pm = &mt6880_mt6359_pm_ops,
	},
	.probe = mt6880_mt6359_dev_probe,
};

module_platform_driver(mt6880_mt6359_driver);

/* Module information */
MODULE_DESCRIPTION("MT6880 MT6359 ALSA SoC machine driver");
MODULE_AUTHOR("Eason Yen <eason.yen@mediatek.com>");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("mt6880 mt6359 soc card");
