#include "cpu.h"


static CPU_t vm_cpu;
CPU_t* g_cpu_ptr = &vm_cpu;


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
    return g_cpu_ptr->ip;
}


word_t get_constant(int i)
{
    return (g_cpu_ptr->data_mem)[i];
}


byte_t get_instruction(void)
{
    return (g_cpu_ptr->code_mem)[g_cpu_ptr->ip];
}


word_t tos(void)
{

}


word_t* get_stack(void)
{

}


int stack_size(void)
{

}


word_t get_local_variable(int i)
{

}


void print_cpu_state(CPU_t* cpu)
{
    dprintf("[CPU DUMP]\n");
    dprintf("\tConstant Pool Size: %i\n", cpu->data_mem_size);
    dprintf("\tText Size: %i\n", cpu->code_mem_size);
    dprintf("\tStack Size: %i\n", cpu->stack_size);

    dprintf("Regs\n");
    dprintf("\tIP: %i\n", cpu->ip);
    dprintf("\tSP: %i\n", cpu->sp);
    dprintf("\tFP: %i\n", cpu->fp);

    dprintf("Consts\n");
    for (unsigned int i = 0; i < cpu->data_mem_size / sizeof(word_t); i++)
    {
        dprintf("\t0x%X\n", swap_uint32((cpu->data_mem)[i]));
    }

    dprintf("OPs\n");
    for (unsigned int i = 0; i < cpu->code_mem_size / sizeof(byte_t); i++)
    {
        dprintf("\t0x%X\n", (cpu->code_mem)[i]);
    }

    // TODO: print out stack elements

    dprintf("\n");
}