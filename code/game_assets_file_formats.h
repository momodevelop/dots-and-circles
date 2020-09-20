#ifndef GAME_ASSETS_TYPE_ID 
#define GAME_ASSETS_TYPE_ID

static constexpr u8 AssetSignature[] = { 'M', 'O', 'M', 'O' };


// NOTE(Momo): Asset Structs
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
    AssetType_AtlasRect,
};


enum asset_id : u32 {
    Asset_ImageRyoji,
    Asset_ImageYuu,
    Asset_ImageAtlasDefault,
    
    Asset_SpritesheetKaru,
    
    
    // NOTE(Momo): Rects
    Asset_RectRyoji,
    Asset_RectYuu,
    Asset_RectKaru00,
    Asset_RectKaru01,
    Asset_RectKaru02,
    Asset_RectKaru10,
    Asset_RectKaru11,
    Asset_RectKaru12,
    Asset_RectKaru20,
    Asset_RectKaru21,
    Asset_RectKaru22,
    Asset_RectKaru30,
    Asset_RectKaru31,
    Asset_RectKaru32,
    
#if 0
    // NOTE(Momo): Font rects ahhhh
    Asset_FontRect_a,
    Asset_FontRect_b,
    Asset_FontRect_c,
    Asset_FontRect_d,
    Asset_FontRect_e,
    Asset_FontRect_f,
    Asset_FontRect_g,
    Asset_FontRect_h,
    Asset_FontRect_i,
    Asset_FontRect_j,
    Asset_FontRect_k,
    Asset_FontRect_l,
    Asset_FontRect_m,
    Asset_FontRect_n,
    Asset_FontRect_o,
    Asset_FontRect_p,
    Asset_FontRect_q,
    Asset_FontRect_r,
    Asset_FontRect_s,
    Asset_FontRect_t,
    Asset_FontRect_u,
    Asset_FontRect_v,
    Asset_FontRect_w,
    Asset_FontRect_x,
    Asset_FontRect_y,
    Asset_FontRect_z,
    
    Asset_FontRect_A,
    Asset_FontRect_B,
    Asset_FontRect_C,
    Asset_FontRect_D,
    Asset_FontRect_E,
    Asset_FontRect_F,
    Asset_FontRect_G,
    Asset_FontRect_H,
    Asset_FontRect_I,
    Asset_FontRect_J,
    Asset_FontRect_K,
    Asset_FontRect_L,
    Asset_FontRect_M,
    Asset_FontRect_N,
    Asset_FontRect_O,
    Asset_FontRect_P,
    Asset_FontRect_Q,
    Asset_FontRect_R,
    Asset_FontRect_S,
    Asset_FontRect_T,
    Asset_FontRect_U,
    Asset_FontRect_V,
    Asset_FontRect_W,
    Asset_FontRect_X,
    Asset_FontRect_Y,
    Asset_FontRect_Z,
#endif
    
    Asset_Count,
};


// NOTE(Momo): asset pack file structures
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


struct yuu_atlas_rect {
    rect2u Rect;
    asset_id AtlasAssetId;
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

// NOTE(Momo): Atlas User Data Structs
enum yuu_atlas_ud_type : u32 {
    YuuAtlasUserDataType_Image,
    YuuAtlasuserdataType_Font,
};


struct yuu_atlas_ud_image {
    yuu_atlas_ud_type Type;
    asset_id AssetId;
    asset_id AtlasAssetId; 
};

struct yuu_atlas_ud_font {
    yuu_atlas_ud_type Type;
    asset_id AssetId;
    asset_id AtlasAssetId; 
    int Codepoint;
};


#pragma pack(pop)


#endif 