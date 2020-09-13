#ifndef GAME_ASSET_ATLAS_H
#define GAME_ASSET_ATLAS_H

#include "game_atlas_types.h"

struct atlas_image {
    u32 RectIndex;
};

struct atlas_entry {
    u32 Id;
    atlas_entry_type Type;
    union {
        atlas_image* Image;
    };
};

struct atlas {
    atlas_entry* Entries;
    u32 EntryCount;
    
    rect2u* Rects;
    u32 RectCount;
    
    u32 Width;
    u32 Height;
    u32 Channels;
    void* Pixels;
    
    u32 BitmapId;
};

struct atlas_id { u32 Value; };

struct atlas_image_id { 
    atlas_id AtlasId;
    u32 Value;
};



static inline void
LoadAtlas(game_assets* Assets, commands* RenderCommands, asset_id Id, u8* Data)  {
    yuu_atlas* YuuAtlas = Read<yuu_atlas>(&Data);
    u8* const AtlasStart = Data;
    // NOTE(Momo): Allocate Atlas
    asset_entry* Entry = Assets->Entries + Id;
    {
        Entry->Type = AssetType_Atlas;
        Entry->Id = Id;
        Entry->Atlas = PushStruct<atlas>(&Assets->Arena);
        
        atlas* Atlas = Entry->Atlas;
        Atlas->Width = YuuAtlas->Width;
        Atlas->Height = YuuAtlas->Height;
        Atlas->Channels = YuuAtlas->Channels;
        Atlas->EntryCount = YuuAtlas->EntryCount;
        Atlas->RectCount = YuuAtlas->RectCount;
        Atlas->BitmapId = Assets->BitmapCounter++;
        
        // NOTE(Momo): Allocate Rects
        {
            Atlas->Rects = PushArray<rect2u>(&Assets->Arena, Atlas->EntryCount);
            for (u32 i = 0; i < Atlas->RectCount; ++i) {
                auto* Rect = Atlas->Rects + i;
                Rect->Min.X = Read32<u32>(&Data, false);
                Rect->Min.Y = Read32<u32>(&Data, false);
                Rect->Max.X = Read32<u32>(&Data, false) + Rect->Min.X;
                Rect->Max.Y = Read32<u32>(&Data, false) + Rect->Min.Y;
            }
        }
        
        // NOTE(Momo): Allocate pixel data
        {
            usize Size = Atlas->Width * Atlas->Height * Atlas->Channels;
            Atlas->Pixels = PushBlock(&Assets->Arena, Size, 1);
            Assert(Atlas->Pixels);
            CopyBlock(Atlas->Pixels, Data, Size);
            Data += Size;
        }
        
        // NOTE(Momo): Allocate entry data
        {
            Atlas->Entries = PushArray<atlas_entry>(&Assets->Arena, Atlas->EntryCount);
            for (u32 i = 0; i < Atlas->EntryCount; ++i) {
                auto* AtlasEntry = Atlas->Entries + i;
                auto* YuuAtlasEntry = Read<yuu_atlas_entry>(&Data); 
                
                AtlasEntry->Id = YuuAtlasEntry->Id;
                AtlasEntry->Type = YuuAtlasEntry->Type;
                
                switch(AtlasEntry->Type) {
                    case AtlasEntryType_Image: {
                        AtlasEntry->Image = PushStruct<atlas_image>(&Assets->Arena);
                        auto* YuuAtlasImage = Read<yuu_atlas_image>(&Data); 
                        AtlasEntry->Image->RectIndex = YuuAtlasImage->RectIndex;
                    } break;
                    
                    default: {
                        Assert(false);
                    }
                };
                
            }
        }
        
        
        
    }
    
    
    PushCommandLinkTexture(RenderCommands,
                           Entry->Atlas->Width, 
                           Entry->Atlas->Height,
                           Entry->Atlas->Pixels,
                           Entry->Atlas->BitmapId);
    
}


// NOTE(Momo): Atlas Interface
static inline atlas_id
GetAtlas(game_assets* Assets, asset_id Id) {
    asset_entry* Entry = Assets->Entries + Id;
    Assert(Entry->Type == AssetType_Atlas);
    return { Id };
}


static inline u32
GetBitmapId(game_assets* Assets, atlas_id Id) {
    auto* Entry = Assets->Entries + Id.Value;
    return Entry->Atlas->BitmapId;
}

static inline rect2u* 
GetRects(game_assets* Assets, atlas_id Id) {
    auto* Entry = Assets->Entries + Id.Value;
    Assert(Entry->Type == AssetType_Atlas);
    
    return Entry->Atlas->Rects;
}

static inline rect2f
GetImageUV(game_assets* Assets, atlas_id Id, u32 AtlasEntryId) {
    auto* Entry = Assets->Entries + Id.Value;
    Assert(Entry->Type == AssetType_Atlas);
    
    auto* Atlas = Entry->Atlas;
    auto* AtlasEntry = Atlas->Entries + AtlasEntryId;
    Assert(AtlasEntry->Type == AtlasEntryType_Image);
    
    u32 RectIndex = AtlasEntry->Image->RectIndex;
    rect2u Rect = Atlas->Rects[RectIndex];
    
    f32 UVXmin = (f32)Rect.Min.X / Atlas->Width;
    f32 UVYmin = 1.f - ((f32)GetHeight(Rect) / Atlas->Height); ;
    f32 UVXmax = UVXmin + ((f32)GetWidth(Rect) / Atlas->Width);
    f32 UVYmax = 1.f;
    
    
    return {
        UVXmin,
        UVYmin,
        UVXmax,
        UVYmax,
    };
}



#endif //GAME_ASSET_ATLAS_H
