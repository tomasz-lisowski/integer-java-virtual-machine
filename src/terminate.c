#include "terminate.h"


void destroy_ijvm(void)
{
	free(g_cpu_ptr->code_mem);
	free(g_cpu_ptr->data_mem);
	free(g_cpu_ptr->stack);
}