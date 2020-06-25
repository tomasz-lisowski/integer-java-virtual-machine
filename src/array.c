#include "array.h"


// Declarations of static functions
static inline uint32_t ref_to_index(word_t arr_ref);
static inline word_t index_to_ref(uint32_t arr_i);
static word_t arr_store(word_t* arr);
static void arr_check_bounds(word_t arr_ref, word_t i);
static void arr_remove(uint32_t arr_i);
static void mark_arrays(void);
static uint32_t sweep_arrays(void);


static const uint32_t k_index_to_ref = 0xAA00000A;
static const uint32_t k_ref_to_index = 0x00FFFFF0;

static MArr_t arr_mem = { 0, NULL, NULL }; // Keep track of arrays
static bool* marked_arrays;


/**
* Recover array index from array reference
**/
static inline uint32_t ref_to_index(word_t arr_ref)
{
    return (k_ref_to_index & (uint32_t)arr_ref) >> 4;
}


/**
* Create an array reference from array index
**/
static inline word_t index_to_ref(uint32_t i)
{
    return (word_t)(k_index_to_ref | (i << 4));
}


/**
* Add an array to array memory so it can be tracked/modified.
* Return  array reference of saved array
**/
static word_t arr_store(word_t* arr)
{
    uint32_t arr_i;
    word_t arr_ref;

    // Initialize if needed
    if (arr_mem.size == 0)
    {
        marr_init(&arr_mem, ARRAYS_MIN_NUM);
    }

    // Save array
    arr_i = marr_add_element(&arr_mem, (uintptr_t)arr);
    if (arr_i >= SIZE_MAX_UINT32_T)
    {
        marr_resize(&arr_mem, arr_mem.size * 8);
        arr_i = marr_add_element(&arr_mem, (uintptr_t)arr);
    }

    // Return array reference
    arr_ref = index_to_ref(arr_i);
    if (((uint32_t)arr_ref & k_index_to_ref) != k_index_to_ref)
    {
        fprintf(stderr, "[ERR] Program requires more arrays than is supported. In \"array.c::arr_store\".\n");
        destroy_ijvm_now();
    }
    return arr_ref;
}


word_t arr_create(word_t count)
{
    uint32_t tmp_count = (uint32_t)count;
    word_t arr_ref;
    word_t* arr_ptr = (word_t*)calloc(tmp_count + 1, sizeof(word_t));
    if (arr_ptr == NULL)
    {
        if (arr_gc() != 0)
        {
            return arr_create(count); // Run GC to be sure memory allocation error is not caused by garbage
        }
        fprintf(stderr, "[ERR] Failed to allocate memory. In \"array.c::arr_create\".\n");
        destroy_ijvm_now();
    }

    arr_ptr[0] = (word_t)count; // First element stores array's size in 'elements' units
    arr_ref = arr_store(arr_ptr);
    return arr_ref;
}


/**
* Utility to check if combintation of array reference and index are valid
**/
static void arr_check_bounds(word_t arr_ref, word_t i)
{
    uint32_t arr_size;
    uint32_t arr_i;
    word_t* arr_ptr;

    arr_i = ref_to_index(arr_ref);
    if (((uint32_t)arr_ref & k_index_to_ref) != k_index_to_ref)
    {
        fprintf(stderr, "[ERR] Invalid array reference. In \"array.c::arr_check_bounds\".\n");
        destroy_ijvm_now();
    }

    arr_ptr = (word_t*)marr_get_element(&arr_mem, arr_i);
    if (arr_ptr == NULL)
    {
        fprintf(stderr, "[ERR] Program tried to access a non-existent array. In \"array.c::arr_check_bounds\".\n");
        destroy_ijvm_now();
    }

    arr_size = (uint32_t)arr_ptr[0];
    if (i + 1 < 1 || (uint32_t)(i + 1) > arr_size)
    {
        fprintf(stderr, "[ERR] Program tried to access a memory outside of an array. In \"array.c::arr_check_bounds\".\n");
        destroy_ijvm_now();
    }
}


word_t arr_get(word_t arr_ref, word_t i)
{
    uint32_t arr_i;
    word_t* arr_ptr;

    arr_i = ref_to_index(arr_ref);
    arr_check_bounds(arr_ref, i);
    arr_ptr = (word_t*)marr_get_element(&arr_mem, arr_i);
    return arr_ptr[(uint32_t)i + 1]; // First element is at index 1 (0'th element stores array size)
}


void arr_set(word_t arr_ref, word_t i, word_t val)
{
    uint32_t arr_i;
    word_t* arr_ptr;

    arr_i = ref_to_index(arr_ref);
    arr_check_bounds(arr_ref, i);
    arr_ptr = (word_t*)marr_get_element(&arr_mem, arr_i);
    arr_ptr[(uint32_t)i + 1] = val; // First element is at index 1 (0'th element stores array size)
}


/**
* Remove a single array based on index in array memory
**/
static void arr_remove(uint32_t arr_i)
{
    free((word_t*)marr_get_element(&arr_mem, arr_i));
    marr_remove_element(&arr_mem, arr_i);
}


void arr_destroy(void)
{
    for (uint32_t arr_i = 0; arr_i < arr_mem.size; arr_i++)
    {
        if (marr_check_marked(&arr_mem, arr_i) == true)
        {
            arr_remove(arr_i);
        }
    }
    marr_destroy(&arr_mem);
    free(marked_arrays);
}


/**
* Mark all inaccessible arrays and return the number of marked arrays
**/
static void mark_arrays(void)
{
    word_t stack_el;

    uint32_t arr_i;
    word_t* arr_ptr;
    uint32_t arr_size;

    word_t arr_el;
    uint32_t arr_el_arr_i;

    // Look for array references on the stack
    for (int stack_i = 0; stack_i < g_cpu->sp; stack_i++)
    {
        stack_el = g_cpu->stack[stack_i];
        if (((uint32_t)stack_el & k_index_to_ref) != k_index_to_ref)
        {
            continue; // Element is not in array reference format
        }

        arr_i = ref_to_index(stack_el);
        if (marr_check_marked(&arr_mem, arr_i) == true)
        {
            marked_arrays[arr_i] = true;
            continue;
        }
    }

    // Check if array elements hold array references
    for (uint32_t marked_i = 0; marked_i < arr_mem.size; marked_i++)
    {
        if (marked_arrays[marked_i] == false)
        {
            /**
            * Array reference needs to exist on the stack
            * for its elements to have an effect on what will not
            * be counted as garbage.
            **/
            continue; 
        }

        arr_i = marked_i;
        arr_ptr = (word_t*)marr_get_element(&arr_mem, arr_i);
        arr_size = (uint32_t)arr_ptr[0];
        for (uint32_t el_i = 1; el_i <= arr_size; el_i++)
        {
            arr_el = arr_ptr[el_i];
            if (((uint32_t)arr_el & k_index_to_ref) != k_index_to_ref)
            {
                continue; // Element is not in array reference format
            }

            arr_el_arr_i = ref_to_index(arr_el);
            if (arr_el_arr_i == arr_i)
            {
                continue; // Array stores reference to itself
            }
            if (marr_check_marked(&arr_mem, arr_el_arr_i) == true)
            {
                marked_arrays[arr_el_arr_i] = true;
                continue;
            }
        }
    }
}


/**
* Sweep all inaccessible arrays and return the number of arrays that were removed
**/
static uint32_t sweep_arrays(void)
{
    uint32_t num_swept = 0;
    for (uint32_t i = 0; i < arr_mem.size; i++)
    {
        if (marked_arrays[i] != true && marr_check_marked(&arr_mem, i) == true)
        {
            arr_remove(i);
            num_swept++;
        }
    }
    return num_swept;
}


uint32_t arr_gc(void)
{
    uint32_t num_freed;

    free(marked_arrays);
    marked_arrays = (bool*)calloc(arr_mem.size, sizeof(bool));
    if (marked_arrays == NULL)
    {
        fprintf(stderr, "[ERR] Failed to allocate memory. In \"array.c::arr_gc\".\n");
        destroy_ijvm_now();
    }

    mark_arrays();
    num_freed = sweep_arrays();

    return num_freed;
}


void arr_print(bool compact)
{
    if (compact)
    {
        dprintf("AR[");
        for (uint32_t i = 0; i < arr_mem.size; i++)
        {
            if (marr_check_marked(&arr_mem, i) == true)
            {
                dprintf(" 0x%X", index_to_ref(i));
            }
        }
        dprintf(" ]");
    }
    else
    {
        dprintf("AR\n");
        for (uint32_t i = 0; i < arr_mem.size; i++)
        {
            if (marr_check_marked(&arr_mem, i) == true)
            {
                dprintf("\t0x%X\n", index_to_ref(i));
            }
        }
    }
}
