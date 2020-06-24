#include "cpu.h"


// Declarations of static functions
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
        printf("[ERR] Failed to pop off the stack because the stack is empty. In \"cpu.c::stack_pop\".\n");
        destroy_ijvm_now();
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
    word_t* tmp_stack;
    int tmp_stack_size;
    uint64_t expected_size = (uint64_t)g_cpu->stack_size * (uint8_t)sizeof(word_t) * 8;
    if (expected_size >= 4294967296 || expected_size == 0)
    {
        printf("[ERR] Program needs more memory than is possible inside IJVM. In \"cpu.c::octuple_stack_size\".\n");
        destroy_ijvm_now();
    }

    g_cpu->stack_size = g_cpu->stack_size * 8;
    tmp_stack = g_cpu->stack;
    tmp_stack_size = g_cpu->stack_size;
    g_cpu->stack = (word_t*)realloc(g_cpu->stack, (uint32_t)tmp_stack_size * sizeof(word_t));
    if (g_cpu->stack == NULL)
    {
        g_cpu->stack_size = g_cpu->stack_size / 8;
        g_cpu->stack = tmp_stack;
        if (gc_arrays() != 0)
        {
            return octuple_stack_size(); // Run GC to be sure memory allocation error is not caused by garbage
        }
        printf("[ERR] Failed to allocate memory. In \"cpu.c::octuple_stack_size\".\n");
        destroy_ijvm_now();
    }

    return true;
}


word_t get_constant(int i)
{
    if (i < 0 || i >= (g_cpu->const_mem_size / 4))
    {
        fprintf(stderr, "[ERR] Invalid constant index. In \"cpu.c::get_constant\".\n");
        destroy_ijvm_now();
    }
    return (g_cpu->const_mem)[i];
}


word_t get_local_variable(int i)
{
    uint32_t offset = (uint32_t)(g_cpu->lv + i);
    if (i < 0 || i >= g_cpu->nv)
    {
        fprintf(stderr, "[ERR] Program tried to access memory outside of variable memory. In \"cpu.c::get_local_variable\".\n");
        destroy_ijvm_now();
    }
    return (g_cpu->stack)[offset];
}


void update_local_variable(word_t new_val, int i)
{
    if (i < 0 || i >= g_cpu->nv)
    {
        fprintf(stderr, "[ERR] Program tried to access memory outside of variable memory. In \"cpu.c::update_local_variable\".\n");
        destroy_ijvm_now();
    }
    else
    {
        (g_cpu->stack)[g_cpu->lv + i] = new_val;
    }
}


void jump(int32_t offset)
{
    if (g_cpu->pc + offset < 0 || offset >= g_cpu->code_mem_size)
    {
        fprintf(stderr, "[ERR] Invalid jump offset. In \"cpu.c::jump\".\n");
        destroy_ijvm_now();
    }
    g_cpu->pc += offset;
}


void destroy_cpu(void)
{
    free(g_cpu->stack);
    free(g_cpu->code_mem);
    free(g_cpu->const_mem);
}
