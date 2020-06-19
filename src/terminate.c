#include "terminate.h"


void destroy_ijvm(void)
{
	// ISO-IEC 9899: free(NULL) becomes a NOP
	destroy_arrays();
	destroy_cpu();
	dprintf("[DESTROY IJVM]\n");
}
