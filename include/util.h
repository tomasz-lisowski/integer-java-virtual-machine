#ifndef UTIL_H
#define UTIL_H


#include <stdio.h>


#include "types.h"
#include "bytecode.h"
#include "cpu.h"


/**
* Swap endianness
**/
uint32_t swap_uint32(uint32_t num);


/**
* Returns base^power where both base and power are positive integers
**/
uint64_t power(uint32_t base, uint32_t power);


/**
* Translate from op code to an instruction mnemonic
**/
const char* op_decode(byte_t op);


/**
* Functions to print out CPU state
**/
void print_cpu_state(bool compact);
void print_cpu_mem_size(bool compact);
void print_cpu_const_mem(bool compact);
void print_cpu_code_mem(bool compact);
void print_cpu_registers(bool compact);
void print_cpu_stack(bool compact); // For current frame
void print_cpu_local_vars(bool compact); // For current frame


/* Debug print */
#ifdef DEBUG
#define dprintf(...) \
    fprintf(stderr,  __VA_ARGS__)
#else
#define dprintf(...)
#endif


#endif
