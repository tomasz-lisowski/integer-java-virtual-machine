#include "util.h"


uint32_t swap_uint32(uint32_t num)
{
    return ((num >> 24) & 0xff) | ((num << 8) & 0xff0000) | ((num >> 8) & 0xff00) | ((num << 24) & 0xff000000);
}


char* op_decode(byte_t op)
{
	switch (op)
	{
	case OP_NOP:
		return "NOP          ";
		break;
	case OP_BIPUSH:
		return "BIPUSH       ";
		break;
	case OP_LDC_W:
		return "LDC_W        ";
		break;
	case OP_ILOAD:
		return "ILOAD        ";
		break;
	case OP_ISTORE:
		return "ISTORE       ";
		break;
	case OP_POP:
		return "POP          ";
		break;
	case OP_DUP:
		return "DUP          ";
		break;
	case OP_SWAP:
		return "SWAP         ";
		break;
	case OP_IADD:
		return "IADD         ";
		break;
	case OP_ISUB:
		return "ISUB         ";
		break;
	case OP_IAND:
		return "IAND         ";
		break;
	case OP_IINC:
		return "IINC         ";
		break;
	case OP_IFEQ:
		return "IFEQ         ";
		break;
	case OP_IFLT:
		return "IFLT         ";
		break;
	case OP_ICMPEQ:
		return "ICMPEQ       ";
		break;
	case OP_GOTO:
		return "GOTO         ";
		break;
	case OP_IRETURN:
		return "IRETURN      ";
		break;
	case OP_IOR:
		return "IOR          ";
		break;
	case OP_INVOKEVIRTUAL:
		return "INVOKEVIRTUAL";
		break;
	case OP_WIDE:
		return "WIDE         ";
		break;
	case OP_IN:
		return "IN           ";
		break;
	case OP_OUT:
		return "OUT          ";
		break;
	case OP_ERR:
		return "ERR          ";
		break;
	case OP_HALT:
		return "HALT         ";
		break;
	default:
		return "NULL         ";
	}
}