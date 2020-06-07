#include "init.h"


FILE* g_out_file;
FILE* g_in_file;


int init_ijvm(char* binary_path)
{
	if (load_bin(binary_path, g_cpu_ptr) != true)
	{
		return -1;
	}

	init_registers(g_cpu_ptr);
	print_cpu_state(g_cpu_ptr);

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


void init_stack(CPU_t* cpu)
{
	cpu->stack_size = STACK_SIZE;
	cpu->stack = (word_t*)malloc(sizeof(word_t) * cpu->stack_size);
}


void init_registers(CPU_t* cpu)
{
	cpu->ip = 0;
	cpu->fp = 0;
	cpu->sp = -1;
}