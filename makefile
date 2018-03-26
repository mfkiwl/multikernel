# Copyright(C) 2011-2018 Pedro H. Penna <pedrohenriquepenna@gmail.com>
# 
# This file is part of Nanvix.
# 
# Nanvix is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
# 
# Nanvix is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with Nanvix. If not, see <http://www.gnu.org/licenses/>.
#

K1_TOOLCHAIN_DIR=/usr/local/k1tools/

# Directories.
BINDIR  = $(CURDIR)/bin
INCDIR  = $(CURDIR)/include
SRCDIR  = $(CURDIR)/src
TESTDIR = $(CURDIR)/test
BENCHDIR = $(CURDIR)/benchmark


# Toolchain Configuration
cflags := -ansi -std=c99
cflags += -Wall -Wextra -Werror
cflags += -Winit-self -Wswitch-default -Wfloat-equal -Wundef -Wshadow -Wuninitialized
cflags += -O3 
cflags += -I $(INCDIR)
cflags += -D_KALRAY_MPPA256_
lflags := -Wl,--defsym=_LIBNOC_DISABLE_FIFO_FULL_CHECK=0

#=============================================================================
# Async Latency Benchmark
#=============================================================================


io-bin += master.elf
master.elf-srcs := $(BENCHDIR)/async-latency/master.c
master.elf-system := bare
master.elf-lflags := -mhypervisor -lutask -lmppa_async -lmppa_request_engine
master.elf-lflags += -lmppapower -lmppanoc -lmpparouting
master.elf-lflags += -lpcie_queue

cluster-bin += slave.elf
slave.elf-srcs := $(BENCHDIR)/async-latency/slave.c
slave.elf-system := bare
slave.elf-lflags := -mhypervisor -lutask -lmppa_async -lmppa_request_engine
slave.elf-lflags += -lmppapower -lmppanoc -lmpparouting
slave.elf-lflags += -Wl,--defsym=USER_STACK_SIZE=0x2000 -Wl,--defsym=KSTACK_SIZE=0x1000

async-latency-objs := master.elf slave.elf
async-latency-name := async-latency.img

#=============================================================================
# Portal Latency Benchmark
#=============================================================================

io-bin += portal-latency-master
portal-latency-master-srcs := $(BENCHDIR)/portal-latency/master.c 
portal-latency-master-lflags := -lmppaipc

cluster-bin += portal-latency-slave
portal-latency-slave-srcs := $(BENCHDIR)/portal-latency/slave.c
portal-latency-slave-lflags := -lmppaipc

portal-latency-objs := portal-latency-master portal-latency-slave
portal-latency-name := portal-latency.img

#=============================================================================
# Compute Cluster Binaries
#=============================================================================

#master.test-srcs := $(TESTDIR)/master.c          \
#					$(SRCDIR)/kernel/sys/timer.c \
#					$(SRCDIR)/kernel/arch/mppa/barrier.c \

#rmem-server.test-srcs := $(SRCDIR)/kernel/arch/mppa/mailbox.c \
#						 $(SRCDIR)/kernel/arch/mppa/portal.c  \
#						 $(SRCDIR)/kernel/arch/mppa/barrier.c \
#						 $(SRCDIR)/kernel/arch/mppa/name.c    \
#						 $(SRCDIR)/kernel/sys/timer.c         \
#						 $(SRCDIR)/kernel/sys/meminit.c       \
#						 $(SRCDIR)/kernel/sys/memwrite.c      \
#						 $(SRCDIR)/kernel/sys/memread.c       \
#						 $(SRCDIR)/servers/rmem.c

#rmem-srcs := $(SRCDIR)/kernel/arch/mppa/mailbox.c \
#			 $(SRCDIR)/kernel/arch/mppa/portal.c  \
#			 $(SRCDIR)/kernel/arch/mppa/barrier.c \
#			 $(SRCDIR)/kernel/arch/mppa/name.c    \
#			 $(SRCDIR)/kernel/sys/timer.c         \
#			 $(SRCDIR)/kernel/sys/meminit.c       \
#			 $(SRCDIR)/kernel/sys/memwrite.c      \
#			 $(SRCDIR)/kernel/sys/memread.c       \
#			 $(TESTDIR)/rmem/rmem.c

#=============================================================================
# Testing Binary
#=============================================================================

#test-objs := master.test      \
#			 rmem-server.test \
#			 rmem 

#test-name := test.img

#=============================================================================
# MPPA Binary
#=============================================================================

mppa-bin := portal-latency async-latency

include $(K1_TOOLCHAIN_DIR)/share/make/Makefile.kalray
