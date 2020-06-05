#include <ijvm.h>
#include "globals.h"
#include <stdlib.h>

int pc = 0;


/**
 * Returns the value of the program counter (as an offset from the first instruction).
 **/
int get_program_counter(void)
{
    return pc;
}


/**
 * @param i, index of variable to obtain.
 * @return Returns the i:th local variable of the current frame.
 **/
word_t get_local_variable(int i)
{
    return 0;
}


/**
 * @param i, index of the constant to obtain
 * @return The constant at location i in the constant pool.
 **/
word_t get_constant(int i)
{
    return b.const_pool[i];
}


/**
 * Step (perform) one instruction and return.
 * In the case of WIDE, perform the whole WIDE_ISTORE or WIDE_ILOAD.
 * Returns true if an instruction was executed. Returns false if machine has
 * halted or encountered an error.
 **/
bool step(void)
{
    fprintf(stderr, "pc=%i\top=%X\n", get_program_counter(), get_instruction());
    pc++;
    return true;
}


/**
 * Check whether the machine has any more instructions to execute.
 *
 * A machine will stop running after:
 * - reaching the end of the text section
 * - encountering either the HALT/ERR instructions
 * - encountering an invalid instruction
 */
bool finished(void)
{
    bool EOT = get_program_counter() == b.text_size;
    bool HALT = get_instruction() == OP_HALT;
    bool ERR = get_instruction() == OP_ERR;

    if (EOT || HALT || ERR)
    {
        if (EOT) {
            fprintf(stderr, "[EOT]\n");
        }
        else if (HALT)
        {
            fprintf(stderr, "[HALT]\n");
        }
        else if (ERR)
        {
            fprintf(stderr, "[ERR]\n");
        }
        return true;
    }
    else
    {
        return false;
    }

    // TODO: Add check of invalid instruction, maybe the step function can mark a flag that this func checks.
}


/**
 * Run the vm with the current state until the machine halts.
 **/
void run(void)
{   
    fprintf(stderr, "[RUN START]\n");
    while (!finished())
    {
        step();
    }
    fprintf(stderr, "[RUN STOP]\n");
}


/**
 * @return The value of the current instruction represented as a byte_t.
 *
 * This should NOT increase the program counter.
 **/
byte_t get_instruction(void)
{
    return b.text[pc];
}


/**
 * Sets the output of the IJVM instance to the provided file
 **/
void set_output(FILE* f)
{

}


/**
 * Sets the input of the IJVM instance to the provided file.
 **/
void set_input(FILE* f)
{

}


/**
 * Destroys a vm, that is to say, free all memory associated with the machine
 * and allow for a new call to init_ijvm().
 */
void destroy_ijvm(void)
{
    free(b.const_pool);
    free(b.text);
}