#ifndef GAME_ASSET_ATLAS_RECT_H
#define GAME_ASSET_ATLAS_RECT_H

struct atlas_rect {
    rect2u Rect;
    asset_id AtlasAssetId;
};

static inline void
LoadAtlasRect(game_assets* Assets, commands* RenderCommands, asset_id Id, u8* Data) {
    auto* Yuu = Read<yuu_atlas_rect>(&Data);
    
    // NOTE(Momo): Allocate the font
    asset_entry* Entry = Assets->Entries + Id;
    {
        Entry->Type = AssetType_AtlasRect;
        Entry->AtlasRect = PushStruct<atlas_rect>(&Assets->Arena);
        Entry->AtlasRect->Rect = Yuu->Rect;
        Entry->AtlasRect->AtlasAssetId = Yuu->AtlasAssetId;
    }
}


// NOTE(Momo): Interfaces
static inline atlas_rect 
GetAtlasRect(game_assets* Assets, asset_id Id) {
    asset_entry* Entry = Assets->Entries + Id;
    Assert(Entry->Type == AssetType_AtlasRect);
    return *(Entry->AtlasRect);
}




#endif //GAME_ASSET_ATLAS_RECT_H
