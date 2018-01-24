
/*
 * board/amlogic/txl_skt_v1/firmware/scp_task/pwr_ctrl.c
 *
 * Copyright (C) 2015 Amlogic, Inc. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include <gpio.h>
//#include "pwm_ctrl.h"

#if 0

#define ON 1
#define OFF 0

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

static void power_on_ddr(void);
enum pwm_id {
	pwm_a = 0,
	pwm_b,
	pwm_c,
	pwm_d,
	pwm_e,
	pwm_f,
	pwm_ao_a,
	pwm_ao_b,
};

static void power_switch_to_ee(unsigned int pwr_ctrl)
{
	if (pwr_ctrl == ON) {
		writel(readl(AO_RTI_PWR_CNTL_REG0) | (0x1 << 9), AO_RTI_PWR_CNTL_REG0);
		_udelay(1000);
		writel(readl(AO_RTI_PWR_CNTL_REG0)
			& (~((0x3 << 3) | (0x1 << 1))), AO_RTI_PWR_CNTL_REG0);
	} else {
		writel(readl(AO_RTI_PWR_CNTL_REG0)
		       | ((0x3 << 3) | (0x1 << 1)), AO_RTI_PWR_CNTL_REG0);

		writel(readl(AO_RTI_PWR_CNTL_REG0) & (~(0x1 << 9)),
		       AO_RTI_PWR_CNTL_REG0);

	}
}

static void pwm_set_voltage(unsigned int id, unsigned int voltage)
{
	int to;

	switch (id) {
	case pwm_a:
	for (to = 0; to < ARRAY_SIZE(pwm_voltage_table); to++) {
		if (pwm_voltage_table[to][1] >= voltage) {
			break;
		}
	}
	if (to >= ARRAY_SIZE(pwm_voltage_table)) {
		to = ARRAY_SIZE(pwm_voltage_table) - 1;
	}
		uart_puts("set vcck to 0x");
		uart_put_hex(to, 16);
		uart_puts("mv\n");
		P_PWM_PWM_A = pwm_voltage_table[to][0];
		break;

	case pwm_ao_b:
		for (to = 0; to < ARRAY_SIZE(pwm_voltage_table_ee); to++) {
			if (pwm_voltage_table_ee[to][1] >= voltage) {
				break;
			}
		}
		if (to >= ARRAY_SIZE(pwm_voltage_table_ee)) {
			to = ARRAY_SIZE(pwm_voltage_table_ee) - 1;
		}
		uart_puts("set vddee to 0x");
		uart_put_hex(to, 16);
		uart_puts("mv\n");
		P_AO_PWM_PWM_B1 = pwm_voltage_table_ee[to][0];
		break;
	default:
		break;
	}
	_udelay(200);
}

#endif

static void power_off_at_24M(unsigned int suspend_from)
{
	return;
}
static void power_on_at_24M(unsigned int suspend_from)
{
	return;
}

void get_wakeup_source(void *response, unsigned int suspend_from)
{
	struct wakeup_info *p = (struct wakeup_info *)response;
	unsigned val;

	p->status = RESPONSE_OK;
	val = (POWER_KEY_WAKEUP_SRC | AUTO_WAKEUP_SRC | REMOTE_WAKEUP_SRC |
	       ETH_PHY_WAKEUP_SRC | BT_WAKEUP_SRC);

#ifdef CONFIG_AUDIO_WAKEUP
	if (readl(EE_AUDIO_CLK_PDMIN_CTRL1) & (1<<31))
		val |= AUD_PWR_WAKEUP_SRC;
#endif
	p->gpio_info_count = 0;
	p->sources = val;
}
extern void __switch_idle_task(void);

static unsigned int detect_key(unsigned int suspend_from)
{
	int exit_reason = 0;
	unsigned *irq = (unsigned *)WAKEUP_SRC_IRQ_ADDR_BASE;
	unsigned char adc_key_cnt = 0;
#ifdef CONFIG_AUDIO_WAKEUP
	char aud_pwr_en = 0;
	unsigned skip_cnt = 100;
	/* unsigned th = 0, v = 0; */
#endif

	init_remote();
	saradc_enable();
#ifdef CONFIG_AUDIO_WAKEUP
	aud_pwr_en = enable_pdm();
#endif

	do {
		if (irq[IRQ_AO_IR_DEC] == IRQ_AO_IR_DEC_NUM) {
			irq[IRQ_AO_IR_DEC] = 0xFFFFFFFF;
			if (remote_detect_key())
				exit_reason = REMOTE_WAKEUP;
		}

		if (irq[IRQ_ETH_PHY] == IRQ_ETH_PHY_NUM) {
			irq[IRQ_ETH_PHY] = 0xFFFFFFFF;
			exit_reason = ETH_PHY_WAKEUP;
		}

		if (irq[IRQ_VRTC] == IRQ_VRTC_NUM) {
			irq[IRQ_VRTC] = 0xFFFFFFFF;
			exit_reason = RTC_WAKEUP;
		}

		if (irq[IRQ_AO_TIMERA] == IRQ_AO_TIMERA_NUM) {
			irq[IRQ_AO_TIMERA] = 0xFFFFFFFF;
			if (check_adc_key_resume()) {
				adc_key_cnt++;
				/*using variable 'adc_key_cnt' to eliminate the dithering of the key*/
				if (2 == adc_key_cnt)
					exit_reason = POWER_KEY_WAKEUP;
			} else {
				adc_key_cnt = 0;
			}
		}
#ifdef CONFIG_AUDIO_WAKEUP
#if 1
		if (aud_pwr_en && irq[IRQ_AUDIO_PWR_DEC] == IRQ_AUDIO_PWR_NUM) {
			if (skip_cnt > 0) {
				skip_cnt--;
			} else {
				irq[IRQ_AUDIO_PWR_DEC] = 0xFFFFFFFF;
				exit_reason = RTC_WAKEUP;
			}
		}
#else
		if (aud_pwr_en && skip_cnt == 0) {
			v = readl(EE_AUDIO_POW_DET_VALUE);
			th = readl(EE_AUDIO_POW_DET_TH_HI);
			v = v & ~(1<<31);
			if (v > th) {
				aud_pwr_cnt++;
				if (aud_pwr_cnt > 10) {
					dbg_print("audio pwr val:", v);
					dbg_print("audio pwr th:", th);
					irq[IRQ_AUDIO_PWR_DEC] = 0xFFFFFFFF;
					exit_reason = RTC_WAKEUP;
				}
			} else
				aud_pwr_cnt = 0;
		} else
			skip_cnt--;
#endif
#endif

		if (exit_reason)
			break;
		else
			__switch_idle_task();
	} while (1);

	saradc_disable();

	return exit_reason;
}

static void pwr_op_init(struct pwr_op *pwr_op)
{
	pwr_op->power_off_at_24M = power_off_at_24M;
	pwr_op->power_on_at_24M = power_on_at_24M;
	pwr_op->detect_key = detect_key;
	pwr_op->get_wakeup_source = get_wakeup_source;
}
