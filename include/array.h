#ifndef ARRAY_H
#define ARRAY_H


#include <stdlib.h> // For calloc and malloc
#include <string.h> // For memcpy


#include "types.h"
#include "config.h"
#include "cpu.h"


/**
* Return the ith element from array identified by arr_ref
* Set CPU's error flag on failure.
**/
word_t get_arr_element(word_t arr_ref, word_t i);


/**
* Overwrite the i'th element from array indentified by arr_ref with new_val.
* Set CPU's error flag on failure.
**/
void set_arr_element(word_t arr_ref, word_t i, word_t new_val);


/**
* Control how the new array is created and if it's possible to create it.
* Return  array reference on success
*         0 on failure and set CPU's error flag
**/
word_t start_array_creation(word_t count);


/**
* Free all arrays and all other data used to manage them
**/
void destroy_arrays(void);


/**
* Free unreachable arrays
**/
void gc_arrays(void);


/**
* Print out all active array references
**/
void print_arr_refs(bool compact);


#endif
