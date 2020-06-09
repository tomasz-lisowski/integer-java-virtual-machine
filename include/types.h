#ifndef TYPES_H
#define TYPES_H

#include <stdio.h>  /* Contains FILE * type */
#include <stdint.h>  /* contains exact integer types int32_t, uint8_t */
#include <stdbool.h> /* Contains bool type */

typedef uint8_t byte_t; /* raw memory will be typed as uint8 */
typedef int32_t word_t; /* the basic unit of the ijvm will be an int32 */

#endif