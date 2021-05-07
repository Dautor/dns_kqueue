#!/bin/sh

CC=clang
CFLAGS="-Wall -Wextra -Werror"
$CC $CFLAGS -c main.c
$CC $CFLAGS -c resolver.c
$CC *.o -o a
