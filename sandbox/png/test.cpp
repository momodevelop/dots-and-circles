#include <stdio.h>
#include <stdlib.h>
#include "../../code/mm_core.h"
#include "../../code/mm_bitwise.h"

#include "../../code/mm_stream.h"
#include "../../code/mm_stream.cpp"

struct Read_File_Result
{
    void* memory;
    usize memory_size;
};

static inline maybe<Read_File_Result>
read_file(const char* filename) {
    FILE* file = {};
    if (fopen_s(&file, filename, "rb") != 0) { 
        printf("Cannot find file\n");
        return No();
    }
    Defer{ fclose(file); };


    fseek(file, 0, SEEK_END);
    i32 file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    void* file_memory = malloc(file_size);
    fread(file_memory, 1, file_size, file); 

    fclose(file);

    Read_File_Result ret = {};
    ret.memory = file_memory;
    ret.memory_size = file_size;

    return Yes(ret);
}

/// Png start here
static constexpr usize PngMaxBits = 15;
static constexpr usize PngMaxDistCodes = 32;
static constexpr usize PngMaxFixedLitCodes = 288;
struct Png_Context {
    Stream main_stream;
    Stream img_stream;
    Stream depressed_img_stream;

    Memory_Arena* arena; 

    b32 is_image_initialized;

    u32 image_width;
    u32 image_height;
    u32 image_channels;

    scratch img_stream_mark;
    scratch depressed_img_stream_mark;
};

struct Png_Image {
    u32 width;
    u32 height; 
    u32 channels;
    void* data;
};


struct Png_Header {
    u8 signature[8];
};

// 5.3 Chunk layout
// | Length | Type | Data | CRC
struct Png_Chunk_Header {
    u32 length;
    union {
        u32 type_u32;
        u8 type[4];
    };
};


#pragma pack(push, 1)
struct Png_Chunk_Data_IHDR {
    u32 width;
    u32 height;
    u8 bit_depth;
    u8 color_type;
    u8 compression_method;
    u8 filter_method;
    u8 interlace_method;
};
#pragma pack(pop)

struct Png_Chunk_Footer {
    u32 crc; 
};

// ZLIB header notes:
// Bytes[0]:
// - compression flags bit 0-3: Compression Method (CM)
// - compression flags bit 4-7: Compression Info (CINFO)
// Bytes[1]:
// - additional flags bit 0-4: FCHECK 
// - additional flags bit 5: Preset dictionary (FDICT)
// - additional flags bit 6-7: Compression level (FLEVEL)
struct Png_IDAT_Header {
    u8 compression_flags;
    u8 additional_flags;
};

static inline void
free_png(Png_Image png) {
    free(png.data);
}

struct Png_Huffman {
    Array<u16> code_sym_table; // Canonical ordered symbols
    Array<u16> len_count_table;
};

static inline Png_Huffman
create_huffman(Memory_Arena* arena, 
               Array<u16> sym_len_table, 
               usize len_count_table_cap,
               usize code_sym_table_cap) 
{
    Png_Huffman ret = {};
    ret.code_sym_table = array<u16>(arena, code_sym_table_cap);
    ret.len_count_table = array<u16>(arena, len_count_table_cap);

    // 1. Count the number of codes for each code length
    for (usize sym = 0; 
         sym < sym_len_table.count;
         ++sym) 
    {
        u16 Len = sym_len_table[sym];
        Assert(Len < PngMaxBits);
        ++ret.len_count_table[Len];
    }

    // 2. Numerical value of smallest code for each code length
    u16 len_offset_table[PngMaxBits+1] = {};
    for (usize len = 1; len < PngMaxBits; ++len)
    {
        len_offset_table[len+1] = len_offset_table[len] + ret.len_count_table[len]; 
    }


    // 3. Assign numerical values to all codes
    for (usize sym = 0;
         sym < sym_len_table.count;
         ++sym)
    {
        u16 len = sym_len_table[sym]; 
        if (len > 0) {
            u16 code = len_offset_table[len]++;
            ret.code_sym_table[code] = (u16)sym;
        }
    }

    return ret;
}

static inline u32
decode(Stream* src_stream, Png_Huffman* huffman) {
    i32 code = 0;
    i32 first = 0;
    i32 index = 0;

    for (i32 len = 1; len <= PngMaxBits; ++len) {
        code |= src_stream->read_bits(1);
        i32 count = huffman->len_count_table[len];
        if(code - count < first)
            return huffman->code_sym_table[index + (code - first)];
        index += count;
        first += count;
        first <<= 1;
        code <<= 1;
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
        BFINAL = (u8)ConsumeBits(&Context->Stream, 1);
        u16 BTYPE = (u8)ConsumeBits(&Context->Stream, 2);
        printf(">>> BFINAL: %d\n", BFINAL);
        printf(">>> BTYPE: %d\n", BTYPE);
        switch(BTYPE) {
            case 0b00: {
                printf(">>>> No compression\n");
                ConsumeBits(&Context->Stream, 5);
                u16 LEN = (u16)ConsumeBits(&Context->Stream, 16);
                u16 NLEN = (u16)ConsumeBits(&Context->Stream, 16);
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

                    BootstrapArray(LitLenTable, u16, PngMaxFixedLitCodes);
                    BootstrapArray(DistLenTable, u16, PngMaxDistCodes);
                    
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
                                         PngMaxBits+1,
                                         PngMaxFixedLitCodes);
                    DistHuffman = Huffman(Context->Arena,
                                          DistLenTable,
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
                        u32 Len = Lens[Sym] + ConsumeBits(&Context->Stream, LenExBits[Sym]);
                        Sym = Decode(Context, &DistHuffman);
                        if (Sym < 0) {
                            printf("Invalid Symbol 2\n");
                            return false;
                        }
                        u32 Dist = Dists[Sym] + ConsumeBits(&Context->Stream, DistExBits[Sym]);
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
deflate(Stream* src_stream, Stream* dest_stream, Memory_Arena* arena) 
{
    static const short lens[29] = { /* Size base for length codes 257..285 */
        3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31,
        35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258};
    static const short len_ex_bits[29] = { /* Extra bits for length codes 257..285 */
        0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2,
        3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0};
    static const short dists[30] = { /* Offset base for distance codes 0..29 */
        1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193,
        257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145,
        8193, 12289, 16385, 24577};
    static const short dist_ex_bits[30] = { /* Extra bits for distance codes 0..29 */
        0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6,
        7, 7, 8, 8, 9, 9, 10, 10, 11, 11,
        12, 12, 13, 13};
    u8 BFINAL = 0;
    while(BFINAL == 0){
        BFINAL = (u8)src_stream->read_bits(1);
        u16 BTYPE = (u8)src_stream->read_bits(2);
        printf(">>> BFINAL: %d\n", BFINAL);
        printf(">>> BTYPE: %d\n", BTYPE);
        switch(BTYPE) {
            case 0b00: {
                printf(">>>> No compression\n");
                src_stream->read_bits(5);
                u16 LEN = (u16)src_stream->read_bits(16);
                u16 NLEN = (u16)src_stream->read_bits(16);
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
                Png_Huffman lit_huffman = {};
                Png_Huffman dist_huffman = {};

                if (BTYPE == 0b01) {
                    // Fixed huffman
                    printf(">>>> Fixed huffman\n");

                    boot_array(lit_len_table, u16, PngMaxFixedLitCodes);
                    boot_array(dist_len_table, u16, PngMaxDistCodes);
                    
                    usize Lit = 0;
                    for (; Lit < 144; ++Lit) 
                        lit_len_table[Lit] = 8;
                    for (; Lit < 256; ++Lit) 
                        lit_len_table[Lit] = 9;
                    for (; Lit < 280; ++Lit) 
                        lit_len_table[Lit] = 7;
                    for (; Lit < PngMaxFixedLitCodes; ++Lit) 
                        lit_len_table[Lit] = 8;
                    for (Lit = 0; Lit < PngMaxDistCodes; ++Lit) 
                        dist_len_table[Lit] = 5;
                     
                    lit_huffman = create_huffman(arena, 
                                                 lit_len_table, 
                                                 PngMaxBits+1,
                                                 PngMaxFixedLitCodes);
                    dist_huffman = create_huffman(arena,
                                                  dist_len_table,
                                                  PngMaxBits+1,
                                                  PngMaxDistCodes);

                }
                else // BTYPE == 0b10
                {
                    // TODO: Dynamic huffman
                    printf(">>>> Dynamic huffman not supported\n");
                    return false;
                }

                u32 len_count_table[PngMaxBits + 1] = {};
                for (;;) 
                {
                    u32 sym = decode(src_stream, &lit_huffman);
                    if (sym <= 255) { 
                        u8 ByteToWrite = (u8)(sym & 0xFF); 
                        printf("%02X ", ByteToWrite);
                        dest_stream->write(ByteToWrite);
                        //Context->ImageData[Context->ImageDataCount++] = ByteToWrite;
                    }
                    else if (sym >= 257) {
                        sym -= 257;
                        if (sym >= 29) {
                            printf("Invalid Symbol 1\n"); 
                            return false;
                        }
                        u32 len = lens[sym] + src_stream->read_bits(len_ex_bits[sym]);
                        sym = decode(src_stream, &dist_huffman);
                        if (sym < 0) {
                            printf("Invalid Symbol 2\n");
                            return false;
                        }
                        u32 dist = dists[sym] + src_stream->read_bits(dist_ex_bits[sym]);
                        while(len--) {
                            usize target_index = dest_stream->current - dist;
                            u8 byte_to_write = dest_stream->contents[target_index];
                            printf("%02X ", byte_to_write);
                            dest_stream->write(byte_to_write);
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
parse_IDAT_chunk(Png_Context* context) {
    u32 CM, CINFO, FCHECK, FDICT, FLEVEL;
    CM = context->main_stream.read_bits(4);
    CINFO = context->main_stream.read_bits(4);
    FCHECK = context->main_stream.read_bits(5); //not needed?
    FDICT = context->main_stream.read_bits(1);
    FLEVEL = context->main_stream.read_bits(2); //useless?
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

    if (!context->is_image_initialized) {
        printf("Result W/H/C: %d, %d, %d\n", context->image_width, 
                context->image_height, context->image_channels);
        usize image_size = context->image_width * 
                          context->image_height * 
                          context->image_channels;

        context->img_stream_mark = BeginScratch(context->arena);
        context->img_stream = Stream::create(context->img_stream_mark, image_size);
    
        context->depressed_img_stream_mark = BeginScratch(context->arena);
        usize depressed_img_size = (context->image_width + 1) *
                                        context->image_height* 
                                        context->image_channels;
        context->depressed_img_stream = Stream::create(context->arena, depressed_img_size);

        context->is_image_initialized = true;
    }

    scratch scr = BeginScratch(context->arena);
    Defer { EndScratch(&scr); };
    // Deflate
    if (!deflate(&context->main_stream, 
                 &context->depressed_img_stream,
                 scr)) 
    {
        return false;
    }
    return true;
}

static inline maybe<Png_Image> 
parse_png(Memory_Arena* arena, 
          void* png_memory, 
          usize png_memory_size) 
{
    Png_Context context = {};
    context.main_stream = Stream::create(png_memory, png_memory_size); 
    context.arena = arena;

    // Read the signature
    auto* png_header = context.main_stream.read<Png_Header>();  
    if (!png_header) { return No(); };

    static constexpr u8 png_signature[] = { 
        137, 80, 78, 71, 13, 10, 26, 10 
    }; 
    for (u32 I = 0; I < ArrayCount(png_signature); ++I) {
        if (png_signature[I] != png_header->signature[I]) {
            printf("Png Singature wrong!\n");
            return No();
        }
    }

    // Check for IHDR which MUST appear first
    auto* chunk_header = context.main_stream.read<Png_Chunk_Header>();
    if (!chunk_header){ return No(); }
    if (chunk_header->type_u32 != FourCC("IHDR")) { return No(); }
    auto* IHDR = context.main_stream.read<Png_Chunk_Data_IHDR>();
    if (!IHDR) { return No(); }

    // Unsupported details
    if (IHDR->color_type != 6 &&
        IHDR->bit_depth != 8 &&
        IHDR->compression_method &&
        IHDR->filter_method != 0 &&
        IHDR->interlace_method != 0) 
    {
        return No();
    }
    EndianSwap(&IHDR->width);
    EndianSwap(&IHDR->height);

    context.image_width = IHDR->width;
    context.image_height = IHDR->height;
    context.image_channels = 4;
    context.main_stream.read<Png_Chunk_Footer>();

    // Search for IDAT header
    while(!context.main_stream.is_eos()) {
        chunk_header = context.main_stream.read<Png_Chunk_Header>();
        EndianSwap(&chunk_header->length);
        switch(chunk_header->type_u32) {
            case FourCC("IDAT"): {
                if (!parse_IDAT_chunk(&context)) {
                    return No();
                }
            } break;
            case FourCC("IEND"): {
                Png_Image ret = {};
                ret.width = context.image_width;
                ret.height = context.image_height;
                ret.channels = context.image_channels;
                return Yes(ret);
            } break;
            default: {
                context.main_stream.read(chunk_header->length);
            };
        }
        context.main_stream.read<Png_Chunk_Footer>();
    }
    return No();
}

int main() {    
    usize memory_size = Megabytes(1);
    void * memory = malloc(memory_size);
    if (!memory) { return 1; }
    Defer { free(memory); };  
    Memory_Arena app_arena = Arena(memory, memory_size);

    maybe<Read_File_Result> png_file_ = read_file("test2.png");
    if (!png_file_){
        return 1;
    }
    Read_File_Result& png_file = png_file_.This;
    Defer { free(png_file.memory); }; 

    parse_png(&app_arena, png_file.memory, png_file.memory_size);

    printf("Done!");
    return 0;
}

