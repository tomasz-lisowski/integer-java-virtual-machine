#include <ijvm.h>
#include "globals.h"
#include <stdlib.h>


BIN_t b;


static uint32_t swap_uint32(uint32_t num)
{
    return((num >> 24) & 0xff) | ((num << 8) & 0xff0000) | ((num >> 8) & 0xff00) | ((num << 24) & 0xff000000);
}


/**
 * Initializes the IJVM with the binary file found at the provided argument
 * Note. You need to be able to re-initialize the IJVM after it has been started.
 *
 * Returns  0 on success
 *         -1 on failure
 **/
int init_ijvm(char* binary_path)
{
    // Uses global variable "BIN b" to store file data;
    FILE* f;
    
    fprintf(stderr, "[LOAD START]\n");
    if ((f = fopen(binary_path, "r")))
    {
        fread(&(b.magic_number), sizeof(b.magic_number), 1, f);
        b.magic_number = swap_uint32(b.magic_number);
        if (b.magic_number != MAGIC_NUMBER)
        {
            fprintf(stderr, "[LOAD BAD]\n");
            return -1;
        }

        fread(&(b.const_pool_origin), sizeof(b.const_pool_origin), 1, f);
        b.const_pool_origin = swap_uint32(b.const_pool_origin);

        fread(&(b.const_pool_size), sizeof(b.const_pool_size), 1, f);
        b.const_pool_size = swap_uint32(b.const_pool_size);

        b.const_pool = (int32_t*)malloc(b.const_pool_size / sizeof(int32_t));
        fread(b.const_pool, sizeof(int32_t), b.const_pool_size / sizeof(int32_t), f);
        for (uint32_t i = 0; i < b.const_pool_size / sizeof(int32_t); i++)
        {
            b.const_pool[i] = swap_uint32(b.const_pool[i]);
        }

        fread(&(b.text_origin), sizeof(b.text_origin), 1, f);
        b.text_origin = swap_uint32(b.text_origin);
        
        fread(&(b.text_size), sizeof(b.text_size), 1, f);
        b.text_size = swap_uint32(b.text_size);
        
        b.text = (byte_t*)malloc(b.text_size / sizeof(byte_t));
        fread(b.text, sizeof(byte_t), b.text_size, f);

        fclose(f);

        fprintf(stderr, "Binary\n\tMagic Num: 0x%X\n\tConstant Pool Origin: 0x%X\n\tConstant Pool Size: 0x%X\n", b.magic_number, b.const_pool_origin, b.const_pool_size);
        fprintf(stderr, "Consts\n");
        for (size_t i = 0; i < b.const_pool_size / sizeof(int32_t); i++)
        {
            fprintf(stderr, "\t0x%X\n", b.const_pool[i]);
        }
        fprintf(stderr, "Text\n\tText Pool Origin: 0x%X\n\tText Size: 0x%X\n", b.text_origin, b.text_size);
        fprintf(stderr, "OPs\n");
        for (size_t i = 0; i < b.text_size / sizeof(char); i++)
        {
            fprintf(stderr, "\t0x%X\n", b.text[i]);
        }
        fprintf(stderr, "[LOAD OK]\n\n");
        return 0;
    }
    else
    {
        fprintf(stderr, "[LOAD BAD]\n");
        return -1;
    }
}


/**
 * Returns the currently loaded program text as a byte array.
 **/
byte_t* get_text(void)
{
    return b.text;
}


/**
 * Returns the size of the currently loaded program text.
 **/
int text_size(void)
{
    return b.text_size;
}
