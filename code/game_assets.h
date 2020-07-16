#ifndef __GAME_ASSETS__
#define __GAME_ASSETS__

#include "ryoji.h"
#include "ryoji_colors.h"
#include "ryoji_arenas.h"

// TODO(Momo): Remove stdlib and stdio 
#include <stdlib.h>
#include <stdio.h>

struct bitmap {
    u32 Width;
    u32 Height;
    
    // NOTE(Momo): Array of pixels. Do we want to store size?
    color_rgba* Pixels; 
    
};

enum game_texture_type {
    GameTextureType_blank,
    GameTextureType_ryoji,
    GameTextureType_yuu,
    GameTextureType_max,
};

struct game_texture {
    bitmap Bitmap;
    game_texture_type Handle;
};



struct game_assets {
    game_texture Textures[1024];
    u32 TextureCount;
};


#define MapTo16(buffer, index) (buffer[index] | (buffer[index+1] << 8))
#define MapTo32(buffer, index) (buffer[index] | (buffer[index+1] << 8) |  (buffer[index+2] << 16) | (buffer[index+3] << 24)) 


#include <stdlib.h>
// TODO(Momo): Replace malloc with arena? Or platform memory code 
static inline bitmap 
DebugMakeEmptyBitmap(u32 Width, u32 Height) {
    bitmap Ret = {};
    Ret.Width = Width;
    Ret.Height = Height;
    
    usize Size = Width * Height * sizeof(color_rgba);
    //Ret.Pixels = New(color_rgba, Arena);
    Ret.Pixels = (color_rgba*)malloc(Size);
    
    ZeroBlock(Ret.Pixels, Size);
    return Ret;
}


// F

// NOTE(Momo): Only loads 32-bit BMP files
// TODO(Momo): Replace with asset loading system?
// TODO(Momo): Replace malloc with arena
static inline bitmap
DebugMakeBitmapFromBmp(void* BmpMemory) {
    constexpr u8 kFileHeaderSize = 14;
    constexpr u8 kInfoHeaderSize = 124;
    constexpr u8 kCompression = 3;
    constexpr u8 kBitsPerPixel = 32;
    constexpr u16 kSignature = 0x4D42;
    
    const u8* const Memory = (const u8*)BmpMemory;
    
    Assert(MapTo16(Memory, 0) == kSignature);
    Assert(MapTo16(Memory, kFileHeaderSize + 14) == kBitsPerPixel);
    Assert(MapTo32(Memory, kFileHeaderSize + 16) == kCompression);
    
    
    u32 Width = MapTo32(Memory,  kFileHeaderSize + 4);
    u32 Height = MapTo32(Memory,  kFileHeaderSize + 8);
    bitmap Ret = DebugMakeEmptyBitmap(Width, Height);
    
    
    u32 Offset = MapTo32(Memory, 10);
    u32 RedMask = MapTo32(Memory, kFileHeaderSize + 40);
    u32 GreenMask = MapTo32(Memory, kFileHeaderSize + 44);
    u32 BlueMask = MapTo32(Memory, kFileHeaderSize + 48);
    u32 AlphaMask = MapTo32(Memory, kFileHeaderSize + 52);
    
    for (u32 i = 0; i < Ret.Width * Ret.Height; ++i) {
        const u8* PixelLocation = Memory + Offset + i * sizeof(color_rgba);
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


static inline void
LoadTexture(game_assets *Assets, game_texture_type Type, void* BitmapMemory) {
    Assert(Type < GameTextureType_max);
    
    // TODO(Momo): If type is already used, free the bitmap?
    Assets->Textures[Type].Bitmap = DebugMakeBitmapFromBmp(BitmapMemory);
    Assets->Textures[Type].Handle = Type;
}


#endif // __BMP32_H__