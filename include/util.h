#ifndef UTIL_H
#define UTIL_H

#define DEBUG // Enable debug info

#include <stdio.h>
#include "types.h"

uint32_t swap_uint32(uint32_t num);

/* debug print */
#ifdef DEBUG
#define dprintf(...) \
    fprintf(stderr,  __VA_ARGS__)
#else
#define dprintf(...)
#endif

#endif
