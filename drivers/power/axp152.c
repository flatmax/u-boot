/*
 * (C) Copyright 2012
 * Henrik Nordstrom <henrik@henriknordstrom.net>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <i2c.h>
#include <axp152.h>

enum axp152_reg {
	AXP152_CHIP_VERSION = 0x3,
	AXP152_LDO0_CONTROL = 0x15,
	AXP152_DCDC2_VOLTAGE = 0x23,
	AXP152_DCDC3_VOLTAGE = 0x27,
	AXP152_ALDO_VOLTAGE = 0x28,
	AXP152_DCDC4_VOLTAGE = 0x2B,
	AXP152_LDO2_VOLTAGE = 0x2A,
	AXP152_SHUTDOWN = 0x32,
	AXP152_GPIO2_CONTROL = 0x92,
	AXP152_GPIO2_LDO = 0x96,
};

#define AXP152_POWEROFF			(1 << 7)

static int axp152_write(enum axp152_reg reg, u8 val)
{
	return i2c_write(0x30, reg, 1, &val, 1);
}

static int axp152_read(enum axp152_reg reg, u8 *val)
{
	return i2c_read(0x30, reg, 1, val, 1);
}

static u8 axp152_mvolt_to_target(int mvolt, int min, int max, int div)
{
	if (mvolt < min)
		mvolt = min;
	else if (mvolt > max)
		mvolt = max;

	return (mvolt - min) / div;
}

int axp152_set_gpio2_ldo(int mvolt)
{
	int ret;
	u8 target;

	target = axp152_mvolt_to_target(mvolt, 1800, 3300, 100);

	/* configure GPIO2 as low noise LDO */
	ret = axp152_write(AXP152_GPIO2_CONTROL, 0x02);
	if (ret)
		return ret;

	return axp152_write(AXP152_GPIO2_LDO, target & 0x0F);
}

int axp152_disable_gpio2_ldo(void)
{
	return axp152_write(AXP152_GPIO2_CONTROL, 0x07);
}

int axp152_set_aldo1(enum axp152_aldo_voltage aldo_val)
{
	int ret;
	u8 current;

	ret = axp152_read(AXP152_ALDO_VOLTAGE, &current);
	if (ret)
		return ret;

	current &= 0x0F;
	current |= (aldo_val << 4) & 0xF0;

	return axp152_write(AXP152_ALDO_VOLTAGE, current);
}

int axp152_set_aldo2(enum axp152_aldo_voltage aldo_val)
{
	int ret;
	u8 current;

	ret = axp152_read(AXP152_ALDO_VOLTAGE, &current);
	if (ret)
		return ret;

	current &= 0xF0;
	current |= aldo_val & 0x0F;

	return axp152_write(AXP152_ALDO_VOLTAGE, current);
}

int axp152_set_dcdc2(int mvolt)
{
	int rc;
	u8 current, target;

	target = axp152_mvolt_to_target(mvolt, 700, 2275, 25);

	/* Do we really need to be this gentle? It has built-in voltage slope */
	while ((rc = axp152_read(AXP152_DCDC2_VOLTAGE, &current)) == 0 &&
	       current != target) {
		if (current < target)
			current++;
		else
			current--;
		rc = axp152_write(AXP152_DCDC2_VOLTAGE, current);
		if (rc)
			break;
	}
	return rc;
}

int axp152_set_dcdc3(int mvolt)
{
	u8 target = axp152_mvolt_to_target(mvolt, 700, 3500, 50);

	return axp152_write(AXP152_DCDC3_VOLTAGE, target);
}

int axp152_set_dcdc4(int mvolt)
{
	u8 target = axp152_mvolt_to_target(mvolt, 700, 3500, 25);

	return axp152_write(AXP152_DCDC4_VOLTAGE, target);
}

int axp152_set_ldo0(enum axp152_ldo0_voltage voltage, enum axp152_ldo0_current current_limit)
{
	u8 reg = 0;

	reg = (1 << 7) | (voltage << 4) | (current_limit);

	return axp152_write(AXP152_LDO0_CONTROL, reg);
}

int axp152_disable_ldo0(void)
{
	return axp152_write(AXP152_LDO0_CONTROL, 0x00);
}

int axp152_set_ldo2(int mvolt)
{
	u8 target = axp152_mvolt_to_target(mvolt, 700, 3500, 100);

	return axp152_write(AXP152_LDO2_VOLTAGE, target);
}

int axp152_init(void)
{
	u8 ver;
	int rc;

	rc = axp152_read(AXP152_CHIP_VERSION, &ver);
	if (rc)
		return rc;

	if (ver != 0x05)
		return -1;

	return 0;
}
