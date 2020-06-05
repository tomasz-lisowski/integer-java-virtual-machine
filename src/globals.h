#include <ijvm.h>

#ifndef GLOBALS_H
#define GLOBALS_H


typedef struct BIN_t
{
    uint32_t magic_number;
    uint32_t const_pool_origin;
    int const_pool_size;
    word_t* const_pool;


    uint32_t text_origin;
    int text_size;
    byte_t* text;
}BIN_t;


typedef struct STACK_t
{
    uint32_t top;
    int size;
    word_t* stack;
}STACK_t;


extern BIN_t b;

#endif