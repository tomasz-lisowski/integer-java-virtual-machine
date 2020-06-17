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
    size_t num_elements_read;

    // Read origin
    uint32_t const_pool_origin;
    num_elements_read = fread(&(const_pool_origin), sizeof(const_pool_origin), 1, f);
    if (num_elements_read != 1)
    {
        return false;
    }
    const_pool_origin = swap_uint32(const_pool_origin);

    // Read size
    num_elements_read = fread(&(g_cpu->const_mem_size), sizeof(g_cpu->const_mem_size), 1, f);
    if (num_elements_read != 1)
    {
        return false;
    }
    g_cpu->const_mem_size = (int)swap_uint32((uint32_t)g_cpu->const_mem_size);

    // Read consts
    g_cpu->const_mem = (word_t*)malloc((uint32_t)g_cpu->const_mem_size * sizeof(word_t));
    if (g_cpu->const_mem == NULL)
    {
        return false;
    }
    num_elements_read = fread(g_cpu->const_mem, sizeof(word_t), (uint32_t)g_cpu->const_mem_size / sizeof(word_t), f);
    if (num_elements_read != (uint32_t)g_cpu->const_mem_size / sizeof(word_t))
    {
        return false;
    }
    for (unsigned int i = 0; i < (uint32_t)g_cpu->const_mem_size / sizeof(word_t); i++)
    {
        (g_cpu->const_mem)[i] = (word_t)swap_uint32((uint32_t)(g_cpu->const_mem)[i]);
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
    size_t num_elements_read;

    // Read origin
    uint32_t text_origin;
    num_elements_read = fread(&(text_origin), sizeof(text_origin), 1, f);
    if (num_elements_read != 1)
    {
        return false;
    }
    text_origin = swap_uint32(text_origin);

    // Read size
    num_elements_read = fread(&(g_cpu->code_mem_size), sizeof(g_cpu->code_mem_size), 1, f);
    if (num_elements_read != 1)
    {
        return false;
    }
    g_cpu->code_mem_size = (int)swap_uint32((uint32_t)g_cpu->code_mem_size);

    // Read code
    g_cpu->code_mem = (byte_t*)malloc((uint32_t)g_cpu->code_mem_size * sizeof(byte_t));
    if (g_cpu->code_mem == NULL)
    {
        return false;
    }
    num_elements_read = fread(g_cpu->code_mem, sizeof(byte_t), (uint32_t)g_cpu->code_mem_size, f);
    if (num_elements_read != (uint32_t)g_cpu->code_mem_size / sizeof(byte_t))
    {
        return false;
    }

    return true;
}


bool load_bin(char* path)
{
    uint32_t num_elements_read;
    uint32_t magic_number;
    FILE* f;

    if (!(f = fopen(path, "rb")))
    {
        dprintf("[FILE COULD NOT BE OPENED]\n");
        return false;
    }

    num_elements_read = fread(&(magic_number), sizeof(magic_number), 1, f);

    if (num_elements_read != 1 ||
        !check_file_signature((uint32_t)magic_number) ||
        !load_consts(f) ||
        !load_code(f)
        )
    {
        dprintf("[BAD FILE FORMAT]\n");
        fclose(f);
        return false;
    }

    dprintf("[LOAD OK]\n");
    fclose(f);
    return true;
}
