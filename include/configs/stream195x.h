/*
 * Copyright 2018 NXP
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __IMX8MM_EVK_H
#define __IMX8MM_EVK_H

#include <linux/sizes.h>
#include <asm/arch/imx-regs.h>

#include "sue_fwupdate_common.h"

#ifdef CONFIG_SECURE_BOOT
#define CONFIG_CSF_SIZE			0x2000 /* 8K region */
#endif

#define CONFIG_SPL_MAX_SIZE		(148 * 1024)
#define CONFIG_SYS_MONITOR_LEN		(512 * 1024)
#define CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_SECTOR
#define CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR	0x300
#define CONFIG_SYS_MMCSD_FS_BOOT_PARTITION	1
#define CONFIG_SYS_UBOOT_BASE		(QSPI0_AMBA_BASE + CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR * 512)

#ifdef CONFIG_SPL_BUILD
/*#define CONFIG_ENABLE_DDR_TRAINING_DEBUG*/
#define CONFIG_SPL_WATCHDOG_SUPPORT
#define CONFIG_SPL_POWER_SUPPORT
#define CONFIG_SPL_DRIVERS_MISC_SUPPORT
#define CONFIG_SPL_LDSCRIPT		"arch/arm/cpu/armv8/u-boot-spl.lds"
#define CONFIG_SPL_STACK		0x91fff0
#define CONFIG_SPL_LIBCOMMON_SUPPORT
#define CONFIG_SPL_LIBGENERIC_SUPPORT
#define CONFIG_SPL_SERIAL_SUPPORT
#define CONFIG_SPL_GPIO_SUPPORT
#define CONFIG_SPL_BSS_START_ADDR      0x00910000
#define CONFIG_SPL_BSS_MAX_SIZE        0x2000	/* 8 KB */
#define CONFIG_SYS_SPL_MALLOC_START    0x42200000
#define CONFIG_SYS_SPL_MALLOC_SIZE     0x80000	/* 512 KB */
#define CONFIG_SYS_ICACHE_OFF
#define CONFIG_SYS_DCACHE_OFF

#define CONFIG_MALLOC_F_ADDR		0x912000 /* malloc f used before GD_FLG_FULL_MALLOC_INIT set */

#define CONFIG_SPL_ABORT_ON_RAW_IMAGE /* For RAW image gives a error info not panic */

#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_MXC_I2C1		/* enable I2C bus 1 */

#if defined(CONFIG_NAND_BOOT)
#define CONFIG_SPL_NAND_MXS
#define CONFIG_SYS_NAND_U_BOOT_OFFS		(4 * SZ_1M)
#define CONFIG_SYS_NAND_U_BOOT_OFFS_REDNUD	(6 * SZ_1M)
#endif

#endif

#define CONFIG_SERIAL_TAG

#define CONFIG_REMAKE_ELF

#define CONFIG_BOARD_LATE_INIT

/* ENET Config */
/* ENET1 */
#if defined(CONFIG_CMD_NET)
#define CONFIG_MII
#define CONFIG_ETHPRIME                 "FEC"

#define CONFIG_FEC_MXC
#define FEC_QUIRK_ENET_MAC
#define CONFIG_FEC_MXC_PHYADDR          0

#define IMX_FEC_BASE			0x30BE0000

#if defined(CONFIG_TARGET_STREAM195X_STREAMKIT)
#define CONFIG_FEC_XCV_TYPE RGMII
#else
#define CONFIG_FEC_XCV_TYPE RMII
#endif

#endif

/* Initial environment variables */
#define CONFIG_EXTRA_ENV_SETTINGS \
	"fdt_addr=0x43000000\0"			\
	"mtdparts_arg=" CONFIG_MTDPARTS_DEFAULT "\0" \
	"console=ttymxc0,115200\0" \
	"bootcmd=" SUE_FWUPDATE_BOOTCOMMAND "\0" \
	"bootcmd_mfg=fastboot 0; reset\0" \
	"mfg_run=run kernel_common_args; setenv bootargs ${bootargs} rootfstype=ramfs; bootm ${fdt_addr}#default_factory@1\0" \
	SUE_FWUPDATE_EXTRA_ENV_SETTINGS

/* Link Definitions */
#define CONFIG_LOADADDR			0x40480000

#define CONFIG_SYS_LOAD_ADDR           CONFIG_LOADADDR

#define CONFIG_SYS_INIT_RAM_ADDR        0x40000000
#define CONFIG_SYS_INIT_RAM_SIZE        0x80000
#define CONFIG_SYS_INIT_SP_OFFSET \
        (CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
        (CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)


#define CONFIG_ENV_OFFSET		((4 + 2 + 2) * SZ_1M)
#define CONFIG_ENV_SIZE			SZ_256K
#define CONFIG_ENV_OVERWRITE

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + 32 * SZ_1M)

#define CONFIG_SYS_SDRAM_BASE           0x40000000
#define PHYS_SDRAM                      0x40000000
#define PHYS_SDRAM_SIZE			0x20000000 /* 512 MiB DDR */
#define CONFIG_NR_DRAM_BANKS		1

#define CONFIG_SYS_MEMTEST_START    PHYS_SDRAM
#define CONFIG_SYS_MEMTEST_END      (CONFIG_SYS_MEMTEST_START + (PHYS_SDRAM_SIZE >> 1))


#if defined(CONFIG_MXC_UART)
#define CONFIG_MXC_UART_BASE		UART1_BASE_ADDR
#endif

/* Monitor Command Prompt */
#define CONFIG_SYS_CBSIZE              2048
#define CONFIG_SYS_MAXARGS             256
#define CONFIG_SYS_BARGSIZE CONFIG_SYS_CBSIZE
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + \
					sizeof(CONFIG_SYS_PROMPT) + 16)

#if defined(CONFIG_NAND_MXS)

/* NAND stuff */
#define CONFIG_SYS_MAX_NAND_DEVICE     1
#define CONFIG_SYS_NAND_BASE           0x20000000
#define CONFIG_SYS_NAND_5_ADDR_CYCLE
#define CONFIG_SYS_NAND_ONFI_DETECTION

#define CONFIG_MTD_DEVICE

#endif /* CONFIG_NAND_MXS */

#define CONFIG_SYS_I2C_SPEED		100000

/* USB configs */
#ifndef CONFIG_SPL_BUILD
#define CONFIG_CMD_USB
#define CONFIG_USB_STORAGE
#define CONFIG_USBD_HS

#define CONFIG_CMD_USB_MASS_STORAGE
#define CONFIG_USB_GADGET_MASS_STORAGE
#define CONFIG_USB_FUNCTION_MASS_STORAGE

#endif

#define CONFIG_USB_GADGET_DUALSPEED
#define CONFIG_USB_GADGET_VBUS_DRAW 2

#define CONFIG_CI_UDC

#define CONFIG_MXC_USB_PORTSC  (PORT_PTS_UTMI | PORT_PTS_PTW)
#define CONFIG_USB_MAX_CONTROLLER_COUNT         2

#define CONFIG_OF_SYSTEM_SETUP

#define CONFIG_SYS_BOOTM_LEN SZ_32M

#endif