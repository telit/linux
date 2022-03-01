/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2023, Daniele Palmas <dnlplm@gmail.com>. All rights reserved.
 *
 */

#ifndef _MHI_BACKPORT_H
#define _MHI_BACKPORT_H

static inline void fsleep(unsigned long usecs)
{
	if (usecs <= 10)
		udelay(usecs);
	else if (usecs <= 20000)
		usleep_range(usecs, 2 * usecs);
	else
		msleep(DIV_ROUND_UP(usecs, 1000));
}

#endif /* _MHI_BACKPORT_H */
