#!/bin/sh

RESOLVER=0

CC=clang
CFLAGS="-Wall -Wextra -Werror"
$CC $CFLAGS -c main.c
$CC $CFLAGS -c resolver$RESOLVER.c -o resolver.o
$CC *.o -o resolv
