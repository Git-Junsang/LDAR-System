# SPDX-License-Identifier: Apache-2.0

CC      ?= gcc
CFLAGS  ?= -O2 -Wall -Wextra -Wpedantic -std=c11
LDFLAGS ?=

TARGETS := ldar_listener

.PHONY: all clean
all: $(TARGETS)

ldar_listener: ldar_listener.c ../shared/ldar_ipc_proto.h
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f $(TARGETS)
