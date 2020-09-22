#include <stdio.h>

#include "ryoji_atlas_builder.h"

#define STB_IMAGE_IMPLEMENTATION
#include "thirdparty/stb/stb_image.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "thirdparty/stb/stb_truetype.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "thirdparty/stb/stb_image_write.h"

#include "game_assets_file_formats.h" 

// NOTE(Momo): Bitmap
struct loaded_bitmap {
    u32 Width;
    u32 Height;
    u32 Channels;
    u8* Pixels;
};

static inline loaded_bitmap
AllocateBitmapFromFile(const char* Filename) {
    i32 W, H, C;
    u8* BitmapData = stbi_load(Filename, &W, &H, &C, 0);
    return { (u32)W, (u32)H, (u32)C, BitmapData };
}

static inline void
FreeBitmap(loaded_bitmap Bitmap) {
    stbi_image_free(Bitmap.Pixels);
}


// NOTE(Momo): Fonts
struct loaded_font {
    stbtt_fontinfo Info;
    void* Data;
};

static inline loaded_font
AllocateFontFromFile(const char* Filename) {
    stbtt_fontinfo Font;
    FILE* FontFile = fopen(Filename, "rb");
    Defer { fclose(FontFile); };
    
    fseek(FontFile, 0, SEEK_END);
    auto Size = ftell(FontFile);
    fseek(FontFile, 0, SEEK_SET);
    
    void* Buffer = malloc(Size);
    fread(Buffer, Size, 1, FontFile);
    stbtt_InitFont(&Font, (u8*)Buffer, 0);
    
    return { Font, Buffer };
}

static inline void
FreeFont(loaded_font Font) {
    free(Font.Data);
}

template<usize N> static inline void 
WriteBitmapToPng(atlas_builder<N>* Builder, const char* PngFilename) {
    printf("[Write] Write to file started \n");
    Defer { printf("[Write] Write to file complete\n"); };
    
    auto Result = AllocateBitmap(Builder);
    Defer { FreeBitmap(Result); };
    
    Assert(Result.Ok);
    printf("\tWriting to %s\n", PngFilename); 
    stbi_write_png(PngFilename, Result.Width, Result.Height, Result.Channels, Result.Bitmap, Builder->Width * Builder->Channels);
}

template<usize N> static inline void 
WriteInfo(atlas_builder<N>* Builder, const char* InfoFilename) {
    printf("[Write] Write to file started \n");
    Defer { printf("[Write] Write to file complete\n"); };
    
    auto Result = AllocateInfo(Builder);
    Assert(Result.Ok);
    Defer { FreeInfo(Result); };
    
    printf("\tWriting to %s\n", InfoFilename); 
    fwrite(Result.Memory, Result.Size, 1, fopen(InfoFilename, "wb"));
}


template<usize N> static inline void 
AddFontGlyph(atlas_builder<N>* Builder, stbtt_fontinfo* Font, int Codepoint, f32 FontSize,  asset_id AssetId, asset_id TargetatlasId) 
{
    
    constexpr u32 Channels = 4;
    f32 Scale = stbtt_ScaleForPixelHeight(Font, FontSize);
    
    i32 Width, Height;
    
    u8 * FontBitmapOneCh = stbtt_GetCodepointBitmap(Font, 0, Scale, Codepoint, &Width, &Height, nullptr, nullptr);
    Defer { stbtt_FreeBitmap( FontBitmapOneCh, nullptr ); };
    
    u32 BitmapDimensions = (u32)(Width * Height);
    u8* FontBitmap = (u8*)malloc(BitmapDimensions * Channels); // Channels for RGBA
    u8* FontBitmapItr = FontBitmap;
    Defer { free(FontBitmap); };
    
    u32 k = 0;
    for (u32 i = 0; i < BitmapDimensions; ++i ){
        for (u32 j = 0; j < Channels; ++j ) {
            FontBitmapItr[k++] = FontBitmapOneCh[i];
        }
    }
    
    AddEntry(Builder, (u32)Width, (u32)Height, (u32)Channels, FontBitmap);
}


struct loaded_atlas_image {
    loaded_bitmap Bitmap;
    yuu_atlas_ud_image UserData;
};

static inline loaded_atlas_image 
LoadAtlasImage(const char* Filename, asset_id AssetId, asset_id TargetAtlasId) 
{
    loaded_atlas_image Ret = {};
    Ret.Bitmap = AllocateBitmapFromFile(Filename);
    Ret.UserData.AssetId = AssetId;
    Ret.UserData.AtlasAssetId = TargetAtlasId;
    Ret.UserData.Type = YuuAtlasUserDataType_Image;
    
    return Ret;
}

static inline void
FreeAtlasImage(loaded_atlas_image Img) { 
    FreeBitmap(Img.Bitmap);
}

struct image_params {
    const char* Filename;
    asset_id Id;
    asset_id TargetAtlasId;
};
static image_params ImageParams[] = {
    { "assets/ryoji.png",  Asset_RectRyoji, Asset_ImageAtlasDefault },
    { "assets/yuu.png",    Asset_RectYuu, Asset_ImageAtlasDefault },
    { "assets/karu00.png", Asset_RectKaru00, Asset_ImageAtlasDefault },
    { "assets/karu01.png", Asset_RectKaru01, Asset_ImageAtlasDefault },
    { "assets/karu02.png", Asset_RectKaru02, Asset_ImageAtlasDefault },
    { "assets/karu10.png", Asset_RectKaru10, Asset_ImageAtlasDefault },
    { "assets/karu11.png", Asset_RectKaru11, Asset_ImageAtlasDefault },
    { "assets/karu12.png", Asset_RectKaru12, Asset_ImageAtlasDefault },
    { "assets/karu20.png", Asset_RectKaru20, Asset_ImageAtlasDefault },
    { "assets/karu21.png", Asset_RectKaru21, Asset_ImageAtlasDefault },
    { "assets/karu22.png", Asset_RectKaru22, Asset_ImageAtlasDefault },
    { "assets/karu30.png", Asset_RectKaru30, Asset_ImageAtlasDefault },
    { "assets/karu31.png", Asset_RectKaru31, Asset_ImageAtlasDefault },
    { "assets/karu32.png", Asset_RectKaru32, Asset_ImageAtlasDefault },
};


int main() {
    loaded_font Font = AllocateFontFromFile("assets/DroidSansMono.ttf");
    loaded_atlas_image AtlasImages[ArrayCount(ImageParams)];
    u32 ImageUserDataCount = 0;
    for (u32 i = 0; i < ArrayCount(ImageParams); ++i) {
        AtlasImages[i] = LoadAtlasImage(ImageParams[i].Filename, ImageParams[i].Id, ImageParams[i].TargetAtlasId);
    }
    
    // NOTE(Momo): Atlas
    atlas_builder<128> Atlas_ = {};
    auto* Atlas = &Atlas_;
    Begin(Atlas);
    {
        for (u32 i = 0; i < ArrayCount(ImageParams); ++i) {
            auto* Image = AtlasImages + i;
            AddEntry(Atlas, Image->Bitmap.Width, Image->Bitmap.Height, Image->Bitmap.Channels, Image->Bitmap.Pixels, &Image->UserData);
        }
        
#if 0
        for (u32 i = 0; i < AtlasFontGlyphCount; ++i ) {
            auto* FontGlyph = AtlasFontGlyphs + i;
            
        }
        
        for (u32 i = Asset_FontRect_a, j = 'a'; i <= Asset_FontRect_z; ++i, ++j)  {
            AddFontGlyph(Atlas, &Font, j, 72.f, (asset_id)i, TargetAtlasAssetId);
        }
        
        for (u32 i = Asset_FontRect_A, j = 'A'; i <= Asset_FontRect_Z; ++i, ++j)  {
            AddFontGlyph(Atlas, &Font, j, 72.f, (asset_id)i, TargetAtlasAssetId);
        }
#endif
    }
    End(Atlas);
    
    
    
    
    
    WriteBitmapToPng(Atlas, "assets/atlas.png");
    WriteInfo(Atlas, "assets/atlas_info.dat");
}