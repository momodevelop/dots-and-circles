#ifndef __GAME_ASSETS__
#define __GAME_ASSETS__

#include "ryoji.h"
#include "ryoji_maths.h"
#include "ryoji_arenas.h"
#include "ryoji_commands.h"

#include "game_assets_file_formats.h"


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

#include "game_assets_image.h"
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
        FileMemoryItr+= ArrayCount(Signature);
        
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
            } break;
            default: {
                Assert(false);
            } break;
            
            
        }
        
    }
    
}




#endif  