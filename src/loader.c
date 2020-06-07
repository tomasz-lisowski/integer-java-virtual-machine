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
static bool check_file_signature(word_t magic_number)
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
static bool load_consts(FILE* f, CPU_t* cpu)
{
    uint32_t const_pool_origin;
    fread(&(const_pool_origin), sizeof(const_pool_origin), 1, f);
    const_pool_origin = swap_uint32(const_pool_origin);

    fread(&(cpu->data_mem_size), sizeof(cpu->data_mem_size), 1, f);
    cpu->data_mem_size = swap_uint32(cpu->data_mem_size);

    cpu->data_mem = (word_t*)malloc(cpu->data_mem_size / sizeof(word_t));
    fread(cpu->data_mem, sizeof(word_t), cpu->data_mem_size / sizeof(word_t), f);
    for (unsigned int i = 0; i < cpu->data_mem_size / sizeof(word_t); i++)
    {
        (cpu->data_mem)[i] = swap_uint32((cpu->data_mem)[i]);
    }

    return true;
}


/**
* Given an open file, load the text origin, size, and code into the CPU
* Returns  1 on success
*          0 on failure
**/
static bool load_code(FILE* f, CPU_t* cpu)
{
    uint32_t text_origin;
    fread(&(text_origin), sizeof(text_origin), 1, f);
    text_origin = swap_uint32(text_origin);

    fread(&(cpu->code_mem_size), sizeof(cpu->code_mem_size), 1, f);
    cpu->code_mem_size = swap_uint32(cpu->code_mem_size);

    cpu->code_mem = (byte_t*)malloc(cpu->code_mem_size / sizeof(byte_t));
    fread(cpu->code_mem, sizeof(byte_t), cpu->code_mem_size, f);

    return true;
}


bool load_bin(char* path, CPU_t* cpu)
{
    FILE* f;
    if (!(f = fopen(path, "rb")))
    {
        dprintf("[FILE COULD NOT BE OPENED]\n");
        return false;
    }

    uint32_t magic_number;
    fread(&(magic_number), sizeof(magic_number), 1, f);
    if (check_file_signature(magic_number) != true)
    {
        return false;
    }

    load_consts(f, cpu);
    load_code(f, cpu);

    fclose(f);
    dprintf("[LOAD OK]\n");
    return true;
}