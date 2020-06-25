#ifndef ARRAY_H
#define ARRAY_H


#include <stdlib.h> // calloc


#include "types.h"
#include "config.h"
#include "cpu.h"
#include "terminate.h"


/**
* Create a new array.
* Return  array reference on success
*         0 on failure
**/
word_t arr_create(word_t count);


/**
* Get a value from the array
**/
word_t arr_get(word_t arr_ref, word_t i);


/**
* Change value inside array
**/
void arr_set(word_t arr_ref, word_t i, word_t val);


/**
* Free all arrays and related data
**/
void arr_destroy(void);


/**
* Free unreachable arrays and return the number of arrays that were free'd
**/
uint32_t arr_gc(void);


/**
* Print out all active array references
**/
void arr_print(bool compact);


#endif