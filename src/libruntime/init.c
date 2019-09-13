/*
 * MIT License
 *
 * Copyright(c) 2011-2019 The Maintainers of Nanvix
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <nanvix/runtime/runtime.h>
#include <nanvix/runtime/stdikc.h>
#include <ulibc/assert.h>
#include <ulibc/stdio.h>
#include <posix/errno.h>

/**
 * @brief Current runtime ring.
 */
static int current_ring = 0;

/**
 * @todo TODO: provide a detailed description for this function.
 */
int __runtime_setup(int ring)
{
	/* Invalid runtime ring. */
	if (ring < 0)
		return (-EINVAL);

	/* Nothing to do. */
	if (ring < current_ring)
		return (0);

	/* Initialize unnamed IKC services. */
	if ((ring - current_ring) >= 0)
	{
		__stdsync_setup();
		__stdmailbox_setup();
		__stdportal_setup();
	}

	/* Initialize Name Service client*/
	if ((ring - current_ring) >= 0)
		__name_setup();

	current_ring = ring;

	return (0);
}

/**
 * @todo TODO: provide a detailed description for this function.
 */
int __runtime_cleanup(void)
{
	__stdportal_cleanup();
	__stdmailbox_cleanup();
	__stdsync_cleanup();

	/* Clean up Name Service client. */
	if (current_ring >= 1)
		__name_cleanup();

	current_ring = 0;

	return (0);
}
