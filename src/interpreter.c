#include "interpreter.h"


static bool last_op_invalid = false;
static bool next_op_wide = false;


/**
* Get a one byte argument from code memory
**/
static byte_t get_arg_byte()
{
	return (g_cpu_ptr->code_mem)[g_cpu_ptr->ip++];
}


/**
* Get a two byte argument from code memory
**/
static short get_arg_short()
{
	byte_t b1 = (g_cpu_ptr->code_mem)[g_cpu_ptr->ip++];
	byte_t b2 = (g_cpu_ptr->code_mem)[g_cpu_ptr->ip++];
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
	stack_push((g_cpu_ptr->data_mem)[const_index]);
}


static void exec_op_iload()
{

}


static void exec_op_istore()
{

}


static void exec_op_pop()
{
	stack_pop();
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

}


static void exec_op_ifeq()
{
	short target = get_arg_short() - 3; // -3 to get offset from instruction call not address of last argument byte
	if (stack_pop() == 0)
	{
		g_cpu_ptr->ip += target;
	}
}


static void exec_op_iflt()
{
	short target = get_arg_short() - 3; // -3 to get offset from instruction call not address of last argument byte
	if (stack_pop() < 0)
	{
		g_cpu_ptr->ip += target;
	}
}


static void exec_op_icmpeq()
{
	short target = get_arg_short() - 3; // -3 to get offset from instruction call not address of last argument byte
	if (stack_pop() ==  stack_pop())
	{
		g_cpu_ptr->ip += target;
	}
}


static void exec_op_goto()
{
	short target = get_arg_short() - 3; // -3 to get offset from instruction call not address of last argument byte
	g_cpu_ptr->ip += target;
}


static void exec_op_ireturn()
{

}


static void exec_op_ior()
{
	word_t b = stack_pop();
	word_t a = stack_pop();
	stack_push(a | b);
}


static void exec_op_invokevirtual()
{

}


static void exec_op_wide()
{

}


static void exec_op_in()
{

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
	// TODO: Create and set a HALT flag inside the CPU
}


void run(void)
{
	// Reset interpreter state
	last_op_invalid = false;
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
	char step_msg[256];
	sprintf(step_msg, "IP %i\tOP %s", g_cpu_ptr->ip, op_decode(get_instruction()));

	switch ((g_cpu_ptr->code_mem)[(g_cpu_ptr->ip)++])
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
		dprintf("[INVALID OP 0x%X]\n", (g_cpu_ptr->code_mem)[g_cpu_ptr->ip - 1]);
		last_op_invalid = true;
	}

	dprintf("%s\t", step_msg);
	print_cpu_registers(g_cpu_ptr, true);
	dprintf(" ");
	print_cpu_stack(g_cpu_ptr, true);
	dprintf("\n");

	return true;
}


bool finished(void)
{
	bool end_of_code = get_program_counter() == g_cpu_ptr->code_mem_size;
	bool halt_op = get_instruction() == OP_HALT;
	bool err_op = get_instruction() == OP_ERR;
	bool invalid_op = last_op_invalid;
	bool cpu_err = g_cpu_ptr->error_flag;

	if (end_of_code || halt_op || err_op || invalid_op || cpu_err)
	{
		if (end_of_code) {
			dprintf("[EOT]\n");
		}
		else if (halt_op)
		{
			dprintf("[HALT]\n");
		}
		else if (err_op)
		{
			dprintf("[ERR]\n");
		}
		else if (invalid_op)
		{
			dprintf("[LOI]\n");
		}
		else if (cpu_err)
		{
			dprintf("[ERR FLAG]\n");
		}
		return true;
	}
	else
	{
		return false;
	}
}