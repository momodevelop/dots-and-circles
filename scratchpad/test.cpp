#include <stdio.h>
#include <stdlib.h>
#include "ryoji_common.cpp"

static inline u32 
Read32(const u8* P, bool isBigEndian) {
    union {
        u32 V;
        u8 C[4];
    } Ret;
    
#if BIG_ENDIAN
    if (isBigEndian) {
        Ret.C[0] = P[0];
        Ret.C[1] = P[1];
        Ret.C[2] = P[2];
        Ret.C[3] = P[3];
    }
    else {
        Ret.C[0] = P[3];
        Ret.C[1] = P[2];
        Ret.C[2] = P[1];
        Ret.C[3] = P[0];
    }
#else
    if (isBigEndian) {
        Ret.C[0] = P[3];
        Ret.C[1] = P[2];
        Ret.C[2] = P[1];
        Ret.C[3] = P[0];
    }
    else {
        Ret.C[0] = P[0];
        Ret.C[1] = P[1];
        Ret.C[2] = P[2];
        Ret.C[3] = P[3];
    }
#endif
    return Ret.V;
    
}

struct bitmap {
    u32 Width;
    u32 Height;
    void* Pixels;
};

struct png_ihdr {
    u32 Width;
    u32 Height;
    u8 BitDepth, ColorType, CompressionMethod, FilterMethod, InterlaceMethod;
};

static inline void
PrintIHDR(png_ihdr IHDR) {
    printf("Width: %d\nHeight: %d\nBitDepth: %d\nColorType: %d\nCompressionMethod: %d\nFilterMethod: %d\nInterlaceMethod: %d\n", IHDR.Width, IHDR.Height, IHDR.BitDepth, IHDR.ColorType, IHDR.CompressionMethod, IHDR.FilterMethod, IHDR.InterlaceMethod); 
}

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
    png_ihdr IHDR = {};
    {
        // Header
        u32 ChunkLength = Read32(Itr, true); 
        Itr += sizeof(ChunkLength);
        u32 ChunkType = Read32(Itr, true);
        Itr += sizeof(ChunkType);
        if (ChunkType != png_header_type_IHDR)  {
            printf("%X\n", ChunkType);
            printf("IHDR not first chunk!\n");
            return {};
        }
        
        // Data?
        IHDR.Width = Read32(Itr, true);
        Itr += sizeof(IHDR.Width);
        IHDR.Height= Read32(Itr, true);
        Itr += sizeof(IHDR.Height);
        IHDR.BitDepth = (*Itr);
        Itr += sizeof(IHDR.BitDepth, true);
        IHDR.ColorType = (*Itr);
        Itr += sizeof(IHDR.ColorType, true);
        IHDR.CompressionMethod = (*Itr);
        Itr += sizeof(IHDR.CompressionMethod, true);
        IHDR.FilterMethod = (*Itr);
        Itr += sizeof(IHDR.FilterMethod, true);
        IHDR.InterlaceMethod = (*Itr);
        Itr += sizeof(IHDR.InterlaceMethod, true);
        
        // Footer
        u32 ChunkCrc = Read32(Itr, true);
        Itr += sizeof(ChunkCrc);
    }
    PrintIHDR(IHDR);
    
    
    return {};
    
}

int main() {
    bitmap Bitmap =  ReadPng("blank.png");
}