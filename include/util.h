#ifndef UTIL_H
#define UTIL_H

#define DEBUG // Enable debug info

#include <stdio.h>

#include "types.h"
#include "bytecode.h"

/**
* Swap endianness
**/
uint32_t swap_uint32(uint32_t num);


/**
* Translate from op code to an instruction mnemonic
**/
char* op_decode(byte_t op);

/* debug print */
#ifdef DEBUG
#define dprintf(...) \
    fprintf(stderr,  __VA_ARGS__)
#else
#define dprintf(...)
#endif

#endif
