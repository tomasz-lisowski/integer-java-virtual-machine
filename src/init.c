#include "init.h"


// Declarations of static functions
static int32_t get_num_local_vars_main(void);
static void init_stack(void);
static void init_registers(void);
static void init_cpu_flags(void);


/**
* Returns the number of local variables inside the main method
**/
static int32_t get_num_local_vars_main(void)
{
	uint32_t addr_first_method_after_main = SIZE_MAX_UINT32_T;
	int32_t var_num = 0;

	// Temporary variables to keep track of loop state
	byte_t op;
	bool next_wide = false;
	int32_t var_index;
	int32_t addr;

	/**
	* Simultaneously find where first method (after main) starts
	* and determine the number of referenced variables inside main.
	* "Loop Jamming"
	**/ 
	for (uint32_t i = 0; (int64_t)i < g_cpu->code_mem_size; i++)
	{
		op = (g_cpu->code_mem)[i];

		// Don't iterate over instructions of another method
		if (i != SIZE_MAX_UINT32_T && i + 2 > addr_first_method_after_main)
		{
			break;
		}

		switch (op)
		{
		case OP_WIDE:
			next_wide = true;
			continue;

		case OP_INVOKEVIRTUAL:
			addr = get_constant(get_code_short((int)i + 1));
			if (g_cpu->error_flag == true)
			{
				return 0;
			}
			if (addr > 0 && addr < (int64_t)addr_first_method_after_main && addr < g_cpu->code_mem_size)
			{
				addr_first_method_after_main = (uint32_t)addr;
			}
			i += 2;
			continue;

		case OP_ISTORE:
		case OP_ILOAD:
		case OP_IINC:
			if (next_wide)
			{
				var_index = get_code_short((int)i + 1);
				next_wide = false;
				i += 2;
			}
			else
			{
				var_index = get_code_byte((int)i + 1);
				i += 1;
			}

			if (op == OP_IINC)
			{
				i += 1;
			}

			if (var_index + 1 > var_num)
			{
				var_num = var_index + 1;
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

		/*
		case OP_DUP:
		case OP_ERR:
		case OP_HALT:
		case OP_IADD:
		case OP_IAND:
		case OP_IN:
		case OP_IOR:
		case OP_IRETURN:
		case OP_ISUB:
		case OP_NOP:
		case OP_OUT:
		case OP_POP:
		case OP_SWAP:
		case OP_NEWARRAY:
		case OP_IALOAD:
		case OP_IASTORE:
		case OP_NETBIND:
		case OP_NETCONNECT:
		case OP_NETIN:
		case OP_NETOUT:
		case OP_NETCLOSE:
			// All these instructions don't take arguments
			continue;
		*/
	}
	return var_num;
}


/**
* Init the CPU stack
**/
static void init_stack(void)
{
	int32_t main_num_vars = get_num_local_vars_main();
	int tmp_mem_size;
	if (main_num_vars < 0 || g_cpu->error_flag == true)
	{
		fprintf(stderr, "[ERR] Invalid number of arguments in main method. In \"init.c::init_stack\".\n");
		destroy_ijvm_now();
	}

	// MAX_SIZE = 4294967296 = (STACK_MIN_SIZE * sizeof(word_t)) * 8^i
	for (uint32_t i = 0; i <= 6; i++)
	{
		// Find smallest suitable stack size 
		if (((uint32_t)main_num_vars * sizeof(word_t)) + 1024 <= (STACK_MIN_SIZE * sizeof(word_t)) * power(8, i)) // 1024 is an arbitrary margin for operands
		{
			g_cpu->stack_size = (int)(STACK_MIN_SIZE * power(8, i));
			break;
		}
	}
	tmp_mem_size = g_cpu->stack_size;
	g_cpu->stack = (word_t*)malloc((uint32_t)tmp_mem_size * sizeof(word_t));
	if (g_cpu->stack == NULL)
	{
		fprintf(stderr, "[ERR] Failed to allocate memory. In \"init.c::init_stack\".\n");
		destroy_ijvm_now();
	}

	// Pre-allocate local variable memory before the operand stack of main
	g_cpu->sp = (int)(main_num_vars) - 1;
	g_cpu->lv = 0;
	g_cpu->nv = (int)(main_num_vars);
	g_cpu->fp = (int)(main_num_vars);

	if (g_cpu->error_flag == false)
	{
		memset(g_cpu->stack, 0, (uint32_t)g_cpu->nv * sizeof(uint32_t)); // Init local variables to 0
	}
}


/**
* Init CPU registers that are not used to manage the stack
**/
static void init_registers(void)
{
	g_cpu->pc = 0;
}


/**
* Init CPU flags
**/
static void init_cpu_flags(void)
{
	g_cpu->error_flag = false;
	g_cpu->halt_flag = false;
}


int init_ijvm(char* binary_path)
{
	if (load_bin(binary_path) != true)
	{
		destroy_ijvm();
		return -1;
	}
	init_registers();
	init_stack();
	init_cpu_flags();
	set_output(stdout);
	set_input(stdin);
	init_interpreter();
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
