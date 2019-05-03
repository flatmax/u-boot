/*
 * (C) Copyright 2019 StreamUnlimited Engineering GmbH
 * Martin Pietryka <martin.pietryka@streamunlimited.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <spl.h>
#include <asm/io.h>
#include <errno.h>
#include <asm/io.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/arch/imx8mm_pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/clock.h>
#include <asm/mach-imx/gpio.h>
#include <asm/arch/imx8m_ddr.h>
#include <asm-generic/gpio.h>
#include <usb.h>

DECLARE_GLOBAL_DATA_PTR;

#define UART_PAD_CTRL	(PAD_CTL_DSE6 | PAD_CTL_FSEL1)
static iomux_v3_cfg_t const uart_pads[] = {
	IMX8MM_PAD_UART1_RXD_UART1_RX | MUX_PAD_CTRL(UART_PAD_CTRL),
	IMX8MM_PAD_UART1_TXD_UART1_TX | MUX_PAD_CTRL(UART_PAD_CTRL),
};

#ifdef CONFIG_NAND_MXS
#define NAND_PAD_CTRL	(PAD_CTL_DSE6 | PAD_CTL_FSEL2 | PAD_CTL_HYS)
#define NAND_PAD_READY0_CTRL (PAD_CTL_DSE6 | PAD_CTL_FSEL2 | PAD_CTL_PUE)
static iomux_v3_cfg_t const gpmi_pads[] = {
	IMX8MM_PAD_NAND_ALE_RAWNAND_ALE | MUX_PAD_CTRL(NAND_PAD_CTRL),
	IMX8MM_PAD_NAND_CE0_B_RAWNAND_CE0_B | MUX_PAD_CTRL(NAND_PAD_CTRL),
	IMX8MM_PAD_NAND_CLE_RAWNAND_CLE | MUX_PAD_CTRL(NAND_PAD_CTRL),
	IMX8MM_PAD_NAND_DATA00_RAWNAND_DATA00 | MUX_PAD_CTRL(NAND_PAD_CTRL),
	IMX8MM_PAD_NAND_DATA01_RAWNAND_DATA01 | MUX_PAD_CTRL(NAND_PAD_CTRL),
	IMX8MM_PAD_NAND_DATA02_RAWNAND_DATA02 | MUX_PAD_CTRL(NAND_PAD_CTRL),
	IMX8MM_PAD_NAND_DATA03_RAWNAND_DATA03 | MUX_PAD_CTRL(NAND_PAD_CTRL),
	IMX8MM_PAD_NAND_DATA04_RAWNAND_DATA04 | MUX_PAD_CTRL(NAND_PAD_CTRL),
	IMX8MM_PAD_NAND_DATA05_RAWNAND_DATA05	| MUX_PAD_CTRL(NAND_PAD_CTRL),
	IMX8MM_PAD_NAND_DATA06_RAWNAND_DATA06	| MUX_PAD_CTRL(NAND_PAD_CTRL),
	IMX8MM_PAD_NAND_DATA07_RAWNAND_DATA07	| MUX_PAD_CTRL(NAND_PAD_CTRL),
	IMX8MM_PAD_NAND_DQS_RAWNAND_DQS | MUX_PAD_CTRL(NAND_PAD_CTRL),
	IMX8MM_PAD_NAND_RE_B_RAWNAND_RE_B | MUX_PAD_CTRL(NAND_PAD_CTRL),
	IMX8MM_PAD_NAND_READY_B_RAWNAND_READY_B | MUX_PAD_CTRL(NAND_PAD_READY0_CTRL),
	IMX8MM_PAD_NAND_WE_B_RAWNAND_WE_B | MUX_PAD_CTRL(NAND_PAD_CTRL),
	IMX8MM_PAD_NAND_WP_B_RAWNAND_WP_B | MUX_PAD_CTRL(NAND_PAD_CTRL),
};
#endif


#define MODULE_ID0_GPIO IMX_GPIO_NR(3, 2)
static iomux_v3_cfg_t const module_id0_pads[] = {
	IMX8MM_PAD_NAND_CE1_B_GPIO3_IO2 | MUX_PAD_CTRL(NO_PAD_CTRL),
};

extern struct dram_timing_info ddr3l_1x4Gb_dram_timing;
extern struct dram_timing_info ddr3l_2x2Gb_dram_timing;

void spl_dram_init(void)
{
	imx_iomux_v3_setup_multiple_pads(module_id0_pads, ARRAY_SIZE(module_id0_pads));
	gpio_request(MODULE_ID0_GPIO, "module_id0");
	gpio_direction_input(MODULE_ID0_GPIO);

	if (gpio_get_value(MODULE_ID0_GPIO)) {
		printf("calling ddr_init() with ddr3l_1x4Gb_dram_timing\n");
		ddr_init(&ddr3l_1x4Gb_dram_timing);
	} else {
		printf("calling ddr_init() with ddr3l_2x2Gb_dram_timing\n");
		ddr_init(&ddr3l_2x2Gb_dram_timing);
	}
}

void spl_board_init(void)
{
	/* Currently does nothing */
}

#ifdef CONFIG_SPL_LOAD_FIT
int board_fit_config_name_match(const char *name)
{
	/*
	 * Since we do not care about mutliple U-Boots inside
	 * the fitImage, we just always return a match (0).
	 */
	return 0;
}
#endif

int board_usb_phy_mode(int port)
{
	/*
	 * When this function is called in the SPL we are booting
	 * over USB so the U-Boot will be loaded over USB as well,
	 * thus we return that we are in device mode in any case.
	 */
	return USB_INIT_DEVICE;
}

void board_boot_order(u32 *spl_boot_list)
{
	spl_boot_list[0] = spl_boot_device();
}

void board_init_f(ulong dummy)
{
	int ret;

	/* Clear global data */
	memset((void *)gd, 0, sizeof(gd_t));

	arch_cpu_init();

#ifdef CONFIG_NAND_MXS
	imx_iomux_v3_setup_multiple_pads(gpmi_pads, ARRAY_SIZE(gpmi_pads));
	init_nand_clk();
#endif

	timer_init();

	imx_iomux_v3_setup_multiple_pads(uart_pads, ARRAY_SIZE(uart_pads));
	preloader_console_init();

	ret = spl_init();
	if (ret) {
		debug("spl_init() failed: %d\n", ret);
		hang();
	}

	enable_tzc380();

	/* DDR initialization */
	spl_dram_init();
}
