#include "marr.h"


// Declarations of static functions
static uint32_t get_free_index(MArr_t* marr);


/**
* Find an unclaimed index in the mapped array and return it.
* Return  index on sucess
*         SIZE_MAX_UINT32_T on failure
* SIZE_MAX_UINT32_T is used as a special value because there will never be this many elements.
**/
static uint32_t get_free_index(MArr_t* marr)
{
    uint32_t free_i = SIZE_MAX_UINT32_T;
    for (uint32_t map_ptr = 0; map_ptr < marr->size; map_ptr++)
    {
        if (marr->map[map_ptr] == false)
        {
            free_i = map_ptr;
            break;
        }
    }
    return free_i;
}


void marr_init(MArr_t* marr, uint32_t size)
{
    marr->map = NULL;
    marr->values = NULL;
    marr->size = 0;
    if (size == 0)
    {
        fprintf(stderr, "[ERR] Invalid ('0') mapped array size. In \"marr.c::marr_init\".\n");
        destroy_ijvm_now();
    }
    marr_resize(marr, size);
}


void marr_resize(MArr_t* marr, uint32_t new_size)
{
    uint32_t tmp_new_size = new_size;
    MArr_t tmp_marr = *marr;

    if (new_size < marr->size)
    {
        fprintf(stderr, "[ERR] New size cannot be less than the old size. In \"marr.c::marr_resize\".\n");
        destroy_ijvm_now();
    }
    if (new_size >= SIZE_MAX_UINT32_T)
    {
        fprintf(stderr, "[ERR] Exceeded the size limit of a mapped array. In \"marr.c::marr_resize\".\n");
        destroy_ijvm_now();
    }

    marr->map = (bool*)calloc(tmp_new_size, sizeof(bool));
    tmp_new_size = new_size;
    marr->values = (uintptr_t*)calloc(tmp_new_size, sizeof(uintptr_t));
    if (marr->map == NULL || marr->values == NULL)
    {
        *marr = tmp_marr;
        if (arr_gc() != 0)
        {
            marr_resize(marr, new_size); // Run GC to be sure memory allocation error is not caused by garbage
            return;
        }
        fprintf(stderr, "[ERR] Failed to allocate memory. In \"marr.c::marr_resize\".\n");
        destroy_ijvm_now();
    }

    marr->size = new_size;
    if (tmp_marr.map != NULL)
    {
        memcpy(marr->map, tmp_marr.map, tmp_marr.size * sizeof(bool));
    }
    if (tmp_marr.values != NULL)
    {
        memcpy(marr->values, tmp_marr.values, tmp_marr.size * sizeof(uintptr_t));
    }
    marr_destroy(&tmp_marr);
}


bool marr_check_marked(MArr_t* marr, uint32_t val_i)
{
    if (val_i >= marr->size)
    {
        fprintf(stderr, "[ERR] Out of bounds map check. In \"marr.c::marr_check_marked\".\n");
        destroy_ijvm_now();
    }
    return marr->map[val_i];
}


uint32_t marr_add_element(MArr_t* marr, uintptr_t data)
{
    uint32_t free_i = get_free_index(marr);
    if (free_i == SIZE_MAX_UINT32_T)
    {
        return free_i;
    }

    marr->values[free_i] = data;
    marr->map[free_i] = true;
    return free_i;
}


void marr_set_element(MArr_t* marr, uint32_t val_i, uintptr_t data)
{
    if (val_i >= marr->size)
    {
        fprintf(stderr, "[ERR] Tried to access out of bounds memory in mapped array. In \"marr.c::marr_set_element\".\n");
        destroy_ijvm_now();
    }

    marr->values[val_i] = data;
    marr->map[val_i] = true;
}


uintptr_t marr_get_element(MArr_t* marr, uint32_t val_i)
{
    if (val_i >= marr->size)
    {
        fprintf(stderr, "[ERR] Tried to access out of bounds memory in mapped array. In \"marr.c::marr_get_element\".\n");
        destroy_ijvm_now();
    }
    if (marr->map[val_i] != true)
    {
        fprintf(stderr, "[ERR] Tried to access unclaimed element. In \"marr.c::marr_get_element\".\n");
        destroy_ijvm_now();
    }
    return marr->values[val_i];
}


void marr_remove_element(MArr_t* marr, uint32_t val_i)
{
    if (val_i >= marr->size)
    {
        fprintf(stderr, "[ERR] Tried to remove a non existent element from a mapped array. In \"marr.c::marr_remove_element\".\n");
        destroy_ijvm_now();
    }

    marr->values[val_i] = 0;
    marr->map[val_i] = false;
}


void marr_destroy(MArr_t* marr)
{
    free(marr->map);
    free(marr->values);

    // Not necessary but eh...
    marr->map = NULL;
    marr->values = NULL;
    marr->size = 0;
}


void marr_print(MArr_t* marr)
{
    printf("Mapped Array\n");
    printf("\tSize: %i\n", marr->size);
    if (marr->size > 0)
    {
        printf("\tData:\n", marr->size);
        printf("\tIndex Map Value\n");
    }
    else
    {
        printf("\tNo Data\n");
    }

    for (uint32_t marr_i = 0; marr_i < marr->size; marr_i++)
    {
        printf("\t#%-4i %-3d 0x%016lX\n", marr_i + 1, marr->map[marr_i], marr->values[marr_i]);
    }

    if (marr->size > 0)
    {
        printf("\n");
    }
}
