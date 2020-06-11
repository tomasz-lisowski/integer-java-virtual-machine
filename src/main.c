#include <time.h>


#include "init.h"
#include "terminate.h"
#include "interpreter.h"
#include "util.h"


static void print_help(void)
{
    printf("Usage: ./ijvm binary \n");
}


int main(int argc, char** argv)
{
    clock_t start, end;
    double cpu_time_used;
    start = clock();

    if (argc < 2)
    {
        print_help();
        return 1;
    }

    if (init_ijvm(argv[1]) < 0)
    {
        fprintf(stderr, "Couldn't load binary %s\n", argv[1]);
        return 1;
    }

    run();

    destroy_ijvm();

    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    fprintf(stderr, "[Took %f seconds]\n", cpu_time_used);
    
    return 0;
}
