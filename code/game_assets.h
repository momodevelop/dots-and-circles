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

typedef u8_cstr msg;


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
    aabb2u TextureAabb = Aabb2u_Create(0, 0, Texture.Width, Texture.Height);
    aabb2f NormalizedAabb = Aabb2u_Ratio(Image->Aabb, TextureAabb);
    return Aabb2f_To_Quad2f(NormalizedAabb);
}

static inline b8
Assets_CheckSignature(void* Memory, u8_cstr Signature) {
    u8* MemoryU8 = (u8*)Memory;
    for (u32 I = 0; I < Signature.Count; ++I) {
        if (MemoryU8[I] != Signature.Data[I]) {
            return false;
        }
    }
    return true; 
}

static inline void*
Assets_ReadBlock(platform_file_handle* File,
                 arena* Arena,
                 u32* FileOffset,
                 u32 BlockSize,
                 u8 BlockAlignment)
{
    void* Ret = Arena_PushBlock(Arena, BlockSize, BlockAlignment);
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
            arena* Arena) 
{
    G_Platform->ClearTexturesFp();
    
    Assets->TextureCount = Texture_Count;
    Assets->Textures = Arena_PushArray(texture, Arena, Texture_Count);
    
    Assets->ImageCount = Image_Count;
    Assets->Images = Arena_PushArray(image, Arena, Image_Count);
    
    Assets->FontCount = Font_Count;
    Assets->Fonts = Arena_PushArray(font, Arena, Font_Count);
    
    Assets->AnimeCount = Anime_Count;
    Assets->Animes = Arena_PushArray(anime, Arena, Anime_Count);
    
    Assets->MsgCount = Msg_Count;
    Assets->Msgs = Arena_PushArray(msg, Arena, Msg_Count);
    
    Assets->SoundCount = Sound_Count;
    Assets->Sounds = Arena_PushArray(sound, Arena, Sound_Count);
    
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
        Defer { Arena_Clear(G_Scratch); };
        
        u8_cstr Signature = {};
        U8CStr_InitFromSiStr(&Signature, Game_AssetFileSignature);
        
        void* ReadSignature = Assets_ReadBlock(&AssetFile,
                                               G_Scratch,
                                               &CurFileOffset,
                                               (u32)Signature.Count,
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
            Defer { Arena_Clear(G_Scratch); };
            
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
                Defer { Arena_Clear(G_Scratch); };
                
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
                                                      Arena, 
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
                Defer { Arena_Clear(G_Scratch); };
                
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
                Defer { Arena_Clear(G_Scratch); };
                
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
                Defer { Arena_Clear(G_Scratch); };
                
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
                Defer { Arena_Clear(G_Scratch); };
                
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
                Defer { Arena_Clear(G_Scratch); };
                
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
                                     Arena, 
                                     &CurFileOffset,
                                     sizeof(s16) * Sound->DataCount,
                                     1);
                
            } break;
            case AssetType_Message: {
                Defer { Arena_Clear(G_Scratch); };
                auto* File = Assets_ReadStruct(asset_file_msg,
                                               &AssetFile,
                                               G_Scratch,
                                               &CurFileOffset);
                if (File == nullptr) { 
                    G_Log("[Assets] Msg is null"); 
                    return false; 
                }
                
                
                msg* Msg = Assets_GetMsg(Assets, File->Id);
                Msg->Count = File->Count;
                
                Msg->Data = (u8*)
                    Assets_ReadBlock(&AssetFile,
                                     Arena, 
                                     &CurFileOffset,
                                     sizeof(u8) * Msg->Count,
                                     1);
                
                
                
            } break;
            case AssetType_Anime: {
                Defer { Arena_Clear(G_Scratch); };
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
                                     Arena, 
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
