#ifndef TERMINATE_H
#define TERMINATE_H


#include <stdlib.h>


#include "cpu.h"
#include "array.h"
#include "net.h"


/**
* Destroys a vm, that is to say, free all memory associated with the machine
* and allow for a new call to init_ijvm().
*/
void destroy_ijvm(void);


/**
* Close the VM immediately. This should only be used when an error occurs.
**/
void destroy_ijvm_now(void);


#endif
