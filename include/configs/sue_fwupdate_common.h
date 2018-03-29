/*
 * sue_fwupdate_common.h
 *
 * This file contains the common U-Boot script for the firmware upgrade process.
 */

#ifndef __SUE_FWUPDATE_COMMON_H
#define __SUE_FWUPDATE_COMMON_H

/*
 * UPDATE PROCESS is fully described at: https://extern.streamunlimited.com:8443/display/Stream800/Firmware+Update+Architecture
 *
 * Update methods:
 *
 * METHODS:
 *              readfitUimage                   - loads uImage to RAM
 *              readswufitUimage                - loads swupdate uImage to RAM
 *              swu_boot                        - main method
 *
 * VARIABLES:
 *              bootcount                       - actual count of uncorrect reboots
 *              bootlimit                       - limit of uncorrcet reboots, if reached, board does not boot anymore
 *              swu_load_addr                   - base RAM address for u-boot operations
 *              factory_state                   - whether the board is in a factory state
 *              usb_update_req                  - whether the USB update request is active
 */

#define SUE_FWUPDATE_EXTRA_ENV_SETTINGS \
    "readfituImage=" \
        "if nand read ${loadaddr} fit; " \
            "then " \
            "echo \"INFO: kernel partition load successful\"; " \
        "else " \
            "echo \"ERROR: cannot load kernel image from nand\"; " \
            "reset; " \
        "fi;\0" \
\
\
    "readswufituImage=" \
        "if nand read ${loadaddr} swufit; " \
            "then " \
            "echo \"INFO: swupdate kernel partition load successful\"; " \
        "else " \
            "echo \"ERROR: cannot load swupdate kernel image from nand\"; " \
            "reset; " \
        "fi;\0" \
\
\
    "bootfitimage=" \
        "if env exists fit_config; " \
            "then " \
            "echo \"INFO: will boot fit config ${fit_config}@1\"; " \
            "bootm ${loadaddr}#${fit_config}@1; " \
        "fi; " \
        "echo \"INFO: will try to boot the default fit config\"; " \
        "bootm ${loadaddr}; " \
        "echo \"INFO: fit boot failed...\"; " \
        "echo \"INFO: resetting...\"; " \
        "reset;\0" \
\
    "kernel_common_args=const toenv eth_int_addr; " \
        "setenv bootargs console=ttyS0,115200 panic=1; " \
        /*"fec.macaddr=${eth_int_addr} ${mtdparts} ${optargs}; " */ \
        "if test ${secure_board} = 1; " \
            "then " \
            "echo \"INFO: board is locked, booting to runlevel 3\"; " \
            "setenv bootargs ${bootargs} 3; " \
        "fi;\0" \
\
    "nandroot=ubi0:nsdk-rootfs rw\0" \
    "nandrootfstype=ubifs\0" \
    "nandargs=run kernel_common_args; " \
        "setenv bootargs ${bootargs} " \
        "ubi.mtd=6 root=${nandroot} noinitrd " \
        "rootfstype=${nandrootfstype};\0" \
    "nand_boot=echo \"Booting from nand ...\"; " \
        "run nandargs; " \
        "echo \"INFO: loading fit image into RAM...\"; " \
        /*"bstate booting; "*/ \
        "run readfituImage; " \
        "echo \"INFO: booting fit image...\"; " \
        "run bootfitimage;\0" \
\
\
    "swunandargs=run kernel_common_args; " \
        "setenv bootargs ${bootargs} rootfstype=ramfs " \
        /* "factory_state=${factory_state} usb_update_req=${usb_update_req} " */ \
        "secure_board=${secure_board};\0" \
    "swu_nand_boot=echo \"Booting swu from nand ...\"; " \
        "run swunandargs; " \
        "echo \"INFO: loading swu fit image into RAM...\"; " \
        /*"bstate dontunplug; "*/ \
        "run readswufituImage; " \
        "if test ${secure_board} = 0; " \
            "then " \
            "echo \"INFO: board is not locked, do not enforce swufit signature\"; " \
            "setenv verify no; " \
        "fi; " \
        "echo \"INFO: booting swu fit image...\"; " \
        "run bootfitimage;\0" \
\
    "swu_boot=" \
        "if fwup fail; " \
            "then " \
            "if test ${bootcount} -gt ${bootlimit}; " \
                "then " \
                "echo \"INFO: bootcount(${bootcount}) greater than bootlimit(${bootlimit})\"; " \
                /*"bstate hardfailure; "*/ \
                "echo \"ERROR: Maximum boot count reached!\"; " \
                "while true; do sleep 100; done; " \
            "fi; " \
        "fi; " \
        "boot_swupdate=no; " \
        "if fwup fail; " \
            "then " \
            "min_boot_retry=3;" \
            "if test ${bootcount} -gt ${min_boot_retry}; " \
                " then " \
                    "echo \"INFO: Fail flag is set, bootcount is greater than ${min_boot_retry}\"; " \
                    "boot_swupdate=yes; " \
                " else " \
                    "echo \"INFO: Fail flag is set but bootcount not greater than ${min_boot_retry}\"; " \
                "fi; " \
        "fi; " \
        "if fwup update; " \
            "then " \
            "echo \"INFO: Update flag is set\"; " \
            "boot_swupdate=yes; " \
        "fi; " \
        "if test ${boot_swupdate} = yes; " \
            "then " \
            "echo \"INFO: Booting swupdate image\"; " \
            /*"bstate dontunplug; "*/ \
            "run swu_nand_boot; " \
        "else " \
            "echo \"INFO: Booting regular image\"; " \
            "echo \"INFO: Setting fail flag...\"; " \
            "fwup set fail; " \
            /*"bstate normal; "*/ \
            "run nand_boot; " \
        "fi;\0" \

#define SUE_FWUPDATE_BOOTCOMMAND \
        "echo \"INFO: attempting SWU boot...\"; " \
        "fwup flags; "\
        "run swu_boot;" \

#define SUE_FWUPDATE_ALTBOOTCOMMAND \
        "echo \"ERROR: Maximum boot count reached!\"; while true; do sleep 100; done; "

#endif /* __SUE_FWUPDATE_COMMON_H */
