#include "ijvm.h"
#include "cpu.h"


/**
* Functions below are used for testing purposes only and not inside the machine
**/


word_t* get_stack(void)
{
    // TODO: return pointer to stack of current frame, not whole stack
    return g_cpu_ptr->stack;
}


int stack_size(void)
{
    // TODO: calculate size based on fp and sp
    return g_cpu_ptr->sp;
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
