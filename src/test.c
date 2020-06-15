#include "test.h"


/**
* Functions below are used for testing purposes only and not inside the machine
**/


word_t* get_stack(void)
{
    return g_cpu->stack;
}


int stack_size(void)
{
    return (g_cpu->sp - g_cpu->fp) + g_cpu->nv;
}


byte_t* get_text(void)
{
    return g_cpu->code_mem;
}


int text_size(void)
{
    return g_cpu->code_mem_size;
}


int get_program_counter(void)
{
    return g_cpu->pc;
}


byte_t get_instruction(void)
{
    return (g_cpu->code_mem)[g_cpu->pc];
}
