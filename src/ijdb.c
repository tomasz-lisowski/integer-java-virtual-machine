#include "ijdb.h"


// Declarations for static functions
static void save_last_prog(char* prog_path);
static void dbg_run(bool step_mode);
static void exec_step(void);
static void exec_continue(void);
static void exec_info(void);
static void exec_backtrace(void);
static void exec_break(char* addr);
static void exec_run(void);
static void exec_input(char* prog_path);
static void exec_file(char* prog_path);
static void exec_help(void);
static void exec_quit(void);
static void cmd_decoder(char* cmd);
static void init_last_prog(void);
static void destroy_last_prog(void);
static void init_breakpoints(void);
static void destroy_breakpoints(void);


static DebuggerState_t dbg_state;
DebuggerState_t* g_dbg_state = &dbg_state;


/**
* Store program path for furutre use when a re-load is needed
**/
static void save_last_prog(char* prog_path)
{
	char* prog_path_cpy = str_dup(prog_path); // In case pointers are the same (case prog_path == g_dbg_state->last_prog_path)
	free(g_dbg_state->last_prog_path);
	g_dbg_state->last_prog_path = str_dup(prog_path_cpy);
	if (g_dbg_state->last_prog_path == NULL)
	{
		printf("Failed to save path to last executed program. "
			"Any subsequent attempts to re-load the program will fail.\n");
		return;
	}
	free(prog_path_cpy);
}


/**
* Run program (taking into account breakpoints)
**/
static void dbg_run(bool step_mode)
{
	if (g_dbg_state->prog_state == FINISHED)
	{
		printf("Program is not running.\n");
		return;
	}
	else if (g_dbg_state->prog_state == EMPTY)
	{
		printf("Program is empty.\n");
		return;
	}
	else if (g_dbg_state->prog_state == LOADED || g_dbg_state->prog_state == STARTED)
	{
		if (step_mode)
		{
			g_dbg_state->prog_state = STARTED;
			step();
			if (finished())
			{
				g_dbg_state->prog_state = FINISHED;
			}
		}
		else
		{
			g_dbg_state->prog_state = STARTED;

			while (!finished())
			{
				step();
			}
			// TODO: Watch out for breakpoints
			g_dbg_state->prog_state = FINISHED;
		}
	}
	
}


static void exec_step(void)
{
	dbg_run(true);
}


static void exec_continue(void)
{
	dbg_run(false);
}


static void exec_info(void)
{

}


static void exec_backtrace(void)
{

}


static void exec_break(char* addr)
{

}


static void exec_run(void)
{
	if (g_dbg_state->prog_state >= STARTED)
	{
		destroy_ijvm();

		destroy_debug_data();
		init_debug_data();

		// Re-load last executed program
		if (init_ijvm(g_dbg_state->last_prog_path) != 0 || 
			load_debug_data(g_dbg_state->last_prog_path) != true)
		{
			destroy_ijvm();

			destroy_debug_data();
			init_debug_data();

			destroy_debugger();
			init_debugger();

			printf("Failed to re-load program. "
				"Path to the previouly executed program and associated breakpoints have been removed.\n");
			return;
		}
		else
		{
			g_dbg_state->prog_state = LOADED;
		}
	}
	dbg_run(false);
}


static void exec_input(char* prog_path)
{
	if (g_dbg_state->prog_state == LOADED || g_dbg_state->prog_state == STARTED)
	{
		set_input(fopen(prog_path, "r"));
	}
	else
	{
		if (g_dbg_state->prog_state == EMPTY)
		{
			printf("Can't set input, program is empty. Load a program using \"file <path/to/binary.ijvm>\" and try again.\n");
		}
		else if (g_dbg_state->prog_state == FINISHED)
		{
			printf("Can't set input, program has finished running. Reload the program and try again.\n");
		}
	}
}


static void exec_file(char* prog_path)
{
	destroy_ijvm();

	destroy_debug_data();
	init_debug_data();

	destroy_debugger();
	init_debugger();

	if (init_ijvm(prog_path) != 0 || 
		load_debug_data(prog_path) != true)
	{
		printf("Failed to load program.\n");
		destroy_ijvm();

		destroy_debug_data();
		init_debug_data();

		destroy_debugger();
		init_debugger();
		return;
	}
	else
	{
		g_dbg_state->prog_state = LOADED;
		save_last_prog(prog_path);
	}
}


static void exec_help(void)
{
	printf(
		"List of commands:\n"
		"\n"
		"file <path/to/binary> -- Loads a program into the IJVM.\n"
		"input <file> -- Sets the IJVM standard input to the specified file.\n"
		"break <addr> -- Sets a breakpoint at a given address in code.\n"
		"step -- Executes the next instruction.\n"
		"continue -- Executes program and stops at the first breakpoint.\n"
		"info -- Shows contents of the current frame (operand stack and local variables).\n"
		"backtrace -- Shows a call-stack of all frames including arguments.\n"
		"\n"
		"Note that running \"file <path/to/binary>\" when a program is running,\n"
		"will terminate the current program and load the new one into a fresh VM.\n"
	);
}


static void exec_quit(void)
{
	destroy_ijvm();
	destroy_debug_data();
	destroy_debugger();
	g_dbg_state->quit_flag = true;
}


static void cmd_decoder(char* cmd)
{
	char cmd_cpy[strlen(cmd)];
	strcpy(cmd_cpy, cmd);

	char* tok1; // Command
	char* tok2; // Argument
	char* tok3; // Argument 2
	tok1 = strtok(cmd_cpy, " ");
	tok2 = strtok(NULL, " ");
	tok3 = strtok(NULL, " ");

	if (strcmp(cmd, "step") == 0)
	{
		exec_step();
	}
	else if (strcmp(cmd, "continue") == 0)
	{
		exec_continue();
	}
	else if (strcmp(cmd, "info") == 0)
	{
		exec_info();
	}
	else if (strcmp(cmd, "backtrace") == 0)
	{
		exec_backtrace();
	}
	else if (strcmp(tok1, "break") == 0 && tok2 != NULL && tok3 == NULL)
	{
		exec_break(tok2);
	}
	else if (strcmp(cmd, "run") == 0)
	{
		exec_run();
	}
	else if (strcmp(tok1, "input") == 0 && tok2 != NULL && tok3 == NULL)
	{
		exec_input(tok2);
	}
	else if (strcmp(tok1, "file") == 0 && tok2 != NULL && tok3 == NULL)
	{
		exec_file(tok2);
	}
	else if (strcmp(cmd, "help") == 0)
	{
		exec_help();
	}
	else if (strcmp(cmd, "quit") == 0)
	{
		exec_quit();
	}
	else
	{
		printf("Undefined command and/or number of arguments: \"%s\". Take a look at \"help\".\n", cmd);
	}
}


static void init_last_prog(void)
{
	g_dbg_state->last_prog_path = NULL;
}


static void destroy_last_prog(void)
{
	free(g_dbg_state->last_prog_path);
}


static void init_breakpoints(void)
{
	g_dbg_state->brkpts.addrs = NULL;
	g_dbg_state->brkpts.num = -1;
}


static void destroy_breakpoints(void)
{
	free(g_dbg_state->brkpts.addrs);
}


void init_debugger()
{
	g_dbg_state->quit_flag = false;
	g_dbg_state->prog_state = EMPTY;

	init_last_prog();
	init_breakpoints();
}


void destroy_debugger(void)
{
	destroy_last_prog();
	destroy_breakpoints();
}


int main(int argc, char** argv)
{
	char* cmd;
	printf("IJVM Debugger\n");
	printf("For help, type \"help\"\n");

	init_debug_data();
	init_debugger();

	while (g_dbg_state->quit_flag != true)
	{
		cmd = readline("\n(ijdb) ");
		if (strlen(cmd) == 0)
		{
			free(cmd);
			continue; // Empty command
		}
		
		add_history(cmd);
		cmd_decoder(cmd);
		free(cmd);
	}
	rl_clear_history();

	return 0;
}
