#!/bin/sh

TOP_DIR ?= $(PWD)/..
INCDIR=$(TOP_DIR)/include
CFLAGS += -I$(INCDIR)
CFLAGS += -I$(TOP_DIR)/src
LDFLAGS += -lpthread
CC=gcc

SRCS  = $(TOP_DIR)/src/wave/wave.c
SRCS += $(TOP_DIR)/pretest/winfo.c
SRCS += $(TOP_DIR)/src/utils/clinkedlist.c
SRCS += $(TOP_DIR)/pretest/cltest.c

UTILS = winfo cltest wqtest

all: $(UTILS)

winfo: winfo.o wave.o
	$(CC) -o $@ $^ $(LDFLAGS)

wave.o: $(TOP_DIR)/src/wave/wave.c
	$(CC) -c $< $(CFLAGS)

winfo.o: $(TOP_DIR)/pretest/winfo.c
	$(CC) -c $< $(CFLAGS)

cltest: clinkedlist.o cltest.o
	$(CC) -o $@ $^ $(LDFLAGS)

clinkedlist.o: $(TOP_DIR)/src/utils/clinkedlist.c
	$(CC) -c $< $(CFLAGS)

cltest.o: $(TOP_DIR)/pretest/cltest.c
	$(CC) -c $< $(CFLAGS)

wqtest: clinkedlist.o workqueue.o wqtest.o
	$(CC) -o $@ $^ $(LDFLAGS)

workqueue.o: $(TOP_DIR)/src/workqueue.c
	$(CC) -c $< $(CFLAGS)

wqtest.o: $(TOP_DIR)/pretest/wqtest.c
	$(CC) -c $< $(CFLAGS)
clean:
	rm -f $(UTILS) *.o
