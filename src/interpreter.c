#include "interpreter.h"


// Declarations for static functions
static inline byte_t get_arg_byte(void);
static inline short get_arg_short(void);

static inline void exec_op_nop(void);
static inline void exec_op_bipush(void);
static inline void exec_op_ldc_w(void);
static inline void exec_op_iload(void);
static inline void exec_op_istore(void);
static inline void exec_op_pop(void);
static inline void exec_op_dup(void);
static inline void exec_op_swap(void);
static inline void exec_op_iadd(void);
static inline void exec_op_isub(void);
static inline void exec_op_iand(void);
static inline void exec_op_iinc(void);
static inline void exec_op_ifeq(void);
static inline void exec_op_iflt(void);
static inline void exec_op_icmpeq(void);
static inline void exec_op_goto(void);
static inline void exec_op_ireturn(void);
static inline void exec_op_ior(void);
static inline void exec_op_invokevirtual(void);
static inline void exec_op_wide(void);
static inline void exec_op_in(void);
static inline void exec_op_out(void);
static inline void exec_op_err(void);
static inline void exec_op_halt(void);

static inline void exec_op_newarray(void);
static inline void exec_op_iaload(void);
static inline void exec_op_iastore(void);
static inline void exec_op_gc(void);

static inline void exec_op_netbind(void);
static inline void exec_op_netconnect(void);
static inline void exec_op_netin(void);
static inline void exec_op_netout(void);
static inline void exec_op_netclose(void);


static bool next_op_wide = false;


/**
* Get a one byte argument from code memory and increment PC
**/
static inline byte_t get_arg_byte(void)
{
	if (g_cpu->pc + 1 > g_cpu->code_mem_size)
	{
		fprintf(stderr, "[ERR] Program counter was moved beyond code memory. In \"interpreter.c::get_arg_short\".\n");
		destroy_ijvm_now();
	}
	return (g_cpu->code_mem)[g_cpu->pc++];
}


/**
* Get a two byte argument from code memory and increment PC twice
**/
static inline short get_arg_short(void)
{
	if (g_cpu->pc + 2 > g_cpu->code_mem_size)
	{
		fprintf(stderr, "[ERR] Program counter was moved beyond code memory. In \"interpreter.c::get_arg_short\".\n");
		destroy_ijvm_now();
	}
	byte_t b1 = (g_cpu->code_mem)[g_cpu->pc++];
	byte_t b2 = (g_cpu->code_mem)[g_cpu->pc++];
	return (short)((b1 << 8) | b2);
}


static inline void exec_op_nop(void)
{}


static inline void exec_op_bipush(void)
{
	word_t arg = (int8_t)get_arg_byte();
	stack_push(arg);
}


static inline void exec_op_ldc_w(void)
{
	uint16_t const_index = (uint16_t)get_arg_short();
	word_t const_val = get_constant(const_index);
	if (g_cpu->error_flag == false)
	{
		stack_push(const_val);
	}
}


static inline void exec_op_iload(void)
{
	uint16_t var_i;
	if (next_op_wide)
	{
		var_i = (uint16_t)get_arg_short();
	}
	else
	{
		var_i = (uint16_t)get_arg_byte();
	}
	stack_push(get_local_variable(var_i));
}


static inline void exec_op_istore(void)
{
	uint16_t var_i;
	word_t val = stack_pop();
	if (next_op_wide)
	{
		var_i = (uint16_t)get_arg_short();
	}
	else
	{
		var_i = (uint16_t)get_arg_byte();
	}
	update_local_variable(val, var_i);
}


static inline void exec_op_pop(void)
{
	stack_pop();
}


static inline void exec_op_dup(void)
{
	stack_push((g_cpu->stack)[g_cpu->sp]);
}


static inline void exec_op_swap(void)
{
	word_t b = stack_pop();
	word_t a = stack_pop();
	stack_push(b);
	stack_push(a);
}


static inline void exec_op_iadd(void)
{
	int64_t b = stack_pop();
	int64_t a = stack_pop();
	stack_push((word_t)(a + b));
}


static inline void exec_op_isub(void)
{
	word_t b = stack_pop();
	word_t a = stack_pop();
	stack_push(a - b);
}


static inline void exec_op_iand(void)
{
	word_t b = stack_pop();
	word_t a = stack_pop();
	stack_push(a & b);
}


static inline void exec_op_iinc(void)
{
	int16_t var_i;
	int8_t c;
	if (next_op_wide)
	{
		var_i = get_arg_short();
	}
	else
	{
		var_i = (int16_t)get_arg_byte();
	}
	c = (int8_t)get_arg_byte();
	update_local_variable((int32_t)(get_local_variable(var_i) + c), var_i);
}


static inline void exec_op_ifeq(void)
{
	int16_t jmp_offset = (int16_t)(get_arg_short() - 3); // -3 to get offset from instruction call not address of last argument byte
	if (stack_pop() == 0)
	{
		jump(jmp_offset);
	}
}


static inline void exec_op_iflt(void)
{
	int16_t jmp_offset = (int16_t)(get_arg_short() - 3); // -3 to get offset from instruction call not address of last argument byte
	if (stack_pop() < 0)
	{
		jump(jmp_offset);
	}
}


static inline void exec_op_icmpeq(void)
{
	int16_t jmp_offset = (int16_t)(get_arg_short() - 3); // -3 to get offset from instruction call not address of last argument byte
	if (stack_pop() ==  stack_pop())
	{
		jump(jmp_offset);
	}
}


static inline void exec_op_goto(void)
{
	int16_t jmp_offset = (int16_t)(get_arg_short() - 3); // -3 to get offset from instruction call not address of last argument byte
	jump(jmp_offset);
}


static inline void exec_op_ireturn(void)
{
	word_t ret_val = stack_pop();
	int old_nv = g_cpu->nv;

	g_cpu->sp = g_cpu->fp + 3;
	g_cpu->pc = stack_pop();
	g_cpu->fp = stack_pop();
	g_cpu->nv = stack_pop();
	g_cpu->lv = stack_pop();
	g_cpu->sp -= old_nv; // Remove all local variables and arguments from stack

	if (g_cpu->sp < -1 || 
		g_cpu->pc < 0 || 
		g_cpu->fp < 0 || 
		g_cpu->nv < 0 || 
		g_cpu->lv < 0)
	{
		fprintf(stderr, "[ERR] Program tried removing a stack frame that did not exist. In \"interpreter.c::exec_op_ireturn\".\n");
		destroy_ijvm_now();
	}
	stack_push(ret_val);
}


static inline void exec_op_ior(void)
{
	word_t b = stack_pop();
	word_t a = stack_pop();
	stack_push(a | b);
}


static inline void exec_op_invokevirtual(void)
{
	word_t offset = get_constant(get_arg_short()); // Move PC to return address while getting offset
	if (g_cpu->error_flag == true)
	{
		return;
	}
	int old_pc = g_cpu->pc;
	uint16_t num_args, num_locals;

	if (offset < 0 || offset >= g_cpu->code_mem_size)
	{
		fprintf(stderr, "[ERR] Invalid method address. In \"interpreter.c::exec_op_invokevirtual\".\n");
		destroy_ijvm_now();
	}

	g_cpu->pc = offset; // Move into method's memory
	num_args = (uint16_t)get_arg_short();
	num_locals = (uint16_t)get_arg_short();

	g_cpu->sp += num_locals;
	if (!stack_push(g_cpu->lv) ||
		!stack_push(g_cpu->nv) ||
		!stack_push(g_cpu->fp) ||
		!stack_push(old_pc))
	{
		fprintf(stderr, "[ERR] Failed to push onto the stack. In \"interpreter.c::exec_op_invokevirtual\".\n");
		destroy_ijvm_now();
	}

	g_cpu->fp = g_cpu->sp - 3;
	g_cpu->nv = num_args + num_locals;
	g_cpu->lv = g_cpu->fp - g_cpu->nv;

	if (g_cpu->lv < 0)
	{
		fprintf(stderr, "[ERR] Method provided an invalid number of arguments. In \"interpreter.c::exec_op_invokevirtual\".\n");
		destroy_ijvm_now();
	}

	if (g_cpu->error_flag == false)
	{
		memset(&g_cpu->stack[g_cpu->lv + num_args], 0, (uint16_t)num_locals * sizeof(uint32_t)); // Init local variables to 0
	}

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


static inline void exec_op_wide(void)
{
	next_op_wide = true;
	step();
	next_op_wide = false;
}


static inline void exec_op_in(void)
{
	stack_push(0);
	/*word_t c = getc(g_in_file);
	if (c == EOF)
	{
		stack_push(0);
	}
	else
	{
		stack_push(c);
	}*/
}


static inline void exec_op_out(void)
{
	char data = (char)stack_pop();
	//fprintf(g_out_file, "%c", data);
}


static inline void exec_op_err(void)
{
	g_cpu->error_flag = true;
}


static inline void exec_op_halt(void)
{
	g_cpu->halt_flag = true;
}


static inline void exec_op_newarray(void)
{
	word_t count = stack_pop();
	stack_push(start_array_creation(count));
}


static inline void exec_op_iaload(void)
{
	word_t array_ref = stack_pop();
	word_t i = stack_pop();
	stack_push(get_arr_element(array_ref, i));
}


static inline void exec_op_iastore(void)
{
	word_t array_ref = stack_pop();
	word_t i = stack_pop();
	word_t val = stack_pop();
	set_arr_element(array_ref, i, val);
}


static inline void exec_op_gc(void)
{
	gc_arrays();
}


static inline void exec_op_netbind(void)
{
	
}


static inline void exec_op_netconnect(void)
{

}


static inline void exec_op_netin(void)
{

}


static inline void exec_op_netout(void)
{

}


static inline void exec_op_netclose(void)
{

}


void run(void)
{
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
	int old_pc = g_cpu->pc;
	const char* op = op_decode((g_cpu->code_mem)[g_cpu->pc]);
#endif
	if (g_cpu->pc < 0 || g_cpu->pc >= g_cpu->code_mem_size)
	{
		g_cpu->error_flag = true;
		return false;
	}

	switch ((g_cpu->code_mem)[(g_cpu->pc)++])
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
	case OP_NEWARRAY:
		exec_op_newarray();
		break;
	case OP_IALOAD:
		exec_op_iaload();
		break;
	case OP_IASTORE:
		exec_op_iastore();
		break;
	case OP_GC:
		exec_op_gc();
		break;
	case OP_NETBIND:
		exec_op_netbind();
		break;
	case OP_NETCONNECT:
		exec_op_netconnect();
		break;
	case OP_NETIN:
		exec_op_netin();
		break;
	case OP_NETOUT:
		exec_op_netout();
		break;
	case OP_NETCLOSE:
		exec_op_netclose();
		break;
	default:
		fprintf(stderr, "[ERR] Invalid instruction. In \"interpreter.c::step\".\n");
		destroy_ijvm_now();
	}

#ifdef DEBUG
	if (g_cpu->error_flag == false)
	{
		dprintf("OPC: %-4i OP: %-14s", old_pc, op);
		print_cpu_registers(true);
		dprintf("  ");
		print_cpu_stack(true);
		dprintf("    ");
		print_cpu_local_vars(true);
		dprintf("    ");
		print_arr_refs(true);
		dprintf("\n");
	}
#endif

	return true;
}


bool finished(void)
{
	bool end_of_code = g_cpu->pc >= g_cpu->code_mem_size;
	bool cpu_err = g_cpu->error_flag;
	bool cpu_halt = g_cpu->halt_flag;

	if (end_of_code || cpu_err || cpu_halt)
	{
		if (cpu_halt)
		{
			dprintf("[HALT FLAG]\n");
		}
		else if (cpu_err)
		{
			dprintf("[ERR FLAG]\n");
		}
		else if (end_of_code)
		{
			dprintf("[TEXT END]\n");
		}
		return true;
	}
	else
	{
		return false;
	}
}


void init_interpreter(void)
{
	next_op_wide = false;
}
