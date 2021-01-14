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

static inline game_assets
CreateAssets(arena* Arena, 
             platform_api* Platform,
             string Filename)
{
    Platform->ClearTextures();

    game_assets Assets = {};
    Assets.Arena = SubArena(Arena, Megabytes(100));
    Assets.Textures = Array<texture>(Arena, Texture_Count);
    Assets.AtlasAabbs = Array<atlas_aabb>(Arena, AtlasAabb_Count);
    Assets.Fonts = Array<font>(Arena, Font_Count);
    
    scratch Scratch = BeginScratch(&Assets.Arena);
    Defer{ EndScratch(&Scratch); };
    
    // NOTE(Momo): File read into temp arena
    // TODO(Momo): We could just create the assets as we read.
    void* FileMemory = nullptr;
    void* FileMemoryItr = nullptr;
    {
        u32 Filesize = Platform->GetFileSize(Filename.Elements);
        Assert(Filesize);
        FileMemory = FileMemoryItr = PushBlock(Scratch, Filesize);
        Platform->ReadFile(FileMemory, Filesize, Filename.Elements);
    }
    
    // NOTE(Momo): Read and check file header
    u32 FileEntryCount = 0;
    {
        const char* Signature = "MOMO";
        FileMemoryItr = (u8*)FileMemoryItr + SiStrLen(Signature);
        
        // NOTE(Momo): Read the counts in order
        FileEntryCount = ReadCopy<u32>(&FileMemoryItr);
    }
    
    // NOTE(Momo): Allocate Assets
    
    for (u32 i = 0; i < FileEntryCount; ++i) {
        // NOTE(Momo): Read header
        auto* YuuEntry = Read<yuu_entry>(&FileMemoryItr);
        
        switch(YuuEntry->Type) {
            case AssetType_Texture: {
                auto* YuuTexture =
                    Read<yuu_texture>(&FileMemoryItr);              
                auto* Texture = Assets.Textures + YuuTexture->Id;
                Texture->Width = YuuTexture->Width;
                Texture->Height = YuuTexture->Height;
                Texture->Channels = YuuTexture->Channels;
                usize TextureSize = Texture->Width * 
                                   Texture->Height * 
                                   Texture->Channels;

                void* Pixels = PushBlock(&Assets.Arena, TextureSize, 1);
                Assert(Pixels);
                Copy(Pixels, FileMemoryItr, TextureSize);
                FileMemoryItr = (u8*)FileMemoryItr + TextureSize;
                Texture->Handle = Platform->AddTexture(YuuTexture->Width, 
                                                       YuuTexture->Height,
                                                       Pixels);
            } break;
            case AssetType_AtlasAabb: { 
                auto* YuuAtlasAabb = Read<yuu_atlas_aabb>(&FileMemoryItr);
                auto* AtlasAabb = Assets.AtlasAabbs + YuuAtlasAabb->Id;
                AtlasAabb->Aabb = YuuAtlasAabb->Aabb;
                AtlasAabb->TextureId = YuuAtlasAabb->TextureId;
            } break;
            case AssetType_Font: {
                auto* YuuFont = Read<yuu_font>(&FileMemoryItr);
                auto* Font = Assets.Fonts + YuuFont->Id;
                Font->LineGap = YuuFont->LineGap;
                Font->Ascent = YuuFont->Ascent;
                Font->Descent = YuuFont->Descent;
            } break;
            case AssetType_FontGlyph: {
                auto* YuuFontGlyph = Read<yuu_font_glyph>(&FileMemoryItr);
                auto* Font = Assets.Fonts + YuuFontGlyph->FontId;
                usize GlyphIndex = HashCodepoint(YuuFontGlyph->Codepoint);
                auto* Glyph = Font->Glyphs + GlyphIndex;
                
                Glyph->AtlasAabb = YuuFontGlyph->AtlasAabb;
                Glyph->TextureId = YuuFontGlyph->TextureId;
                Glyph->Advance = YuuFontGlyph->Advance;
                Glyph->LeftBearing = YuuFontGlyph->LeftBearing;
                Glyph->Box = YuuFontGlyph->Box;
            } break;
            case AssetType_FontKerning: {
                auto* YuuFontKerning =
                    Read<yuu_font_kerning>(&FileMemoryItr);
                font* Font = Assets.Fonts + YuuFontKerning->FontId;
                usize HashedCpA= HashCodepoint(YuuFontKerning->CodepointA);
                usize HashedCpB= HashCodepoint(YuuFontKerning->CodepointB);
                
                Font->Kernings[HashedCpA][HashedCpB] = YuuFontKerning->Kerning;
                
            } break;
            default: {
                Assert(false);
            } break;
            
            
        }
        
    }
    
    return Assets;
}

static inline game_assets*
AllocateAssets(arena* Arena,
               platform_api* Platform) 
{
    Platform->ClearTextures();

    game_assets* Ret = PushStruct<game_assets>(Arena);
    Ret->Textures = Array<texture>(Arena, Texture_Count);
    Ret->AtlasAabbs = Array<atlas_aabb>(Arena, AtlasAabb_Count);
    Ret->Fonts = Array<font>(Arena, Font_Count);


    platform_file_handle AssetFile = Platform->OpenAssetFile();
    if(AssetFile.Error) {
        Platform->LogFileError(&AssetFile);
        return nullptr;
    }


    usize CurrentFileOffset = 0;
    u32 FileEntryCount = 0;
    // Check file signaure
    {        
        scratch Scratch = BeginScratch(Arena);
        Defer { EndScratch(&Scratch); };
        
        string Signature = String("MOMO");
        void* Dest = PushBlock(Scratch, Signature.Count, 1);  

        Platform->ReadFile2(&AssetFile,
                            CurrentFileOffset, 
                            Signature.Count, 
                            Dest);

        if (!CheckAssetSignature(Dest, Signature)) {
            Platform->Log("[Assets] Wrong asset signature\n");
            return nullptr;
        }
        CurrentFileOffset += Signature.Count;
        
        // Get File Entry
        Platform->ReadFile2(&AssetFile,
                            CurrentFileOffset,
                            sizeof(u32),
                            Dest);

        FileEntryCount = PeekCopy<u32>(Dest);
    }
   
    // Allocate assets
    {


    }


    
    return Ret;
}


#endif  
