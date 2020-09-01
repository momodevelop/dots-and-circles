#ifndef __GAME_ASSETS__
#define __GAME_ASSETS__

#include "ryoji.h"
#include "ryoji_maths.h"
#include "ryoji_arenas.h"
#include "ryoji_asset_types.h"

static constexpr u8 AssetSignature[] = { 'M', 'O', 'M', 'O' };

struct asset_entry_image {
    u32 Width;
    u32 Height;
    void* Pixels; 
};

struct asset_entry_spritesheet {
    u32 Width;
    u32 Height;
    u32 FramesCapacity;
    void* Pixels; 
    rect2f* Frames;
};


enum struct asset_type : u32 {
    Image,
    Font,
    Spritesheet,
    Animation,
    Sound,
};

enum struct asset_id : u32 {
    Invalid, 
    
    // NOTE(Momo): Images
    Image_Ryoji,
    Image_Yuu,
    
    // NOTE(Momo): Spritesheets
    Spritesheet_Karu,
    
    Max,
};
using image_id = asset_id;
using spritesheet_id = asset_id;

struct asset_entry {
    // TODO(Momo): states for loaded/unloaded etc?
    asset_id Id;
    asset_type Type;
    union {
        asset_entry_image Image;
        asset_entry_spritesheet Spritesheet;
    };
};

struct game_assets {
    memory_arena Arena;
    asset_entry Entries[asset_id::Max];
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



static inline void
SetFrame(asset_entry_spritesheet* Spritesheet, u32 FrameIndex, rect2f Frame) {
    Assert(FrameIndex < Spritesheet->FramesCapacity);
    Spritesheet->Frames[FrameIndex] = Frame;
}

// NOTE(Momo): Assets
static inline void
Init(game_assets* Assets, memory_arena* Arena, usize Capacity) {
    SubArena(&Assets->Arena, Arena, Capacity);
}


static inline asset_entry_image* 
GetImage(game_assets* Assets, asset_id Id) {
    Assert(Id > asset_id::Invalid && Id < asset_id::Max);
    Assert(Assets->Entries[(u32)Id].Type == asset_type::Image);
    return &Assets->Entries[(u32)Id].Image;
}

static inline asset_entry_spritesheet* 
GetSpritesheet(game_assets* Assets, asset_id Id) {
    Assert(Id > asset_id::Invalid && Id < asset_id::Max);
    Assert(Assets->Entries[(u32)Id].Type == asset_type::Spritesheet);
    return &Assets->Entries[(u32)Id].Spritesheet;
}

static inline asset_entry_spritesheet
MakeSpritesheet(asset_entry_image Bitmap,
                rect2f* Frames, 
                u32 FramesCapacity,
                memory_arena* Arena) 
{
    Assert(FramesCapacity > 0);
    
    asset_entry_spritesheet Ret;
    
    Ret.Width = Bitmap.Width;
    Ret.Height = Bitmap.Height;
    Ret.Pixels = Bitmap.Pixels;
    Ret.Frames = PushArray<rect2f>(Arena, FramesCapacity);
    Ret.FramesCapacity = FramesCapacity;
    for (u32 i = 0; i < FramesCapacity; ++i) {
        Ret.Frames[i] = Frames[i]; 
    }
    
    return Ret;
}

// TODO(Momo): remove?
static inline asset_entry_image 
MakeEmptyBitmap(u32 Width, u32 Height, u32 Channels, memory_arena* Arena) {
    asset_entry_image Ret = {};
    Ret.Width = Width;
    Ret.Height = Height;
    
    usize Count = Width * Height * Channels;
    Ret.Pixels = PushBlock(Arena, Count, 1);
    Assert(Ret.Pixels);
    ZeroBlock(Ret.Pixels, Count);
    return Ret;
}

#endif  