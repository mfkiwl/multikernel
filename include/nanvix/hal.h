/*
 * Copyright(C) 2011-2018 Pedro H. Penna <pedrohenriquepenna@gmail.com>
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

#ifndef NANVIX_HAL_H_
#define NANVIX_HAL_H_

	#include <stddef.h>

	#ifdef _KALRAY_MPPA256
		#include <nanvix/arch/mppa.h>
	#endif

/*============================================================================*
 * Processor Management                                                       *
 *============================================================================*/

	/**
	 * @brief Type of cores.
	 */
	/**@{*/
	#define HAL_CORE_USER   0 /**< User core.                */
	#define HAL_CORE_RMAN   1 /**< Resource management core. */
	#define HAL_CORE_SYSTEM 2 /**< System core.              */
	/**@}*/

	/* Forward definitions. */
	extern int hal_get_cluster_id(void);
	extern int hal_get_core_id(void);
	extern int hal_get_core_type(void);
	extern int hal_is_ucore(int);
	extern int hal_is_rcore(int);
	extern int hal_is_score(int);
	extern int hal_get_num_cores(void);

/*============================================================================*
 * Power Management                                                           *
 *============================================================================*/

	/* Forward definitions. */
	extern int hal_get_core_freq(void);
	extern int hal_get_ucore_freq(void);
	extern int hal_get_rcore_freq(void);
	extern int hal_get_score_freq(void);

/*============================================================================*
 * Communication                                                              *
 *============================================================================*/

	/* Forward definitions. */
	extern int hal_get_node_id(void);

	/* Forward definitions. */
	extern int hal_mailbox_create(int);
	extern int hal_mailbox_open(int);
	extern int hal_mailbox_unlink(int);
	extern int hal_mailbox_close(int);
	extern size_t hal_mailbox_write(int, const void *, size_t);
	extern size_t hal_mailbox_read(int, void *, size_t);

	/* Forward definitions .*/
	extern int hal_portal_allow(portal_t *, int, int);
	extern int hal_portal_create(portal_t *, int);
	extern int hal_portal_open(portal_t *, int, int);
	extern int hal_portal_read(portal_t *, void *, size_t);
	extern int hal_portal_write(portal_t *, const void *, size_t);
	extern int hal_portal_close(portal_t *);
	extern int hal_portal_unlink(portal_t *);

#endif /* NANVIX_HAL_H_ */
