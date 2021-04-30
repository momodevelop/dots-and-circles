#ifndef GAME_ASSETS_TYPES 
#define GAME_ASSETS_TYPES


enum texture_id {
    Texture_AtlasDefault,
    
    Texture_Count,
};
// NOTE(Momo): We cater for a fixed set of codepoints. 
// ASCII 32 to 126 
// Worry about sparseness next time.

#define FontGlyph_CodepointStart 32
#define FontGlyph_CodepointEnd 126
#define FontGlyph_Count FontGlyph_CodepointEnd - FontGlyph_CodepointStart

enum sound_id {
    Sound_Test,
    
    Sound_Count,
};

enum font_id {
    Font_Default,
    
    Font_Count,
};

enum anime_id {
    Anime_KaruFront,
    
    Anime_Count,
};

enum atlas_aabb_id {
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

enum msg_id {
    Msg_ConsoleJumpToMain,
    Msg_ConsoleJumpToSandbox,
    Msg_ConsoleJumpToMenu,
    
    Msg_Count,
};

enum asset_type {
    AssetType_Texture,
    AssetType_AtlasAabb,
    AssetType_Font,
    AssetType_FontGlyph,
    AssetType_FontKerning,
    AssetType_Sound,
    AssetType_Anime,
    AssetType_Message,
};


// NOTE(Momo): asset pack file structures
#pragma pack(push, 1)
struct asset_file_entry {
    asset_type Type;
};

struct asset_file_msg {
    msg_id Id;
    u32 Count;
    // NOTE(Momo): Data is:
    // u8 String[Count]
};

struct asset_file_texture {
    texture_id Id;
    u32 Width;
    u32 Height;
    u32 Channels;
    // NOTE(Momo): Data is:
    // u8 Pixels[Width * Height * Channels]; 
    
};

struct asset_file_atlas_aabb {
    atlas_aabb_id Id;
    texture_id TextureId;
    
    aabb2u Aabb;
    
};

struct asset_file_font {
    font_id Id;
    f32 Ascent;
    f32 Descent;
    f32 LineGap;
};

struct asset_file_font_glyph {
    font_id FontId;
    texture_id TextureId;
    u32 Codepoint;
    f32 Advance;
    f32 LeftBearing;
    aabb2f Box; 
    aabb2u AtlasAabb;
    
};

struct asset_file_font_kerning {
    font_id FontId;
    s32 Kerning;
    u32 CodepointA;
    u32 CodepointB;
};

struct asset_file_anime {
    anime_id Id;
    u32 FrameCount;
    // NOTE(Momo): Data is:
    // atlas_aabb_id Data[Framecount]
};

struct asset_file_sound {
    sound_id SoundId;
    u32 DataSize;
    // NOTE(Momo): Data is: 
    // u16 Data[???]
};
#pragma pack(pop)

#endif 
