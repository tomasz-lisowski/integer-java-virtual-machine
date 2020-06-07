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
    return (g_cpu_ptr->stack)[g_cpu_ptr->sp];
}

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

bool stack_push(word_t e)
{
    if (g_cpu_ptr->sp >= g_cpu_ptr->stack_size)
    {
        g_cpu_ptr->error_flag = true; // Stack overflow
        return false;
    }
    (g_cpu_ptr->stack)[++g_cpu_ptr->sp] = e;
    return true;
}

word_t stack_pop()
{
    if (g_cpu_ptr->sp < 0)
    {
        g_cpu_ptr->error_flag = true; // Could not pop since stack is empty
        return 0;
    }
    return (g_cpu_ptr->stack)[g_cpu_ptr->sp--];
}

word_t get_local_variable(int i)
{
    // TODO: Keep a count of variables in current frame to detect if an inexistent variable is being accessed
    return (g_cpu_ptr->stack)[(g_cpu_ptr->fp) + i];
}

void print_cpu_state(CPU_t* cpu, bool compact)
{
    dprintf("[CPU DUMP]\n");
    print_cpu_mem_size(cpu, compact);
    print_cpu_registers(cpu, compact);
    print_cpu_data_mem(cpu, compact);
    print_cpu_code_mem(cpu, compact);
    print_cpu_stack(cpu, compact);
}

void print_cpu_mem_size(CPU_t* cpu, bool compact)
{
    if (compact)
    {
        dprintf("Mem Size [");
        dprintf(" DM:%i", cpu->data_mem_size);
        dprintf(" CM:%i", cpu->code_mem_size);
        dprintf(" SS:%i", cpu->stack_size);
        dprintf(" ]");
    }
    else
    {
        dprintf("Mem Size\n");
        dprintf("\tConstant Pool Size: %i\n", cpu->data_mem_size);
        dprintf("\tText Size: %i\n", cpu->code_mem_size);
        dprintf("\tStack Size: %i\n", cpu->stack_size);
    }
}

void print_cpu_data_mem(CPU_t* cpu, bool compact)
{
    if (compact)
    {
        dprintf("Consts [");
        for (unsigned int i = 0; i < cpu->data_mem_size / sizeof(word_t); i++)
        {
            dprintf(" %i", (cpu->data_mem)[i]);
        }
        dprintf(" ]");
    }
    else
    {
        dprintf("Consts\n");
        for (unsigned int i = 0; i < cpu->data_mem_size / sizeof(word_t); i++)
        {
            dprintf("\t%i\n", (cpu->data_mem)[i]);
        }
    }
}

void print_cpu_code_mem(CPU_t* cpu, bool compact)
{
    if (compact)
    {
        dprintf("OP [");
        for (unsigned int i = 0; i < cpu->code_mem_size / sizeof(byte_t); i++)
        {
            dprintf(" 0x%X", (cpu->code_mem)[i]);
        }
        dprintf(" ]");
    }
    else
    {
        dprintf("OPs\n");
        for (unsigned int i = 0; i < cpu->code_mem_size / sizeof(byte_t); i++)
        {
            dprintf("\t0x%X\n", (cpu->code_mem)[i]);
        }
    }
}

void print_cpu_registers(CPU_t* cpu, bool compact)
{
    if (compact)
    {
        dprintf("IP:%i", cpu->ip);
        dprintf("  SP:%i", cpu->sp);
        dprintf("  FP:%i", cpu->fp);
    }
    else
    {
        dprintf("Regs\n");
        dprintf("\tIP: %i\n", cpu->ip);
        dprintf("\tSP: %i\n", cpu->sp);
        dprintf("\tFP: %i\n", cpu->fp);
    }
}

void print_cpu_stack(CPU_t* cpu, bool compact)
{
    if (compact)
    {
        dprintf("S[");
        for (int i = 0; i <= cpu->sp; i++)
        {
            dprintf(" %i", (cpu->stack)[i]);
        }
        dprintf(" ]");
    }
    else
    {
        dprintf("Stack\n");
        for (int i = 0; i <= cpu->sp; i++)
        {
            dprintf("\t%i\n", (cpu->stack)[i]);
        }
    }
}