#!/bin/sh

TOP_DIR=$(PWD)
INCDIR=$(TOP_DIR)/include
CFLAGS = -I$(INCDIR)

SRCS  = $(TOP_DIR)/src/wave/wave.c
SRCS += $(TOP_DIR)/src/utils/winfo.c

UTILS = winfo

all: $(UTILS)

winfo: winfo.o wave.o
	gcc -o winfo winfo.o wave.o

wave.o: $(TOP_DIR)/src/wave/wave.c
	gcc -c $(TOP_DIR)/src/wave/wave.c $(CFLAGS)

winfo.o: $(TOP_DIR)/src/utils/winfo.c
	gcc -c $(TOP_DIR)/src/utils/winfo.c $(CFLAGS)

clean:
	rm -f $(UTILS) *.o
