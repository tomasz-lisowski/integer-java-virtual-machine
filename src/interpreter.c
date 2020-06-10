#include "interpreter.h"


static bool next_op_wide = false;


/**
* Get a one byte argument from code memory and increment PC
**/
static byte_t get_arg_byte()
{
	return (g_cpu_ptr->code_mem)[g_cpu_ptr->pc++];
}


/**
* Get a two byte argument from code memory and increment PC twice
**/
static short get_arg_short()
{
	byte_t b1 = (g_cpu_ptr->code_mem)[g_cpu_ptr->pc++];
	byte_t b2 = (g_cpu_ptr->code_mem)[g_cpu_ptr->pc++];
	return (b1 << 8) | b2;
}


static void exec_op_nop()
{}


static void exec_op_bipush()
{
	word_t arg = (word_t)((int8_t)get_arg_byte());
	stack_push(arg);
}


static void exec_op_ldc_w()
{
	short const_index = get_arg_short();
	stack_push((g_cpu_ptr->const_mem)[const_index]);
}


static void exec_op_iload()
{
	uint32_t var_i;
	if (next_op_wide)
	{
		var_i = get_arg_short();
	}
	else
	{
		var_i = get_arg_byte();
	}
	stack_push(get_local_variable(var_i));
}


static void exec_op_istore()
{
	uint32_t var_i;
	if (next_op_wide)
	{
		var_i = get_arg_short();
	}
	else
	{
		var_i = get_arg_byte();
	}
	word_t val = stack_pop();
	update_local_variable(val, var_i);
}


static void exec_op_pop()
{
	g_cpu_ptr->sp -= 1; // TODO: Measure performance difference vs. calling stack_pop();
}


static void exec_op_dup()
{
	stack_push((g_cpu_ptr->stack)[g_cpu_ptr->sp]);
}


static void exec_op_swap()
{
	word_t b = stack_pop();
	word_t a = stack_pop();
	stack_push(b);
	stack_push(a);
}


static void exec_op_iadd()
{
	word_t b = stack_pop();
	word_t a = stack_pop();
	stack_push(a + b);
}


static void exec_op_isub()
{
	word_t b = stack_pop();
	word_t a = stack_pop();
	stack_push(a - b);
}


static void exec_op_iand()
{
	word_t b = stack_pop();
	word_t a = stack_pop();
	stack_push(a & b);
}


static void exec_op_iinc()
{
	uint32_t var_i;
	if (next_op_wide)
	{
		var_i = get_arg_short();
	}
	else
	{
		var_i = get_arg_byte();
	}
	int8_t c = (int8_t)get_arg_byte();
	update_local_variable(c + get_local_variable(var_i), var_i);
}


static void exec_op_ifeq()
{
	short target = get_arg_short() - 3; // -3 to get offset from instruction call not address of last argument byte
	if (stack_pop() == 0)
	{
		g_cpu_ptr->pc += target;
	}
}


static void exec_op_iflt()
{
	short target = get_arg_short() - 3; // -3 to get offset from instruction call not address of last argument byte
	if (stack_pop() < 0)
	{
		g_cpu_ptr->pc += target;
	}
}


static void exec_op_icmpeq()
{
	short target = get_arg_short() - 3; // -3 to get offset from instruction call not address of last argument byte
	if (stack_pop() ==  stack_pop())
	{
		g_cpu_ptr->pc += target;
	}
}


static void exec_op_goto()
{
	short target = get_arg_short() - 3; // -3 to get offset from instruction call not address of last argument byte
	g_cpu_ptr->pc += target;
}


static void exec_op_ireturn()
{
	int ret_val = stack_pop();
	int old_nv = g_cpu_ptr->nv;

	g_cpu_ptr->sp = g_cpu_ptr->fp + 3;
	g_cpu_ptr->pc = stack_pop();
	g_cpu_ptr->fp = stack_pop();
	g_cpu_ptr->nv = stack_pop();
	g_cpu_ptr->lv = stack_pop();
	g_cpu_ptr->sp -= old_nv; // Remove all local variables from stack
	stack_push(ret_val);
}


static void exec_op_ior()
{
	word_t b = stack_pop();
	word_t a = stack_pop();
	stack_push(a | b);
}


static void exec_op_invokevirtual()
{
	short offset = get_constant(get_arg_short()); // Move PC to return address while getting offset
	int old_pc = g_cpu_ptr->pc;
	g_cpu_ptr->pc = offset; // Move into method's memory
	short num_args = get_arg_short();
	short num_locals = get_arg_short();

	g_cpu_ptr->sp += num_locals;
	stack_push(g_cpu_ptr->lv);
	stack_push(g_cpu_ptr->nv);
	stack_push(g_cpu_ptr->fp);
	stack_push(old_pc);
	
	g_cpu_ptr->fp = g_cpu_ptr->sp - 3;
	g_cpu_ptr->nv = num_args + num_locals;
	g_cpu_ptr->lv = g_cpu_ptr->fp - g_cpu_ptr->nv;

	/**
	* Stack after call:
	*   
	*   fp+3: old_pc    <- sp
	*   fp+2: old_fp
	*   fp+1: old_nv
	*   fp+0: old_lv    <- fp
	*   lv+3: local 2
	*   lv+2: local 1
	*   lv+1: arg_2
	*   lv+0: arg_1     <- lv
	**/
}


static void exec_op_wide()
{
	next_op_wide = true;
	step();
	next_op_wide = false;
}


static void exec_op_in()
{
	word_t c = (word_t)getc(g_in_file);
	if (c == EOF)
	{
		stack_push(0);
	}
	else
	{
		stack_push(c);
	}
}


static void exec_op_out()
{
	fprintf(g_out_file, "%c", (char)stack_pop());
}


static void exec_op_err()
{
	g_cpu_ptr->error_flag = true;
}


static void exec_op_halt()
{
	g_cpu_ptr->halt_flag = true;
}


void run(void)
{
	// Reset interpreter state
	next_op_wide = false;
	
	dprintf("[VM START]\n");
	while (!finished())
	{
		step();
	}
	dprintf("[VM STOP]\n");
}


bool step(void)
{
#ifdef DEBUG
	int old_pc = g_cpu_ptr->pc;
	char* op = op_decode((g_cpu_ptr->code_mem)[g_cpu_ptr->pc]);
#endif

	switch ((g_cpu_ptr->code_mem)[(g_cpu_ptr->pc)++])
	{
	case OP_NOP:
		exec_op_nop();
		break;
	case OP_BIPUSH:
		exec_op_bipush();
		break;
	case OP_LDC_W:
		exec_op_ldc_w();
		break;
	case OP_ILOAD:
		exec_op_iload();
		break;
	case OP_ISTORE:
		exec_op_istore();
		break;
	case OP_POP:
		exec_op_pop();
		break;
	case OP_DUP:
		exec_op_dup();
		break;
	case OP_SWAP:
		exec_op_swap();
		break;
	case OP_IADD:
		exec_op_iadd();
		break;
	case OP_ISUB:
		exec_op_isub();
		break;
	case OP_IAND:
		exec_op_iand();
		break;
	case OP_IINC:
		exec_op_iinc();
		break;
	case OP_IFEQ:
		exec_op_ifeq();
		break;
	case OP_IFLT:
		exec_op_iflt();
		break;
	case OP_ICMPEQ:
		exec_op_icmpeq();
		break;
	case OP_GOTO:
		exec_op_goto();
		break;
	case OP_IRETURN:
		exec_op_ireturn();
		break;
	case OP_IOR:
		exec_op_ior();
		break;
	case OP_INVOKEVIRTUAL:
		exec_op_invokevirtual();
		break;
	case OP_WIDE:
		exec_op_wide();
		break;
	case OP_IN:
		exec_op_in();
		break;
	case OP_OUT:
		exec_op_out();
		break;
	case OP_ERR:
		exec_op_err();
		break;
	case OP_HALT:
		exec_op_halt();
		break;
	default:
		dprintf("[INVALID OP 0x%X]\n", (g_cpu_ptr->code_mem)[g_cpu_ptr->pc - 1]);
		g_cpu_ptr->error_flag = true;
	}

#ifdef DEBUG
	dprintf("OPC: %-4i OP: %-14s", old_pc, op);
	print_cpu_registers(true);
	dprintf(" ");
	print_cpu_stack(true);
	dprintf("\t");
	print_cpu_local_vars(true);
	dprintf("\n");
#endif

	return true;
}


bool finished(void)
{
	bool end_of_code = g_cpu_ptr->pc == g_cpu_ptr->code_mem_size;
	bool cpu_err = g_cpu_ptr->error_flag;
	bool cpu_halt = g_cpu_ptr->halt_flag;

	if (end_of_code || cpu_err || cpu_halt)
	{
		if (cpu_halt)
		{
			dprintf("[HALT_FLAG]\n");
		}
		else if (cpu_err)
		{
			dprintf("[ERR_FLAG]\n");
		}
		else if (end_of_code) {
			dprintf("[TEXT_END]\n");
		}
		return true;
	}
	else
	{
		return false;
	}
}