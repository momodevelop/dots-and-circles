#ifndef GAME_ASSETS_TYPES 
#define GAME_ASSETS_TYPES

#include "mm_core.h"
#include "mm_maths.h"

enum game_asset_texture_id : u32 {
    Texture_Ryoji,
    Texture_Yuu,
    Texture_AtlasDefault,
    
    Texture_Count,
};

static constexpr u32 Codepoint_Start = 32;
static constexpr u32 Codepoint_End = 126;
static constexpr u32 Codepoint_Count = Codepoint_End - Codepoint_Start;

enum game_asset_font_id : u32 {
    Font_Default,
    Font_Count,
};


enum game_asset_atlas_aabb_id : u32 {
    AtlasAabb_Ryoji,
    AtlasAabb_Yuu,
    AtlasAabb_Karu00,
    AtlasAabb_Karu01,
    AtlasAabb_Karu02,
    AtlasAabb_Karu10,
    AtlasAabb_Karu11,
    AtlasAabb_Karu12,
    AtlasAabb_Karu20,
    AtlasAabb_Karu21,
    AtlasAabb_Karu22,
    AtlasAabb_Karu30,
    AtlasAabb_Karu31,
    AtlasAabb_Karu32,
    
    AtlasAabb_PlayerDot,
    AtlasAabb_PlayerCircle,
    AtlasAabb_BulletDot,
    AtlasAabb_BulletCircle,
    AtlasAabb_Enemy,

    
    AtlasAabb_Count,
};

enum game_asset_type : u32 {
    AssetType_Texture,
    AssetType_AtlasAabb,
    AssetType_Font,
    AssetType_FontGlyph,
    AssetType_FontKerning,
};


// NOTE(Momo): asset pack file structures
#pragma pack(push, 1)
struct game_asset_file_entry {
    game_asset_type Type;
};

struct game_asset_file_texture {
    game_asset_texture_id Id;
    u32 Width;
    u32 Height;
    u32 Channels;
    // NOTE(Momo): Data is:
    // u8 Pixels[Width * Height * Channels]; 
    
};

struct game_asset_file_atlas_aabb {
    game_asset_atlas_aabb_id Id;
    game_asset_texture_id TextureId;
    
    aabb2u Aabb;
    
};

struct game_asset_file_font {
    game_asset_font_id Id;
    f32 Ascent;
    f32 Descent;
    f32 LineGap;
};

struct game_asset_file_font_glyph {
    game_asset_font_id FontId;
    game_asset_texture_id TextureId;
    u32 Codepoint;
    f32 Advance;
    f32 LeftBearing;
    aabb2f Box; 
    aabb2u AtlasAabb;
    
};

struct game_asset_file_font_kerning {
    game_asset_font_id FontId;
    i32 Kerning;
    u32 CodepointA;
    u32 CodepointB;
};

#pragma pack(pop)

#endif 
