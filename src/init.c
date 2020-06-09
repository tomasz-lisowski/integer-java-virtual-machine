#include "init.h"

/**
* Returns the number of local variables inside the main method
**/
static void get_num_local_vars_main()
{
	uint32_t addr_first_method_after_main = (~(uint32_t)0); // = SIZE_MAX
	int64_t greatest_var_index = -1;

	// Temporary variables to keep track of loop state
	byte_t op;
	bool next_wide = false;
	uint32_t var_index;

	for (uint32_t i = 0; i < g_cpu_ptr->code_mem_size; i++)
	{
		op = (g_cpu_ptr->code_mem)[i];

		// Don't iterate over instructions of another method
		if (i != (~(uint32_t)0) && i + 2 > addr_first_method_after_main)
		{
			break;
		}

		if (op == OP_WIDE)
		{
			next_wide = true;
			dprintf("next wide\n");
			continue;
		}

		// Store the maximum referenced index
		if (op == OP_ISTORE || op == OP_ILOAD || op == OP_IINC)
		{
			if (next_wide)
			{
				var_index = get_arg_short(i + 1);
				//dprintf("wide var index %i\n", var_index);
				i += 2;
				next_wide = false;
			}
			else
			{
				var_index = get_arg_byte(i + 1);
				//dprintf("[%s] var index %i\n", op_decode(op), var_index);
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
		}

		if (op == OP_INVOKEVIRTUAL)
		{
			uint32_t addr = get_arg_short(i + 1);
			//dprintf("[INV] %u\n", addr);
			if (addr < addr_first_method_after_main)
			{
				addr_first_method_after_main = addr;
			}
			i += 2;
			continue;
		}

		// Skip over other instructions + arguments
		switch (op)
		{
		case OP_BIPUSH:
			i += 1;
			continue;
		case OP_GOTO:
		case OP_IFEQ:
		case OP_IFLT:
		case OP_ICMPEQ:
		case OP_IINC:
		case OP_LDC_W:
			i += 2;
			continue;
		}
	}

	dprintf("number of local variables %li\n", greatest_var_index + 1);
}


/**
* Use machine config (config.h) to init the CPU stack
**/
static void init_stack()
{
	// TODO: Analyze number of local variables in main function to pre-allocate a local var memory on stack before main's operand stack
	g_cpu_ptr->stack_size = STACK_SIZE;
	g_cpu_ptr->stack = (word_t*)malloc(sizeof(word_t) * g_cpu_ptr->stack_size);
}


/**
* Init CPU registers
**/
static void init_registers()
{
	g_cpu_ptr->pc = 0;
	g_cpu_ptr->fp = 0;
	g_cpu_ptr->sp = -1;
	g_cpu_ptr->lv = -1;
	g_cpu_ptr->nv = 0;
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
	// At this point the CPU memory is well defined

	set_output(stdout);
	set_input(stdin);

	//get_num_local_vars_main();
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