#include "init.h"


/**
* Returns the i'th byte from code memory
**/
static byte_t get_arg_byte(int i)
{
	return (g_cpu_ptr->code_mem)[i];
}


/**
* Returns a short starting at i'th byte from code memory
**/
static short get_arg_short(int i)
{
	byte_t b1 = (g_cpu_ptr->code_mem)[i];
	byte_t b2 = (g_cpu_ptr->code_mem)[i + 1];
	return (b1 << 8) | b2;
}


/**
* Returns the number of local variables inside the main method
**/
static uint32_t get_num_local_vars_main()
{
	uint32_t addr_first_method_after_main = (~(uint32_t)0); // = SIZE_MAX
	int64_t greatest_var_index = -1;

	// Temporary variables to keep track of loop state
	byte_t op;
	bool next_wide = false;
	uint32_t var_index;
	uint32_t addr;

	/**
	* Simultaneously find where first method (after main) starts
	* and determine the number of referenced variables inside main.
	**/ 
	for (uint32_t i = 0; i < g_cpu_ptr->code_mem_size; i++)
	{
		op = (g_cpu_ptr->code_mem)[i];

		// Don't iterate over instructions of another method
		if (i != (~(uint32_t)0) && i + 2 > addr_first_method_after_main)
		{
			break;
		}

		switch (op)
		{
		case OP_WIDE:
			next_wide = true;
			continue;

		case OP_INVOKEVIRTUAL:
			addr = get_arg_short(i + 1);
			if (addr < addr_first_method_after_main)
			{
				addr_first_method_after_main = addr;
			}
			i += 2;
			continue;

		case OP_ISTORE:
		case OP_ILOAD:
		case OP_IINC:
			if (next_wide)
			{
				var_index = get_arg_short(i + 1);
				next_wide = false;
				i += 2;
			}
			else
			{
				var_index = get_arg_byte(i + 1);
				i += 1;
			}

			if (op == OP_IINC)
			{
				i += 1;
			}

			if (var_index > greatest_var_index)
			{
				greatest_var_index = var_index;
			}
			continue;

		case OP_BIPUSH:
			i += 1;
			continue;

		case OP_GOTO:
		case OP_IFEQ:
		case OP_IFLT:
		case OP_ICMPEQ:
		case OP_LDC_W:
			i += 2;
			continue;
		}
	}

	return (uint32_t)greatest_var_index + 1;
}


/**
* Use machine config (config.h) to init the CPU stack
**/
static void init_stack()
{
	uint32_t main_num_vars = get_num_local_vars_main();

	// MAX_SIZE = 4294967296 = (4096 * 4) * 8^i where i == 6
	for (uint32_t i = 0; i <= 6; i++)
	{
		// Find smallest suitable stack size 
		if (main_num_vars + 1024 <= 4096 * 4 * power(8, i)) // 1024 is an arbitrary margin for operands
		{
			g_cpu_ptr->stack_size = 4096 * power(8, i);
			break;
		}
	}
	g_cpu_ptr->stack = (word_t*)malloc(sizeof(word_t) * g_cpu_ptr->stack_size);

	// Pre-allocate local variable memory before the operand stack of main
	g_cpu_ptr->sp = main_num_vars - 1;
	g_cpu_ptr->lv = 0;
	g_cpu_ptr->nv = main_num_vars;
	g_cpu_ptr->fp = main_num_vars;
}


/**
* Init CPU registers that are not part of managing the stack
**/
static void init_registers()
{
	g_cpu_ptr->pc = 0;
}


/**
* Init CPU flags
**/
static void init_cpu_flags()
{
	g_cpu_ptr->error_flag = false;
	g_cpu_ptr->halt_flag = false;
}


int init_ijvm(char* binary_path)
{
	if (load_bin(binary_path) != true)
	{
		destroy_ijvm(); // Ensure memory is free'd
		return -1;
	}

	init_registers();
	init_stack();
	init_cpu_flags();

	set_output(stdout);
	set_input(stdin);
	// At this point the CPU memory is well defined

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