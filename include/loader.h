#ifndef LOADER_H
#define LOADER_H

#include <stdlib.h> /* For malloc */
#include <stdio.h>  /* Contains FILE * type */
#include <stdbool.h> /* Contains bool type */
#include <endian.h> /* Endianness conversions */

#include "cpu.h"


/**
* Load binary from file into the IJVM CPU
* Returns  1 on success
*          0 on failure
**/
bool load_bin(char* path, CPU_t* cpu);

#endif