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

#include <nanvix/arch/mppa.h>
#include <nanvix/klib.h>
#include <assert.h>
#include <inttypes.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

/**
 * @brief Number of barriers.
 */
#define NR_BARRIER 3

/**
 * @brief Mailbox flags.
 */
/**@{*/
#define BARRIER_USED   (1 << 0)
#define BARRIER_WRONLY (1 << 1)
/**@}*/

/**
 *
 */

/**
 * @brief Barrier.
 */
struct barrier
{
	int local;  /**< Local cluster sync.  */
	int remote; /**< Remote cluster sync. */
	int flags;  /**< Flags.               */
};

/**
 * @brief table of barriers.
 */
static struct barrier barriers[NR_BARRIER];

/*=======================================================================*
 * barrier_alloc()                                                       *
 *=======================================================================*/

/**
 * @brief Allocates a barrier.
 *
 * @return Upon successful completion the ID of the newly allocated
 * barrier is returned. Upon failure, a negative error code is returned
 * instead.
 */
static int barrier_alloc(void)
{
	/* Search for a free barrier. */
	for (int i = 0; i < NR_BARRIER; i++)
	{
		/* Found. */
		if (!(barriers[i].flags & BARRIER_USED))
		{
			barriers[i].flags |= BARRIER_USED;
			return (i);
		}
	}

	return (-ENOENT);
}

/*=======================================================================*
 * barrier_free()                                                        *
 *=======================================================================*/

/**
 * @brief Frees a barrier.
 *
 * @param barrierid ID of the target barrier.
 */
static void barrier_free(int barrierid)
{
	/* Sanity check. */
	assert((barrierid >= 0) && (barrierid < NR_BARRIER));
	assert(barriers[barrierid].flags & BARRIER_USED);

	barriers[barrierid].flags = 0;
}

/*=======================================================================*
 * barrier_noctag()                                                      *
 *=======================================================================*/

/**
 * @brief Computes the barrier NoC tag for a cluster.
 *
 * @param local Id of target cluster.
 */
static int barrier_noctag(int local)
{
	if ((local == IOCLUSTER0) || (local == IOCLUSTER1))
		return (2);
	else if (local == CCLUSTER0)
		return (3);

	return (4);
}

/*=======================================================================*
 * barrier_open()                                                        *
 *=======================================================================*/

/**
 * @brief Opens the global barrier.
 *
 * @param nclusters Number of compute clusters in the barrier.
 */
int barrier_open(int ncclusters)
{
	int local;
	int barrierid;
	uint64_t mask;
	char pathname[128];

	local = k1_get_cluster_id();

	/* Allocate barrier. */
	barrierid = barrier_alloc();
	if (barrierid < 0)
		return (-ENOENT);

	/* I0 0 cluster barrier. */
	if (local == IOCLUSTER0)
	{
		snprintf(pathname,
			ARRAY_LENGTH(pathname),
			"/mppa/sync/%d:%d",
			IOCLUSTER0,
			barrier_noctag(IOCLUSTER0)
		);
		assert((barriers[barrierid].local = mppa_open(pathname, O_RDONLY)) != -1);

		mask = 0;
		assert(mppa_ioctl(barriers[barrierid].local, MPPA_RX_SET_MATCH, mask) == 0);

		snprintf(pathname,
			ARRAY_LENGTH(pathname),
			"/mppa/sync/%d:%d",
			IOCLUSTER1,
			barrier_noctag(IOCLUSTER1)
		);
		assert((barriers[barrierid].remote = mppa_open(pathname, O_WRONLY)) != -1);
	}

	/* IO 0 cluster barrier. */
	else if (local == IOCLUSTER1)
	{
		snprintf(pathname,
			ARRAY_LENGTH(pathname),
			"/mppa/sync/%d:%d",
			IOCLUSTER1,
			barrier_noctag(IOCLUSTER1)
		);
		assert((barriers[barrierid].local = mppa_open(pathname, O_RDONLY)) != -1);

		mask = 0;
		assert(mppa_ioctl(barriers[barrierid].local, MPPA_RX_SET_MATCH, mask) == 0);

		snprintf(pathname,
			ARRAY_LENGTH(pathname),
			"/mppa/sync/%d:%d",
			IOCLUSTER0,
			barrier_noctag(IOCLUSTER0)
		);
		assert((barriers[barrierid].remote = mppa_open(pathname, O_WRONLY)) != -1);
	}

	else
	{
		/* Invalid number of clusters. */
		if ((ncclusters < 2) || (ncclusters > NR_CCLUSTER))
			return (-EINVAL);

		/* Master compute cluster. */
		if (local == CCLUSTER0)
		{
			int cclusters[NR_CCLUSTER];

			snprintf(pathname,
				ARRAY_LENGTH(pathname),
				"/mppa/sync/%d:%d",
				CCLUSTER0,
				barrier_noctag(CCLUSTER0)
			);
			assert((barriers[barrierid].local = mppa_open(pathname, O_RDONLY)) != -1);

			mask = ~((1 << (ncclusters - 1)) - 1);
			assert(mppa_ioctl(barriers[barrierid].local, MPPA_RX_SET_MATCH, mask) == 0);

			snprintf(pathname,
				ARRAY_LENGTH(pathname),
				"/mppa/sync/[%d..%d]:%d",
				CCLUSTER1,
				CCLUSTER0 + (ncclusters - 1),
				barrier_noctag(CCLUSTER0 + ncclusters)
			);
			assert((barriers[barrierid].remote = mppa_open(pathname, O_WRONLY)) != -1);

			for (int i = 0; i < (ncclusters - 1); i++)
				cclusters[i] = i;
			assert(mppa_ioctl(barriers[barrierid].remote, MPPA_TX_SET_RX_RANKS, ncclusters - 1, cclusters) == 0);
		}
		else
		{
			snprintf(pathname,
				ARRAY_LENGTH(pathname),
				"/mppa/sync/[%d..%d]:%d",
				CCLUSTER1,
				CCLUSTER0 + (ncclusters - 1),
				barrier_noctag(CCLUSTER0 + ncclusters)
			);
			assert((barriers[barrierid].local = mppa_open(pathname, O_RDONLY)) != -1);

			mask = 0;
			assert(mppa_ioctl(barriers[barrierid].local, MPPA_RX_SET_MATCH, mask) == 0);

			snprintf(pathname,
				ARRAY_LENGTH(pathname),
				"/mppa/sync/%d:%d",
				CCLUSTER0,
				barrier_noctag(CCLUSTER0)
			);
			assert((barriers[barrierid].remote = mppa_open(pathname, O_WRONLY)) != -1);
		}
	}

	return (barrierid);
}

/*=======================================================================*
 * barrier_wait()                                                        *
 *=======================================================================*/

/**
 * @brief Waits on a barrier.
 *
 * @param barrierid ID of the target barrier.
 */
int barrier_wait(int barrierid)
{
	int local;
	uint64_t mask;

	/* Invalid barrier ID. */
	if ((barrierid < 0) || (barrierid >= NR_BARRIER))
		return (-EINVAL);

	/* Invalid barrier. */
	if (!(barriers[barrierid].flags & BARRIER_USED))
		return (-EINVAL);

	local = k1_get_cluster_id();

	/* IO 0 cluster barrier. */
	if (local == IOCLUSTER0)
	{
		assert(mppa_read(barriers[barrierid].local, &mask, sizeof(uint64_t)) == sizeof(uint64_t));
		mask = ~0;	
		assert(mppa_write(barriers[barrierid].remote, &mask, sizeof(uint64_t)) == sizeof(uint64_t));
	}
	else if (local == IOCLUSTER1)
	{
		mask = ~0;	
		assert(mppa_write(barriers[barrierid].remote, &mask, sizeof(uint64_t)) == sizeof(uint64_t));
		assert(mppa_read(barriers[barrierid].local, &mask, sizeof(uint64_t)) == sizeof(uint64_t));
	}

	/* Compute cluster barrier. */
	else
	{
		/* Master compute cluster. */
		if (local == CCLUSTER0)
		{
			assert(mppa_read(barriers[barrierid].local, &mask, sizeof(uint64_t)) == sizeof(uint64_t));
			mask = ~0;
			assert(mppa_write(barriers[barrierid].remote, &mask, sizeof(uint64_t)) == sizeof(uint64_t));
		}

		/* Worker compute cluster. */
		else
		{
			mask = 1 << (local - 1);
			assert(mppa_write(barriers[barrierid].remote, &mask, sizeof(uint64_t)) == sizeof(uint64_t));
			assert(mppa_read(barriers[barrierid].local, &mask, sizeof(uint64_t)) == sizeof(uint64_t));
		}
	}
	
	return (0);
}

/**
 * @brief Closes the global barrier.
 *
 * @param barrierid ID of the target barrier.
 */
int barrier_close(int barrierid)
{
	/* Invalid barrier ID. */
	if ((barrierid < 0) || (barrierid >= NR_BARRIER))
		return (-EINVAL);

	/* Invalid barrier. */
	if (!(barriers[barrierid].flags & BARRIER_USED))
		return (-EINVAL);

	assert(mppa_close(barriers[barrierid].local) != -1);
	assert(mppa_close(barriers[barrierid].remote) != -1);

	barrier_free(barrierid);

	return (0);
}
