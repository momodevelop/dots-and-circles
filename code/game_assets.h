#ifndef __GAME_ASSETS_H__
#define __GAME_ASSETS_H__

// NOTE(Momo): Asset types
struct texture {
    u32 Width, Height, Channels;
    renderer_texture_handle Handle;
};

struct anime {
    atlas_aabb_id *Frames;
    u32 FrameCount;
};

struct atlas_aabb {
    aabb2u Aabb;
    texture_id TextureId;
};


struct font_glyph {
    aabb2u AtlasAabb;
    aabb2f Box; 
    texture_id TextureId;
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

struct assets {
    texture* Textures;
    u32 TextureCount;
    
    atlas_aabb* AtlasAabbs;
    u32 AtlasAabbCount;
    
    font* Fonts;
    u32 FontCount;
    
    anime* Animes;
    u32 AnimeCount;
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

static inline atlas_aabb*
Assets_GetAtlasAabb(assets* Assets, atlas_aabb_id AtlasAabbId) {
    Assert(AtlasAabbId < AtlasAabb_Count);
    return Assets->AtlasAabbs + AtlasAabbId;
}

static inline anime*
Assets_GetAnime(assets* Assets, anime_id AnimeId) {
    Assert(AnimeId < Anime_Count);
    return Assets->Animes + AnimeId;
}



// TODO: Maybe we should just pass in ID instead of pointer.
// Should be waaaay cleaner
static inline quad2f
GetAtlasUV(assets* Assets, 
           atlas_aabb* AtlasAabb) 
{
    auto Texture = Assets->Textures[AtlasAabb->TextureId];
    aabb2u TextureAabb = Aabb2u_Create(0, 0, Texture.Width, Texture.Height);
    aabb2f NormalizedAabb = Aabb2u_Ratio(AtlasAabb->Aabb, TextureAabb);
    return Aabb2f_To_Quad2f(NormalizedAabb);
}

static inline quad2f
GetAtlasUV(assets* Assets, 
           font_glyph* Glyph) {
    auto Texture = Assets->Textures[Glyph->TextureId];
    aabb2u TextureAabb = Aabb2u_Create(0, 0, Texture.Width, Texture.Height);
    aabb2f NormalizedAabb = Aabb2u_Ratio(Glyph->AtlasAabb, TextureAabb);
    return Aabb2f_To_Quad2f(NormalizedAabb);
}


static inline b32
Assets_CheckSignature(void* Memory, u8_cstr Signature) {
    u8* MemoryU8 = (u8*)Memory;
    for (u32 I = 0; I < Signature.Count; ++I) {
        if (MemoryU8[I] != Signature.Data[I]) {
            return False;
        }
    }
    return True; 
}

// TODO: Maybe we create some kind of struct for reading files?
static inline void*
Assets_ReadBlock(platform_file_handle* File,
                 platform_api* Platform,
                 arena* Arena,
                 u32* FileOffset,
                 u32 BlockSize,
                 u8 BlockAlignment)
{
    void* Ret = Arena_PushBlock(Arena, BlockSize, BlockAlignment);
    if(!Ret) {
        return nullptr; 
    }
    Platform->ReadFileFp(File,
                         (*FileOffset),
                         BlockSize,
                         Ret);
    (*FileOffset) += BlockSize;
    return Ret;
}
#define Assets_ReadStruct(Type, File, Platform, Arena, FileOffset) \
(Type*)Assets_ReadBlock(File, Platform, Arena, FileOffset, sizeof(Type), alignof(Type))

static inline b32
Assets_Create(assets* Assets,
              arena* Arena,
              platform_api* Platform) 
{
#define CheckFile(Handle) \
if (Handle.Error) { \
Platform->LogFileErrorFp(&Handle); \
return False; \
}
    
#define CheckPtr(Ptr) \
if ((Ptr) == nullptr ) { \
Platform->LogFp("[Assets] " #Ptr " is null"); \
return False; \
} \
    
    
    Platform->ClearTexturesFp();
    
    Assets->TextureCount = Texture_Count;
    Assets->Textures = Arena_PushArray(texture, Arena, Texture_Count);
    
    Assets->AtlasAabbCount = AtlasAabb_Count;
    Assets->AtlasAabbs = Arena_PushArray(atlas_aabb, Arena, AtlasAabb_Count);
    
    Assets->FontCount = Font_Count;
    Assets->Fonts = Arena_PushArray(font, Arena, Font_Count);
    
    Assets->AnimeCount = Anime_Count;
    Assets->Animes = Arena_PushArray(anime, Arena, Anime_Count);
    
    platform_file_handle AssetFile = Platform->OpenAssetFileFp();
    CheckFile(AssetFile);
    Defer { Platform->CloseFileFp(&AssetFile); }; 
    
    u32 CurFileOffset = 0;
    u32 FileEntryCount = 0;
    
    // Check file signaure
    {        
        arena_mark Scratch = Arena_Mark(Arena);
        Defer{ Arena_Revert(&Scratch); };
        
        u8_cstr Signature = U8CStr_FromSiStr("MOMO");
        void* ReadSignature = Assets_ReadBlock(&AssetFile,
                                               Platform,
                                               Scratch.Arena,
                                               &CurFileOffset,
                                               (u32)Signature.Count,
                                               1);
        CheckPtr(ReadSignature);
        CheckFile(AssetFile);
        
        if (!Assets_CheckSignature(ReadSignature, Signature)) {
            Platform->LogFp("[Assets] Wrong asset signature\n");
            return False;
        }
        
        // Get File Entry
        FileEntryCount = *Assets_ReadStruct(u32,
                                            &AssetFile,
                                            Platform,
                                            Scratch.Arena,
                                            &CurFileOffset);
        CheckFile(AssetFile);
    }
    
    
    for (u32 I = 0; I < FileEntryCount; ++I) 
    {
        
        // NOTE(Momo): Read header
        asset_file_entry FileEntry = {};
        {
            arena_mark Scratch = Arena_Mark(Arena);
            Defer{ Arena_Revert(&Scratch); };
            
            auto* FileEntryPtr = Assets_ReadStruct(asset_file_entry,
                                                   &AssetFile,
                                                   Platform,
                                                   Scratch.Arena,
                                                   &CurFileOffset);
            CheckPtr(FileEntryPtr);
            CheckFile(AssetFile);
            FileEntry = *FileEntryPtr;
        }
        
        switch(FileEntry.Type) {
            case AssetType_Texture: {
                arena_mark Scratch = Arena_Mark(Arena);
                Defer{ Arena_Revert(&Scratch); };
                
                auto* FileTexture = Assets_ReadStruct(asset_file_texture,
                                                      &AssetFile, 
                                                      Platform, 
                                                      Scratch.Arena,
                                                      &CurFileOffset);              
                
                CheckPtr(FileTexture);
                CheckFile(AssetFile);
                
                texture* Texture = Assets->Textures + FileTexture->Id;
                Texture->Width = FileTexture->Width;
                Texture->Height = FileTexture->Height;
                Texture->Channels = FileTexture->Channels;
                u32 TextureSize = Texture->Width * 
                    Texture->Height * 
                    Texture->Channels;
                
                void* Pixels = Assets_ReadBlock(&AssetFile, 
                                                Platform,
                                                Scratch.Arena, 
                                                &CurFileOffset,
                                                TextureSize,
                                                1);
                CheckPtr(Pixels);
                CheckFile(AssetFile);
                
                Texture->Handle = Platform->AddTextureFp(FileTexture->Width, 
                                                         FileTexture->Height,
                                                         Pixels);
                if (!Texture->Handle.Success) {
                    Platform->LogFp("[Assets] Cannot add assets!");
                }
            } break;
            case AssetType_AtlasAabb: { 
                arena_mark Scratch = Arena_Mark(Arena);
                Defer{ Arena_Revert(&Scratch); };
                
                auto* FileAtlasAabb = 
                    Assets_ReadStruct(asset_file_atlas_aabb,
                                      &AssetFile, 
                                      Platform, 
                                      Scratch.Arena,
                                      &CurFileOffset);              
                CheckPtr(FileAtlasAabb);
                CheckFile(AssetFile);
                
                auto* AtlasAabb = Assets->AtlasAabbs + FileAtlasAabb->Id;
                AtlasAabb->Aabb = FileAtlasAabb->Aabb;
                AtlasAabb->TextureId = FileAtlasAabb->TextureId;
            } break;
            case AssetType_Font: {
                arena_mark Scratch = Arena_Mark(Arena);
                Defer{ Arena_Revert(&Scratch); };
                
                auto* FileFont = Assets_ReadStruct(asset_file_font,
                                                   &AssetFile,
                                                   Platform,
                                                   Scratch.Arena,
                                                   &CurFileOffset);
                CheckPtr(FileFont);
                CheckFile(AssetFile);
                
                auto* Font = Assets->Fonts + FileFont->Id;
                Font->LineGap = FileFont->LineGap;
                Font->Ascent = FileFont->Ascent;
                Font->Descent = FileFont->Descent;
            } break;
            case AssetType_FontGlyph: {
                arena_mark Scratch = Arena_Mark(Arena);
                Defer{ Arena_Revert(&Scratch); };
                
                auto* FileFontGlyph = Assets_ReadStruct(asset_file_font_glyph,
                                                        &AssetFile,
                                                        Platform,
                                                        Scratch.Arena,
                                                        &CurFileOffset);
                CheckPtr(FileFontGlyph);
                CheckFile(AssetFile);
                
                font* Font = Assets_GetFont(Assets, FileFontGlyph->FontId);
                font_glyph* Glyph = Font_GetGlyph(Font, FileFontGlyph->Codepoint);
                
                Glyph->AtlasAabb = FileFontGlyph->AtlasAabb;
                Glyph->TextureId = FileFontGlyph->TextureId;
                Glyph->Advance = FileFontGlyph->Advance;
                Glyph->LeftBearing = FileFontGlyph->LeftBearing;
                Glyph->Box = FileFontGlyph->Box;
            } break;
            case AssetType_FontKerning: {
                arena_mark Scratch = Arena_Mark(Arena);
                Defer{ Arena_Revert(&Scratch); };
                
                auto* FileFontKerning = Assets_ReadStruct(asset_file_font_kerning,
                                                          &AssetFile,
                                                          Platform,
                                                          Scratch.Arena,
                                                          &CurFileOffset);
                CheckPtr(FileFontKerning);
                CheckFile(AssetFile);
                font* Font = Assets_GetFont(Assets, FileFontKerning->FontId);
                Font_SetKerning(Font, 
                                FileFontKerning->CodepointA, 
                                FileFontKerning->CodepointB,
                                FileFontKerning->Kerning);
                
            } break;
            case AssetType_Sound: {
                return False;
            } break;
            case AssetType_Anime: {
                arena_mark Scratch = Arena_Mark(Arena);
                
                auto* FileAnime = Assets_ReadStruct(asset_file_anime,
                                                    &AssetFile,
                                                    Platform,
                                                    Scratch.Arena,
                                                    &CurFileOffset);
                
                CheckPtr(FileAnime);
                CheckFile(AssetFile);
                
                anime* Anime = Assets_GetAnime(Assets, FileAnime->AnimeId);
                Anime->FrameCount = FileAnime->FrameCount;
                
                // NOTE(Momo): we have to revert here so that our arena's 
                // memory does not screw up. A bit janky but hey.
                Arena_Revert(&Scratch); 
                
                Anime->Frames = (atlas_aabb_id*)
                    Assets_ReadBlock(&AssetFile,
                                     Platform, 
                                     Arena, 
                                     &CurFileOffset,
                                     sizeof(atlas_aabb_id) * Anime->FrameCount,
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
