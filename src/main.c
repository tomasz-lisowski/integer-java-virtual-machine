#include <time.h>
#include <signal.h>


#include "init.h"
#include "terminate.h"
#include "interpreter.h"
#include "util.h"


// Declarations of static functions
static void print_usage(void);
static void interrupt_handler(const int sig);


static void print_usage(void)
{
    printf("Integer Java Virtual Machine\n");
    printf("Usage: ./ijvm <path/to/binary.ijvm> [<in_file>] [<out_file>]\n");
}


static void interrupt_handler(const int sig)
{
    signal(sig, SIG_IGN);
    destroy_ijvm_now();
}


int main(int argc, char** argv)
{
    clock_t start, end;
    double cpu_time_used;
    start = clock();
    signal(SIGINT, interrupt_handler);

    if (argc < 2)
    {
        print_usage();
        return 1;
    }

    if (init_ijvm(argv[1]) < 0)
    {
        fprintf(stderr, "[ERR] Failed to load binary %s. In \"main.c::main\".\n", argv[1]);
        return 1;
    }

    if (argc >= 3)
    {
        FILE* in_file = fopen(argv[2], "r");
        if (in_file == NULL)
        {
            fprintf(stderr, "[ERR] Failed to open %s. In \"main.c::main\".\n", argv[2]);
            return -1;
        }
        set_input(in_file);
    }

    if (argc == 4)
    {
        FILE* out_file = fopen(argv[3], "w+");
        if (out_file == NULL)
        {
            fprintf(stderr, "[ERR] Failed to open %s. In \"main.c::main\".\n", argv[3]);
            return -1;
        }
        set_output(out_file);
    }

    run();

    destroy_ijvm();

    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    fprintf(stderr, "[Took %f seconds]\n", cpu_time_used);
    
    return 0;
}
