#ifndef __GAME_ASSETS__
#define __GAME_ASSETS__

#include "ryoji.h"
#include "ryoji_arenas.h"
#include "ryoji_asset_types.h"

// TODO(Momo): Kind of want to treat all these different asset types as the same thing...?


// NOTE(Momo): Bitmaps
enum game_bitmap_handle : u32 {
    GameBitmapHandle_Blank,
    GameBitmapHandle_Ryoji,
    GameBitmapHandle_Yuu,
    GameBitmapHandle_Karu,
    GameBitmapHandle_Max,
};

// NOTE(Momo): Spritesheets
enum game_spritesheet_handle {
    GameSpritesheetHandle_Karu,
    GameSpritesheetHandle_Max,
};


struct game_spritesheet {
    game_bitmap_handle BitmapHandle;
    rect2f* Frames;
    u32 FramesCapacity; 
};




static inline void
SetFrame(game_spritesheet* Spritesheet, u32 FrameIndex, rect2f Frame) {
    Assert(FrameIndex < Spritesheet->FramesCapacity);
    Spritesheet->Frames[FrameIndex] = Frame;
}



// NOTE(Momo): Assets
struct game_assets {
    memory_arena Arena;
    bitmap Bitmaps[GameBitmapHandle_Max];
    game_spritesheet Spritesheets[GameSpritesheetHandle_Max];
};


static inline void
Init(game_assets* Assets, memory_arena* Arena, usize Capacity) {
    SubArena(&Assets->Arena, Arena, Capacity);
}


static inline bitmap 
GetBitmap(game_assets* Assets, game_bitmap_handle BitmapHandle) {
    Assert(BitmapHandle >= 0 && BitmapHandle < GameBitmapHandle_Max);
    return Assets->Bitmaps[BitmapHandle];
}

static inline game_spritesheet 
GetSpritesheet(game_assets* Assets, game_spritesheet_handle SpritesheetHandle) {
    Assert(SpritesheetHandle >= 0 && SpritesheetHandle < GameSpritesheetHandle_Max);
    return Assets->Spritesheets[SpritesheetHandle];
}

static inline bitmap
GetBitmapFromSpritesheet(game_assets* Assets, game_spritesheet_handle SpritesheetHandle) {
    return GetBitmap(Assets, GetSpritesheet(Assets, SpritesheetHandle).BitmapHandle);
}

static inline u32 
Read32(const u8* P) {
#if BIG_ENDIAN
    return P[0] << 24 | (P[1] << 18) |  (P[2] << 8) | (P[3]);
#else
    return P[0] | (P[1] << 8) |  (P[2] << 16) | (P[3] << 24);
#endif
}


static inline u16
Read16(const u8* P) {
#if BIG_ENDIAN
    return  (P[0] << 8) | P[1];
#else
    return  (P[0]) | (P[1] << 8);
#endif
}


struct color_rgba {
    u8 Red, Green, Blue, Alpha;
};

static inline game_spritesheet
MakeSpritesheet(game_bitmap_handle BitmapHandle, 
                rect2f* Frames, 
                u32 FramesCapacity,
                memory_arena* Arena) 
{
    Assert(FramesCapacity > 0);
    
    game_spritesheet Ret;
    
    Ret.BitmapHandle = BitmapHandle;
    Ret.Frames = PushArray<rect2f>(Arena, FramesCapacity);
    Ret.FramesCapacity = FramesCapacity;
    for (u32 i = 0; i < FramesCapacity; ++i) {
        Ret.Frames[i] = Frames[i]; 
    }
    
    
    return Ret;
}


static inline bitmap 
MakeEmptyBitmap(u32 Width, u32 Height, memory_arena* Arena) {
    bitmap Ret = {};
    Ret.Width = Width;
    Ret.Height = Height;
    
    usize Count = Width * Height * sizeof(color_rgba);
    Ret.Pixels = PushBlock(Arena, Count, alignof(color_rgba));
    Assert(Ret.Pixels);
    ZeroBlock(Ret.Pixels, Count);
    return Ret;
}

// NOTE(Momo): Only loads 32-bit BMP files
static inline bitmap
MakeBitmapFromBmp(void* BmpMemory, memory_arena* Arena) {
    constexpr u8 kFileHeaderSize = 14;
    constexpr u8 kInfoHeaderSize = 124;
    constexpr u8 kCompression = 3;
    constexpr u8 kBitsPerPixel = 32;
    constexpr u16 kSignature = 0x4D42;
    
    const u8* const Memory = (const u8*)BmpMemory;
    
    Assert(Read16(Memory +  0) == kSignature);
    Assert(Read16(Memory + kFileHeaderSize + 14) == kBitsPerPixel);
    Assert(Read32(Memory + kFileHeaderSize + 16) == kCompression);
    
    u32 Width = Read32(Memory + kFileHeaderSize + 4);
    u32 Height = Read32(Memory + kFileHeaderSize + 8);
    bitmap Ret = MakeEmptyBitmap(Width, Height, Arena);
    
    u32 Offset = Read32(Memory +  10);
    u32 RedMask = Read32(Memory + kFileHeaderSize + 40);
    u32 GreenMask = Read32(Memory +  kFileHeaderSize + 44);
    u32 BlueMask = Read32(Memory +  kFileHeaderSize + 48);
    u32 AlphaMask = Read32(Memory + kFileHeaderSize + 52);
    
    // NOTE(Momo): Treat Pixels as color_rgba
    color_rgba* RetPixels = (color_rgba*)Ret.Pixels;
    for (u32 i = 0; i < Ret.Width * Ret.Height; ++i) {
        const u8* PixelLocation = Memory + Offset + i * sizeof(color_rgba);
        u32 PixelData = *(u32*)(PixelLocation);
        u32 mask = RedMask;
        if (mask > 0) {
            u32 color = PixelData & mask;
            while( color != 0 && (mask & 1) == 0 ) mask >>= 1, color >>= 1;
            RetPixels[i].Red = (u8)color;
        }
        
        mask = GreenMask;
        if (mask > 0) {
            u32 color = PixelData & mask;
            while( color != 0 && (mask & 1) == 0 ) mask >>= 1, color >>= 1;
            RetPixels[i].Green = (u8)color;
        }
        
        mask = BlueMask;
        if (mask > 0) {
            u32 color = PixelData & mask;
            while( color != 0 && (mask & 1) == 0 ) mask >>= 1, color >>= 1;
            RetPixels[i].Blue = (u8)color;
        }
        
        mask = AlphaMask;
        if(mask > 0) {
            u32 color = PixelData & mask;
            while( color != 0 && (mask & 1) == 0 ) mask >>= 1, color >>= 1;
            RetPixels[i].Alpha = (u8)color;
        }
    }
    return Ret;
}

inline void
LoadBitmap(game_assets *Assets, 
           game_bitmap_handle BitmapHandle, 
           void* BitmapMemory) 
{
    Assert(BitmapHandle < GameBitmapHandle_Max);
    Assets->Bitmaps[BitmapHandle] = MakeBitmapFromBmp(BitmapMemory, &Assets->Arena);
}

// TODO(Momo): Load from file....????
inline void
LoadSpritesheet(game_assets* Assets, 
                game_spritesheet_handle SpritesheetHandle, 
                game_bitmap_handle TargetBitmapHandle,
                rect2f* Frames,
                u32 TotalFrames) 
{
    Assert(SpritesheetHandle < GameSpritesheetHandle_Max);
    Assets->Spritesheets[SpritesheetHandle] = MakeSpritesheet(TargetBitmapHandle, Frames, TotalFrames, &Assets->Arena);
}


#endif  