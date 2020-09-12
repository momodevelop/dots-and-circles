#ifndef GAME_ASSET_ATLAS_H
#define GAME_ASSET_ATLAS_H

struct atlas_image {
    u32 RectIndex;
};

struct atlas_entry {
    u32 Id;
    atlas_entry_type Type;
    union {
        void* Data;
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
    u8* AtlasStart = Data;
    
    u32 EntryCount = Read32<u32>(&Data, false);
    u32 EntryDataOffset = Read32<u32>(&Data, false);
    u32 RectCount = Read32<u32>(&Data, false);
    u32 RectDataOffset = Read32<u32>(&Data, false);
    u32 BitmapWidth = Read32<u32>(&Data, false);
    u32 BitmapHeight = Read32<u32>(&Data, false);
    u32 BitmapChannels = Read32<u32>(&Data, false);
    u32 BitmapDataOffset = Read32<u32>(&Data, false);
    
    
    // NOTE(Momo): Allocate Atlas
    asset_entry* Entry = Assets->Entries + Id;
    {
        Entry->Type = AssetType_Atlas;
        Entry->Id = Id;
        
        Entry->Atlas = PushStruct<atlas>(&Assets->Arena);
        Entry->Atlas->Width = BitmapWidth;
        Entry->Atlas->Height = BitmapHeight;
        Entry->Atlas->Channels = BitmapChannels;
        Entry->Atlas->EntryCount = EntryCount;
        Entry->Atlas->RectCount = RectCount;
        
        
        // NOTE(Momo): Allocate pixel data
        u8* BitmapAt = AtlasStart + BitmapDataOffset;
        usize Size = BitmapWidth * BitmapHeight * BitmapChannels;
        Entry->Atlas->Pixels = PushBlock(&Assets->Arena, Size, 1);
        Assert(Entry->Atlas->Pixels);
        CopyBlock(Entry->Atlas->Pixels, BitmapAt, Size);
        
        // NOTE(Momo): Allocate Rects
        Entry->Atlas->Rects = PushArray<rect2u>(&Assets->Arena, RectCount);
        u8* RectAt = AtlasStart + RectDataOffset;
        for (u32 i = 0; i < RectCount; ++i) {
            auto Rect = Entry->Atlas->Rects + i;
            Rect->Min.X = Read32<u32>(&RectAt, false);
            Rect->Min.Y = Read32<u32>(&RectAt, false);
            Rect->Max.X = Read32<u32>(&RectAt, false) + Rect->Min.X;
            Rect->Max.Y = Read32<u32>(&RectAt, false) + Rect->Min.Y;
        }
        
        
        Entry->Atlas->Entries = PushArray<atlas_entry>(&Assets->Arena, EntryCount);
        u8* EntryAt = AtlasStart + EntryDataOffset;
        for (u32 i = 0; i < EntryCount; ++i) {
            auto* AtlasEntry = Entry->Atlas->Entries + i;
            AtlasEntry->Id = Read32<u32>(&EntryAt, false);
            AtlasEntry->Type = Read32<atlas_entry_type>(&EntryAt, false);
            u32 DataOffset = Read32<u32>(&EntryAt, false);
            
            u8* DataAt = AtlasStart + DataOffset;
            switch(AtlasEntry->Type) {
                case AtlasType_Image: {
                    AtlasEntry->Image = PushStruct<atlas_image>(&Assets->Arena);
                    AtlasEntry->Image->RectIndex = Read32<u32>(&DataAt, false);
                } break;
                
                default: {
                    Assert(false);
                }
            };
            
        }
        
        
        
        Entry->Atlas->BitmapId = Assets->BitmapCounter++;
    }
    
    
    PushCommandLinkTexture(RenderCommands,
                           Entry->Atlas->Width, 
                           Entry->Atlas->Height,
                           Entry->Atlas->Pixels,
                           Entry->Atlas->BitmapId);
    
}


// NOTE(Momo): Atlas  Interface
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
    Assert(AtlasEntry->Type == AtlasType_Image);
    
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
