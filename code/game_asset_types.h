#ifndef GAME_ASSETS_TYPE_ID 
#define GAME_ASSETS_TYPE_ID

static constexpr u8 AssetSignature[] = { 'M', 'O', 'M', 'O' };


enum struct asset_type : u32 {
    Image,
    Font,
    Spritesheet,
    Sound,
};


enum struct image_id : u32 {
    Ryoji,
    Yuu,
};

enum struct spritesheet_id : u32 {
    Karu,
};

#endif 
