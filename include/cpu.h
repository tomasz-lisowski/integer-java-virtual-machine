#ifndef CPU_H
#define CPU_H


#include <stdlib.h>


#include "types.h"
#include "util.h"
#include "array.h"


typedef struct CPU_t
{
	// Sizes are in "elements" units
	int const_mem_size;
	int code_mem_size;
	int stack_size;
	
	word_t* const_mem;
	byte_t* code_mem;
	word_t* stack;

	int pc;
	int sp;
    int fp;
    int lv;
    int nv; // Number of vars in current frame (number of arguments + local variables)

    bool error_flag;
    bool halt_flag;
}CPU_t;


extern CPU_t* restrict g_cpu; // One CPU shared across the entire machine
extern FILE* restrict g_out_file;
extern FILE* restrict g_in_file;


/**
* Return the word at the top of the stack
**/
word_t tos(void);


/**
* Pushes element on top of stack
* Returns  true on success
*          false on failure
**/
bool stack_push(word_t e);


/**
* Returns top element of the stack and decreases stack pointer
**/
word_t stack_pop(void);


/**
* Returns the i'th constant from constant memory
**/
word_t get_constant(int i);


/**
* Return the i'th variable in the current frame
**/
word_t get_local_variable(int i);


/**
* Update the value of i'th local variable in current frame (or create one if it does not exist)
**/
void update_local_variable(word_t new_val, int i);


/**
* Perform a jump in code memory (from current PC) by a given offset
**/
void jump(int32_t offset);


/**
* Remove all data associated with the CPU
**/
void destroy_cpu(void);


#endif
