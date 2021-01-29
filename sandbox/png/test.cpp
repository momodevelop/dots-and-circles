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
        while(IsRunning) {
            auto* ChunkHeader = Read<png_chunk_header>(&Itr);
            EndianSwap(&ChunkHeader->Length);
            printf("%c%c%c%c: %d\n", 
                ChunkHeader->Type[0], 
                ChunkHeader->Type[1],
                ChunkHeader->Type[2],
                ChunkHeader->Type[3],
                ChunkHeader->Length);
            switch(ChunkHeader->TypeU32) {
                case FourCC("IHDR"): {
                    auto* IHDR = Read<png_chunk_data_IHDR>(&Itr);
                    EndianSwap(&IHDR->Width);
                    EndianSwap(&IHDR->Height);

                    // We are only interested in certain types of PNGs
                    
                    printf(">> %d,%d,%d,%d,%d,%d,%d\n", 
                         IHDR->Width,
                         IHDR->Height,
                         IHDR->BitDepth,
                         IHDR->ColourType,
                         IHDR->CompressionMethod,
                         IHDR->FilterMethod,
                         IHDR->InterlaceMethod);
                } break;
                case FourCC("IEND"): {
                    Advance(&Itr, ChunkHeader->Length);
                    IsRunning = false; 
                } break;
                case FourCC("PLTE"): {
                    Advance(&Itr, ChunkHeader->Length);
                } break;
                case FourCC("sRGB"): {
                    Advance(&Itr, ChunkHeader->Length);
                } break;
                case FourCC("gAMA"): {
                    Advance(&Itr, ChunkHeader->Length);
                } break;
                case FourCC("pHYs"): {
                    Advance(&Itr, ChunkHeader->Length);
                } break;
                case FourCC("IDAT"): {
                    Advance(&Itr, ChunkHeader->Length);
                } break;
                default: {
                    printf("Unknown chunk header!\n");
                    IsRunning = false;
                }

            }
            Advance<png_chunk_footer>(&Itr);
        }


    }


    printf("Done!");
    return 0;
}
