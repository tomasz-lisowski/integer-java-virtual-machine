#ifndef INTERPRETER_H
#define INTERPRETER_H

#include <stdbool.h> /* Contains bool type */

#include "types.h"
#include "cpu.h"


/**
 * Run the vm with the current state until the machine halts.
 **/
void run(void);


/**
 * Step (perform) one instruction and return.
 * In the case of WIDE, perform the whole WIDE_ISTORE or WIDE_ILOAD.
 * Returns true if an instruction was executed. Returns false if machine has
 * halted or encountered an error.
 **/
bool step(void);


/**
 * Check whether the machine has any more instructions to execute.
 *
 * A machine will stop running after:
 * - reaching the end of the text section
 * - encountering either the HALT/ERR instructions
 * - encountering an invalid instruction
 */
bool finished(void);

#endif