#ifndef UTIL_H
#define UTIL_H


#include <stdio.h>
#include <string.h>


#include "types.h"
#include "bytecode.h"
#include "cpu.h"
#include "array.h"
#include "terminate.h"


/**
* Returns the i'th byte, from code memory
**/
byte_t get_code_byte(const int i);


/**
* Returns a short starting at i'th byte, from code memory
**/
short get_code_short(const int i);


/**
* Swap endianness
**/
uint32_t swap_uint32(const uint32_t num);


/**
* Returns base^power where both base and power are positive integers
**/
uint64_t power(uint32_t base, const uint32_t power);


/**
* Duplicate source string and return a pointer to it.
* Return  Pointer to duplicated string on success
*         NULL on failure
**/
char* str_dup(const char* src);


/**
* Translate from op code to an instruction mnemonic
**/
const char* op_decode(const byte_t op);


/**
* Functions to print out CPU state
**/
void print_cpu_state(const bool compact);
void print_cpu_mem_size(const bool compact);
void print_cpu_const_mem(const bool compact);
void print_cpu_code_mem(const bool compact);
void print_cpu_registers(const bool compact);
void print_cpu_stack(const bool compact); // For current frame
void print_cpu_local_vars(const bool compact); // For current frame


/* Debug print */
#ifdef DEBUG
#define dprintf(...) \
    fprintf(stderr,  __VA_ARGS__)
#else
#define dprintf(...)
#endif


#endif
