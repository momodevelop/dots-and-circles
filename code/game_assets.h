#ifndef __GAME_ASSETS__
#define __GAME_ASSETS__

#include "mm_core.h"

#include "mm_array.h"
#include "mm_maths.h"
#include "mm_arena.h"
#include "mm_mailbox.h"
#include "mm_string.h"

#include "game_platform.h"
#include "game_assets_file_formats.h"

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
    font_glyph Glyphs[Codepoint_Count];
    u32 Kernings[Codepoint_Count][Codepoint_Count];
};

static inline usize
HashCodepoint(u32 Codepoint) {
    Assert(Codepoint >= Codepoint_Start);
    Assert(Codepoint <= Codepoint_End);
    return Codepoint - Codepoint_Start;
}

static inline f32 
Height(font* Font) {
    return Abs(Font->Ascent) + Abs(Font->Descent);
}


struct game_assets {
    // TODO: remove
    arena Arena;
   
    array<texture> Textures;
    array<atlas_aabb> AtlasAabbs;
    array<font> Fonts;
};


// TODO: Maybe we should just pass in ID instead of pointer.
// Should be waaaay cleaner
static inline quad2f
GetAtlasUV(game_assets* Assets, atlas_aabb* AtlasAabb) {
    auto Texture = Assets->Textures[AtlasAabb->TextureId];
    aabb2u Aabb = {0, 0, Texture.Width, Texture.Height };
    return Quad2f(RatioAabb(AtlasAabb->Aabb,Aabb));
}

static inline quad2f
GetAtlasUV(game_assets* Assets, font_glyph* Glyph) {
    auto Texture = Assets->Textures[Glyph->TextureId];
    aabb2u Aabb = {0, 0, Texture.Width, Texture.Height };
    return Quad2f(RatioAabb(Glyph->AtlasAabb, Aabb));
}


static inline renderer_texture_handle
GetRendererTextureHandle(game_assets* Assets, texture_id TextureId) {
    return Assets->Textures[TextureId].Handle;
}


static inline b32
CheckAssetSignature(void* Memory, string Signature) {
    u8* MemoryU8 = (u8*)Memory;
    for (u32 i = 0; i < Signature.Count; ++i) {
        if (MemoryU8[i] != Signature[i]) {
            return false;
        }
    }
    return true; 
}

// TODO: Maybe we create some kind of struct for reading files?
static inline void*
Read(platform_file_handle* File,
     platform_api* Platform,
     arena* Arena,
     usize* FileOffset,
     usize BlockSize,
     u8 BlockAlignment)
{
    void* Ret = PushBlock(Arena, BlockSize, BlockAlignment);
    if(!Ret) {
        return nullptr; 
    }
    Platform->ReadFile2(File,
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
     usize* FileOffset)
{
    return (t*)Read(File, Platform, Arena, FileOffset, sizeof(t), alignof(t));
}

template<typename t>
static inline t
ReadCopy(platform_file_handle* File,
         platform_api* Platform,
         arena* Arena,
         usize* FileOffset) 
{
    t* Ret = Read<t>(File, Platform, Arena, FileOffset);
    Assert(Ret);
    return *Ret;
}

static inline game_assets*
AllocateAssets(arena* Arena,
               platform_api* Platform) 
{
#define CheckFile(Handle) \
    if (Handle.Error) { \
        Platform->LogFileError(&Handle); \
        return nullptr; \
    }

#define CheckPtr(Ptr) \
    if ((Ptr) == nullptr ) { \
        Platform->Log(#Ptr " is null"); \
        return nullptr; \
    } \


    Platform->ClearTextures();

    game_assets* Ret = PushStruct<game_assets>(Arena);
    Ret->Textures = Array<texture>(Arena, Texture_Count);
    Ret->AtlasAabbs = Array<atlas_aabb>(Arena, AtlasAabb_Count);
    Ret->Fonts = Array<font>(Arena, Font_Count);


    platform_file_handle AssetFile = Platform->OpenAssetFile();
    CheckFile(AssetFile);
    Defer { Platform->CloseFile(&AssetFile); }; 

    usize CurFileOffset = 0;
    u32 FileEntryCount = 0;
    // Check file signaure
    {        
        scratch Scratch = BeginScratch(Arena);
        Defer { EndScratch(&Scratch); };
        
        string Signature = String("MOMO");
        void* ReadSignature = Read(&AssetFile,
                                   Platform,
                                   Arena,
                                   &CurFileOffset,
                                   Signature.Count,
                                   1);
        CheckPtr(ReadSignature);
        CheckFile(AssetFile);

        if (!CheckAssetSignature(ReadSignature, Signature)) {
            Platform->Log("[Assets] Wrong asset signature\n");
            return nullptr;
        }
        
        // Get File Entry
        FileEntryCount = ReadCopy<u32>(&AssetFile,
                                       Platform,
                                       Scratch,
                                       &CurFileOffset);
        CheckFile(AssetFile);
    }
   

    for (u32 I = 0; I < FileEntryCount; ++I) 
    {
        scratch Scratch = BeginScratch(Arena);
        Defer { EndScratch(&Scratch); };

        // NOTE(Momo): Read header
        auto* YuuEntry = Read<yuu_entry>(&AssetFile,
                                         Platform,
                                         Scratch,
                                         &CurFileOffset);
        CheckPtr(YuuEntry);
        CheckFile(AssetFile);

        switch(YuuEntry->Type) {
            case AssetType_Texture: {
                auto* YuuTexture = Read<yuu_texture>(&AssetFile, 
                                                     Platform, 
                                                     Scratch,
                                                     &CurFileOffset);              

                CheckPtr(YuuTexture);
                CheckFile(AssetFile);

                auto* Texture = Ret->Textures + YuuTexture->Id;
                Texture->Width = YuuTexture->Width;
                Texture->Height = YuuTexture->Height;
                Texture->Channels = YuuTexture->Channels;
                usize TextureSize = Texture->Width * 
                                    Texture->Height * 
                                    Texture->Channels;

                void* Pixels = Read(&AssetFile, 
                                    Platform,
                                    Arena, 
                                    &CurFileOffset,
                                    TextureSize,
                                    1);
                CheckPtr(Pixels);
                CheckFile(AssetFile);

                Texture->Handle = Platform->AddTexture(YuuTexture->Width, 
                                                       YuuTexture->Height,
                                                       Pixels);
            } break;
            case AssetType_AtlasAabb: { 
                auto* YuuAtlasAabb = Read<yuu_atlas_aabb>(&AssetFile, 
                                                          Platform, 
                                                          Scratch,
                                                          &CurFileOffset);              
                CheckPtr(YuuAtlasAabb);
                CheckFile(AssetFile);

                auto* AtlasAabb = Ret->AtlasAabbs + YuuAtlasAabb->Id;
                AtlasAabb->Aabb = YuuAtlasAabb->Aabb;
                AtlasAabb->TextureId = YuuAtlasAabb->TextureId;
            } break;
            case AssetType_Font: {
                auto* YuuFont = Read<yuu_font>(&AssetFile,
                                               Platform,
                                               Scratch,
                                               &CurFileOffset);
                CheckPtr(YuuFont);
                CheckFile(AssetFile);

                auto* Font = Ret->Fonts + YuuFont->Id;
                Font->LineGap = YuuFont->LineGap;
                Font->Ascent = YuuFont->Ascent;
                Font->Descent = YuuFont->Descent;
            } break;
            case AssetType_FontGlyph: {
                auto* YuuFontGlyph = Read<yuu_font_glyph>(&AssetFile,
                                                          Platform,
                                                          Scratch,
                                                          &CurFileOffset);
                CheckPtr(YuuFontGlyph);
                CheckFile(AssetFile);

                auto* Font = Ret->Fonts + YuuFontGlyph->FontId;
                usize GlyphIndex = HashCodepoint(YuuFontGlyph->Codepoint);
                auto* Glyph = Font->Glyphs + GlyphIndex;
                
                Glyph->AtlasAabb = YuuFontGlyph->AtlasAabb;
                Glyph->TextureId = YuuFontGlyph->TextureId;
                Glyph->Advance = YuuFontGlyph->Advance;
                Glyph->LeftBearing = YuuFontGlyph->LeftBearing;
                Glyph->Box = YuuFontGlyph->Box;
            } break;
            case AssetType_FontKerning: {
                auto* YuuFontKerning = Read<yuu_font_kerning>(&AssetFile,
                                                              Platform,
                                                              Scratch,
                                                              &CurFileOffset);
                CheckPtr(YuuFontKerning);
                CheckFile(AssetFile);
                font* Font = Ret->Fonts + YuuFontKerning->FontId;
                usize HashedCpA= HashCodepoint(YuuFontKerning->CodepointA);
                usize HashedCpB= HashCodepoint(YuuFontKerning->CodepointB);
                
                Font->Kernings[HashedCpA][HashedCpB] = YuuFontKerning->Kerning;
                
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
