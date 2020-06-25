#include "terminate.h"


void destroy_ijvm(void)
{
    // ISO-IEC 9899: free(NULL) becomes a NOP
    net_destroy();
    arr_destroy();
    destroy_cpu();
    dprintf("[DESTROY IJVM]\n");
}


void destroy_ijvm_now(void)
{
    destroy_ijvm();
    exit(0);
}

