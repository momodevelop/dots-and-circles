#ifndef GAME_ASSETS_TYPE_ID 
#define GAME_ASSETS_TYPE_ID

static constexpr u8 AssetSignature[] = { 'M', 'O', 'M', 'O' };

enum asset_type : u32 {
    AssetType_Image,
    AssetType_Font,
    AssetType_Spritesheet,
    AssetType_Sound,
    AssetType_Atlas,
};


enum asset_id : u32 {
    Asset_ImageRyoji,
    Asset_ImageYuu,
    Asset_SpritesheetKaru,
};


enum atlas_image_id : u32 {
    Atlas_ImageRyoji,
    Atlas_ImageYuu,
};


#endif 