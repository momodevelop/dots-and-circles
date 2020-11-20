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
    rect2u Rect;
    bitmap_id BitmapId;
};


struct font_glyph {
    rect2u AtlasRect;
    rect2f Box; 
    bitmap_id BitmapId;
    f32 Advance;
    f32 LeftBearing;
};

static inline f32 
AspectRatio(font_glyph* Glyph) {
    return AspectRatio(Glyph->Box);
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
HashCodepoint(u32 Codepoint) {
    Assert(Codepoint >= Codepoint_Start);
    Assert(Codepoint <= Codepoint_End);
    return Codepoint - Codepoint_Start;
}

#include "mm_array.h"

struct game_assets {
    arena Arena;
    
    array<bitmap> Bitmaps;
    array<atlas_rect> AtlasRects;
    array<font> Fonts;

    platform_api* Platform;
};


static inline quad2f
GetAtlasUV(game_assets* Assets, atlas_rect* AtlasRect) {
    auto Bitmap = Assets->Bitmaps[AtlasRect->BitmapId];
    return Quad2F(RatioRect(AtlasRect->Rect, {0, 0, Bitmap.Width, Bitmap.Height}));
}

static inline quad2f
GetAtlasUV(game_assets* Assets, font_glyph* Glyph) {
    auto Bitmap = Assets->Bitmaps[Glyph->BitmapId];
    return Quad2F(RatioRect(Glyph->AtlasRect, {0, 0, Bitmap.Width, Bitmap.Height}));
}


static inline b32
CheckAssetSignature(void* Memory, const char* Signature) {
    u8* MemoryU8 = (u8*)Memory;
    u32 SigLen = SiStrLen(Signature);
    for (u32 i = 0; i < SigLen; ++i) {
        if (MemoryU8[i] != Signature[i]) {
            return false;
        }
    }
    return true; 
}


// TODO(Momo): Perhaps remove render_commands and replace with platform call for linking textures?
// TODO(Momo): Change to CreateAssets() structure to align convention
static inline game_assets
CreateAssets(arena* Arena, 
     platform_api* Platform,
     mmcmd_commands* RenderCommands,
     const char* Filename)
{
    game_assets Assets = {};
    Assets.Arena = SubArena(Arena, Megabytes(100));
    Assets.Platform = Platform;
    Assets.Bitmaps = Array<bitmap>(Arena, Bitmap_Count);
    Assets.AtlasRects = Array<atlas_rect>(Arena, AtlasRect_Count);
    Assets.Fonts = Array<font>(Arena, Font_Count);
    
    scratch Scratch = BeginScratch(&Assets.Arena);
    Defer{ EndScratch(&Scratch); };
    
    // NOTE(Momo): File read into temp arena
    // TODO(Momo): We could just create the assets as we read.
    u8* FileMemory = nullptr;
    u8* FileMemoryItr = nullptr;
    {
        u32 Filesize = Platform->GetFileSize(Filename);
        Assert(Filesize);
        FileMemory = FileMemoryItr = (u8*)PushBlock(Scratch, Filesize);
        Platform->ReadFile(FileMemory, Filesize, Filename);
    }
    
    // NOTE(Momo): Read and check file header
    u32 FileEntryCount = 0;
    {
        const char* Signature = "MOMO";
        Assert(CheckAssetSignature(FileMemoryItr, Signature));
        FileMemoryItr+= SiStrLen(Signature);
        
        // NOTE(Momo): Read the counts in order
        FileEntryCount = *(Read<u32>(&FileMemoryItr));
    }
    
    // NOTE(Momo): Allocate Assets
    
    for (u32 i = 0; i < FileEntryCount; ++i) {
        // NOTE(Momo): Read header
        auto* YuuEntry = Read<yuu_entry>(&FileMemoryItr);
        
        switch(YuuEntry->Type) {
            case AssetType_Bitmap: {
                auto* YuuBitmap = Read<yuu_bitmap>(&FileMemoryItr);
                
                // NOTE(Momo): Allocate Image
                auto* Bitmap = Assets.Bitmaps + YuuBitmap->Id;
                Bitmap->Width = YuuBitmap->Width;
                Bitmap->Height = YuuBitmap->Height;
                Bitmap->Channels = YuuBitmap->Channels;
                
                // NOTE(Momo): Allocate pixel data
                usize BitmapSize = Bitmap->Width * Bitmap->Height * Bitmap->Channels;
                Bitmap->Pixels = PushBlock(&Assets.Arena, BitmapSize, 1);
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
                auto* YuuAtlasRect = Read<yuu_atlas_rect>(&FileMemoryItr);
                auto* AtlasRect = Assets.AtlasRects + YuuAtlasRect->Id;
                AtlasRect->Rect = YuuAtlasRect->Rect;
                AtlasRect->BitmapId = YuuAtlasRect->BitmapId;
            } break;
            case AssetType_Font: {
                auto* YuuFont = Read<yuu_font>(&FileMemoryItr);
                auto* Font = Assets.Fonts + YuuFont->Id;
                Font->LineGap = YuuFont->LineGap;
            } break;
            case AssetType_FontGlyph: {
                auto* YuuFontGlyph = Read<yuu_font_glyph>(&FileMemoryItr);
                auto* Font = Assets.Fonts + YuuFontGlyph->FontId;
                usize GlyphIndex = HashCodepoint(YuuFontGlyph->Codepoint);
                auto* Glyph = Font->Glyphs + GlyphIndex;
                
                Glyph->AtlasRect = YuuFontGlyph->AtlasRect;
                Glyph->BitmapId = YuuFontGlyph->BitmapId;
                Glyph->Advance = YuuFontGlyph->Advance;
                Glyph->LeftBearing = YuuFontGlyph->LeftBearing;
                Glyph->Box = YuuFontGlyph->Box;
            } break;
            case AssetType_FontKerning: {
                auto* YuuFontKerning = Read<yuu_font_kerning>(&FileMemoryItr);
                auto* Font = Assets.Fonts + YuuFontKerning->FontId;
                Font->Kernings[HashCodepoint(YuuFontKerning->CodepointA)][HashCodepoint(YuuFontKerning->CodepointB)] = YuuFontKerning->Kerning;
                
            } break;
            default: {
                Assert(false);
            } break;
            
            
        }
        
    }
    
    return Assets;
}



#endif  
