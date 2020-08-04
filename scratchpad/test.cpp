#include <stdio.h>
#include <stdlib.h>
#include "ryoji_common.cpp"
static inline u32 
Read32(const u8* P) {
#if BIG_ENDIAN
    return P[0] << 24 | (P[1] << 18) |  (P[2] << 8) | (P[3]);
#else
    return P[0] | (P[1] << 8) |  (P[2] << 16) | (P[3] << 24);
#endif
}

struct bitmap {
    u32 Width;
    u32 Height;
    void* Pixels;
};

constexpr u32 png_header_type_IHDR = 0x49484452;
constexpr u32 png_header_type_IEND = 0x49454E68;

// NOTE(Momo): https://www.w3.org/TR/2003/REC-PNG-20031110/#5Chunk-naming-conventions
bitmap ReadPng(const char* Filepath) {
    FILE* File = fopen(Filepath, "rb");
    Defer {
        fclose(File);
    };
    
    fseek(File, 0, SEEK_END);
    u32 Filesize = ftell(File);
    fseek(File, 0, SEEK_SET);
    
    u8* Memory = (u8*)malloc(Filesize);
    Defer {
        free(Memory);
    };
    fread(Memory, Filesize, 1, File);
    
    // NOTE(Momo): actual reading
    u8* Itr = Memory;
    
    // NOTE(Momo): Check signature headers
    {
        if (!(Itr[0] == 0x89 &&
              Itr[1] == 0x50 &&
              Itr[2] == 0x4E &&
              Itr[3] == 0x47 &&
              Itr[4] == 0x0D &&
              Itr[5] == 0x0A &&
              Itr[6] == 0x1A &&
              Itr[7] == 0x0A)) {
            printf("Bad Signature\n");
            return {};
        }
        Itr += 8;
    }
    printf("It's png!\n");
    
    // NOTE(Momo): Trying to process a IHDR chunk (must be first)
    u32 Width;
    u32 Height;
    u8 BitDepth, ColorType, CompressionMethod, FilterMethod, InterlaceMethod;
    {
        // Header
        u32 ChunkLength = Read32(Itr); 
        Itr += sizeof(ChunkLength);
        u32 ChunkType = Read32(Itr);
        Itr += sizeof(ChunkType);
        
        if (ChunkType != png_header_type_IHDR)  {
            printf("IHDR not first chunk!\n");
            return {};
        }
        
        // Data?
        Width = Read32(Itr);
        Itr += sizeof(Width);
        Height= Read32(Itr);
        Itr += sizeof(Height);
        BitDepth = (*Itr);
        Itr += sizeof(BitDepth);
        ColorType = (*Itr));
        Itr += sizeof(ColorType);
        CompressionMethod = (*Itr);
        Itr += sizeof(CompressionMethod);
        FilterMethod = (*Itr);
        Itr += sizeof(FilterMethod);
        InterlaceMethod = (*Itr);
        Itr += sizeof(InterlaceMathod);
        
        // Footer
        u32 ChunkCrc = Read32(Itr);
        Itr += sizeof(ChunkCrc);
        
        
    }
    
    
    return {};
    
}

int main() {
    bitmap Bitmap =  ReadPng("blank.png");
}