#ifndef LOADER_H
#define LOADER_H

#include <stdlib.h> // For malloc
#include <stdio.h>  // Contains FILE * type

#include "cpu.h"
#include "types.h"
#include "terminate.h"


/**
* Load binary from file into the IJVM CPU
* Returns  1 on success
*          0 on failure
**/
bool load_bin(char* path);

#endif
