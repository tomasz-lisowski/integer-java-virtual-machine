#include "ijdb.h"


// Declarations of static functions
static void print_frame_stack(void);
static void print_frame_local_vars(void);
static void print_all_breakpoints(void);
static void print_func_calls(const uint32_t n);
static void print_breakpoint_msg(const uint32_t id);

static const char* addr_to_func_name(const uint32_t addr);
static uint32_t label_name_to_addr(const char* label_name);
static uint32_t at_breakpoint(void);

static void create_call_history(void);
static void store_func_call(void);
static void remove_last_func_call(void);

static void save_last_prog(const char* prog_path);

static void dbg_step(const bool step_log);
static void dbg_run(const bool single_step);

static void exec_step(void);
static void exec_continue(void);
static void exec_info(const char* about);
static void exec_backtrace(void);
static void exec_break(const char* offset);
static void exec_run(void);
static void exec_input(const char* prog_path);
static void exec_file(const char* prog_path);
static void exec_help(void);
static void exec_quit(void);

static void cmd_decoder(const char* cmd);
static void init_last_prog(void);
static void destroy_last_prog(void);
static void init_breakpoints(void);
static void destroy_breakpoints(void);
static void init_call_history(void);
static void destroy_call_history(void);
static void restart_debugger(void);


static DebuggerState_t dbg_state;
DebuggerState_t* g_dbg_state = &dbg_state;


static void print_frame_stack(void)
{
    printf("%15s [", "Stack");
    if (g_cpu->sp >= 0)
    {
        for (uint32_t i = (uint32_t)g_cpu->fp; i <= (uint32_t)g_cpu->sp; i++)
        {
            printf(" 0x%X", (g_cpu->stack)[i]);
        }
    }
    printf(" ]\n");
}


static void print_frame_local_vars(void)
{
    printf("%15s [", "Local Variables");
    for (uint32_t i = (uint32_t)g_cpu->lv; i < (uint32_t)(g_cpu->lv + g_cpu->nv); i++)
    {
        printf(" 0x%X", (g_cpu->stack)[i]);
    }
    printf(" ]\n");
}


static void print_all_breakpoints(void)
{
    uint32_t func_addr;

    if (g_dbg_state->brkpts.num == 0)
    {
        printf("There are no breakpoints.  Use the \"break\" command to create one.\n");
        return;
    }

    printf("Num      Address    Where\n");
    for (uint32_t i = 0; i < (uint32_t)g_dbg_state->brkpts.num; i++)
    {
        func_addr = g_dbg_state->brkpts.addrs[i];
        printf("%-8i 0x%08X %s\n", i + 1, func_addr, addr_to_func_name(func_addr));
    }
}


/**
* Print out last n function calls or all calls if n is 0.
* When n = 0, prints a numbered vertical list.
**/
static void print_func_calls(const uint32_t n)
{
    uint32_t func_addr = 0;
    uint32_t calls_printed = 0;
    uint32_t hist_offset = g_dbg_state->call_history_top;

    for (; hist_offset > 0 && (calls_printed != n || n == 0);)
    {
        func_addr = g_dbg_state->call_history[--hist_offset];
        if (n == 0)
        {
            printf("#%i  ", calls_printed + 1);
        }
        printf("0x%08X %s (", func_addr, addr_to_func_name(func_addr));
        if (hist_offset > 0)
        {
            for (uint32_t arg_offset = g_dbg_state->call_history[--hist_offset]; arg_offset > 0; arg_offset--)
            {
                printf(" 0x%08X", g_dbg_state->call_history[--hist_offset]);
                if (arg_offset - 1 > 0)
                {
                    printf(",");
                }
            }
        }
        printf(" )");
        if (n == 0)
        {
            printf("\n");
        }
        calls_printed++;
    }
}


/**
* Print out information regarding the breakpoint with a given id.
* Called is responsible for printing a new line character (after adding information if needed).
**/
static void print_breakpoint_msg(const uint32_t id)
{
    printf("Breakpoint %i, in ", id);
    print_func_calls(1);
}


/**
* Return a function name that contains a given address.
* Return  function name string on success
*         "??" string on failure (e.g. when there are no symbols in the binary)
**/
static const char* addr_to_func_name(const uint32_t addr)
{
    char* parent_symb_name = NULL;

    if (g_debug_data->func_label.num == 0)
    {
        return "??";
    }

    for (uint32_t i = 0; i < g_debug_data->func_label.num; i++)
    {
        if (addr == g_debug_data->func_label.addr[i])
        {
            parent_symb_name = get_func_name(i);
            break;
        }

        if (addr < g_debug_data->func_label.addr[i])
        {
            // Previous symbol had the closest address hence the "i - 1"
            parent_symb_name = get_func_name(i - 1);
            break;
        }
    }

    if (parent_symb_name == NULL)
    {
        return "??";
    }
    return parent_symb_name;
}


/**
* Return the address of a given label name. (function or section)
* Return  label address on success
*         SIZE_MAX_UINT32_T on failure (e.g. when there are no symbols in the binary)
**/
static uint32_t label_name_to_addr(const char* label_name)
{
    uint32_t symb_addr = SIZE_MAX_UINT32_T;

    if (g_debug_data->sec_label.num == 0)
    {
        return symb_addr;
    }

    // Looking in section names
    for (uint32_t i = 0; i < g_debug_data->sec_label.num; i++)
    {
        if (strcmp(label_name, get_section_name(i)) == 0)
        {
            symb_addr = g_debug_data->sec_label.addr[i];
            break;
        }
    }
    if (symb_addr != SIZE_MAX_UINT32_T || g_debug_data->func_label.num == 0)
    {
        return symb_addr;
    }

    // Looking in function names
    for (uint32_t i = 0; i < g_debug_data->func_label.num; i++)
    {
        if (strcmp(label_name, get_func_name(i)) == 0)
        {
            symb_addr = g_debug_data->func_label.addr[i];
            break;
        }
    }
    return symb_addr;
}


/**
* Uses the program counter to determine if any of the breakpoints have been reached.
* Return  number of breakpoint that was reached
*         0 if no breakpoint was reached
**/
static uint32_t at_breakpoint(void)
{
    const uint32_t curr_pc = (uint32_t)g_cpu->pc;
    for (uint32_t i = 0; i < (uint32_t)g_dbg_state->brkpts.num; i++)
    {
        if (g_dbg_state->brkpts.addrs[i] == curr_pc)
        {
            return i + 1;
        }
    }
    return 0;
}


/**
* Needs to be called on every step to create a function call history
* for use by the debugger.
**/
static void create_call_history(void)
{
    if (g_cpu->code_mem[g_cpu->pc] == OP_INVOKEVIRTUAL)
    {
        store_func_call();
        if (g_dbg_state->quit_flag == true)
        {
            return; // Something went wrong when storing the function call
        }
    }
    else if (g_cpu->code_mem[g_cpu->pc] == OP_IRETURN)
    {
        remove_last_func_call();
    }
}


/**
* Save the function call at current PC
**/
static void store_func_call(void)
{
    uint32_t func_addr;
    uint32_t func_num_args;

    uint32_t* tmp_call_history;
    uint32_t call_elements_num;
    uint32_t tmp_call_history_size;

    func_addr = (uint32_t)get_constant(get_code_short(g_cpu->pc + 1));
    func_num_args = (uint32_t)get_code_short((int)func_addr);
    call_elements_num = 1 + 1 + func_num_args;

    // Check if call history can fit the new call
    if (g_dbg_state->call_history_top + call_elements_num > g_dbg_state->call_history_size)
    {
        g_dbg_state->call_history_size = g_dbg_state->call_history_top + call_elements_num + 1024; // Store current function plus an extra margin

        tmp_call_history = g_dbg_state->call_history; // In case allocation fails
        tmp_call_history_size = g_dbg_state->call_history_size;
        g_dbg_state->call_history = (uint32_t*)realloc(g_dbg_state->call_history, tmp_call_history_size * sizeof(uint32_t));
        if (g_dbg_state->call_history == NULL)
        {
            fprintf(stderr, "[ERR] Failed to allocate memory. In \"ijdb.c::store_func_call\".\n");
            g_dbg_state->quit_flag = true;
            g_dbg_state->call_history = tmp_call_history;
            return;
        }
    }

    // Save all elements of the call
    for (uint32_t i = 0; i < func_num_args; i++)
    {
        g_dbg_state->call_history[g_dbg_state->call_history_top++] = (uint32_t)g_cpu->stack[(uint32_t)g_cpu->sp - i];
    }
    g_dbg_state->call_history[g_dbg_state->call_history_top++] = func_num_args;
    g_dbg_state->call_history[g_dbg_state->call_history_top++] = func_addr;

    /**
    * Call history after:
    *
    *  [empty]         <- top
    *  func addr
    *  arg num
    *  arg 1
    *  arg 2
    *  ...
    *  arg n - 1
    *  arg n
    *  ...
    **/
}


static void remove_last_func_call(void)
{
    uint32_t func_num_args;
    g_dbg_state->call_history_top -= 2; // Skip empty element and function address
    func_num_args = g_dbg_state->call_history[g_dbg_state->call_history_top];
    g_dbg_state->call_history_top -= func_num_args;
}


/**
* Store program path for furutre use when a re-load is needed
**/
static void save_last_prog(const char* prog_path)
{
    const char* prog_path_cpy = str_dup(prog_path); // In case pointers are the same (case prog_path == g_dbg_state->last_prog_path)
    free(g_dbg_state->last_prog_path);
    g_dbg_state->last_prog_path = str_dup(prog_path_cpy);
    if (g_dbg_state->last_prog_path == NULL)
    {
        printf("Failed to save path to last executed program.  "
            "Any subsequent attempts to re-load the program will fail.\n");
        return;
    }
    free(prog_path_cpy);
}


/**
* Step the VM and print out debugger information about the step (including breakpoint information)
**/
static void dbg_step(const bool step_log)
{
    const char* op = op_decode(g_cpu->code_mem[g_cpu->pc]);

    create_call_history();
    if (step_log)
    {
        printf("0x%08X: %-14s\n", g_cpu->pc, op);
    }
    step();

    if (at_breakpoint() != 0)
    {
        op = op_decode(g_cpu->code_mem[g_cpu->pc]);
        print_breakpoint_msg(at_breakpoint());
        printf(" when PC=0x%08X and OP=%-14s\n", g_cpu->pc, op);
    }
}


/**
* Step through program (taking into account breakpoints)
**/
static void dbg_run(const bool single_step)
{
    if (single_step)
    {
        dbg_step(true);
        if (finished())
        {
            g_dbg_state->prog_state = FINISHED;
        }
    }
    else
    {
        do
        {
            dbg_step(false);
        } 
        while ((at_breakpoint() == 0 && !finished()) || g_dbg_state->quit_flag == true);

        if (finished())
        {
            g_dbg_state->prog_state = FINISHED;
        }
    }
}


static void exec_step(void)
{
    if (g_dbg_state->prog_state == STARTED)
    {
        dbg_run(true);
    }
    else if (g_dbg_state->prog_state == LOADED || g_dbg_state->prog_state == FINISHED)
    {
        printf("Program is not running.  Use the \"start\" command.\n");
        return;
    }
    else if (g_dbg_state->prog_state == EMPTY)
    {
        printf("No program is loaded.  Use the \"file\" command.\n");
        return;
    }
}


static void exec_continue(void)
{
    if (g_dbg_state->prog_state == STARTED)
    {
        dbg_run(false);
    }
    else if (g_dbg_state->prog_state == LOADED || g_dbg_state->prog_state == FINISHED)
    {
        printf("Program is not running.  Use the \"start\" command.\n");
        return;
    }
    else if (g_dbg_state->prog_state == EMPTY)
    {
        printf("No program is loaded.  Use the \"file\" command.\n");
        return;
    }
}


static void exec_start(void)
{
    FILE* out_file = NULL;

    // Check if a re-start is needed
    if (g_dbg_state->prog_state >= STARTED)
    {
        restart_debugger();
    }
    
    if (g_dbg_state->prog_state == LOADED)
    {
        out_file = fopen(OUT_FILE, "w");
        if (out_file == NULL)
        {
            printf("Failed to create a file for program output. "
                "Program output will be redirected to \"/dev/null\" instead.\n");
            out_file = fopen("/dev/null", "w");
        }
        else
        {
            printf("Program output will be saved to \"%s\".\n", OUT_FILE);
        }
        set_output(out_file);

        printf("Starting program: %s\n", g_dbg_state->last_prog_path);
        if (at_breakpoint() != 0)
        {
            print_breakpoint_msg(at_breakpoint());
            printf(" when PC=0x%08X\n", g_cpu->pc);
        }
        g_dbg_state->prog_state = STARTED;
    }
    else if (g_dbg_state->prog_state == EMPTY)
    {
        printf("No program is loaded.  Use the \"file\" command.\n");
    }
}


static void exec_info(const char* about)
{
    if (g_dbg_state->prog_state == EMPTY)
    {
        printf("Program is empty.  Use the \"file\" command.\n");
        return;
    }

    if (strcmp(about, "frame") == 0)
    {
        if (g_dbg_state->prog_state != STARTED)
        {
            printf("Program is not running.  Use the \"start\" command.\n");
            return;
        }
        print_frame_stack();
        print_frame_local_vars();
    }
    else if (strcmp(about, "break") == 0)
    {
        print_all_breakpoints();
    }
    else
    {
        printf("Invalid argument.  Use the \"help\" command.\n");
    }
}


static void exec_backtrace(void)
{
    if (g_dbg_state->prog_state >= STARTED)
    {
        print_func_calls(0);
    }
    else
    {
        printf("Program is not running.  Use the \"start\" command.\n");
    }
}


static void exec_break(const char* offset)
{
    uint32_t offset_num;
    uint32_t tmp_mem_size;

    if (g_dbg_state->prog_state == EMPTY)
    {
        printf("No program is loaded.  Use the \"file\" command.\n");
        return;
    }

    tmp_mem_size = ++g_dbg_state->brkpts.num;
    g_dbg_state->brkpts.addrs = (uint32_t*)realloc(g_dbg_state->brkpts.addrs, tmp_mem_size * sizeof(uint32_t));
    if (g_dbg_state->brkpts.addrs == NULL)
    {
        fprintf(stderr, "[ERR] Failed to allocate memory. In \"ijdb.c::exec_break\".\n");
        g_dbg_state->quit_flag = true; // Failed to allocate memory
        return;
    }
    
    offset_num = label_name_to_addr(offset);
    if (offset_num == SIZE_MAX_UINT32_T)
    {
        offset_num = (uint32_t)strtol(offset, NULL, 0);
        if (g_debug_data->func_label.num > 0)
        {
            printf("Label with the name \"%s\" was not found.\n", offset);
        }
    }
    
    g_dbg_state->brkpts.addrs[g_dbg_state->brkpts.num - 1] = offset_num;
    printf("Breakpoint %i at offset \"0x%08X\".\n", g_dbg_state->brkpts.num, offset_num);
}


static void exec_run(void)
{
    exec_start();
    if (g_dbg_state->prog_state == STARTED)
    {
        dbg_run(false);
    }
}


static void exec_input(const char* prog_path)
{
    FILE* prog;
    if (g_dbg_state->prog_state == LOADED || g_dbg_state->prog_state == STARTED)
    {
        if (!(prog = fopen(prog_path, "rb")))
        {
            printf("File could not be opened.\n");
        }
        else
        {
            set_input(prog);
            printf("Input is \"%s\".\n", prog_path);
        }
    }
    else
    {
        if (g_dbg_state->prog_state == EMPTY)
        {
            printf("Program is empty.  Use the \"file\" command.\n");
        }
        else if (g_dbg_state->prog_state == FINISHED)
        {
            printf("Program is not running.  Use the \"start\" command.\n");
        }
    }
}


static void exec_file(const char* prog_path)
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
        printf("Program has been loaded.\n", prog_path);
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
        "break <offset> -- Sets a breakpoint at a given offset from the first instruction (decimal or hexadecimal with the '0x' prefix).\n"
        "start -- Starts running the program.\n"
        "step -- Executes the next instruction.\n"
        "run -- Re-start the program and run it until a breakpoint is reached or until it is finished.\n"
        "continue -- Executes program and stops at the first breakpoint.\n"
        "info frame -- Shows contents of the current frame (operand stack and local variables).\n"
        "info break -- Shows a list of all active breakpoints.\n"
        "backtrace -- Shows a call-stack of all frames including arguments.\n"
        "\n"
        "* Note that running \"file <path/to/binary>\" when a program is running,\n"
        "will terminate the current program and load the new one into a fresh VM.\n"
        "\n"
        "* Frame information will be printed out in the following format: \"[bottom ... top]\".\n"
        "* Breakpoint information will be printed in the following format: \"Breakpoint <id>, in <func_addr> <func_name> ( <arg1>, <arg2>, ... )\" but other information may also be appended to the end.\n"
        "* Backtrace information will be printed in the following format: \"#<id>  <func_addr> <func_name> ( <arg1>, <arg2>, ... )\".\n"
    );
}


static void exec_quit(void)
{
    destroy_ijvm();
    destroy_debug_data();
    destroy_debugger();
    g_dbg_state->quit_flag = true;
}


static void cmd_decoder(const char* whole_cmd)
{
    char cmd_cpy[strlen(whole_cmd)];
    char* cmd;
    char* arg1 = NULL;
    char* arg2 = NULL;

    strcpy(cmd_cpy, whole_cmd);
    cmd = strtok(cmd_cpy, " ");
    arg1 = strtok(NULL, " ");
    arg2 = strtok(NULL, " ");

    if (strcmp(whole_cmd, "step") == 0)
    {
        exec_step();
    }
    else if (strcmp(whole_cmd, "continue") == 0)
    {
        exec_continue();
    }
    else if (strcmp(cmd, "info") == 0 && arg1 != NULL && arg2 == NULL)
    {
        exec_info(arg1);
    }
    else if (strcmp(whole_cmd, "backtrace") == 0)
    {
        exec_backtrace();
    }
    else if (strcmp(cmd, "break") == 0 && arg1 != NULL && arg2 == NULL)
    {
        exec_break(arg1);
    }
    else if (strcmp(whole_cmd, "run") == 0)
    {
        exec_run();
    }
    else if (strcmp(cmd, "input") == 0 && arg1 != NULL && arg2 == NULL)
    {
        exec_input(arg1);
    }
    else if (strcmp(cmd, "file") == 0 && arg1 != NULL && arg2 == NULL)
    {
        exec_file(arg1);
    }
    else if (strcmp(whole_cmd, "start") == 0)
    {
        exec_start();
    }
    else if (strcmp(whole_cmd, "help") == 0)
    {
        exec_help();
    }
    else if (strcmp(whole_cmd, "quit") == 0)
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
    g_dbg_state->brkpts.num = 0;
}


static void destroy_breakpoints(void)
{
    free(g_dbg_state->brkpts.addrs);
}


static void init_call_history(void)
{
    g_dbg_state->call_history = (uint32_t*)malloc(2 * sizeof(uint32_t));

    // Insert "main" by default
    g_dbg_state->call_history[0] = 0;
    g_dbg_state->call_history[1] = 0;
    g_dbg_state->call_history_size = 2;
    g_dbg_state->call_history_top = 2;
}


static void destroy_call_history(void)
{
    free(g_dbg_state->call_history);
}


static void restart_debugger(void)
{
    destroy_ijvm();

    destroy_debug_data();
    init_debug_data();

    // Re-load last executed program
    if (init_ijvm(g_dbg_state->last_prog_path) == 0 &&
        load_debug_data(g_dbg_state->last_prog_path) == true)
    {
        printf("Program has been re-started.\n");
        g_dbg_state->prog_state = LOADED;
    }
    else
    {
        destroy_ijvm();

        destroy_debug_data();
        init_debug_data();

        destroy_debugger();
        init_debugger();

        printf("Failed to re-start program.  Use the \"file\" command.\n");
        return;
    }
}


void init_debugger()
{
    g_dbg_state->quit_flag = false;
    g_dbg_state->prog_state = EMPTY;
    
    init_call_history();
    init_last_prog();
    init_breakpoints();
}


void destroy_debugger(void)
{
    destroy_call_history();
    destroy_last_prog();
    destroy_breakpoints();
}


static void print_usage(const char* binary_path)
{
    printf("IJVM Debugger\n");
    printf("Usage: %s <path/to/binary.ijvm> [<in_file>] [<out_file>]\n", binary_path);
}


int main(int argc, char** argv)
{
    char* cmd;

    if (argc != 1)
    {
        print_usage(argv[0]);
    }

    printf("IJVM Debugger\n");
    printf("For help, type \"help\"\n");

    init_debug_data();
    init_debugger();
    using_history();

    while (g_dbg_state->quit_flag != true)
    {
        cmd = readline("\n(ijdb) ");
        if (strlen(cmd) == 0)
        {
            free(cmd);
            if (history_length == 0)
            {
                continue; // Nothing to repeat
            }
            cmd = str_dup(history_get(history_length)->line); // Repeat previous command
        }
        else
        {
            add_history(cmd); // Don't store repeated commands
        }
        
        cmd_decoder(cmd);
        free(cmd);
    }
    rl_clear_history();

    return 0;
}
