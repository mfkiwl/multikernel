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

#include <sys/types.h>
#include <errno.h>

#include <nanvix/mm.h>

/**
 * @brief Truncates a file to a specified length.
 *
 * @param fd     Target file descriptor.
 * @param length File length (in bytes).
 *
 * @returns Upon successful completion, zero is returned. Upon
 * failure, -1 is returned instead, and errno is set to indicate the
 * error.
 */
int nanvix_ftruncate(int fd, off_t length)
{
	/* Invalid file descriptor. */
	if (fd < 0)
	{
		errno = EINVAL;
		return (-1);
	}

	/* Invalid length. */
	if (length <= 0)
	{
		errno = EINVAL;
		return (-1);
	}

	return (nanvix_mtruncate(fd, length));
}

