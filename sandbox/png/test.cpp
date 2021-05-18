#include <stdio.h>
#include <stdlib.h>
#include "../../code/momo.h"

struct read_file_result
{
    void* Memory;
    u32 MemorySize;
};

static inline b32
ReadFileToMemory(arena* Arena, read_file_result* Result, const char* Filename) {
    FILE* File = {};
    if (fopen_s(&File, Filename, "rb") != 0) { 
        printf("Cannot find file\n");
        return False;
    }
    Defer{ fclose(File); };
    
    fseek(File, 0, SEEK_END);
    s32 Filesize = ftell(File);
    fseek(File, 0, SEEK_SET);
    
    void* FileMemory = Arena_PushBlock(Arena, Filesize);
    fread(FileMemory, 1, Filesize, File); 
    
    Result->Memory = FileMemory;
    Result->MemorySize = Filesize;
    
    return True;
}


int main() {    
    u32 MemorySize = Megabytes(1);
    void * Memory = malloc(MemorySize);
    if (!Memory) { return 1; }
    Defer { free(Memory); };  
    
    arena AppArena = Arena_Create(Memory, MemorySize);
    read_file_result PngFile = {};
    if (!ReadFileToMemory(&AppArena, &PngFile, "test2.png")){
        return 1;
    }
    png_image Png = {};
    png_error Err = Png_Parse(&Png, &AppArena, PngFile.Memory, PngFile.MemorySize);
    
    printf("Done at %d\n", Err);
    return 0;
}

