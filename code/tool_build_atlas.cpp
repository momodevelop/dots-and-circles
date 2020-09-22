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
LoadBitmapFromFile(const char* Filename) {
    i32 W, H, C;
    u8* BitmapData = stbi_load(Filename, &W, &H, &C, 0);
    return { (u32)W, (u32)H, (u32)C, BitmapData };
}

static inline void
FreeLoadedBitmapFromFile(loaded_bitmap Bitmap) {
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


struct loaded_atlas_image {
    loaded_bitmap Bitmap;
    yuu_atlas_ud_image UserData;
};

static inline loaded_atlas_image 
LoadAtlasImage(const char* Filename, asset_id AssetId, asset_id TargetAtlasId) 
{
    loaded_atlas_image Ret = {};
    Ret.Bitmap = LoadBitmapFromFile(Filename);
    Ret.UserData.AssetId = AssetId;
    Ret.UserData.AtlasAssetId = TargetAtlasId;
    Ret.UserData.Type = YuuAtlasUserDataType_Image;
    
    return Ret;
}

static inline void
FreeAtlasImage(loaded_atlas_image Img) { 
    FreeLoadedBitmapFromFile(Img.Bitmap);
}

struct loaded_atlas_font {
    loaded_bitmap Bitmap;
    yuu_atlas_ud_font UserData;
};

static inline loaded_atlas_font
LoadAtlasFont(loaded_font Font, f32 FontSize, u32 Codepoint, asset_id AssetId, asset_id TargetAtlasId) {
    loaded_atlas_font Ret = {};
    
    constexpr u32 Channels = 4;
    f32 Scale = stbtt_ScaleForPixelHeight(&Font.Info, FontSize);
    
    i32 Width, Height;
    
    u8* FontBitmapOneCh = stbtt_GetCodepointBitmap(&Font.Info, 0, Scale, 
                                                   Codepoint, &Width, &Height, nullptr, nullptr);
    Defer { stbtt_FreeBitmap( FontBitmapOneCh, nullptr ); };
    
    u32 BitmapDimensions = (u32)(Width * Height);
    u8* FontBitmap = (u8*)malloc(BitmapDimensions * Channels); 
    u8* FontBitmapItr = FontBitmap;
    u32 k = 0;
    for (u32 i = 0; i < BitmapDimensions; ++i ){
        for (u32 j = 0; j < Channels; ++j ) {
            FontBitmapItr[k++] = FontBitmapOneCh[i];
        }
    }
    
    Ret.Bitmap.Width = (u32)Width;
    Ret.Bitmap.Height = (u32)Height;
    Ret.Bitmap.Channels = Channels;
    Ret.Bitmap.Pixels = FontBitmap;
    
    Ret.UserData.Codepoint = Codepoint;
    Ret.UserData.AssetId = AssetId;
    Ret.UserData.AtlasAssetId = TargetAtlasId;
    Ret.UserData.Size = FontSize;
    Ret.UserData.Type = YuuAtlasUserDataType_Font; 
    
    return Ret;
}

static inline void 
FreeAtlasFont(loaded_atlas_font AtlasFont) {
    free(AtlasFont.Bitmap.Pixels);
}

// NOTE(Momo): Data stuff
struct {
    const char* Filename;
    asset_id Id;
    asset_id TargetAtlasId;
} ImageParams[] = {
    { "assets/ryoji.png",  Asset_RectRyoji, Asset_ImageAtlasDefault },
    { "assets/yuu.png",    Asset_RectYuu,    Asset_ImageAtlasDefault },
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

struct {
    u32 Codepoint;
    f32 FontSize;
    asset_id Id;
    asset_id TargetAtlasId;
} FontParams[] = {
    { 'a', 72, Asset_FontRect_a, Asset_ImageAtlasDefault },
    { 'b', 72, Asset_FontRect_b, Asset_ImageAtlasDefault },
    { 'c', 72, Asset_FontRect_c, Asset_ImageAtlasDefault },
    { 'd', 72, Asset_FontRect_d, Asset_ImageAtlasDefault },
    { 'e', 72, Asset_FontRect_e, Asset_ImageAtlasDefault },
    { 'f', 72, Asset_FontRect_f, Asset_ImageAtlasDefault },
    { 'g', 72, Asset_FontRect_g, Asset_ImageAtlasDefault },
    { 'h', 72, Asset_FontRect_h, Asset_ImageAtlasDefault },
    { 'i', 72, Asset_FontRect_i, Asset_ImageAtlasDefault },
    { 'j', 72, Asset_FontRect_j, Asset_ImageAtlasDefault },
    { 'k', 72, Asset_FontRect_k, Asset_ImageAtlasDefault },
    { 'l', 72, Asset_FontRect_l, Asset_ImageAtlasDefault },
    { 'm', 72, Asset_FontRect_m, Asset_ImageAtlasDefault },
    { 'n', 72, Asset_FontRect_n, Asset_ImageAtlasDefault },
    { 'o', 72, Asset_FontRect_o, Asset_ImageAtlasDefault },
    { 'p', 72, Asset_FontRect_p, Asset_ImageAtlasDefault },
    { 'q', 72, Asset_FontRect_q, Asset_ImageAtlasDefault },
    { 'r', 72, Asset_FontRect_r, Asset_ImageAtlasDefault },
    { 's', 72, Asset_FontRect_s, Asset_ImageAtlasDefault },
    { 't', 72, Asset_FontRect_t, Asset_ImageAtlasDefault },
    { 'u', 72, Asset_FontRect_u, Asset_ImageAtlasDefault },
    { 'v', 72, Asset_FontRect_v, Asset_ImageAtlasDefault },
    { 'w', 72, Asset_FontRect_w, Asset_ImageAtlasDefault },
    { 'x', 72, Asset_FontRect_x, Asset_ImageAtlasDefault },
    { 'y', 72, Asset_FontRect_y, Asset_ImageAtlasDefault },
    { 'z', 72, Asset_FontRect_z, Asset_ImageAtlasDefault },
    
    { 'A', 72, Asset_FontRect_A, Asset_ImageAtlasDefault },
    { 'B', 72, Asset_FontRect_B, Asset_ImageAtlasDefault },
    { 'C', 72, Asset_FontRect_C, Asset_ImageAtlasDefault },
    { 'D', 72, Asset_FontRect_D, Asset_ImageAtlasDefault },
    { 'E', 72, Asset_FontRect_E, Asset_ImageAtlasDefault },
    { 'F', 72, Asset_FontRect_F, Asset_ImageAtlasDefault },
    { 'G', 72, Asset_FontRect_G, Asset_ImageAtlasDefault },
    { 'H', 72, Asset_FontRect_H, Asset_ImageAtlasDefault },
    { 'I', 72, Asset_FontRect_I, Asset_ImageAtlasDefault },
    { 'J', 72, Asset_FontRect_J, Asset_ImageAtlasDefault },
    { 'K', 72, Asset_FontRect_K, Asset_ImageAtlasDefault },
    { 'L', 72, Asset_FontRect_L, Asset_ImageAtlasDefault },
    { 'M', 72, Asset_FontRect_M, Asset_ImageAtlasDefault },
    { 'N', 72, Asset_FontRect_N, Asset_ImageAtlasDefault },
    { 'O', 72, Asset_FontRect_O, Asset_ImageAtlasDefault },
    { 'P', 72, Asset_FontRect_P, Asset_ImageAtlasDefault },
    { 'Q', 72, Asset_FontRect_Q, Asset_ImageAtlasDefault },
    { 'R', 72, Asset_FontRect_R, Asset_ImageAtlasDefault },
    { 'S', 72, Asset_FontRect_S, Asset_ImageAtlasDefault },
    { 'T', 72, Asset_FontRect_T, Asset_ImageAtlasDefault },
    { 'U', 72, Asset_FontRect_U, Asset_ImageAtlasDefault },
    { 'V', 72, Asset_FontRect_V, Asset_ImageAtlasDefault },
    { 'W', 72, Asset_FontRect_W, Asset_ImageAtlasDefault },
    { 'X', 72, Asset_FontRect_X, Asset_ImageAtlasDefault },
    { 'Y', 72, Asset_FontRect_Y, Asset_ImageAtlasDefault },
    { 'Z', 72, Asset_FontRect_Z, Asset_ImageAtlasDefault },
};


int main() {
    loaded_font LoadedFont = AllocateFontFromFile("assets/DroidSansMono.ttf");
    loaded_atlas_image AtlasImages[ArrayCount(ImageParams)];
    loaded_atlas_font AtlasFonts[ArrayCount(FontParams)];
    
    u32 ImageUserDataCount = 0;
    for (u32 i = 0; i < ArrayCount(ImageParams); ++i)  {
        AtlasImages[i] = LoadAtlasImage(ImageParams[i].Filename, 
                                        ImageParams[i].Id, 
                                        ImageParams[i].TargetAtlasId);
    }
    for (u32 i = 0; i < ArrayCount(FontParams); ++i) {
        AtlasFonts[i] = LoadAtlasFont(LoadedFont, 
                                      FontParams[i].FontSize,
                                      FontParams[i].Codepoint, 
                                      FontParams[i].Id, 
                                      FontParams[i].TargetAtlasId);
    }
    
    
    Defer {
        for (u32 i = 0; i < ArrayCount(ImageParams); ++i) 
            FreeAtlasImage(AtlasImages[i]);
        for (u32 i = 0; i < ArrayCount(ImageParams); ++i) 
            FreeAtlasImage(AtlasImages[i]);
    };
    
    
    // NOTE(Momo): Atlas
    atlas_builder<128> Atlas_ = {};
    auto* Atlas = &Atlas_;
    Begin(Atlas);
    {
        for (u32 i = 0; i < ArrayCount(ImageParams); ++i) {
            auto* Image = AtlasImages + i;
            AddEntry(Atlas, Image->Bitmap.Width, Image->Bitmap.Height, Image->Bitmap.Channels, Image->Bitmap.Pixels, &Image->UserData);
        }
        
        for(u32 i = 0; i < ArrayCount(FontParams); ++i) {
            auto* Font = AtlasFonts + i;
            AddEntry(Atlas, Font->Bitmap.Width, Font->Bitmap.Height, Font->Bitmap.Channels, Font->Bitmap.Pixels, &Font->UserData);
            
        }
    }
    End(Atlas);
    
    
    WriteBitmapToPng(Atlas, "assets/atlas.png");
    WriteInfo(Atlas, "assets/atlas_info.dat");
}