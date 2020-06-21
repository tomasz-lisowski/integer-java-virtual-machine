#ifndef IJDB_H
#define IJDB_H


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>


#include "types.h"
#include "init.h"
#include "terminate.h"
#include "interpreter.h"
#include "debug_data_loader.h"


#define OUT_FILE "ijdb_program_output.txt" // Program output is redirected to outside of the console


typedef enum EProgramState { EMPTY, LOADED, STARTED, FINISHED }EProgramState;


/**
* Store breakpoint information
**/
typedef struct Breakpoints_t
{
	uint32_t num;
	uint32_t* addrs;
}Breakpoints_t;


typedef struct DebuggerState_t
{
	bool quit_flag; // Quit flag is set to true when a critical error occurs
	char* last_prog_path;
	EProgramState prog_state;
	Breakpoints_t brkpts;

	uint32_t* call_history;
	uint32_t call_history_top;
	uint32_t call_history_size;
}DebuggerState_t;


extern DebuggerState_t* g_dbg_state;


/**
* Initialize the debugger state
**/
void init_debugger(void);


/**
* Safely remove all data associated with the debugger
**/
void destroy_debugger(void);


#endif
