#include <stdio.h>
#include <stdlib.h>
#include "../../code/mm_core.h"
#include "../../code/mm_bitwise.h"

#define _CRT_SECURE_NO_WARNINGS 1

static inline void* 
ReadFileToMemory(const char* Filename) {
    FILE* File = {};
    if (fopen_s(&File, "test.png", "rb") == 0) { 
        printf("Cannot find file\n");
        return nullptr;
    }
    
    Defer{ fclose(File); };

    fseek(File, 0, SEEK_END);
    i32 Filesize = ftell(File);
    fseek(File, 0, SEEK_SET);

    void* Ret = malloc(Filesize);
    fread(Ret, 1, Filesize, File); 

    return Ret;
}


int main() {    
    const u8 PngSignature[] = { 137, 80, 78, 71, 13, 10, 26, 10 };
    
    void* PngMemory = ReadFileToMemory("test.png");
    if (!PngMemory){
        printf("Cannot allocate memory\n");
        return 1;
    }
    Defer { free(PngMemory); }; 

    // let's go
    void* Itr = PngMemory;
    {
        // Read the signature
        u8* Signature = (u8*)Itr;
        for (u32 I = 0; I < ArrayCount(PngSignature); ++I) {
            if (PngSignature[I] != Signature[I]) {
                printf("Png Singature wrong!\n");
                return 1;
            }
        }
        Itr = (u8*)Itr + ArrayCount(PngSignature);  
    }




    printf("Done!");
    return 0;
}
