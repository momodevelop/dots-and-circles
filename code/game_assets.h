#ifndef __GAME_ASSETS__
#define __GAME_ASSETS__

#include "ryoji.h"
#include "ryoji_maths.h"
#include "ryoji_arenas.h"
#include "ryoji_asset_types.h"

static constexpr u8 AssetSignature[] = { 'M', 'O', 'M', 'O' };

struct loaded_image {
    u32 Width;
    u32 Height;
    void* Pixels; 
};

struct loaded_spritesheet {
    u32 Width;
    u32 Height;
    void* Pixels; 
    
    rect2f* Frames;
    u32 Rows;
    u32 Cols;
    
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
    // TODO(Momo): States for loaded/unloaded etc?
    // TODO(Momo): Change unions to pointer?
    asset_id Id;
    asset_type Type;
    union {
        loaded_image Image;
        loaded_spritesheet Spritesheet;
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
SetFrame(loaded_spritesheet* Spritesheet, u32 Row, u32 Col, rect2f Frame) {
    u32 FrameIndex = TwoToOne(Row, Col, Spritesheet->Cols);
    Assert(FrameIndex < (Spritesheet->Rows * Spritesheet->Cols));
    Spritesheet->Frames[FrameIndex] = Frame;
}

static inline void
Init(game_assets* Assets, memory_arena* Arena) {
    SubArena(&Assets->Arena, Arena, Megabytes(10));
}

static inline loaded_image* 
GetImage(game_assets* Assets, asset_id Id) {
    Assert(Id > asset_id::Invalid && Id < asset_id::Max);
    Assert(Assets->Entries[(u32)Id].Type == asset_type::Image);
    return &Assets->Entries[(u32)Id].Image;
}

static inline loaded_spritesheet* 
GetSpritesheet(game_assets* Assets, asset_id Id) {
    Assert(Id > asset_id::Invalid && Id < asset_id::Max);
    Assert(Assets->Entries[(u32)Id].Type == asset_type::Spritesheet);
    return &Assets->Entries[(u32)Id].Spritesheet;
}

static inline asset_entry*
LoadAsset(game_assets* Assets, asset_id Id, asset_type Type) {
    Assert(Id > asset_id::Invalid && Id < asset_id::Max);
    asset_entry* Asset = &Assets->Entries[(u32)Id];
    Asset->Id = Id;
    Asset->Type = Type;
    
    return Asset;
}

static inline loaded_image*
LoadImage(game_assets* Assets, 
          asset_id Id, 
          u32 Width, 
          u32 Height, 
          u32 Channels, 
          void* Data) 
{
    asset_entry* Entry = LoadAsset(Assets, Id, asset_type::Image);
    
    loaded_image Image = {};
    {
        Image.Width = Width;
        Image.Height = Height;
        
        usize Count = Width * Height * Channels;
        Image.Pixels = PushBlock(&Assets->Arena, Count, 1);
        Assert(Image.Pixels);
        CopyBlock(Image.Pixels, Data, Count);
    }
    Entry->Image = Image;
    return &Entry->Image;
    
}

static inline loaded_spritesheet*
LoadSpritesheet(game_assets* Assets, 
                asset_id Id, 
                u32 Width,
                u32 Height,
                u32 Channels,
                u32 Rows,
                u32 Cols,
                void* Data) 
{
    asset_entry* Entry = LoadAsset(Assets, Id, asset_type::Spritesheet);
    loaded_spritesheet Spritesheet = {};
    {
        Spritesheet.Width = Width;
        Spritesheet.Height = Height;
        Spritesheet.Rows = Rows;
        Spritesheet.Cols = Cols; 
        
        usize Count = Width * Height * Channels;
        Spritesheet.Pixels = PushBlock(&Assets->Arena, Count, 1);
        Assert(Spritesheet.Pixels);
        CopyBlock(Spritesheet.Pixels, Data, Width * Height * Channels);
        
        
        f32 FrameWidth = (f32)Width/Cols;
        f32 FrameHeight = (f32)Height/Rows;
        f32 CellWidth = FrameWidth/Width;
        f32 CellHeight = FrameHeight/Height;
        f32 HalfPixelWidth =  0.25f/Width;
        f32 HalfPixelHeight = 0.25f/Height;
        
        // NOTE(Momo):  Init sprite frames
        u32 TotalFrames = Rows * Cols;
        rect2f* Rects = PushArray<rect2f>(&Assets->Arena, TotalFrames); 
        for (u8 r = 0; r < Rows; ++r) {
            for (u8 c = 0; c < Cols; ++c) {
                Rects[TwoToOne(r,c,Cols)] = { 
                    c * CellWidth + HalfPixelWidth,
                    r * CellHeight + HalfPixelHeight,
                    (c+1) * CellWidth - HalfPixelWidth,
                    (r+1) * CellHeight - HalfPixelHeight,
                };
            }
        }
        Spritesheet.Frames = Rects;
        
    }
    
    
    Entry->Spritesheet = Spritesheet;
    
    return &Entry->Spritesheet;
}

#endif  