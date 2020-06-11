#include "loader.h"
#include "util.h"

/**
* Unique file signature of IJVM libraries
**/
static uint32_t MAGIC_NUMBER = 0x1DEADFAD;


/**
* Check that magic number fits the IJVM binary signature
* Returns  1 on good magic number
*          0 on bad magic number
**/
static bool check_file_signature(uint32_t magic_number)
{
    if (swap_uint32(magic_number) != MAGIC_NUMBER)
    {
        dprintf("[BAD FILE SIGNATURE]\n");
        return false;
    }
    else
    {
        return true;
    }
}


/**
* Given an open file, load the const origin, size, and pool into the CPU
* Returns  1 on success
*          0 on failure
**/
static bool load_consts(FILE* f)
{
    uint32_t const_pool_origin;
    fread(&(const_pool_origin), sizeof(const_pool_origin), 1, f);
    const_pool_origin = swap_uint32(const_pool_origin);


    fread(&(g_cpu_ptr->const_mem_size), sizeof(g_cpu_ptr->const_mem_size), 1, f);
    g_cpu_ptr->const_mem_size = swap_uint32(g_cpu_ptr->const_mem_size);
    g_cpu_ptr->const_mem_size = (int)swap_uint32((uint32_t)g_cpu_ptr->const_mem_size);

    g_cpu_ptr->const_mem = (word_t*)malloc((uint32_t)g_cpu_ptr->const_mem_size * sizeof(word_t));
    if (g_cpu_ptr->const_mem == NULL)
    {
        return false;
    }
    fread(g_cpu_ptr->const_mem, sizeof(word_t), g_cpu_ptr->const_mem_size / sizeof(word_t), f);
    for (unsigned int i = 0; i < g_cpu_ptr->const_mem_size / sizeof(word_t); i++)
    {
        (g_cpu_ptr->const_mem)[i] = swap_uint32((g_cpu_ptr->const_mem)[i]);
    }

    return true;
}


/**
* Given an open file, load the text origin, size, and code into the CPU
* Returns  1 on success
*          0 on failure
**/
static bool load_code(FILE* f)
{
    uint32_t text_origin;
    fread(&(text_origin), sizeof(text_origin), 1, f);
    text_origin = swap_uint32(text_origin);

    fread(&(g_cpu_ptr->code_mem_size), sizeof(g_cpu_ptr->code_mem_size), 1, f);
    g_cpu_ptr->code_mem_size = swap_uint32(g_cpu_ptr->code_mem_size);

    g_cpu_ptr->code_mem = (byte_t*)malloc(g_cpu_ptr->code_mem_size * sizeof(byte_t));
    if (g_cpu_ptr->code_mem == NULL)
    {
        return false;
    }
    fread(g_cpu_ptr->code_mem, sizeof(byte_t), g_cpu_ptr->code_mem_size, f);

    return true;
}


bool load_bin(char* path)
{
    uint32_t magic_number;
    FILE* f;
    if (!(f = fopen(path, "rb")))
    {
#ifdef DEBUG
        dprintf("[FILE COULD NOT BE OPENED]\n");
#endif
        return false;
    }

    num_elements_read = fread(&(magic_number), sizeof(magic_number), 1, f);
    fread(&(magic_number), sizeof(magic_number), 1, f);

    if (!check_file_signature(magic_number) || (!load_consts(f) || (!load_code(f))))
    {
        fclose(f);
        return false;
    }

    fclose(f);
#ifdef DEBUG
    dprintf("[LOAD OK]\n");
#endif
    return true;
}
