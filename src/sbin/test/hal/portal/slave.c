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

#include <stdlib.h>
#include <errno.h>

#define __NEED_HAL_CORE_
#define __NEED_HAL_NOC_
#define __NEED_HAL_SYNC_
#define __NEED_HAL_PORTAL_
#include <nanvix/hal.h>
#include <nanvix/limits.h>
#include <nanvix/pm.h>

/**
 * @brief Asserts a logic expression.
 */
#define TEST_ASSERT(x) { if (!(x)) exit(EXIT_FAILURE); }

/**
 * @brief Data Size.
 */
#define DATA_SIZE 128

/**
 * @brief ID of master node.
 */
static int masternode;

/**
 * @brief Underlying NoC node ID.
 */
static int nodeid;

/**
 * @brief Data buffer.
 */
static char buffer[DATA_SIZE];

/*============================================================================*
 * Utilities                                                                  *
 *============================================================================*/

/**
 * @brief Sync slaves.
 *
 * @param nclusters Number of slaves.
 */
static void sync_slaves(int nclusters)
{
	int syncid1, syncid2;
	int nodes[NANVIX_PROC_MAX];

	/* Build nodes list. */
	for (int i = 0; i < nclusters; i++)
		nodes[i] = i;

	/* Open synchronization points. */
	if (nodeid == 0)
	{
		TEST_ASSERT((syncid1 = hal_sync_create(nodes,
			nclusters,
			HAL_SYNC_ALL_TO_ONE)) >= 0
		);
		TEST_ASSERT((syncid2 = hal_sync_open(nodes,
			nclusters,
			HAL_SYNC_ONE_TO_ALL)) >= 0
		);

		TEST_ASSERT(hal_sync_wait(syncid1) == 0);
		TEST_ASSERT(hal_sync_signal(syncid2) == 0);

		/* House keeping. */
		TEST_ASSERT(hal_sync_close(syncid2) == 0);
		TEST_ASSERT(hal_sync_unlink(syncid1) == 0);
	}
	else
	{
		TEST_ASSERT((syncid2 = hal_sync_create(nodes,
			nclusters,
			HAL_SYNC_ONE_TO_ALL)) >= 0
		);
		TEST_ASSERT((syncid1 = hal_sync_open(nodes,
			nclusters,
			HAL_SYNC_ALL_TO_ONE)) >= 0
		);

		TEST_ASSERT(hal_sync_signal(syncid1) == 0);
		TEST_ASSERT(hal_sync_wait(syncid2) == 0);

		/* House keeping. */
		TEST_ASSERT(hal_sync_unlink(syncid2) == 0);
		TEST_ASSERT(hal_sync_close(syncid1) == 0);
	}
}

/**
 * @brief Sync with master.
 *
 * @poaram nclusters Number of slaves.
 */
static void sync_master(int nclusters)
{
	int syncid;
	int nodes[NANVIX_PROC_MAX + 1];

	/* Build nodes list. */
	nodes[0] = masternode;
	for (int i = 0; i < nclusters; i++)
		nodes[i + 1] = i;

	TEST_ASSERT((syncid = hal_sync_open(nodes,
		nclusters + 1,
		HAL_SYNC_ALL_TO_ONE)) >= 0
	);

	TEST_ASSERT(hal_sync_signal(syncid) == 0);
	TEST_ASSERT(hal_sync_close(syncid) == 0);
}

/*============================================================================*
 * API Test: Create Unlink CC                                                 *
 *============================================================================*/

/**
 * @brief API Test: Create Unlink CC
 */
static void test_hal_portal_create_unlink(void)
{
	int inportal;

	TEST_ASSERT((inportal = hal_portal_create(nodeid)) >= 0);
	TEST_ASSERT(hal_portal_unlink(inportal) == 0);
}

/*============================================================================*
 * API Test: Open Close CC                                                    *
 *============================================================================*/

/**
 * @brief API Test: Open Close CC
 */
static void test_hal_portal_open_close(void)
{
	int outportal;

	TEST_ASSERT((outportal = hal_portal_open(masternode)) >= 0);
	TEST_ASSERT(hal_portal_close(outportal) == 0);
}

/*============================================================================*
 * API Test: Read Write CC                                                    *
 *============================================================================*/

/**
 * @brief API Test: Read Write CC
 */
static void test_hal_portal_read_write(int nclusters)
{
	int inportal;
	int outportal;

	TEST_ASSERT((inportal = hal_portal_create(nodeid)) >= 0);

	sync_slaves(nclusters);

	TEST_ASSERT((outportal = hal_portal_open((nodeid + 1)%nclusters)) >= 0);

	if (nodeid != 0)
	{
		TEST_ASSERT((hal_portal_allow(
			inportal,
			(nodeid == 0) ?
				nclusters - 1 :
				(nodeid - 1)%nclusters)) == 0
		);
	}

	if (nodeid != (nclusters - 1))
	{
		TEST_ASSERT((hal_portal_write(
			outportal,
			buffer,
			DATA_SIZE) == DATA_SIZE)
		);
	}

	if (nodeid != 0)
	{
		TEST_ASSERT((hal_portal_read(
			inportal,
			buffer,
			DATA_SIZE) == DATA_SIZE)
		);
	}

	/* House keeping. */
	TEST_ASSERT(hal_portal_close(outportal) == 0);
	TEST_ASSERT(hal_portal_unlink(inportal) == 0);
}

/*============================================================================*
 * API Test: Read Write 2 CC                                                  *
 *============================================================================*/

/**
 * @brief API Test: Read Write 2 CC
 */
static void test_hal_portal_read_write2(int nclusters)
{
	int inportal;

	TEST_ASSERT((inportal = hal_portal_create(nodeid)) >= 0);

	sync_master(nclusters);

	TEST_ASSERT((hal_portal_allow(
		inportal,
		masternode) == 0)
	);

	TEST_ASSERT((hal_portal_read(
		inportal,
		buffer,
		DATA_SIZE) == DATA_SIZE)
	);

	/* House keeping. */
	TEST_ASSERT(hal_portal_unlink(inportal) == 0);
}

/*============================================================================*/

/**
 * @brief HAL Mailbox Test Driver
 */
int main2(int argc, char **argv)
{
	int test;
	int nclusters;

	/* Retrieve kernel parameters. */
	TEST_ASSERT(argc == 4);
	masternode = atoi(argv[1]);
	nclusters = atoi(argv[2]);
	test = atoi(argv[3]);

	nodeid = hal_get_node_id();

	((void) nclusters);

	switch (test)
	{
		case 0:
			test_hal_portal_create_unlink();
			break;
		case 1:
			test_hal_portal_open_close();
			break;
		case 2:
			test_hal_portal_read_write(nclusters);
			break;
		case 3:
			test_hal_portal_read_write2(nclusters);
			break;
		default:
			exit(EXIT_FAILURE);
	}

	return (EXIT_SUCCESS);
}
