/*
 * flags_imx8.h
 *
 * Copyright (C) 2019, StreamUnlimited Engineering GmbH, http://www.streamunlimited.com/
 * Martin Pietryka <martin.pietryka@streamunlimited.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR /PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __FLAGS_IMX8_H__
#define __FLAGS_IMX8_H__

#include <common.h>

int flag_write(u8 index, bool val);
int flag_read(u8 index, bool *val);
int flags_clear(void);

int bootcnt_write(u8 data);
int bootcnt_read(u8 *data);

#endif /* __FLAGS_IMX8_H__ */
