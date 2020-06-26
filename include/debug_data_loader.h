#ifndef DEBUG_DATA_LOADER_H
#define DEBUG_DATA_LOADER_H


#include "types.h"
#include "util.h"
#include "init.h"
#include "ijdb.h"


/**
* Store one block (i.e. origin-size-data) of symbols
**/
typedef struct SymbolBlock_t
{
    int64_t num;
    uint32_t* addr;
    char* names;
    uint32_t* names_start; // Offsets to the start of each name in names memory
}SymbolBlock_t;


/**
* Store break information and symbol data
**/
typedef struct DebugData_t
{
    SymbolBlock_t func_label; // Function labels
    SymbolBlock_t sec_label; // Function section labels
}DebugData_t;


extern DebugData_t* g_debug_data;


/**
* Given an initialized VM, load debug data into the debugger.
* Load program's debug data (addresses + symbols) into global debug data.
* Requires that program is a valid binary and optionlly contains debug data.
* Return  true on success
*         false on failure
**/
bool load_debug_data(const char* prog_path);


/**
* Initialize debug data
**/
void init_debug_data(void);


/**
* Remove debug data from memory
**/
void destroy_debug_data(void);


/**
* Return the i'th function name from debug data
**/
char* get_func_name(const uint32_t i);


/**
* Return the i'th section name from debug data
**/
char* get_section_name(const uint32_t i);


#endif
