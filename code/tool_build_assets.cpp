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


// Atlas stuff ////////////////////////
enum atlas_context_type {
    AtlasContextType_Image,
    AtlasContextType_Font,
};

struct atlas_context_image {
    atlas_context_type Type;
    const char* Filename;
    asset_id AssetId;
    asset_id AtlasAssetId;
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
    asset_id AssetId;
    asset_id AtlasAssetId;
    loaded_font LoadedFont;
    u8* Bitmap;
};    

static inline void
AtlasContextFontLoadCb(void* LoadContext, u8** BitmapData, u32* BitmapWidth, u32* BitmapHeight, u32* BitmapChannels) 
{
    auto* Context = (atlas_context_font*)LoadContext; 
    constexpr u32 Channels = 4;
    f32 Scale = stbtt_ScaleForPixelHeight(&Context->LoadedFont.Info, Context->Size);
    i32 Width, Height;
    u8* FontBitmapOneCh = stbtt_GetCodepointBitmap(&Context->LoadedFont.Info, 0, Scale, 
                                                   Context->Codepoint, &Width, &Height, nullptr, nullptr);
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
    
    *BitmapWidth = (u32)Width;
    *BitmapHeight = (u32)Height;
    *BitmapChannels = Channels;
    *BitmapData = FontBitmap;
}

static inline void
AtlasContextFontUnloadCb(void* LoadContext) {
    auto* Context = (atlas_context_font*)LoadContext; 
    stbtt_FreeBitmap(Context->Bitmap, nullptr);
}


// Asset stuff
struct asset_context_image_filename {
    const char* Filename;
    asset_id Id;
    void* Data;
};

static inline void
AssetContextImageFilenameWriteCb(void* UserContext, void** DataToWrite, usize* DataSize) {
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
    usize DatSize = sizeof(yuu_entry) + sizeof(yuu_image) + BitmapSize;
    void* Dat = calloc(DatSize, 1);
    u8* DatItr = (u8*)Dat;
    
    yuu_entry Entry = {};
    Entry.Type = AssetType_Image;
    Entry.Id = Context->Id;
    
    yuu_image FileImage = {};
    FileImage.Width = Width;
    FileImage.Height = Height;
    FileImage.Channels = Channels;
    
    Write(&DatItr, Entry);
    Write(&DatItr, FileImage);
    CopyBlock(DatItr, LoadedImage, BitmapSize);
    
    (*DataToWrite) = Dat;
    (*DataSize) = DatSize;
    
    printf("Loaded Image: Width = %d, Height = %d, Channels = %d\n",  Width, Height, Channels);
    
}


static inline void
AssetContextImageFilenameFreeCb(void* UserContext) {
    auto* Context = (asset_context_image_filename*)UserContext;
    free(Context->Data);
}

struct asset_context_image_raw {
    u32 Width, Height, Channels;
    asset_id Id;
    void* Bitmap;
    void* Data;
};

static inline void
AssetContextImageRawWriteCb(void* UserContext, void** DataToWrite, usize* DataSize) {
    auto* Context = (asset_context_image_raw*)UserContext;
    
    u32 BitmapSize = Context->Width * Context->Height * Context->Channels;
    usize DatSize = sizeof(yuu_entry) + sizeof(yuu_image) + BitmapSize;
    void* Dat = calloc(DatSize, 1);
    u8* DatItr = (u8*)Dat;
    
    yuu_entry Entry = {};
    Entry.Type = AssetType_Image;
    Entry.Id = Context->Id;
    
    yuu_image FileImage = {};
    FileImage.Width = Context->Width;
    FileImage.Height = Context->Height;
    FileImage.Channels = Context->Channels;
    
    Write(&DatItr, Entry);
    Write(&DatItr, FileImage);
    CopyBlock(DatItr, Context->Bitmap, BitmapSize);
    
    printf("Loaded Image: Width = %d, Height = %d, Channels = %d\n",  Context->Width, Context->Height, Context->Channels);
    
}

static inline void
AssetContextImageRawFreeCb(void* UserContext) {
    auto* Context = (asset_context_image_raw*)UserContext;
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
        
        atlas_context_type Type = *(atlas_context_type*)Rect.UserData;
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
    atlas_context_image ImageContexts[] = {
        { AtlasContextType_Image, "assets/ryoji.png",  Asset_RectRyoji, Asset_ImageAtlasDefault },
        { AtlasContextType_Image, "assets/yuu.png",    Asset_RectYuu,    Asset_ImageAtlasDefault },
        { AtlasContextType_Image, "assets/karu00.png", Asset_RectKaru00, Asset_ImageAtlasDefault },
        { AtlasContextType_Image, "assets/karu01.png", Asset_RectKaru01, Asset_ImageAtlasDefault },
        { AtlasContextType_Image, "assets/karu02.png", Asset_RectKaru02, Asset_ImageAtlasDefault },
        { AtlasContextType_Image, "assets/karu10.png", Asset_RectKaru10, Asset_ImageAtlasDefault },
        { AtlasContextType_Image, "assets/karu11.png", Asset_RectKaru11, Asset_ImageAtlasDefault },
        { AtlasContextType_Image, "assets/karu12.png", Asset_RectKaru12, Asset_ImageAtlasDefault },
        { AtlasContextType_Image, "assets/karu20.png", Asset_RectKaru20, Asset_ImageAtlasDefault },
        { AtlasContextType_Image, "assets/karu21.png", Asset_RectKaru21, Asset_ImageAtlasDefault },
        { AtlasContextType_Image, "assets/karu22.png", Asset_RectKaru22, Asset_ImageAtlasDefault },
        { AtlasContextType_Image, "assets/karu30.png", Asset_RectKaru30, Asset_ImageAtlasDefault },
        { AtlasContextType_Image, "assets/karu31.png", Asset_RectKaru31, Asset_ImageAtlasDefault },
        { AtlasContextType_Image, "assets/karu32.png", Asset_RectKaru32, Asset_ImageAtlasDefault },
    };
    atlas_context_font FontContexts[] = {
        {  AtlasContextType_Font, 'a', 72, Asset_FontRect_a, Asset_ImageAtlasDefault },
        {  AtlasContextType_Font, 'b', 72, Asset_FontRect_b, Asset_ImageAtlasDefault },
        {  AtlasContextType_Font, 'c', 72, Asset_FontRect_c, Asset_ImageAtlasDefault },
        {  AtlasContextType_Font, 'd', 72, Asset_FontRect_d, Asset_ImageAtlasDefault },
        {  AtlasContextType_Font, 'e', 72, Asset_FontRect_e, Asset_ImageAtlasDefault },
        {  AtlasContextType_Font, 'f', 72, Asset_FontRect_f, Asset_ImageAtlasDefault },
        {  AtlasContextType_Font, 'g', 72, Asset_FontRect_g, Asset_ImageAtlasDefault },
        {  AtlasContextType_Font, 'h', 72, Asset_FontRect_h, Asset_ImageAtlasDefault },
        {  AtlasContextType_Font, 'i', 72, Asset_FontRect_i, Asset_ImageAtlasDefault },
        {  AtlasContextType_Font, 'j', 72, Asset_FontRect_j, Asset_ImageAtlasDefault },
        {  AtlasContextType_Font, 'k', 72, Asset_FontRect_k, Asset_ImageAtlasDefault },
        {  AtlasContextType_Font, 'l', 72, Asset_FontRect_l, Asset_ImageAtlasDefault },
        {  AtlasContextType_Font, 'm', 72, Asset_FontRect_m, Asset_ImageAtlasDefault },
        {  AtlasContextType_Font, 'n', 72, Asset_FontRect_n, Asset_ImageAtlasDefault },
        {  AtlasContextType_Font, 'o', 72, Asset_FontRect_o, Asset_ImageAtlasDefault },
        {  AtlasContextType_Font, 'p', 72, Asset_FontRect_p, Asset_ImageAtlasDefault },
        {  AtlasContextType_Font, 'q', 72, Asset_FontRect_q, Asset_ImageAtlasDefault },
        {  AtlasContextType_Font, 'r', 72, Asset_FontRect_r, Asset_ImageAtlasDefault },
        {  AtlasContextType_Font, 's', 72, Asset_FontRect_s, Asset_ImageAtlasDefault },
        {  AtlasContextType_Font, 't', 72, Asset_FontRect_t, Asset_ImageAtlasDefault },
        {  AtlasContextType_Font, 'u', 72, Asset_FontRect_u, Asset_ImageAtlasDefault },
        {  AtlasContextType_Font, 'v', 72, Asset_FontRect_v, Asset_ImageAtlasDefault },
        {  AtlasContextType_Font, 'w', 72, Asset_FontRect_w, Asset_ImageAtlasDefault },
        {  AtlasContextType_Font, 'x', 72, Asset_FontRect_x, Asset_ImageAtlasDefault },
        {  AtlasContextType_Font, 'y', 72, Asset_FontRect_y, Asset_ImageAtlasDefault },
        {  AtlasContextType_Font, 'z', 72, Asset_FontRect_z, Asset_ImageAtlasDefault },
        {  AtlasContextType_Font, 'A', 72, Asset_FontRect_A, Asset_ImageAtlasDefault },
        {  AtlasContextType_Font, 'B', 72, Asset_FontRect_B, Asset_ImageAtlasDefault },
        {  AtlasContextType_Font, 'C', 72, Asset_FontRect_C, Asset_ImageAtlasDefault },
        {  AtlasContextType_Font, 'D', 72, Asset_FontRect_D, Asset_ImageAtlasDefault },
        {  AtlasContextType_Font, 'E', 72, Asset_FontRect_E, Asset_ImageAtlasDefault },
        {  AtlasContextType_Font, 'F', 72, Asset_FontRect_F, Asset_ImageAtlasDefault },
        {  AtlasContextType_Font, 'G', 72, Asset_FontRect_G, Asset_ImageAtlasDefault },
        {  AtlasContextType_Font, 'H', 72, Asset_FontRect_H, Asset_ImageAtlasDefault },
        {  AtlasContextType_Font, 'I', 72, Asset_FontRect_I, Asset_ImageAtlasDefault },
        {  AtlasContextType_Font, 'J', 72, Asset_FontRect_J, Asset_ImageAtlasDefault },
        {  AtlasContextType_Font, 'K', 72, Asset_FontRect_K, Asset_ImageAtlasDefault },
        {  AtlasContextType_Font, 'L', 72, Asset_FontRect_L, Asset_ImageAtlasDefault },
        {  AtlasContextType_Font, 'M', 72, Asset_FontRect_M, Asset_ImageAtlasDefault },
        {  AtlasContextType_Font, 'N', 72, Asset_FontRect_N, Asset_ImageAtlasDefault },
        {  AtlasContextType_Font, 'O', 72, Asset_FontRect_O, Asset_ImageAtlasDefault },
        {  AtlasContextType_Font, 'P', 72, Asset_FontRect_P, Asset_ImageAtlasDefault },
        {  AtlasContextType_Font, 'Q', 72, Asset_FontRect_Q, Asset_ImageAtlasDefault },
        {  AtlasContextType_Font, 'R', 72, Asset_FontRect_R, Asset_ImageAtlasDefault },
        {  AtlasContextType_Font, 'S', 72, Asset_FontRect_S, Asset_ImageAtlasDefault },
        {  AtlasContextType_Font, 'T', 72, Asset_FontRect_T, Asset_ImageAtlasDefault },
        {  AtlasContextType_Font, 'U', 72, Asset_FontRect_U, Asset_ImageAtlasDefault },
        {  AtlasContextType_Font, 'V', 72, Asset_FontRect_V, Asset_ImageAtlasDefault },
        {  AtlasContextType_Font, 'W', 72, Asset_FontRect_W, Asset_ImageAtlasDefault },
        {  AtlasContextType_Font, 'X', 72, Asset_FontRect_X, Asset_ImageAtlasDefault },
        {  AtlasContextType_Font, 'Y', 72, Asset_FontRect_Y, Asset_ImageAtlasDefault },
        {  AtlasContextType_Font, 'Z', 72, Asset_FontRect_Z, Asset_ImageAtlasDefault },
    };
    
    ryyrp_rect PackedRects[128];
    usize PackedRectCount = 0;
    {
        loaded_font LoadedFont = AllocateFontFromFile("assets/DroidSansMono.ttf");
        for (u32 i = 0; i < ArrayCount(ImageContexts); ++i ) {
            Init(PackedRects + PackedRectCount++, ImageContexts + i);
        }
        for (u32 i = 0; i < ArrayCount(FontContexts); ++i) {
            auto* Font = FontContexts + i;
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
    stbi_write_png("test.png", AtlasWidth, AtlasHeight, 4, AtlasBitmap, AtlasWidth*4);
    
    // NOTE(Momo): Gather all the assets we need to load
    asset_builder<128> Assets_ = {};
    auto* Assets = &Assets_;
    {
        // NOTE(Momo): Image by filename
        asset_context_image_filename AssetContextImageFilename[] = {
            { "assets/ryoji.png", Asset_ImageRyoji },
            { "assets/yuu.png", Asset_ImageYuu},
        };
        
        for(u32 i = 0; i < ArrayCount(AssetContextImageFilename); ++i) {
            AddEntry(Assets, 
                     AssetContextImageFilename + i, 
                     AssetContextImageFilenameWriteCb, 
                     AssetContextImageFilenameFreeCb); 
        }
        
        // NOTE(Momo): Image by raw
        asset_context_image_raw AssetContextImageRaw[] = {
            {  AtlasWidth, AtlasHeight, 4, Asset_ImageAtlasDefault, AtlasBitmap }
        };
        
        for(u32 i = 0; i < ArrayCount(AssetContextImageRaw); ++i) {
            AddEntry(Assets, 
                     AssetContextImageRaw + i, 
                     AssetContextImageRawWriteCb, 
                     AssetContextImageRawFreeCb); 
        }
        
    }
    Write(Assets, "yuu", "MOMO");
    
    return 0;
    
}

