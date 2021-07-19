

//~ NOTE(Momo): Font functions
static inline f32 
Font_Height(Font* f) {
    return Abs(f->ascent) + Abs(f->descent);
}

u32
Font_GetKerning(Font* f, u32 codepoint_a, u32 codepoint_b) {
    ASSERT(codepoint_a >= FONT_GLYPH_CODEPOINT_START);
    ASSERT(codepoint_a <= FONT_GLYPH_CODEPOINT_END);
    
    ASSERT(codepoint_b >= FONT_GLYPH_CODEPOINT_START);
    ASSERT(codepoint_b <= FONT_GLYPH_CODEPOINT_END);
    u32 index_a = codepoint_a - FONT_GLYPH_CODEPOINT_START;
    u32 index_b = codepoint_b - FONT_GLYPH_CODEPOINT_START;
    u32 ret = f->kernings[index_a][index_b];
    
    return ret;
}

void
Font_SetKerning(Font* f, u32 codepoint_a, u32 codepoint_b, u32 kerning) {
    ASSERT(codepoint_a >= FONT_GLYPH_CODEPOINT_START);
    ASSERT(codepoint_a <= FONT_GLYPH_CODEPOINT_END);
    
    ASSERT(codepoint_b >= FONT_GLYPH_CODEPOINT_START);
    ASSERT(codepoint_b <= FONT_GLYPH_CODEPOINT_END);
    
    u32 index_a = codepoint_a - FONT_GLYPH_CODEPOINT_START;
    u32 index_b = codepoint_b - FONT_GLYPH_CODEPOINT_START;
    f->kernings[index_a][index_b] = kerning;
}

Font_Glyph* 
Font_GetGlyph(Font* f, u32 codepoint) {
    ASSERT(codepoint >= FONT_GLYPH_CODEPOINT_START);
    ASSERT(codepoint <= FONT_GLYPH_CODEPOINT_END);
    u32 index = codepoint - FONT_GLYPH_CODEPOINT_START;
    Font_Glyph* ret = f->glyphs + index;
    
    return ret;
}


//~ NOTE(Momo): Asset functions
Font*
Assets_GetFont(Assets* a, Font_ID font_id) {
    ASSERT(font_id < FONT_COUNT);
    return a->fonts + font_id;
}

Texture*
Assets_GetTexture(Assets* a, Texture_ID texture_id) {
    ASSERT(texture_id < TEXTURE_COUNT);
    return a->textures + texture_id;
}

Image*
Assets_GetImage(Assets* a, Image_ID image_id) {
    ASSERT(image_id < IMAGE_COUNT);
    return a->images + image_id;
}

Anime*
Assets_GetAnime(Assets* a, Anime_ID anime_id) {
    ASSERT(anime_id < ANIME_COUNT);
    return a->animes + anime_id;
}


Msg*
Assets_GetMsg(Assets* a, Msg_ID msg_id) {
    ASSERT(msg_id < MSG_COUNT);
    return a->msgs + msg_id;
}


Sound*
Assets_GetSound(Assets* a, Sound_ID sound_id) {
    ASSERT(sound_id < SOUND_COUNT);
    return a->sounds + sound_id;
}


// TODO: Maybe we should just pass in ID instead of pointer.
// Should be waaaay cleaner
quad2f
Assets_GetAtlasUV(Assets* a, Image* image) 
{
    auto texture = a->textures[image->texture_id];
    aabb2u texture_aabb = aabb2u_Create(0, 0, texture.width, texture.height);
    aabb2f normalized_aabb = Ratio(image->box, texture_aabb);
    return Aabb2f_To_Quad2f(normalized_aabb);
}

static inline b8
Assets__CheckSignature(void* memory, String signature) {
    u8* memory_u8 = (u8*)memory;
    for (u32 I = 0; I < signature.count; ++I) {
        if (memory_u8[I] != signature.data[I]) {
            return false;
        }
    }
    return true; 
}


// TODO: extract this to Platform_File_Handle or something other API?
static inline void*
Assets__ReadBlock(Platform_File_Handle* file,
                  Arena* arena,
                  u32* file_offset,
                  u32 block_size,
                  u8 block_alignment)
{
    void* ret = Arena_Push_Block(arena, block_size, block_alignment);
    if(!ret) {
        return nullptr; 
    }
    g_platform->read_file(file,
                          (*file_offset),
                          block_size,
                          ret);
    (*file_offset) += block_size;
    if (file->error) {
        g_platform->log_file_error(file);
        return nullptr;
    }
    return ret;
}

template<typename T>
static inline T*
Assets__ReadStruct(Platform_File_Handle* file, Arena* arena, u32* file_offset) {
    return (T*)Assets__ReadBlock(file, arena, file_offset, sizeof(T), alignof(T));
}

b8
Assets_Init(Assets* a, Arena* arena) 
{
    g_platform->clear_textures();
    
    a->texture_count = TEXTURE_COUNT;
    a->textures = Arena_PushArray<Texture>(arena, TEXTURE_COUNT);
    
    a->image_count = IMAGE_COUNT;
    a->images = Arena_PushArray<Image>(arena, IMAGE_COUNT);
    
    a->font_count = FONT_COUNT;
    a->fonts = Arena_PushArray<Font>(arena, FONT_COUNT);
    
    a->anime_count = ANIME_COUNT;
    a->animes = Arena_PushArray<Anime>(arena, ANIME_COUNT);
    
    a->msg_count = MSG_COUNT;
    a->msgs = Arena_PushArray<Msg>(arena, MSG_COUNT);
    
    a->sound_count = SOUND_COUNT;
    a->sounds = Arena_PushArray<Sound>(arena, SOUND_COUNT);
    
    Platform_File_Handle asset_file = g_platform->open_asset_file();
    if (asset_file.error) {
        g_platform->log_file_error(&asset_file);
        return false;
    }
    defer { g_platform->close_file(&asset_file); }; 
    
    u32 cur_file_offset = 0;
    u32 file_entry_count = 0;
    
    // Check file signaure
    {        
        defer { Arena_Clear(g_scratch); };
        
        String signature = String_Create(GAME_ASSET_FILE_SIGNATURE);
        
        void* read_sig = Assets__ReadBlock(&asset_file,
                                           g_scratch,
                                           &cur_file_offset,
                                           (u32)signature.count,
                                           1);
        if (read_sig == nullptr) {
            g_log("[Assets] Cannot read signature\n");
            return false;
        }
        
        if (!Assets__CheckSignature(read_sig, signature)) {
            g_log("[Assets] Wrong asset signature\n");
            return false;
        }
        
        // Get File Entry
        u32* file_entry_count_ptr = Assets__ReadStruct<u32>(&asset_file,
                                                            g_scratch,
                                                            &cur_file_offset);
        if (file_entry_count_ptr == nullptr) {
            g_log("[Assets] Cannot get file entry count\n");
            return false;
        }
        file_entry_count = *file_entry_count_ptr;
    }
    
    
    for (u32 i = 0; i < file_entry_count; ++i) 
    {
        
        // NOTE(Momo): Read header
        Asset_File_Entry file_entry = {};
        {
            defer { Arena_Clear(g_scratch); };
            
            auto* file_entry_ptr = Assets__ReadStruct<Asset_File_Entry>(&asset_file,
                                                                        g_scratch,
                                                                        &cur_file_offset);
            if (file_entry_ptr == nullptr) {
                g_log("[Assets] Cannot get file entry\n");
                return false;
            }
            file_entry = *file_entry_ptr;
        }
        
        switch(file_entry.type) {
            case ASSET_TYPE_TEXTURE: {
                defer { Arena_Clear(g_scratch); };
                
                auto* file_texture = Assets__ReadStruct<Asset_File_Texture>(&asset_file,
                                                                            g_scratch,
                                                                            &cur_file_offset);              
                if (file_texture == nullptr) {
                    g_log("[Assets] Error getting texture\n");
                    return false;
                }
                auto* texture = a->textures + file_texture->id;
                texture->width = file_texture->width;
                texture->height = file_texture->height;
                texture->channels = file_texture->channels;
                u32 texture_size = texture->width * 
                    texture->height * 
                    texture->channels;
                
                texture->data = (u8*)Assets__ReadBlock(&asset_file, 
                                                       arena, 
                                                       &cur_file_offset,
                                                       texture_size,
                                                       1);
                if (texture->data == nullptr) {
                    g_log("[Assets] Error getting texture pixels\n");
                    return false;
                }
                texture->handle = g_platform->add_texture(file_texture->width, 
                                                          file_texture->height,
                                                          texture->data);
                if (!texture->handle.success) {
                    g_log("[Assets] Cannot add assets!");
                    return false;
                }
            } break;
            case ASSET_TYPE_IMAGE: { 
                defer { Arena_Clear(g_scratch); };
                
                auto* file_image = 
                    Assets__ReadStruct<Asset_File_Image>(&asset_file, 
                                                         g_scratch,
                                                         &cur_file_offset);              
                
                if (file_image == nullptr) {
                    g_log("[Assets] Error getting image\n");
                    return false;
                }
                
                auto* image = Assets_GetImage(a, file_image->id);
                image->box = file_image->box;
                image->texture_id = file_image->texture_id;
            } break;
            case ASSET_TYPE_FONT: {
                defer { Arena_Clear(g_scratch); };
                
                auto* file_font = Assets__ReadStruct<Asset_File_Font>(&asset_file,
                                                                      g_scratch,
                                                                      &cur_file_offset);
                
                if (file_font == nullptr) {
                    g_log("[Assets] Error getting font\n");
                    return false;
                }
                
                auto* font = a->fonts + file_font->id;
                font->line_gap = file_font->line_gap;
                font->ascent = file_font->ascent;
                font->descent = file_font->descent;
            } break;
            case ASSET_TYPE_FONT_GLYPH: {
                defer { Arena_Clear(g_scratch); };
                
                auto* file_font_glyph = Assets__ReadStruct<Asset_File_Font_Glyph>(&asset_file,
                                                                                  g_scratch,
                                                                                  &cur_file_offset);
                
                
                if (file_font_glyph == nullptr) {
                    g_log("[Assets] Error getting font glyph\n");
                    return false;
                }
                
                Font* font = Assets_GetFont(a, file_font_glyph->font_id);
                Font_Glyph* glyph = Font_GetGlyph(font, file_font_glyph->codepoint);
                
                glyph->image_id = file_font_glyph->image_id;
                glyph->advance = file_font_glyph->advance;
                glyph->left_bearing = file_font_glyph->left_bearing;
                glyph->box = file_font_glyph->box;
            } break;
            case ASSET_TYPE_FONT_KERNING: {
                defer { Arena_Clear(g_scratch); };
                
                auto* file_font_kerning = Assets__ReadStruct<Asset_File_Font_Kerning>(&asset_file,
                                                                                      g_scratch,
                                                                                      &cur_file_offset);
                if (file_font_kerning == nullptr) {
                    g_log("[Assets] Error getting font kerning\n");
                    return false;
                }
                
                Font* font = Assets_GetFont(a, file_font_kerning->font_id);
                Font_SetKerning(font, file_font_kerning->codepoint_a, 
                                file_font_kerning->codepoint_b,
                                file_font_kerning->kerning);
                
            } break;
            case ASSET_TYPE_SOUND: {
                defer { Arena_Clear(g_scratch); };
                
                auto* file = Assets__ReadStruct<Asset_File_Sound>(&asset_file,
                                                                  g_scratch,
                                                                  &cur_file_offset);
                
                if (file == nullptr) { 
                    g_log("[Assets] Error getitng sound\n"); 
                    return false; 
                }
                
                Sound* sound = Assets_GetSound(a, file->id);
                sound->data_count = file->data_count;
                
                sound->data = (s16*)Assets__ReadBlock(&asset_file,
                                                      arena, 
                                                      &cur_file_offset,
                                                      sizeof(s16) * sound->data_count,
                                                      1);
                
            } break;
            case ASSET_TYPE_MSG: {
                defer { Arena_Clear(g_scratch); };
                auto* file = Assets__ReadStruct<Asset_File_Msg>(&asset_file,
                                                                g_scratch,
                                                                &cur_file_offset);
                if (file == nullptr) { 
                    g_log("[Assets] Msg is null"); 
                    return false; 
                }
                
                
                Msg* msg = Assets_GetMsg(a, file->id);
                msg->count = file->count;
                
                msg->data = (u8*)Assets__ReadBlock(&asset_file,
                                                   arena, 
                                                   &cur_file_offset,
                                                   sizeof(u8) * msg->count,
                                                   1);
                
                
                
            } break;
            case ASSET_TYPE_ANIME: {
                defer { Arena_Clear(g_scratch); };
                auto* file = Assets__ReadStruct<Asset_File_Anime>(&asset_file,
                                                                  g_scratch,
                                                                  &cur_file_offset);
                
                if (file == nullptr) { 
                    g_log("[Assets] Anime is null"); 
                    return false; 
                }
                
                
                Anime* anime = Assets_GetAnime(a, file->id);
                anime->frame_count = file->frame_count;
                
                anime->frames = (Image_ID*)Assets__ReadBlock(&asset_file,
                                                             arena, 
                                                             &cur_file_offset,
                                                             sizeof(Image_ID) * anime->frame_count,
                                                             1);
                
            } break;
            default: {
                return false;
            } break;
            
            
        }
    }
    
    
    return true;
    
}
