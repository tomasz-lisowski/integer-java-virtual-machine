#include <ijvm.h>
#include "globals.h"


/**
* Return an empty initialized stack
**/
STACK_t stack_create(int size) {
    STACK s;
    s.top = 0;
    s.size = size;
    s.stack = malloc(sizeof(word_t) * size);
    return s;
}


/**
 * This function should return the word at the top of the stack of the current
 * frame, interpreted as a signed integer.
 **/
word_t tos(void)
{
    return 0;
}


/**
 * Returns the stack of the current frame as an array of integers,
 * with entry[0] being the very bottom of the stack and
 * entry[stack_size() - 1] being the top.
 **/
word_t* get_stack(void)
{
    return NULL;
}


/**
 * Returns the size of the stack of the current frame.
 **/
int stack_size(void)
{
    return 0;
}