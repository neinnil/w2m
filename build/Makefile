#!/bin/sh

TOP_DIR ?= $(PWD)/..
INCDIR=$(TOP_DIR)/include
CFLAGS = -I$(INCDIR)
LDFLAGS = -lpthread

SRCS  = $(TOP_DIR)/src/wave/wave.c
SRCS += $(TOP_DIR)/src/utils/winfo.c
SRCS += $(TOP_DIR)/src/osa/clinkedlist.c
SRCS += $(TOP_DIR)/pretest/cltest.c

UTILS = winfo cltest workqueue

all: $(UTILS)

winfo: winfo.o wave.o
	gcc -o winfo winfo.o wave.o

wave.o: $(TOP_DIR)/src/wave/wave.c
	gcc -c $(TOP_DIR)/src/wave/wave.c $(CFLAGS)

winfo.o: $(TOP_DIR)/src/utils/winfo.c
	gcc -c $(TOP_DIR)/src/utils/winfo.c $(CFLAGS)

cltest: clinkedlist.o cltest.o
	gcc  -o cltest cltest.o clinkedlist.o

clinkedlist.o: $(TOP_DIR)/src/osa/clinkedlist.c
	gcc -c $(TOP_DIR)/src/osa/clinkedlist.c $(CFLAGS)

cltest.o: $(TOP_DIR)/pretest/cltest.c
	gcc -c $< $(CFLAGS)

workqueue: clinkedlist.o workqueue.o
	gcc  -o $@ $^ $(LDFLAGS)

workqueue.o: $(TOP_DIR)/pretest/workqueue.c
	gcc -c $< $(CFLAGS)
clean:
	rm -f $(UTILS) *.o
