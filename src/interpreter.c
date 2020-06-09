#include "interpreter.h"


static bool next_op_wide = false;


static void exec_op_nop()
{}


static void exec_op_bipush()
{
	word_t arg = (word_t)((int8_t)get_arg_byte(g_cpu_ptr->pc++));
	stack_push(arg);
}


static void exec_op_ldc_w()
{
	short const_index = get_arg_short(g_cpu_ptr->pc);
	g_cpu_ptr->pc += 2;
	stack_push((g_cpu_ptr->const_mem)[const_index]);
}


static void exec_op_iload()
{
	//byte_t i = get_arg_byte();
	//if (inside_method)
	//{
	//	word_t num_args = (g_cpu_ptr->stack)[g_cpu_ptr->fp + 3];
	//	stack_push((g_cpu_ptr->stack)[g_cpu_ptr->fp - num_args + i - 1]);
	//}
	//else
	//{
	//	stack_push(get_local_variable(i));
	//}
}


static void exec_op_istore()
{
	/*word_t val = stack_pop();
	byte_t i = get_arg_byte();
	update_local_variable(val, i);*/
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
	//byte_t var_i = get_arg_byte();
	//int8_t c = (int8_t)get_arg_byte();
	//if (inside_method)
	//{
	//	word_t num_args = (g_cpu_ptr->stack)[g_cpu_ptr->fp + 3];
	//	(g_cpu_ptr->stack)[g_cpu_ptr->fp - num_args + var_i] = (g_cpu_ptr->stack)[g_cpu_ptr->fp - num_args + var_i] + c;
	//}
	//else
	//{
	//	update_local_variable(c + get_local_variable(var_i), var_i);
	//}
}


static void exec_op_ifeq()
{
	short target = get_arg_short(g_cpu_ptr->pc) - 3; // -3 to get offset from instruction call not address of last argument byte
	g_cpu_ptr->pc += 2;
	if (stack_pop() == 0)
	{
		g_cpu_ptr->pc += target;
	}
}


static void exec_op_iflt()
{
	short target = get_arg_short(g_cpu_ptr->pc) - 3; // -3 to get offset from instruction call not address of last argument byte
	g_cpu_ptr->pc += 2;
	if (stack_pop() < 0)
	{
		g_cpu_ptr->pc += target;
	}
}


static void exec_op_icmpeq()
{
	short target = get_arg_short(g_cpu_ptr->pc) - 3; // -3 to get offset from instruction call not address of last argument byte
	g_cpu_ptr->pc += 2;
	if (stack_pop() ==  stack_pop())
	{
		g_cpu_ptr->pc += target;
	}
}


static void exec_op_goto()
{
	short target = get_arg_short(g_cpu_ptr->pc) - 3; // -3 to get offset from instruction call not address of last argument byte
	g_cpu_ptr->pc += 2;
	g_cpu_ptr->pc += target;
}


static void exec_op_ireturn()
{
	//word_t ret_val = stack_pop();
	//g_cpu_ptr->sp = g_cpu_ptr->fp + 3; // Point to num_args
	//word_t num_args = stack_pop();
	//g_cpu_ptr->pc = stack_pop();
	//g_cpu_ptr->tlv = g_cpu_ptr->blv - 1;
	//g_cpu_ptr->blv = stack_pop();
	//g_cpu_ptr->fp = stack_pop();
	//g_cpu_ptr->sp = g_cpu_ptr->sp - num_args; // Pop off all arguments
	//stack_push(ret_val);

	//if (g_cpu_ptr->fp == 0)
	//{
	//	inside_method = false;
	//}
}


static void exec_op_ior()
{
	word_t b = stack_pop();
	word_t a = stack_pop();
	stack_push(a | b);
}


static void exec_op_invokevirtual()
{
	//short offset = get_constant(get_arg_short()); // Move PC to instruction after INVOKEVIRTUAL which is the RET_ADDR we want to save on stack

	//stack_push(g_cpu_ptr->fp);
	//g_cpu_ptr->fp = g_cpu_ptr->sp;
	//stack_push(g_cpu_ptr->blv);
	//g_cpu_ptr->blv = g_cpu_ptr->tlv + 1;
	//stack_push(g_cpu_ptr->pc);

	//g_cpu_ptr->pc = offset; // Move into method's memory

	//short num_args = get_arg_short() - 1; // Ignore OBJ_REF which is always an argument
	//stack_push((word_t)num_args);

	//short num_local_vars = get_arg_short();
	//g_cpu_ptr->sp += num_local_vars;

	/**
	* Stack looks like:
	*   fp+5: local var 2  <- sp
	*   fp+4: local var 1
	*   fp+3: num_args
	*   fp+2: old_pc
	*   fp+1: old_blv
	*   fp+0: old_fp       <- fp
	*   fp-1: arg_2
	*   fp-2: arg_1
	**/
	//inside_method = true;
}


static void exec_op_wide()
{
	// Supported by iinc, istore, iload
	/*next_op_wide = true;
	step();
	next_op_wide = false;*/
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
	print_cpu_local_vars_current_frame(true);
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