#ifndef __GAME_ASSETS__
#define __GAME_ASSETS__

#include "mm_core.h"

#include "mm_maths.h"
#include "mm_arena.h"
#include "mm_commands.h"

#include "platform.h"
#include "game_assets_file_formats.h"
#include "game_renderer.h"

// NOTE(Momo): Asset types
struct bitmap {
    u32 Width, Height, Channels;
    void* Pixels;
};

struct atlas_rect {
    mmm_rect2u Rect;
    bitmap_id BitmapId;
};


struct font_glyph {
    mmm_rect2u AtlasRect;
    mmm_rect2f Box; 
    bitmap_id BitmapId;
    f32 Advance;
    f32 LeftBearing;
};

static inline f32 
mmm_AspectRatio(font_glyph* Glyph) {
    return mmm_AspectRatio(Glyph->Box);
}

struct font {
    // NOTE(Momo): We cater for a fixed set of codepoints. 
    // ASCII 32 to 126 
    // Worry about sparseness next time.
    f32 LineGap;
    font_glyph Glyphs[Codepoint_Count];
    u32 Kernings[Codepoint_Count][Codepoint_Count];
};


static inline usize
GetGlyphIndexFromCodepoint(u32 Codepoint) {
    Assert(Codepoint >= Codepoint_Start);
    Assert(Codepoint <= Codepoint_End);
    return Codepoint - Codepoint_Start;
}

struct game_assets {
    mmarn_arena Arena;
    
    bitmap Bitmaps[Bitmap_Count];
    atlas_rect AtlasRects[AtlasRect_Count];
    font Fonts[Font_Count];
    
    platform_api* Platform;
};


static inline mmm_quad2f
GetAtlasUV(game_assets* Assets, atlas_rect* AtlasRect) {
    auto Bitmap = Assets->Bitmaps[AtlasRect->BitmapId];
    return mmm_Quad2F(mmm_RatioRect(AtlasRect->Rect, {0, 0, Bitmap.Width, Bitmap.Height}));
}

static inline mmm_quad2f
GetAtlasUV(game_assets* Assets, font_glyph* Glyph) {
    auto Bitmap = Assets->Bitmaps[Glyph->BitmapId];
    return mmm_Quad2F(mmm_RatioRect(Glyph->AtlasRect, {0, 0, Bitmap.Width, Bitmap.Height}));
}


static inline b32
CheckAssetSignature(void* Memory, const char* Signature) {
    u8* MemoryU8 = (u8*)Memory;
    u32 SigLen = StrLen(Signature);
    for (u32 i = 0; i < SigLen; ++i) {
        if (MemoryU8[i] != Signature[i]) {
            return false;
        }
    }
    return true; 
}


// TODO(Momo): Perhaps remove render_commands and replace with platform call for linking textures?
static inline void
Init(game_assets* Assets, 
     mmarn_arena* Arena, 
     platform_api* Platform,
     mmcmd_commands* RenderCommands,
     const char* Filename)
{
    Assets->Arena = mmarn_PushArena(Arena, Megabytes(100));
    Assets->Platform = Platform;
    
    mmarn_scratch Scratch = mmarn_BeginScratch(&Assets->Arena);
    Defer{ mmarn_EndScratch(&Scratch); };
    
    // NOTE(Momo): File read into temp mmarn_arena
    // TODO(Momo): We could just create the assets as we read.
    u8* FileMemory = nullptr;
    u8* FileMemoryItr = nullptr;
    {
        u32 Filesize = Platform->GetFileSize(Filename);
        Assert(Filesize);
        FileMemory = FileMemoryItr = (u8*)mmarn_PushBlock(&Scratch, Filesize);
        Platform->ReadFile(FileMemory, Filesize, Filename);
    }
    
    // NOTE(Momo): Read and check file header
    u32 FileEntryCount = 0;
    {
        const char* Signature = "MOMO";
        Assert(CheckAssetSignature(FileMemoryItr, Signature));
        FileMemoryItr+= StrLen(Signature);
        
        // NOTE(Momo): Read the counts in order
        FileEntryCount = *(mmbw_Read<u32>(&FileMemoryItr));
    }
    
    // NOTE(Momo): Allocate Assets
    
    for (u32 i = 0; i < FileEntryCount; ++i) {
        // NOTE(Momo): Read header
        auto* YuuEntry = mmbw_Read<yuu_entry>(&FileMemoryItr);
        
        switch(YuuEntry->Type) {
            case AssetType_Bitmap: {
                auto* YuuBitmap = mmbw_Read<yuu_bitmap>(&FileMemoryItr);
                
                // NOTE(Momo): Allocate Image
                auto* Bitmap = Assets->Bitmaps + YuuBitmap->Id;
                Bitmap->Width = YuuBitmap->Width;
                Bitmap->Height = YuuBitmap->Height;
                Bitmap->Channels = YuuBitmap->Channels;
                
                // NOTE(Momo): Allocate pixel data
                usize BitmapSize = Bitmap->Width * Bitmap->Height * Bitmap->Channels;
                Bitmap->Pixels = mmarn_PushBlock(&Assets->Arena, BitmapSize, 1);
                Assert(Bitmap->Pixels);
                MemCopy(Bitmap->Pixels, FileMemoryItr, BitmapSize);
                FileMemoryItr += BitmapSize;
                
                PushCommandLinkTexture(RenderCommands, 
                                       Bitmap->Width, 
                                       Bitmap->Height,
                                       Bitmap->Pixels,
                                       YuuBitmap->Id);
                
            } break;
            case AssetType_AtlasRect: { 
                auto* YuuAtlasRect = mmbw_Read<yuu_atlas_rect>(&FileMemoryItr);
                auto* AtlasRect = Assets->AtlasRects + YuuAtlasRect->Id;
                AtlasRect->Rect = YuuAtlasRect->Rect;
                AtlasRect->BitmapId = YuuAtlasRect->BitmapId;
            } break;
            case AssetType_Font: {
                auto* YuuFont = mmbw_Read<yuu_font>(&FileMemoryItr);
                auto* Font = Assets->Fonts + YuuFont->Id;
                Font->LineGap = YuuFont->LineGap;
            } break;
            case AssetType_FontGlyph: {
                auto* YuuFontGlyph = mmbw_Read<yuu_font_glyph>(&FileMemoryItr);
                auto* Font = Assets->Fonts + YuuFontGlyph->FontId;
                usize GlyphIndex = GetGlyphIndexFromCodepoint(YuuFontGlyph->Codepoint);
                auto* Glyph = Font->Glyphs + GlyphIndex;
                
                Glyph->AtlasRect = YuuFontGlyph->AtlasRect;
                Glyph->BitmapId = YuuFontGlyph->BitmapId;
                Glyph->Advance = YuuFontGlyph->Advance;
                Glyph->LeftBearing = YuuFontGlyph->LeftBearing;
                Glyph->Box = YuuFontGlyph->Box;
            } break;
            case AssetType_FontKerning: {
                auto* YuuFontKerning = mmbw_Read<yuu_font_kerning>(&FileMemoryItr);
                auto* Font = Assets->Fonts + YuuFontKerning->FontId;
                Font->Kernings[YuuFontKerning->CodepointA][YuuFontKerning->CodepointB] = YuuFontKerning->Kerning;
                
            } break;
            default: {
                Assert(false);
            } break;
            
            
        }
        
    }
    
}


#endif  
