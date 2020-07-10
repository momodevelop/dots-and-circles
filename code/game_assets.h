#ifndef __BMP5_32BPP_H__
#define __BMP5_32BPP_H__

#include "ryoji.h"
#include "ryoji_colors.h"

// TODO(Momo): Remove stdlib and stdio 
#include <stdlib.h>
#include <stdio.h>

#define MapTo16(buffer, index) (buffer[index] | (buffer[index+1] << 8))
#define MapTo32(buffer, index) (buffer[index] | (buffer[index+1] << 8) |  (buffer[index+2] << 16) | (buffer[index+3] << 24)) 

// TODO(Momo): Shift this to ryoji_color.h?
struct color_rgba {
    u8 Red, Green, Blue, Alpha;
};


struct loaded_bitmap {
    u32 Width;
    u32 Height;
    
    // NOTE(Momo): Array of pixels. Do we want to store size?
    color_rgba* Pixels; 
    
};

struct game_texture {
    loaded_bitmap Bitmap;
    u32 Handle;
};

struct game_assets {
    game_texture Textures[2];
};


struct debug_read_file_result {
    void * Content;
    i32 ContentSize;
};

static inline debug_read_file_result
DebugReadFileToMemory(const char * path) {
    FILE* file = fopen(path, "rb");
    if (file == nullptr) {
        return {};
    }
    debug_read_file_result Ret = {};
    Defer{
        fclose(file);
    };
    
    // Get file size
    fseek(file, 0, SEEK_END);
    Ret.ContentSize = ftell(file); 
    fseek(file, 0, SEEK_SET);
    
    Ret.Content = malloc(Ret.ContentSize);
    
    fread(Ret.Content, 1, Ret.ContentSize, file);
    
    return Ret;
}

// TODO(Momo): Replace malloc with arena. 
static inline loaded_bitmap 
DebugMakeEmptyBitmap(u32 Width, u32 Height) {
    loaded_bitmap Ret = {};
    Ret.Width = Width;
    Ret.Height = Height;
    
    usize MemorySize = Width * Height * sizeof(color_rgba);
    Ret.Pixels = (color_rgba*)malloc(MemorySize);
    ZeroBlock(Ret.Pixels, MemorySize);
    return Ret;
}


// NOTE(Momo): Only loads 32-bit BMP files
// TODO(Momo): Replace with asset loading system?
// TODO(Momo): Replace malloc with arena
static inline loaded_bitmap
DebugMakeBitmapFromBmp(const char* filepath) {
    constexpr u8 kFileHeaderSize = 14;
    constexpr u8 kInfoHeaderSize = 124;
    constexpr u8 kCompression = 3;
    constexpr u8 kBitsPerPixel = 32;
    constexpr u16 kSignature = 0x4D42;
    
    auto ReadFileResult  = DebugReadFileToMemory(filepath);
    Defer { free(ReadFileResult.Content); };
    
    const u8* const ReadFileContent = (const u8*)ReadFileResult.Content;
    
    Assert(MapTo16(ReadFileContent, 0) == kSignature);
    Assert(MapTo16(ReadFileContent, kFileHeaderSize + 14) == kBitsPerPixel);
    Assert(MapTo32(ReadFileContent, kFileHeaderSize + 16) == kCompression);
    
    
    u32 Width = MapTo32(ReadFileContent,  kFileHeaderSize + 4);
    u32 Height = MapTo32(ReadFileContent,  kFileHeaderSize + 8);
    loaded_bitmap Ret = DebugMakeEmptyBitmap(Width, Height);
    
    usize MemorySize = Ret.Width * Ret.Height * sizeof(color_rgba);
    Ret.Pixels = (color_rgba*)malloc(MemorySize);
    if (Ret.Pixels == nullptr) {
        return {};
    }
    
    u32 Offset = MapTo32(ReadFileContent, 10);
    u32 RedMask = MapTo32(ReadFileContent, kFileHeaderSize + 40);
    u32 GreenMask = MapTo32(ReadFileContent, kFileHeaderSize + 44);
    u32 BlueMask = MapTo32(ReadFileContent, kFileHeaderSize + 48);
    u32 AlphaMask = MapTo32(ReadFileContent, kFileHeaderSize + 52);
    
    for (u32 i = 0; i < Ret.Width * Ret.Height; ++i) {
        const u8* PixelLocation = ReadFileContent + Offset + i * sizeof(color_rgba);
        u32 Pixel = *(u32*)(PixelLocation);
        
        // TODO(Momo): Fill in pixels as if it's an array
        u32 mask = RedMask;
        if (mask > 0) {
            u32 color = Pixel & mask;
            while( color != 0 && (mask & 1) == 0 ) mask >>= 1, color >>= 1;
            Ret.Pixels[i].Red = (u8)color;
        }
        
        mask = GreenMask;
        if (mask > 0) {
            u32 color = Pixel & mask;
            while( color != 0 && (mask & 1) == 0 ) mask >>= 1, color >>= 1;
            Ret.Pixels[i].Green = (u8)color;
        }
        
        mask = BlueMask;
        if (mask > 0) {
            u32 color = Pixel & mask;
            while( color != 0 && (mask & 1) == 0 ) mask >>= 1, color >>= 1;
            Ret.Pixels[i].Blue = (u8)color;
        }
        
        mask = AlphaMask;
        if(mask > 0) {
            u32 color = Pixel & mask;
            while( color != 0 && (mask & 1) == 0 ) mask >>= 1, color >>= 1;
            Ret.Pixels[i].Alpha = (u8)color;
        }
        
    }
    
    
    return Ret;
}



#undef MapTo16
#undef MapTo32

#endif // __BMP32_H__