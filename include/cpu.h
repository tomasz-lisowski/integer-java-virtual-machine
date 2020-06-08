#ifndef CPU_H
#define CPU_H

#include "types.h"
#include "util.h"

typedef struct CPU_t
{
	int const_mem_size;
	int code_mem_size;
	int stack_size;
    int local_var_mem_size;

	word_t* const_mem;
	byte_t* code_mem;
	word_t* stack;
    word_t* local_var_mem;

	int sp;
	int pc;
	int fp;

    int blv; // Pointer to first element in local variable memory
    int tlv; // Pointer to last element in local variable memory

    bool error_flag;
    bool halt_flag;
}CPU_t;


extern CPU_t* g_cpu_ptr; // One CPU shared across the entire machine


/**
 * Returns the currently loaded program text as a byte array
 **/
byte_t* get_text(void);


/**
 * Returns the size of the currently loaded program text
 **/
int text_size(void);


/**
 * Returns the value of the program counter (as an integer offset from the first instruction).
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
 * Pushes element on top of stack
 * Returns  1 on success
 *          0 on failure
 **/
bool stack_push(word_t e);


/**
 * Returns top element of the stack and decreases stack pointer
 **/
word_t stack_pop(void);


/**
 * @param i, index of variable to obtain.
 * @return Returns the i:th local variable of the current frame.
 **/
word_t get_local_variable(int i);


/**
 * Update the value of i'th local variable in current frame (or create one if it does not exist)
 **/
void update_local_variable(word_t new_val, int i);


/**
* Print out a complete state of a CPU
**/
void print_cpu_state(CPU_t* cpu, bool compact);


/**
* Print out CPU memory block sizes
**/
void print_cpu_mem_size(CPU_t* cpu, bool compact);


/**
* Print out data memory contents
**/
void print_cpu_const_mem(CPU_t* cpu, bool compact);


/**
* Print out code memory contents
**/
void print_cpu_code_mem(CPU_t* cpu, bool compact);


/**
* Print out CPU register contents
**/
void print_cpu_registers(CPU_t* cpu, bool compact);


/**
* Print out stack contents
**/
void print_cpu_stack(CPU_t* cpu, bool compact);


/**
* Print out all local variables
**/
void print_cpu_local_vars(CPU_t* cpu, bool compact);


/**
* Print out local variables in current local frame
**/
void print_cpu_local_vars_current_frame(CPU_t* cpu, bool compact);

#endif