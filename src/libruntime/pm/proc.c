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


#include <nanvix/runtime/pm.h>

/**
 * @brief Processes table
 */
static struct proc proc_table[PROC_TABLE_LENGHT];

/**
 * @brief Process groups table
 */
static struct group group_table[GROUP_TABLE_LENGHT];

/*
 * @brief Running process
 */
static struct proc proc;

/**
 * @brief Initializes processes table
 */
void proc_table_init(void)
{
	for (int i = 0; i < PROC_TABLE_LENGHT; i++)
	{
		proc_table[i].pid = -1;
		proc_table[i].group.gid = -1;
	}
	proc_table[0].pid = 1;
	proc_table[0].group.gid = 1;
	proc = proc_table[0];
}

/**
 * @brief Initializes process groups table
 */
void group_table_init(void)
{
	for (int i = 0; i < GROUP_TABLE_LENGHT; i++)
		group_table[i].gid = -1;
	group_table[0].gid = 1;
}

/**
 * @brief Processes table
 * Temporary function
 */
void nanvix_setpid(pid_t pid)
{
	for (int i = 0; i < PROC_TABLE_LENGHT; i++)
	{
		if (proc_table[i].pid == -1)
		{
			proc_table[i].pid = pid;
			proc = proc_table[i];
			break;
		}
	}
}

/**
 * @brief Returns process id
 */
pid_t nanvix_getpid(void)
{
	return (proc.pid);
}

/**
 * @brief Returns process group id
 */
pid_t nanvix_getpgid(pid_t pid)
{
	for (int i = 0; i < PROC_TABLE_LENGHT; i++)
	{
		if (proc_table[i].pid == pid)
			return (proc_table[i].group.gid);
	}
	return (-EINVAL);
}

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
int nanvix_setpgid(pid_t pid, pid_t pgid)
{
	struct group group;
	int new = 0;

	/* invalid pgid */
	if (pgid < 0)
		return (-EINVAL);

	for (int i = 0; i < GROUP_TABLE_LENGHT; i++)
	{
		if (group_table[i].gid == -1)
		{
			if (pgid && (pid != pgid))
				return (-EPERM);
			else
			{
				new = i;
				break;
			}
		}
		else if (group_table[i].gid == pgid)
		{
			group = group_table[i];
			break;
		}
	}

	/* pid becomes the calling process id */
	if (!pid)
		pid = nanvix_getpid();

	for (int i = 0; i < PROC_TABLE_LENGHT; i++)
	{
		if (proc_table[i].pid == pid)
		{
			if (new)
			{
				group.gid = pid;
				group_table[new] = group;
			}
			proc_table[i].group = group;
			return (0);
		}
	}
	return (-ESRCH);
}
