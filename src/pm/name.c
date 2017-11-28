/*
 * Copyright(C) 2011-2017 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * This file is part of Nanvix.
 * 
 * Nanvix is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Nanvix is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Nanvix. If not, see <http://www.gnu.org/licenses/>.
 */

#include <errno.h>
#include <stdlib.h>

#include <nanvix/name.h>
#include <nanvix/klib.h>

/**
 * @brief Lookup table of addresses.
 */
static struct {
	const char *name;    /**< Process name.       */
	const char *address; /**< Physical address.   */
	unsigned short port; /**< Communication port. */
} addresses[] = {
	{ "/sys/bdev",     "10.111.0.1",  0x8001 },
	{ "/dev/ramdisk0", "10.111.0.5",  0x8002 },
	{ "/dev/ramdisk1", "10.111.0.6",  0x8003 },
	{ "/dev/ramdisk2", "10.111.0.7",  0x8004 },
	{ "/dev/ramdisk3", "10.111.0.8",  0x8005 },
	{ "/dev/ramdisk4", "10.111.0.9",  0x8006 },
	{ "/dev/ramdisk5", "10.111.0.10", 0x8007 },
	{ "/dev/ramdisk6", "10.111.0.11", 0x8008 },
	{ "/dev/ramdisk7", "10.111.0.12", 0x8009 },
	{ NULL, NULL, 0 }
};

/**
 * @brief Resolves a process name into an address.
 *
 * @param name Process name.
 * @param addr Address stor elocation.
 *
 * @returns Upon successful completion zero is returned;
 * otherwise a negative error code number is returned instead.
 */
int nanvix_lookup(const char *name, struct nanvix_process_addr *addr)
{
	/* Sanity check. */
	if ((name == NULL) || (addr == NULL))
		return (-EINVAL);

	/* Resolve process name. */
	for (int i = 0; addresses[i].name != NULL; i++)
	{
		/* Found. */
		if (!kstrcmp(addresses[i].name, name))
		{
			addr->port = htons(addresses[i].port);
			inet_pton(AF_INET, addresses[i].address, &addr->addr);

			return (0);
		}
	}

	return (-EINVAL);
}