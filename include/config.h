/**
* Configuration constants for the VM
**/

#ifndef CONFIG_H
#define CONFIG_H

/**
* Indicates the minimum size of the stack.
* Stack is automatically resized by a factor of 8 hence a good choice
* of this number would allow the stack to resize some integer number of times
* before becoming too big (4294967296).
* E.g. (4096 * 4) * 8^6 = 4294967296 so the stack can be resized upto 6 times.
**/
#define STACK_MIN_SIZE 4096 // Elements


/**
* The array keeping track of array references will start at this size then resize
* every time more space is needed when a new array is created.
**/
#define ARRAYS_MIN_NUM 64 // Elements
/**
* From AA00000A to AAFFFFFA (inclusive)
* which means 16^5 = 1048576 unique array references
**/
#define ARRAYS_MAX_NUM 1048576 // Elements


/**
* Start at this number of connections then resize if necessary
**/
#define NET_CONN_MIN_NUM 16 // Connections
/**
* From EE00000E to EEFFFFFE (inclusive)
* which means 16^5 = 1048576 unique network references
**/
#define NET_CONN_MAX_NUM 128 // Connnections
/**
* Maximum queue length for pending connections
**/
#define NET_MAX_BACKLOG 1


#endif
