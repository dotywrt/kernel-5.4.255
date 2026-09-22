/* SPDX-License-Identifier: GPL-2.0 */
/*
 * DOTY-AW1000-MT-GPIO-LINUX54-COMPAT
 *
 * Minimal legacy MediaTek mt_gpio sysfs ABI for AW1000 Ultra / FG360.
 *
 * The original vendor debug file in this tree was copied from an older
 * pinctrl API and referenced bias_get_combo(), bias_set_combo() and
 * mtk_pctrl_show_one_pin(), which are not present in this Linux-5.4
 * pinctrl implementation.
 *
 * This compatibility layer intentionally implements only the ABI used by
 * the bundled aqrd and ght-slic userspace:
 *
 *   mode, dir, out, driving, smt, ies, set
 *
 * and a vendor-style readable pin table at:
 *
 *   /sys/devices/platform/10005000.pinctrl/mt_gpio
 *
 * Pull-control commands from the old engineering debug interface are not
 * exposed here because current AW1000 userspace does not use them.
 */

#include <linux/module.h>
#include <linux/device.h>
#include <linux/gpio.h>
#include <linux/gpio/consumer.h>
#include <linux/string.h>
#include <linux/kernel.h>

#include "../../gpio/gpiolib.h"
#include "pinctrl-paris.h"

#define MTK_PINCTRL_DEV_NAME "pinctrl_paris"

static const char * const aw1000_pinctrl_name = MTK_PINCTRL_DEV_NAME;
static struct mtk_pinctrl *g_hw;

static void aw1000_find_mtk_pinctrl(void)
{
	struct gpio_desc *gdesc;
	unsigned int pin = ARCH_NR_GPIOS - 1;

	if (g_hw)
		return;

	do {
		gdesc = gpio_to_desc(pin);
		if (gdesc &&
		    !strncmp(aw1000_pinctrl_name,
			     gdesc->gdev->chip->label,
			     strlen(aw1000_pinctrl_name))) {
			g_hw = gpiochip_get_data(gdesc->gdev->chip);
			return;
		}

		if (gdesc)
			pin = (pin + 1) - gdesc->gdev->chip->base;

		if (!gdesc || pin == 0)
			break;
	} while (1);

	pr_notice("aw1000-mt-gpio: cannot find %s gpiochip\n",
		  aw1000_pinctrl_name);
}

static int aw1000_pin_valid(struct mtk_pinctrl *hw, unsigned int gpio)
{
	return hw && gpio < hw->soc->npins;
}

static const struct mtk_pin_desc *
aw1000_pin_desc(struct mtk_pinctrl *hw, unsigned int gpio)
{
	if (!aw1000_pin_valid(hw, gpio))
		return NULL;

	return &hw->soc->pins[gpio];
}

static int aw1000_hw_get(struct mtk_pinctrl *hw, unsigned int gpio,
			 int field, int *value)
{
	const struct mtk_pin_desc *desc = aw1000_pin_desc(hw, gpio);

	if (!desc)
		return -EINVAL;

	return mtk_hw_get_value(hw, desc, field, value);
}

static int aw1000_hw_set(struct mtk_pinctrl *hw, unsigned int gpio,
			 int field, int value)
{
	const struct mtk_pin_desc *desc = aw1000_pin_desc(hw, gpio);

	if (!desc)
		return -EINVAL;

	return mtk_hw_set_value(hw, desc, field, value);
}

static int aw1000_drive_get(struct mtk_pinctrl *hw, unsigned int gpio)
{
	const struct mtk_pin_desc *desc = aw1000_pin_desc(hw, gpio);
	int value = 0;

	if (!desc || !hw->soc->drive_get)
		return 0;

	if (hw->soc->drive_get(hw, desc, &value))
		return 0;

	return value;
}

static int aw1000_drive_set(struct mtk_pinctrl *hw, unsigned int gpio,
			    unsigned int value)
{
	const struct mtk_pin_desc *desc = aw1000_pin_desc(hw, gpio);

	if (!desc || !hw->soc->drive_set)
		return -ENOTSUPP;

	return hw->soc->drive_set(hw, desc, value);
}

static int aw1000_read_or_zero(struct mtk_pinctrl *hw, unsigned int gpio,
			       int field)
{
	int value = 0;

	if (aw1000_hw_get(hw, gpio, field, &value))
		return 0;

	return value;
}

static ssize_t mt_gpio_show(struct device *dev,
			    struct device_attribute *attr, char *buf)
{
	struct mtk_pinctrl *hw = dev_get_drvdata(dev);
	unsigned int i;
	unsigned int len = 0;

	if (!hw || !buf)
		return 0;

	len += scnprintf(buf + len, PAGE_SIZE - len,
		"PIN:(MODE)(DIR)(DOUT)(DIN)(DRV)(SMT)(IES)(PULLEN)(PULLSEL)[R1 R0]\n");

	for (i = 0; i < hw->chip.ngpio; i++) {
		int mode, dir, dout, din, drv, smt, ies;

		if (len > PAGE_SIZE - 64)
			break;

		mode = aw1000_read_or_zero(hw, i, PINCTRL_PIN_REG_MODE);
		dir  = aw1000_read_or_zero(hw, i, PINCTRL_PIN_REG_DIR);
		dout = aw1000_read_or_zero(hw, i, PINCTRL_PIN_REG_DO);
		din  = aw1000_read_or_zero(hw, i, PINCTRL_PIN_REG_DI);
		drv  = aw1000_drive_get(hw, i);
		smt  = aw1000_read_or_zero(hw, i, PINCTRL_PIN_REG_SMT);
		ies  = aw1000_read_or_zero(hw, i, PINCTRL_PIN_REG_IES);

		/*
		 * Preserve the vendor's leading layout:
		 *   "%03d: %1d%1d%1d%1d..."
		 *
		 * aqrd/ght-slic grep this exact line shape and inspect the
		 * early DIR/DOUT fields by fixed byte position.
		 *
		 * Pull fields are printed as zero because the old combo-bias
		 * debug ABI does not exist in this 5.4 pinctrl implementation.
		 */
		len += scnprintf(buf + len, PAGE_SIZE - len,
			"%03u: %1d%1d%1d%1d%02d%1d%1d00 [0 0]\n",
			i, mode, dir, dout, din, drv, smt, ies);
	}

	return len;
}

static int aw1000_parse_gpio_value(const char *buf, size_t prefix,
				   unsigned int *gpio, int *value)
{
	return sscanf(buf + prefix, "%u %d", gpio, value) == 2 ? 0 : -EINVAL;
}

static int aw1000_apply_legacy_set(struct mtk_pinctrl *hw,
				   unsigned int gpio,
				   const char *digits)
{
	int vals[12] = { 0 };
	size_t i, n;
	int ret;

	if (!aw1000_pin_valid(hw, gpio))
		return -EINVAL;

	n = strnlen(digits, 12);
	if (n == 0)
		return -EINVAL;

	for (i = 0; i < n; i++) {
		if (digits[i] < '0' || digits[i] > '9')
			break;
		vals[i] = digits[i] - '0';
	}

	/* MODE */
	ret = aw1000_hw_set(hw, gpio, PINCTRL_PIN_REG_MODE, vals[0]);
	if (ret)
		return ret;

	/* DIR: 0=input, 1=output */
	ret = aw1000_hw_set(hw, gpio, PINCTRL_PIN_REG_DIR, !!vals[1]);
	if (ret)
		return ret;

	/* DOUT is meaningful only in output mode. */
	if (vals[1]) {
		ret = aw1000_hw_set(hw, gpio, PINCTRL_PIN_REG_DO, !!vals[2]);
		if (ret)
			return ret;
	}

	/* Two legacy digits represent drive strength. */
	if (n > 5 && hw->soc->drive_set) {
		unsigned int drv = vals[4] * 10 + vals[5];

		ret = aw1000_drive_set(hw, gpio, drv);
		if (ret && ret != -ENOTSUPP)
			return ret;
	}

	if (n > 6) {
		ret = aw1000_hw_set(hw, gpio, PINCTRL_PIN_REG_SMT, !!vals[6]);
		if (ret)
			return ret;
	}

	if (n > 7) {
		ret = aw1000_hw_set(hw, gpio, PINCTRL_PIN_REG_IES, !!vals[7]);
		if (ret)
			return ret;
	}

	/*
	 * vals[8..11] were legacy pull/bias controls. They are deliberately
	 * ignored: this 5.4 SoC data has no bias_*_combo callbacks and the
	 * AW1000 bundled userspace does not issue standalone pull commands.
	 */
	return 0;
}

static ssize_t mt_gpio_store(struct device *dev,
			     struct device_attribute *attr,
			     const char *buf, size_t count)
{
	struct mtk_pinctrl *hw = dev_get_drvdata(dev);
	unsigned int gpio;
	int value;
	int ret = 0;

	if (!hw)
		return -ENODEV;

	if (!strncmp(buf, "mode", 4)) {
		ret = aw1000_parse_gpio_value(buf, 4, &gpio, &value);
		if (!ret)
			ret = aw1000_hw_set(hw, gpio, PINCTRL_PIN_REG_MODE, value);
	} else if (!strncmp(buf, "dir", 3)) {
		ret = aw1000_parse_gpio_value(buf, 3, &gpio, &value);
		if (!ret)
			ret = aw1000_hw_set(hw, gpio, PINCTRL_PIN_REG_DIR, !!value);
	} else if (!strncmp(buf, "out", 3)) {
		ret = aw1000_parse_gpio_value(buf, 3, &gpio, &value);
		if (!ret)
			ret = aw1000_hw_set(hw, gpio, PINCTRL_PIN_REG_DIR, 1);
		if (!ret)
			ret = aw1000_hw_set(hw, gpio, PINCTRL_PIN_REG_DO, !!value);
	} else if (!strncmp(buf, "driving", 7)) {
		ret = aw1000_parse_gpio_value(buf, 7, &gpio, &value);
		if (!ret)
			ret = aw1000_drive_set(hw, gpio, value);
	} else if (!strncmp(buf, "smt", 3)) {
		ret = aw1000_parse_gpio_value(buf, 3, &gpio, &value);
		if (!ret)
			ret = aw1000_hw_set(hw, gpio, PINCTRL_PIN_REG_SMT, !!value);
	} else if (!strncmp(buf, "ies", 3)) {
		ret = aw1000_parse_gpio_value(buf, 3, &gpio, &value);
		if (!ret)
			ret = aw1000_hw_set(hw, gpio, PINCTRL_PIN_REG_IES, !!value);
	} else if (!strncmp(buf, "set", 3)) {
		char digits[16] = { 0 };

		if (sscanf(buf + 3, "%u %15s", &gpio, digits) != 2)
			ret = -EINVAL;
		else
			ret = aw1000_apply_legacy_set(hw, gpio, digits);
	} else {
		pr_notice("aw1000-mt-gpio: unsupported legacy command: %.*s",
			  (int)min_t(size_t, count, 64), buf);
		/* Preserve old engineering-interface behavior. */
		return count;
	}

	if (ret) {
		pr_notice("aw1000-mt-gpio: command failed (%d): %.*s",
			  ret, (int)min_t(size_t, count, 64), buf);
		return ret;
	}

	return count;
}

static DEVICE_ATTR(mt_gpio, 0644, mt_gpio_show, mt_gpio_store);

static int __init aw1000_mt_gpio_init(void)
{
	int ret;

	aw1000_find_mtk_pinctrl();
	if (!g_hw)
		return -ENODEV;

	ret = device_create_file(g_hw->dev, &dev_attr_mt_gpio);
	if (ret)
		pr_notice("aw1000-mt-gpio: cannot create mt_gpio attribute: %d\n",
			  ret);
	else
		pr_info("aw1000-mt-gpio: legacy mt_gpio ABI enabled\n");

	return ret;
}

late_initcall(aw1000_mt_gpio_init);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("AW1000/FG360 MediaTek legacy mt_gpio compatibility");
