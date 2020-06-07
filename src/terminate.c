#include "terminate.h"


void destroy_ijvm(void)
{
	if (g_cpu_ptr->code_mem != NULL)
	{
		free(g_cpu_ptr->code_mem);
	}
	if (g_cpu_ptr->data_mem != NULL)
	{
		free(g_cpu_ptr->data_mem);
	}
	if (g_cpu_ptr->stack != NULL)
	{
		free(g_cpu_ptr->stack);
	}
}