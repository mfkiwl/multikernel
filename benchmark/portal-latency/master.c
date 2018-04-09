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

#include <mppa/osconfig.h>
#include <nanvix/arch/mppa.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "kernel.h"

/*===================================================================*
 * Barrier                                                           *
 *===================================================================*/

/**
 * @brief Global barrier.
 */
static struct
{
	int sync_slaves;           /**< Slaves sync NoC connector.           */
	int sync_master;           /**< Master sync NoC connector.           */
	int nclusters;             /**< Number of cclusters in the barrier.  */
	int clusters[NR_CCLUSTER]; /**< Cclusters in the barrier.            */
} barrier;

/**
 * @brief Opens the global barrier.
 *
 * @param nclusters Number of cclusters in the barrier.
 */
static void barrier_open(int nclusters)
{
	char pathname[128];

	/* Open slave sync connector. */
	sprintf(pathname,
			"/mppa/sync/[%d..%d]:%d",
			CCLUSTER0,
			CCLUSTER15,
			BARRIER_SLAVE_CNOC
	);
	barrier.sync_slaves = mppa_open(pathname, O_WRONLY);
	assert(barrier.sync_slaves != -1);

	/* Open master sync connector. */
	sprintf(pathname,
			"/mppa/sync/%d:%d",
			IOCLUSTER0,
			BARRIER_MASTER_CNOC
	);
	barrier.sync_master = mppa_open(pathname, O_RDONLY);
	assert(barrier.sync_master != -1);

	/* Initialize barrier.*/
	barrier.nclusters = nclusters;
	for (int j = 0; j < barrier.nclusters; j++)
		barrier.clusters[j] = j;
}

/**
 * @brief Waits on the global barrier.
 */
static void barrier_wait(void)
{
	uint64_t mask;

	/* Wait for slaves. */
	mask = ~((1 << barrier.nclusters) - 1);
	assert(mppa_ioctl(barrier.sync_master, MPPA_RX_SET_MATCH, mask) == 0);
	assert(mppa_read(barrier.sync_master, &mask, sizeof(uint64_t)) != -1);

	/* Unblock slaves. */
	mask = -1;
	assert(mppa_ioctl(barrier.sync_slaves, MPPA_TX_SET_RX_RANKS, barrier.nclusters, barrier.clusters) == 0);
	assert(mppa_write(barrier.sync_slaves, &mask, sizeof(uint64_t)) != -1);
}

/**
 * @brief Closes the global barrier.
 */
static void barrier_close(void)
{
	mppa_close(barrier.sync_master);
	mppa_close(barrier.sync_slaves);
}

/*===================================================================*
 * Portal                                                            *
 *===================================================================*/

/**
 * @brief Input portals.
 */
static struct
{
	int fd;            /**< Portal connector.               */
	mppa_aiocb_t aiocb; /**< Pending asynchronous operation. */
} portals[NR_IOCLUSTER_DMA];

/**
 * @brief Opens input portal.
 *
 * @para buffer Target buffer.
 *
 * @param buffer  Target buffer.
 * @parm  size    Write size.
 * @param dma     Target DMA channel.
 * @param trigger Trigger level.
 */
static void portal_open(char *buffer, int size, int dma, int trigger)
{
	char pathname[128];

	/* Open portal connector. */
	sprintf(pathname,
			"/mppa/portal/%d:%d",
			IOCLUSTER0 + dma,
			PORTAL_DNOC
	);
	portals[dma].fd = mppa_open(pathname, O_RDONLY);
	assert(portals[dma].fd != -1);

	/* Setup read operation. */
	mppa_aiocb_ctor(&portals[dma].aiocb, portals[dma].fd, &buffer[dma*(NR_CCLUSTER/NR_IOCLUSTER_DMA)*size], (NR_CCLUSTER/NR_IOCLUSTER_DMA)*size);
	mppa_aiocb_set_trigger(&portals[dma].aiocb, trigger);
	assert(mppa_aio_read(&portals[dma].aiocb) != -1);
}

/**
 * @brief Closes input portal.
 *
 * @param dma Target DMA channel.
 */
static inline void portal_close(int dma)
{
	assert(mppa_close(portals[dma].fd) != -1);
}

/**
 * @brief Reads data from input portal.
 *
 * @param dma Target DMA channel.
 */
static inline void portal_read(int dma)
{
	assert(mppa_aio_rearm(&portals[dma].aiocb) != -1);
}

/*===================================================================*
 * Process Management                                                *
 *===================================================================*/

/**
 * @brief ID of slave processes.
 */
static int pids[NR_CCLUSTER];

/**
 * @brief Spawns slave processes. 
 *
 * @param nclusters Number of clusters to spawn.
 * @param args      Cluster arguments.
 */
static void spawn_slaves(int nclusters, char **args) 
{
	const char *argv[] = {
		"portal-latency-slave",
		args[1],
		args[2],
		args[3],
		NULL
	};

	for (int i = 0; i < nclusters; i++)
		assert((pids[i] = mppa_spawn(i, NULL, argv[0], argv, NULL)) != -1);
}

/**
 * @brief Wait for slaves to complete.
 *
 * @param nclusters Number of slaves to wait.
 */
static void join_slaves(int nclusters) 
{
	for (int i = 0; i < nclusters; i++)
		assert(mppa_waitpid(pids[i], NULL, 0) != -1);
}

/*===================================================================*
 * Kernel                                                            *
 *===================================================================*/

/**
 * @brief Buffer.
 */
static char buffer[NR_CCLUSTER*MAX_BUFFER_SIZE];

/**
 * @brief Benchmarks write operations on a portal connector.
 */
int main(int argc, char **argv)
{
	int size;                      /* Write size.            */
	int nclusters;                 /* Number of cclusters.   */
	int trigger[NR_IOCLUSTER_DMA]; /* Trigger level per DMA. */

	assert(argc == 4);

	/* Retrieve kernel parameters. */
	nclusters = atoi(argv[2]);
	assert((size = atoi(argv[3])) <= MAX_BUFFER_SIZE);

	spawn_slaves(nclusters, argv);

	/* Distribute messages across DMA channels. */
	for (int i = 0; i < NR_IOCLUSTER_DMA; i++)
		trigger[i] = nclusters/NR_IOCLUSTER_DMA;
	for (int i = 0; i < (nclusters%NR_IOCLUSTER_DMA); i++)
		trigger[i]++;

	/* Open input portals. */
	for (int i = 0; i < NR_IOCLUSTER_DMA; i++)
		portal_open(buffer, size, i, trigger[i]);

	barrier_open(nclusters);

	/*
	 * Touch data to initialize all pages
	 * and warmup D-cache.
	 */
	memset(buffer, 0, NR_CCLUSTER*size);

	/* 
	 * Benchmark. First iteration is
	 * used to warmup resources.
	 */
	for (int i = 0; i <= NITERATIONS; i++)
	{
		barrier_wait();

		/* Read. */
		for (int j = 0; j < NR_IOCLUSTER_DMA; j++)
			portal_read(j);
		barrier_wait();
	}

	/* House keeping. */
	barrier_close();
	for (int i = 0; i < NR_IOCLUSTER_DMA; i++)
		portal_close(i);
	join_slaves(nclusters);

	return (EXIT_SUCCESS);
}