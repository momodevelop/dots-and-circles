#include "tool_build_assets.h"
#include "mm_bitwise.h"
#include "mm_rect_packer.h"

static inline rect2u
Rect2u(rp_rect Rect) {
	return { 
        Rect.X, 
        Rect.Y, 
        Rect.X + Rect.W, 
        Rect.Y + Rect.H 
    };    
}

////////
struct loaded_font {
	stbtt_fontinfo Info;
    void* Data;
};

static inline maybe<loaded_font>
AllocateFontFromFile(const char* Filename) {
	stbtt_fontinfo Font;
    FILE* FontFile = nullptr; 
	fopen_s(&FontFile, Filename, "rb");
    if (FontFile == nullptr) {
        return No();
    }
    Defer { fclose(FontFile); };
	    
    fseek(FontFile, 0, SEEK_END);
    auto Size = ftell(FontFile);
    fseek(FontFile, 0, SEEK_SET);
    
    void* Buffer = malloc(Size);
    fread(Buffer, Size, 1, FontFile);
    stbtt_InitFont(&Font, (u8*)Buffer, 0);
    
    loaded_font Ret = { Font, Buffer };

    return Yes(Ret);
}

static inline void
FreeFont(loaded_font Font) {
    free(Font.Data);
}


// NOTE(Momo):  Atlas stuff ////////////////////////
enum atlas_context_type {
    AtlasContextType_Image,
    AtlasContextType_Font,
};

struct atlas_context_image {
    atlas_context_type Type;
    const char* Filename;
    atlas_rect_id Id;
    texture_id TextureId;
    u8* Texture;
};


struct atlas_context_font {
    atlas_context_type Type;
    font_id FontId;
    texture_id TextureId;
    f32 RasterScale;
    u32 Codepoint;
    loaded_font LoadedFont;
    u8* Texture;
};    



static inline void  
Init(rp_rect* Rect, atlas_context_image* Context){
    i32 W, H, C;
    stbi_info(Context->Filename, &W, &H, &C);
    Rect->W = (u32)W;
    Rect->H = (u32)H;
    Rect->UserData = Context;
}

static inline void
Init(rp_rect* Rect, atlas_context_font* Context){
    i32 ix0, iy0, ix1, iy1;
    stbtt_GetCodepointTextureBox(&Context->LoadedFont.Info, 
			Context->Codepoint, 
			Context->RasterScale, 
			Context->RasterScale, 
			&ix0, &iy0, &ix1, &iy1);
    
    Rect->W= (u32)(ix1 - ix0);
    Rect->H= (u32)(iy1 - iy0);
    Rect->UserData = Context;
}


static inline void 
WriteSubTextureToAtlas(u8** AtlasMemory, u32 AtlasWidth, u32 AtlasHeight,
                      u8* TextureMemory, rp_rect TextureRect) 
{
    i32 j = 0;
    for (u32 y = TextureRect.Y; y < TextureRect.Y + TextureRect.H; ++y) {
        for (u32 x = TextureRect.X; x < TextureRect.X + TextureRect.W; ++x) {
            u32 Index = TwoToOne(y, x, AtlasWidth) * 4;
            Assert(Index < (AtlasWidth * AtlasHeight * 4));
            for (u32 c = 0; c < 4; ++c) {
                (*AtlasMemory)[Index + c] = TextureMemory[j++];
            }
        }
    }
}


static inline u8*
GenerateAtlas(const rp_rect* Rects, usize RectCount, u32 Width, u32 Height) {
    u32 AtlasSize = Width * Height * 4;
    u8* AtlasMemory = (u8*)malloc(AtlasSize);
    
    
    for (u32 i = 0; i < RectCount; ++i) {
        auto Rect = Rects[i];
        
        auto Type = *(atlas_context_type*)Rect.UserData;
        switch(Type) {
            case AtlasContextType_Image: {
                auto* Context = (atlas_context_image*)Rect.UserData;
                i32 W, H, C;
                u8* TextureMemory = stbi_load(Context->Filename, &W, &H, &C, 0);
                Defer { stbi_image_free(TextureMemory); };
                WriteSubTextureToAtlas(&AtlasMemory, Width, Height, TextureMemory, Rect);
            } break;
            case AtlasContextType_Font: {
                auto* Context = (atlas_context_font*)Rect.UserData; 
                constexpr u32 Channels = 4;
                
                i32 W, H;
                u8* FontTextureOneCh = stbtt_GetCodepointTexture(&Context->LoadedFont.Info, 
                                                               Context->RasterScale,
                                                               Context->RasterScale, 
                                                               Context->Codepoint, 
                                                               &W, &H, nullptr, nullptr);
                Defer { stbtt_FreeTexture( FontTextureOneCh, nullptr ); };
                
                u32 TextureDimensions = (u32)(W * H);
                u8* FontTexture = (u8*)malloc(TextureDimensions * Channels); 
                Defer { free(FontTexture); };
                
                u8* FontTextureItr = FontTexture;
                for (u32 j = 0, k = 0; j < TextureDimensions; ++j ){
                    for (u32 l = 0; l < Channels; ++l ) {
                        FontTextureItr[k++] = FontTextureOneCh[j];
                    }
                }
                WriteSubTextureToAtlas(&AtlasMemory, Width, Height, FontTexture, Rect);
                
            } break;
            
        }
        
    }
    
    return AtlasMemory;
    
}

int main() {
    printf("tool_build_assets start!\n");

    maybe<loaded_font> LoadedFont = AllocateFontFromFile("assets/DroidSansMono.ttf");
    if (!LoadedFont) {
        printf("Failed to load font\n");
        return 1; 
    }
    f32 FontPixelScale = stbtt_ScaleForPixelHeight(&LoadedFont.Item.Info, 1.f);
    
    Defer { FreeFont(LoadedFont.Item); };
    
    atlas_context_image AtlasImageContexts[] = {
        { AtlasContextType_Image, "assets/ryoji.png",  AtlasRect_Ryoji, Texture_AtlasDefault },
        { AtlasContextType_Image, "assets/yuu.png",    AtlasRect_Yuu,    Texture_AtlasDefault },
        { AtlasContextType_Image, "assets/karu00.png", AtlasRect_Karu00, Texture_AtlasDefault },
        { AtlasContextType_Image, "assets/karu01.png", AtlasRect_Karu01, Texture_AtlasDefault },
        { AtlasContextType_Image, "assets/karu02.png", AtlasRect_Karu02, Texture_AtlasDefault },
        { AtlasContextType_Image, "assets/karu10.png", AtlasRect_Karu10, Texture_AtlasDefault },
        { AtlasContextType_Image, "assets/karu11.png", AtlasRect_Karu11, Texture_AtlasDefault },
        { AtlasContextType_Image, "assets/karu12.png", AtlasRect_Karu12, Texture_AtlasDefault },
        { AtlasContextType_Image, "assets/karu20.png", AtlasRect_Karu20, Texture_AtlasDefault },
        { AtlasContextType_Image, "assets/karu21.png", AtlasRect_Karu21, Texture_AtlasDefault },
        { AtlasContextType_Image, "assets/karu22.png", AtlasRect_Karu22, Texture_AtlasDefault },
        { AtlasContextType_Image, "assets/karu30.png", AtlasRect_Karu30, Texture_AtlasDefault },
        { AtlasContextType_Image, "assets/karu31.png", AtlasRect_Karu31, Texture_AtlasDefault },
        { AtlasContextType_Image, "assets/karu32.png", AtlasRect_Karu32, Texture_AtlasDefault },
        { AtlasContextType_Image, "assets/player_white.png", AtlasRect_PlayerDot, Texture_AtlasDefault },
        { AtlasContextType_Image, "assets/player_black.png", AtlasRect_PlayerCircle, Texture_AtlasDefault },
        { AtlasContextType_Image, "assets/bullet_dot.png", AtlasRect_BulletDot, Texture_AtlasDefault },
        { AtlasContextType_Image, "assets/bullet_circle.png", AtlasRect_BulletCircle, Texture_AtlasDefault },
        { AtlasContextType_Image, "assets/enemy.png", AtlasRect_Enemy, Texture_AtlasDefault },
    };
    atlas_context_font AtlasFontContexts[Codepoint_Count];

    constexpr usize TotalRects = ArrayCount(AtlasImageContexts) + ArrayCount(AtlasFontContexts);
    
    rp_rect PackedRects[TotalRects];
       usize PackedRectCount = 0;
    {
        for (u32 i = 0; i < ArrayCount(AtlasImageContexts); ++i ) {
            Init(PackedRects + PackedRectCount++, AtlasImageContexts + i);
        }
        for (u32 i = 0; i < ArrayCount(AtlasFontContexts); ++i) {
            auto* Font = AtlasFontContexts + i;
            Font->Type = AtlasContextType_Font;
            Font->LoadedFont = LoadedFont.Item;
            Font->Codepoint = Codepoint_Start + i;
            Font->RasterScale = stbtt_ScaleForPixelHeight(&LoadedFont.Item.Info, 72.f);
            Font->FontId = Font_Default;
            Font->TextureId = Texture_AtlasDefault;
            Init(PackedRects + PackedRectCount++, Font);
        }
    }
    
    // NOTE(Momo): Pack rects
    u32 AtlasWidth = 1024;
    u32 AtlasHeight = 1024;
    {
        constexpr usize NodeCount = ArrayCount(PackedRects) + 1;
        
        rp_node RectPackNodes[NodeCount];
        rp_context RectPackContext = rp_CreateRectPacker(AtlasWidth, AtlasHeight, RectPackNodes, NodeCount);
        if (!rp_Pack(&RectPackContext, PackedRects, PackedRectCount, MmrpSort_Height)) {
            printf("Failed to generate texture\n");
            return 1;
        }
    }
   
    // NOTE(Momo): Generate atlas from rects
    u8* AtlasTexture = GenerateAtlas(PackedRects, PackedRectCount, AtlasWidth, AtlasHeight);
#if 1
    stbi_write_png("test.png", AtlasWidth, AtlasHeight, 4, AtlasTexture, AtlasWidth*4);
    printf("Written test atlas: test.png\n");
#endif
    
                    
    ab_context AssetBuilder_ = {};
    auto* AssetBuilder = &AssetBuilder_;
    Begin(AssetBuilder, "yuu", "MOMO");
    {
        WriteTexture(AssetBuilder, Texture_Ryoji, "assets/ryoji.png");
        WriteTexture(AssetBuilder, Texture_Yuu, "assets/yuu.png");
        WriteTexture(AssetBuilder, Texture_AtlasDefault, AtlasWidth, AtlasHeight, 4, AtlasTexture);
        for(u32 i = 0; i <  PackedRectCount; ++i) {
            auto Rect = PackedRects[i];
            auto Type = *(atlas_context_type*)Rect.UserData;
            switch(Type) {
                case AtlasContextType_Image: {
                    auto* Image = (atlas_context_image*)Rect.UserData;
                    rect2u AtlasRect = Rect2u(Rect);
                    WriteAtlasRect(AssetBuilder, Image->Id, Image->TextureId, AtlasRect);
                    
                } break;
                case AtlasContextType_Font: {
                    auto* Font  = (atlas_context_font*)Rect.UserData;
                    
                    i32 Advance;
                    i32 LeftSideBearing; 
                    stbtt_GetCodepointHMetrics(&LoadedFont.Item.Info, Font->Codepoint, &Advance, &LeftSideBearing);
                    
                    rect2i Box;
                    stbtt_GetCodepointBox(&LoadedFont.Item.Info, Font->Codepoint, &Box.Min.X, &Box.Min.Y, &Box.Max.X, &Box.Max.Y);
                    
                    WriteFontGlyph(AssetBuilder, Font->FontId, Font->TextureId, Font->Codepoint, FontPixelScale * Advance,
                                   FontPixelScale * LeftSideBearing,
                                   Rect2u(Rect), 
                                   Rect2f(Box) * FontPixelScale);
                    
                } break;
                
            }
        }
        
        i32 Ascent, Descent, LineGap;
        stbtt_GetFontVMetrics(&LoadedFont.Item.Info, &Ascent, &Descent, &LineGap); 
        
        rect2i BoundingBox = {}; 
        stbtt_GetFontBoundingBox(&LoadedFont.Item.Info, 
            &BoundingBox.Min.X,
            &BoundingBox.Min.Y,
            &BoundingBox.Max.X,
            &BoundingBox.Max.Y
        );

        WriteFont(AssetBuilder, Font_Default, 
                Ascent * FontPixelScale, 
                Descent * FontPixelScale, 
                LineGap * FontPixelScale
        ); 
        
        for (u32 i = Codepoint_Start; i <= Codepoint_End; ++i) {
            for(u32 j = Codepoint_Start; j <= Codepoint_End; ++j) {
                i32 Kerning = stbtt_GetCodepointKernAdvance(&LoadedFont.Item.Info, (i32)i, (i32)j);
                WriteFontKerning(AssetBuilder, Font_Default, i, j, Kerning);
            }
        }
        
        
        
    }
    End(AssetBuilder);
    
    printf("tool_build_assets done!\n");
    
    return 0;
    
}

