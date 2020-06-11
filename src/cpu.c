#include "cpu.h"

FILE* restrict g_out_file;
FILE* restrict g_in_file;

static CPU_t vm_cpu;
CPU_t* restrict g_cpu_ptr = &vm_cpu;


word_t tos(void)
{
    return (g_cpu_ptr->stack)[g_cpu_ptr->sp];
}


bool stack_push(word_t e)
{
    if (++g_cpu_ptr->sp < g_cpu_ptr->stack_size)
    {
        (g_cpu_ptr->stack)[g_cpu_ptr->sp] = e;
    }
    else
    {
        // Resize until the size is enough
        while (g_cpu_ptr->sp >= g_cpu_ptr->stack_size)
        {
            if (octuple_stack_size() != true)
            {
                return false; // Resizing failed
            }
        }
        (g_cpu_ptr->stack)[g_cpu_ptr->sp] = e;
    }
    
    return true;
}


word_t stack_pop(void)
{
    if (g_cpu_ptr->sp < g_cpu_ptr->lv)
    {
        g_cpu_ptr->error_flag = true; // Could not pop since stack is empty
        return 0;
    }
    return (g_cpu_ptr->stack)[g_cpu_ptr->sp--];
}


bool octuple_stack_size(void)
{
    if (g_cpu_ptr->stack_size * 4 >= 4294967296)
    {
        g_cpu_ptr->error_flag = true; // Program needs more memory than is possible in IJVM
        return false;
    }

    g_cpu_ptr->stack_size *= 8;
    g_cpu_ptr->stack = (word_t*)realloc(g_cpu_ptr->stack, g_cpu_ptr->stack_size * 4);

    if (g_cpu_ptr->stack == NULL)
    {
        g_cpu_ptr->error_flag = true; // Not enough memory
        return false;
    }

    return true;
}


word_t get_constant(int i)
{
    return (g_cpu_ptr->const_mem)[i];
}


word_t get_local_variable(int i)
{
    uint32_t offset = g_cpu_ptr->lv + i;
    if (i >= g_cpu_ptr->nv)
    {
        g_cpu_ptr->error_flag = true; // Tried to access memory beyond variable memory
        return 0;
    }
    return (g_cpu_ptr->stack)[offset];
}


void update_local_variable(word_t new_val, int i)
{
    if (i >= g_cpu_ptr->nv)
    {
        g_cpu_ptr->error_flag = true; // Tried to access memory beyond variable memory
    }
    else
    {
        (g_cpu_ptr->stack)[g_cpu_ptr->lv + i] = new_val;
    }
}


#ifdef DEBUG
void print_cpu_state(bool compact)
{
    dprintf("[CPU DUMP]\n");
    print_cpu_mem_size(compact);
    print_cpu_registers(compact);
    print_cpu_const_mem(compact);
    print_cpu_code_mem(compact);
    print_cpu_stack(compact);
    print_cpu_local_vars(compact);
}


void print_cpu_mem_size(bool compact)
{
    if (compact)
    {
        dprintf("Mem Size [");
        dprintf(" DM:%i", g_cpu_ptr->const_mem_size);
        dprintf(" CM:%i", g_cpu_ptr->code_mem_size);
        dprintf(" ST:%i", g_cpu_ptr->stack_size);
        dprintf(" V:%i",  g_cpu_ptr->nv);
        dprintf(" ]");
    }
    else
    {
        dprintf("Mem Size\n");
        dprintf("\tConstant Pool Size: %i\n",   g_cpu_ptr->const_mem_size);
        dprintf("\tText Size: %i\n",            g_cpu_ptr->code_mem_size);
        dprintf("\tStack Size: %i\n",           g_cpu_ptr->stack_size);
        dprintf("\tVariable Memory Size: %i\n", g_cpu_ptr->nv);
    }
}


void print_cpu_const_mem(bool compact)
{
    if (compact)
    {
        dprintf("Consts [");
        for (unsigned int i = 0; i < g_cpu_ptr->const_mem_size / sizeof(word_t); i++)
        {
            dprintf(" %i", (g_cpu_ptr->const_mem)[i]);
        }
        dprintf(" ]");
    }
    else
    {
        dprintf("Consts\n");
        for (unsigned int i = 0; i < g_cpu_ptr->const_mem_size / sizeof(word_t); i++)
        {
            dprintf("\t%i\n", (g_cpu_ptr->const_mem)[i]);
        }
    }
}


void print_cpu_code_mem(bool compact)
{
    if (compact)
    {
        dprintf("OP [");
        for (unsigned int i = 0; i < g_cpu_ptr->code_mem_size / sizeof(byte_t); i++)
        {
            dprintf(" 0x%X", (g_cpu_ptr->code_mem)[i]);
        }
        dprintf(" ]");
    }
    else
    {
        dprintf("OPs\n");
        for (unsigned int i = 0; i < g_cpu_ptr->code_mem_size / sizeof(byte_t); i++)
        {
            dprintf("\t0x%X\n", (g_cpu_ptr->code_mem)[i]);
        }
    }
}


void print_cpu_registers(bool compact)
{
    if (compact)
    {
        dprintf("PC:%-4i",   g_cpu_ptr->pc);
        dprintf("  SP:%-4i", g_cpu_ptr->sp);
        dprintf("  FP:%-4i", g_cpu_ptr->fp);
        dprintf("  LV:%-4i", g_cpu_ptr->lv);
        dprintf("  NV:%-4i", g_cpu_ptr->nv);
    }
    else
    {
        dprintf("Regs\n");
        dprintf("\tPC: %i\n", g_cpu_ptr->pc);
        dprintf("\tSP: %i\n", g_cpu_ptr->sp);
        dprintf("\tFP: %i\n", g_cpu_ptr->fp);
        dprintf("\tLV: %i\n", g_cpu_ptr->lv);
        dprintf("\tNV: %i\n", g_cpu_ptr->nv);
    }
}


void print_cpu_stack(bool compact)
{
    if (compact)
    {
        dprintf("S[");
        for (int64_t i = g_cpu_ptr->fp; i <= g_cpu_ptr->sp; i++)
        {
            dprintf(" %i", (g_cpu_ptr->stack)[i]);
        }
        dprintf(" ]");
    }
    else
    {
        dprintf("Stack\n");
        for (int64_t i = g_cpu_ptr->fp; i <= g_cpu_ptr->sp; i++)
        {
            dprintf("\t%i\n", (g_cpu_ptr->stack)[i]);
        }
    }
}


void print_cpu_local_vars(bool compact)
{
    if (compact)
    {
        dprintf("LV[");
        for (int64_t i = g_cpu_ptr->lv; i < g_cpu_ptr->lv + g_cpu_ptr->nv; i++)
        {
            dprintf(" %i", (g_cpu_ptr->stack)[i]);
        }
        dprintf(" ]");
    }
    else
    {
        dprintf("LV\n");
        for (int64_t i = g_cpu_ptr->lv; i < g_cpu_ptr->lv + g_cpu_ptr->nv; i++)
        {
            dprintf("\t%i\n", (g_cpu_ptr->stack)[i]);
        }
    }
}
#endif
