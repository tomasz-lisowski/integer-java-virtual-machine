#!/bin/bash

export CC=afl-clang
export AFL_HARDEN=1
export AFL_INST_RATIO=100
./build.sh
