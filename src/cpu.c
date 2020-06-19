#include "cpu.h"


// Declarations for static functions
static bool octuple_stack_size(void);


FILE* restrict g_out_file;
FILE* restrict g_in_file;


static CPU_t vm_cpu;
CPU_t* restrict g_cpu = &vm_cpu;


word_t tos(void)
{
    return (g_cpu->stack)[g_cpu->sp];
}


bool stack_push(word_t e)
{
    if (++g_cpu->sp < g_cpu->stack_size)
    {
        (g_cpu->stack)[g_cpu->sp] = e;
    }
    else
    {
        // Resize until the size is enough
        while (g_cpu->sp >= g_cpu->stack_size)
        {
            if (octuple_stack_size() != true)
            {
                return false; // Resizing failed
            }
        }
        (g_cpu->stack)[g_cpu->sp] = e;
    }
    
    return true;
}


word_t stack_pop(void)
{
    if (g_cpu->sp < g_cpu->lv)
    {
        g_cpu->error_flag = true; // Could not pop since stack is empty
        return 0;
    }
    return (g_cpu->stack)[g_cpu->sp--];
}


/**
* Makes the stack 8 times larger
* Returns  true on success
*          false on failure
* Toggles the error flag if stack is resized beyond 4294967296
**/
static bool octuple_stack_size(void)
{
    if ((uint64_t)(g_cpu->stack_size * 4) >= 4294967296)
    {
        g_cpu->error_flag = true; // Program needs more memory than is possible in IJVM
        return false;
    }

    g_cpu->stack_size *= 8;
    g_cpu->stack = (word_t*)realloc(g_cpu->stack, (uint32_t)g_cpu->stack_size * sizeof(word_t));

    if (g_cpu->stack == NULL)
    {
        g_cpu->error_flag = true; // Not enough memory
        return false;
    }

    return true;
}


word_t get_constant(int i)
{
    return (g_cpu->const_mem)[i];
}


word_t get_local_variable(int i)
{
    uint32_t offset = (uint32_t)(g_cpu->lv + i);
    if (i >= g_cpu->nv)
    {
        g_cpu->error_flag = true; // Tried to access memory beyond variable memory
        return 0;
    }
    return (g_cpu->stack)[offset];
}


void update_local_variable(word_t new_val, int i)
{
    if (i >= g_cpu->nv)
    {
        g_cpu->error_flag = true; // Tried to access memory beyond variable memory
    }
    else
    {
        (g_cpu->stack)[g_cpu->lv + i] = new_val;
    }
}


void destroy_cpu(void)
{
    free(g_cpu->stack);
    free(g_cpu->code_mem);
    free(g_cpu->const_mem);
}
