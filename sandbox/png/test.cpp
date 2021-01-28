#include <stdio.h>
#include <stdlib.h>
#include "../../code/mm_core.h"
#include "../../code/mm_bitwise.h"

static inline void* 
ReadFileToMemory(const char* Filename) {
    FILE* File = {};
    if (fopen_s(&File, "test.png", "rb") != 0) { 
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

struct png_header {
    u8 Signature[8];
};

#pragma pack(push, 1)
// 5.3 Chunk layout
// | Length | Type | Data | CRC
struct png_chunk_header {
    u32 Length;
    union {
        u32 TypeU32;
        u8 Type[4];
    };
};

struct png_chunk_data_IHDR {
    u32 Width;
    u32 Height;
    u8 BitDepth;
    u8 ColourType;
    u8 CompressionMethod;
    u8 FilterMethod;
    u8 InterlaceMethod;
};

struct png_chunk_footer {
    u32 Crc; 
};
#pragma pack(pop)

int main() {    
    
    void* PngMemory = ReadFileToMemory("test.png");
    if (!PngMemory){
        return 1;
    }
    Defer { free(PngMemory); }; 

    // Let's go
    const u8 PngSignature[] = { 137, 80, 78, 71, 13, 10, 26, 10 };
    void* Itr = PngMemory;

    // Read the signature
    {
        auto* PngHeader = Read<png_header>(&Itr);  

        for (u32 I = 0; I < ArrayCount(PngSignature); ++I) {
            if (PngSignature[I] != PngHeader->Signature[I]) {
                printf("Png Singature wrong!\n");
                return 1;
            }
        }
    }

    // Process the rest of the chunks
    {
        b32 IsRunning = true;
        while(!IsRunning) {
            auto* ChunkHeader = Read<png_chunk_header>(&Itr);
            switch(ChunkHeader->TypeU32) {
                case FourCC("IHDR"): {
                    printf("IHDR\n");
                } break;
                case FourCC("IEND"): {
                    printf("IEND\n");
                    IsRunning = false; 
                } break;
                case FourCC("PLTE"): {
                    printf("PLTE\n");
                } break;
                                     
                case FourCC("IDAT"): {
                    printf("IDAT\n");
                } break;
                default: {
                    printf("Unknown Type\n");
                    IsRunning = false;
                }
            }
        }


    }


    printf("Done!");
    return 0;
}
