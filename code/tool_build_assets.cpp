#include <stdio.h>
#include <stdlib.h>
#include "momo.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "game_assets_types.h"  

#include "tool_build_assets_util.h"
#include "tool_build_assets_atlas.h"
#include "tool_build_assets_msg.h"
#include "tool_build_assets_asset_builder.h"


#define Tba_GenerateTestPng 1
#define Tba_MemorySize Megabytes(8)
#define Tba_MemCheck printf("[Memcheck] Line %d: %d bytes used\n", __LINE__, Arena.Used)


int main() {
    printf("Start!\n");
    
    void* Memory = malloc(Tba_MemorySize);
    if (!Memory) {
        printf("Failed to initialize memory");
        return 1; 
    }
    Defer { free(Memory); };
    
    arena Arena = Arena_Create(Memory, Tba_MemorySize);
    
    //~ NOTE(Momo): Load font
    
    // TODO(Momo): Preload stuff related to font that we actually need and then free
    // loaded font's data to be more memory efficient.
    loaded_font LoadedFont = {};
    if (!Tba_LoadFont(&LoadedFont, &Arena, "assets/DroidSansMono.ttf")) 
    {
        printf("[Atlas] Failed to load font\n");
        return 1; 
    }
    f32 FontPixelScale = stbtt_ScaleForPixelHeight(&LoadedFont.Info, 1.f); 
    
    //~ NOTE(Momo): Initialize context for images in atlas
    
    
    atlas_context_image AtlasImageContexts[ArrayCount(Tba_ImageContexts)] = {};
    for (u32 I = 0; I < ArrayCount(Tba_ImageContexts); ++I) {
        tba_image_context* TbaCtx = Tba_ImageContexts + I;
        atlas_context_image* AtlasCtx = AtlasImageContexts + I;
        
        AtlasCtx->Type = AtlasContextType_Image;
        AtlasCtx->Filename = TbaCtx->Filename;
        AtlasCtx->Id = TbaCtx->ImageId;
        AtlasCtx->TextureId = TbaCtx->TextureId;
    }
    
    
    
    //~ NOTE(Momo): Initialize contexts for font images in atlas
    atlas_context_font AtlasFontContexts[FontGlyph_Count];
    
    const u32 AabbCount = ArrayCount(AtlasImageContexts) + ArrayCount(AtlasFontContexts);
    aabb2u Aabbs[AabbCount] = {};
    void* UserDatas[AabbCount] = {};
    u32 AabbCounter = 0;
    for (u32 I = 0; I < ArrayCount(AtlasImageContexts); ++I ) {
        atlas_context_image* Context = AtlasImageContexts + I;
        aabb2u* Aabb = Aabbs + AabbCounter;
        
        s32 W, H, C;
        stbi_info(Context->Filename, &W, &H, &C);
        
        (*Aabb) = Aabb2u_CreateWH((u32)W, (u32)H);
        
        
        UserDatas[AabbCounter] = Context;
        
        ++AabbCounter;
    }
    
    for (u32 I = 0; I < ArrayCount(AtlasFontContexts); ++I) {
        atlas_context_font* Font = AtlasFontContexts + I;
        Font->Type = AtlasContextType_Font;
        Font->LoadedFont = &LoadedFont;
        Font->Codepoint = FontGlyph_CodepointStart + I;
        Font->RasterScale = stbtt_ScaleForPixelHeight(&LoadedFont.Info, 72.f);
        Font->FontId = Font_Default;
        Font->ImageId = image_id(Image_FontStart + I);
        Font->TextureId = Texture_AtlasDefault;
        
        aabb2u* Aabb = Aabbs + AabbCounter;
        s32 ix0, iy0, ix1, iy1;
        stbtt_GetCodepointTextureBox(&Font->LoadedFont->Info, 
                                     Font->Codepoint, 
                                     Font->RasterScale, 
                                     Font->RasterScale, 
                                     &ix0, &iy0, &ix1, &iy1);
        
        (*Aabb) = Aabb2u_CreateWH((u32)(ix1 - ix0), (u32)(iy1 - iy0));
        UserDatas[AabbCounter] = Font;
        
        ++AabbCounter;
    }
    
    
    //~ NOTE(Momo): Pack rects
    u32 AtlasWidth = 1024;
    u32 AtlasHeight = 1024;
    if (!AabbPacker_Pack(&Arena,
                         AtlasWidth,
                         AtlasHeight,
                         Aabbs,
                         AabbCount, 
                         AabbPackerSortType_Height)) 
    {
        printf("[Atlas] Failed to generate texture\n");
        return 1;
    }
    
    // NOTE(Momo): Generate atlas from rects
    u8* AtlasTexture = Tba_GenerateAtlas(&Arena,
                                         Aabbs, 
                                         UserDatas,
                                         AabbCount, 
                                         AtlasWidth, 
                                         AtlasHeight);
    if (!AtlasTexture) {
        printf("[Atlas] Cannot generate atlas, not enough memory\n");
        return 1;
    }
#if Tba_GenerateTestPng 
    stbi_write_png("test.png", AtlasWidth, AtlasHeight, 4, AtlasTexture, AtlasWidth*4);
    printf("[Atlas] Written test atlas: test.png\n");
#endif
    printf("[Atlas] Atlas generated\n");
    
    //~ NOTE(Momo): Actual asset building
    printf("[Build Assets] Building Assets\n");
    
    tba_asset_builder AssetBuilder_ = {};
    tba_asset_builder* AssetBuilder = &AssetBuilder_;
    Tba_AssetBuilderBegin(AssetBuilder, "yuu", "MOMO");
    
    // NOTE(Momo): Write atlas
    {
        Tba_AssetBuilderWriteTexture(AssetBuilder, 
                                     Texture_AtlasDefault, 
                                     AtlasWidth, 
                                     AtlasHeight, 
                                     4, 
                                     AtlasTexture);
        
        // NOTE(Momo): Write atlas aabb for images in font
        for(u32 i = 0; i <  AabbCount; ++i) {
            aabb2u Aabb = Aabbs[i];
            auto Type = *(atlas_context_type*)UserDatas[i];
            switch(Type) {
                case AtlasContextType_Image: {
                    auto* Image = (atlas_context_image*)UserDatas[i];
                    Tba_AssetBuilderWriteImage(AssetBuilder, 
                                               Image->Id, 
                                               Image->TextureId, 
                                               Aabb);
                } break;
                case AtlasContextType_Font: {
                    auto* Font  = (atlas_context_font*)UserDatas[i];
                    
                    s32 Advance;
                    s32 LeftSideBearing; 
                    stbtt_GetCodepointHMetrics(&LoadedFont.Info, 
                                               Font->Codepoint, 
                                               &Advance, 
                                               &LeftSideBearing);
                    
                    aabb2i Box;
                    stbtt_GetCodepointBox(&LoadedFont.Info, 
                                          Font->Codepoint, 
                                          &Box.Min.X, 
                                          &Box.Min.Y, 
                                          &Box.Max.X, 
                                          &Box.Max.Y);
                    
                    aabb2f ScaledBox = Aabb2f_Mul(Aabb2i_To_Aabb2f(Box), FontPixelScale);
                    Tba_AssetBuilderWriteFontGlyph(AssetBuilder, 
                                                   Font->FontId, 
                                                   Font->ImageId,
                                                   Font->TextureId,
                                                   Font->Codepoint, FontPixelScale * Advance,
                                                   FontPixelScale * LeftSideBearing,
                                                   Aabb,
                                                   ScaledBox);
                    
                } break;
                
            }
        }
    }
    
    // NOTE(Momo): Font info
    {
        s32 Ascent, Descent, LineGap;
        stbtt_GetFontVMetrics(&LoadedFont.Info, &Ascent, &Descent, &LineGap); 
        
        aabb2i BoundingBox = {}; 
        stbtt_GetFontBoundingBox(&LoadedFont.Info, 
                                 &BoundingBox.Min.X,
                                 &BoundingBox.Min.Y,
                                 &BoundingBox.Max.X,
                                 &BoundingBox.Max.Y
                                 );
        printf("[Build Assets] Writing font information...\n");
        Tba_AssetBuilderWriteFont(AssetBuilder, Font_Default, 
                                  Ascent * FontPixelScale, 
                                  Descent * FontPixelScale, 
                                  LineGap * FontPixelScale); 
    }
    
    // NOTE(Momo): Kerning info
    {
        printf("[Build Assets] Writing kerning...\n");
        for (u32 i = FontGlyph_CodepointStart;
             i <= FontGlyph_CodepointEnd; 
             ++i) 
        {
            for(u32 j = FontGlyph_CodepointStart;
                j <= FontGlyph_CodepointEnd; 
                ++j) 
            {
                s32 Kerning = stbtt_GetCodepointKernAdvance(&LoadedFont.Info, (s32)i, (s32)j);
                Tba_AssetBuilderWriteFontKerning(AssetBuilder, Font_Default, i, j, Kerning);
            }
        }
    }
    
    
    //NOTE(Momo): Anime
    printf("[Build Assets] Writing Anime...\n");
    {
        for(u32 I = 0; I < ArrayCount(Tba_AnimeContexts); ++I) {
            tba_anime_context * Ctx = Tba_AnimeContexts + I;
            Tba_AssetBuilderWriteAnime(AssetBuilder, 
                                       Ctx->Id,
                                       Ctx->Frames,
                                       Ctx->FrameCount);
        }
    }
    
    // NOTE(Momo): Msg
    printf("[Build Assets] Writing Msg...\n");
    {
        for(u32 I = 0; I < ArrayCount(Tba_MsgContexts); ++I) {
            tba_msg_context* Ctx = Tba_MsgContexts + I;
            Tba_AssetBuilderWriteMsg(AssetBuilder,
                                     Ctx->Id,
                                     Ctx->Str);
        }
    }
    
    
    printf("[Build Assets] Writing Sound...\n");
    {
        for(u32 I = 0; I < ArrayCount(Tba_SoundContexts); ++I) {
            tba_sound_context* Ctx = Tba_SoundContexts + I;
            
            arena_mark Mark = Arena_Mark(&Arena);
            Defer { Arena_Revert(&Mark); };
            
            read_file_result FileResult = {};
            if(!Tba_ReadFileIntoMemory(&FileResult, Mark, Ctx->Filename)) {
                return 1;
            }
            
            wav_load_result WavResult = {};
            if(!Wav_LoadFromMemory(&WavResult,
                                   FileResult.Data,
                                   FileResult.Size)) {
                printf("test1");
                return 1;
            }
            Tba_AssetBuilderWriteWav(AssetBuilder,
                                     Ctx->Id,
                                     &WavResult);
        }
    }
    
    Tba_AssetBuilderEnd(AssetBuilder);
    printf("[Build Assets] Assets Built\n");
    
    Tba_MemCheck;
    return 0;
    
}

