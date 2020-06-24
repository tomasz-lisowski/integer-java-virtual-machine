#include "debug_data_loader.h"


// Declarations of static functions
static int64_t get_block_size(FILE* f);
static bool skip_block(FILE* f);
static bool inc_memory_size(void** arr, uint32_t* arr_size, uint32_t el_size);
static bool read_symbol_block(FILE* f, SymbolBlock_t* symb_block);
static void init_symbol_block(SymbolBlock_t* symb_block);
static void destroy_symbol_block(SymbolBlock_t* symb_block);


static DebugData_t debug_data;
DebugData_t* g_debug_data = &debug_data;


/**
* Given a file, whose position indicator points at the origin of a block,
* return the size of the block in bytes.
* Return  >0 on success
*         -1 on failure
**/
static int64_t get_block_size(FILE* f)
{
    size_t num_elements_read;
    uint32_t block_size;

    fseek(f, 4, SEEK_CUR); // Skip origin
    num_elements_read = fread(&block_size, 4, 1, f);
    if (num_elements_read != 1)
    {
        return -1;
    }
    return (int64_t)swap_uint32(block_size);
}


/**
* Given a file, get the block size then skip all data bytes in the block.
**/
static bool skip_block(FILE* f)
{
    int64_t block_size = get_block_size(f);
    if (block_size < 0)
    {
        return false;
    }
    fseek(f, block_size, SEEK_CUR);
    return true;
}


/**
* Make an array bigger by one element (increment memory size).
* Return  true on success
*         false on failure (array pointer remains the same)
**/
static bool inc_memory_size(void** arr, uint32_t* arr_size, uint32_t el_size)
{
    void* new_ptr;
    new_ptr = *arr;

    new_ptr = realloc(new_ptr, ++(*arr_size) * el_size);
    if (new_ptr == NULL)
    {
        return false;
    }
    *arr = new_ptr;
    new_ptr = NULL;
    return true;
}


/**
* Read and parse the entire symbols block and load addresses, symbols, and symbol start indices.
**/
static bool read_symbol_block(FILE* f, SymbolBlock_t* symb_block)
{
    size_t num_elements_read;
    int64_t block_size;

    /**
    * Number of symbols is unknown so symbol memory needs to be
    * dynamically allocated until loading is done (after loading size remains unchanged).
    **/
    uint32_t addr_mem_size = 0;
    uint32_t names_mem_size = 0;
    uint32_t names_start_mem_size = 0;

    block_size = get_block_size(f); // Moves position indicator to start of first symbol if it exists
    if (block_size == 0)
    {
        symb_block->num = 0;
        return true; // Block is empty
    }
    
    for (uint32_t bytes_read = 0; bytes_read < block_size;)
    {
        if (inc_memory_size((void*)&symb_block->addr, &addr_mem_size, sizeof(uint32_t)) != true)
        {
            return false;
        }
        // Read symbol address
        num_elements_read = fread(&symb_block->addr[symb_block->num], sizeof(uint32_t), 1, f);
        bytes_read += 4;
        symb_block->addr[symb_block->num] = swap_uint32(symb_block->addr[symb_block->num]);
        if (num_elements_read != 1)
        {
            return false;
        }

        if (inc_memory_size((void*)&symb_block->names_start, &names_start_mem_size, sizeof(uint32_t)) != true)
        {
            return false;
        }
        // Save starting index in name memory before loading name
        symb_block->names_start[symb_block->num] = names_mem_size;

        // Read symbol name
        do
        {
            if (inc_memory_size((void*)&symb_block->names, &names_mem_size, sizeof(uint32_t)) != true)
            {
                return false;
            }
            symb_block->names[names_mem_size - 1] = (char)getc(f);
            bytes_read++;
        }
        while (symb_block->names[names_mem_size - 1] != '\0');

        symb_block->num++; // Successfully read a symbol
    }

    return true;
}


bool load_debug_data(char* prog_path)
{
    FILE* f;
    bool read_success = true;

    if (!(f = fopen(prog_path, "rb")))
    {
        dprintf("File could not be opened.\n");
        return false;
    }

    fseek(f, 4, SEEK_CUR); // Skip magic number
    read_success = skip_block(f) ? (read_success && true) : false; // Skip constants
    read_success = skip_block(f) ? (read_success && true) : false; // Skip code
    read_success = read_symbol_block(f, &g_debug_data->func_label) ? (read_success && true) : false; // Read function symbols
    read_success = read_symbol_block(f, &g_debug_data->sec_label) ? (read_success && true) : false; // Read section symbols
    if (!read_success)
    {
        destroy_debug_data();
        init_debug_data();
        return false;
    }

    if (g_debug_data->func_label.num == 0) // Can't have a section without a function
    {
        printf("There are no debug symbols in this binary.\n");
    }
    else
    {
        printf("Debug symbols have been loaded.\n");
    }
    fclose(f);
    return true;
}


/**
* Initialize a SymbolBlock struct
**/
static void init_symbol_block(SymbolBlock_t* symb_block)
{
    symb_block->num = 0;
    symb_block->addr = NULL;
    symb_block->names = NULL;
    symb_block->names_start = NULL;
}


/**
* Safely free memory occupied by a SymbolBlock struct
**/
static void destroy_symbol_block(SymbolBlock_t* symb_block)
{
    free(symb_block->addr);
    free(symb_block->names);
    free(symb_block->names_start);
}


void init_debug_data(void)
{
    init_symbol_block(&g_debug_data->func_label);
    init_symbol_block(&g_debug_data->sec_label);
}


void destroy_debug_data(void)
{
    destroy_symbol_block(&g_debug_data->func_label);
    destroy_symbol_block(&g_debug_data->sec_label);
}


char* get_func_name(uint32_t i)
{
    return str_dup(&g_debug_data->func_label.names[g_debug_data->func_label.names_start[i]]);
}


char* get_section_name(uint32_t i)
{
    return str_dup(&g_debug_data->sec_label.names[g_debug_data->sec_label.names_start[i]]);
}
