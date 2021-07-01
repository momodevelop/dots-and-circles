#ifndef __GAME_ASSETS_H__
#define __GAME_ASSETS_H__

// NOTE(Momo): Asset types
struct Texture {
    u32 width, height, channels;
    u8* data; // RGBA format
    renderer_texture_handle handle;
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
    Font_Glyph glyph[FONT_GLYPH_COUNT];
    u32 kerning[FONT_GLYPH_COUNT][FONT_GLYPH_COUNT];
    
    void set_kerning(u32 codepoint_a, u32 codepoint_b, u32 kerning);
    u32 get_kerning(u32 codepoint_a, u32 codepoint_b);
    f32 height();
    Font_Glyph get_glyph(u32 codepoint);
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
    
    
    b8 init(Arena* arena);
    Font* get_font(Font_ID font_id);
    Texture* get_texture(Texture_ID texture_id);
    Image* get_image(Image_ID image_id);
    Anime* get_anime(Anime_ID anime_id);
    Sound* get_sound(Sound_ID sound_id);
    Msg* get_msg(Msg_ID msg_id);
    
    
    
};

//~ NOTE(Momo): Font functions
f32 
Font::height() {
    return ABS(ascent) + ABS(descent);
}

u32
Font::get_kerning(u32 codepoint_a, u32 codepoint_b) {
    ASSERT(codepoint_a >= FONT_GLYPH_CODEPOINT_START);
    ASSERT(codepoint_a <= FONT_GLYPH_CODEPOINT_END);
    
    ASSERT(codepoint_b >= FONT_GLYPH_CODEPOINT_START);
    ASSERT(codepoint_b <= FONT_GLYPH_CODEPOINT_END);
    u32 index_a = codepoint_a - FONT_GLYPH_CODEPOINT_START;
    u32 index_b = codepoint_b - FONT_GLYPH_CODEPOINT_START;
    u32 ret = kernings[index_a][index_b];
    
    return ret;
}

void
Font::set_kerning(u32 codepoint_a, u32 codepoint_b, u32 kerning) {
    ASSERT(codepoint_a >= FONT_GLYPH_CODEPOINT_START);
    ASSERT(codepoint_a <= FONT_GLYPH_CODEPOINT_END);
    
    ASSERT(codepoint_b >= FONT_GLYPH_CODEPOINT_START);
    ASSERT(codepoint_b <= FONT_GLYPH_CODEPOINT_END);
    
    u32 index_a = codepoint_a - FONT_GLYPH_CODEPOINT_START;
    u32 index_b = codepoint_b - FONT_GLYPH_CODEPOINT_START;
    Font->kernings[index_a][index_b] = kerning;
}

Font_Glyph* 
Font::get_glyph(u32 codepoint) {
    ASSERT(codepoint >= FONT_GLYPH_CODEPOINT_START);
    ASSERT(codepoint <= FONT_GLYPH_CODEPOINT_END);
    u32 index = codepoint - FONT_GLYPH_CODEPOINT_START;
    Font_Glyph* ret = glyph + index;
    
    return ret;
}

//~ NOTE(Momo): Asset functions
Font*
Assets::get_font(Font_ID font_id) {
    ASSERT(font_id < FONT_COUNT);
    return fonts + font_id;
}

Texture*
Assets::get_texture(Texture_ID texture_id) {
    ASSERT(texture_id < TEXTURE_COUNT);
    return textures + texture_id;
}

Image*
Assets::get_image(Image_ID image_id) {
    ASSERT(image_id < IMAGE_COUNT);
    return images + image_id;
}

Anime*
Assets::get_anime(Anime_ID anime_id) {
    ASSERT(anime_id < ANIME_COUNT);
    return animes + anime_id;
}


Msg*
Assets::get_msg(Msg_ID msg_id) {
    ASSERT(msg_id < MSG_COUNT);
    return msgs + msg_id;
}


Sound*
Assets::get_sound(Sound_ID sound_id) {
    ASSERT(sound_id < SOUND_COUNT);
    return sounds + sound_id;
}


// TODO: Maybe we should just pass in ID instead of pointer.
// Should be waaaay cleaner
static inline quad2f
Assets_GetAtlasUV(assets* Assets, 
                  image* Image) 
{
    auto Texture = Assets->Textures[Image->TextureId];
    aabb2u TextureAabb = aabb2u::create(0, 0, Texture.Width, Texture.Height);
    aabb2f NormalizedAabb = ratio(Image->Aabb, TextureAabb);
    return Aabb2f_To_Quad2f(NormalizedAabb);
}

static inline b8
check_asset_signature(void* memory, String signature) {
    u8* memory_u8 = (u8*)memory;
    for (u32 I = 0; I < signature.count; ++I) {
        if (memory_u8[I] != signature.data[I]) {
            return false;
        }
    }
    return true; 
}


// TODO: extract this to platform_file_handle or something other API?
static inline void*
read_block(platform_file_handle* File,
           Arena* arena,
           u32* FileOffset,
           u32 BlockSize,
           u8 BlockAlignment)
{
    void* Ret = arena->push_block(BlockSize, BlockAlignment);
    if(!Ret) {
        return nullptr; 
    }
    G_Platform->ReadFileFp(File,
                           (*FileOffset),
                           BlockSize,
                           Ret);
    (*FileOffset) += BlockSize;
    if (File->Error) {
        G_Platform->LogFileErrorFp(File);
        return nullptr;
    }
    return Ret;
}

template<typename T>
static inline T*
read_struct(platform_file_handle* file, Arena* arena, u32* file_offset) {
    read_block(file, arena, file_offset, sizeof(T), alignof(T));
}

b8
Assets::init(Arena* arena) 
{
    G_Platform->ClearTexturesFp();
    
    this->texture_count = TEXTURE_COUNT;
    this->textures = arena->push_array<Texture>(TEXTURE_COUNT);
    
    this->image_count = IMAGE_COUNT;
    this->images = arena->push_array<Image>(IMAGE_COUNT);
    
    this->font_count = FONT_COUNT;
    this->fonts = arena->push_array<Font>(FONT_COUNT);
    
    this->anime_count = ANIME_COUNT;
    this->animes = arena->push_array<Anime>(ANIME_COUNT);
    
    this->msg_count = MSG_COUNT;
    this->msgs = arena->push_array<Msg>(MSG_COUNT);
    
    this->sound_count = SOUND_COUNT;
    this->sounds = arena->push_array<Sound>(SOUND_COUNT);
    
    platform_file_handle asset_file = G_Platform->OpenAssetFileFp();
    if (asset_file.Error) {
        G_Platform->LogFileErrorFp(&asset_file);
        return false;
    }
    defer { G_Platform->CloseFileFp(&asset_file); }; 
    
    u32 cur_file_offset = 0;
    u32 file_entry_count = 0;
    
    // Check file signaure
    {        
        
        defer { G_Scratch->clear(); };
        
        String signature = {};
        signature.init(Game_AssetFileSignature);
        
        void* read_sig = read_block(&asset_file,
                                    G_Scratch,
                                    &cur_file_offset,
                                    (u32)signature.count,
                                    1);
        if (read_sig == nullptr) {
            G_Log("[Assets] Cannot read signature\n");
            return false;
        }
        
        if (!check_asset_signature(read_sig, signature)) {
            G_Log("[Assets] Wrong asset signature\n");
            return false;
        }
        
        // Get File Entry
        u32* file_entry_count_ptr = read_struct<u32>(&asset_file,
                                                     G_Scratch,
                                                     &cur_file_offset);
        if (file_entry_count_ptr == nullptr) {
            G_Log("[Assets] Cannot get file entry count\n");
            return false;
        }
        file_entry_count = *file_entry_count_ptr;
    }
    
    
    for (u32 I = 0; I < file_entry_count; ++I) 
    {
        
        // NOTE(Momo): Read header
        Asset_File_Entry file_entry = {};
        {
            defer { G_Scratch->clear(); };
            
            auto* file_entry_ptr = read_struct<Asset_File_Entry>(&asset_file,
                                                                 G_Scratch,
                                                                 &cur_file_offset);
            if (file_entry_ptr == nullptr) {
                G_Log("[Assets] Cannot get file entry\n");
                return false;
            }
            file_entry = *file_entry_ptr;
        }
        
        switch(file_entry.type) {
            case ASSET_TYPE_TEXTURE: {
                defer { G_Scratch->clear(); };
                
                auto* FileTexture = read_struct<Asset_File_Texture>(&asset_file,
                                                                    G_Scratch,
                                                                    &cur_file_offset);              
                if (FileTexture == nullptr) {
                    G_Log("[Assets] Error getting texture\n");
                    return false;
                }
                auto* Texture = this->Textures + FileTexture->Id;
                Texture->Width = FileTexture->Width;
                Texture->Height = FileTexture->Height;
                Texture->Channels = FileTexture->Channels;
                u32 TextureSize = Texture->Width * 
                    Texture->Height * 
                    Texture->Channels;
                
                Texture->Data = (u8*)Assets_ReadBlock(&asset_file, 
                                                      arena, 
                                                      &cur_file_offset,
                                                      TextureSize,
                                                      1);
                if (Texture->Data == nullptr) {
                    G_Log("[Assets] Error getting texture pixels\n");
                    return false;
                }
                Texture->Handle = G_Platform->AddTextureFp(FileTexture->Width, 
                                                           FileTexture->Height,
                                                           Texture->Data);
                if (!Texture->Handle.Success) {
                    G_Log("[Assets] Cannot add assets!");
                    return false;
                }
            } break;
            case ASSET_TYPE_IMAGE: { 
                defer { G_Scratch->clear(); };
                
                auto* FileImage = 
                    read_struct<Asset_File_Image>(&asset_file, 
                                                  G_Scratch,
                                                  &cur_file_offset);              
                
                if (FileImage == nullptr) {
                    G_Log("[Assets] Error getting image\n");
                    return false;
                }
                
                auto* Image = get_image(FileImage->Id);
                Image->Aabb = FileImage->Aabb;
                Image->TextureId = FileImage->TextureId;
            } break;
            case ASSET_TYPE_FONT: {
                defer { G_Scratch->clear(); };
                
                auto* FileFont = read_struct<Asset_File_Font>(&asset_file,
                                                              G_Scratch,
                                                              &cur_file_offset);
                
                if (FileFont == nullptr) {
                    G_Log("[Assets] Error getting font\n");
                    return false;
                }
                
                auto* Font = this->Fonts + FileFont->Id;
                Font->LineGap = FileFont->LineGap;
                Font->Ascent = FileFont->Ascent;
                Font->Descent = FileFont->Descent;
            } break;
            case ASSET_TYPE_FONT_GLYPH: {
                defer { G_Scratch->clear(); };
                
                auto* FileFontGlyph = Assets_ReadStruct(Asset_File_Font_Glyph,
                                                        &asset_file,
                                                        G_Scratch,
                                                        &cur_file_offset);
                
                
                if (FileFontGlyph == nullptr) {
                    G_Log("[Assets] Error getting font glyph\n");
                    return false;
                }
                
                font* Font = Assets_GetFont(Assets, FileFontGlyph->FontId);
                font_glyph* Glyph = Font_GetGlyph(Font, FileFontGlyph->Codepoint);
                
                Glyph->ImageId = FileFontGlyph->ImageId;
                Glyph->Advance = FileFontGlyph->Advance;
                Glyph->LeftBearing = FileFontGlyph->LeftBearing;
                Glyph->Box = FileFontGlyph->Box;
            } break;
            case ASSET_TYPE_FONT_KERNING: {
                defer { G_Scratch->clear(); };
                
                auto* FileFontKerning = Assets_ReadStruct(Asset_File_Font_Kerning,
                                                          &asset_file,
                                                          G_Scratch,
                                                          &cur_file_offset);
                if (FileFontKerning == nullptr) {
                    G_Log("[Assets] Error getting font kerning\n");
                    return false;
                }
                
                font* Font = Assets_GetFont(Assets, FileFontKerning->FontId);
                Font_SetKerning(Font, 
                                FileFontKerning->CodepointA, 
                                FileFontKerning->CodepointB,
                                FileFontKerning->Kerning);
                
            } break;
            case ASSET_TYPE_SOUND: {
                defer { G_Scratch->clear(); };
                
                auto* File = Assets_ReadStruct(Asset_File_Sound,
                                               &asset_file,
                                               G_Scratch,
                                               &cur_file_offset);
                
                if (File == nullptr) { 
                    G_Log("[Assets] Error getitng sound\n"); 
                    return false; 
                }
                
                sound* Sound = Assets_GetSound(Assets, File->Id);
                Sound->DataCount = File->DataCount;
                
                Sound->Data = (s16*)
                    Assets_ReadBlock(&asset_file,
                                     arena, 
                                     &cur_file_offset,
                                     sizeof(s16) * Sound->DataCount,
                                     1);
                
            } break;
            case ASSET_TYPE_MSG: {
                defer { G_Scratch->clear(); };
                auto* File = Assets_ReadStruct(Asset_File_Msg,
                                               &asset_file,
                                               G_Scratch,
                                               &cur_file_offset);
                if (File == nullptr) { 
                    G_Log("[Assets] Msg is null"); 
                    return false; 
                }
                
                
                msg* Msg = Assets_GetMsg(Assets, File->Id);
                Msg->count = File->Count;
                
                Msg->data = (u8*)
                    Assets_ReadBlock(&asset_file,
                                     arena, 
                                     &cur_file_offset,
                                     sizeof(u8) * Msg->count,
                                     1);
                
                
                
            } break;
            case ASSET_TYPE_ANIME: {
                defer { G_Scratch->clear(); };
                auto* File = Assets_ReadStruct(Asset_File_Anime,
                                               &asset_file,
                                               G_Scratch,
                                               &cur_file_offset);
                
                if (File == nullptr) { 
                    G_Log("[Assets] Anime is null"); 
                    return false; 
                }
                
                anime* Anime = Assets_GetAnime(Assets, File->Id);
                Anime->FrameCount = File->FrameCount;
                
                Anime->Frames = (Image_ID*)
                    Assets_ReadBlock(&asset_file,
                                     arena, 
                                     &cur_file_offset,
                                     sizeof(Image_ID) * Anime->FrameCount,
                                     1);
                
            } break;
            default: {
                return false;
            } break;
            
            
        }
    }
    
    
    return true;
    
#undef CheckFile
#undef CheckPtr
}

#endif  
