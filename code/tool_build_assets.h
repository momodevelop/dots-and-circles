/* date = April 30th 2021 10:59 am */

#ifndef TOOL_BUILD_ASSETS_H
#define TOOL_BUILD_ASSETS_H

struct Asset_Builder {
    FILE* file;
    u32 entry_count;
    long int entry_count_at;
    
    void begin(const char* filename, const char* signature);
    void end();
    void write_entry(Asset_Type asset_type);
    void write_texture(Texture_ID id, u32 w, u32 h, u32 c, u8* pixels);
    void write_texture_from_file(Texture_ID id, const char* filename);
    void write_image(Image_ID id, Texture_ID target_texture_id, aabb2u box);
    void write_font(Font_ID id, f32 ascent, f32 descent, f32 line_gap);
    void write_font_glyph(Font_ID font_id, Image_ID image_id, Texture_ID texture_id, u32 codepoint, f32 advance, f32 left_bearing, aabb2u image_aabb, aabb2f box);
    void write_font_kerning(Font_ID font_id, u32 codepoint_a, u32 codepoint_b, s32 kerning);
    void write_anime(Anime_ID id, Image_ID* frames, u32 frame_count);
    void write_msg(Msg_ID id, const char* str);
    void write_sound(Sound_ID id, Wav_Load_Result* wav_result);
};

void
Asset_Builder::begin(const char* filename, 
                     const char* signature) 
{
    
    this->entry_count = 0;
    this->file = nullptr; 
	fopen_s(&this->file, filename, "wb");
    ASSERT(this->file);
    
    // NOTE(Momo): Write signature
    fwrite(signature, sizeof(u8), cstr_length(signature), this->file);
    this->entry_count_at = ftell(this->file);
    
    // NOTE(Momo): Reserve space for entry_count
    fwrite(&this->entry_count, sizeof(u32), 1, this->file);
}

void
Asset_Builder::end() 
{
    fseek(this->file, this->entry_count_at, SEEK_SET);
    fwrite(&this->entry_count, sizeof(u32), 1, this->file);
    fclose(this->file);
}

void
Asset_Builder::write_entry(Asset_Type asset_type) 
{
    Asset_File_Entry entry = {};
    entry.type = asset_type;
    fwrite(&entry, sizeof(entry),  1, this->file);
    ++this->entry_count;
}

void 
Asset_Builder::write_texture(Texture_ID id, 
                             u32 width, 
                             u32 height, 
                             u32 channels, 
                             u8* pixels) 
{
    write_entry(ASSET_TYPE_TEXTURE);
    
    Asset_File_Texture texture = {};
    texture.id = id;
    texture.width = width;
    texture.height = height;
    texture.channels = channels;
    fwrite(&texture, sizeof(texture), 1, this->file);
    
    
    u32 texture_size = width * height * channels;
    for(u32 i = 0; i < texture_size; ++i) {
        fwrite(pixels + i, 1, 1, this->file);
    }
}


void 
Asset_Builder::write_texture_from_file(Texture_ID id, 
                                       const char* filename) 
{
    u32 width = 0, height = 0, channels = 0;
    u8* loaded_image = nullptr;
    {
        s32 w, h, c;
        loaded_image = stbi_load(filename, &w, &h, &c, 0);
        ASSERT(loaded_image != nullptr);
        
        width = (u32)w;
        height = (u32)h; 
        channels = (u32)c;
    }
    defer { stbi_image_free(loaded_image); };
    write_texture(id, width, height, channels, loaded_image);
}


void 
Asset_Builder::write_image(Image_ID id, 
                           Texture_ID target_texture_id, 
                           aabb2u box) 
{
    write_entry(ASSET_TYPE_IMAGE);
    
    Asset_File_Image image = {};
    image.id = id;
    image.texture_id = target_texture_id;
    image.box = box;
    fwrite(&image, sizeof(image), 1,  this->file);
}

void
Asset_Builder::write_font(Font_ID id, 
                          f32 ascent, 
                          f32 descent, 
                          f32 line_gap) 
{
    write_entry(ASSET_TYPE_FONT);
    
    Asset_File_Font font = {};
    font.id = id;
    font.ascent = ascent;
    font.descent = descent;
    font.line_gap = line_gap;
    fwrite(&font, sizeof(font), 1, this->file);
}

void 
Asset_Builder::write_font_glyph(Font_ID font_id, 
                                Image_ID image_id,
                                Texture_ID texture_id, 
                                u32 codepoint, 
                                f32 advance, 
                                f32 left_bearing, 
                                aabb2u image_box, 
                                aabb2f box) 
{
    write_image(image_id, texture_id, image_box);
    write_entry(ASSET_TYPE_FONT_GLYPH);
    
    Asset_File_Font_Glyph font_glyph = {};
    font_glyph.font_id = font_id;
    font_glyph.image_id = image_id;
    font_glyph.codepoint = codepoint;
    font_glyph.left_bearing = left_bearing;
    font_glyph.advance = advance;
    font_glyph.box = box;
    fwrite(&font_glyph, sizeof(font_glyph), 1, this->file);
    
}

void 
Asset_Builder::write_font_kerning(Font_ID font_id, 
                                  u32 codepoint_a,
                                  u32 codepoint_b, 
                                  s32 kerning) 
{
    write_entry(ASSET_TYPE_FONT_KERNING);
    
    Asset_File_Font_Kerning font = {};
    font.font_id = font_id;
    font.kerning = kerning;
    font.codepoint_a = codepoint_a;
    font.codepoint_b = codepoint_b;
    fwrite(&font, sizeof(font), 1, this->file);
    
}

void 
Asset_Builder::write_anime(Anime_ID anime_id, 
                           Image_ID* frames,
                           u32 frame_count) 
{
    write_entry(ASSET_TYPE_ANIME);
    
    Asset_File_Anime anime = {};
    anime.id = anime_id;
    anime.frame_count = frame_count;
    
    fwrite(&anime, sizeof(anime), 1, this->file);
    fwrite(frames, sizeof(Image_ID), frame_count, this->file);
}

void 
Asset_Builder::write_msg(Msg_ID msg_id,
                         const char* message) 
{
    write_entry(ASSET_TYPE_MSG);
    
    Asset_File_Msg msg = {};
    msg.id = msg_id;
    msg.count = cstr_length(message) - 1;
    
    fwrite(&msg, sizeof(msg), 1, this->file);
    fwrite(message, sizeof(char), msg.count, this->file);
}

void
Asset_Builder::write_sound(Sound_ID sound_id,
                           Wav_Load_Result* wav_result) 
{
    // We restrict the type of sound the game allows here
    ASSERT(wav_result->fmt_chunk.num_channels == GAME_AUDIO_CHANNELS);
    ASSERT(wav_result->fmt_chunk.sample_rate == GAME_AUDIO_SAMPLES_PER_SECOND);
    ASSERT(wav_result->fmt_chunk.bits_per_sample == GAME_AUDIO_BITS_PER_SAMPLE);
    
    write_entry(ASSET_TYPE_SOUND);
    
    Asset_File_Sound sound = {};
    sound.id = sound_id;
    sound.data_count = wav_result->data_chunk.size / sizeof(s16);
    
    fwrite(&sound, sizeof(sound), 1, this->file);
    fwrite(wav_result->data, sizeof(s16), sound.data_count, this->file);
    
}

#endif //TOOL_BUILD_ASSETS_H
