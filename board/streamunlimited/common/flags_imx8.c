/*
 * flags_imx8.c
 *
 * Copyright (C) 2019, StreamUnlimited Engineering GmbH, http://www.streamunlimited.com/
 * Martin Pietryka <martin.pietryka@streamunlimited.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR /PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <common.h>
#include <asm/io.h>
#include <dm.h>
#include <power/axp15060_pmic.h>

#define FLAGS_IMX8_BUFFER_REG_FLAGS	0
#define FLAGS_IMX8_BUFFER_REG_BOOTCNT	1

/*
 * On the imx8 it does not seem to be straight forward to use the
 * general purpose SNVS registers to preserve some flags across
 * resets. The reason is probably this (from the reference manual):
 * "This is a privileged read/write register"
 * So probably these registers can only be directly accessed from
 * the ARM trusted firmware.
 *
 * To workaround this, we can use two of the 4 data buffer registers
 * of the AXP15060 which are intended to cover this use case.
 */
static int axp15060_read_buffer(int buffnr, u8 *val)
{
	struct udevice *dev;
	int ret;

	ret = pmic_get("axp15060", &dev);
	if (ret)
		return ret;

	return pmic_read(dev, AXP15060_DATA_BUFFER(buffnr), val, 1);
}

static int axp15060_write_buffer(int buffnr, u8 val)
{
	struct udevice *dev;
	int ret;

	ret = pmic_get("axp15060", &dev);
	if (ret)
		return ret;

	return pmic_write(dev, AXP15060_DATA_BUFFER(buffnr), &val, 1);
}

int flag_write(u8 index, bool val)
{
	int ret;
	u8 reg;

	if (index > 7)
		return -EINVAL;

	ret = axp15060_read_buffer(FLAGS_IMX8_BUFFER_REG_FLAGS, &reg);
	if (ret)
		return ret;

	reg &= ~(1 << index);
	reg |= (val << index);

	ret = axp15060_write_buffer(FLAGS_IMX8_BUFFER_REG_FLAGS, reg);
	if (ret)
		return ret;


	return 0;
}

int flag_read(u8 index, bool *val)
{
	int ret;
	u8 reg;

	if (index > 7)
		return -EINVAL;

	ret = axp15060_read_buffer(FLAGS_IMX8_BUFFER_REG_FLAGS, &reg);
	if (ret)
		return ret;

	*val = reg & (1 << index);

	return 0;
}

int flags_clear(void)
{
	return axp15060_write_buffer(FLAGS_IMX8_BUFFER_REG_FLAGS, 0);
}

int bootcnt_write(u8 data)
{
	return axp15060_write_buffer(FLAGS_IMX8_BUFFER_REG_BOOTCNT, data);
}

int bootcnt_read(u8 *data)
{
	return axp15060_read_buffer(FLAGS_IMX8_BUFFER_REG_BOOTCNT, data);
}
