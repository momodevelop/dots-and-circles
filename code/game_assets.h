#ifndef __GAME_ASSETS_H__
#define __GAME_ASSETS_H__

// NOTE(Momo): Asset types
struct texture {
    u32 Width, Height, Channels;
    renderer_texture_handle Handle;
};

struct anime {
    image_id *Frames;
    u32 FrameCount;
};

struct image {
    MM_Aabb2u Aabb;
    texture_id TextureId;
};

struct font_glyph {
    image_id ImageId;
    MM_Aabb2f Box;
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
static inline MM_Quad2f
Assets_GetAtlasUV(assets* Assets, 
                  image* Image) 
{
    auto Texture = Assets->Textures[Image->TextureId];
    MM_Aabb2u TextureAabb = MM_Aabb2u_Create(0, 0, Texture.Width, Texture.Height);
    MM_Aabb2f NormalizedAabb = MM_Aabb2u_Ratio(Image->Aabb, TextureAabb);
    return MM_Aabb2f_ToQuad2f(NormalizedAabb);
}

static inline b32
Assets_CheckSignature(void* Memory, u8_cstr Signature) {
    u8* MemoryU8 = (u8*)Memory;
    for (u32 I = 0; I < Signature.count; ++I) {
        if (MemoryU8[I] != Signature.data[I]) {
            return False;
        }
    }
    return True; 
}

static inline void*
Assets_ReadBlock(platform_file_handle* File,
                 platform_api* Platform,
                 MM_Arena* Arena,
                 u32* FileOffset,
                 u32 BlockSize,
                 u8 BlockAlignment)
{
    void* Ret = MM_Arena_PushBlock(Arena, BlockSize, BlockAlignment);
    if(!Ret) {
        return Null; 
    }
    Platform->ReadFileFp(File,
                         (*FileOffset),
                         BlockSize,
                         Ret);
    (*FileOffset) += BlockSize;
    if (File->Error) {
        Platform->LogFileErrorFp(File);
        return Null;
    }
    return Ret;
}
#define Assets_ReadStruct(Type, File, Platform, Arena, FileOffset) \
(Type*)Assets_ReadBlock(File, Platform, Arena, FileOffset, sizeof(Type), alignof(Type))

static inline b32
Assets_Init(assets* Assets,
            MM_Arena* Arena,
            platform_api* Platform) 
{
    Platform->ClearTexturesFp();
    
    Assets->TextureCount = Texture_Count;
    Assets->Textures = MM_Arena_PushArray(texture, Arena, Texture_Count);
    
    Assets->ImageCount = Image_Count;
    Assets->Images = MM_Arena_PushArray(image, Arena, Image_Count);
    
    Assets->FontCount = Font_Count;
    Assets->Fonts = MM_Arena_PushArray(font, Arena, Font_Count);
    
    Assets->AnimeCount = Anime_Count;
    Assets->Animes = MM_Arena_PushArray(anime, Arena, Anime_Count);
    
    Assets->MsgCount = Msg_Count;
    Assets->Msgs = MM_Arena_PushArray(msg, Arena, Msg_Count);
    
    Assets->SoundCount = Sound_Count;
    Assets->Sounds = MM_Arena_PushArray(sound, Arena, Sound_Count);
    
    platform_file_handle AssetFile = Platform->OpenAssetFileFp();
    if (AssetFile.Error) {
        Platform->LogFileErrorFp(&AssetFile);
        return False;
    }
    Defer { Platform->CloseFileFp(&AssetFile); }; 
    
    u32 CurFileOffset = 0;
    u32 FileEntryCount = 0;
    
    // Check file signaure
    {        
        MM_ArenaMark Scratch = MM_Arena_Mark(Arena);
        Defer{ MM_Arena_Revert(&Scratch); };
        
        u8_cstr Signature = {};
        U8CStr_InitFromSiStr(&Signature, Game_AssetFileSignature);
        
        void* ReadSignature = Assets_ReadBlock(&AssetFile,
                                               Platform,
                                               Scratch,
                                               &CurFileOffset,
                                               (u32)Signature.count,
                                               1);
        if (ReadSignature == Null) {
            Platform->LogFp("[Assets] Cannot read signature\n");
            return False;
        }
        
        if (!Assets_CheckSignature(ReadSignature, Signature)) {
            Platform->LogFp("[Assets] Wrong asset signature\n");
            return False;
        }
        
        // Get File Entry
        u32* FileEntryCountPtr = Assets_ReadStruct(u32,
                                                   &AssetFile,
                                                   Platform,
                                                   Scratch,
                                                   &CurFileOffset);
        if (FileEntryCountPtr == Null) {
            Platform->LogFp("[Assets] Cannot get file entry count\n");
            return False;
        }
        FileEntryCount = *FileEntryCountPtr;
    }
    
    
    for (u32 I = 0; I < FileEntryCount; ++I) 
    {
        
        // NOTE(Momo): Read header
        asset_file_entry FileEntry = {};
        {
            MM_ArenaMark Scratch = MM_Arena_Mark(Arena);
            Defer{ MM_Arena_Revert(&Scratch); };
            
            auto* FileEntryPtr = Assets_ReadStruct(asset_file_entry,
                                                   &AssetFile,
                                                   Platform,
                                                   Scratch,
                                                   &CurFileOffset);
            if (FileEntryPtr == Null) {
                Platform->LogFp("[Assets] Cannot get file entry\n");
                return False;
            }
            FileEntry = *FileEntryPtr;
        }
        
        switch(FileEntry.Type) {
            case AssetType_Texture: {
                MM_ArenaMark Scratch = MM_Arena_Mark(Arena);
                Defer{ MM_Arena_Revert(&Scratch); };
                
                auto* FileTexture = Assets_ReadStruct(asset_file_texture,
                                                      &AssetFile, 
                                                      Platform, 
                                                      Scratch,
                                                      &CurFileOffset);              
                if (FileTexture == Null) {
                    Platform->LogFp("[Assets] Error getting texture\n");
                    return False;
                }
                texture* Texture = Assets->Textures + FileTexture->Id;
                Texture->Width = FileTexture->Width;
                Texture->Height = FileTexture->Height;
                Texture->Channels = FileTexture->Channels;
                u32 TextureSize = Texture->Width * 
                    Texture->Height * 
                    Texture->Channels;
                
                void* Pixels = Assets_ReadBlock(&AssetFile, 
                                                Platform,
                                                Scratch, 
                                                &CurFileOffset,
                                                TextureSize,
                                                1);
                if (Pixels == Null) {
                    Platform->LogFp("[Assets] Error getting texture pixels\n");
                    return False;
                }
                Texture->Handle = Platform->AddTextureFp(FileTexture->Width, 
                                                         FileTexture->Height,
                                                         Pixels);
                if (!Texture->Handle.Success) {
                    Platform->LogFp("[Assets] Cannot add assets!");
                    return False;
                }
            } break;
            case AssetType_Image: { 
                MM_ArenaMark Scratch = MM_Arena_Mark(Arena);
                Defer{ MM_Arena_Revert(&Scratch); };
                
                auto* FileImage = 
                    Assets_ReadStruct(asset_file_image,
                                      &AssetFile, 
                                      Platform, 
                                      Scratch,
                                      &CurFileOffset);              
                
                if (FileImage == Null) {
                    Platform->LogFp("[Assets] Error getting image\n");
                    return False;
                }
                
                auto* Image = Assets_GetImage(Assets, FileImage->Id);
                Image->Aabb = FileImage->Aabb;
                Image->TextureId = FileImage->TextureId;
            } break;
            case AssetType_Font: {
                MM_ArenaMark Scratch = MM_Arena_Mark(Arena);
                Defer{ MM_Arena_Revert(&Scratch); };
                
                auto* FileFont = Assets_ReadStruct(asset_file_font,
                                                   &AssetFile,
                                                   Platform,
                                                   Scratch,
                                                   &CurFileOffset);
                
                if (FileFont == Null) {
                    Platform->LogFp("[Assets] Error getting font\n");
                    return False;
                }
                
                auto* Font = Assets->Fonts + FileFont->Id;
                Font->LineGap = FileFont->LineGap;
                Font->Ascent = FileFont->Ascent;
                Font->Descent = FileFont->Descent;
            } break;
            case AssetType_FontGlyph: {
                MM_ArenaMark Scratch = MM_Arena_Mark(Arena);
                Defer{ MM_Arena_Revert(&Scratch); };
                
                auto* FileFontGlyph = Assets_ReadStruct(asset_file_font_glyph,
                                                        &AssetFile,
                                                        Platform,
                                                        Scratch,
                                                        &CurFileOffset);
                
                
                if (FileFontGlyph == Null) {
                    Platform->LogFp("[Assets] Error getting font glyph\n");
                    return False;
                }
                
                font* Font = Assets_GetFont(Assets, FileFontGlyph->FontId);
                font_glyph* Glyph = Font_GetGlyph(Font, FileFontGlyph->Codepoint);
                
                Glyph->ImageId = FileFontGlyph->ImageId;
                Glyph->Advance = FileFontGlyph->Advance;
                Glyph->LeftBearing = FileFontGlyph->LeftBearing;
                Glyph->Box = FileFontGlyph->Box;
            } break;
            case AssetType_FontKerning: {
                MM_ArenaMark Scratch = MM_Arena_Mark(Arena);
                Defer{ MM_Arena_Revert(&Scratch); };
                
                auto* FileFontKerning = Assets_ReadStruct(asset_file_font_kerning,
                                                          &AssetFile,
                                                          Platform,
                                                          Scratch,
                                                          &CurFileOffset);
                if (FileFontKerning == Null) {
                    Platform->LogFp("[Assets] Error getting font kerning\n");
                    return False;
                }
                
                font* Font = Assets_GetFont(Assets, FileFontKerning->FontId);
                Font_SetKerning(Font, 
                                FileFontKerning->CodepointA, 
                                FileFontKerning->CodepointB,
                                FileFontKerning->Kerning);
                
            } break;
            case AssetType_Sound: {
                MM_ArenaMark Scratch = MM_Arena_Mark(Arena);
                
                auto* File = Assets_ReadStruct(asset_file_sound,
                                               &AssetFile,
                                               Platform,
                                               Scratch,
                                               &CurFileOffset);
                
                if (File == Null) { 
                    Platform->LogFp("[Assets] Error getitng sound\n"); 
                    MM_Arena_Revert(&Scratch); 
                    return False; 
                }
                
                sound* Sound = Assets_GetSound(Assets, File->Id);
                Sound->DataCount = File->DataCount;
                
                // NOTE(Momo): we have to revert here so that our arena's 
                // memory does not screw up. A bit janky but hey.
                MM_Arena_Revert(&Scratch); 
                
                Sound->Data = (s16*)
                    Assets_ReadBlock(&AssetFile,
                                     Platform, 
                                     Arena, 
                                     &CurFileOffset,
                                     sizeof(s16) * Sound->DataCount,
                                     1);
                
            } break;
            case AssetType_Message: {
                MM_ArenaMark Scratch = MM_Arena_Mark(Arena);
                
                auto* File = Assets_ReadStruct(asset_file_msg,
                                               &AssetFile,
                                               Platform,
                                               Scratch,
                                               &CurFileOffset);
                if (File == Null) { 
                    Platform->LogFp("[Assets] Msg is null"); 
                    MM_Arena_Revert(&Scratch); 
                    return False; 
                }
                
                
                msg* Msg = Assets_GetMsg(Assets, File->Id);
                Msg->count = File->Count;
                
                
                // NOTE(Momo): we have to revert here so that our arena's 
                // memory does not screw up. A bit janky but hey.
                MM_Arena_Revert(&Scratch); 
                
                Msg->data = (u8*)
                    Assets_ReadBlock(&AssetFile,
                                     Platform, 
                                     Arena, 
                                     &CurFileOffset,
                                     sizeof(u8) * Msg->count,
                                     1);
                
                
                
            } break;
            case AssetType_Anime: {
                MM_ArenaMark Scratch = MM_Arena_Mark(Arena);
                
                auto* File = Assets_ReadStruct(asset_file_anime,
                                               &AssetFile,
                                               Platform,
                                               Scratch,
                                               &CurFileOffset);
                
                if (File == Null) { 
                    Platform->LogFp("[Assets] Anime is null"); 
                    MM_Arena_Revert(&Scratch); 
                    return False; 
                }
                
                anime* Anime = Assets_GetAnime(Assets, File->Id);
                Anime->FrameCount = File->FrameCount;
                
                // NOTE(Momo): we have to revert here so that our arena's 
                // memory does not screw up. A bit janky but hey.
                MM_Arena_Revert(&Scratch); 
                
                Anime->Frames = (image_id*)
                    Assets_ReadBlock(&AssetFile,
                                     Platform, 
                                     Arena, 
                                     &CurFileOffset,
                                     sizeof(image_id) * Anime->FrameCount,
                                     1);
                
            } break;
            default: {
                return False;
            } break;
            
            
        }
    }
    
    
    return True;
    
#undef CheckFile
#undef CheckPtr
}

#endif  
