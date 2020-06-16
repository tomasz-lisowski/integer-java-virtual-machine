#include "array.h"

static const uint32_t k_index_to_ref = 0xAA00000A;
static const uint32_t k_ref_to_index = 0x00FFFFF0;
static const uint32_t k_max_uint32_t = (~(uint32_t)0); // Used as a special value because there can never be this many arrays


/**
* Array identity memory holds 32bit array references
* and their corresponding 64bit pointers in memory at
* same indices in both arrays (refs and ptrs).
**/
static struct ArrIDMem
{
	uint32_t size;
	word_t* refs;
	uintptr_t* ptrs;
} arr_id_mem = { 0, NULL, NULL };


/**
* Called when creating the first ever array.
* It handles allocation and initialization of array identity memory.
* Sets CPU's error flag on failure.
**/
static void init_arr_id_mem(void)
{
	arr_id_mem.size = ARRAYS_MIN_NUM;
	arr_id_mem.refs = (word_t*)calloc(arr_id_mem.size, sizeof(word_t));
	arr_id_mem.ptrs = (uintptr_t*)calloc(arr_id_mem.size, sizeof(uintptr_t));

	if (arr_id_mem.refs == NULL || arr_id_mem.ptrs == NULL)
	{
		g_cpu->error_flag = true;
		return; // Could not allocate memory
	}
}


/**
* Return an index where the array pointer lies in array identity memory given an array reference
**/
static inline uint32_t ref_to_index(word_t arr_ref)
{
	return (k_ref_to_index & (uint32_t)arr_ref) >> 4;
}


/**
* Return an array reference of an array which lies at index 'i' in array identity memory
**/
static inline word_t index_to_ref(uint32_t i)
{
	return (word_t)(k_index_to_ref | (i << 4));
}


/**
* Resize array identity memory if possible.
* New size will be 8 times the initial size.
* Return  true on success
*         false on failure (and sets error flag on CPU)
**/
static bool resize_arr_id_mem(void)
{
	word_t* old_refs = arr_id_mem.refs;
	uintptr_t* old_ptrs = arr_id_mem.ptrs;
	arr_id_mem.size *= 8;

	arr_id_mem.refs = (word_t*)calloc(arr_id_mem.size, sizeof(word_t));
	if (arr_id_mem.refs == NULL)
	{
		arr_id_mem.size /= 8;
		arr_id_mem.refs = old_refs;
		g_cpu->error_flag = true; // Could not resize the references array
		return false;
	}

	arr_id_mem.ptrs = (uintptr_t*)calloc(arr_id_mem.size, sizeof(uintptr_t));
	if (arr_id_mem.ptrs == NULL)
	{
		arr_id_mem.size /= 8;
		arr_id_mem.ptrs = old_ptrs;
		g_cpu->error_flag = true; // Could not resize the pointers array
		return false;
	}

	memcpy(arr_id_mem.refs, old_refs, (arr_id_mem.size / 8) * sizeof(word_t));
	memcpy(arr_id_mem.ptrs, old_ptrs, (arr_id_mem.size / 8) * sizeof(uintptr_t));
	free(old_refs);
	free(old_ptrs);
	return true;
}


/**
* Find an un-claimed array index in array identity memory
* Return  array index on success
*         0 on failure (it will try to resize and run GC before failing)
* Sets CPU's error flag on failure.
**/
static uint32_t get_unclaimed_array_index(void)
{
	uint32_t arr_i = 0;
	for (; arr_i < arr_id_mem.size; arr_i++)
	{
		if (arr_id_mem.refs[arr_i] != 0)
		{
			continue;
		}
		return arr_i;
	}

	if (resize_arr_id_mem() != false)
	{
		return arr_i + 1;
	}
	else
	{
		// TODO: Run GC here then re-call self
		g_cpu->error_flag = true; // Heap is full
		return k_max_uint32_t;
	}
}


/**
* Allocate then initialize a new array
* Return  true on success
*         false on failure
* Sets CPU's error flag on failure.
**/
static bool create_array(uint32_t arr_i, uint32_t count)
{
	word_t* arr_ptr = (word_t*)malloc((count + 1) * sizeof(word_t));
	if (arr_ptr == NULL)
	{
		g_cpu->error_flag = true; // Could not allocate memory for new array
		return false;
	}

	arr_ptr[0] = (word_t)count; // First element stores array's size in 'elements' units

	arr_id_mem.refs[arr_i] = index_to_ref(arr_i);
	arr_id_mem.ptrs[arr_i] = (uintptr_t)arr_ptr;
	return true;
}


word_t get_arr_element(word_t arr_ref, word_t i)
{
	word_t* arr_ptr = (word_t*)arr_id_mem.ptrs[ref_to_index(arr_ref)];
	uint32_t arr_size = (uint32_t)arr_ptr[0];
	if (i + 1 >= 0 && i + 1 <= (word_t)arr_size)
	{
		return arr_ptr[i + 1]; // 0'th element stores size
	}
	else
	{
		g_cpu->error_flag = true; // Tried to access memory outside of array
		return 0;
	}
}


void set_arr_element(word_t arr_ref, word_t i, word_t new_val)
{
	word_t* arr_ptr = (word_t*)arr_id_mem.ptrs[ref_to_index(arr_ref)];
	uint32_t arr_size = (uint32_t)arr_ptr[0];
	if (i + 1 >= 0 && i + 1 <= (word_t)arr_size)
	{
		arr_ptr[i + 1] = new_val; // 0'th element stores size
	}
	else
	{
		g_cpu->error_flag = true; // Tried to access memory outside of array
	}
}


word_t start_array_creation(word_t count)
{
	uint32_t arr_i;
	if (arr_id_mem.refs == NULL)
	{
		init_arr_id_mem();
	}

	arr_i = get_unclaimed_array_index();
	if (arr_i == k_max_uint32_t)
	{
		g_cpu->error_flag = true; // Could not create an array
		return 0;
	}

	if (create_array(arr_i, (uint32_t)count))
	{
		return index_to_ref(arr_i);
	}
	else
	{
		g_cpu->error_flag = true; // Could not initialize the newly created array
		return 0;
	}
}


void destroy_all_arrays(void)
{
#ifdef DEBUG
	dprintf("[DESTROY_ARRAYS]\n");
#endif
	word_t* arr_ptr;
	for (uint32_t i = 0; i < arr_id_mem.size; i++)
	{
		arr_ptr = (word_t*)arr_id_mem.ptrs[i];
		free(arr_ptr); // ISO-IEC 9899: free(NULL) becomes a NOP making this line safe
	}
	free(arr_id_mem.ptrs);
	free(arr_id_mem.refs);

	arr_id_mem.size = 0;
	arr_id_mem.refs = NULL;
	arr_id_mem.ptrs = NULL;
}


/**
* Mark all inaccessible arrays and return the number of marked arrays
**/
static uint32_t mark_arrays(uint32_t* marked_arrays)
{
	word_t mem_data;
	word_t ref;
	uint32_t num_marked = 0;

	for (int mem_ptr = 0; mem_ptr <= g_cpu->sp; mem_ptr++)
	{
		mem_data = g_cpu->stack[mem_ptr];
		if (mem_data < 0xAA00000A || mem_data > 0xAAFFFFFA)
		{
			// Entry in memory is definately not an array reference
			continue;
		}

		for (uint32_t arr_id_ptr = 0; arr_id_ptr < arr_id_mem.size; arr_id_ptr++)
		{
			ref = arr_id_mem.refs[arr_id_ptr];
			if (ref == 0)
			{
				// Skip unclaimed (0) array reference
				continue;
			}

			if (mem_data == ref)
			{
				marked_arrays[num_marked++] = ref_to_index(ref);
				break;
			}
		}
	}

	return num_marked + 1;
}


/**
* Sweep all inaccessible arrays
**/
static void sweep_arrays(uint32_t* marked_arrays, uint32_t num_marked)
{
	for (uint32_t i = 0, j = 0; i < arr_id_mem.size; i++)
	{
		if (arr_id_mem.refs[i] == 0)
		{
			continue;
		}
		if (marked_arrays[j] == i)
		{
			j++;
			continue;
		}

		// Remove array
		arr_id_mem.refs[i] = 0;
		free((word_t*)arr_id_mem.ptrs[i]);
		arr_id_mem.ptrs[i] = 0;
	}
}


void gc_arrays(void)
{
	uint32_t marked_arrays[arr_id_mem.size];
	uint32_t num_marked;
	
	num_marked = mark_arrays(marked_arrays);
	sweep_arrays(marked_arrays, num_marked);
}


#ifdef DEBUG
void print_arr_refs(bool compact)
{
	if (compact)
	{
		dprintf("AR[");
		for (uint32_t i = 0; i < arr_id_mem.size; i++)
		{
			if (arr_id_mem.refs[i] != 0)
			{
				dprintf(" 0x%X", arr_id_mem.refs[i]);
			}
		}
		dprintf(" ]");
	}
	else
	{
		dprintf("AR\n");
		for (uint32_t i = 0; i < arr_id_mem.size; i++)
		{
			if (arr_id_mem.refs[i] != 0)
			{
				dprintf("\t0x%X\n", arr_id_mem.refs[i]);
			}
		}
	}
}
#endif