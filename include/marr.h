#ifndef MARR_H
#define MARR_H


#include <string.h>


#include "types.h"
#include "terminate.h"
#include "array.h"


/**
* A mapped array is just an array that is kept track of using a map.
* Given the n'th bool in the map is true, 
* then the n'th value is occupied.
* The 'bool map' can therefore be used to more quickly insert/find
* occupied elements in large arrays.
**/
typedef struct MappedArray_t
{
    uint32_t size;
    bool* map;
    uintptr_t* values;
}MArr_t;


/**
* Set the size and allocate memory for map and values.
* All memory cells will be initialized to 0's.
* Size must be strictly less than SIZE_MAX_UINT32_T.
**/
void marr_init(MArr_t* marr, uint32_t size);


/**
* Resize map and values memory to a new size.
* Mapped arrays can get bigger but never smaller.
* Size must be strictly less than SIZE_MAX_UINT32_T.
**/
void marr_resize(MArr_t* marr, uint32_t new_size);


/**
* Return the "claimed" state of an element from the map
**/
bool marr_check_marked(MArr_t* marr, uint32_t val_i);


/**
* Add an entry (no matter where) to the mapped array.
* Return  index claimed for the new entry
*         SIZE_MAX_UINT32_T if a free name was not found
**/
uint32_t marr_add_element(MArr_t* marr, uintptr_t data);


/**
* Update a value in the mapped array.
**/
void marr_set_element(MArr_t* marr, uint32_t val_i, uintptr_t data);


/**
* Return the i'th value from the mapped array.
* If the value requested is unclaimed, crash.
**/
uintptr_t marr_get_element(MArr_t* marr, uint32_t val_i);


/**
* Properly remove an element from the mapped array
**/
void marr_remove_element(MArr_t* marr, uint32_t val_i);


/**
* Clear all memory.
* Caller is responsible for freeing the pointer to the mapped array itself.
**/
void marr_destroy(MArr_t* marr);


/**
* Print out a list of all claimed values: "index:value"
**/
void marr_print(MArr_t* marr);


#endif
