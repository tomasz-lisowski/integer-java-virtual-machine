#include <time.h>


#include "init.h"
#include "terminate.h"
#include "interpreter.h"
#include "util.h"


static void print_usage(void)
{
    printf("Integer Java Virtual Machine\n");
    printf("Usage: ./ijvm <path/to/binary.ijvm> [<in_file>] [<out_file>]\n");
}


int main(int argc, char** argv)
{
    clock_t start, end;
    double cpu_time_used;
    start = clock();

    if (argc < 2)
    {
        print_usage();
        return 1;
    }

    if (init_ijvm(argv[1]) < 0)
    {
        fprintf(stderr, "Couldn't load binary %s\n", argv[1]);
        return 1;
    }

    if (argc >= 3)
    {
        FILE* in_file = fopen(argv[2], "r");
        if (in_file == NULL)
        {
            fprintf(stderr, "Couldn't open %s\n", argv[2]);
            return -1;
        }
        set_input(in_file);
        
    }

    if (argc == 4)
    {
        FILE* out_file = fopen(argv[3], "w+");
        if (out_file == NULL)
        {
            fprintf(stderr, "Couldn't open %s\n", argv[3]);
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
