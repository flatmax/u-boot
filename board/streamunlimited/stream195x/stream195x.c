/*
 * (C) Copyright 2019 StreamUnlimited Engineering GmbH
 * Martin Pietryka <martin.pietryka@streamunlimited.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <malloc.h>
#include <errno.h>
#include <asm/io.h>
#include <miiphy.h>
#include <asm-generic/gpio.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-imx/gpio.h>
#include <asm/mach-imx/hab.h>
#include <asm/arch/clock.h>
#include <power/axp15060_pmic.h>
#include <usb.h>
#include <dm.h>

#include "../common/fwupdate.h"
#include "../common/device_interface.h"

static struct sue_device_info __attribute__((section (".data"))) current_device;

DECLARE_GLOBAL_DATA_PTR;

int board_early_init_f(void)
{
	/* TODO: verify that the WDOG is working in the U-BOOT */
	struct wdog_regs *wdog = (struct wdog_regs *)WDOG1_BASE_ADDR;
	set_wdog_reset(wdog);

	return 0;
}

int dram_init(void)
{
	/* rom_pointer[1] contains the size of TEE occupies */
	if (rom_pointer[1])
		gd->ram_size = PHYS_SDRAM_SIZE - rom_pointer[1];
	else
		gd->ram_size = PHYS_SDRAM_SIZE;

	return 0;
}

#ifdef CONFIG_FEC_MXC

#if defined(CONFIG_TARGET_STREAM195X_STREAMKIT)
#define FEC_RST_PAD IMX_GPIO_NR(1, 0)
#define FEC_PDOWN_PAD IMX_GPIO_NR(1, 9)
#else
#define FEC_RST_PAD IMX_GPIO_NR(1, 29)
#endif

static int setup_fec(void)
{
	int ret;
	struct iomuxc_gpr_base_regs *const iomuxc_gpr_regs
		= (struct iomuxc_gpr_base_regs *) IOMUXC_GPR_BASE_ADDR;

#if defined(CONFIG_TARGET_STREAM195X_STREAMKIT)
	gpio_request(FEC_PDOWN_PAD, "fec1_pdown");
	gpio_direction_output(FEC_PDOWN_PAD, 0);
	udelay(1000);
	gpio_direction_output(FEC_PDOWN_PAD, 1);
	udelay(1000);
#endif

	/* Reset any external ethernet PHY */
	gpio_request(FEC_RST_PAD, "fec1_rst");
	gpio_direction_output(FEC_RST_PAD, 0);
	udelay(1000);
	gpio_direction_output(FEC_RST_PAD, 1);
	udelay(1000);

	/* Use the internally generated RMII clock */
	setbits_le32(&iomuxc_gpr_regs->gpr[1],
			(1 << IOMUXC_GPR_GPR1_GPR_ENET1_TX_CLK_SEL_SHIFT));

#if defined(CONFIG_TARGET_STREAM195X_STREAMKIT)
	ret = set_clk_enet(ENET_125MHZ);
#else
	ret = set_clk_enet(ENET_50MHZ);
#endif
	return ret;
}

int board_phy_config(struct phy_device *phydev)
{
	if (phydev->drv->config)
		phydev->drv->config(phydev);

	return 0;
}
#endif


int board_ehci_usb_phy_mode(struct udevice *dev)
{
	/*
	 * We only allow device mode (for fastboot) on the first port
	 * when the boot was done over USB. In all other cases we do
	 * not want to enabled device mode.
	 */
	if (is_boot_from_usb() && dev->seq == 0)
		return USB_INIT_DEVICE;

	return USB_INIT_HOST;
}

#if defined(CONFIG_TARGET_STREAM195X_STREAMKIT)
#define USB1_DRVVBUS_GPIO IMX_GPIO_NR(1, 12)
#define USB2_DRVVBUS_GPIO IMX_GPIO_NR(4, 10)
#else
#define USB1_DRVVBUS_GPIO IMX_GPIO_NR(1, 10)
#define USB2_DRVVBUS_GPIO IMX_GPIO_NR(1, 11)
#endif

static int setup_usb(void)
{
	gpio_request(USB1_DRVVBUS_GPIO, "usb1_drvvbus");
	gpio_direction_output(USB1_DRVVBUS_GPIO, 0);

	gpio_request(USB2_DRVVBUS_GPIO, "usb2_drvvbus");
	gpio_direction_output(USB2_DRVVBUS_GPIO, 0);
	return 0;
}

int board_usb_init(int index, enum usb_init_type init)
{
	int ret = 0;

	imx8m_usb_power(index, true);

	/* Set DRVVBUS for the respective port to HIGH */
	if (init == USB_INIT_HOST) {
		if (index == 0)
			gpio_set_value(USB1_DRVVBUS_GPIO, 1);
		else
			gpio_set_value(USB2_DRVVBUS_GPIO, 1);
	}

	return ret;
}

int board_usb_cleanup(int index, enum usb_init_type init)
{
	int ret = 0;

	/* Set DRVVBUS for the respective port to LOW */
	if (init == USB_INIT_HOST) {
		if (index == 0)
			gpio_set_value(USB1_DRVVBUS_GPIO, 0);
		else
			gpio_set_value(USB2_DRVVBUS_GPIO, 0);
	}

	imx8m_usb_power(index, false);
	return ret;
}

int power_init_board(void)
{
	struct udevice *dev;
	int ret;
	uint val;

	ret = pmic_get("axp15060", &dev);
	if (ret)
		return ret;

	/* Enable restarting the PMIC by when PWROK pin is driven low */
	ret = pmic_clrsetbits(dev, AXP15060_PWR_DIS_SEQ, 0, AXP15060_PWR_DIS_PWROK_RESTART_EN);
	if (ret)
		return ret;

	/* Enable 85% low voltage power off for all DCDC rails */
	val = AXP15060_OUT_MON_CTRL_DCDC_EN(1) | AXP15060_OUT_MON_CTRL_DCDC_EN(2) |
		AXP15060_OUT_MON_CTRL_DCDC_EN(3) | AXP15060_OUT_MON_CTRL_DCDC_EN(4) |
		AXP15060_OUT_MON_CTRL_DCDC_EN(5) | AXP15060_OUT_MON_CTRL_DCDC_EN(6);
	ret = pmic_clrsetbits(dev, AXP15060_OUT_MON_CTRL, 0, val);
	if (ret)
		return ret;

	/*
	 * Set DCDC6 (3V3_OUT) from 3.1 V to 3.3 V
	 * min: 0.5 V, 100 mV/step => (3.3 V - 0.5 V) / 0,1 V = 28
	 */
	ret = pmic_reg_write(dev, AXP15060_DCDC6_V_CTRL, 28);
	if (ret)
		return ret;

	return 0;
}

#define WIFI_REG_EN_GPIO IMX_GPIO_NR(2, 8)

int board_init(void)
{
	int ret;

	setup_usb();

	ret = power_init_board();
	if (ret)
		printf("power_init_board() failed\n");

	/*
	 * Perform a reset of the WiFi chip and de-assert the reset line.
	 * This is required, because without the reset, after a reboot, the
	 * SDIO device will not be found on the bus leading to the `bcmdhd`
	 * driver not probing.
	 */
	gpio_request(WIFI_REG_EN_GPIO, "wifi_reg");
	gpio_direction_output(WIFI_REG_EN_GPIO, 0);
	udelay(1000);
	gpio_set_value(WIFI_REG_EN_GPIO, 1);

#ifdef CONFIG_FEC_MXC
	setup_fec();
#endif

	sue_device_detect(&current_device);
	/* current_device.reset_cause = reset_cause(); */
	/* handle_reset_cause(current_device.reset_cause); */
	sue_carrier_ops_init(&current_device);
	sue_print_device_info(&current_device);


	sue_carrier_init(&current_device);

	return 0;
}

int board_late_init(void)
{
	char buffer[64];


	if (fwupdate_init(&current_device) < 0) {
		printf("ERROR: fwupdate_init() call failed!\n");
	}


	if (current_device.carrier_flags & SUE_CARRIER_FLAGS_HAS_DAUGHTER) {
		snprintf(buffer, sizeof(buffer), "%s_l%d_%s_%s",
				sue_device_get_canonical_module_name(&current_device),
				current_device.module_version,
				sue_device_get_canonical_carrier_name(&current_device),
				sue_device_get_canonical_daughter_name(&current_device));
	} else {
		snprintf(buffer, sizeof(buffer), "%s_l%d_%s",
				sue_device_get_canonical_module_name(&current_device),
				current_device.module_version,
				sue_device_get_canonical_carrier_name(&current_device));
	}
	printf("Setting fit_config: %s\n", buffer);
	env_set("fit_config", buffer);

	if (imx_hab_is_enabled()) {
		printf("Secure boot is enabled, setting secure_board to 1\n");
		env_set_ulong("secure_board", 1);
	} else {
		printf("Secure boot is disabled, setting secure_board to 0\n");
		env_set_ulong("secure_board", 0);
	}

	sue_carrier_late_init(&current_device);

	return 0;
}

#if defined(CONFIG_FASTBOOT)
int is_fastboot_allowed(void)
{
	if (imx_hab_is_enabled()) {
		printf("Secure boot is enabled, disallowing fastboot\n");
		return 0;
	} else {
		printf("Secure boot is disabled, allowing fastboot\n");
		return 1;
	}
}
#endif