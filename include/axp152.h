/*
 * (C) Copyright 2012 Henrik Nordstrom <henrik@henriknordstrom.net>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

enum aldo_voltage {
	AXP152_ALDO_1V2	= 0x0,
	AXP152_ALDO_1V3	= 0x1,
	AXP152_ALDO_1V4	= 0x2,
	AXP152_ALDO_1V5	= 0x3,
	AXP152_ALDO_1V6	= 0x4,
	AXP152_ALDO_1V7	= 0x5,
	AXP152_ALDO_1V8	= 0x6,
	AXP152_ALDO_1V9	= 0x7,
	AXP152_ALDO_2V0	= 0x8,
	AXP152_ALDO_2V5	= 0x9,
	AXP152_ALDO_2V7	= 0xA,
	AXP152_ALDO_2V8	= 0xB,
	AXP152_ALDO_3V0	= 0xC,
	AXP152_ALDO_3V1	= 0xD,
	AXP152_ALDO_3V2	= 0xE,
	AXP152_ALDO_3V3	= 0xF,
};

int axp152_set_dcdc2(int mvolt);
int axp152_set_dcdc3(int mvolt);
int axp152_set_dcdc4(int mvolt);
int axp152_set_ldo2(int mvolt);
int axp152_set_aldo1(enum aldo_voltage aldo_val);
int axp152_set_aldo2(enum aldo_voltage aldo_val);
int axp152_init(void);
