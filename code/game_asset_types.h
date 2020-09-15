#ifndef GAME_ASSETS_TYPE_ID 
#define GAME_ASSETS_TYPE_ID

static constexpr u8 AssetSignature[] = { 'M', 'O', 'M', 'O' };


enum atlas_entry_type : u32 {
    AtlasEntryType_Image,
};

enum atlas_entry_id : u32 {
    // NOTE(Momo): for default
    AtlasDefault_Ryoji = 0,
    AtlasDefault_Yuu,
    
    AtlasDefault_Karu00,
    AtlasDefault_Karu01,
    AtlasDefault_Karu02,
    AtlasDefault_Karu10,
    AtlasDefault_Karu11,
    AtlasDefault_Karu12,
    AtlasDefault_Karu20,
    AtlasDefault_Karu21,
    AtlasDefault_Karu22,
    AtlasDefault_Karu30,
    AtlasDefault_Karu31,
    AtlasDefault_Karu32,
    AtlasDefault_Count,
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
struct yuu_entry {
    asset_type Type;
    u32 OffsetToData;
    asset_id Id;
};

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
    atlas_entry_id Id; // TODO(Momo): atlas_entry_id
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