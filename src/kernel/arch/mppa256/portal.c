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

#include <nanvix/name.h>

#include <nanvix/klib.h>
#include <nanvix/hal.h>
#include <nanvix/arch/mppa.h>
#include <errno.h>
#include <inttypes.h>
#include <string.h>
#include <stdio.h>

#include "mppa.h"

/*=======================================================================*
 * hal_portal_create()                                                   *
 *=======================================================================*/

/**
 * @brief Creates a portal.
 *
 * @param portal    Adress where the portal will be stored.
 * @param local     ID of the local NoC node.
 *
 * @returns Upon successful completion, zero is returned.
 * Upon failure, a negative error code is returned instead.
 */
int hal_portal_create(portal_t *portal, int local)
{
	int fd;               /* Portal NoC Connector.       */
	char pathname[128];   /* NoC connector name.         */
	int noctag;           /* NoC tag used for transfers. */
	int remote;           /* ID of the remote NoC node.  */

	/* Invalid portal adress */
	if (portal == NULL)
		return (-EINVAL);

	remote = hal_get_node_id();

	/* Invalid node ID. */
	if (local != remote)
		return (-EINVAL);

	noctag = noctag_portal(local);

	/* Build pathname for NoC connector. */
	snprintf(pathname,
			ARRAY_LENGTH(pathname),
			"/mppa/portal/%d:%d",
			local,
			noctag
	);

	/* Open NoC connector. */
	if ((fd = mppa_open(pathname, O_RDONLY)) == -1)
		return (-EAGAIN);

	portal->portal_fd = fd;
	portal->sync_fd = -1;
	portal->remote = -1;
	portal->local = local;

	return (0);
}

/*=======================================================================*
 * hal_portal_allow()                                                    *
 *=======================================================================*/

/**
 * @brief Enables read operations from a remote.
 *
 * @param portal    Adress of the target portal.
 * @param remote    NoC node ID of target remote.
 *
 * @returns Upons successful completion zero is returned. Upon failure,
 * a negative error code is returned instead.
 */
int hal_portal_allow(portal_t *portal, int remote)
{
	int local;          /* Local NoC node ID.   */
	int sync_fd;        /* Sync NoC connector.  */
	char pathname[128]; /* Portal pathname.     */

	/* Invalid portal.*/
	if (portal == NULL)
		return (-EINVAL);

	/* Invalid remote. */
	if (!(noc_is_ionode(remote) || noc_is_cnode(remote)))
		return (-EINVAL);

	local = hal_get_node_id();

	/* Invalid remote. */
	if (remote == local)
		return (-EINVAL);

	/* Create underlying sync. */
	snprintf(pathname,
			ARRAY_LENGTH(pathname),
			"/mppa/sync/%d:%d",
			remote,
			(k1_is_ccluster(remote) || k1_is_ccluster(local)) ?
							 noctag_portal(portal->local) : 127
	);

	if ((sync_fd = mppa_open(pathname, O_WRONLY)) == -1)
		return (-EAGAIN);

	/* Initialize portal. */
	portal->remote = remote;
	portal->sync_fd = sync_fd;

	return (0);
}

/*=======================================================================*
 * hal_portal_open()                                                     *
 *=======================================================================*/

/**
 * @brief Opens a portal.
 *
 * @param portal     Adress where the portal will be stored
 * @param remote     ID of the target NoC node.
 *
 * @returns Upon successful completion zero is returned.
 * Upon failure, a negative error code is returned instead.
 */
int hal_portal_open(portal_t *portal, int remote)
{
	int local;          /* ID of local NoC node.  */
	int portal_fd;      /* Portal NoC Connector.  */
	int sync_fd;        /* Sync NoC connector.    */
	char pathname[128]; /* NoC connector name.    */

	/* Invalid portal. */
	if (portal == NULL)
		return (-EINVAL);

	/* Invalid node ID. */
	if (remote < 0)
		return (-EINVAL);

	local = hal_get_node_id();

	/* Invalid remote. */
	if (remote == local)
		return (-EINVAL);

	/* Build pathname for NoC connector. */
	snprintf(pathname,
			ARRAY_LENGTH(pathname),
			"/mppa/portal/%d:%d",
			remote,
			noctag_portal(remote)
	);

	/* Open NoC connector. */
	if ((portal_fd = mppa_open(pathname, O_WRONLY)) == -1)
		return (-EAGAIN);

	/* Build pathname for NoC connector. */
	snprintf(pathname,
			ARRAY_LENGTH(pathname),
			"/mppa/sync/%d:%d",
			local,
			(k1_is_ccluster(remote) || k1_is_ccluster(local)) ?
									noctag_portal(remote) : 127
	);

	/* Open NoC connector. */
	if ((sync_fd = mppa_open(pathname, O_RDONLY)) == -1)
		return (-EAGAIN);

	portal->portal_fd = portal_fd;
	portal->sync_fd = sync_fd;
	portal->remote = remote;
	portal->local = local;

	return (0);
}

/*=======================================================================*
 * portal_sync()                                                         *
 *=======================================================================*/

/**
 * @brief Builds sync mask for a nodeid.
 *
 * @param nodeid Target nodeid.
 *
 * @return Sync mask.
 */
static inline uint64_t portal_sync(int nodeid)
{
	if ((nodeid >= IOCLUSTER0) && (nodeid < IOCLUSTER0 + NR_IOCLUSTER_DMA))
		return (1 << (CCLUSTER15 + 1 + nodeid%NR_IOCLUSTER_DMA));
	else if ((nodeid >= IOCLUSTER1) && (nodeid < IOCLUSTER1 + NR_IOCLUSTER_DMA))
		return (1 << (CCLUSTER15 + 1 + NR_IOCLUSTER_DMA +
								nodeid%NR_IOCLUSTER_DMA));
	return (1 << nodeid);
}

/*=======================================================================*
 * hal_portal_read()                                                     *
 *=======================================================================*/

/**
 * @brief Reads data from a portal.
 *
 * @param portal  Targeted adress portal.
 * @param buf     Location from where data should be written.
 * @param n       Number of bytes to read.
 *
 * @returns Upon successful completion zero is returned. Upon failure, a
 * negative error code is returned instead.
 */
int hal_portal_read(portal_t *portal, void *buf, size_t n)
{
	uint64_t mask;
	mppa_aiocb_t aiocb;
	size_t nread;

	/* Invalid portal ID.*/
	if (portal == NULL)
		return (-EINVAL);

	/* Invalid buffer. */
	if (buf == NULL)
		return (-EINVAL);

	/* Invalid read size. */
	if (n < 1)
		return (-EINVAL);

	/* Setup read operation. */
	mppa_aiocb_ctor(&aiocb, portal->portal_fd, buf, n);
	if (mppa_aio_read(&aiocb) == -1)
		return (-EINVAL);

	/* Unblock remote. */
	mask = portal_sync(portal->local);
	if (mppa_write(portal->sync_fd, &mask, sizeof(uint64_t)) == -1)
		return (-EAGAIN);

	/* Wait read operation to complete. */
	nread = mppa_aio_wait(&aiocb);
	mppa_close(portal->sync_fd);

	return (nread);
}

/*=======================================================================*
 * hal_portal_write()                                                    *
 *=======================================================================*/

/**
 * @brief Writes data to a portal.
 *
 * @param portal  Targeted adress portal.
 * @param buf     Location from where data should be read.
 * @param n       Number of bytes to write.
 *
 * @returns Upon successful the number of bytes wrote is returned.
 * Upon failure, a negative error code is returned instead.
 */
int hal_portal_write(portal_t *portal, const void *buf, size_t n)
{
	uint64_t mask;
	size_t nwrite;

	/* Invalid portal.*/
	if (portal == NULL)
		return (-EINVAL);

	/* Invalid buffer. */
	if (buf == NULL)
		return (-EINVAL);

	/* Invalid write size. */
	if (n < 1)
		return (-EINVAL);

	/* Wait for remote to be ready. */
	mask = portal_sync(portal->remote);
	if (mppa_ioctl(portal->sync_fd, MPPA_RX_SET_MATCH, ~mask) == -1)
		return (-EINVAL);

	if (mppa_read(portal->sync_fd, &mask, sizeof(uint64_t)) == -1)
		return (-EAGAIN);

	/* Write. */
	nwrite = mppa_pwrite(portal->portal_fd, buf, n, 0);

	return (nwrite);
}

/*=======================================================================*
 * hal_portal_close()                                                    *
 *=======================================================================*/

/**
 * @brief Closes a portal.
 *
 * @param portal     Adress of the target portal.
 *
 * @returns Upon successful completion zero is returned. Upon failure, a
 * negative error code is returned instead.
 */
int hal_portal_close(portal_t *portal)
{
	/* Invalid portal.*/
	if (portal == NULL)
		return (-EINVAL);

	if (portal->portal_fd != -1)
		mppa_close(portal->portal_fd);

	if (portal->sync_fd != -1)
		mppa_close(portal->sync_fd);

	portal->remote = -1;
	portal->local = -1;

	return (0);
}

/*=======================================================================*
 * portal_unlink()                                                       *
 *=======================================================================*/

/**
 * @brief Destroys a portal.
 *
 * @param portalid ID of the target portal.
 *
 * @returns Upon successful completion zero is returned. Upon failure, a
 * negative error code is returned instead.
 */
int hal_portal_unlink(portal_t *portal)
{
	/*  Invalid portal. */
	if (portal == NULL)
		return (-EINVAL);

	if (portal->portal_fd != -1)
		mppa_close(portal->portal_fd);

	portal->sync_fd = -1;
	portal->remote = -1;
	portal->local = -1;

	return (0);
}