#include "terminate.h"


void destroy_ijvm(void)
{
	destroy_arrays();

	// ISO-IEC 9899: free(NULL) becomes a NOP making function calls below safe
	free(g_cpu->stack);
	free(g_cpu->code_mem);
	free(g_cpu->const_mem);
}
