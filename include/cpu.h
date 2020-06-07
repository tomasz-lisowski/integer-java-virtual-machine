#ifndef CPU_H
#define CPU_H

#include "types.h"
#include "util.h"

typedef struct CPU_t
{
	int data_mem_size;
	int code_mem_size;
	int stack_size;

	word_t* data_mem;
	byte_t* code_mem;
	word_t* stack;

	int sp;
	int fp;
	int ip;
}CPU_t;


extern CPU_t* g_cpu_ptr; // One CPU shared across the entire machine


/**
 * Returns the currently loaded program text as a byte array.
 **/
byte_t* get_text(void);


/**
 * Returns the size of the currently loaded program text.
 **/
int text_size(void);


/**
 * Returns the value of the program counter (as an offset from the first instruction).
 **/
int get_program_counter(void);


/**
 * @param i, index of the constant to obtain
 * @return The constant at location i in the constant pool.
 **/
word_t get_constant(int i);


/**
 * @return The value of the current instruction represented as a byte_t.
 *
 * This should NOT increase the program counter.
 **/
byte_t get_instruction(void);


/**
 * This function should return the word at the top of the stack of the current
 * frame, interpreted as a signed integer.
 **/
word_t tos(void);


/**
 * Returns the stack of the current frame as an array of integers,
 * with entry[0] being the very bottom of the stack and
 * entry[stack_size() - 1] being the top.
 **/
word_t* get_stack(void);


/**
 * Returns the size of the stack of the current frame.
 **/
int stack_size(void);


/**
 * @param i, index of variable to obtain.
 * @return Returns the i:th local variable of the current frame.
 **/
word_t get_local_variable(int i);


/**
* Print out a complete state of a CPU
**/
void print_cpu_state(CPU_t* cpu);

#endif