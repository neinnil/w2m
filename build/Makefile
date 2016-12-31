#!/bin/sh

TOP_DIR ?= $(PWD)/..

CC=gcc
KERNEL = $(shell uname -s)

INCDIR=$(TOP_DIR)/include
EXTINC=$(TOP_DIR)/ext_lib/include

CFLAGS += -D_POSIX
CFLAGS += -I$(INCDIR)
CFLAGS += -I$(EXTINC)
CFLAGS += -I$(TOP_DIR)/src
CFLAGS += -O3
CFLAGS += -pipe
CFLAGS += -funroll-loops
#LDFLAGS += -static-libgcc -static

ifeq ($(findstring MINGW, $(KERNEL)),MINGW)
LDFLAGS += -Wl,-Bstatic -lpthread
else
LDFLAGS += -lpthread
endif

LDFLAGS += -L$(TOP_DIR)/ext_lib/lib -Wl,-Bstatic -lmp3lame
ifneq ($(OS),Msys)
LDFLAGS += -Wl,-Bdynamic -lm
endif

VPATH += $(TOP_DIR)/src
SRCS += getname.c
SRCS += jobitem.c
SRCS += w2m.c
SRCS += workqueue.c

VPATH += $(TOP_DIR)/src/osa
SRCS += osa.c

VPATH += $(TOP_DIR)/src/wave
SRCS += wave.c
SRCS += pcm.c

VPATH += $(TOP_DIR)/src/utils
SRCS += clinkedlist.c

VPATH += $(TOP_DIR)/pretest
SRCS += winfo.c
SRCS += cltest.c

UTILS = winfo wqtest cltest

all: w2m $(UTILS)

w2m: w2m.o jobitem.o workqueue.o clinkedlist.o getname.o osa.o pcm.o wave.o
	$(CC) -o $@ $^ $(LDFLAGS)

winfo: winfo.o wave.o getname.o
	$(CC) -o $@ $^ $(LDFLAGS)

cltest: cltest.o clinkedlist.o
	$(CC) -o $@ $^ $(LDFLAGS)

wqtest: clinkedlist.o workqueue.o wqtest.o
	$(CC) -o $@ $^ $(LDFLAGS)


clean:
	rm -f w2m $(UTILS) *.o

