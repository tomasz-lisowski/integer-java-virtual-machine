#include "util.h"


uint32_t swap_uint32(uint32_t num)
{
    return ((num >> 24) & 0xff) | ((num << 8) & 0xff0000) | ((num >> 8) & 0xff00) | ((num << 24) & 0xff000000);
}


uint64_t power(uint32_t base, uint32_t power)
{
	if (power == 0)
	{
		return 1;
	}
	for (uint32_t i = 0; i < power; i++)
	{
		base *= power;
	}
	return base;
}


const char* op_decode(byte_t op)
{
    switch (op)
    {
    case OP_NOP:
        return "NOP";
        break;
    case OP_BIPUSH:
        return "BIPUSH";
        break;
    case OP_LDC_W:
        return "LDC_W";
        break;
    case OP_ILOAD:
        return "ILOAD";
        break;
    case OP_ISTORE:
        return "ISTORE";
        break;
    case OP_POP:
        return "POP";
        break;
    case OP_DUP:
        return "DUP";
        break;
    case OP_SWAP:
        return "SWAP";
        break;
    case OP_IADD:
        return "IADD";
        break;
    case OP_ISUB:
        return "ISUB";
        break;
    case OP_IAND:
        return "IAND";
        break;
    case OP_IINC:
        return "IINC";
        break;
    case OP_IFEQ:
        return "IFEQ";
        break;
    case OP_IFLT:
        return "IFLT";
        break;
    case OP_ICMPEQ:
        return "ICMPEQ";
        break;
    case OP_GOTO:
        return "GOTO";
        break;
    case OP_IRETURN:
        return "IRETURN";
        break;
    case OP_IOR:
        return "IOR";
        break;
    case OP_INVOKEVIRTUAL:
        return "INVOKEVIRTUAL";
        break;
    case OP_WIDE:
        return "WIDE";
        break;
    case OP_IN:
        return "IN";
        break;
    case OP_OUT:
        return "OUT";
        break;
    case OP_ERR:
        return "ERR";
        break;
    case OP_HALT:
        return "HALT";
        break;
    case OP_NEWARRAY:
        return "NEWARRAY";
        break;
    case OP_IALOAD:
        return "IALOAD";
        break;
    case OP_IASTORE:
        return "IASTORE";
        break;
    case OP_GC:
        return "GC";
        break;
    case OP_NETBIND:
        return "NETBIND";
        break;
    case OP_NETCONNECT:
        return "NETCONNECT";
        break;
    case OP_NETIN:
        return "NETIN";
        break;
    case OP_NETOUT:
        return "NETOUT";
        break;
    case OP_NETCLOSE:
        return "NETCLOSE";
        break;
    default:
        return "NULL";
    }
}


void print_cpu_state(bool compact)
{
    dprintf("[CPU DUMP]\n");
    print_cpu_mem_size(compact);
    print_cpu_registers(compact);
    print_cpu_const_mem(compact);
    print_cpu_code_mem(compact);
    print_cpu_stack(compact);
    print_cpu_local_vars(compact);
}


void print_cpu_mem_size(bool compact)
{
    if (compact)
    {
        dprintf("Mem Size [");
        dprintf(" DM:%i", g_cpu->const_mem_size);
        dprintf(" CM:%i", g_cpu->code_mem_size);
        dprintf(" ST:%i", g_cpu->stack_size);
        dprintf(" V:%i", g_cpu->nv);
        dprintf(" ]");
    }
    else
    {
        dprintf("Mem Size\n");
        dprintf("\tConstant Pool Size: %i\n", g_cpu->const_mem_size);
        dprintf("\tText Size: %i\n", g_cpu->code_mem_size);
        dprintf("\tStack Size: %i\n", g_cpu->stack_size);
        dprintf("\tVariable Memory Size: %i\n", g_cpu->nv);
    }
}


void print_cpu_const_mem(bool compact)
{
    if (compact)
    {
        dprintf("C[");
        for (uint32_t i = 0; i < (uint32_t)g_cpu->const_mem_size / sizeof(word_t); i++)
        {
            dprintf(" %i", (g_cpu->const_mem)[i]);
        }
        dprintf(" ]");
    }
    else
    {
        dprintf("Consts\n");
        for (uint32_t i = 0; i < (uint32_t)g_cpu->const_mem_size / sizeof(word_t); i++)
        {
            dprintf("\t%i\n", (g_cpu->const_mem)[i]);
        }
    }
}


void print_cpu_code_mem(bool compact)
{
    if (compact)
    {
        dprintf("OP[");
        for (uint32_t i = 0; i < (uint32_t)g_cpu->code_mem_size / sizeof(byte_t); i++)
        {
            dprintf(" 0x%X", (g_cpu->code_mem)[i]);
        }
        dprintf(" ]");
    }
    else
    {
        dprintf("OPs\n");
        for (uint32_t i = 0; i < (uint32_t)g_cpu->code_mem_size / sizeof(byte_t); i++)
        {
            dprintf("\t0x%X\n", (g_cpu->code_mem)[i]);
        }
    }
}


void print_cpu_registers(bool compact)
{
    if (compact)
    {
        dprintf("PC:%-4i", g_cpu->pc);
        dprintf("  SP:%-4i", g_cpu->sp);
        dprintf("  FP:%-4i", g_cpu->fp);
        dprintf("  LV:%-4i", g_cpu->lv);
        dprintf("  NV:%-4i", g_cpu->nv);
    }
    else
    {
        dprintf("Regs\n");
        dprintf("\tPC: %i\n", g_cpu->pc);
        dprintf("\tSP: %i\n", g_cpu->sp);
        dprintf("\tFP: %i\n", g_cpu->fp);
        dprintf("\tLV: %i\n", g_cpu->lv);
        dprintf("\tNV: %i\n", g_cpu->nv);
    }
}


void print_cpu_stack(bool compact)
{
    if (compact)
    {
        dprintf("S[");
        for (int64_t i = (uint32_t)g_cpu->fp; i <= g_cpu->sp; i++)
        {
            dprintf(" %i", (g_cpu->stack)[i]);
        }
        dprintf(" ]");
    }
    else
    {
        dprintf("Stack\n");
        for (int64_t i = (uint32_t)g_cpu->fp; i <= g_cpu->sp; i++)
        {
            dprintf("\t%i\n", (g_cpu->stack)[i]);
        }
    }
}


void print_cpu_local_vars(bool compact)
{
    if (compact)
    {
        dprintf("LV[");
        for (uint32_t i = (uint32_t)g_cpu->lv; i < (uint32_t)(g_cpu->lv + g_cpu->nv); i++)
        {
            dprintf(" %i", (g_cpu->stack)[i]);
        }
        dprintf(" ]");
    }
    else
    {
        dprintf("LV\n");
        for (uint32_t i = (uint32_t)g_cpu->lv; i < (uint32_t)(g_cpu->lv + g_cpu->nv); i++)
        {
            dprintf("\t%i\n", (g_cpu->stack)[i]);
        }
    }
}
