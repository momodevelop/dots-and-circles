#ifndef TOOL_BUILD_ASSETS_H
#define TOOL_BUILD_ASSETS_H

#include <stdio.h>
#include <stdlib.h>
#include "mm_core.h"


#define STB_IMAGE_IMPLEMENTATION
#include "thirdparty/stb/stb_image.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "thirdparty/stb/stb_truetype.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "thirdparty/stb/stb_image_write.h"

#include "game_assets_file_formats.h"  


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
WriteEntry(ab_context* Context, asset_type AssetType) {
    yuu_entry Entry = {};
    Entry.Type = AssetType;
    fwrite(&Entry, sizeof(Entry),  1, Context->File);
    ++Context->EntryCount;
}

static inline void 
WriteBitmap(ab_context* Context, bitmap_id Id, u32 Width, u32 Height, u32 Channels, u8* Pixels) 
{
    WriteEntry(Context, AssetType_Bitmap);
    
    yuu_bitmap Bitmap = {};
    Bitmap.Id = Id;
    Bitmap.Width = Width;
    Bitmap.Height = Height;
    Bitmap.Channels = Channels;
    fwrite(&Bitmap, sizeof(Bitmap), 1, Context->File);
    
    
    u32 BitmapSize = Width * Height * Channels;
    for(u32 i = 0; i < BitmapSize; ++i) {
        fwrite(Pixels + i, 1, 1, Context->File);
    }
}

static inline void 
WriteBitmap(ab_context* Context, bitmap_id Id, const char* Filename) {
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
    WriteBitmap(Context, Id, Width, Height, Channels, LoadedImage);
}


static inline void 
WriteAtlasRect(ab_context* Context, atlas_rect_id Id, bitmap_id TargetBitmapId, rect2u Rect) 
{
    WriteEntry(Context,  AssetType_AtlasRect);
    
    yuu_atlas_rect AtlasRect = {};
    AtlasRect.Id = Id;
    AtlasRect.BitmapId = TargetBitmapId;
    AtlasRect.Rect = Rect;
    fwrite(&AtlasRect, sizeof(AtlasRect), 1,  Context->File);
}

static inline void
WriteFont(ab_context* Context, font_id Id, f32 Ascent, f32 Descent, f32 LineGap) 
{
    WriteEntry(Context, AssetType_Font);
    
    yuu_font Font = {};
    Font.Id = Id;
    Font.Ascent = Ascent;
    Font.Descent = Descent;
    Font.LineGap = LineGap;
    fwrite(&Font, sizeof(Font), 1, Context->File);
}

static inline void 
WriteFontGlyph(ab_context* Context, font_id FontId, bitmap_id TargetBitmapId, u32 Codepoint, f32 Advance, f32 LeftBearing, rect2u AtlasRect, rect2f Box) 
{
    WriteEntry(Context, AssetType_FontGlyph);
    
    yuu_font_glyph FontGlyph = {};
    FontGlyph.FontId = FontId;
    FontGlyph.BitmapId = TargetBitmapId;
    FontGlyph.Codepoint = Codepoint;
    FontGlyph.AtlasRect = AtlasRect;
    FontGlyph.LeftBearing = LeftBearing;
    FontGlyph.Advance = Advance;
    FontGlyph.Box = Box;
    fwrite(&FontGlyph, sizeof(FontGlyph), 1, Context->File);
    
}

static inline void 
WriteFontKerning(ab_context* Context, font_id FontId, u32 CodepointA, u32 CodepointB, i32 Kerning) 
{
    WriteEntry(Context, AssetType_FontKerning);
    
    yuu_font_kerning Font = {};
    Font.FontId = FontId;
    Font.Kerning = Kerning;
    Font.CodepointA = CodepointA;
    Font.CodepointB = CodepointB;
    fwrite(&Font, sizeof(Font), 1, Context->File);
    
}

#endif //TOOL_BUILD_ASSETS_H
