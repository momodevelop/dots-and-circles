#ifndef __GAME_ASSETS__
#define __GAME_ASSETS__

#include "ryoji.h"
#include "ryoji_maths.h"
#include "ryoji_arenas.h"
#include "ryoji_asset_types.h"

static constexpr u8 AssetSignature[] = { 'M', 'O', 'M', 'O' };

enum struct asset_type : u32 {
    Image,
    Font,
    Spritesheet,
    Animation,
    Sound,
};

static inline b32
CheckAssetSignature(void *Memory) {
    u8* MemoryU8 = (u8*)Memory;
    for (u32 i = 0; i < ArrayCount(AssetSignature); ++i) {
        if (MemoryU8[i] != AssetSignature[i]) {
            return false;
        }
    }
    return true;
}

enum struct asset_type_id : u32 {
    None, 
    
    // NOTE(Momo): Images
    Image_Ryoji,
    Image_Yuu,
    
    // NOTE(Momo): Spritesheets
    Spritesheet_Karu,
    
    // TODO(Momo): Fonts
};


// NOTE(Momo): Bitmaps
// TODO(Momo): Consider using enum structs
enum game_bitmap_handle : u32 {
    GameBitmapHandle_Blank,
    GameBitmapHandle_Ryoji,
    GameBitmapHandle_Yuu,
    GameBitmapHandle_Karu,
    GameBitmapHandle_Max,
};

// NOTE(Momo): Spritesheets
enum game_spritesheet_handle : u32 {
    // TODO(Momo): HACK
    GameSpritesheetHandle_Karu = GameBitmapHandle_Max + 1,
    GameSpritesheetHandle_Max,
};

struct game_spritesheet {
    bitmap Bitmap;
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
    return GetSpritesheet(Assets, SpritesheetHandle).Bitmap;
}



// TODO(Momo): Think of a more general way that ignores endianness? 
static inline u32 
PeekU32(const u8* P, bool isBigEndian) {
    union {
        u32 V;
        u8 C[4];
    } Ret;
    
#if BIG_ENDIAN
    if (isBigEndian) {
        Ret.C[0] = P[0];
        Ret.C[1] = P[1];
        Ret.C[2] = P[2];
        Ret.C[3] = P[3];
    }
    else {
        Ret.C[0] = P[3];
        Ret.C[1] = P[2];
        Ret.C[2] = P[1];
        Ret.C[3] = P[0];
    }
#else
    if (isBigEndian) {
        Ret.C[0] = P[3];
        Ret.C[1] = P[2];
        Ret.C[2] = P[1];
        Ret.C[3] = P[0];
    }
    else {
        Ret.C[0] = P[0];
        Ret.C[1] = P[1];
        Ret.C[2] = P[2];
        Ret.C[3] = P[3];
    }
#endif
    return Ret.V;
    
}



static inline u16
PeekU16(const u8* P, bool isBigEndian) {
    union {
        u16 V;
        u8 C[2];
    } Ret;
    
    if (IsSystemBigEndian()) {
        if (isBigEndian) {
            Ret.C[0] = P[0];
            Ret.C[1] = P[1];
        }
        else {
            Ret.C[0] = P[1];
            Ret.C[1] = P[0];
        }
    }
    else {
        if (isBigEndian) {
            Ret.C[0] = P[1];
            Ret.C[1] = P[0];
        }
        else {
            Ret.C[0] = P[0];
            Ret.C[1] = P[1];
        }
    }
    return Ret.V;
}

static inline u32
ReadU32(u8** P, bool isBigEndian) {
    u32 Ret = PeekU32((*P), isBigEndian);
    (*P) += sizeof(u32);
    return Ret;
}

static inline u16
ReadU16(u8** P, bool isBigEndian) {
    u16 Ret = PeekU16((*P), isBigEndian);
    (*P) += sizeof(u16);
    return Ret;
}

struct color_rgba {
    u8 Red, Green, Blue, Alpha;
};

static inline game_spritesheet
MakeSpritesheet(bitmap Bitmap,
                rect2f* Frames, 
                u32 FramesCapacity,
                memory_arena* Arena) 
{
    Assert(FramesCapacity > 0);
    
    game_spritesheet Ret;
    
    Ret.Bitmap = Bitmap;
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

inline void
LoadBitmap(game_assets *Assets, 
           game_bitmap_handle BitmapHandle, 
           bitmap Bitmap) 
{
    Assert(BitmapHandle < GameBitmapHandle_Max);
    Assets->Bitmaps[BitmapHandle] = Bitmap;
}

inline void
LoadSpritesheet(game_assets* Assets, 
                game_spritesheet_handle SpritesheetHandle, 
                bitmap Bitmap,
                rect2f* Frames,
                u32 TotalFrames) 
{
    Assert(SpritesheetHandle < GameSpritesheetHandle_Max);
    Assets->Spritesheets[SpritesheetHandle] = MakeSpritesheet(Bitmap, Frames, TotalFrames, &Assets->Arena);
}


#endif  