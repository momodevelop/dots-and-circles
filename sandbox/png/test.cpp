#include <stdio.h>
#include <stdlib.h>
#include "../../code/momo.h"
#include "../../code/mm_bitwise.h"
#include "../../code/mm_stream.h"

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
        return FALSE;
    }
    Defer{ fclose(File); };
    
    
    fseek(File, 0, SEEK_END);
    s32 Filesize = ftell(File);
    fseek(File, 0, SEEK_SET);
    
    void* FileMemory = Arena_PushBlock(Arena, Filesize);
    fread(FileMemory, 1, Filesize, File); 
    
    Result->Memory = FileMemory;
    Result->MemorySize = Filesize;
    
    return TRUE;
}

/// Png start here
static constexpr usize PngMaxBits = 15;
static constexpr usize PngMaxDistCodes = 32;
static constexpr usize PngMaxFixedLitCodes = 288;
struct png_context {
    stream Stream;
    arena* Arena; 
    
    b32 IsImageInitialized;
    stream ImageStream;
    stream DepressedImageStream;
    
    u32 ImageWidth;
    u32 ImageHeight;
    u32 ImageChannels;
    
    arena_mark ImageStreamMark;
    arena_mark DepressedImageStreamMark;
};

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

struct png_huffman {
    u16* CodeSymTable; // Canonical ordered symbols
    u32 CodeSymTableSize;

    u16* LenCountTable;
    u32 LenCountTableSize;
};

static inline png_huffman
Huffman(arena* Arena, 
        u16* SymLenTable,
        u32 SymLenTableSize, 
        u32 LenCountTableCap,
        u32 CodeSymTableCap) 
{
    png_huffman Ret = {};

    Ret.CodeSymTableSize = CodeSymTableCap;
    Ret.CodeSymTable = Arena_PushArray(u16, Arena, CodeSymTableCap);

    Ret.LenCountTableSize = LenCountTableCap;
    Ret.LenCountTable = Arena_PushArray(u16, Arena, LenCountTableCap);
    
    // 1. Count the number of codes for each code length
    for (u32 Sym = 0; Sym < SymLenTableSize; ++Sym) 
    {
        u16 Len = SymLenTable[Sym];
        Assert(Len < PngMaxBits);
        ++Ret.LenCountTable[Len];
    }
    
    // 2. Numerical value of smallest code for each code length
    u16 LenOffsetTable[PngMaxBits+1] = {};
    for (u32 Len = 1; Len < PngMaxBits; ++Len)
    {
        LenOffsetTable[Len+1] = LenOffsetTable[Len] + Ret.LenCountTable[Len]; 
    }
    
    
    // 3. Assign numerical values to all codes
    for (u32 Sym = 0; Sym < SymLenTableSize; ++Sym)
    {
        u16 Len = SymLenTable[Sym];
        if (Len > 0) {
            u16 Code = LenOffsetTable[Len]++;
            Ret.CodeSymTable[Code] = (u16)Sym;
        }
    }
    
    return Ret;
}

static inline u32
Decode(stream* SrcStream, png_huffman* Huffman) {
    s32 Code = 0;
    s32 First = 0;
    s32 Index = 0;
    
    for (s32 Len = 1; Len <= PngMaxBits; ++Len) {
        Code |= Stream_ConsumeBits(SrcStream, 1);
        s32 Count = Huffman->LenCountTable[Len];
        if(Code - Count < First)
            return Huffman->CodeSymTable[Index + (Code - First)];
        Index += Count;
        First += Count;
        First <<= 1;
        Code <<= 1;
    }
    
    // TODO: Should return some kind of error code
    return 0;
}

// TODO: This is already zlib/DEFLATE related
// Maybe we can move this to another function/use another struct?
// The issue is optimization I guess? Especially for fixed-huffman codes.
#if 0
static inline b32
Deflate(png_context* Context) 
{
    u8 BFINAL = 0;
    while(BFINAL == 0){
        BFINAL = (u8)Stream_ConsumeBits(&Context->Stream, 1);
        u16 BTYPE = (u8)Stream_ConsumeBits(&Context->Stream, 2);
        printf(">>> BFINAL: %d\n", BFINAL);
        printf(">>> BTYPE: %d\n", BTYPE);
        switch(BTYPE) {
            case 0b00: {
                printf(">>>> No compression\n");
                Stream_ConsumeBits(&Context->Stream, 5);
                u16 LEN = (u16)Stream_ConsumeBits(&Context->Stream, 16);
                u16 NLEN = (u16)Stream_ConsumeBits(&Context->Stream, 16);
                printf(">>>>> LEN: %d\n", LEN);
                printf(">>>>> NLEN: %d\n", NLEN);
                if ((u16)LEN != ~((u16)(NLEN))) {
                    printf("LEN vs NLEN mismatch!\n");
                    return false;
                }
                // TODO: complete this
            } break;
            case 0b01: 
            case 0b10: {
                png_huffman LitHuffman = {};
                png_huffman DistHuffman = {};
                
                if (BTYPE == 0b01) {
                    // Fixed huffman
                    printf(">>>> Fixed huffman\n");
                    
                    u16 LitLenTable[PngMaxFixedLitCodes] = {};
                    u16 DistLenTable[PngMaxDistCodes] = {};
                    
                    usize Lit = 0;
                    for (; Lit < 144; ++Lit) 
                        LitLenTable[Lit] = 8;
                    for (; Lit < 256; ++Lit) 
                        LitLenTable[Lit] = 9;
                    for (; Lit < 280; ++Lit) 
                        LitLenTable[Lit] = 7;
                    for (; Lit < PngMaxFixedLitCodes; ++Lit) 
                        LitLenTable[Lit] = 8;
                    for (Lit = 0; Lit < PngMaxDistCodes; ++Lit) 
                        DistLenTable[Lit] = 5;
                    
                    LitHuffman = Huffman(Context->Arena, 
                                         LitLenTable, 
                                         PngMaxFixedLitCodes
                                         PngMaxBits+1,
                                         PngMaxFixedLitCodes);
                    DistHuffman = Huffman(Context->Arena,
                                          DistLenTable,
                                          PngMaxDistCodes
                                          PngMaxBits+1,
                                          PngMaxDistCodes);
                    
                }
                else // BTYPE == 0b10
                {
                    // TODO: Dynamic huffman
                    printf(">>>> Dynamic huffman not supported\n");
                    return false;
                }
                
                static const short Lens[29] = { /* Size base for length codes 257..285 */
                    3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31,
                    35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258};
                static const short LenExBits[29] = { /* Extra bits for length codes 257..285 */
                    0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2,
                    3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0};
                static const short Dists[30] = { /* Offset base for distance codes 0..29 */
                    1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193,
                    257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145,
                    8193, 12289, 16385, 24577};
                static const short DistExBits[30] = { /* Extra bits for distance codes 0..29 */
                    0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6,
                    7, 7, 8, 8, 9, 9, 10, 10, 11, 11,
                    12, 12, 13, 13};
                u32 LenCountTable[PngMaxBits + 1] = {};
                for (;;) 
                {
                    u32 Sym = Decode(Context, &LitHuffman);
                    if (Sym <= 255) { 
                        u8 ByteToWrite = (u8)(Sym & 0xFF); 
                        printf("%02X ", ByteToWrite);
                        Context->ImageData[Context->ImageDataCount++] = ByteToWrite;
                    }
                    else if (Sym >= 257) {
                        Sym -= 257;
                        if (Sym >= 29) {
                            printf("Invalid Symbol 1\n"); 
                            return false;
                        }
                        u32 Len = Lens[Sym] + Stream_ConsumeBits(&Context->Stream, LenExBits[Sym]);
                        Sym = Decode(Context, &DistHuffman);
                        if (Sym < 0) {
                            printf("Invalid Symbol 2\n");
                            return false;
                        }
                        u32 Dist = Dists[Sym] + Stream_ConsumeBits(&Context->Stream, DistExBits[Sym]);
                        while(Len--) {
                            u8 ByteToWrite = Context->ImageData[Context->ImageDataCount - Dist];
                            printf("%02X ", ByteToWrite);
                            Context->ImageData[Context->ImageDataCount++] = ByteToWrite; 
                        }
                    }
                    else { 
                        // Sym == 256
                        break;
                    }
                }
                printf("\n");
                
                // TODO: Reconstruct pixels and send to final pixel output
                
            } break;
            default: {
                printf("Error\n");
                return false;
            }
        }
    }
    return true;
}
#else 
static inline b32
Deflate(stream* SrcStream, stream* DestStream, arena* Arena) 
{
    static const short Lens[29] = { /* Size base for length codes 257..285 */
        3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31,
        35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258};
    static const short LenExBits[29] = { /* Extra bits for length codes 257..285 */
        0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2,
        3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0};
    static const short Dists[30] = { /* Offset base for distance codes 0..29 */
        1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193,
        257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145,
        8193, 12289, 16385, 24577};
    static const short DistExBits[30] = { /* Extra bits for distance codes 0..29 */
        0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6,
        7, 7, 8, 8, 9, 9, 10, 10, 11, 11,
        12, 12, 13, 13};
    u8 BFINAL = 0;
    while(BFINAL == 0){
        BFINAL = (u8)Stream_ConsumeBits(SrcStream, 1);
        u16 BTYPE = (u8)Stream_ConsumeBits(SrcStream, 2);
        printf(">>> BFINAL: %d\n", BFINAL);
        printf(">>> BTYPE: %d\n", BTYPE);
        switch(BTYPE) {
            case 0b00: {
                printf(">>>> No compression\n");
                Stream_ConsumeBits(SrcStream, 5);
                u16 LEN = (u16)Stream_ConsumeBits(SrcStream, 16);
                u16 NLEN = (u16)Stream_ConsumeBits(SrcStream, 16);
                printf(">>>>> LEN: %d\n", LEN);
                printf(">>>>> NLEN: %d\n", NLEN);
                if ((u16)LEN != ~((u16)(NLEN))) {
                    printf("LEN vs NLEN mismatch!\n");
                    return false;
                }
                // TODO: complete this
            } break;
            case 0b01: 
            case 0b10: {
                png_huffman LitHuffman = {};
                png_huffman DistHuffman = {};
                
                if (BTYPE == 0b01) {
                    // Fixed huffman
                    printf(">>>> Fixed huffman\n");
                    
                    u16 LitLenTable[PngMaxFixedLitCodes] = {};
                    u16 DistLenTable[PngMaxDistCodes] = {};
                    
                    usize Lit = 0;
                    for (; Lit < 144; ++Lit) 
                        LitLenTable[Lit] = 8;
                    for (; Lit < 256; ++Lit) 
                        LitLenTable[Lit] = 9;
                    for (; Lit < 280; ++Lit) 
                        LitLenTable[Lit] = 7;
                    for (; Lit < PngMaxFixedLitCodes; ++Lit) 
                        LitLenTable[Lit] = 8;
                    for (Lit = 0; Lit < PngMaxDistCodes; ++Lit) 
                        DistLenTable[Lit] = 5;
                    
                    LitHuffman = Huffman(Arena, 
                                         LitLenTable, 
                                         PngMaxFixedLitCodes,
                                         PngMaxBits+1,
                                         PngMaxFixedLitCodes);
                    DistHuffman = Huffman(Arena,
                                          DistLenTable,
                                          PngMaxDistCodes,
                                          PngMaxBits+1,
                                          PngMaxDistCodes);
                    
                }
                else // BTYPE == 0b10
                {
                    // TODO: Dynamic huffman
                    printf(">>>> Dynamic huffman not supported\n");
                    return false;
                }
                
                u32 LenCountTable[PngMaxBits + 1] = {};
                for (;;) 
                {
                    u32 Sym = Decode(SrcStream, &LitHuffman);
                    if (Sym <= 255) { 
                        u8 ByteToWrite = (u8)(Sym & 0xFF); 
                        printf("%02X ", ByteToWrite);
                        Stream_WriteStruct(u8, DestStream, ByteToWrite);
                    }
                    else if (Sym >= 257) {
                        Sym -= 257;
                        if (Sym >= 29) {
                            printf("Invalid Symbol 1\n"); 
                            return false;
                        }
                        u32 Len = Lens[Sym] + Stream_ConsumeBits(SrcStream, LenExBits[Sym]);
                        Sym = Decode(SrcStream, &DistHuffman);
                        if (Sym < 0) {
                            printf("Invalid Symbol 2\n");
                            return false;
                        }
                        u32 Dist = Dists[Sym] + Stream_ConsumeBits(SrcStream, DistExBits[Sym]);
                        while(Len--) {
                            usize TargetIndex = DestStream->Current - Dist;
                            u8 ByteToWrite = DestStream->Contents[TargetIndex];
                            printf("%02X ", ByteToWrite);
                            Stream_WriteStruct(u8, DestStream, ByteToWrite);
                        }
                    }
                    else { 
                        // Sym == 256
                        break;
                    }
                }
                printf("\n");
                
                // TODO: Reconstruct pixels and send to final pixel output
                
            } break;
            default: {
                printf("Error\n");
                return false;
            }
        }
    }
    return true;
}
#endif



static inline b32
ParseIDATChunk(png_context* Context) {
    u32 CM, CINFO, FCHECK, FDICT, FLEVEL;
    CM = Stream_ConsumeBits(&Context->Stream, 4);
    CINFO = Stream_ConsumeBits(&Context->Stream, 4);
    FCHECK = Stream_ConsumeBits(&Context->Stream, 5); //not needed?
    FDICT = Stream_ConsumeBits(&Context->Stream, 1);
    FLEVEL = Stream_ConsumeBits(&Context->Stream, 2); //useless?
    printf(">> CM: %d\n\
           >> CINFO: %d\n\
           >> FCHECK: %d\n\
           >> FDICT: %d\n\
           >> FLEVEL: %d\n",
           CM, 
           CINFO, 
           FCHECK, 
           FDICT, 
           FLEVEL); 
    if (CM != 8 || FDICT != 0 || CINFO > 7) {
        return false;
    }
    
    if (!Context->IsImageInitialized) {
        printf("Result W/H/C: %d, %d, %d\n", Context->ImageWidth, 
               Context->ImageHeight, Context->ImageChannels);
        u32 ImageSize = Context->ImageWidth * 
            Context->ImageHeight * 
            Context->ImageChannels;
        
        Context->ImageStreamMark = Arena_Mark(Context->Arena);
        Context->ImageStream = Stream_CreateFromArena(Context->Arena, ImageSize);
        
        Context->DepressedImageStreamMark = Arena_Mark(Context->Arena);
        u32 DepressedImageDataSize = (Context->ImageWidth + 1) *
            Context->ImageHeight* 
            Context->ImageChannels;
        Context->DepressedImageStream = Stream_CreateFromArena(Context->Arena, DepressedImageDataSize);
        
        Context->IsImageInitialized = true;
    }
    
    arena_mark Scratch = Arena_Mark(Context->Arena);
    Defer { Arena_Revert(&Scratch); };
    // Deflate
    if (!Deflate(&Context->Stream, 
                 &Context->DepressedImageStream,
                 Scratch.Arena)) 
    {
        return false;
    }
    return true;
}

static inline maybe<png_image> 
ParsePng(arena* Arena, 
         void* PngMemory, 
         u32 PngMemorySize) 
{
    png_context Context = {};
    Context.Stream = Stream_CreateFromMemory(PngMemory, PngMemorySize); 
    Context.Arena = Arena;
    
    // Read the signature
    auto* PngHeader = Stream_ConsumeStruct(png_header, &Context.Stream);  
    if (!PngHeader) { return No(); }
    static constexpr u8 PngSignature[] = { 
        137, 80, 78, 71, 13, 10, 26, 10 
    }; 
    for (u32 I = 0; I < ArrayCount(PngSignature); ++I) {
        if (PngSignature[I] != PngHeader->Signature[I]) {
            printf("Png Singature wrong!\n");
            return No();
        }
    }
    
    // Check for IHDR which MUST appear first
    auto* ChunkHeader = Stream_ConsumeStruct(png_chunk_header, &Context.Stream);
    if (!ChunkHeader){ return No(); }
    if (ChunkHeader->TypeU32 != FourCC("IHDR")) { 
        return No(); 
        }
    auto* IHDR = Stream_ConsumeStruct(png_chunk_data_IHDR, &Context.Stream);
    if (!IHDR) { return No(); }
    
    // Unsupported details
    if (IHDR->ColourType != 6 &&
        IHDR->BitDepth != 8 &&
        IHDR->CompressionMethod &&
        IHDR->FilterMethod != 0 &&
        IHDR->InterlaceMethod != 0) 
    {
        return No();
    }
    EndianSwap(&IHDR->Width);
    EndianSwap(&IHDR->Height);
    
    Context.ImageWidth = IHDR->Width;
    Context.ImageHeight = IHDR->Height;
    Context.ImageChannels = 4;
    Stream_ConsumeStruct(png_chunk_footer, &Context.Stream);
    
    // Search for IDAT header
    while(!Stream_IsEos(&Context.Stream)) {
        ChunkHeader = Stream_ConsumeStruct(png_chunk_header, &Context.Stream);
        EndianSwap(&ChunkHeader->Length);
        switch(ChunkHeader->TypeU32) {
            case FourCC("IDAT"): {
                if (!ParseIDATChunk(&Context)) {
                    return No();
                }
            } break;
            case FourCC("IEND"): {
                png_image Ret = {};
                Ret.Width = Context.ImageWidth;
                Ret.Height = Context.ImageHeight;
                Ret.Channels = Context.ImageChannels;
                return Yes(Ret);
            } break;
            default: {
                Stream_ConsumeBlock(&Context.Stream, ChunkHeader->Length);
            };
        }
        Stream_ConsumeStruct(png_chunk_footer, &Context.Stream);
    }
    return No();
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
    
    ParsePng(&AppArena, PngFile.Memory, PngFile.MemorySize);
    
    printf("Done!");
    return 0;
}

