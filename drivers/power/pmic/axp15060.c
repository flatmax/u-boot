/*
 * (C) Copyright 2019 StreamUnlimited Engineering GmbH
 * Martin Pietryka <martin.pietryka@streamunlimited.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <power/pmic.h>
#include <power/axp15060_pmic.h>

DECLARE_GLOBAL_DATA_PTR;

static int axp15060_reg_count(struct udevice *dev)
{
	return AXP15060_NUM_OF_REGS;
}

static int axp15060_write(struct udevice *dev, uint reg, const uint8_t *buff, int len)
{
	if (dm_i2c_write(dev, reg, buff, len)) {
		pr_err("write error to device: %p register: %#x!", dev, reg);
		return -EIO;
	}

	return 0;
}

static int axp15060_read(struct udevice *dev, uint reg, uint8_t *buff, int len)
{
	if (dm_i2c_read(dev, reg, buff, len)) {
		pr_err("read error from device: %p register: %#x!", dev, reg);
		return -EIO;
	}

	return 0;
}

static struct dm_pmic_ops axp15060_ops = {
	.reg_count = axp15060_reg_count,
	.read = axp15060_read,
	.write = axp15060_write,
};

static const struct udevice_id axp15060_ids[] = {
	{ .compatible = "x-powers,axp15060" },
	{ }
};

U_BOOT_DRIVER(pmic_axp15060) = {
	.name = "axp15060_pmic",
	.id = UCLASS_PMIC,
	.of_match = axp15060_ids,
	.ops = &axp15060_ops,
};
