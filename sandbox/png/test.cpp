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
// - compression flags bit 0-3: Compression Method (CM)
// - compression flags bit 4-7: Compression Info (CINFO)
// Bytes[1]:
// - additional flags bit 0-4: FCHECK 
// - additional flags bit 5: Preset dictionary (FDICT)
// - additional flags bit 6-7: Compression level (FLEVEL)
struct png_IDAT_header {
    u8 CompressionFlags;
    u8 AdditionalFlags;
};

static inline void
FreePng(png_image Png) {
    free(Png.Data);
}

static inline maybe<png_image> 
ParsePng(arena* Arena, 
         void* PngMemory, 
         usize PngMemorySize) 
{
    const u8 PngSignature[] = { 137, 80, 78, 71, 13, 10, 26, 10 };
    

    stream PngStream = Stream(PngMemory, PngMemorySize); 
    scratch Scratch = BeginScratch(Arena);

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
    Consume<png_chunk_footer>(&PngStream);

    // Search for IDAT header
    u32 CM, CINFO, FCHECK, FDICT, FLEVEL;
    while(!IsEos(&PngStream)) {
        ChunkHeader = Consume<png_chunk_header>(&PngStream);
        EndianSwap(&ChunkHeader->Length);
        switch(ChunkHeader->TypeU32) {
            case FourCC("IDAT"): {
                printf("Length: %d\n", ChunkHeader->Length);
#if 0
                auto* Header = Consume<png_IDAT_header>(&PngStream);
                CM = Header->CompressionFlags & 0x0F;
                CINFO = (Header->CompressionFlags >> 4) & 0x0F;
                FCHECK = Header->AdditionalFlags & 0x1F;
                FDICT = Header->AdditionalFlags >> 5 & 0x01;
                FLEVEL = Header->AdditionalFlags >> 6 & 0x03;
#else
                CINFO = ConsumeBits(&PngStream, 4);
                CM = ConsumeBits(&PngStream, 4);
                FLEVEL = ConsumeBits(&PngStream, 2); //useless?
                FDICT = ConsumeBits(&PngStream, 1);
                FCHECK = ConsumeBits(&PngStream, 5); //not needed?
#endif
                printf(">> CM: %d\n>> CINFO: %d\n>> FCHECK: %d\n>> FDICT: %d\n>> FLEVEL: %d\n",
                        CM, 
                        CINFO, 
                        FCHECK, 
                        FDICT, 
                        FLEVEL); 
                if (CM != 8 || FDICT != 0 || CINFO > 7) {
                    return No();
                }

                // TODO: Change to arena
                
                void* BitmapData = PushBlock(Scratch,
                                             IHDR->Width * 
                                             IHDR->Height * 
                                             4);
                u8 BFINAL = 0;
                while(BFINAL == 0){
                    u16 BTYPE = (u8)ConsumeBits(&PngStream, 2);
                    BFINAL = (u8)ConsumeBits(&PngStream, 1);
                    printf(">>> BFINAL: %d\n", BFINAL);
                    printf(">>> BTYPE: %d\n", BTYPE);
                    switch(BTYPE) {
                        case 0b00: {
                            // no compression
                            ConsumeBits(&PngStream, 5);
                            u16 LEN = (u16)ConsumeBits(&PngStream, 16);
                            u16 NLEN = (u16)ConsumeBits(&PngStream, 16);
                            printf(">>>>No compression\n");
                            printf(">>>>> LEN: %d\n", LEN);
                            printf(">>>>> NLEN: %d\n", NLEN);
                            if ((u16)LEN != ~((u16)(NLEN))) {
                                printf("LEN vs NLEN mismatch!\n");
                                return No();
                            }
                        } break;
                        case 0b01: 
                        case 0b10:
                        {
                            printf("Huffman\n");
                            // TODO: Is 512 really the max?
                            u32 LitLenTable[512];
                            u32 HLIT = 0;
                            u32 HDIST = 0;
                            if (BTYPE == 0b01) {
                                // Fixed huffman
                                // Read representation of code trees
                                HLIT = 288;
                                HDIST = 32;
                                u32 BitCounts[][2] = {
                                    {143, 8},
                                    {255, 9},
                                    {279, 7},
                                    {287, 8},
                                };

                                for(u32 RangeIndex = 0, BitCountIndex = 0; 
                                    RangeIndex < ArrayCount(BitCounts); 
                                    ++RangeIndex) {
                                    u32 BitCount = BitCounts[RangeIndex][1];
                                    u32 EndRange = BitCounts[RangeIndex][0];
                                    while(BitCountIndex <= EndRange) {
                                        LitLenTable[BitCountIndex++] = BitCount;
                                    }
                                }
                            }
                            else // BTYPE == 0b10
                            {
                                //Dynamic buffman
                            }


                            // Then we do the Huffman decoding
                            u32 SymbolCount = HLIT;
                            {
#define PNG_HUFFMAN_MAX_BIT_COUNT 16
                                // 1. Count the number of codes for each code length
                                u32 LenCountTable[PNG_HUFFMAN_MAX_BIT_COUNT] = {};
                                for (u32 LitIndex = 0; 
                                     LitIndex < SymbolCount;
                                     ++LitIndex) 
                                {
                                    u32 Len = LitLenTable[LitIndex];
                                    Assert(Len < PNG_HUFFMAN_MAX_BIT_COUNT);
                                    ++LenCountTable[Len];
                                }

                                // 2. Numerical value of smallest code for each code length
                                u32 NextUnusedCode[PNG_HUFFMAN_MAX_BIT_COUNT] = {};
                                LenCountTable[0] = 0;
                                {
                                }
                            }



                        } break;
                        default: {
                            printf("Error\n");
                            return No();
                        }
                    }
                }
                //Advance(&PngStream, ChunkHeader->Length - 2);
            } break;
            case FourCC("IEND"): {
                png_image Ret = {};
                Ret.Width = IHDR->Width;
                Ret.Height = IHDR->Height;
                Ret.Channels = 4;
                return Yes(Ret);
            } break;
            default: {
                printf("%c%c%c%c: %d\n", 
                    ChunkHeader->Type[0], 
                    ChunkHeader->Type[1],
                    ChunkHeader->Type[2],
                    ChunkHeader->Type[3],
                    ChunkHeader->Length);
                Consume(&PngStream, ChunkHeader->Length);
            };
        }
        Consume<png_chunk_footer>(&PngStream);
    }
    return No();
}

int main() {    
    usize MemorySize = Megabytes(1);
    void * Memory = malloc(MemorySize);
    if (!Memory) { return 1; }
    Defer { free(Memory); };  
    arena AppArena = Arena(Memory, MemorySize);

    maybe<read_file_result> PngFile_ = ReadFile("test.png");
    if (!PngFile_){
        return 1;
    }
    read_file_result& PngFile = PngFile_.This;
    Defer { free(PngFile.Memory); }; 

    ParsePng(&AppArena, PngFile.Memory, PngFile.MemorySize);

    printf("Done!");
    return 0;
}
