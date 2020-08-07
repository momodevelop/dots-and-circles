#include <stdio.h>
#include <stdlib.h>
#include "ryoji_common.cpp"


struct buffer {
    u8* Data;
    usize Count;
};
using string = buffer;



static inline u8*
Advance(buffer* Buffer, usize Count) {
    Assert(Buffer->Count >= Count);
    u8* Result = Buffer->Data;
    Buffer->Data += Count;
    Buffer->Count -= Count;
    
    return Result;
}

struct stream_chunk {
    buffer Contents;
    stream_chunk* Next;
};

struct stream {
    u32 BitCount;
    u32 BitBuffer;
    
    // NOTE(Momo): Points to Head's contents.
    // This is so that we don't modify Head directly.
    buffer Contents; 
    
    stream_chunk* Head;
    stream_chunk* Tail;
};

static inline void 
Init(stream* Stream, void* Memory, usize Size) {
    Stream->Contents = { (u8*)Memory, Size };
}


static inline void 
RefillIfNeeded(stream* Stream) {
    if (Stream->Contents.Count == 0 ) {
        stream_chunk* Temp = Stream->Head;
        Stream->Contents = Temp->Contents;
        Stream->Head = Temp->Next; //essentially a pop!
    }
}

static inline u32
PeekBits(const u8* P, u32 BitCount) {
    Assert(BitCount <= 32);
#if BIG_ENDIAN
#else
#endif
}


static inline void* 
ReadSize(stream* Stream, usize Size) {
    RefillIfNeeded(Stream);
    void* Ret = Advance(&Stream->Contents, Size);
    Assert(Ret);
    return Ret;
}

template<typename T>
static inline T* 
Read(stream* Stream) {
    return (T*)ReadSize(Stream, sizeof(T));
}

static inline void* 
PeekSize(stream* Stream, usize Size) {
    RefillIfNeeded(Stream);
    return Stream->Contents.Data;
}

template<typename T>
static inline T* 
Peek(stream* Stream) {
    return (T*)PeekSize(Stream, sizeof(T));
}


static inline b8
IsEOS(stream* Stream) {
    return Stream->Contents.Count == 0 && Stream->Head == nullptr;
}

static inline void
EndianSwap(u32 *Value) {
    u32 V = (*Value);
    *Value = ((V << 24) |
              ((V & 0xFF00) << 8) |
              ((V >> 8) & 0xFF00) |
              (V >> 24));
}


struct bitmap {
    u32 Width;
    u32 Height;
    void* Pixels;
};

struct png_signature {
    u8 E[8];
};

struct png_ihdr {
    u32 Width;
    u32 Height;
    u8 BitDepth, ColorType, CompressionMethod, FilterMethod, InterlaceMethod;
};

struct png_idat_header {
    u8 ZLibMethodFlags;
    u8 AdditionalFlags;
};

static inline void
PrintIHDR(png_ihdr IHDR) {
    printf("Width: %d\nHeight: %d\nBitDepth: %d\nColorType: %d\nCompressionMethod: %d\nFilterMethod: %d\nInterlaceMethod: %d\n", IHDR.Width, IHDR.Height, IHDR.BitDepth, IHDR.ColorType, IHDR.CompressionMethod, IHDR.FilterMethod, IHDR.InterlaceMethod); 
}

constexpr static inline u32 
FourCC(const char p[5]) {
    return (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3];
}

// TODO(Momo): Maybe a better way to do this?
#if BIG_ENDIAN
#define SwapIfDiffEndian(V)
#else
#define SwapIfDiffEndian(V) EndianSwap(&V)
#endif

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
    
    stream FileStream = {};
    Init(&FileStream, Memory, Filesize);
    
    // NOTE(Momo): Check signature headers
    {
        auto* Signature = Read<png_signature>(&FileStream);
        if (!(Signature->E[0] == 0x89 &&
              Signature->E[1] == 0x50 &&
              Signature->E[2] == 0x4E &&
              Signature->E[3] == 0x47 &&
              Signature->E[4] == 0x0D &&
              Signature->E[5] == 0x0A &&
              Signature->E[6] == 0x1A &&
              Signature->E[7] == 0x0A)) {
            printf("Bad Signature\n");
            return {};
        }
    }
    printf("It's png!\n");
    
    // NOTE(Momo): Trying to process a IHDR chunk (must be first)
    png_ihdr IHDR;
    stream IdatStream = {};
    
    bool IsSupported  = false;
    for(;;) {
        // Header
        u32 ChunkLength = *Read<u32>(&FileStream); 
        u32 ChunkType = *Read<u32>(&FileStream);
        
        SwapIfDiffEndian(ChunkLength);
        SwapIfDiffEndian(ChunkType);
        
        u8* ChunkTypeU8 = (u8*)&ChunkType;
        printf("%c%c%c%c\n", ChunkTypeU8[3], ChunkTypeU8[2], ChunkTypeU8[1], ChunkTypeU8[0]);
        
        // Data
        if (ChunkType == FourCC("IHDR")) {
            
            IHDR.Width = *Read<u32>(&FileStream); 
            IHDR.Height = *Read<u32>(&FileStream); 
            IHDR.BitDepth = *Read<u8>(&FileStream); 
            IHDR.ColorType = *Read<u8>(&FileStream); 
            IHDR.CompressionMethod = *Read<u8>(&FileStream); 
            IHDR.FilterMethod = *Read<u8>(&FileStream); 
            IHDR.InterlaceMethod = *Read<u8>(&FileStream); 
            
            SwapIfDiffEndian(IHDR.Width);
            SwapIfDiffEndian(IHDR.Height);
            
            PrintIHDR(IHDR);
            
            if (IHDR.BitDepth == 8 &&
                IHDR.ColorType == 6 &&
                IHDR.CompressionMethod == 0 &&
                IHDR.FilterMethod == 0 &&
                IHDR.InterlaceMethod == 0) 
            {
                printf("Supported!\n");
                IsSupported = true;
            }
        }
        
        else if (ChunkType == FourCC("IDAT")) {
            if (!IsSupported) 
                break;
            auto* NewChunk = (stream_chunk*)malloc(sizeof(stream_chunk));
            if (IdatStream.Head == nullptr) {
                IdatStream.Head = IdatStream.Tail = NewChunk;
            }
            else {
                IdatStream.Tail->Next = NewChunk;
                IdatStream.Tail = NewChunk;
            }
            
            u8* Data = (u8*)ReadSize(&FileStream, ChunkLength);
            NewChunk->Contents = { Data, ChunkLength };
        }
        else if (ChunkType == FourCC("IEND")) {
            break;
        }
        else {
            if (!IsSupported) 
                break;
            ReadSize(&FileStream, ChunkLength);
        }
        
        // Footer
        ReadSize(&FileStream, 4);
    }
    
    if (!IsSupported) {
        return {};
    }
    
    auto* IdatHeader = Read<png_idat_header>(&IdatStream); // no padding
    
    // Extract Zlib Info
    u8 CM = (IdatHeader->ZLibMethodFlags & 0xF);
    u8 CINFO = (IdatHeader->ZLibMethodFlags >> 4);
    u8 FCHECK = (IdatHeader->AdditionalFlags & 0x1F);
    u8 FDICT = (IdatHeader->AdditionalFlags >> 5) & 0x1;
    u8 FLEVEL = (IdatHeader->AdditionalFlags >> 6);
    
    printf("\tCM: %d\n", CM);
    printf("\tCINFO: %d\n", CINFO);
    printf("\tFCHECK: %d\n", FCHECK);
    printf("\tFDICT: %d\n", FDICT);
    printf("\tFLEVEL: %d\n", FLEVEL);
    
    if (FDICT != 0 || CM != 8) {
        return {};
    }
    
    // NOTE(Momo): Decompressing
    // TODO(Momo): Can start to allocate pixels here
    u8 BFINAL = 0;
#if 0
    while(BFINAL == 0) {
        
        BFINAL = (*ChunkItr) & 0x1;
        u8 BTYPE = ((*ChunkItr) >> 1) & 0x2;
        ChunkItr >>= 3;
        
        if (BType == 0) {
            // Uncompressed
            u16 LEN = ReadU16(ChunkItr, true);
            ChunkItr >>= 16;
            u16 NLEN = ReadU16(ChunkItr, true);
            ChunkItr >>= 16;
            
            printf("\tLEN: %d\n", LEN);
            printf("\tNLEN: %d\n", NLEN);
            
            
        }
        
        else if (BTYPE == 3) { // Error
            return {}
        }
        
        else {
        }
    }
    
    
    u32 CheckValue;
    
    u32 CompressedDataBlockSize = HeadChunk->DataSize - sizeof(CompressionMethodAndFlags) - sizeof(AdditionalFlags) - sizeof(CheckValue);
    
    printf("\t");
    for (u32 i = 0; i < CompressedDataBlockSize; ++i ){
        //printf("%X ", ChunkItr[i]);
    }
    printf("\n");
    //ChunkItr += CompressedDataBlockSize;
    
    //CheckValue = ReadU32(ChunkItr, true);
    //ChunkItr += sizeof(CheckValue);
    
    
#endif
    return {};
}

int main() {
    ReadPng("karu.png");
}