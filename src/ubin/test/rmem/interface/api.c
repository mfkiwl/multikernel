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

#define __NEED_RMEM_CACHE

#include <nanvix/runtime/rmem.h>
#include <nanvix/ulib.h>
#include "../../test.h"

#define __VERBOSE_TESTS 0

/**
 * @brief Dummy buffer used for tests.
 */
static char buffer[RMEM_BLOCK_SIZE];

/*============================================================================*
 * API Test: Alloc/Free                                                       *
 *============================================================================*/

/**
 * @brief API Test: Alloc/Free
 */
static void test_rmem_interface_alloc_free(void)
{
	for (size_t i = 1; i <= RMEM_BLOCK_SIZE; i = (i << 1))
	{
		void *ptr;

#if (__VERBOSE_TESTS)
		uprintf("ralloc() size=%d\n", i);
#endif

		TEST_ASSERT((ptr = nanvix_ralloc(i)) != RMEM_NULL);
		TEST_ASSERT(nanvix_rfree(ptr) == 0);

#if (__VERBOSE_TESTS)
		uprintf("rfree()  ptr=%x\n", ptr);
#endif
	}
}

/*============================================================================*
 * API Test: Read/Write                                                       *
 *============================================================================*/

/**
 * @brief API Test: Read/Write
 */
static void test_rmem_interface_read_write(void)
{
	char *ptr;

	TEST_ASSERT((ptr = nanvix_ralloc(RMEM_BLOCK_SIZE)) != RMEM_NULL);

	for (size_t base = 0; base < RMEM_BLOCK_SIZE; base = (base == 0) ? 1 : (base << 1))
	{
		size_t n = RMEM_BLOCK_SIZE - base;

#if (__VERBOSE_TESTS)
		uprintf("rwrite() base=%d n=%d\n", base, n);
#endif

		/* Aligned read. */
		umemset(buffer, 1, RMEM_BLOCK_SIZE);
		TEST_ASSERT(nanvix_rwrite(&ptr[base], buffer, n) == n);

#if (__VERBOSE_TESTS)
		uprintf("rread()  base=%d n=%d\n", base, n);
#endif

		/* Aligned write. */
		umemset(buffer, 0, RMEM_BLOCK_SIZE);
		TEST_ASSERT(nanvix_rread(buffer, &ptr[base], n) == n);

		/* Checksum. */
		for (size_t i = 0; i < n; i++)
			TEST_ASSERT(buffer[i] == 1);
	}

	TEST_ASSERT(nanvix_rfree(ptr) == 0);
}

/*============================================================================*/

/**
 * @brief Unit tests.
 */
struct test tests_rmem_interface_api[] = {
	{ test_rmem_interface_alloc_free, "alloc/free" },
	{ test_rmem_interface_read_write, "read/write" },
	{ NULL,                            NULL        },
};
