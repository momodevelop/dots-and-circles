#ifndef __GAME_ASSETS_H__
#define __GAME_ASSETS_H__

// NOTE(Momo): Asset types
struct Texture {
    u32 width, height, channels;
    u8* data; // RGBA format
    Renderer_Texture_Handle handle;
};

struct Anime {
    Image_ID* frames;
    u32 frame_count;
};

struct Image {
    aabb2u box;
    Texture_ID texture_id;
};

struct Font_Glyph {
    Image_ID image_id;
    aabb2f box;
    f32 advance;
    f32 left_bearing;
};

struct Font {
    // NOTE(Momo): We cater for a fixed set of codepoints. 
    // ASCII 32 to 126 
    // Worry about sparseness next time.
    f32 line_gap;
    f32 ascent;
    f32 descent;
    Font_Glyph glyphs[FONT_GLYPH_COUNT];
    u32 kernings[FONT_GLYPH_COUNT][FONT_GLYPH_COUNT];
};

struct Sound {
    s16* data;
    u32 data_count;
};

typedef String Msg;

struct Assets {
    Texture* textures;
    u32 texture_count;
    
    Image* images;
    u32 image_count;
    
    Font* fonts;
    u32 font_count;
    
    Anime* animes;
    u32 anime_count;
    
    Msg* msgs;
    u32 msg_count;
    
    Sound* sounds;
    u32 sound_count;
};

#include "game_assets.cpp"
#endif  
