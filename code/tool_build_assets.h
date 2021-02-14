#ifndef TOOL_BUILD_ASSETS_H
#define TOOL_BUILD_ASSETS_H

#include <stdio.h>
#include <stdlib.h>
#include "mm_core.h"


#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "game_assets_types.h"  


/////
struct ab_context {
    FILE* File;
    u32 EntryCount;
    long int EntryCountAt;
};

static inline void
Begin(ab_context* Context, const char* Filename, const char* Signature) 
{
    Context->EntryCount = 0;
    Context->File = nullptr; 
	fopen_s(&Context->File, Filename, "wb");
    Assert(Context->File);
    
    // NOTE(Momo): Write signature
    fwrite(Signature, sizeof(u8), SiStrLen(Signature), Context->File);
    Context->EntryCountAt = ftell(Context->File);
    
    // NOTE(Momo): Reserve space for EntryCount
    fwrite(&Context->EntryCount, sizeof(u32), 1, Context->File);
}

static inline void
End(ab_context* Context) {
    fseek(Context->File, Context->EntryCountAt, SEEK_SET);
    fwrite(&Context->EntryCount, sizeof(u32), 1, Context->File);
    fclose(Context->File);
}

static inline void
WriteEntry(ab_context* Context, game_asset_type AssetType) {
    game_asset_file_entry Entry = {};
    Entry.Type = AssetType;
    fwrite(&Entry, sizeof(Entry),  1, Context->File);
    ++Context->EntryCount;
}

static inline void 
WriteTexture(ab_context* Context, 
             game_asset_texture_id Id, 
             u32 Width, 
             u32 Height, 
             u32 Channels, 
             u8* Pixels) 
{
    WriteEntry(Context, AssetType_Texture);
    
    game_asset_file_texture Texture = {};
    Texture.Id = Id;
    Texture.Width = Width;
    Texture.Height = Height;
    Texture.Channels = Channels;
    fwrite(&Texture, sizeof(Texture), 1, Context->File);
    
    
    u32 TextureSize = Width * Height * Channels;
    for(u32 i = 0; i < TextureSize; ++i) {
        fwrite(Pixels + i, 1, 1, Context->File);
    }
}

static inline void 
WriteTexture(ab_context* Context, 
             game_asset_texture_id Id, 
             const char* Filename) 
{
    u32 Width = 0, Height = 0, Channels = 0;
    u8* LoadedImage = nullptr;
    {
        i32 W, H, C;
        LoadedImage = stbi_load(Filename, &W, &H, &C, 0);
        Assert(LoadedImage != nullptr);
        
        Width = (u32)W;
        Height = (u32)H; 
        Channels = (u32)C;
    }
    Defer { stbi_image_free(LoadedImage); };
    WriteTexture(Context, Id, Width, Height, Channels, LoadedImage);
}


static inline void 
WriteAtlasAabb(ab_context* Context, 
               game_asset_atlas_aabb_id Id, 
               game_asset_texture_id TargetTextureId, 
               aabb2u Aabb) 
{
    WriteEntry(Context,  AssetType_AtlasAabb);
    
    game_asset_file_atlas_aabb AtlasAabb = {};
    AtlasAabb.Id = Id;
    AtlasAabb.TextureId = TargetTextureId;
    AtlasAabb.Aabb = Aabb;
    fwrite(&AtlasAabb, sizeof(AtlasAabb), 1,  Context->File);
}

static inline void
WriteFont(ab_context* Context, 
          game_asset_font_id Id, 
          f32 Ascent, 
          f32 Descent, 
          f32 LineGap) 
{
    WriteEntry(Context, AssetType_Font);
    
    game_asset_file_font Font = {};
    Font.Id = Id;
    Font.Ascent = Ascent;
    Font.Descent = Descent;
    Font.LineGap = LineGap;
    fwrite(&Font, sizeof(Font), 1, Context->File);
}

static inline void 
WriteFontGlyph(ab_context* Context, 
               game_asset_font_id FontId, 
               game_asset_texture_id TargetTextureId, 
               u32 Codepoint, 
               f32 Advance, 
               f32 LeftBearing, 
               aabb2u AtlasAabb, 
               aabb2f Box) 
{
    WriteEntry(Context, AssetType_FontGlyph);
    
    game_asset_file_font_glyph FontGlyph = {};
    FontGlyph.FontId = FontId;
    FontGlyph.TextureId = TargetTextureId;
    FontGlyph.Codepoint = Codepoint;
    FontGlyph.AtlasAabb = AtlasAabb;
    FontGlyph.LeftBearing = LeftBearing;
    FontGlyph.Advance = Advance;
    FontGlyph.Box = Box;
    fwrite(&FontGlyph, sizeof(FontGlyph), 1, Context->File);
    
}

static inline void 
WriteFontKerning(ab_context* Context, 
                 game_asset_font_id FontId, 
                 u32 CodepointA,
                 u32 CodepointB, 
                 i32 Kerning) 
{
    WriteEntry(Context, AssetType_FontKerning);
    
    game_asset_file_font_kerning Font = {};
    Font.FontId = FontId;
    Font.Kerning = Kerning;
    Font.CodepointA = CodepointA;
    Font.CodepointB = CodepointB;
    fwrite(&Font, sizeof(Font), 1, Context->File);
    
}

#endif //TOOL_BUILD_ASSETS_H
