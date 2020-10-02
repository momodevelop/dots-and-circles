#ifndef __GAME_ASSETS__
#define __GAME_ASSETS__

#include "ryoji.h"
#include "ryoji_maths.h"
#include "ryoji_arenas.h"
#include "ryoji_commands.h"

#include "game_assets_file_formats.h"


// NOTE(Momo): Asset types
struct image {
    u32 Width;
    u32 Height;
    u32 Channels;
    void* Pixels; 
    u32 BitmapId;
};

struct atlas_rect {
    rect2u Rect;
    asset_id AtlasAssetId;
};

struct asset_entry {
    asset_type Type;
    union {
        struct image* Image;
        struct atlas_rect* AtlasRect;
    };
};

struct game_assets {
    arena Arena;
    asset_entry Entries[Asset_Count];
    
    u32 BitmapCounter;
    
    platform_api* Platform;
};



struct image_id { u32 Value; };

#include "game_assets_atlas_rect.h"

inline b32
CheckAssetSignature(void *Memory, const char* Signature) {
    u8* MemoryU8 = (u8*)Memory;
    u32 SigLen = NtsLength(Signature);
    for (u32 i = 0; i < SigLen; ++i) {
        if (MemoryU8[i] != Signature[i]) {
            return false;
        }
    }
    return true;
}


// TODO(Momo): Perhaps remove render_commands and replace with platform call for linking textures?
static inline void
Init(game_assets* Assets, 
     arena* Arena, 
     platform_api* Platform,
     commands* RenderCommands,
     const char* Filename)
{
    arena* AssetArena = &Assets->Arena;
    SubArena(AssetArena, Arena, Megabytes(100));
    Assets->Platform = Platform;
    
    auto Scratch = BeginTempArena(AssetArena);
    Defer{ EndTempArena(Scratch); };
    
    // NOTE(Momo): File read into temp arena
    // TODO(Momo): We could just create the assets as we read.
    u8* FileMemory = nullptr;
    u8* FileMemoryItr = nullptr;
    {
        u32 Filesize = Platform->GetFileSize(Filename);
        Assert(Filesize);
        FileMemory = FileMemoryItr = (u8*)PushBlock(&Scratch, Filesize);
        Platform->ReadFile(FileMemory, Filesize, Filename);
    }
    
    // NOTE(Momo): Read and check file header
    u32 FileEntryCount = 0;
    {
        Assert(CheckAssetSignature(FileMemoryItr, "MOMO"));
        FileMemoryItr+= NtsLength("MOMO");
        
        // NOTE(Momo): Read the counts in order
        FileEntryCount = *(Read<u32>(&FileMemoryItr));
    }
    
    // NOTE(Momo): Allocate Assets
    
    for (u32 i = 0; i < FileEntryCount; ++i) {
        // NOTE(Momo): Read header
        auto* YuuEntry = Read<yuu_entry>(&FileMemoryItr);
        
        switch(YuuEntry->Type) {
            case AssetType_Image: {
                auto* YuuImage = Read<yuu_image>(&FileMemoryItr);
                
                // NOTE(Momo): Allocate Image
                auto* Entry = Assets->Entries + YuuEntry->Id;
                Entry->Type = AssetType_Image;
                Entry->Image = PushStruct<image>(&Assets->Arena);
                
                auto* Image = Entry->Image;
                Image->Width = YuuImage->Width;
                Image->Height = YuuImage->Height;
                Image->Channels = YuuImage->Channels;
                Image->BitmapId = Assets->BitmapCounter++;
                
                // NOTE(Momo): Allocate pixel data
                usize BitmapSize = Image->Width * Image->Height * Image->Channels;
                Image->Pixels = PushBlock(&Assets->Arena, BitmapSize, 1);
                Assert(Image->Pixels);
                CopyBlock(Image->Pixels, FileMemoryItr, BitmapSize);
                FileMemoryItr += BitmapSize;
                
                PushCommandLinkTexture(RenderCommands, 
                                       Image->Width, 
                                       Image->Height,
                                       Image->Pixels,
                                       Image->BitmapId);
                
            } break;
            case AssetType_AtlasRect: { 
                auto* YuuAtlasRect = Read<yuu_atlas_rect>(&FileMemoryItr);
                
                auto* Entry = Assets->Entries + YuuEntry->Id;
                Entry->Type = AssetType_AtlasRect;
                Entry->AtlasRect = PushStruct<atlas_rect>(&Assets->Arena);
                
                auto* AtlasRect = Entry->AtlasRect;
                AtlasRect->Rect = YuuAtlasRect->Rect;
                AtlasRect->AtlasAssetId = YuuAtlasRect->AtlasAssetId;
                
            } break;
            default: {
                Assert(false);
            } break;
            
            
        }
        
    }
    
}


// NOTE(Momo): Image Interface
static inline image_id
GetImage(game_assets* Assets, asset_id Id) {
    asset_entry* Entry = Assets->Entries + Id;
    Assert(Entry->Type == AssetType_Image);
    return { Id };
}

static inline image*
GetImagePtr(game_assets* Assets, asset_id Id) {
    asset_entry* Entry = Assets->Entries + Id;
    Assert(Entry->Type == AssetType_Image);
    return Entry->Image;
}

static inline u32 
GetBitmapId(game_assets* Assets, image_id Id) {
    asset_entry* Entry = Assets->Entries + Id.Value;
    return Entry->Image->BitmapId;
}

// NOTE(Momo): Interfaces
static inline atlas_rect 
GetAtlasRect(game_assets* Assets, asset_id Id) {
    asset_entry* Entry = Assets->Entries + Id;
    Assert(Entry->Type == AssetType_AtlasRect);
    return *(Entry->AtlasRect);
}



#endif  