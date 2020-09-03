#ifndef GAME_ASSETS_TYPE_ID 
#define GAME_ASSETS_TYPE_ID

static constexpr u8 AssetSignature[] = { 'M', 'O', 'M', 'O' };


enum struct asset_type : u32 {
    Image,
    Font,
    Spritesheet,
    Sound,
};

enum struct asset_id : u32 {
    // NOTE(Momo): Images
    Image_Ryoji,
    Image_Yuu,
    
    // NOTE(Momo): Spritesheets
    Spritesheet_Karu,
    
    Max,
};

#endif 
