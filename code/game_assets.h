#ifndef __GAME_ASSETS__
#define __GAME_ASSETS__

#include "ryoji.h"
#include "ryoji_maths.h"
#include "ryoji_arenas.h"
#include "ryoji_commands.h"

#include "game_asset_types.h"

struct asset_image {
    u32 Width;
    u32 Height;
    void* Pixels; 
    u32 BitmapId;
};

struct asset_spritesheet {
    u32 Width;
    u32 Height;
    void* Pixels; 
    
    rect2f* Frames;
    u32 Rows;
    u32 Cols;
    
    u32 BitmapId;
};

struct game_assets {
    memory_arena Arena;
    
    asset_image* Images;
    u32 ImageCount;
    
    asset_spritesheet* Spritesheets;
    u32 SpritesheetCount;
    
    // TODO(Momo): Eventually want to merge spritesheets and images together.
    u32 BitmapCounter;
    
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


static inline void
LoadImage(game_assets* Assets, commands* RenderCommands, image_id Id, u8** Data) {
    asset_image* Image = Assets->Images + (u32)Id;
    
    u32 Width = Read32<u32>(Data, false);
    u32 Height = Read32<u32>(Data, false);
    u32 Channels = Read32<u32>(Data, false);
    
    Image->Width = Width;
    Image->Height = Height;
    Image->BitmapId = Assets->BitmapCounter++;
    
    // NOTE(Momo): Allocate pixel data
    usize Size = Width * Height * Channels;
    Image->Pixels = PushBlock(&Assets->Arena, Size, 1);
    Assert(Image->Pixels);
    CopyBlock(Image->Pixels, (*Data), Size);
    
    PushCommandLinkTexture(RenderCommands, 
                           Image->Width, 
                           Image->Height,
                           Image->Pixels,
                           Image->BitmapId);
    
}


static inline void
LoadSpritesheet(game_assets* Assets, commands* RenderCommands, spritesheet_id Id, u8** Data) 
{
    asset_spritesheet* Spritesheet = Assets->Spritesheets + (u32)Id;
    
    u32 Width = Read32<u32>(Data, false);
    u32 Height = Read32<u32>(Data, false);
    u32 Channels = Read32<u32>(Data, false);
    u32 Rows = Read32<u32>(Data, false);
    u32 Cols = Read32<u32>(Data, false);
    
    Spritesheet->Width = Width;
    Spritesheet->Height = Height;
    Spritesheet->Rows = Rows;
    Spritesheet->Cols = Cols; 
    Spritesheet->BitmapId = Assets->BitmapCounter++;
    
    // NOTE(Momo): Allocate pixel data
    usize Size = Width * Height * Channels;
    Spritesheet->Pixels = PushBlock(&Assets->Arena, Size, 1);
    Assert(Spritesheet->Pixels);
    CopyBlock(Spritesheet->Pixels, (*Data), Size);
    
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
    Spritesheet->Frames = Rects;
    
    PushCommandLinkTexture(RenderCommands,
                           Spritesheet->Width, 
                           Spritesheet->Height,
                           Spritesheet->Pixels,
                           Spritesheet->BitmapId);
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
        
        // NOTE(Momo): Read the counts in order
        Assets->ImageCount = Read32<u32>(&FileMemoryItr, false);
        Assets->SpritesheetCount = Read32<u32>(&FileMemoryItr, false);
        
    }
    
    // NOTE(Momo): Allocate Assets
    {
        // NOTE(Momo): All entries are allocated first. Their data will follow after.
        Assets->Images = PushArray<asset_image>(&Assets->Arena, Assets->ImageCount);
        Assets->Spritesheets = PushArray<asset_spritesheet>(&Assets->Arena, Assets->SpritesheetCount);
        u32 AssetCount = Assets->ImageCount + Assets->SpritesheetCount;
        
        for (u32 i = 0; i < AssetCount; ++i)
        {
            // NOTE(Momo): Read headers
            auto FileAssetType = Read32<asset_type>(&FileMemoryItr, false);
            u32 FileOffsetToEntry = Read32<u32>(&FileMemoryItr, false);
            u8* FileEntryItr = FileMemory + FileOffsetToEntry;
            
            switch(FileAssetType) {
                case asset_type::Image: {
                    auto Id = Read32<image_id>(&FileMemoryItr, false);
                    LoadImage(Assets, RenderCommands, Id, &FileEntryItr);
                    
                } break;
                case asset_type::Font: {
                    // TODO(Momo): Implement
                } break;
                case asset_type::Spritesheet: {
                    auto Id = Read32<spritesheet_id>(&FileMemoryItr, false);
                    LoadSpritesheet(Assets, RenderCommands, Id, &FileEntryItr);
                } break;
                case asset_type::Sound: {
                    // TODO(Momo): Implement
                } break;
                
                
                
            }
            
        }
    }
    
}


// NOTE(Momo): Image Interface
static inline u32 
GetBitmapId(game_assets* Assets, image_id Id) {
    return (Assets->Images + (u32)Id)->BitmapId;
}

// NOTE(Momo): Spritesheet Interface
static inline rect2f 
GetSpritesheetFrame(game_assets* Assets, spritesheet_id Id, u32 FrameIndex) {
    asset_spritesheet* Spritesheet = Assets->Spritesheets + (u32)Id;
    Assert(FrameIndex < (Spritesheet->Rows * Spritesheet->Cols));
    return Spritesheet->Frames[FrameIndex];
}


static inline u32 
GetBitmapId(game_assets* Assets, spritesheet_id Id) {
    return (Assets->Spritesheets + (u32)Id)->BitmapId;
}

#endif  