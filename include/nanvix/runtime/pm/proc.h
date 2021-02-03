/*
 * MIT License
 *
 * Copyright(c) 2011-2020 The Maintainers of Nanvix
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


#ifndef NANVIX_RUNTIME_PM_PROC_H_
#define NANVIX_RUNTIME_PM_PROC_H_

	#define PROC_TABLE_LENGHT 64
	#define GROUP_TABLE_LENGHT 64

	/*
	 * @brief Process group
	 */
	struct group
	{
		pid_t gid;
	};

	/*
	 * @brief Process
	 */
	struct proc
	{
		pid_t pid;
		struct group group;
	};

	/**
	 * @brief Initializes processes table
	 */
	extern void proc_table_init(void);

	/**
	 * @brief Initializes process groups table
	 */
	extern void group_table_init(void);

	/**
	 * @brief Processes table
	 * Temporary function
	 */
	extern void nanvix_setpid(pid_t pid);

	/**
	 * @brief Returns process id
	 */
	extern pid_t nanvix_getpid(void);

	/**
	 * @brief Returns process group id
	 */
	extern pid_t nanvix_getpgid(pid_t pid);

	/**
	 * @brief Sets a process group id
	 *
	 * @param pid Target process id. If pid equals zero, the calling process id is used
	 * @param pgid Target process group id. If pid equals zero, a new group is
	 * created
	 *
	 * @return Upon successful completion, zero is returned. Upon
	 * failure, a negative error code is returned instead.
	 */
	extern int nanvix_setpgid(pid_t pid, pid_t pgid);

#endif /* NANVIX_RUNTIME_PM_PROC_H_ */
