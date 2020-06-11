#include "test.h"


/**
* Functions below are used for testing purposes only and not inside the machine
**/


word_t* get_stack(void)
{
    return g_cpu_ptr->stack;
}


int stack_size(void)
{
    return (g_cpu_ptr->sp - g_cpu_ptr->fp) + g_cpu_ptr->nv;
}


byte_t* get_text(void)
{
    return g_cpu_ptr->code_mem;
}


int text_size(void)
{
    return g_cpu_ptr->code_mem_size;
}


int get_program_counter(void)
{
    return g_cpu_ptr->pc;
}


byte_t get_instruction(void)
{
    return (g_cpu_ptr->code_mem)[g_cpu_ptr->pc];
}
