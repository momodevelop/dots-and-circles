#ifndef __GAME_ASSETS_H__
#define __GAME_ASSETS_H__

// NOTE(Momo): Asset types
struct texture {
    u32 Width, Height, Channels;
    u8* Data; // RGBA format
    renderer_texture_handle Handle;
};

struct anime {
    image_id *Frames;
    u32 FrameCount;
};

struct image {
    aabb2u Aabb;
    texture_id TextureId;
};

struct font_glyph {
    image_id ImageId;
    aabb2f Box;
    f32 Advance;
    f32 LeftBearing;
};

struct font {
    // NOTE(Momo): We cater for a fixed set of codepoints. 
    // ASCII 32 to 126 
    // Worry about sparseness next time.
    f32 LineGap;
    f32 Ascent;
    f32 Descent;
    font_glyph Glyphs[FontGlyph_Count];
    u32 Kernings[FontGlyph_Count][FontGlyph_Count];
};

struct sound {
    s16* Data;
    u32 DataCount;
};

typedef String msg;


struct assets {
    
    texture* Textures;
    u32 TextureCount;
    
    image* Images;
    u32 ImageCount;
    
    font* Fonts;
    u32 FontCount;
    
    anime* Animes;
    u32 AnimeCount;
    
    msg* Msgs;
    u32 MsgCount;
    
    sound* Sounds;
    u32 SoundCount;
};

//~ NOTE(Momo): Font functions
static inline f32 
Font_GetHeight(font* Font) {
    return AbsOf(Font->Ascent) + AbsOf(Font->Descent);
}

static inline u32
Font_GetKerning(font* Font, u32 CodepointA, u32 CodepointB) {
    Assert(CodepointA >= FontGlyph_CodepointStart);
    Assert(CodepointA <= FontGlyph_CodepointEnd);
    
    Assert(CodepointB >= FontGlyph_CodepointStart);
    Assert(CodepointB <= FontGlyph_CodepointEnd);
    u32 IndexA = CodepointA - FontGlyph_CodepointStart;
    u32 IndexB = CodepointB - FontGlyph_CodepointStart;
    u32 Ret = Font->Kernings[IndexA][IndexB];
    
    return Ret;
}

static inline void
Font_SetKerning(font* Font, u32 CodepointA, u32 CodepointB, u32 Kerning) {
    Assert(CodepointA >= FontGlyph_CodepointStart);
    Assert(CodepointA <= FontGlyph_CodepointEnd);
    
    Assert(CodepointB >= FontGlyph_CodepointStart);
    Assert(CodepointB <= FontGlyph_CodepointEnd);
    
    u32 IndexA = CodepointA - FontGlyph_CodepointStart;
    u32 IndexB = CodepointB - FontGlyph_CodepointStart;
    Font->Kernings[IndexA][IndexB] = Kerning;
    
}

static inline font_glyph* 
Font_GetGlyph(font* Font, u32 Codepoint) {
    Assert(Codepoint >= FontGlyph_CodepointStart);
    Assert(Codepoint <= FontGlyph_CodepointEnd);
    u32 Index = Codepoint - FontGlyph_CodepointStart;
    font_glyph* Ret = Font->Glyphs + Index;
    
    return Ret;
}

//~ NOTE(Momo): Asset functions
static inline font*
Assets_GetFont(assets* Assets, font_id FontId) {
    Assert(FontId < Font_Count);
    return Assets->Fonts + FontId;
}

static inline texture*
Assets_GetTexture(assets* Assets, texture_id TextureId) {
    Assert(TextureId < Texture_Count);
    return Assets->Textures + TextureId;
}

static inline image*
Assets_GetImage(assets* Assets, image_id ImageId) {
    Assert(ImageId < Image_Count);
    return Assets->Images + ImageId;
}

static inline anime*
Assets_GetAnime(assets* Assets, anime_id AnimeId) {
    Assert(AnimeId < Anime_Count);
    return Assets->Animes + AnimeId;
}


static inline msg*
Assets_GetMsg(assets* Assets, msg_id MsgId) {
    Assert(MsgId < Msg_Count);
    return Assets->Msgs + MsgId;
}


static inline sound*
Assets_GetSound(assets* Assets, sound_id SoundId) {
    Assert(SoundId < Sound_Count);
    return Assets->Sounds + SoundId;
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
Assets_CheckSignature(void* Memory, String Signature) {
    u8* MemoryU8 = (u8*)Memory;
    for (u32 I = 0; I < Signature.count; ++I) {
        if (MemoryU8[I] != Signature.data[I]) {
            return false;
        }
    }
    return true; 
}

static inline void*
Assets_ReadBlock(platform_file_handle* File,
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
#define Assets_ReadStruct(Type, File, Arena, FileOffset) \
(Type*)Assets_ReadBlock(File, Arena, FileOffset, sizeof(Type), alignof(Type))

static inline b8
Assets_Init(assets* Assets,
            Arena* arena) 
{
    G_Platform->ClearTexturesFp();
    
    Assets->TextureCount = Texture_Count;
    Assets->Textures = arena->push_array<texture>(Texture_Count);
    
    Assets->ImageCount = Image_Count;
    Assets->Images = arena->push_array<image>(Image_Count);
    
    Assets->FontCount = Font_Count;
    Assets->Fonts = arena->push_array<font>(Font_Count);
    
    Assets->AnimeCount = Anime_Count;
    Assets->Animes = arena->push_array<anime>(Anime_Count);
    
    Assets->MsgCount = Msg_Count;
    Assets->Msgs = arena->push_array<msg>(Msg_Count);
    
    Assets->SoundCount = Sound_Count;
    Assets->Sounds = arena->push_array<sound>(Sound_Count);
    
    platform_file_handle AssetFile = G_Platform->OpenAssetFileFp();
    if (AssetFile.Error) {
        G_Platform->LogFileErrorFp(&AssetFile);
        return false;
    }
    Defer { G_Platform->CloseFileFp(&AssetFile); }; 
    
    u32 CurFileOffset = 0;
    u32 FileEntryCount = 0;
    
    // Check file signaure
    {        
        
        Defer { G_Scratch->clear(); };
        
        String Signature = {};
        init(&Signature, Game_AssetFileSignature);
        
        void* ReadSignature = Assets_ReadBlock(&AssetFile,
                                               G_Scratch,
                                               &CurFileOffset,
                                               (u32)Signature.count,
                                               1);
        if (ReadSignature == nullptr) {
            G_Log("[Assets] Cannot read signature\n");
            return false;
        }
        
        if (!Assets_CheckSignature(ReadSignature, Signature)) {
            G_Log("[Assets] Wrong asset signature\n");
            return false;
        }
        
        // Get File Entry
        u32* FileEntryCountPtr = Assets_ReadStruct(u32,
                                                   &AssetFile,
                                                   G_Scratch,
                                                   &CurFileOffset);
        if (FileEntryCountPtr == nullptr) {
            G_Log("[Assets] Cannot get file entry count\n");
            return false;
        }
        FileEntryCount = *FileEntryCountPtr;
    }
    
    
    for (u32 I = 0; I < FileEntryCount; ++I) 
    {
        
        // NOTE(Momo): Read header
        asset_file_entry FileEntry = {};
        {
            Defer { G_Scratch->clear(); };
            
            auto* FileEntryPtr = Assets_ReadStruct(asset_file_entry,
                                                   &AssetFile,
                                                   G_Scratch,
                                                   &CurFileOffset);
            if (FileEntryPtr == nullptr) {
                G_Log("[Assets] Cannot get file entry\n");
                return false;
            }
            FileEntry = *FileEntryPtr;
        }
        
        switch(FileEntry.Type) {
            case AssetType_Texture: {
                Defer { G_Scratch->clear(); };
                
                auto* FileTexture = Assets_ReadStruct(asset_file_texture,
                                                      &AssetFile,
                                                      G_Scratch,
                                                      &CurFileOffset);              
                if (FileTexture == nullptr) {
                    G_Log("[Assets] Error getting texture\n");
                    return false;
                }
                texture* Texture = Assets->Textures + FileTexture->Id;
                Texture->Width = FileTexture->Width;
                Texture->Height = FileTexture->Height;
                Texture->Channels = FileTexture->Channels;
                u32 TextureSize = Texture->Width * 
                    Texture->Height * 
                    Texture->Channels;
                
                Texture->Data = (u8*)Assets_ReadBlock(&AssetFile, 
                                                      arena, 
                                                      &CurFileOffset,
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
            case AssetType_Image: { 
                Defer { G_Scratch->clear(); };
                
                auto* FileImage = 
                    Assets_ReadStruct(asset_file_image,
                                      &AssetFile, 
                                      G_Scratch,
                                      &CurFileOffset);              
                
                if (FileImage == nullptr) {
                    G_Log("[Assets] Error getting image\n");
                    return false;
                }
                
                auto* Image = Assets_GetImage(Assets, FileImage->Id);
                Image->Aabb = FileImage->Aabb;
                Image->TextureId = FileImage->TextureId;
            } break;
            case AssetType_Font: {
                Defer { G_Scratch->clear(); };
                
                auto* FileFont = Assets_ReadStruct(asset_file_font,
                                                   &AssetFile,
                                                   G_Scratch,
                                                   &CurFileOffset);
                
                if (FileFont == nullptr) {
                    G_Log("[Assets] Error getting font\n");
                    return false;
                }
                
                auto* Font = Assets->Fonts + FileFont->Id;
                Font->LineGap = FileFont->LineGap;
                Font->Ascent = FileFont->Ascent;
                Font->Descent = FileFont->Descent;
            } break;
            case AssetType_FontGlyph: {
                Defer { G_Scratch->clear(); };
                
                auto* FileFontGlyph = Assets_ReadStruct(asset_file_font_glyph,
                                                        &AssetFile,
                                                        G_Scratch,
                                                        &CurFileOffset);
                
                
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
            case AssetType_FontKerning: {
                Defer { G_Scratch->clear(); };
                
                auto* FileFontKerning = Assets_ReadStruct(asset_file_font_kerning,
                                                          &AssetFile,
                                                          G_Scratch,
                                                          &CurFileOffset);
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
            case AssetType_Sound: {
                Defer { G_Scratch->clear(); };
                
                auto* File = Assets_ReadStruct(asset_file_sound,
                                               &AssetFile,
                                               G_Scratch,
                                               &CurFileOffset);
                
                if (File == nullptr) { 
                    G_Log("[Assets] Error getitng sound\n"); 
                    return false; 
                }
                
                sound* Sound = Assets_GetSound(Assets, File->Id);
                Sound->DataCount = File->DataCount;
                
                Sound->Data = (s16*)
                    Assets_ReadBlock(&AssetFile,
                                     arena, 
                                     &CurFileOffset,
                                     sizeof(s16) * Sound->DataCount,
                                     1);
                
            } break;
            case AssetType_Message: {
                Defer { G_Scratch->clear(); };
                auto* File = Assets_ReadStruct(asset_file_msg,
                                               &AssetFile,
                                               G_Scratch,
                                               &CurFileOffset);
                if (File == nullptr) { 
                    G_Log("[Assets] Msg is null"); 
                    return false; 
                }
                
                
                msg* Msg = Assets_GetMsg(Assets, File->Id);
                Msg->count = File->Count;
                
                Msg->data = (u8*)
                    Assets_ReadBlock(&AssetFile,
                                     arena, 
                                     &CurFileOffset,
                                     sizeof(u8) * Msg->count,
                                     1);
                
                
                
            } break;
            case AssetType_Anime: {
                Defer { G_Scratch->clear(); };
                auto* File = Assets_ReadStruct(asset_file_anime,
                                               &AssetFile,
                                               G_Scratch,
                                               &CurFileOffset);
                
                if (File == nullptr) { 
                    G_Log("[Assets] Anime is null"); 
                    return false; 
                }
                
                anime* Anime = Assets_GetAnime(Assets, File->Id);
                Anime->FrameCount = File->FrameCount;
                
                Anime->Frames = (image_id*)
                    Assets_ReadBlock(&AssetFile,
                                     arena, 
                                     &CurFileOffset,
                                     sizeof(image_id) * Anime->FrameCount,
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
