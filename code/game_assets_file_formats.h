#ifndef GAME_ASSETS_TYPE_ID 
#define GAME_ASSETS_TYPE_ID

#include "ryoji_maths.h"

enum bitmap_id : u32 {
    Bitmap_Ryoji,
    Bitmap_Yuu,
    Bitmap_AtlasDefault,
    
    Bitmap_Count,
};

static constexpr u32 Codepoint_Start = 32;
static constexpr u32 Codepoint_End = 126;
static constexpr u32 Codepoint_Count = Codepoint_End - Codepoint_Start;

enum font_id : u32 {
    Font_Default,
    Font_Count,
};


enum atlas_rect_id : u32 {
    AtlasRect_Ryoji,
    AtlasRect_Yuu,
    AtlasRect_Karu00,
    AtlasRect_Karu01,
    AtlasRect_Karu02,
    AtlasRect_Karu10,
    AtlasRect_Karu11,
    AtlasRect_Karu12,
    AtlasRect_Karu20,
    AtlasRect_Karu21,
    AtlasRect_Karu22,
    AtlasRect_Karu30,
    AtlasRect_Karu31,
    AtlasRect_Karu32,
    
    AtlasRect_Count,
};

enum asset_type : u32 {
    AssetType_Bitmap,
    AssetType_AtlasRect,
    AssetType_Font,
    AssetType_FontGlyph,
    AssetType_FontKerning,
};


// NOTE(Momo): asset pack file structures
#pragma pack(push, 1)
struct yuu_entry {
    asset_type Type;
};

struct yuu_bitmap {
    bitmap_id Id;
    u32 Width;
    u32 Height;
    u32 Channels;
    // NOTE(Momo): Data is:
    /* 
u8 Pixels[Width * Height * Channels]; 
*/
};


struct yuu_atlas_rect {
    atlas_rect_id Id;
    bitmap_id BitmapId;
    rect2u Rect;
    
};

struct yuu_font {
    font_id Id;
    f32 Size;
    // NOTE(Momo): Other general font info here
};

struct yuu_font_glyph {
    font_id FontId;
    bitmap_id BitmapId;
    u32 Codepoint;
    u32 Advance;
    rect2u Rect;
};

struct yuu_font_kerning {
    font_id FontId;
    u32 Kerning;
    u32 CodepointA;
    u32 CodepointB;
};

#pragma pack(pop)

#endif 