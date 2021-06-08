/* date = May 16th 2021 8:30 pm */
#ifndef MOMO_PNG_H
#define MOMO_PNG_H


//~ TODO(Momo)
// Dynamic Huffman
// Different filter methods

#define Png_Debug 0
#define Png_MaxBits 15
#define Png_MaxDistCodes 32
#define Png_MaxFixedLitCodes 288

#if Png_Debug
#include <stdio.h>
#define Png_Log(...) printf(__VA_ARGS__)
#else
#define Png_Log
#endif

struct png_context {
    stream Stream;
    arena* Arena; 
    
    b8 IsImageInitialized;
    stream ImageStream;
    stream DepressedImageStream;
    
    u32 ImageWidth;
    u32 ImageHeight;
    u32 ImageChannels;
    
    arena_mark ImageStreamMark;
    arena_mark DepressedImageStreamMark;
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

struct png_image {
    u32 Width;
    u32 Height; 
    u32 Channels;
    void* Data;
};


struct png_huffman {
    u16* CodeSymTable; // Canonical ordered symbols
    u32 CodeSymTableSize;
    
    u16* LenCountTable;
    u32 LenCountTableSize;
};

enum png_error {
    PngError_None, // 0 
    PngError_CannotReadHeader,
    PngError_BadSignature,
    PngError_CannotReadChunkHeader,
    PngError_FirstHeaderIsNotIHDR,
    PngError_CannotReadIHDR, // 5
    PngError_UnsupportedFormat,
    PngError_BadFormat,
    PngError_NLENvsLENMismatch,
    PngError_BadExtraCode,
    PngError_BadSymbol,
    PngError_BadBTYPE,
    PngError_UnsupportedIDATFormat,
    PngError_CannotReadFilterType,
    PngError_NotEnoughPixels,
    PngError_BadFilterType,
    PngError_DynamicHuffmanNotSupported,
};

static inline s32
Png_HuffmanDecode(bitstream* SrcStream, png_huffman* Huffman) {
    s32 Code = 0;
    s32 First = 0;
    s32 Index = 0;
    
    for (s32 Len = 1; Len <= Png_MaxBits; ++Len) {
        Code |= Bitstream_ConsumeBits(SrcStream, 1);
        s32 Count = Huffman->LenCountTable[Len];
        if(Code - Count < First)
            return Huffman->CodeSymTable[Index + (Code - First)];
        Index += Count;
        First += Count;
        First <<= 1;
        Code <<= 1;
    }
    
    return -1;
}

static inline png_huffman
Png_Huffman(arena* Arena, 
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
        Assert(Len < Png_MaxBits);
        ++Ret.LenCountTable[Len];
    }
    
    // 2. Numerical value of smallest code for each code length
    u16 LenOffsetTable[Png_MaxBits+1] = {};
    for (u32 Len = 1; Len < Png_MaxBits; ++Len)
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


static inline png_error
Png_Deflate(bitstream* SrcStream, stream* DestStream, arena* Arena) 
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
        12, 12, 13, 13 };
    
    u8 BFINAL = 0;
    while(BFINAL == 0){
        arena_mark Scratch = Arena_Mark(Arena);
        Defer { Arena_Revert(&Scratch); };
        
        BFINAL = (u8)Bitstream_ConsumeBits(SrcStream, 1);
        u16 BTYPE = (u8)Bitstream_ConsumeBits(SrcStream, 2);
        Png_Log(">>> BFINAL: %d\n", BFINAL);
        Png_Log(">>> BTYPE: %d\n", BTYPE);
        switch(BTYPE) {
            case 0b00: {
                Png_Log(">>>> No compression\n");
                Bitstream_ConsumeBits(SrcStream, 5);
                u16 LEN = (u16)Bitstream_ConsumeBits(SrcStream, 16);
                u16 NLEN = (u16)Bitstream_ConsumeBits(SrcStream, 16);
                Png_Log(">>>>> LEN: %d\n", LEN);
                Png_Log(">>>>> NLEN: %d\n", NLEN);
                if ((u16)LEN != ~((u16)(NLEN))) {
                    Png_Log("LEN vs NLEN mismatch!\n");
                    return PngError_NLENvsLENMismatch;
                }
                // TODO: complete this
            } break;
            case 0b01: 
            case 0b10: {
                png_huffman LitHuffman = {};
                png_huffman DistHuffman = {};
                
                if (BTYPE == 0b01) {
                    // Fixed huffman
                    Png_Log(">>>> Fixed huffman\n");
                    
                    u16 LitLenTable[Png_MaxFixedLitCodes] = {};
                    u16 DistLenTable[Png_MaxDistCodes] = {};
                    
                    u32 Lit = 0;
                    for (; Lit < 144; ++Lit) 
                        LitLenTable[Lit] = 8;
                    for (; Lit < 256; ++Lit) 
                        LitLenTable[Lit] = 9;
                    for (; Lit < 280; ++Lit) 
                        LitLenTable[Lit] = 7;
                    for (; Lit < Png_MaxFixedLitCodes; ++Lit) 
                        LitLenTable[Lit] = 8;
                    for (Lit = 0; Lit < Png_MaxDistCodes; ++Lit) 
                        DistLenTable[Lit] = 5;
                    
                    LitHuffman = Png_Huffman(Arena, 
                                             LitLenTable, 
                                             Png_MaxFixedLitCodes,
                                             Png_MaxBits+1,
                                             Png_MaxFixedLitCodes);
                    DistHuffman = Png_Huffman(Arena,
                                              DistLenTable,
                                              Png_MaxDistCodes,
                                              Png_MaxBits+1,
                                              Png_MaxDistCodes);
                    
                }
                else // BTYPE == 0b10
                {
                    // TODO: Dynamic huffman
                    Png_Log(">>>> Dynamic huffman not supported\n");
                    return PngError_DynamicHuffmanNotSupported;
                }
                
                u32 LenCountTable[Png_MaxBits + 1] = {};
                for (;;) 
                {
                    s32 Sym = Png_HuffmanDecode(SrcStream, &LitHuffman);
                    // NOTE(Momo): Normal case
                    if (Sym <= 255) { 
                        u8 ByteToWrite = (u8)(Sym & 0xFF); 
                        Stream_Write<u8>(DestStream, ByteToWrite);
                    }
                    // NOTE(Momo): Extra code case
                    else if (Sym >= 257) {
                        Sym -= 257;
                        if (Sym >= 29) {
                            return PngError_BadExtraCode;
                        }
                        u32 Len = Lens[Sym] + Bitstream_ConsumeBits(SrcStream, LenExBits[Sym]);
                        Sym = Png_HuffmanDecode(SrcStream, &DistHuffman);
                        if (Sym < 0) {
                            return PngError_BadSymbol;
                        }
                        u32 Dist = Dists[Sym] + Bitstream_ConsumeBits(SrcStream, DistExBits[Sym]);
                        while(Len--) {
                            u32 TargetIndex = DestStream->Current - Dist;
                            u8 ByteToWrite = DestStream->Contents[TargetIndex];
                            Stream_Write<u8>(DestStream, ByteToWrite);
                        }
                    }
                    else { 
                        // Sym == 256
                        break;
                    }
                }
                Png_Log("\n");
            } break;
            default: {
                Png_Log("Error\n");
                return PngError_BadBTYPE;
            }
        }
    }
    return PngError_None;
}


static inline png_error
Png_Parse(png_image* Png,
          arena* Arena,
          void* PngMemory,
          u32 PngMemorySize) 
{
    bitstream Stream = {};
    Bitstream_Create(&Stream, PngMemory, PngMemorySize); 
    
    auto* PngHeader = Bitstream_Consume<png_header>(&Stream);  
    if (!PngHeader) { 
        return PngError_CannotReadHeader; 
    }
    static const u8 PngSignature[] = { 
        137, 80, 78, 71, 13, 10, 26, 10 
    };
    for (u32 I = 0; I < ArrayCount(PngSignature); ++I) {
        if (PngSignature[I] != PngHeader->Signature[I]) {
            Png_Log("Png Singature wrong!\n");
            return PngError_BadSignature;
        }
    }
    
    auto* ChunkHeader = Bitstream_Consume<png_chunk_header>(&Stream);
    if (!ChunkHeader) { 
        return PngError_CannotReadChunkHeader; 
    }
    
    // NOTE(Momo): IHDR must appear first
    if (ChunkHeader->TypeU32 != FourCC("IHDR")) { 
        return PngError_FirstHeaderIsNotIHDR; 
    }
    auto* IHDR = Bitstream_Consume<png_chunk_data_IHDR>(&Stream);
    if (!IHDR) { 
        return PngError_CannotReadIHDR; 
    }
    
    
    
    
    if ((IHDR->ColourType != 6 || IHDR->ColourType == 2) &&
        IHDR->BitDepth != 8 &&
        IHDR->CompressionMethod &&
        IHDR->FilterMethod != 0 &&
        IHDR->InterlaceMethod != 0) 
    {
        // TODO: Expand this to different errors
        return PngError_UnsupportedFormat;
    }
    
    // ColourType 1 = Pallete used
    // ColourType 2 = Colour used 
    // ColourType 4 = Alpha used
    u32 ImageChannels = 0;
    switch(IHDR->ColourType){
        case 2: {
            ImageChannels = 3; // RGB
        } break;
        case 6: { 
            ImageChannels = 4; // RGBA
        } break;
        default: {
            return PngError_UnsupportedFormat;
        }
    }
    EndianSwapU32(&IHDR->Width);
    EndianSwapU32(&IHDR->Height);
    
    // Just consume the footer. 
    // TODO: CRC check with footer
    Bitstream_Consume<png_chunk_footer>(&Stream);
    
    u32 ImageSize = IHDR->Width * IHDR->Height * ImageChannels;
    
    // NOTE(Momo): For reserving memory for unfiltered data that is generated 
    // as we decode the file
    arena_mark ActualImageStreamMark = Arena_Mark(Arena);
    stream ActualImageStream = {};
    Stream_CreateFromArena(&ActualImageStream, Arena, ImageSize);
    
    // Search for IDAT header
    while(!Bitstream_IsEos(&Stream)) {
        ChunkHeader = Bitstream_Consume<png_chunk_header>(&Stream);
        EndianSwapU32(&ChunkHeader->Length);
        switch(ChunkHeader->TypeU32) {
            case FourCC("IDAT"): {
                // temporary stream just to process IDAT
                bitstream IDATStream = Stream; 
                
                u32 CM = Bitstream_ConsumeBits(&IDATStream, 4);
                u32 CINFO = Bitstream_ConsumeBits(&IDATStream, 4);
                u32 FCHECK = Bitstream_ConsumeBits(&IDATStream, 5); //not needed?
                u32 FDICT = Bitstream_ConsumeBits(&IDATStream, 1);
                u32 FLEVEL = Bitstream_ConsumeBits(&IDATStream, 2); //useless?
                
                Png_Log(">> CM: %d\n>> CINFO: %d\n>> FCHECK: %d\n>> FDICT: %d\n>>FLEVEL: %d\n",
                        CM, 
                        CINFO,
                        FCHECK, 
                        FDICT, 
                        FLEVEL); 
                if (CM != 8 || FDICT != 0 || CINFO > 7) {
                    return PngError_UnsupportedIDATFormat;
                }
                
                // NOTE(Momo): Allow space for unfiltered image
                u32 UnfilteredImageSize = IHDR->Width * (ImageChannels + 1);
                arena_mark UnfilteredImageStreamMark = Arena_Mark(Arena);
                
                stream UnfilteredImageStream = {};
                Stream_CreateFromArena(&UnfilteredImageStream, Arena, UnfilteredImageSize);
                
                png_error DeflateError = Png_Deflate(&IDATStream, &UnfilteredImageStream, Arena);
                if (DeflateError != PngError_None) {
                    Arena_Revert(&ActualImageStreamMark);
                    return DeflateError;
                }
                
                // NOTE(Momo): Filter
                // Data always starts with 1 byte indicating the type of filter
                // followed by the rest of the chunk.
                Stream_Reset(&UnfilteredImageStream);
                while(!Stream_IsEos(&UnfilteredImageStream)) {
                    u8* FilterType = Stream_Consume<u8>(&UnfilteredImageStream);
                    if (FilterType == nullptr) {
                        Arena_Revert(&ActualImageStreamMark);
                        return PngError_CannotReadFilterType;
                    }
                    Png_Log("%02X: ", (u32)(*FilterType));
                    switch(*FilterType) {
                        case 0: { // None
                            for (u32 I = 0; I < IHDR->Width; ++I ){
                                for (u32 J = 0; J < ImageChannels; ++J) {
                                    u8* PixelByte = Stream_Consume<u8>(&UnfilteredImageStream);
                                    if (PixelByte == nullptr) {
                                        Arena_Revert(&ActualImageStreamMark);
                                        return PngError_NotEnoughPixels;
                                    }
                                    Png_Log("%02X ", (u32)(*PixelByte));
                                    Stream_Write<u8>(&ActualImageStream, *PixelByte);
                                    
                                }
                            }
                            
                            
                            
                        } break;
                        /*case 1: { // Sub
                        } break;
                        case 2: { // Up
                        } break;
                        case 3: { // Average
                        } break;
                        case 4: { // Paeth
                        } break;*/
                        default: {
                            return PngError_BadFilterType;
                        };
                    };
                    Png_Log("\n");
                    Arena_Revert(&UnfilteredImageStreamMark);
                }
                
                Bitstream_ConsumeBlock(&Stream, ChunkHeader->Length);
                
            } break;
            case FourCC("IEND"): {
                Png->Width = IHDR->Width;
                Png->Height = IHDR->Height;
                Png->Channels = ImageChannels;
                Png->Data = ActualImageStream.Contents;
                return PngError_None;
            } break;
            default: {
                // NOTE(Momo): For now, we don't care about the rest of the chunks
                Bitstream_ConsumeBlock(&Stream, ChunkHeader->Length);
                
            };
        }
        Bitstream_Consume<png_chunk_footer>(&Stream);
    }
    
    return PngError_BadFormat;
    
    
    
    
}

#endif //MOMO_PNG_H
