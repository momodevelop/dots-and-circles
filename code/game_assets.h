#ifndef __GAME_ASSETS__
#define __GAME_ASSETS__

#include "ryoji.h"
#include "ryoji_maths.h"
#include "ryoji_arenas.h"
#include "ryoji_commands.h"

#include "game_asset_types.h"

struct asset_data_image {
    u32 Width;
    u32 Height;
    void* Pixels; 
};

struct asset_data_spritesheet {
    u32 Width;
    u32 Height;
    void* Pixels; 
    
    rect2f* Frames;
    u32 Rows;
    u32 Cols;
    
};

struct asset_entry {
    asset_id Id;
    asset_type Type;
    union {
        asset_data_image* Image;
        asset_data_spritesheet* Spritesheet;
    };
};

struct image_id {
    u32 Value;
}

struct spritesheet_id {
    u32 Value
}; 

struct game_assets {
    memory_arena Arena;
    
    asset_entry* Entries;
    u32 EntryCount;
    
    platform_api* Platform;
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


// NOTE(Momo): Load functions
static inline asset_entry*
LoadAsset(game_assets* Assets, asset_id Id, asset_type Type) {
    Assert(Id < asset_id::Max);
    asset_entry* Asset = &Assets->Entries[(u32)Id];
    Asset->Id = Id;
    Asset->Type = Type;
    
    return Asset;
}


static inline asset_data_image*
LoadImage(game_assets* Assets, asset_id Id, u8** Data) 
{
    asset_entry* Entry = LoadAsset(Assets, Id, asset_type::Image);
    
    u32 Width = Read32<u32>(Data, false);
    u32 Height = Read32<u32>(Data, false);
    u32 Channels = Read32<u32>(Data, false);
    
    Entry->Image = PushStruct<asset_data_image>(&Assets->Arena);
    Entry->Image->Width = Width;
    Entry->Image->Height = Height;
    
    // NOTE(Momo): Allocate pixel data
    usize Count = Width * Height * Channels;
    Entry->Image->Pixels = PushBlock(&Assets->Arena, Count, 1);
    Assert(Entry->Image->Pixels);
    CopyBlock(Entry->Image->Pixels, (*Data), Count);
    
    return Entry->Image;
}




static inline asset_data_spritesheet*
LoadSpritesheet(game_assets* Assets, asset_id Id, u8** Data) 
{
    asset_entry* Entry = LoadAsset(Assets, Id, asset_type::Spritesheet);
    
    u32 Width = Read32<u32>(Data, false);
    u32 Height = Read32<u32>(Data, false);
    u32 Channels = Read32<u32>(Data, false);
    u32 Rows = Read32<u32>(Data, false);
    u32 Cols = Read32<u32>(Data, false);
    
    
    Entry->Spritesheet = PushStruct<asset_data_spritesheet>(&Assets->Arena);
    Entry->Spritesheet->Width = Width;
    Entry->Spritesheet->Height = Height;
    Entry->Spritesheet->Rows = Rows;
    Entry->Spritesheet->Cols = Cols; 
    
    // NOTE(Momo): Allocate pixel data
    usize Size = Width * Height * Channels;
    Entry->Spritesheet->Pixels = PushBlock(&Assets->Arena, Size, 1);
    Assert(Entry->Spritesheet->Pixels);
    CopyBlock(Entry->Spritesheet->Pixels, (*Data), Size);
    
    f32 FrameWidth = (f32)Width/Cols;
    f32 FrameHeight = (f32)Height/Rows;
    f32 CellWidth = FrameWidth/Width;
    f32 CellHeight = FrameHeight/Height;
    f32 HalfPixelWidth =  0.25f/Width;
    f32 HalfPixelHeight = 0.25f/Height;
    
    // NOTE(Momo): Allocate sprite frames
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
    Entry->Spritesheet->Frames = Rects;
    
    return Entry->Spritesheet;
}


// TODO(Momo): Perhaps remove render_commands and replace with platform call for linking textures?
static inline void
Init(game_assets* Assets, 
     memory_arena* Arena, 
     platform_api* Platform,
     commands* RenderCommands,
     const char* Filename) 

{
    memory_arena* AssetArena = &Assets->Arena;
    SubArena(AssetArena, Arena, Megabytes(10));
    Assets->Platform = Platform;
    
    auto Scratch = BeginTempArena(AssetArena);
    Defer{ EndTempArena(Scratch); };
    
    // NOTE(Momo): File read into temp arena
    u8* FileMemory = nullptr;
    u8* FileMemoryItr = nullptr;
    {
        u32 Filesize = Platform->GetFileSize(Filename);
        Assert(Filesize);
        FileMemory = FileMemoryItr = (u8*)PushBlock(&Scratch, Filesize);
        Platform->ReadFile(FileMemory, Filesize, Filename);
    }
    
    // NOTE(Momo): Read and check file header
    {
        Assert(CheckAssetSignature(FileMemoryItr));
        FileMemoryItr+= ArrayCount(AssetSignature);
        Assets->EntryCount = Read32<u32>(&FileMemoryItr, false);
        Assert(Assets->EntryCount == (u32)asset_id::Max); 
    }
    
    // NOTE(Momo): Load Assets
    {
        // NOTE(Momo): All entries are allocated first. Their data will follow after.
        Assets->Entries = PushArray<asset_entry>(&Assets->Arena, Assets->EntryCount);
        for (u32 i = 0; i < Assets->EntryCount; ++i)
        {
            // NOTE(Momo): Read headers
            auto FileAssetType = Read32<asset_type>(&FileMemoryItr, false);
            u32 FileOffsetToEntry = Read32<u32>(&FileMemoryItr, false);
            auto FileAssetId = Read32<asset_id>(&FileMemoryItr, false);
            u8* FileEntryItr = FileMemory + FileOffsetToEntry;
            
            switch(FileAssetType) {
                case asset_type::Image: {
                    auto* Image = LoadImage(Assets, FileAssetId, &FileEntryItr);
                    PushCommandLinkTexture(RenderCommands, 
                                           Image->Width, 
                                           Image->Height,
                                           Image->Pixels,
                                           (u32)FileAssetId);
                } break;
                case asset_type::Font: {
                    // TODO(Momo): Implement
                } break;
                case asset_type::Spritesheet: {
                    auto* Spritesheet = LoadSpritesheet(Assets, FileAssetId, &FileEntryItr);
                    PushCommandLinkTexture(RenderCommands,
                                           Spritesheet->Width, 
                                           Spritesheet->Height,
                                           Spritesheet->Pixels,
                                           (u32)FileAssetId);
                } break;
                case asset_type::Sound: {
                    // TODO(Momo): Implement
                } break;
                
                
                
            }
            
        }
    }
    
}


// NOTE(Momo): Image Interface
static inline asset_data_image* 
GetImage(game_assets* Assets, asset_id Id) {
    Assert(Id < asset_id::Max);
    Assert(Assets->Entries[(u32)Id].Type == asset_type::Image);
    
    return Assets->Entries[(u32)Id].Image;
}


// NOTE(Momo): Spritesheet Interface
static inline rect2f 
GetSpritesheetFrame(game_assets* Assets, asset_id Id, u32 FrameIndex) {
    Assert(Id < asset_id::Max);
    Assert(Assets->Entries[(u32)Id].Type == asset_type::Spritesheet);
    
    asset_data_spritesheet* Spritesheet = Assets->Entries[(u32)Id].Spritesheet;
    Assert(FrameIndex < (Spritesheet->Rows * Spritesheet->Cols));
    return Spritesheet->Frames[FrameIndex];
}



#endif  