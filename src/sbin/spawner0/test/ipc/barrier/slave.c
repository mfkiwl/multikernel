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

#include <stdio.h>
#include <stdlib.h>

#include <nanvix/const.h>
#include <nanvix/syscalls.h>
#include <nanvix/pm.h>

/**
 * @brief Asserts a logic expression.
 */
#define TEST_ASSERT(x) { if (!(x)) exit(EXIT_FAILURE); }

/*============================================================================*
 * API Test: Create Unlink CC                                                 *
 *============================================================================*/

/**
 * @brief API Test: Create Unlink CC
 */
static void test_ipc_barrier_create_unlink_cc(int nclusters)
{
	int barrier;
	int nodes[nclusters];

	/* Build nodes list. */
	for (int i = 0; i < nclusters; i++)
		nodes[i] = i;

	TEST_ASSERT((barrier = barrier_create(nodes, nclusters)) >= 0);
	TEST_ASSERT(barrier_unlink(barrier) == 0);
}

/*============================================================================*
 * API Test: Wait CC                                                          *
 *============================================================================*/

/**
 * @brief API Test: Wait CC
 */
static void test_ipc_barrier_wait_cc(int nclusters)
{
	int barrier;
	int nodes[nclusters];

	/* Build nodes list. */
	for (int i = 0; i < nclusters; i++)
		nodes[i] = i;

	TEST_ASSERT((barrier = barrier_create(nodes, nclusters)) >= 0);
	TEST_ASSERT(barrier_wait(barrier) == 0);
	TEST_ASSERT(barrier_unlink(barrier) == 0);
}

/*============================================================================*
 * API Test: Wait 2 CC                                                        *
 *============================================================================*/

/**
 * @brief API Test: Wait 2 CC
 */
static void test_ipc_barrier_wait2_cc(int masternode, int nclusters)
{
	int barrier;
	int nodes[nclusters + 1];

	/* Build nodes list. */
	nodes[0] = masternode;
	for (int i = 0; i < nclusters; i++)
		nodes[i + 1] = i;

	TEST_ASSERT((barrier = barrier_create(nodes, nclusters + 1)) >= 0);
	TEST_ASSERT(barrier_wait(barrier) == 0);
	TEST_ASSERT(barrier_unlink(barrier) == 0);
}

/*====================================================================*
 * main                                                               *
 *====================================================================*/

/**
 * @brief Mailbox unit test.
 */
int main2(int argc, char **argv)
{
	int test;
	int nclusters;
	int masternode;

	/* Retrieve kernel parameters. */
	TEST_ASSERT(argc == 4);
	masternode = atoi(argv[1]);
	nclusters = atoi(argv[2]);
	test = atoi(argv[3]);

	switch(test)
	{
		/* Create Unlink CC */
		case 0:
			test_ipc_barrier_create_unlink_cc(nclusters);
			break;

		/* Compute clusters test. */
		case 1:
			test_ipc_barrier_wait_cc(nclusters);
			break;

		/* IO clusters - Compute clusters test. */
		case 2:
			test_ipc_barrier_wait2_cc(masternode, nclusters);
			break;

		/* Should not happen. */
		default:
			break;
	}

	return (EXIT_SUCCESS);
}