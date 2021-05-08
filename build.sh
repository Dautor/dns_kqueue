#!/bin/sh

CC=clang
CFLAGS="-Wall -Wextra -Werror"
$CC $CFLAGS -c main.c
$CC $CFLAGS -c resolver_simple.c
$CC *.o -o a
