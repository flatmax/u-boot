
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

/*
 * NOTE: Those i2c_* functions only use the i2c_ao bus and require the i2c
 *       interface to be already configured from Linux.
 */
static void i2c_write(int slave_addr, int reg, int val)
{
	int i;

	writel(0x00, AO_I2C_M_0_TOKEN_LIST0);
	writel(0x00, AO_I2C_M_0_TOKEN_LIST1);

	writel((slave_addr & 0x3F) << 1, AO_I2C_M_0_SLAVE_ADDR);

	// 0x1 - start
	// 0x2 - slave addr write
	// 0x4 - data
	// 0x4 - data
	// 0x5 - data last
	writel(0x54421, AO_I2C_M_0_TOKEN_LIST0);

	writel((reg & 0xFF) | ((val & 0xFF) << 8), AO_I2C_M_0_WDATA_REG0);

	/* start xfer */
	writel(readl(AO_I2C_M_0_CONTROL_REG) & ~(1), AO_I2C_M_0_CONTROL_REG);
	writel(readl(AO_I2C_M_0_CONTROL_REG) | 1, AO_I2C_M_0_CONTROL_REG);


	for (i = 0; i < 1000000; i++) {
		_udelay(5);
		if (!(readl(AO_I2C_M_0_CONTROL_REG) & (1 << 2)))
			break;
	}

	writel(0x00, AO_I2C_M_0_TOKEN_LIST0);
	writel(0x00, AO_I2C_M_0_TOKEN_LIST1);
}

static int i2c_read(int slave_addr, int reg)
{
	int i;
	int val;

	writel(0x00, AO_I2C_M_0_TOKEN_LIST0);
	writel(0x00, AO_I2C_M_0_TOKEN_LIST1);

	writel((slave_addr & 0x3F) << 1, AO_I2C_M_0_SLAVE_ADDR);

	// 0x1 - start
	// 0x2 - slave addr write
	// 0x4 - data
	writel(0x421, AO_I2C_M_0_TOKEN_LIST0);

	// 0x12
	writel((reg & 0xFF), AO_I2C_M_0_WDATA_REG0);

	/* start xfer */
	writel(readl(AO_I2C_M_0_CONTROL_REG) & ~(1), AO_I2C_M_0_CONTROL_REG);
	writel(readl(AO_I2C_M_0_CONTROL_REG) | 1, AO_I2C_M_0_CONTROL_REG);


	for (i = 0; i < 1000000; i++) {
		_udelay(5);
		if (!(readl(AO_I2C_M_0_CONTROL_REG) & (1 << 2)))
			break;
	}

	writel(0x00, AO_I2C_M_0_TOKEN_LIST0);
	writel(0x00, AO_I2C_M_0_TOKEN_LIST1);



	// 0x1 - start
	// 0x3 - slave addr read
	// 0x5 - data last
	writel(0x531, AO_I2C_M_0_TOKEN_LIST0);

	/* start xfer */
	writel(readl(AO_I2C_M_0_CONTROL_REG) & ~(1), AO_I2C_M_0_CONTROL_REG);
	writel(readl(AO_I2C_M_0_CONTROL_REG) | 1, AO_I2C_M_0_CONTROL_REG);

	for (i = 0; i < 1000000; i++) {
		_udelay(5);
		if (!(readl(AO_I2C_M_0_CONTROL_REG) & (1 << 2)))
			break;
	}

	val = readl(AO_I2C_M_0_RDATA_REG0) & 0xFF;

	_udelay(5);
	writel(0x00, AO_I2C_M_0_TOKEN_LIST0);
	writel(0x00, AO_I2C_M_0_TOKEN_LIST1);

	return val;
}

#define AXP152_VDD_825MV	0x05

int old_output_ctrl;
int old_vddee_setting;

static void power_off_at_24M(unsigned int suspend_from)
{
	int reg;

	old_output_ctrl = i2c_read(0x32, 0x12);
	reg = old_output_ctrl;

#if 0
	/* Turning this supply off will hang the kernel on resuming */
	/* disable DCDC1 (VDDIO_1V8) */
	reg &= ~(1 << 7);
#endif

	/* disable DCDC2 (VDDCPU) */
	reg &= ~(1 << 6);

	/* disable ALDO1 (VDDIO_3V3) */
	reg &= ~(1 << 3);

	i2c_write(0x32, 0x12, reg);

	uart_puts("output_ctrl: old: 0x");
	uart_put_hex(old_output_ctrl, 8);
	uart_puts(", new: 0x");
	uart_put_hex(reg, 8);
	uart_puts("\n");



#if 0
	/* TODO: we cannot disable VDD_EE, instead we should lower it at least */
	/* disable DCDC4 (VDD_EE) */
	reg &= ~(1 << 4);
#endif

	/* backup old VDD_EE setting */
	old_vddee_setting = i2c_read(0x32, 0x2B);

	/* set VDD_EE to 825mV */
	reg = AXP152_VDD_825MV;
	i2c_write(0x32, 0x2B, reg);

	uart_puts("vdd_ee: old: 0x");
	uart_put_hex(old_vddee_setting, 8);
	uart_puts(", new: 0x");
	uart_put_hex(reg, 8);
	uart_puts("\n");


	_udelay(1000);
}

static void power_on_at_24M(unsigned int suspend_from)
{
	uart_puts("restoring vddee to: 0x");
	uart_put_hex(old_vddee_setting, 8);
	uart_puts("\n");
	i2c_write(0x32, 0x2B, old_vddee_setting);

	uart_puts("restoring output_ctrl to: 0x");
	uart_put_hex(old_output_ctrl, 8);
	uart_puts("\n");
	i2c_write(0x32, 0x12, old_output_ctrl);

	_udelay(1000);
}

void get_wakeup_source(void *response, unsigned int suspend_from)
{
	struct wakeup_info *p = (struct wakeup_info *)response;
	unsigned val;

	p->status = RESPONSE_OK;
	val = (POWER_KEY_WAKEUP_SRC | AUTO_WAKEUP_SRC | ETH_PHY_WAKEUP_SRC | BT_WAKEUP_SRC);

	p->sources = val;
	p->gpio_info_count = 0;
}

extern void __switch_idle_task(void);

static unsigned int detect_key(unsigned int suspend_from)
{
	int exit_reason = 0;
	unsigned *irq = (unsigned *)WAKEUP_SRC_IRQ_ADDR_BASE;

	/* Set GPPIOAO_3 and GPIOAO_5 to input */
	writel(readl(AO_GPIO_O_EN_N) | (1 << 3) | (1 << 5), AO_GPIO_O_EN_N);

	/* Set GPIOAO_3 and GPIOAO_5 mux as GPIO */
	writel(readl(AO_RTI_PINMUX_REG0) & (~(0xf << 12) & ~(0xf << 20)), AO_RTI_PINMUX_REG0);

	/* Enable pull-up for GPIOAO_5 */
	writel(readl(PAD_PULL_UP_REG0) | (1 << 5), PAD_PULL_UP_REG0);
	writel(readl(AO_RTI_PULL_UP_REG) | (1 << 5), AO_RTI_PULL_UP_REG);

	do {
		if (irq[IRQ_ETH_PHY] == IRQ_ETH_PHY_NUM) {
			irq[IRQ_ETH_PHY] = 0xFFFFFFFF;
			uart_puts("ETH_PHY wakeup\n");
			exit_reason = ETH_PHY_WAKEUP;
		}

		if (irq[IRQ_VRTC] == IRQ_VRTC_NUM) {
			irq[IRQ_VRTC] = 0xFFFFFFFF;
			uart_puts("RTC wakeup\n");
			exit_reason = RTC_WAKEUP;
		}

		if (irq[IRQ_AO_TIMERA] == IRQ_AO_TIMERA_NUM) {
			unsigned int gpio_ao;
			irq[IRQ_AO_TIMERA] = 0xFFFFFFFF;

			gpio_ao = readl(AO_GPIO_I);

			if (gpio_ao & (1 << 3)) {
				uart_puts("NPB_IN wakeup\n");
				exit_reason = POWER_KEY_WAKEUP;
			} else if (gpio_ao & (1 << 5)) {
#if 0
				/* For now the WIFI_W2H wakeup is disabled in software */
				uart_puts("WIFI_W2H wakeup\n");
				exit_reason = BT_WAKEUP;
#endif
			}
		}

		if (exit_reason)
			break;
		else
			__switch_idle_task();
	} while (1);

	return exit_reason;
}

static void pwr_op_init(struct pwr_op *pwr_op)
{
	pwr_op->power_off_at_24M = power_off_at_24M;
	pwr_op->power_on_at_24M = power_on_at_24M;
	pwr_op->detect_key = detect_key;
	pwr_op->get_wakeup_source = get_wakeup_source;
}
