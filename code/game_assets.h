#ifndef __GAME_ASSETS_H__
#define __GAME_ASSETS_H__

// NOTE(Momo): Asset types
struct texture {
    u32 Width, Height, Channels;
    renderer_texture_handle Handle;
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
CheckAssetSignature(void* Memory, u8_cstr Signature) {
    u8* MemoryU8 = (u8*)Memory;
    for (u32 I = 0; I < Signature.Size; ++I) {
        if (MemoryU8[I] != Signature.Data[I]) {
            return False;
        }
    }
    return True; 
}

// TODO: Maybe we create some kind of struct for reading files?
static inline void*
Read(platform_file_handle* File,
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


template<typename t>
static inline t*
Read(platform_file_handle* File,
     platform_api* Platform,
     arena* Arena,
     u32* FileOffset)
{
    return (t*)Read(File, Platform, Arena, FileOffset, sizeof(t), alignof(t));
}

template<typename t>
static inline t
ReadCopy(platform_file_handle* File,
         platform_api* Platform,
         arena* Arena,
         u32* FileOffset) 
{
    t* Ret = Read<t>(File, Platform, Arena, FileOffset);
    Assert(Ret);
    return *Ret;
}

static inline assets*
AllocateAssets(arena* Arena,
               platform_api* Platform) 
{
#define CheckFile(Handle) \
if (Handle.Error) { \
Platform->LogFileErrorFp(&Handle); \
return nullptr; \
}
    
#define CheckPtr(Ptr) \
if ((Ptr) == nullptr ) { \
Platform->LogFp("[Assets] " #Ptr " is null"); \
return nullptr; \
} \
    
    
    Platform->ClearTexturesFp();
    
    assets* Ret = Arena_PushStruct(assets, Arena);
    
    Ret->TextureCount = Texture_Count;
    Ret->Textures = Arena_PushArray(texture, Arena, Texture_Count);
    
    Ret->AtlasAabbCount = AtlasAabb_Count;
    Ret->AtlasAabbs = Arena_PushArray(atlas_aabb, Arena, AtlasAabb_Count);
    
    Ret->FontCount = Font_Count;
    Ret->Fonts = Arena_PushArray(font, Arena, Font_Count);
    
    
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
        void* ReadSignature = Read(&AssetFile,
                                   Platform,
                                   Scratch.Arena,
                                   &CurFileOffset,
                                   (u32)Signature.Size,
                                   1);
        CheckPtr(ReadSignature);
        CheckFile(AssetFile);
        
        if (!CheckAssetSignature(ReadSignature, Signature)) {
            Platform->LogFp("[Assets] Wrong asset signature\n");
            return nullptr;
        }
        
        // Get File Entry
        FileEntryCount = ReadCopy<u32>(&AssetFile,
                                       Platform,
                                       Scratch.Arena,
                                       &CurFileOffset);
        CheckFile(AssetFile);
    }
    
    
    for (u32 I = 0; I < FileEntryCount; ++I) 
    {
        arena_mark Scratch = Arena_Mark(Arena);
        Defer{ Arena_Revert(&Scratch); };
        
        // NOTE(Momo): Read header
        auto* FileEntry = Read<asset_file_entry>(&AssetFile,
                                                 Platform,
                                                 Scratch.Arena,
                                                 &CurFileOffset);
        CheckPtr(FileEntry);
        CheckFile(AssetFile);
        
        switch(FileEntry->Type) {
            case AssetType_Texture: {
                using data_t = asset_file_texture;
                auto* FileTexture = Read<data_t>(&AssetFile, 
                                                 Platform, 
                                                 Scratch.Arena,
                                                 &CurFileOffset);              
                
                CheckPtr(FileTexture);
                CheckFile(AssetFile);
                
                auto* Texture = Ret->Textures + FileTexture->Id;
                Texture->Width = FileTexture->Width;
                Texture->Height = FileTexture->Height;
                Texture->Channels = FileTexture->Channels;
                u32 TextureSize = Texture->Width * 
                    Texture->Height * 
                    Texture->Channels;
                
                void* Pixels = Read(&AssetFile, 
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
                using data_t = asset_file_atlas_aabb;
                auto* FileAtlasAabb = Read<data_t>(&AssetFile, 
                                                   Platform, 
                                                   Scratch.Arena,
                                                   &CurFileOffset);              
                CheckPtr(FileAtlasAabb);
                CheckFile(AssetFile);
                
                auto* AtlasAabb = Ret->AtlasAabbs + FileAtlasAabb->Id;
                AtlasAabb->Aabb = FileAtlasAabb->Aabb;
                AtlasAabb->TextureId = FileAtlasAabb->TextureId;
            } break;
            case AssetType_Font: {
                using data_t = asset_file_font;
                auto* FileFont = Read<data_t>(&AssetFile,
                                              Platform,
                                              Scratch.Arena,
                                              &CurFileOffset);
                CheckPtr(FileFont);
                CheckFile(AssetFile);
                
                auto* Font = Ret->Fonts + FileFont->Id;
                Font->LineGap = FileFont->LineGap;
                Font->Ascent = FileFont->Ascent;
                Font->Descent = FileFont->Descent;
            } break;
            case AssetType_FontGlyph: {
                using data_t = asset_file_font_glyph;
                auto* FileFontGlyph = Read<data_t>(&AssetFile,
                                                   Platform,
                                                   Scratch.Arena,
                                                   &CurFileOffset);
                CheckPtr(FileFontGlyph);
                CheckFile(AssetFile);
                
                font* Font = Assets_GetFont(Ret, FileFontGlyph->FontId);
                font_glyph* Glyph = Font_GetGlyph(Font, FileFontGlyph->Codepoint);
                
                Glyph->AtlasAabb = FileFontGlyph->AtlasAabb;
                Glyph->TextureId = FileFontGlyph->TextureId;
                Glyph->Advance = FileFontGlyph->Advance;
                Glyph->LeftBearing = FileFontGlyph->LeftBearing;
                Glyph->Box = FileFontGlyph->Box;
            } break;
            case AssetType_FontKerning: {
                using data_t = asset_file_font_kerning;
                auto* FileFontKerning = Read<data_t>(&AssetFile,
                                                     Platform,
                                                     Scratch.Arena,
                                                     &CurFileOffset);
                CheckPtr(FileFontKerning);
                CheckFile(AssetFile);
                font* Font = Assets_GetFont(Ret, FileFontKerning->FontId);
                Font_SetKerning(Font, 
                                FileFontKerning->CodepointA, 
                                FileFontKerning->CodepointB,
                                FileFontKerning->Kerning);
                
            } break;
            case AssetType_Sound: {
                
            } break;
            default: {
                Assert(false);
            } break;
            
            
        }
    }
    
    
    
    return Ret;
    
#undef CheckFile
#undef CheckPtr
}

#endif  
