/*
 * MIT License
 *
 * Copyright (c) 2011-2018 Pedro Henrique Penna <pedrohenriquepenna@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.  THE SOFTWARE IS PROVIDED
 * "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
 * LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#define __NEED_HAL_CORE_
#define __NEED_HAL_NOC_
#define __NEED_HAL_SETUP_
#define __NEED_HAL_SYNC_
#include <nanvix/hal.h>

/**
 * @brief ID of master node.
 */
static int masternode;

/*============================================================================*
 * API Test: Create Unlink                                                    *
 *============================================================================*/

/**
 * @brief API Test: Create Unlink
 */
static void test_hal_sync_create_unlink(int nclusters)
{
	int syncid;
	int nodes[nclusters];

	/* Build nodes list. */
	for (int i = 0; i < nclusters; i++)
		nodes[i] = i;

	/* Fix nodes list. */
	if (nodes[0] == hal_get_node_id())
	{
		nodes[0] += nodes[1];
		nodes[1] -= nodes[1];
		nodes[0] -= nodes[1];
	}

	assert((syncid = hal_sync_create(nodes,
		nclusters,
		HAL_SYNC_ONE_TO_ALL)) >= 0
	);

	assert(hal_sync_unlink(syncid) == 0);
}

/*============================================================================*
 * API Test: Open Close                                                       *
 *============================================================================*/

/**
 * @brief API Test: Open Close
 */
static void test_hal_sync_master_open_close(int nclusters)
{
	int nodeid;
	int syncid;
	int syncid_local;
	int nodes[nclusters];
	int nodes_local[nclusters];

	/* Build local nodes list. */
	for (int i = 0; i < nclusters; i++)
		nodes_local[i] = i;

	nodeid = hal_get_node_id();

	if (nodes_local[0] == nodeid)
	{
		nodes_local[0] += nodes_local[1];
		nodes_local[1] -= nodes_local[1];
		nodes_local[0] -= nodes_local[1];
	}

	assert((syncid_local = hal_sync_create(
		nodes_local,
		nclusters,
		HAL_SYNC_ONE_TO_ALL)) == 0
	);

	/* Build nodes list. */
	for (int i = 0; i < nclusters; i++)
		nodes[i] = i;

	/* Fix nodes list. */
	if (nodes[0] != nodeid)
	{
		for (int i = 1; i < nclusters; i++)
		{
			if (nodes[i] == nodeid)
			{
				nodes[i] = nodes[0];
				nodes[0] = nodeid;
			}
		}
	}

	assert((syncid = hal_sync_open(
		nodes,
		nclusters,
		HAL_SYNC_ONE_TO_ALL)) >= 0
	);

	assert(hal_sync_close(syncid) == 0);

	assert(hal_sync_unlink(syncid_local) == 0);
}

/*============================================================================*
 * API Test: Wait Signal                                                      *
 *============================================================================*/

/**
 * @brief API Test: Wait Signal
 */
static void test_hal_sync_wait_signal(int nclusters)
{
	int syncid;
	int nodes[nclusters + 1];

	/* Build nodes list. */
	nodes[0] = masternode;

	for (int i = 0; i < nclusters; i++)
		nodes[i + 1] = i;

	assert((syncid = hal_sync_create(
		nodes,
		nclusters + 1,
		HAL_SYNC_ONE_TO_ALL)) >= 0
	);

	assert(hal_sync_wait(syncid) == 0);

	assert(hal_sync_unlink(syncid) == 0);
}

/*============================================================================*
 * API Test: Signal Wait                                                      *
 *============================================================================*/

/**
 * @brief API Test: Signal Wait
 */
static void test_hal_sync_signal_wait(int nclusters)
{
	int syncid;
	int nodes[nclusters + 1];

	/* Build nodes list. */
	nodes[0] = masternode;

	for (int i = 0; i < nclusters; i++)
		nodes[i + 1] = i;

	assert((syncid = hal_sync_open(nodes,
		nclusters + 1,
		HAL_SYNC_ALL_TO_ONE)) >= 0
	);

	assert(hal_sync_signal(syncid) == 0);

	assert(hal_sync_close(syncid) == 0);
}

/*============================================================================*/

/**
 * @brief HAL Sync Test Driver
 */
int main2(int argc, char **argv)
{
	int test;
	int nclusters;

	/* Retrieve kernel parameters. */
	assert(argc == 4);
	masternode = atoi(argv[1]);
	nclusters = atoi(argv[2]);
	test = atoi(argv[3]);

	switch (test)
	{
		case 0:
			test_hal_sync_create_unlink(nclusters);
			break;
		case 1:
			test_hal_sync_master_open_close(nclusters);
			break;
		case 2:
			test_hal_sync_wait_signal(nclusters);
			break;
		case 3:
			test_hal_sync_signal_wait(nclusters);
			break;
		default:
			exit(EXIT_FAILURE);
	}

	return (EXIT_SUCCESS);
}
