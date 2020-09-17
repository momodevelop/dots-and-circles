#ifndef __GAME_ASSETS__
#define __GAME_ASSETS__

#include "ryoji.h"
#include "ryoji_maths.h"
#include "ryoji_arenas.h"
#include "ryoji_commands.h"

#include "game_asset_types.h"


struct asset_entry {
    asset_type Type;
    union {
        struct image* Image;
        struct spritesheet* Spritesheet;
        struct atlas* Atlas;
        struct atlas_rect* AtlasRect;
    };
};

struct game_assets {
    arena Arena;
    asset_entry* Entries;
    u32 EntryCount;
    
    u32 BitmapCounter;
    
    platform_api* Platform;
};

#include "game_asset_image.h"
#include "game_asset_spritesheet.h"
#include "game_asset_atlas.h"
#include "game_asset_atlas_rect.h"

inline b32
CheckAssetSignature(void *Memory) {
    u8* MemoryU8 = (u8*)Memory;
    for (u32 i = 0; i < ArrayCount(AssetSignature); ++i) {
        if (MemoryU8[i] != AssetSignature[i]) {
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
        Assets->EntryCount = Read32<u32>(&FileMemoryItr, false);
    }
    
    // NOTE(Momo): Allocate Assets
    {
        // NOTE(Momo): All entries are allocated first. Their data will follow after.
        Assets->Entries = PushArray<asset_entry>(&Assets->Arena, Asset_Count);
        
        for (u32 i = 0; i < Assets->EntryCount; ++i)
        {
            // NOTE(Momo): Read headers
            auto* YuuEntry =  Read<yuu_entry>(&FileMemoryItr);
            
            u8* FileEntryDataItr = FileMemory +  YuuEntry->OffsetToData;
            switch(YuuEntry->Type) {
                case AssetType_Image: {
                    LoadImage(Assets, RenderCommands, YuuEntry->Id, FileEntryDataItr);
                } break;
                case AssetType_Spritesheet: {
                    LoadSpritesheet(Assets, RenderCommands, YuuEntry->Id, FileEntryDataItr);
                } break;
                case AssetType_Atlas: {
                    LoadAtlas(Assets, RenderCommands, YuuEntry->Id, FileEntryDataItr);
                } break;
                case AssetType_AtlasRect: { 
                    LoadAtlasRect(Assets, RenderCommands, YuuEntry->Id, FileEntryDataItr);
                } break;
                default: {
                    Assert(false);
                } break;
                
                
            }
            
        }
    }
    
}




#endif  