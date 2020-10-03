#include "tool_build_assets.h"
#include "ryoji_bitmanip.h"
#include "ryoji_rectpack.h"
#include "game_assets_file_formats.h"  


#define STB_IMAGE_IMPLEMENTATION
#include "thirdparty/stb/stb_image.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "thirdparty/stb/stb_truetype.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "thirdparty/stb/stb_image_write.h"


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
    u32 Codepoint;
    f32 Size;
    atlas_rect_id Id;
    bitmap_id BitmapId;
    loaded_font LoadedFont;
    u8* Bitmap;
};    


static inline void
AtlasContextFontUnloadCb(void* LoadContext) {
    auto* Context = (atlas_context_font*)LoadContext; 
    stbtt_FreeBitmap(Context->Bitmap, nullptr);
}


// NOTE(Momo):  Asset stuff
struct asset_context_image_filename {
    const char* Filename;
    bitmap_id Id;
    void* Data;
};

static inline asset_write_context
AssetContextImageFilenameWriteCb(void* UserContext) {
    asset_write_context Ret = {};
    
    auto* Context = (asset_context_image_filename*)UserContext;
    u32 Width = 0, Height = 0, Channels = 0;
    u8* LoadedImage = nullptr;
    {
        i32 W, H, C;
        LoadedImage = stbi_load(Context->Filename, &W, &H, &C, 0);
        Assert(LoadedImage != nullptr);
        
        Width = (u32)W;
        Height = (u32)H; 
        Channels = (u32)C;
    }
    Defer { stbi_image_free(LoadedImage); };
    
    u32 BitmapSize = Width * Height * Channels;
    usize DatSize = sizeof(yuu_entry) + sizeof(yuu_bitmap) + BitmapSize;
    void* Dat = calloc(DatSize, 1);
    u8* DatItr = (u8*)Dat;
    
    yuu_entry Entry = {};
    Entry.Type = AssetType_Bitmap;
    
    
    yuu_bitmap FileImage = {};
    FileImage.Id = Context->Id;
    FileImage.Width = Width;
    FileImage.Height = Height;
    FileImage.Channels = Channels;
    
    Write(&DatItr, Entry);
    Write(&DatItr, FileImage);
    CopyBlock(DatItr, LoadedImage, BitmapSize);
    
    printf("Loaded Image: Width = %d, Height = %d, Channels = %d\n",  Width, Height, Channels);
    
    Ret.DataToWrite = Dat;
    Ret.DataSize = DatSize;
    
    return Ret;
    
}


struct asset_context_image_raw {
    u32 Width, Height, Channels;
    bitmap_id Id;
    void* Bitmap;
    void* Data;
};

static inline asset_write_context
AssetContextImageRawWriteCb(void* UserContext) {
    asset_write_context Ret = {};
    auto* Context = (asset_context_image_raw*)UserContext;
    
    u32 BitmapSize = Context->Width * Context->Height * Context->Channels;
    usize DatSize = sizeof(yuu_entry) + sizeof(yuu_bitmap) + BitmapSize;
    void* Dat = calloc(DatSize, 1);
    u8* DatItr = (u8*)Dat;
    
    yuu_entry Entry = {};
    Entry.Type = AssetType_Bitmap;
    
    yuu_bitmap FileImage = {};
    FileImage.Id = Context->Id;
    FileImage.Width = Context->Width;
    FileImage.Height = Context->Height;
    FileImage.Channels = Context->Channels;
    
    Write(&DatItr, Entry);
    Write(&DatItr, FileImage);
    CopyBlock(DatItr, Context->Bitmap, BitmapSize);
    
    
    printf("Loaded Image: Width = %d, Height = %d, Channels = %d\n",  Context->Width, Context->Height, Context->Channels);
    
    Ret.DataToWrite = Dat;
    Ret.DataSize = DatSize;
    
    return Ret;
    
}

static inline void 
AssetContextFreeCb(asset_write_context Context) {
    free(Context.DataToWrite);
}


struct asset_context_atlas_rect {
    atlas_rect_id Id;
    bitmap_id BitmapId;
    rect2u Rect;
    void* Data;
};


static inline asset_write_context 
AssetContextAtlasRectWriteCb(void* UserContext) {
    asset_write_context Ret = {};
    auto* Context = (asset_context_atlas_rect*)UserContext;
    
    usize DatSize = sizeof(yuu_entry) + sizeof(yuu_atlas_rect);
    void* Dat = calloc(DatSize, 1);
    u8* DatItr = (u8*)Dat;
    
    yuu_entry Entry = {};
    Entry.Type = AssetType_AtlasRect;
    
    yuu_atlas_rect AtlasRect  = {};
    AtlasRect.Id = Context->Id;
    AtlasRect.Rect = Context->Rect;
    AtlasRect.BitmapId = Context->BitmapId;
    
    Write(&DatItr, Entry);
    Write(&DatItr, AtlasRect);
    
    printf("Loaded Rect: X = %d, Y = %d, W = %d, H = %d\n", Context->Rect.Min.X, Context->Rect.Min.Y, GetWidth(Context->Rect), GetHeight(Context->Rect));
    
    Ret.DataToWrite = Dat;
    Ret.DataSize = DatSize;
    
    return Ret;
}

static inline void
AssetContextAtlasRectFreeCb(void* UserContext) {
    auto* Context = (asset_context_atlas_rect*)UserContext;
    free(Context->Data);
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


static inline void*
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


int main() {
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
    atlas_context_font AtlasFontContexts[] = {
        {  AtlasContextType_Font, 'a', 72, AtlasRect_a, Bitmap_AtlasDefault },
        {  AtlasContextType_Font, 'b', 72, AtlasRect_b, Bitmap_AtlasDefault },
        {  AtlasContextType_Font, 'c', 72, AtlasRect_c, Bitmap_AtlasDefault },
        {  AtlasContextType_Font, 'd', 72, AtlasRect_d, Bitmap_AtlasDefault },
        {  AtlasContextType_Font, 'e', 72, AtlasRect_e, Bitmap_AtlasDefault },
        {  AtlasContextType_Font, 'f', 72, AtlasRect_f, Bitmap_AtlasDefault },
        {  AtlasContextType_Font, 'g', 72, AtlasRect_g, Bitmap_AtlasDefault },
        {  AtlasContextType_Font, 'h', 72, AtlasRect_h, Bitmap_AtlasDefault },
        {  AtlasContextType_Font, 'i', 72, AtlasRect_i, Bitmap_AtlasDefault },
        {  AtlasContextType_Font, 'j', 72, AtlasRect_j, Bitmap_AtlasDefault },
        {  AtlasContextType_Font, 'k', 72, AtlasRect_k, Bitmap_AtlasDefault },
        {  AtlasContextType_Font, 'l', 72, AtlasRect_l, Bitmap_AtlasDefault },
        {  AtlasContextType_Font, 'm', 72, AtlasRect_m, Bitmap_AtlasDefault },
        {  AtlasContextType_Font, 'n', 72, AtlasRect_n, Bitmap_AtlasDefault },
        {  AtlasContextType_Font, 'o', 72, AtlasRect_o, Bitmap_AtlasDefault },
        {  AtlasContextType_Font, 'p', 72, AtlasRect_p, Bitmap_AtlasDefault },
        {  AtlasContextType_Font, 'q', 72, AtlasRect_q, Bitmap_AtlasDefault },
        {  AtlasContextType_Font, 'r', 72, AtlasRect_r, Bitmap_AtlasDefault },
        {  AtlasContextType_Font, 's', 72, AtlasRect_s, Bitmap_AtlasDefault },
        {  AtlasContextType_Font, 't', 72, AtlasRect_t, Bitmap_AtlasDefault },
        {  AtlasContextType_Font, 'u', 72, AtlasRect_u, Bitmap_AtlasDefault },
        {  AtlasContextType_Font, 'v', 72, AtlasRect_v, Bitmap_AtlasDefault },
        {  AtlasContextType_Font, 'w', 72, AtlasRect_w, Bitmap_AtlasDefault },
        {  AtlasContextType_Font, 'x', 72, AtlasRect_x, Bitmap_AtlasDefault },
        {  AtlasContextType_Font, 'y', 72, AtlasRect_y, Bitmap_AtlasDefault },
        {  AtlasContextType_Font, 'z', 72, AtlasRect_z, Bitmap_AtlasDefault },
        {  AtlasContextType_Font, 'A', 72, AtlasRect_A, Bitmap_AtlasDefault },
        {  AtlasContextType_Font, 'B', 72, AtlasRect_B, Bitmap_AtlasDefault },
        {  AtlasContextType_Font, 'C', 72, AtlasRect_C, Bitmap_AtlasDefault },
        {  AtlasContextType_Font, 'D', 72, AtlasRect_D, Bitmap_AtlasDefault },
        {  AtlasContextType_Font, 'E', 72, AtlasRect_E, Bitmap_AtlasDefault },
        {  AtlasContextType_Font, 'F', 72, AtlasRect_F, Bitmap_AtlasDefault },
        {  AtlasContextType_Font, 'G', 72, AtlasRect_G, Bitmap_AtlasDefault },
        {  AtlasContextType_Font, 'H', 72, AtlasRect_H, Bitmap_AtlasDefault },
        {  AtlasContextType_Font, 'I', 72, AtlasRect_I, Bitmap_AtlasDefault },
        {  AtlasContextType_Font, 'J', 72, AtlasRect_J, Bitmap_AtlasDefault },
        {  AtlasContextType_Font, 'K', 72, AtlasRect_K, Bitmap_AtlasDefault },
        {  AtlasContextType_Font, 'L', 72, AtlasRect_L, Bitmap_AtlasDefault },
        {  AtlasContextType_Font, 'M', 72, AtlasRect_M, Bitmap_AtlasDefault },
        {  AtlasContextType_Font, 'N', 72, AtlasRect_N, Bitmap_AtlasDefault },
        {  AtlasContextType_Font, 'O', 72, AtlasRect_O, Bitmap_AtlasDefault },
        {  AtlasContextType_Font, 'P', 72, AtlasRect_P, Bitmap_AtlasDefault },
        {  AtlasContextType_Font, 'Q', 72, AtlasRect_Q, Bitmap_AtlasDefault },
        {  AtlasContextType_Font, 'R', 72, AtlasRect_R, Bitmap_AtlasDefault },
        {  AtlasContextType_Font, 'S', 72, AtlasRect_S, Bitmap_AtlasDefault },
        {  AtlasContextType_Font, 'T', 72, AtlasRect_T, Bitmap_AtlasDefault },
        {  AtlasContextType_Font, 'U', 72, AtlasRect_U, Bitmap_AtlasDefault },
        {  AtlasContextType_Font, 'V', 72, AtlasRect_V, Bitmap_AtlasDefault },
        {  AtlasContextType_Font, 'W', 72, AtlasRect_W, Bitmap_AtlasDefault },
        {  AtlasContextType_Font, 'X', 72, AtlasRect_X, Bitmap_AtlasDefault },
        {  AtlasContextType_Font, 'Y', 72, AtlasRect_Y, Bitmap_AtlasDefault },
        {  AtlasContextType_Font, 'Z', 72, AtlasRect_Z, Bitmap_AtlasDefault },
    };
    
    constexpr usize TotalRects = ArrayCount(AtlasImageContexts) + ArrayCount(AtlasFontContexts);
    ryyrp_rect PackedRects[TotalRects];
    usize PackedRectCount = 0;
    {
        loaded_font LoadedFont = AllocateFontFromFile("assets/DroidSansMono.ttf");
        for (u32 i = 0; i < ArrayCount(AtlasImageContexts); ++i ) {
            Init(PackedRects + PackedRectCount++, AtlasImageContexts + i);
        }
        for (u32 i = 0; i < ArrayCount(AtlasFontContexts); ++i) {
            auto* Font = AtlasFontContexts + i;
            Font->LoadedFont = LoadedFont;
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
        if (!ryyrp_Pack(&RectPackContext, PackedRects, PackedRectCount)) {
            printf("Failed to generate 1024x1024 bitmap\n");
            Assert(false);
        }
    }
    
    // NOTE(Momo): Generate atlas from rects
    void* AtlasBitmap = GenerateAtlas(PackedRects, PackedRectCount, AtlasWidth, AtlasHeight);
    Defer { free(AtlasBitmap); };
    
#if 0
    stbi_write_png("test.png", AtlasWidth, AtlasHeight, 4, AtlasBitmap, AtlasWidth*4);
#endif
    
    // NOTE(Momo): Gather all the assets we need to load
    asset_builder<128> Assets_ = {};
    auto* Assets = &Assets_;
    {
        // NOTE(Momo): Image by filename
        asset_context_image_filename AssetContextImageFilename[] = {
            { "assets/ryoji.png", Bitmap_Ryoji },
            { "assets/yuu.png", Bitmap_Yuu},
        };
        
        for(u32 i = 0; i < ArrayCount(AssetContextImageFilename); ++i) {
            AddEntry(Assets, 
                     AssetContextImageFilename + i, 
                     AssetContextImageFilenameWriteCb, 
                     AssetContextFreeCb); 
        }
        
        // NOTE(Momo): Image by raw
        asset_context_image_raw AssetContextImageRaw[] = {
            {  AtlasWidth, AtlasHeight, 4, Bitmap_AtlasDefault, AtlasBitmap }
        };
        
        for(u32 i = 0; i < ArrayCount(AssetContextImageRaw); ++i) {
            AddEntry(Assets, 
                     AssetContextImageRaw + i, 
                     AssetContextImageRawWriteCb, 
                     AssetContextFreeCb); 
        }
        
        // NOTE(Momo): Atlas rects (image rects?)
        asset_context_atlas_rect AssetContextAtlasRects[TotalRects];
        for(u32 i = 0; i <  PackedRectCount; ++i) {
            auto Rect = PackedRects[i];
            auto Type = *(atlas_context_type*)Rect.UserData;
            switch(Type) {
                case AtlasContextType_Image: {
                    auto AtlasContextImage = (atlas_context_image*)Rect.UserData;
                    auto* AssetContextAtlasRect = AssetContextAtlasRects + i;
                    AssetContextAtlasRect->Id = AtlasContextImage->Id;
                    AssetContextAtlasRect->BitmapId = AtlasContextImage->BitmapId;
                    AssetContextAtlasRect->Rect = { 
                        Rect.X,
                        Rect.Y,
                        Rect.X + Rect.W,
                        Rect.Y + Rect.H,
                    };
                    AddEntry(Assets, 
                             AssetContextAtlasRect,
                             AssetContextAtlasRectWriteCb,
                             AssetContextFreeCb);
                    
                } break;
                case AtlasContextType_Font: {
                    auto AtlasContextImage = (atlas_context_font*)Rect.UserData;
                    auto* AssetContextAtlasRect = AssetContextAtlasRects + i;
                    AssetContextAtlasRect->Id = AtlasContextImage->Id;
                    AssetContextAtlasRect->BitmapId = AtlasContextImage->BitmapId;
                    AssetContextAtlasRect->Rect = { 
                        Rect.X,
                        Rect.Y,
                        Rect.X + Rect.W,
                        Rect.Y + Rect.H,
                    };
                    AddEntry(Assets, 
                             AssetContextAtlasRect,
                             AssetContextAtlasRectWriteCb,
                             AssetContextFreeCb);
                } break;
                
            }
            
        }
        
    }
    Write(Assets, "yuu", "MOMO");
    
    return 0;
    
}

