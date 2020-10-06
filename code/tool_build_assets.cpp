#include "tool_build_assets.h"
#include "ryoji_bitmanip.h"
#include "ryoji_rectpack.h"



////////
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


// NOTE(Momo):  Atlas stuff ////////////////////////
enum atlas_context_type {
    AtlasContextType_Image,
    AtlasContextType_Font,
};

struct atlas_context_image {
    atlas_context_type Type;
    const char* Filename;
    atlas_rect_id Id;
    bitmap_id BitmapId;
    u8* Bitmap;
};

static inline void
AtlasContextImageUnloadCb(void* LoadContext) {
    auto* Context = (atlas_context_image*)LoadContext; 
    stbi_image_free(Context->Bitmap);
}

struct atlas_context_font {
    atlas_context_type Type;
    font_id FontId;
    bitmap_id BitmapId;
    f32 Size;
    u32 Codepoint;
    loaded_font LoadedFont;
    u8* Bitmap;
};    


static inline void
AtlasContextFontUnloadCb(void* LoadContext) {
    auto* Context = (atlas_context_font*)LoadContext; 
    stbtt_FreeBitmap(Context->Bitmap, nullptr);
}



static inline void  
Init(ryyrp_rect* Rect, atlas_context_image* Context){
    i32 W, H, C;
    stbi_info(Context->Filename, &W, &H, &C);
    Rect->W = (u32)W;
    Rect->H = (u32)H;
    Rect->UserData = Context;
}

static inline void
Init(ryyrp_rect* Rect, atlas_context_font* Context){
    i32 ix0, iy0, ix1, iy1;
    f32 Scale = stbtt_ScaleForPixelHeight(&Context->LoadedFont.Info, Context->Size);
    stbtt_GetCodepointBitmapBox(&Context->LoadedFont.Info, Context->Codepoint, Scale, Scale, &ix0, &iy0, &ix1, &iy1);
    
    Rect->W= (u32)(ix1 - ix0);
    Rect->H= (u32)(iy1 - iy0);
    Rect->UserData = Context;
}

static inline void 
WriteSubBitmapToAtlas(u8** AtlasMemory, u32 AtlasWidth, u32 AtlasHeight,
                      u8* BitmapMemory, ryyrp_rect BitmapRect) 
{
    i32 j = 0;
    for (u32 y = BitmapRect.Y; y < BitmapRect.Y + BitmapRect.H; ++y) {
        for (u32 x = BitmapRect.X; x < BitmapRect.X + BitmapRect.W; ++x) {
            u32 Index = TwoToOne(y, x, AtlasWidth) * 4;
            Assert(Index < (AtlasWidth * AtlasHeight * 4));
            for (u32 c = 0; c < 4; ++c) {
                (*AtlasMemory)[Index + c] = BitmapMemory[j++];
            }
        }
    }
}


static inline u8*
GenerateAtlas(const ryyrp_rect* Rects, usize RectCount, u32 Width, u32 Height) {
    u32 AtlasSize = Width * Height * 4;
    u8* AtlasMemory = (u8*)malloc(AtlasSize);
    
    
    for (u32 i = 0; i < RectCount; ++i) {
        auto Rect = Rects[i];
        
        auto Type = *(atlas_context_type*)Rect.UserData;
        switch(Type) {
            case AtlasContextType_Image: {
                auto* Context = (atlas_context_image*)Rect.UserData;
                i32 W, H, C;
                u8* BitmapMemory = stbi_load(Context->Filename, &W, &H, &C, 0);
                Defer { stbi_image_free(BitmapMemory); };
                WriteSubBitmapToAtlas(&AtlasMemory, Width, Height, BitmapMemory, Rect);
            } break;
            case AtlasContextType_Font: {
                auto* Context = (atlas_context_font*)Rect.UserData; 
                constexpr u32 Channels = 4;
                f32 Scale = stbtt_ScaleForPixelHeight(&Context->LoadedFont.Info, Context->Size);
                i32 W, H;
                u8* FontBitmapOneCh = stbtt_GetCodepointBitmap(&Context->LoadedFont.Info, 0, Scale, 
                                                               Context->Codepoint, &W, &H, nullptr, nullptr);
                Defer { stbtt_FreeBitmap( FontBitmapOneCh, nullptr ); };
                
                u32 BitmapDimensions = (u32)(W * H);
                u8* FontBitmap = (u8*)malloc(BitmapDimensions * Channels); 
                Defer { free(FontBitmap); };
                
                u8* FontBitmapItr = FontBitmap;
                for (u32 j = 0, k = 0; j < BitmapDimensions; ++j ){
                    for (u32 l = 0; l < Channels; ++l ) {
                        FontBitmapItr[k++] = FontBitmapOneCh[j];
                    }
                }
                WriteSubBitmapToAtlas(&AtlasMemory, Width, Height, FontBitmap, Rect);
                
            } break;
            
        }
        
    }
    
    return AtlasMemory;
    
}

#if 0
// NOTE(Momo): Rects WILL be sorted after this function
static inline b32
ryyrp_PackDebug(ryyrp_context* Context, ryyrp_rect* Rects, usize RectCount) {
    qsort(Rects, RectCount, sizeof(ryyrp_rect), ryy__SortPred);
    
    
    usize CurrentNodeCount = 0;
    Context->Nodes[CurrentNodeCount++] = { 0, 0, Context->Width, Context->Height };
    
    for (u32 i = 0; i < RectCount; ++i ) {
        auto Rect = Rects[i];
        
        // NOTE(Momo): Iterate the empty spaces backwards to find the best fit index
        usize ChosenSpaceIndex = CurrentNodeCount;
        {
            for (usize  j = 0; j < ChosenSpaceIndex ; ++j ) {
                usize Index = ChosenSpaceIndex - j - 1;
                ryyrp_node Space = Context->Nodes[Index];
                // NOTE(Momo): Check if the image fits
                if (Rect.W <= Space.W && Rect.H <= Space.H) {
                    ChosenSpaceIndex = Index;
                    break;
                }
            }
        }
        
        
        // NOTE(Momo): If an empty space that can fit is found, we remove that space and split.
        if (ChosenSpaceIndex == CurrentNodeCount) {
            return false;
        }
        
        // NOTE(Momo): Swap and pop the chosen space
        ryyrp_node ChosenSpace = Context->Nodes[ChosenSpaceIndex];
        if (CurrentNodeCount > 0) {
            Context->Nodes[ChosenSpaceIndex] = Context->Nodes[CurrentNodeCount-1];
            --CurrentNodeCount;
        }
        
        // NOTE(Momo): Split if not perfect fit
        if (ChosenSpace.W != Rect.W && ChosenSpace.H == Rect.H) {
            // Split right
            ryyrp_node SplitSpaceRight = {
                ChosenSpace.X + Rect.W,
                ChosenSpace.Y,
                ChosenSpace.W - Rect.W,
                ChosenSpace.H
            };
            Context->Nodes[CurrentNodeCount++] = SplitSpaceRight;
        }
        else if (ChosenSpace.W == Rect.W && ChosenSpace.H != Rect.H) {
            // Split down
            ryyrp_node SplitSpaceDown = {
                ChosenSpace.X,
                ChosenSpace.Y + Rect.H,
                ChosenSpace.W,
                ChosenSpace.H - Rect.H
            };
            Context->Nodes[CurrentNodeCount++] = SplitSpaceDown;
        }
        else if (ChosenSpace.W != Rect.W && ChosenSpace.H != Rect.H) {
            // Split right
            ryyrp_node SplitSpaceRight = {
                ChosenSpace.X + Rect.W,
                ChosenSpace.Y,
                ChosenSpace.W - Rect.W,
                Rect.H,
            };
            
            // Split down
            ryyrp_node SplitSpaceDown = {
                ChosenSpace.X,
                ChosenSpace.Y + Rect.H,
                ChosenSpace.W,
                ChosenSpace.H - Rect.H,
            };
            
            // Choose to insert the bigger one first before the smaller one
            u32 RightArea = SplitSpaceRight.W * SplitSpaceRight.H;
            u32 DownArea = SplitSpaceDown.W * SplitSpaceDown.H;
            if (RightArea > DownArea) {
                Context->Nodes[CurrentNodeCount++] = SplitSpaceRight;
                Context->Nodes[CurrentNodeCount++] = SplitSpaceDown;
            }
            else {
                Context->Nodes[CurrentNodeCount++] = SplitSpaceDown;
                Context->Nodes[CurrentNodeCount++] = SplitSpaceRight;
            }
            
        }
        
        // NOTE(Momo): Translate the rect
        Rects[i].X = ChosenSpace.X;
        Rects[i].Y = ChosenSpace.Y;
        
        u8* AtlasBitmap = GenerateAtlas(Rects, RectCount, Context->Width, Context->Height);
        Defer { free(AtlasBitmap); };
        stbi_write_png("test.png", Context->Width, Context->Height, 4, AtlasBitmap, Context->Width*4);
        
    }
    
    return true;
}
#endif


int main() {
    loaded_font LoadedFont = AllocateFontFromFile("assets/DroidSansMono.ttf");
    Defer { FreeFont(LoadedFont); };
    
    atlas_context_image AtlasImageContexts[] = {
        { AtlasContextType_Image, "assets/ryoji.png",  AtlasRect_Ryoji, Bitmap_AtlasDefault },
        { AtlasContextType_Image, "assets/yuu.png",    AtlasRect_Yuu,    Bitmap_AtlasDefault },
        { AtlasContextType_Image, "assets/karu00.png", AtlasRect_Karu00, Bitmap_AtlasDefault },
        { AtlasContextType_Image, "assets/karu01.png", AtlasRect_Karu01, Bitmap_AtlasDefault },
        { AtlasContextType_Image, "assets/karu02.png", AtlasRect_Karu02, Bitmap_AtlasDefault },
        { AtlasContextType_Image, "assets/karu10.png", AtlasRect_Karu10, Bitmap_AtlasDefault },
        { AtlasContextType_Image, "assets/karu11.png", AtlasRect_Karu11, Bitmap_AtlasDefault },
        { AtlasContextType_Image, "assets/karu12.png", AtlasRect_Karu12, Bitmap_AtlasDefault },
        { AtlasContextType_Image, "assets/karu20.png", AtlasRect_Karu20, Bitmap_AtlasDefault },
        { AtlasContextType_Image, "assets/karu21.png", AtlasRect_Karu21, Bitmap_AtlasDefault },
        { AtlasContextType_Image, "assets/karu22.png", AtlasRect_Karu22, Bitmap_AtlasDefault },
        { AtlasContextType_Image, "assets/karu30.png", AtlasRect_Karu30, Bitmap_AtlasDefault },
        { AtlasContextType_Image, "assets/karu31.png", AtlasRect_Karu31, Bitmap_AtlasDefault },
        { AtlasContextType_Image, "assets/karu32.png", AtlasRect_Karu32, Bitmap_AtlasDefault },
    };
    atlas_context_font AtlasFontContexts[Codepoint_Count];
    
    constexpr usize TotalRects = ArrayCount(AtlasImageContexts) + ArrayCount(AtlasFontContexts);
    
    ryyrp_rect PackedRects[TotalRects];
    
    usize PackedRectCount = 0;
    {
        for (u32 i = 0; i < ArrayCount(AtlasImageContexts); ++i ) {
            Init(PackedRects + PackedRectCount++, AtlasImageContexts + i);
        }
        for (u32 i = 0; i < ArrayCount(AtlasFontContexts); ++i) {
            auto* Font = AtlasFontContexts + i;
            Font->Type = AtlasContextType_Font;
            Font->LoadedFont = LoadedFont;
            Font->Codepoint = Codepoint_Start + i;
            Font->Size = 72.f;
            Font->FontId = Font_Default;
            Font->BitmapId = Bitmap_AtlasDefault;
            Init(PackedRects + PackedRectCount++, Font);
        }
    }
    
    // NOTE(Momo): Pack rects
    u32 AtlasWidth = 1024;
    u32 AtlasHeight = 1024;
    {
        constexpr usize NodeCount = ArrayCount(PackedRects) + 1;
        ryyrp_context RectPackContext;
        ryyrp_node RectPackNodes[NodeCount];
        ryyrp_Init(&RectPackContext, AtlasWidth, AtlasHeight, RectPackNodes, NodeCount);
        if (!ryyrp_Pack(&RectPackContext, PackedRects, PackedRectCount, RyyrpSort_Height)) {
            printf("Failed to generate bitmap\n");
            Assert(false);
        }
    }
    
    // NOTE(Momo): Generate atlas from rects
    u8* AtlasBitmap = GenerateAtlas(PackedRects, PackedRectCount, AtlasWidth, AtlasHeight);
    Defer { free(AtlasBitmap); };
    
#if 1
    stbi_write_png("test.png", AtlasWidth, AtlasHeight, 4, AtlasBitmap, AtlasWidth*4);
    printf("Written test atlas: test.png\n");
#endif
    
    ab_context AssetBuilder_ = {};
    auto* AssetBuilder = &AssetBuilder_;
    Begin(AssetBuilder, "yuu", "MOMO");
    {
        WriteBitmap(AssetBuilder, Bitmap_Ryoji, "assets/ryoji.png");
        WriteBitmap(AssetBuilder, Bitmap_Yuu, "assets/yuu.png");
        WriteBitmap(AssetBuilder, Bitmap_AtlasDefault, AtlasWidth, AtlasHeight, 4, AtlasBitmap);
        for(u32 i = 0; i <  PackedRectCount; ++i) {
            auto Rect = PackedRects[i];
            auto Type = *(atlas_context_type*)Rect.UserData;
            switch(Type) {
                case AtlasContextType_Image: {
                    auto* Image = (atlas_context_image*)Rect.UserData;
                    rect2u Rect2U = { Rect.X, Rect.Y, Rect.X + Rect.W, Rect.Y + Rect.H };
                    WriteAtlasRect(AssetBuilder, Image->Id, Image->BitmapId,  Rect2U);
                    
                } break;
                case AtlasContextType_Font: {
                    auto* Font  = (atlas_context_font*)Rect.UserData;
                    rect2u Rect2U = { Rect.X, Rect.Y, Rect.X + Rect.W, Rect.Y + Rect.H };
                    WriteFontGlyph(AssetBuilder, Font->FontId, Font->BitmapId, Font->Codepoint,  Rect2U);
                } break;
                
            }
        }
        
        
        for (u32 i = Codepoint_Start; i <= Codepoint_End; ++i) {
            for(u32 j = Codepoint_Start; j <= Codepoint_End; ++j) {
                u32 Kerning = (u32)stbtt_GetCodepointKernAdvance(&LoadedFont.Info, (i32)i, (i32)j);
                WriteFontKerning(AssetBuilder, Font_Default, i, j, Kerning);
            }
        }
        
        
        
    }
    End(AssetBuilder);
    
    printf("Done!\n");
    
    return 0;
    
}

