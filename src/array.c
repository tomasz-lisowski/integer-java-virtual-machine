#include "array.h"


// Declarations for static functions
static void init_arr_id_mem(void);
static inline uint32_t ref_to_index(word_t arr_ref);
static inline word_t index_to_ref(uint32_t i);
static bool resize_arr_id_mem(void);
static uint32_t get_unclaimed_array_index(void);
static bool create_array(uint32_t arr_i, uint32_t count);
static void mark_arrays(bool* marked_arrays);
static uint32_t sweep_arrays(bool* marked_arrays);


static const uint32_t k_index_to_ref = 0xAA00000A;
static const uint32_t k_ref_to_index = 0x00FFFFF0;
static const uint32_t k_max_uint32_t = SIZE_MAX_UINT32_T; // Used as a special value because there can never be this many arrays


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
	uint32_t tmp_mem_size;
	arr_id_mem.size = ARRAYS_MIN_NUM;
	tmp_mem_size = arr_id_mem.size;
	arr_id_mem.refs = (word_t*)calloc(tmp_mem_size, sizeof(word_t));
	tmp_mem_size = arr_id_mem.size;
	arr_id_mem.ptrs = (uintptr_t*)calloc(tmp_mem_size, sizeof(uintptr_t));
	if (arr_id_mem.refs == NULL || arr_id_mem.ptrs == NULL)
	{
		fprintf(stderr, "[ERR] Failed to allocate memory. In \"array.c::init_arr_id_mem\".\n");
		destroy_ijvm_now();
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
	word_t* tmp_refs = arr_id_mem.refs;
	uintptr_t* tmp_ptrs = arr_id_mem.ptrs;
	uint32_t tmp_mem_size;

	arr_id_mem.size *= 8;
	if (arr_id_mem.size > (k_ref_to_index >> 4))
	{
		fprintf(stderr, "[ERR] Maximum number of arrays has been reached. In \"array.c::resize_arr_id_mem\".\n");
		destroy_ijvm_now();
	}

	tmp_mem_size = arr_id_mem.size;
	arr_id_mem.refs = (word_t*)calloc(tmp_mem_size, sizeof(word_t));
	tmp_mem_size = arr_id_mem.size;
	arr_id_mem.ptrs = (uintptr_t*)calloc(tmp_mem_size, sizeof(uintptr_t));
	if (arr_id_mem.refs == NULL || arr_id_mem.ptrs == NULL)
	{
		arr_id_mem.refs = tmp_refs;
		arr_id_mem.ptrs = tmp_ptrs;
		if (gc_arrays() != 0)
		{
			return resize_arr_id_mem(); // Run GC to be sure memory allocation error is not caused by garbage
		}
		fprintf(stderr, "[ERR] Failed to allocate memory. In \"array.c::resize_arr_id_mem\".\n");
		destroy_ijvm_now();
	}

	memcpy(arr_id_mem.refs, tmp_refs, (arr_id_mem.size / 8) * sizeof(word_t));
	memcpy(arr_id_mem.ptrs, tmp_ptrs, (arr_id_mem.size / 8) * sizeof(uintptr_t));
	free(tmp_refs);
	free(tmp_ptrs);
	return true;
}


/**
* Find an un-claimed array index in array identity memory
* Return  array index on success
*         0 on failure (it will try to resize and run GC before failing)
**/
static uint32_t get_unclaimed_array_index(void)
{
	uint32_t arr_i = 0;
	// Find an unclaimed reference in arr_id_mem.refs
	for (; arr_i < arr_id_mem.size; arr_i++)
	{
		if (arr_id_mem.refs[arr_i] != 0)
		{
			continue;
		}
		return arr_i;
	}

	// All references in arr_id_mem.refs are taken
	if (resize_arr_id_mem() != false)
	{
		return arr_i;
	}
	else
	{
		if (gc_arrays() != 0)
		{
			return get_unclaimed_array_index(); // Run GC to be sure memory allocation error is not caused by garbage
		}
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
	uint32_t tmp_count = count;
	word_t arr_ref = index_to_ref(arr_i);
	word_t* arr_ptr = (word_t*)malloc((tmp_count + 1) * sizeof(word_t));
	if (arr_ptr == NULL)
	{
		if (gc_arrays() != 0)
		{
			return create_array(arr_i, count); // Run GC to be sure memory allocation error is not caused by garbage
		}
		fprintf(stderr, "[ERR] Failed to allocate memory. In \"array.c::create_array\".\n");
		destroy_ijvm_now();
	}

	arr_ptr[0] = (word_t)count; // First element stores array's size in 'elements' units
	
	arr_id_mem.refs[arr_i] = arr_ref;
	arr_id_mem.ptrs[arr_i] = (uintptr_t)arr_ptr;
	return true;
}


word_t get_arr_element(word_t arr_ref, word_t i)
{
	uint32_t arr_i;
	word_t* arr_ptr;
	uint32_t arr_size;

	if ((arr_ref & k_index_to_ref) != 0xAA00000A)
	{
		fprintf(stderr, "[ERR] Invalid array reference. In \"array.c::get_arr_element\".\n");
		destroy_ijvm_now();
	}

	arr_i = ref_to_index(arr_ref);
	if (arr_i >= arr_id_mem.size)
	{
		fprintf(stderr, "[ERR] Invalid array index. In \"array.c::get_arr_element\".\n");
		destroy_ijvm_now();
	}

	arr_ptr = (word_t*)arr_id_mem.ptrs[arr_i];
	if (arr_ptr == NULL)
	{
		fprintf(stderr, "[ERR] Program tried to access a non-existent array. In \"array.c::get_arr_element\".\n");
		destroy_ijvm_now();
	}

	arr_size = (uint32_t)arr_ptr[0];
	if (i + 1 < 0 || i + 1 > (word_t)arr_size)
	{
		fprintf(stderr, "[ERR] Program tried to access a memory outside of an array. In \"array.c::get_arr_element\".\n");
		destroy_ijvm_now();
	}

	return arr_ptr[i + 1]; // First element is at index 1 (0'th element stores array size)
}


void set_arr_element(word_t arr_ref, word_t i, word_t new_val)
{
	uint32_t arr_i;
	word_t* arr_ptr;
	uint32_t arr_size;

	if ((arr_ref & k_index_to_ref) != 0xAA00000A)
	{
		fprintf(stderr, "[ERR] Invalid array reference. In \"array.c::get_arr_element\".\n");
		destroy_ijvm_now();
	}

	arr_i = ref_to_index(arr_ref);
	if (arr_i >= arr_id_mem.size)
	{
		fprintf(stderr, "[ERR] Invalid array index. In \"array.c::set_arr_element\".\n");
		destroy_ijvm_now();
	}

	arr_ptr = (word_t*)arr_id_mem.ptrs[arr_i];
	if (arr_ptr == NULL)
	{
		fprintf(stderr, "[ERR] Program tried to access a non-existent array. In \"array.c::set_arr_element\".\n");
		destroy_ijvm_now();
	}

	arr_size = (uint32_t)arr_ptr[0];
	if (i + 1 < 0 || i + 1 > (word_t)arr_size)
	{
		fprintf(stderr, "[ERR] Program tried to access a memory outside of an array. In \"array.c::set_arr_element\".\n");
		destroy_ijvm_now();
	}

	arr_ptr[i + 1] = new_val; // First element is at index 1 (0'th element stores array size)
}


word_t start_array_creation(word_t count)
{
	uint32_t arr_i;
	if (arr_id_mem.refs == NULL)
	{
		init_arr_id_mem();
		if (g_cpu->error_flag == true)
		{
			return 0;
		}
	}

	arr_i = get_unclaimed_array_index();
	if (arr_i == k_max_uint32_t)
	{
		fprintf(stderr, "[ERR] Failed to acquire an unclaimed array reference. In \"array.c::start_array_creation\".\n");
		destroy_ijvm_now();
	}

	if (create_array(arr_i, (uint32_t)count))
	{
		return index_to_ref(arr_i);
	}
	else
	{
		fprintf(stderr, "[ERR] Failed to create an array. In \"array.c::start_array_creation\".\n");
		destroy_ijvm_now();
	}
	return 0;
}


void destroy_arrays(void)
{
	word_t* arr_ptr;
	if (arr_id_mem.refs == NULL)
	{
		return;
	}
	for (uint32_t i = 0; i < arr_id_mem.size; i++)
	{
		if (arr_id_mem.refs[i] != 0)
		{
			arr_ptr = (word_t*)arr_id_mem.ptrs[i];
			free(arr_ptr); // ISO-IEC 9899: free(NULL) becomes a NOP making this line safe
		}
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
static void mark_arrays(bool* marked_arrays)
{
	word_t mem_data;
	word_t ref;

	// For scanning array element
	uint32_t arr_i;
	word_t* arr_ptr;
	uint32_t arr_size;

	// Look for array references on the operand stack and in local variables
	for (int mem_ptr = 0; mem_ptr <= g_cpu->sp; mem_ptr++)
	{
		mem_data = g_cpu->stack[mem_ptr];
		if ((mem_data & (word_t)k_index_to_ref) == 0xAA00000A) 
		{
			continue; // Entry in memory is definately not an array reference
		}

		for (uint32_t arr_id_ptr = 0; arr_id_ptr < arr_id_mem.size; arr_id_ptr++)
		{
			ref = arr_id_mem.refs[arr_id_ptr];
			if (ref == 0)
			{
				continue; // Skip unclaimed (0) array reference
			}

			if (mem_data == ref)
			{
				marked_arrays[ref_to_index(ref)] = true;
				break;
			}
		}
	}

	// Look for array references inside of arrays that are reachable via the stack
	for (uint32_t i = 0; i < arr_id_mem.size; i++)
	{
		if (marked_arrays[i] != 1)
		{
			continue; // No reference on the stack so it will be removed whatever the elements are
		}

		arr_i = i;
		arr_ptr = (word_t*)arr_id_mem.ptrs[arr_i];
		arr_size = (uint32_t)arr_ptr[0];
		for (uint32_t j = 1; j <= arr_size; j++)
		{
			if ((arr_ptr[j] & (word_t)k_index_to_ref) == 0xAA00000A ||
				j == i)
			{
				/**
				* Entry in array is definately not an array reference or
				* an array stores a reference to itself.
				**/
				continue;
			}

			for (uint32_t arr_id_ptr = 0; arr_id_ptr < arr_id_mem.size; arr_id_ptr++)
			{
				ref = arr_id_mem.refs[arr_id_ptr];
				if (ref == 0)
				{
					continue; // Skip unclaimed (0) array reference
				}

				if (arr_ptr[j] == ref)
				{
					marked_arrays[arr_i] = true;
					break;
				}
			}
		}
	}
}


/**
* Sweep all inaccessible arrays and return the number of arrays that were removed
**/
static uint32_t sweep_arrays(bool* marked_arrays)
{
	uint32_t num_swept = 0;
	for (uint32_t i = 0; i < arr_id_mem.size; i++)
	{
		if (marked_arrays[i] != true && arr_id_mem.refs[i] != 0)
		{
			// Remove array
			arr_id_mem.refs[i] = 0;
			free((word_t*)arr_id_mem.ptrs[i]);
			arr_id_mem.ptrs[i] = 0;
			num_swept++;
		}
	}
	return num_swept;
}


uint32_t gc_arrays(void)
{
	uint32_t tmp_mem_size = arr_id_mem.size;
	bool* marked_arrays = (bool*)calloc(tmp_mem_size, sizeof(bool));
	uint32_t num_freed;
	if (marked_arrays == NULL)
	{
		fprintf(stderr, "[ERR] Failed to allocate memory. In \"array.c::gc_arrays\".\n");
		destroy_ijvm_now();
	}
	mark_arrays(marked_arrays);
	num_freed = sweep_arrays(marked_arrays);

	free(marked_arrays);

	printf("GC free'd %i", num_freed);
	return num_freed;
}


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
