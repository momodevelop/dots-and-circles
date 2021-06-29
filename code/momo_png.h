/* date = May 16th 2021 8:30 pm */
#ifndef MOMO_PNG_H
#define MOMO_PNG_H


//~ TODO(Momo)
// Dynamic Huffman
// Different filter methods

#define PNG_DEBUG 0
#define PNG_MAX_BITS 15
#define PNG_MAX_DIST_CODES 32
#define PNG_MAX_FIXED_LIT_CODES 288

#if PNG_DEBUG
#include <stdio.h>
#define PNG_LOG(...) printf(__VA_ARGS__)
#else
#define PNG_LOG
#endif

struct Png_Context {
    Stream stream;
    Arena* arena; 
    
    b8 is_image_initialized;
    Stream image_stream;
    Stream depressed_image_stream;
    
    u32 image_width;
    u32 image_height;
    u32 image_channels;
    
    Arena_Mark image_streamMark;
    Arena_Mark depressed_image_streamMark;
};


struct Png_Header {
    u8 signature[8];
};

// 5.3 Chunk layout
// | length | type | data | CRC
struct Png_Chunk_Header {
    u32 length;
    union {
        u32 type_u32;
        u8 type[4];
    };
};


#pragma pack(push, 1)
struct Png_Chunk_data_IHDR {
    u32 width;
    u32 height;
    u8 bit_depth;
    u8 colour_type;
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
    u8 CompressionFlags;
    u8 AdditionalFlags;
};

struct Png_Image {
    u32 width;
    u32 height; 
    u32 channels;
    void* data;
};


struct Png_Huffman {
    u16* code_sym_table; // Canonical ordered symbols
    u32 code_sym_table_size;
    
    u16* len_count_table;
    u32 len_count_table_size;
};

enum Png_Error {
    PngError_None, // 0 
    PngError_CannotReadHeader,
    PngError_Badsignature,
    PngError_CannotReadchunk_header,
    PngError_FirstHeaderIsNotIHDR,
    PngError_CannotReadIHDR, // 5
    PngError_UnsupportedFormat,
    PngError_BadFormat,
    PngError_NLENvsLENMismatch,
    PngError_BadExtraCode,
    PngError_Badsymbol,
    PngError_BadBTYPE,
    PngError_UnsupportedIDATFormat,
    PngError_CannotReadfilter_type,
    PngError_NotEnoughPixels,
    PngError_Badfilter_type,
    PngError_DynamicHuffmanNotSupported,
};

static inline s32
png_huffman_decode(Bitstream* src_stream, Png_Huffman* huffman) {
    s32 code = 0;
    s32 first = 0;
    s32 index = 0;
    
    for (s32 Len = 1; Len <= PNG_MAX_BITS; ++Len) {
        code |= src_stream->consume_bits(1);
        s32 count = huffman->len_count_table[Len];
        if(code - count < first)
            return huffman->code_sym_table[index + (code - first)];
        index += count;
        first += count;
        first <<= 1;
        code <<= 1;
    }
    
    return -1;
}

static inline Png_Huffman
png_create_huffman(Arena* arena, 
                   u16* sym_len_table,
                   u32 sym_len_table_size, 
                   u32 len_count_table_cap,
                   u32 code_sym_table_cap) 
{
    Png_Huffman ret = {};
    
    ret.code_sym_table_size = code_sym_table_cap;
    ret.code_sym_table = arena->push_array<u16>(code_sym_table_cap);
    
    ret.len_count_table_size = len_count_table_cap;
    ret.len_count_table = arena->push_array<u16>(len_count_table_cap);
    
    // 1. Count the number of codes for each code length
    for (u32 sym = 0; sym < sym_len_table_size; ++sym) 
    {
        u16 len = sym_len_table[sym];
        ASSERT(len < PNG_MAX_BITS);
        ++ret.len_count_table[len];
    }
    
    // 2. Numerical value of smallest code for each code length
    u16 len_offset_table[PNG_MAX_BITS+1] = {};
    for (u32 len = 1; len < PNG_MAX_BITS; ++len)
    {
        len_offset_table[len+1] = len_offset_table[len] + ret.len_count_table[len]; 
    }
    
    
    // 3. Assign numerical values to all codes
    for (u32 sym = 0; sym < sym_len_table_size; ++sym)
    {
        u16 len = sym_len_table[sym];
        if (len > 0) {
            u16 code = len_offset_table[len]++;
            ret.code_sym_table[code] = (u16)sym;
        }
    }
    
    return ret;
}


static inline Png_Error
png_deflate(Bitstream* src_stream, Stream* dest_stream, Arena* arena) 
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
        12, 12, 13, 13 };
    
    u8 BFINAL = 0;
    while(BFINAL == 0){
        Arena_Mark Scratch = arena->mark();
        defer { Scratch.revert(); };
        
        BFINAL = (u8)src_stream->consume_bits(1);
        u16 BTYPE = (u8)src_stream->consume_bits(2);
        PNG_LOG(">>> BFINAL: %d\n", BFINAL);
        PNG_LOG(">>> BTYPE: %d\n", BTYPE);
        switch(BTYPE) {
            case 0b00: {
                PNG_LOG(">>>> No compression\n");
                src_stream->consume_bits(5);
                u16 LEN = (u16)src_stream->consume_bits(16);
                u16 NLEN = (u16)src_stream->consume_bits(16);
                PNG_LOG(">>>>> LEN: %d\n", LEN);
                PNG_LOG(">>>>> NLEN: %d\n", NLEN);
                if ((u16)LEN != ~((u16)(NLEN))) {
                    PNG_LOG("LEN vs NLEN mismatch!\n");
                    return PngError_NLENvsLENMismatch;
                }
                // TODO: complete this
            } break;
            case 0b01: 
            case 0b10: {
                Png_Huffman lit_huffman = {};
                Png_Huffman dist_huffman = {};
                
                if (BTYPE == 0b01) {
                    // Fixed huffman
                    PNG_LOG(">>>> Fixed huffman\n");
                    
                    u16 lit_len_table[PNG_MAX_FIXED_LIT_CODES] = {};
                    u16 dist_len_table[PNG_MAX_DIST_CODES] = {};
                    
                    u32 Lit = 0;
                    for (; Lit < 144; ++Lit) 
                        lit_len_table[Lit] = 8;
                    for (; Lit < 256; ++Lit) 
                        lit_len_table[Lit] = 9;
                    for (; Lit < 280; ++Lit) 
                        lit_len_table[Lit] = 7;
                    for (; Lit < PNG_MAX_FIXED_LIT_CODES; ++Lit) 
                        lit_len_table[Lit] = 8;
                    for (Lit = 0; Lit < PNG_MAX_DIST_CODES; ++Lit) 
                        dist_len_table[Lit] = 5;
                    
                    lit_huffman = png_create_huffman(arena, 
                                                     lit_len_table, 
                                                     PNG_MAX_FIXED_LIT_CODES,
                                                     PNG_MAX_BITS+1,
                                                     PNG_MAX_FIXED_LIT_CODES);
                    dist_huffman = png_create_huffman(arena,
                                                      dist_len_table,
                                                      PNG_MAX_DIST_CODES,
                                                      PNG_MAX_BITS+1,
                                                      PNG_MAX_DIST_CODES);
                    
                }
                else // BTYPE == 0b10
                {
                    // TODO: Dynamic huffman
                    PNG_LOG(">>>> Dynamic huffman not supported\n");
                    return PngError_DynamicHuffmanNotSupported;
                }
                
                u32 len_count_table[PNG_MAX_BITS + 1] = {};
                for (;;) 
                {
                    s32 sym = png_huffman_decode(src_stream, &lit_huffman);
                    // NOTE(Momo): Normal case
                    if (sym <= 255) { 
                        u8 byte_to_write = (u8)(sym & 0xFF); 
                        dest_stream->write_struct<u8>(byte_to_write);
                    }
                    // NOTE(Momo): Extra code case
                    else if (sym >= 257) {
                        sym -= 257;
                        if (sym >= 29) {
                            return PngError_BadExtraCode;
                        }
                        u32 Len = lens[sym] + src_stream->consume_bits(len_ex_bits[sym]);
                        sym = png_huffman_decode(src_stream, &dist_huffman);
                        if (sym < 0) {
                            return PngError_Badsymbol;
                        }
                        u32 Dist = dists[sym] + src_stream->consume_bits(dist_ex_bits[sym]);
                        while(Len--) {
                            u32 target_index = dest_stream->current - Dist;
                            u8 byte_to_write = dest_stream->contents[target_index];
                            dest_stream->write_struct<u8>(byte_to_write);
                        }
                    }
                    else { 
                        // sym == 256
                        break;
                    }
                }
                PNG_LOG("\n");
            } break;
            default: {
                PNG_LOG("Error\n");
                return PngError_BadBTYPE;
            }
        }
    }
    return PngError_None;
}


static inline Png_Error
load_png_from_memory(Png_Image* png,
                     Arena* arena,
                     void* png_memory,
                     u32 png_memory_size) 
{
    Bitstream stream = {};
    stream.init(png_memory, png_memory_size); 
    
    auto* png_header = stream.consume_struct<Png_Header>();  
    if (!png_header) { 
        return PngError_CannotReadHeader; 
    }
    static constexpr u8 png_signature[] = { 
        137, 80, 78, 71, 13, 10, 26, 10 
    };
    for (u32 I = 0; I < ARRAY_COUNT(png_signature); ++I) {
        if (png_signature[I] != png_header->signature[I]) {
            PNG_LOG("Png Singature wrong!\n");
            return PngError_Badsignature;
        }
    }
    
    auto* chunk_header = stream.consume_struct<Png_Chunk_Header>();
    if (!chunk_header) { 
        return PngError_CannotReadchunk_header; 
    }
    
    // NOTE(Momo): IHDR must appear first
    if (chunk_header->type_u32 != four_cc("IHDR")) { 
        return PngError_FirstHeaderIsNotIHDR; 
    }
    auto* IHDR = stream.consume_struct<Png_Chunk_data_IHDR>();
    if (!IHDR) { 
        return PngError_CannotReadIHDR; 
    }
    
    
    
    
    if ((IHDR->colour_type != 6 || IHDR->colour_type == 2) &&
        IHDR->bit_depth != 8 &&
        IHDR->compression_method &&
        IHDR->filter_method != 0 &&
        IHDR->interlace_method != 0) 
    {
        // TODO: Expand this to different errors
        return PngError_UnsupportedFormat;
    }
    
    // colour_type 1 = Pallete used
    // colour_type 2 = Colour used 
    // colour_type 4 = Alpha used
    u32 image_channels = 0;
    switch(IHDR->colour_type){
        case 2: {
            image_channels = 3; // RGB
        } break;
        case 6: { 
            image_channels = 4; // RGBA
        } break;
        default: {
            return PngError_UnsupportedFormat;
        }
    }
    endian_swap(&IHDR->width);
    endian_swap(&IHDR->height);
    
    // Just consume the footer. 
    // TODO: CRC check with footer
    stream.consume_struct<Png_Chunk_Footer>();
    
    u32 image_size = IHDR->width * IHDR->height * image_channels;
    
    // NOTE(Momo): For reserving memory for unfiltered data that is generated 
    // as we decode the file
    Arena_Mark actual_image_stream_mark = arena->mark();
    Stream actual_image_stream = {};
    actual_image_stream.alloc(arena, image_size);
    
    // Search for IDAT header
    while(!stream.is_eos()) {
        chunk_header = stream.consume_struct<Png_Chunk_Header>();
        endian_swap(&chunk_header->length);
        switch(chunk_header->type_u32) {
            case four_cc("IDAT"): {
                // temporary stream just to process IDAT
                Bitstream idat_stream = stream; 
                
                u32 CM = idat_stream.consume_bits(4);
                u32 CINFO = idat_stream.consume_bits(4);
                u32 FCHECK = idat_stream.consume_bits(5); //not needed?
                u32 FDICT = idat_stream.consume_bits(1);
                u32 FLEVEL = idat_stream.consume_bits(2); //useless?
                
                PNG_LOG(">> CM: %d\n>> CINFO: %d\n>> FCHECK: %d\n>> FDICT: %d\n>>FLEVEL: %d\n",
                        CM, 
                        CINFO,
                        FCHECK, 
                        FDICT, 
                        FLEVEL); 
                if (CM != 8 || FDICT != 0 || CINFO > 7) {
                    return PngError_UnsupportedIDATFormat;
                }
                
                // NOTE(Momo): Allow space for unfiltered image
                u32 unfiltered_image_size = IHDR->width * (image_channels + 1);
                Arena_Mark unfiltered_image_stream_mark = arena->mark();
                
                Stream unfiltered_image_stream = {};
                unfiltered_image_stream.init(arena, unfiltered_image_size);
                
                Png_Error deflate_error = png_deflate(&idat_stream, &unfiltered_image_stream, arena);
                if (deflate_error != PngError_None) {
                    actual_image_stream_mark.revert();
                    return deflate_error;
                }
                
                // NOTE(Momo): Filter
                // data always starts with 1 byte indicating the type of filter
                // followed by the rest of the chunk.
                unfiltered_image_stream.reset();
                while(!unfiltered_image_stream.is_eos()) {
                    u8* filter_type = unfiltered_image_stream.consume_struct<u8>();
                    if (filter_type == nullptr) {
                        actual_image_stream_mark.revert();
                        return PngError_CannotReadfilter_type;
                    }
                    PNG_LOG("%02X: ", (u32)(*filter_type));
                    switch(*filter_type) {
                        case 0: { // None
                            for (u32 I = 0; I < IHDR->width; ++I ){
                                for (u32 J = 0; J < image_channels; ++J) {
                                    u8* pixel_byte = unfiltered_image_stream.consume_struct<u8>();
                                    if (pixel_byte == nullptr) {
                                        actual_image_stream_mark.revert();
                                        return PngError_NotEnoughPixels;
                                    }
                                    PNG_LOG("%02X ", (u32)(*pixel_byte));
                                    actual_image_stream.write_struct<u8>(*pixel_byte);
                                    
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
                            return PngError_Badfilter_type;
                        };
                    };
                    PNG_LOG("\n");
                    unfiltered_image_stream_mark.revert();
                }
                
                stream.consume_block(chunk_header->length);
                
            } break;
            case four_cc("IEND"): {
                png->width = IHDR->width;
                png->height = IHDR->height;
                png->channels = image_channels;
                png->data = actual_image_stream.contents;
                return PngError_None;
            } break;
            default: {
                // NOTE(Momo): For now, we don't care about the rest of the chunks
                stream.consume_block(chunk_header->length);
                
            };
        }
        stream.consume_struct<Png_Chunk_Footer>();
    }
    
    return PngError_BadFormat;
    
    
    
    
}

#endif //MOMO_PNG_H
