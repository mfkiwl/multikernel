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

#include <string.h>
#include <stdlib.h>

#include <nanvix/syscalls.h>
#include <nanvix/name.h>
#include <nanvix/limits.h>
#include <nanvix/pm.h>

#include "test.h"

/*============================================================================*
 * API Test: Invalid Create                                                   *
 *============================================================================*/

/**
 * @brief API Test: Invalid Create
 */
static void test_nanvix_ipc_mailbox_invalid_create(void)
{
	TEST_ASSERT(mailbox_create(NULL) < 0);
}

/*============================================================================*
 * API Test: Double Create                                                    *
 *============================================================================*/

/**
 * @brief API Test: Double Create
 */
static void test_nanvix_ipc_mailbox_double_create(void)
{
	int inbox;

	TEST_ASSERT((inbox = mailbox_create("cool-name")) >=  0);
	TEST_ASSERT(mailbox_create("cool-name") < 0);
	TEST_ASSERT(mailbox_unlink(inbox) == 0);
}

/*============================================================================*
 * API Test: Invalid Unlink                                                   *
 *============================================================================*/

/**
 * @brief API Test: Invalid Unlink
 */
static void test_nanvix_ipc_mailbox_invalid_unlink(void)
{
	TEST_ASSERT(mailbox_unlink(-1) < 0);
	TEST_ASSERT(mailbox_unlink(1000000) < 0);
}

/*============================================================================*
 * API Test: Double Unlink                                                    *
 *============================================================================*/

/**
 * @brief API Test: Double Unlink
 */
static void test_nanvix_ipc_mailbox_double_unlink(void)
{
	int inbox;

	TEST_ASSERT((inbox = mailbox_create("cool-name")) >=  0);
	TEST_ASSERT(mailbox_unlink(inbox) == 0);
	TEST_ASSERT(mailbox_unlink(inbox) < 0);
}

/*============================================================================*
 * API Test: Invalid Open                                                     *
 *============================================================================*/

/**
 * @brief API Test: Invalid Open
 */
static void test_nanvix_ipc_mailbox_invalid_open(void)
{
	TEST_ASSERT(mailbox_open(NULL) < 0);
}

/*============================================================================*
 * API Test: Bad Open                                                         *
 *============================================================================*/

/**
 * @brief API Test: Bad Open
 */
static void test_nanvix_ipc_mailbox_bad_open(void)
{
	int inbox;
	char pathname[NANVIX_PROC_NAME_MAX + 1];

	memset(pathname, 1, NANVIX_PROC_NAME_MAX + 1);

	TEST_ASSERT(mailbox_open("") < 0);
	TEST_ASSERT(mailbox_open(pathname) < 0);
	TEST_ASSERT(mailbox_open("missing-name") < 0);
	TEST_ASSERT((inbox = mailbox_create("cool-name")) >= 0);
	TEST_ASSERT(mailbox_open("cool-name") < 0);
	TEST_ASSERT(mailbox_unlink(inbox) == 0);
}

/*============================================================================*
 * API Test: Invalid Close                                                    *
 *============================================================================*/

/**
 * @brief API Test: Invalid Close
 */
static void test_nanvix_ipc_mailbox_invalid_close(void)
{
	TEST_ASSERT(mailbox_close(-1) < 0);
	TEST_ASSERT(mailbox_close(1000000) < 0);
}

/*============================================================================*
 * API Test: Bad Close                                                        *
 *============================================================================*/

/**
 * @brief API Test: Bad Close
 */
static void test_nanvix_ipc_mailbox_bad_close(void)
{
	int inbox;

	TEST_ASSERT((inbox = mailbox_create("cool-name")) >=  0);
	TEST_ASSERT(mailbox_close(inbox) < 0);
	TEST_ASSERT(mailbox_unlink(inbox) == 0);
}

/*============================================================================*
 * API Test: Invalid Read                                                     *
 *============================================================================*/

/**
 * @brief API Test: Invalid Read
 */
static void test_nanvix_ipc_mailbox_invalid_read(void)
{
	char buffer[MAILBOX_MSG_SIZE];

	TEST_ASSERT(mailbox_read(-1, buffer, MAILBOX_MSG_SIZE) < 0);
	TEST_ASSERT(mailbox_read(1000000, buffer, MAILBOX_MSG_SIZE) < 0);
}

/*============================================================================*
 * API Test: Invalid Read Size                                                *
 *============================================================================*/

/**
 * @brief API Test: Invalid Read Size
 */
static void test_nanvix_ipc_mailbox_invalid_read_size(void)
{
	int inbox;
	char buffer[MAILBOX_MSG_SIZE];

	TEST_ASSERT((inbox = mailbox_create("cool-name")) >=  0);
	TEST_ASSERT(mailbox_read(inbox, buffer, -1) < 0);
	TEST_ASSERT(mailbox_read(inbox, buffer, 0) < 0);
	TEST_ASSERT(mailbox_read(inbox, buffer, MAILBOX_MSG_SIZE - 1) < 0);
	TEST_ASSERT(mailbox_read(inbox, buffer, MAILBOX_MSG_SIZE + 1) < 0);
	TEST_ASSERT(mailbox_unlink(inbox) == 0);
}

/*============================================================================*
 * API Test: Null Read                                                        *
 *============================================================================*/

/**
 * @brief API Test: Null Read
 */
static void test_nanvix_ipc_mailbox_null_read(void)
{
	int inbox;

	TEST_ASSERT((inbox = mailbox_create("cool-name")) >=  0);
	TEST_ASSERT(mailbox_read(inbox, NULL, MAILBOX_MSG_SIZE) < 0);
	TEST_ASSERT(mailbox_unlink(inbox) == 0);
}

/*============================================================================*
 * API Test: Invalid Write                                                    *
 *============================================================================*/

/**
 * @brief API Test: Invalid Write
 */
static void test_nanvix_ipc_mailbox_invalid_write(void)
{
	char buffer[MAILBOX_MSG_SIZE];

	TEST_ASSERT(mailbox_write(-1, buffer, MAILBOX_MSG_SIZE) < 0);
	TEST_ASSERT(mailbox_write(1000000, buffer, MAILBOX_MSG_SIZE) < 0);
}

/*============================================================================*
 * API Test: Bad Write                                                        *
 *============================================================================*/

/**
 * @brief API Test: Bad Write
 */
static void test_nanvix_ipc_mailbox_bad_write(void)
{
	int inbox;
	char buffer[MAILBOX_MSG_SIZE];

	TEST_ASSERT((inbox = mailbox_create("cool-name")) >=  0);
	TEST_ASSERT(mailbox_write(inbox, buffer, MAILBOX_MSG_SIZE) < 0);
	TEST_ASSERT(mailbox_unlink(inbox) == 0);
}

/*============================================================================*/

/**
 * @brief Unit tests.
 */
struct test nanvix_ipc_mailbox_tests_fault[] = {
	{ test_nanvix_ipc_mailbox_invalid_create,     "Invalid Create"    },
	{ test_nanvix_ipc_mailbox_double_create,      "Double Create"     },
	{ test_nanvix_ipc_mailbox_invalid_unlink,     "Invalid Unlink"    },
	{ test_nanvix_ipc_mailbox_double_unlink,      "Double Unlink"     },
	{ test_nanvix_ipc_mailbox_invalid_open,       "Invalid Open"      },
	{ test_nanvix_ipc_mailbox_bad_open,           "Bad Open"          },
	{ test_nanvix_ipc_mailbox_invalid_close,      "Invalid Close"     },
	{ test_nanvix_ipc_mailbox_bad_close,          "Bad Close"         },
	{ test_nanvix_ipc_mailbox_invalid_read,       "Invalid Read"      },
	{ test_nanvix_ipc_mailbox_invalid_read_size,  "Invalid Read Size" },
	{ test_nanvix_ipc_mailbox_null_read,          "Null Read"         },
	{ test_nanvix_ipc_mailbox_invalid_write,      "Invalid Write"     },
	{ test_nanvix_ipc_mailbox_bad_write,          "Bad Write"         },
	{ NULL,                                        NULL               },
};