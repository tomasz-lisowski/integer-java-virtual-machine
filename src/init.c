#include "init.h"


FILE* g_out_file;
FILE* g_in_file;


/**
* Use machine config (config.h) to init the CPU stack
**/
static void init_stack(CPU_t* cpu)
{
	cpu->stack_size = STACK_SIZE;
	cpu->stack = (word_t*)malloc(sizeof(word_t) * cpu->stack_size);
}


/**
* Init CPU registers
**/
static void init_registers(CPU_t* cpu)
{
	cpu->ip = 0;
	cpu->fp = 0;
	cpu->sp = -1;
}


/**
* Init CPU flags
**/
static void init_cpu_flags(CPU_t* cpu)
{
	cpu->error_flag = false;
}


int init_ijvm(char* binary_path)
{
	if (load_bin(binary_path, g_cpu_ptr) != true)
	{
		return -1;
	}

	init_registers(g_cpu_ptr);
	init_stack(g_cpu_ptr);
	init_cpu_flags(g_cpu_ptr);
	// At this point the CPU memory is well defined

	//print_cpu_state(g_cpu_ptr, false);

	set_output(stdout);
	set_input(stdin);

	return 0;
}


void set_output(FILE* f)
{
	g_out_file = f;
}


void set_input(FILE* f)
{
	g_in_file = f;
}