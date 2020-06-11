#ifndef BYTECODE_H
#define BYTECODE_H

#define OP_NOP            ((byte_t) 0x00)
#define OP_BIPUSH         ((byte_t) 0x10)
#define OP_LDC_W          ((byte_t) 0x13)
#define OP_ILOAD          ((byte_t) 0x15)
#define OP_ISTORE         ((byte_t) 0x36)
#define OP_POP            ((byte_t) 0x57)
#define OP_DUP            ((byte_t) 0x59)
#define OP_SWAP           ((byte_t) 0x5F)
#define OP_IADD           ((byte_t) 0x60)
#define OP_ISUB           ((byte_t) 0x64)
#define OP_IAND           ((byte_t) 0x7E)
#define OP_IINC           ((byte_t) 0x84)
#define OP_IFEQ           ((byte_t) 0x99)
#define OP_IFLT           ((byte_t) 0x9B)
#define OP_ICMPEQ         ((byte_t) 0x9F)
#define OP_GOTO           ((byte_t) 0xA7)
#define OP_IRETURN        ((byte_t) 0xAC)
#define OP_IOR            ((byte_t) 0xB0)
#define OP_INVOKEVIRTUAL  ((byte_t) 0xB6)
#define OP_WIDE           ((byte_t) 0xC4)
#define OP_IN             ((byte_t) 0xFC)
#define OP_OUT            ((byte_t) 0xFD)
#define OP_ERR            ((byte_t) 0xFE)
#define OP_HALT           ((byte_t) 0xFF)

#endif
