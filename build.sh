#!/bin/bash
shopt -s extglob

mkdir -p build
pushd build
SRC=$(ls -t ../src/*.c)
CFLAGS+=("-Wall -std=c11 -I ../include -l readline")

$CC \
	$CFLAGS \
	$SRC \
	-c

OBJ_IJVM=$(ls -t !(ijdb).o)
OBJ_IJDB=$(ls -t !(main).o)

$CC \
	$CFLAGS \
	$OBJ_IJVM \
	-o ijvm.bin

$CC \
	$CFLAGS \
	$OBJ_IJDB \
	-o ijdb.bin
popd
