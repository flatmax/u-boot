#include <common.h>

#include "device_interface.h"


static int fwup_request = 0;

static int generic_board_init(const struct sue_device_info *device)
{
	return 0;
}

static int generic_board_late_init(const struct sue_device_info *device)
{
	return 0;
}

static int generic_board_get_usb_update_request(const struct sue_device_info *device)
{
	return fwup_request;
}

struct sue_carrier_ops generic_board_ops = {
	.init = generic_board_init,
	.late_init = generic_board_late_init,
	.get_usb_update_request = generic_board_get_usb_update_request,
};
