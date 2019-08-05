/*
 * (C) Copyright 2019 StreamUnlimited Engineering GmbH
 * Martin Pietryka <martin.pietryka@streamunlimited.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __AXP15060_PMIC_H__
#define __AXP15060_PMIC_H__

#include <power/pmic.h>

enum axp15060_reg {
	AXP15060_DATA_BUFFER0	= 0x04,
	AXP15060_DATA_BUFFER1	= 0x05,
	AXP15060_DATA_BUFFER2	= 0x06,
	AXP15060_DATA_BUFFER3	= 0x07,
	AXP15060_PWR_CTRL2	= 0x11,
	AXP15060_ADLO1_V_CTRL	= 0x19,
	AXP15060_OUT_MON_CTRL	= 0x1E,
	AXP15060_ADLO2_V_CTRL	= 0x20,
	AXP15060_ADLO3_V_CTRL	= 0x21,
	AXP15060_ADLO4_V_CTRL	= 0x22,
	AXP15060_ADLO5_V_CTRL	= 0x23,
	AXP15060_PWR_DIS_SEQ	= 0x32,
	AXP15060_NUM_OF_REGS,
};

#define AXP15060_DATA_BUFFER(__x) (AXP15060_DATA_BUFFER0 + (__x))

#define AXP15060_PWR_DIS_PWROK_RESTART_EN	(1 << 4)

#define AXP15060_OUT_MON_CTRL_DCDC_EN(__x)	(1 << (__x))

#endif /* __AXP15060_PMIC_H__ */
