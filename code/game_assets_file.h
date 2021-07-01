#ifndef GAME_ASSETS_TYPES 
#define GAME_ASSETS_TYPES

enum Texture_ID {
    TEXTURE_ATLAS_DEFAULT,
    
    TEXTURE_COUNT,
};

// NOTE(Momo): For now we cater for a fixed set of codepoints. 
// Namely ASCII 32 to 126. Worry about unicode next time.


#define FONT_GLYPH_CODEPOINT_START 32
#define FONT_GLYPH_CODEPOINT_END 126
#define FONT_GLYPH_COUNT FONT_GLYPH_CODEPOINT_END - FONT_GLYPH_CODEPOINT_START

enum Sound_ID {
    SOUND_TEST,
    SOUND_MAIN_BGM,
    
    SOUND_COUNT,
};

enum Font_ID {
    FONT_DEFAULT,
    
    FONT_COUNT,
};

enum Anime_ID {
    ANIME_KARU_FRONT,
    
    ANIME_COUNT,
};

enum Image_ID {
    IMAGE_RYOJI,
    IMAGE_YUU,
    IMAGE_KARU_00,
    IMAGE_KARU_01,
    IMAGE_KARU_02,
    IMAGE_KARU_10,
    IMAGE_KARU_11,
    IMAGE_KARU_12,
    IMAGE_KARU_20,
    IMAGE_KARU_21,
    IMAGE_KARU_22,
    IMAGE_KARU_30,
    IMAGE_KARU_31,
    IMAGE_KARU_32,
    
    IMAGE_PLAYER_DOT,
    IMAGE_PLAYER_CIRCLE,
    IMAGE_BULLET_DOT,
    IMAGE_BULLET_CIRCLE,
    IMAGE_ENEMY,
    IMAGE_PARTICLE,
    
    IMAGE_FONT_START,
    IMAGE_FONT_END = IMAGE_FONT_START + FONT_GLYPH_COUNT,
    
    IMAGE_COUNT,
};

enum Msg_ID {
    MSG_CONSOLE_JUMP_TO_MAIN,
    MSG_CONSOLE_JUMP_TO_SANDBOX,
    MSG_CONSOLE_JUMP_TO_MENU,
    
    MSG_COUNT,
};

enum Asset_Type {
    ASSET_TYPE_TEXTURE,
    ASSET_TYPE_IMAGE,
    ASSET_TYPE_FONT,
    ASSET_TYPE_FONT_GLYPH,
    ASSET_TYPE_FONT_KERNING,
    ASSET_TYPE_SOUND,
    ASSET_TYPE_ANIME,
    ASSET_TYPE_MSG,
};


// NOTE(Momo): asset pack file structures
#pragma pack(push, 1)
struct Asset_File_Entry {
    Asset_Type type;
};

struct Asset_File_Msg {
    Msg_ID id;
    u32 count;
    // NOTE(Momo): Data is:
    // u8 string[count]
};

struct Asset_File_Texture {
    Texture_ID id;
    u32 width;
    u32 height;
    u32 channels;
    // NOTE(Momo): Data is:
    // u8 Pixels[width * height * channels]; 
    
};

struct Asset_File_Image {
    Image_ID id;
    
    Texture_ID texture_id;
    aabb2u box;
    
};

struct Asset_File_Font {
    Font_ID id;
    f32 ascent;
    f32 descent;
    f32 line_gap;
};

struct Asset_File_Font_Glyph {
    Font_ID font_id;
    Image_ID image_id;
    u32 codepoint;
    f32 advance;
    f32 left_bearing;
    aabb2f box; 
};

struct Asset_File_Font_Kerning {
    Font_ID font_id;
    s32 kerning;
    u32 codepoint_a;
    u32 codepoint_b;
};

struct Asset_File_Anime {
    Anime_ID id;
    u32 frame_count;
    // NOTE(Momo): Data is:
    // Image_ID Data[FrameCount]
};

struct Asset_File_Sound {
    Sound_ID id;
    
    u32 data_count;
    // NOTE(Momo): Data is: 
    // s16 Data[DataCount]
};
#pragma pack(pop)

#endif 
