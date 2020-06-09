#ifndef UTIL_H
#define UTIL_H


#include <stdio.h>


#include "types.h"
#include "bytecode.h"
#include "cpu.h"


#define DEBUG // Enable debug info


/**
* Swap endianness
**/
uint32_t swap_uint32(uint32_t num);


/**
* Translate from op code to an instruction mnemonic
**/
char* op_decode(byte_t op);


/**
* Get a one byte argument from code memory by index in memory
**/
byte_t get_arg_byte(int i);


/**
* Get a two byte argument from code memory by index in memory
**/
short get_arg_short(int i);


/* debug print */
#ifdef DEBUG
#define dprintf(...) \
    fprintf(stderr,  __VA_ARGS__)
#else
#define dprintf(...)
#endif

#endif
