#ifndef GAME_ASSETS_TYPE_ID 
#define GAME_ASSETS_TYPE_ID

static constexpr u8 AssetSignature[] = { 'M', 'O', 'M', 'O' };


enum atlas_entry_type : u32 {
    AtlasEntryType_Image,
};

enum atlas_entry_id : u32 {
    // NOTE(Momo): for default
    AtlasEntry_Default_Ryoji,
    AtlasEntry_Default_Yuu,
    
    
    // NOTE(Momo): Other atlas types
};



enum asset_type : u32 {
    AssetType_Image,
    AssetType_Spritesheet,
    AssetType_Sound,
    AssetType_Atlas,
};


enum asset_id : u32 {
    Asset_ImageRyoji,
    Asset_ImageYuu,
    Asset_SpritesheetKaru,
    Asset_AtlasDefault,
    Asset_Count,
};


#pragma pack(push, 1)
struct yuu_image {
    u32 Width;
    u32 Height;
    u32 Channels;
    // NOTE(Momo): Data is:
    /*
u8 Pixels[Width * Height * Channels];
*/
    
    
};


struct yuu_atlas_image {
    u32 RectIndex;
};

struct yuu_atlas_entry {
    u32 Id; // TODO(Momo): atlas_entry_id
    atlas_entry_type Type; 
    
    // NOTE(Momo): Data is:
    /*
    if Image: {
----RectIndex
}
else : {

}
        
    
*/
};


struct yuu_atlas {
    u32 EntryCount;
    u32 RectCount;
    u32 Width;
    u32 Height;
    u32 Channels;
    // NOTE(Momo): Data is:
    /*
rect2u Rects[RectCount]
u8 Pixles[Width * Height * Channels]
yuu_atlas_entry Entries[EntryCount]
*/
    
};


struct yuu_spritesheet {
    u32 Width;
    u32 Height;
    u32 Channels;
    
    u32 Rows;
    u32 Cols;
    
    // NOTE(Momo): Data is:
    /*
u8 Pixels[Width * Height * Channels];
*/
};
#pragma pack(pop)


#endif 