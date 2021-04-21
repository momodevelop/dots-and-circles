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

#include "tool_build_assets_util.h"
#include "tool_build_assets_atlas.h"
#include "tool_build_assets_msg.h"
#include "tool_build_assets_asset_builder.h"

#define GENERATE_TEST_PNG 1

#define ToolBuildAssets_MemorySize Megabytes(8)
#define MemCheck printf("[Memcheck] Line %d: %d bytes used\n", __LINE__, Arena.Used);


int main() {
    printf("Start!\n");
    
    void* Memory = malloc(ToolBuildAssets_MemorySize);
    if (!Memory) {
        printf("Failed to initialize memory");
        return 1; 
    }
    Defer { free(Memory); };
    
    arena Arena = Arena_Create(Memory, ToolBuildAssets_MemorySize);
    
    //~ NOTE(Momo): Initialize context for images in atlas
    atlas_context_image AtlasImageContexts[] = {
        { AtlasContextType_Image, "assets/ryoji.png",  AtlasAabb_Ryoji, Texture_AtlasDefault },
        { AtlasContextType_Image, "assets/yuu.png",    AtlasAabb_Yuu,    Texture_AtlasDefault },
        { AtlasContextType_Image, "assets/karu00.png", AtlasAabb_Karu00, Texture_AtlasDefault },
        { AtlasContextType_Image, "assets/karu01.png", AtlasAabb_Karu01, Texture_AtlasDefault },
        { AtlasContextType_Image, "assets/karu02.png", AtlasAabb_Karu02, Texture_AtlasDefault },
        { AtlasContextType_Image, "assets/karu10.png", AtlasAabb_Karu10, Texture_AtlasDefault },
        { AtlasContextType_Image, "assets/karu11.png", AtlasAabb_Karu11, Texture_AtlasDefault },
        { AtlasContextType_Image, "assets/karu12.png", AtlasAabb_Karu12, Texture_AtlasDefault },
        { AtlasContextType_Image, "assets/karu20.png", AtlasAabb_Karu20, Texture_AtlasDefault },
        { AtlasContextType_Image, "assets/karu21.png", AtlasAabb_Karu21, Texture_AtlasDefault },
        { AtlasContextType_Image, "assets/karu22.png", AtlasAabb_Karu22, Texture_AtlasDefault },
        { AtlasContextType_Image, "assets/karu30.png", AtlasAabb_Karu30, Texture_AtlasDefault },
        { AtlasContextType_Image, "assets/karu31.png", AtlasAabb_Karu31, Texture_AtlasDefault },
        { AtlasContextType_Image, "assets/karu32.png", AtlasAabb_Karu32, Texture_AtlasDefault },
        { AtlasContextType_Image, "assets/player_white.png", AtlasAabb_PlayerDot, Texture_AtlasDefault },
        { AtlasContextType_Image, "assets/player_black.png", AtlasAabb_PlayerCircle, Texture_AtlasDefault },
        { AtlasContextType_Image, "assets/bullet_dot.png", AtlasAabb_BulletDot, Texture_AtlasDefault },
        { AtlasContextType_Image, "assets/bullet_circle.png", AtlasAabb_BulletCircle, Texture_AtlasDefault },
        { AtlasContextType_Image, "assets/enemy.png", AtlasAabb_Enemy, Texture_AtlasDefault },
    };
    
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
    
    //~ NOTE(Momo): Initialize contexts for font images in atlas
    atlas_context_font AtlasFontContexts[FontGlyph_Count];
    
    const u32 AabbCount = ArrayCount(AtlasImageContexts) + ArrayCount(AtlasFontContexts);
    aabb2u Aabbs[AabbCount] = {};
    void* UserDatas[AabbCount] = {};
    u32 AabbCounter = 0;
    for (u32 I = 0; I < ArrayCount(AtlasImageContexts); ++I ) {
        atlas_context_image* Context = AtlasImageContexts + I;
        aabb2u* Aabb = Aabbs + AabbCounter;
        
        i32 W, H, C;
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
        Font->TextureId = Texture_AtlasDefault;
        
        aabb2u* Aabb = Aabbs + AabbCounter;
        i32 ix0, iy0, ix1, iy1;
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
#if GENERATE_TEST_PNG 
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
                    Tba_AssetBuilderWriteAtlasAabb(AssetBuilder, 
                                                   Image->Id, 
                                                   Image->TextureId, 
                                                   Aabb);
                } break;
                case AtlasContextType_Font: {
                    auto* Font  = (atlas_context_font*)UserDatas[i];
                    
                    i32 Advance;
                    i32 LeftSideBearing; 
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
        i32 Ascent, Descent, LineGap;
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
                i32 Kerning = stbtt_GetCodepointKernAdvance(&LoadedFont.Info, (i32)i, (i32)j);
                Tba_AssetBuilderWriteFontKerning(AssetBuilder, Font_Default, i, j, Kerning);
            }
        }
    }
    
    
    //NOTE(Momo): Anime
    printf("[Build Assets] Writing Anime...\n");
    {
        
        // Test frames: Karu_front
        atlas_aabb_id Frames[] = {
            AtlasAabb_Karu30,
            AtlasAabb_Karu31,
            AtlasAabb_Karu32,
        };
        
        Tba_AssetBuilderWriteAnime(AssetBuilder, 
                                   Anime_KaruFront,
                                   Frames,
                                   ArrayCount(Frames));
    }
    
    // NOTE(Momo): Msg
    printf("[Build Assets] Writing Msg...\n");
    {
        for(u32 I = 0; I < ArrayCount(Tba_MessageContexts); ++I) {
            tba_message_context* Ctx = Tba_MessageContexts + I;
            Tba_AssetBuilderWriteMsg(AssetBuilder,
                                     Ctx->Id,
                                     Ctx->Str);
        }
    }
    
    Tba_AssetBuilderEnd(AssetBuilder);
    printf("[Build Assets] Assets Built\n");
    return 0;
    
}

