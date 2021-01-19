#include "tool_build_assets.h"
#include "mm_bitwise.h"
#include "mm_aabb_packer.h"

static inline aabb2u
Aabb2u(aabb_packer_aabb Aabb) {
	return { 
        Aabb.X, 
        Aabb.Y, 
        Aabb.X + Aabb.W, 
        Aabb.Y + Aabb.H 
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
    atlas_aabb_id Id;
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
Init(aabb_packer_aabb* Aabb, atlas_context_image* Context){
    i32 W, H, C;
    stbi_info(Context->Filename, &W, &H, &C);
    Aabb->W = (u32)W;
    Aabb->H = (u32)H;
    Aabb->UserData = Context;
}

static inline void
Init(aabb_packer_aabb* Aabb, atlas_context_font* Context){
    i32 ix0, iy0, ix1, iy1;
    stbtt_GetCodepointTextureBox(&Context->LoadedFont.Info, 
			Context->Codepoint, 
			Context->RasterScale, 
			Context->RasterScale, 
			&ix0, &iy0, &ix1, &iy1);
    
    Aabb->W= (u32)(ix1 - ix0);
    Aabb->H= (u32)(iy1 - iy0);
    Aabb->UserData = Context;
}


static inline void 
WriteSubTextureToAtlas(u8** AtlasMemory, u32 AtlasWidth, u32 AtlasHeight,
                      u8* TextureMemory, aabb_packer_aabb TextureAabb) 
{
    i32 j = 0;
    for (u32 y = TextureAabb.Y; y < TextureAabb.Y + TextureAabb.H; ++y) {
        for (u32 x = TextureAabb.X; x < TextureAabb.X + TextureAabb.W; ++x) {
            u32 Index = TwoToOne(y, x, AtlasWidth) * 4;
            Assert(Index < (AtlasWidth * AtlasHeight * 4));
            for (u32 c = 0; c < 4; ++c) {
                (*AtlasMemory)[Index + c] = TextureMemory[j++];
            }
        }
    }
}


static inline u8*
GenerateAtlas(const aabb_packer_aabb* Aabbs, usize AabbCount, u32 Width, u32 Height) {
    u32 AtlasSize = Width * Height * 4;
    u8* AtlasMemory = (u8*)malloc(AtlasSize);
    
    
    for (u32 i = 0; i < AabbCount; ++i) {
        auto Aabb = Aabbs[i];
        
        auto Type = *(atlas_context_type*)Aabb.UserData;
        switch(Type) {
            case AtlasContextType_Image: {
                auto* Context = (atlas_context_image*)Aabb.UserData;
                i32 W, H, C;
                u8* TextureMemory = stbi_load(Context->Filename, &W, &H, &C, 0);
                Defer { stbi_image_free(TextureMemory); };
                WriteSubTextureToAtlas(&AtlasMemory, Width, Height, TextureMemory, Aabb);
            } break;
            case AtlasContextType_Font: {
                auto* Context = (atlas_context_font*)Aabb.UserData; 
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
                WriteSubTextureToAtlas(&AtlasMemory, Width, Height, FontTexture, Aabb);
                
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
    atlas_context_font AtlasFontContexts[Codepoint_Count];

    constexpr usize TotalAabbs = ArrayCount(AtlasImageContexts) + ArrayCount(AtlasFontContexts);
    
    aabb_packer_aabb PackedAabbs[TotalAabbs];
       usize PackedAabbCount = 0;
    {
        for (u32 i = 0; i < ArrayCount(AtlasImageContexts); ++i ) {
            Init(PackedAabbs + PackedAabbCount++, AtlasImageContexts + i);
        }
        for (u32 i = 0; i < ArrayCount(AtlasFontContexts); ++i) {
            auto* Font = AtlasFontContexts + i;
            Font->Type = AtlasContextType_Font;
            Font->LoadedFont = LoadedFont.Item;
            Font->Codepoint = Codepoint_Start + i;
            Font->RasterScale = stbtt_ScaleForPixelHeight(&LoadedFont.Item.Info, 72.f);
            Font->FontId = Font_Default;
            Font->TextureId = Texture_AtlasDefault;
            Init(PackedAabbs + PackedAabbCount++, Font);
        }
    }
    
    // NOTE(Momo): Pack rects
    u32 AtlasWidth = 1024;
    u32 AtlasHeight = 1024;
    {
        constexpr usize NodeCount = ArrayCount(PackedAabbs) + 1;
        
        aabb_packer_node AabbPackNodes[NodeCount];
        aabb_packer AabbPackContext = AabbPacker(AtlasWidth, AtlasHeight, AabbPackNodes, NodeCount);
        if (!Pack(&AabbPackContext, PackedAabbs, PackedAabbCount, AabbPackerSortType_Height)) {
            printf("Failed to generate texture\n");
            return 1;
        }
    }
   
    // NOTE(Momo): Generate atlas from rects
    u8* AtlasTexture = GenerateAtlas(PackedAabbs, PackedAabbCount, AtlasWidth, AtlasHeight);
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
        for(u32 i = 0; i <  PackedAabbCount; ++i) {
            auto Aabb = PackedAabbs[i];
            auto Type = *(atlas_context_type*)Aabb.UserData;
            switch(Type) {
                case AtlasContextType_Image: {
                    auto* Image = (atlas_context_image*)Aabb.UserData;
                    aabb2u AtlasAabb = Aabb2u(Aabb);
                    WriteAtlasAabb(AssetBuilder, Image->Id, Image->TextureId, AtlasAabb);
                    
                } break;
                case AtlasContextType_Font: {
                    auto* Font  = (atlas_context_font*)Aabb.UserData;
                    
                    i32 Advance;
                    i32 LeftSideBearing; 
                    stbtt_GetCodepointHMetrics(&LoadedFont.Item.Info, Font->Codepoint, &Advance, &LeftSideBearing);
                    
                    aabb2i Box;
                    stbtt_GetCodepointBox(&LoadedFont.Item.Info, Font->Codepoint, &Box.Min.X, &Box.Min.Y, &Box.Max.X, &Box.Max.Y);
                    
                    WriteFontGlyph(AssetBuilder, Font->FontId, 
                                   Font->TextureId, 
                                   Font->Codepoint, FontPixelScale * Advance,
                                   FontPixelScale * LeftSideBearing,
                                   Aabb2u(Aabb), 
                                   Aabb2f(Box) * FontPixelScale);
                    
                } break;
                
            }
        }
        
        i32 Ascent, Descent, LineGap;
        stbtt_GetFontVMetrics(&LoadedFont.Item.Info, &Ascent, &Descent, &LineGap); 
        
        aabb2i BoundingBox = {}; 
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

