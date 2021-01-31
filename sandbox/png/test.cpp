#include <stdio.h>
#include <stdlib.h>
#include "../../code/mm_core.h"
#include "../../code/mm_bitwise.h"
#include "../../code/mm_stream.h"

static inline maybe<stream>
ReadFileToMemory(const char* Filename) {
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

    stream Ret = Stream(FileMemory, Filesize);

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

struct png_chunk_data_pHYs {
    // Intended pixel size
    u32 PixelsPerUnitX;
    u32 PixelsPerUnitY;
    u8 UnitSpecifier;
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

struct png_chunk_data_gAMA {
    // Represents gamma * 100000
    u32 Gamma;
};

struct png_chunk_data_sRGB {
    u8 RenderingIntent; 
};

struct png_chunk_data_PLTE {
    u8 Red, Green, Blue;
};

struct png_chunk_footer {
    u32 Crc; 
};

#pragma pack(pop)

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

    if (IHDR->ColourType != 6 &&
        IHDR->BitDepth != 8 &&
        IHDR->CompressionMethod &&
        IHDR->FilterMethod != 0 &&
        IHDR->InterlaceMethod != 0) {
        return false;
    }

    return true;
}


int main() {    
    maybe<stream> PngStream_ = ReadFileToMemory("test.png");
    if (!PngStream_){
        return 1;
    }
    stream& PngStream = PngStream_.This;
    Defer { free(PngStream.Contents.Elements); }; 

    const u8 PngSignature[] = { 137, 80, 78, 71, 13, 10, 26, 10 };

    // Read the signature
    {
        auto* PngHeader = Consume<png_header>(&PngStream);  
        for (u32 I = 0; I < ArrayCount(PngSignature); ++I) {
            if (PngSignature[I] != PngHeader->Signature[I]) {
                printf("Png Singature wrong!\n");
                return 1;
            }
        }
    }


    // Process the rest of the chunks
    {

        b8 IsRunning = true;
        png_image Image = {};
        b8 IsZlibInitialized = false;
        u32 CM, CINFO, FCHECK, FDICT, FLEVEL; 
        while(IsRunning ) {
            auto* ChunkHeader = Consume<png_chunk_header>(&PngStream);
            EndianSwap(&ChunkHeader->Length);
            printf("%c%c%c%c: %d\n", 
                ChunkHeader->Type[0], 
                ChunkHeader->Type[1],
                ChunkHeader->Type[2],
                ChunkHeader->Type[3],
                ChunkHeader->Length);
            switch(ChunkHeader->TypeU32) {
                case FourCC("IHDR"): {
                    auto* Chunk = Consume<png_chunk_data_IHDR>(&PngStream);
                    EndianSwap(&Chunk->Width);
                    EndianSwap(&Chunk->Height);

                    if (!IsSupported(Chunk)) {
                        printf("Not supported format\n");
                        IsRunning = false;
                    }
                    else {
                        Image.Width = Chunk->Width;
                        Image.Height = Chunk->Height;
                        Image.Channels = 4;
                        // 4 for channels
                        Image.Data = malloc(Image.Width * 
                                            Image.Height * 
                                            Image.Channels);
                    }
                } break;
                case FourCC("IDAT"): { 
                    if (!IsZlibInitialized) {
                        auto* Raw = Peek<png_zlib_header>(&PngStream);
                        CINFO = ConsumeBits(&PngStream, 4);
                        CM = ConsumeBits(&PngStream, 4);
                        FLEVEL = ConsumeBits(&PngStream, 2);
                        FDICT = ConsumeBits(&PngStream, 1);
                        FCHECK = ConsumeBits(&PngStream, 5);
                        printf("%d %d\n", Raw->Bytes[0], Raw->Bytes[1]); 
                        printf(">> CM: %d\n>> CINFO: %d\n>> FCHECK: %d\n>> FDICT: %d\n>> FLEVEL: %d\n",
                                CM, 
                                CINFO, 
                                FCHECK, 
                                FDICT, 
                                FLEVEL); 
                        
                        if (CM != 8) {
                            printf("Not supported format\n");
                            IsRunning = false;
                        }
                        else {
                            IsZlibInitialized = true;
                            for (u32 I = 0; I < ChunkHeader->Length - 2; ++I) {
                                u8* Byte = Consume<u8>(&PngStream);
                                printf("%d ", (*Byte)); 
                            }
                        }

                    }
                    else {
                        for (u32 I = 0; I < ChunkHeader->Length - 2; ++I) {
                            u8* Byte = Consume<u8>(&PngStream);
                            printf("%d ", (*Byte)); 
                        }
                    }
                    printf("\n");
                } break;
                case FourCC("IEND"): {
                    IsRunning = false; 
                } break;
                case FourCC("PLTE"): {
                    // Only absolutely required by ColorType == 3
                    // Thus we don't give a shit about this atm.
                    Consume<png_chunk_data_PLTE>(&PngStream);
                } break;
                case FourCC("sRGB"): {
                    // We don't give a shit about this atm
                    Consume<png_chunk_data_sRGB>(&PngStream);
                } break;
                case FourCC("gAMA"): {
                    // We don't give a shit about this atm
                    Consume<png_chunk_data_gAMA>(&PngStream);
                } break;
                case FourCC("pHYs"): {
                    // We don't give a shit about this atm
                    Consume<png_chunk_data_pHYs>(&PngStream);
                } break;
                default: {
                    printf("Unknown chunk header!\n");
                    IsRunning = false;
                }
            }
            // Do we give a shit about this?
            Consume<png_chunk_footer>(&PngStream);
        }

        free(Image.Data);

    }


    printf("Done!");
    return 0;
}
