#ifndef INIT_H
#define INIT_H


#include "cpu.h"
#include "config.h"
#include "loader.h"
#include "util.h"
#include "interpreter.h"
#include "terminate.h"


/**
* Initializes the IJVM with the binary file found at the provided argument
* Note. You need to be able to re-initialize the IJVM after it has been started.
*
* Returns  0 on success
*         -1 on failure
**/
int init_ijvm(char* binary_path);


/**
* Sets the output of the IJVM instance to the provided file
**/
void set_output(FILE* f);


/**
* Sets the input of the IJVM instance to the provided file.
**/
void set_input(FILE* f);


#endif
