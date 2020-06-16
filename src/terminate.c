#include "terminate.h"


void destroy_ijvm(void)
{
	// ISO-IEC 9899: free(NULL) becomes a NOP making function calls below safe
	destroy_all_arrays();
	free(g_cpu->stack);
	free(g_cpu->code_mem);
	free(g_cpu->const_mem);
}
