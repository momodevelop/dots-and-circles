#include <stdio.h>
#include <stdlib.h>
#include "../../code/mm_core.h"
#include "../../code/mm_bitwise.h"
#include "../../code/mm_stream.h"

struct read_file_result
{
    void* Memory;
    usize MemorySize;
};

static inline maybe<read_file_result>
ReadFile(const char* Filename) {
    FILE* File = {};
    if (fopen_s(&File, "test.png", "rb") != 0) { 
        printf("Cannot find file\n");
        return No();
    }
    Defer{ fclose(File); };


    fseek(File, 0, SEEK_END);
    i32 Filesize = ftell(File);
    fseek(File, 0, SEEK_SET);
    
    void* FileMemory = malloc(Filesize);
    fread(FileMemory, 1, Filesize, File); 

    fclose(File);

    read_file_result Ret = {};
    Ret.Memory = FileMemory;
    Ret.MemorySize = Filesize;

    return Yes(Ret);
}

struct png_image {
    u32 Width;
    u32 Height; 
    u32 Channels;
    void* Data;
};


struct png_header {
    u8 Signature[8];
};

// 5.3 Chunk layout
// | Length | Type | Data | CRC
struct png_chunk_header {
    u32 Length;
    union {
        u32 TypeU32;
        u8 Type[4];
    };
};


#pragma pack(push, 1)
struct png_chunk_data_IHDR {
    u32 Width;
    u32 Height;
    u8 BitDepth;
    u8 ColourType;
    u8 CompressionMethod;
    u8 FilterMethod;
    u8 InterlaceMethod;
};
#pragma pack(pop)

struct png_chunk_footer {
    u32 Crc; 
};

// ZLIB header notes:
// Bytes[0]:
// - bits 0-3: Compression Method (CM)
// - bits 4-7: Compression Info (CINFO)
// Bytes[1]:
// - bits 0-4: FCHECK 
// - bits 5: Preset dictionary (FDICT)
// - bits 6-7: Compression level (FLEVEL)
struct png_zlib_header {
    u8 Bytes[2];
};

struct png_zlib_header_parsed {
    u8 CM;
    u8 CINFO;
    u8 FCHECK;
    u8 FDICT;
    u8 FLEVEL; 
};

static inline png_zlib_header_parsed
ParseZlibHeader(png_zlib_header Header) {
    png_zlib_header_parsed Ret = {};
    Ret.CM = Header.Bytes[0] & 0x0F;
    Ret.CINFO = (Header.Bytes[0] >> 4) & 0x0F;

    Ret.FCHECK = Header.Bytes[1] & 0x1F;
    Ret.FDICT = Header.Bytes[1] >> 5 & 0x01;
    Ret.FLEVEL = Header.Bytes[1] >> 6 & 0x03;
        
    return Ret;
}

static inline b32
IsSupported(png_chunk_data_IHDR* IHDR) {
    // We are only interested in certain types of PNGs


    return true;
}

static inline void
FreePng(png_image* Png) {
    free(Png->Data);
}

static inline maybe<png_image> 
ParsePng(void* Memory, usize MemorySize) {
    const u8 PngSignature[] = { 137, 80, 78, 71, 13, 10, 26, 10 };
    stream PngStream = Stream(Memory, MemorySize); 
    png_image PngImage = {};

    // Read the signature
    auto* PngHeader = Consume<png_header>(&PngStream);  
    if (!PngHeader) { 
        return No(); 
    }

    for (u32 I = 0; I < ArrayCount(PngSignature); ++I) {
        if (PngSignature[I] != PngHeader->Signature[I]) {
            printf("Png Singature wrong!\n");
            return No();
        }
    }

    // Check for IHDR which MUST appear first
    auto* ChunkHeader = Consume<png_chunk_header>(&PngStream);
    if (!ChunkHeader) {
        return No(); 
    }
    
    EndianSwap(&ChunkHeader->Length);
    if (ChunkHeader->TypeU32 != FourCC("IHDR")) {
        return No();
    }
    auto* IHDR = Consume<png_chunk_data_IHDR>(&PngStream);
    if (!IHDR) {
        return No();
    }

    // Unsupported details
    if (IHDR->ColourType != 6 &&
        IHDR->BitDepth != 8 &&
        IHDR->CompressionMethod &&
        IHDR->FilterMethod != 0 &&
        IHDR->InterlaceMethod != 0) {
        return No();
    }
    EndianSwap(&IHDR->Width);
    EndianSwap(&IHDR->Height);
    PngImage.Width = IHDR->Width;
    PngImage.Height = IHDR->Height;
    PngImage.Channels = 4;
    PngImage.Data = malloc(PngImage.Width * 
                           PngImage.Height * 
                           PngImage.Channels);
    Advance<png_chunk_footer>(&PngStream);

    // Search for IDAT header
    u32 CM, CINFO, FCHECK, FDICT, FLEVEL; 
    while(!IsEos(&PngStream)) {
        ChunkHeader = Consume<png_chunk_header>(&PngStream);
        EndianSwap(&ChunkHeader->Length);
        switch(ChunkHeader->TypeU32) {
            case FourCC("IDAT"): {
                CINFO = ConsumeBits(&PngStream, 4);
                CM = ConsumeBits(&PngStream, 4);
                FLEVEL = ConsumeBits(&PngStream, 2);
                FDICT = ConsumeBits(&PngStream, 1);
                FCHECK = ConsumeBits(&PngStream, 5);
                printf(">> CM: %d\n>> CINFO: %d\n>> FCHECK: %d\n>> FDICT: %d\n>> FLEVEL: %d\n",
                        CM, 
                        CINFO, 
                        FCHECK, 
                        FDICT, 
                        FLEVEL); 
                if (CM != 8) {
                    FreePng(&PngImage);
                    return No();
                }
                Advance(&PngStream, ChunkHeader->Length - 2);
            } break;
            case FourCC("IEND"): {
                return Yes(PngImage);
            } break;
            default: {
                printf("%c%c%c%c: %d\n", 
                    ChunkHeader->Type[0], 
                    ChunkHeader->Type[1],
                    ChunkHeader->Type[2],
                    ChunkHeader->Type[3],
                    ChunkHeader->Length);
                Advance(&PngStream, ChunkHeader->Length);
            };
        }
        Advance<png_chunk_footer>(&PngStream);
    }

    FreePng(&PngImage);
    return No();
}

int main() {    
    maybe<read_file_result> PngFile_ = ReadFile("test.png");
    if (!PngFile_){
        return 1;
    }
    read_file_result& PngFile = PngFile_.This;
    Defer { free(PngFile.Memory); }; 

    ParsePng(PngFile.Memory, PngFile.MemorySize);

    printf("Done!");
    return 0;
}
