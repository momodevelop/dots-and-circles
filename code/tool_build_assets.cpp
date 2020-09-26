#include "tool_build_assets.h"
#include "ryoji_atlas_builder.h"
#include "ryoji_bitmanip.h"
#include "ryoji_atlas_builder.h"


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


///////////////////////////
enum load_type {
    LoadType_Image,
    LoadType_Font,
};

struct load_context_font {
    load_type Type;
    u32 Codepoint;
    f32 Size;
    asset_id AssetId;
    asset_id AtlasAssetId;
    loaded_font LoadedFont;
    u8* Bitmap;
} FontContexts[] = {
    {  LoadType_Font, 'a', 72, Asset_FontRect_a, Asset_ImageAtlasDefault },
    {  LoadType_Font, 'b', 72, Asset_FontRect_b, Asset_ImageAtlasDefault },
    {  LoadType_Font, 'c', 72, Asset_FontRect_c, Asset_ImageAtlasDefault },
    {  LoadType_Font, 'd', 72, Asset_FontRect_d, Asset_ImageAtlasDefault },
    {  LoadType_Font, 'e', 72, Asset_FontRect_e, Asset_ImageAtlasDefault },
    {  LoadType_Font, 'f', 72, Asset_FontRect_f, Asset_ImageAtlasDefault },
    {  LoadType_Font, 'g', 72, Asset_FontRect_g, Asset_ImageAtlasDefault },
    {  LoadType_Font, 'h', 72, Asset_FontRect_h, Asset_ImageAtlasDefault },
    {  LoadType_Font, 'i', 72, Asset_FontRect_i, Asset_ImageAtlasDefault },
    {  LoadType_Font, 'j', 72, Asset_FontRect_j, Asset_ImageAtlasDefault },
    {  LoadType_Font, 'k', 72, Asset_FontRect_k, Asset_ImageAtlasDefault },
    {  LoadType_Font, 'l', 72, Asset_FontRect_l, Asset_ImageAtlasDefault },
    {  LoadType_Font, 'm', 72, Asset_FontRect_m, Asset_ImageAtlasDefault },
    {  LoadType_Font, 'n', 72, Asset_FontRect_n, Asset_ImageAtlasDefault },
    {  LoadType_Font, 'o', 72, Asset_FontRect_o, Asset_ImageAtlasDefault },
    {  LoadType_Font, 'p', 72, Asset_FontRect_p, Asset_ImageAtlasDefault },
    {  LoadType_Font, 'q', 72, Asset_FontRect_q, Asset_ImageAtlasDefault },
    {  LoadType_Font, 'r', 72, Asset_FontRect_r, Asset_ImageAtlasDefault },
    {  LoadType_Font, 's', 72, Asset_FontRect_s, Asset_ImageAtlasDefault },
    {  LoadType_Font, 't', 72, Asset_FontRect_t, Asset_ImageAtlasDefault },
    {  LoadType_Font, 'u', 72, Asset_FontRect_u, Asset_ImageAtlasDefault },
    {  LoadType_Font, 'v', 72, Asset_FontRect_v, Asset_ImageAtlasDefault },
    {  LoadType_Font, 'w', 72, Asset_FontRect_w, Asset_ImageAtlasDefault },
    {  LoadType_Font, 'x', 72, Asset_FontRect_x, Asset_ImageAtlasDefault },
    {  LoadType_Font, 'y', 72, Asset_FontRect_y, Asset_ImageAtlasDefault },
    {  LoadType_Font, 'z', 72, Asset_FontRect_z, Asset_ImageAtlasDefault },
    {  LoadType_Font, 'A', 72, Asset_FontRect_A, Asset_ImageAtlasDefault },
    {  LoadType_Font, 'B', 72, Asset_FontRect_B, Asset_ImageAtlasDefault },
    {  LoadType_Font, 'C', 72, Asset_FontRect_C, Asset_ImageAtlasDefault },
    {  LoadType_Font, 'D', 72, Asset_FontRect_D, Asset_ImageAtlasDefault },
    {  LoadType_Font, 'E', 72, Asset_FontRect_E, Asset_ImageAtlasDefault },
    {  LoadType_Font, 'F', 72, Asset_FontRect_F, Asset_ImageAtlasDefault },
    {  LoadType_Font, 'G', 72, Asset_FontRect_G, Asset_ImageAtlasDefault },
    {  LoadType_Font, 'H', 72, Asset_FontRect_H, Asset_ImageAtlasDefault },
    {  LoadType_Font, 'I', 72, Asset_FontRect_I, Asset_ImageAtlasDefault },
    {  LoadType_Font, 'J', 72, Asset_FontRect_J, Asset_ImageAtlasDefault },
    {  LoadType_Font, 'K', 72, Asset_FontRect_K, Asset_ImageAtlasDefault },
    {  LoadType_Font, 'L', 72, Asset_FontRect_L, Asset_ImageAtlasDefault },
    {  LoadType_Font, 'M', 72, Asset_FontRect_M, Asset_ImageAtlasDefault },
    {  LoadType_Font, 'N', 72, Asset_FontRect_N, Asset_ImageAtlasDefault },
    {  LoadType_Font, 'O', 72, Asset_FontRect_O, Asset_ImageAtlasDefault },
    {  LoadType_Font, 'P', 72, Asset_FontRect_P, Asset_ImageAtlasDefault },
    {  LoadType_Font, 'Q', 72, Asset_FontRect_Q, Asset_ImageAtlasDefault },
    {  LoadType_Font, 'R', 72, Asset_FontRect_R, Asset_ImageAtlasDefault },
    {  LoadType_Font, 'S', 72, Asset_FontRect_S, Asset_ImageAtlasDefault },
    {  LoadType_Font, 'T', 72, Asset_FontRect_T, Asset_ImageAtlasDefault },
    {  LoadType_Font, 'U', 72, Asset_FontRect_U, Asset_ImageAtlasDefault },
    {  LoadType_Font, 'V', 72, Asset_FontRect_V, Asset_ImageAtlasDefault },
    {  LoadType_Font, 'W', 72, Asset_FontRect_W, Asset_ImageAtlasDefault },
    {  LoadType_Font, 'X', 72, Asset_FontRect_X, Asset_ImageAtlasDefault },
    {  LoadType_Font, 'Y', 72, Asset_FontRect_Y, Asset_ImageAtlasDefault },
    {  LoadType_Font, 'Z', 72, Asset_FontRect_Z, Asset_ImageAtlasDefault },
};

struct load_context_image {
    load_type Type;
    const char* Filename;
    asset_id AssetId;
    asset_id AtlasAssetId;
    u8* Bitmap;
    
} ImageContexts[] = {
    { LoadType_Image, "assets/ryoji.png",  Asset_RectRyoji, Asset_ImageAtlasDefault },
    { LoadType_Image, "assets/yuu.png",    Asset_RectYuu,    Asset_ImageAtlasDefault },
    { LoadType_Image, "assets/karu00.png", Asset_RectKaru00, Asset_ImageAtlasDefault },
    { LoadType_Image, "assets/karu01.png", Asset_RectKaru01, Asset_ImageAtlasDefault },
    { LoadType_Image, "assets/karu02.png", Asset_RectKaru02, Asset_ImageAtlasDefault },
    { LoadType_Image, "assets/karu10.png", Asset_RectKaru10, Asset_ImageAtlasDefault },
    { LoadType_Image, "assets/karu11.png", Asset_RectKaru11, Asset_ImageAtlasDefault },
    { LoadType_Image, "assets/karu12.png", Asset_RectKaru12, Asset_ImageAtlasDefault },
    { LoadType_Image, "assets/karu20.png", Asset_RectKaru20, Asset_ImageAtlasDefault },
    { LoadType_Image, "assets/karu21.png", Asset_RectKaru21, Asset_ImageAtlasDefault },
    { LoadType_Image, "assets/karu22.png", Asset_RectKaru22, Asset_ImageAtlasDefault },
    { LoadType_Image, "assets/karu30.png", Asset_RectKaru30, Asset_ImageAtlasDefault },
    { LoadType_Image, "assets/karu31.png", Asset_RectKaru31, Asset_ImageAtlasDefault },
    { LoadType_Image, "assets/karu32.png", Asset_RectKaru32, Asset_ImageAtlasDefault },
};

static inline void
LoadImage(void* LoadContext, u8** BitmapData, u32* BitmapWidth, u32* BitmapHeight, u32* BitmapChannels) 
{
    auto* Context = (load_context_image*)LoadContext; 
    i32 W, H, C;
    Context->Bitmap = (*BitmapData) = stbi_load(Context->Filename, &W, &H, &C, 0);
    (*BitmapWidth) = (u32)W;
    (*BitmapHeight) = (u32)H;
    (*BitmapChannels) = (u32)C;
}

static inline void
GetInfoImage(void* LoadContext, u32* BitmapWidth, u32* BitmapHeight, u32* BitmapChannels) 
{
    auto* Context = (load_context_image*)LoadContext; 
    i32 W, H, C;
    stbi_info(Context->Filename, &W, &H, &C);
    (*BitmapWidth) = (u32)W;
    (*BitmapHeight) = (u32)H;
    (*BitmapChannels) = (u32)C;
}

static inline void
UnloadImage(void* LoadContext) {
    auto* Context = (load_context_image*)LoadContext; 
    stbi_image_free(Context->Bitmap);
}


static inline void
LoadFont(void* LoadContext, u8** BitmapData, u32* BitmapWidth, u32* BitmapHeight, u32* BitmapChannels) 
{
    auto* Context = (load_context_font*)LoadContext; 
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
GetInfoFont(void* LoadContext, u32* BitmapWidth, u32* BitmapHeight, u32* BitmapChannels) 
{
    auto* Context = (load_context_font*)LoadContext; 
    i32 ix0, iy0, ix1, iy1;
    f32 Scale = stbtt_ScaleForPixelHeight(&Context->LoadedFont.Info, Context->Size);
    stbtt_GetCodepointBitmapBox(&Context->LoadedFont.Info, Context->Codepoint, Scale, Scale, &ix0, &iy0, &ix1, &iy1);
    
    (*BitmapWidth) = (u32)(ix1 - ix0);
    (*BitmapHeight) = (u32)(iy1 - iy0);
    (*BitmapChannels) = 4;
}

static inline void
UnloadFont(void* LoadContext) {
    auto* Context = (load_context_font*)LoadContext; 
    stbtt_FreeBitmap(Context->Bitmap, nullptr);
}

int main() {
    u32 AtlasWidth, AtlasHeight, AtlasChannels;
    void* AtlasBitmap;
    atlas_builder<128> Atlas_ = {};
    auto* Atlas = &Atlas_;
    {
        loaded_font LoadedFont = AllocateFontFromFile("assets/DroidSansMono.ttf");
        
        Begin(Atlas);
        {
            for (u32 i = 0; i < ArrayCount(ImageContexts); ++i) {
                auto* Image = ImageContexts + i;
                AddEntry(Atlas, GetInfoImage, LoadImage, UnloadImage, Image);
            }
            for(u32 i = 0; i < ArrayCount(FontContexts); ++i) {
                auto* Font = FontContexts + i;
                Font->LoadedFont = LoadedFont;
                AddEntry(Atlas, GetInfoFont, LoadFont, UnloadFont, Font);
            }
        }
        End(Atlas);
        AtlasBitmap = malloc(GetAtlasBitmapSize(Atlas));
        GetAtlasBitmap(Atlas, AtlasBitmap, &AtlasWidth, &AtlasHeight, &AtlasChannels);
    }
    Defer { free(AtlasBitmap); };
    
    // NOTE(Momo): Gather all the assets we need to load
    asset_builder Assets_ = {};
    asset_builder* Assets = &Assets_;
    Init(Assets, Asset_Count);
    Defer { Free(Assets); };
    {
#if 1
        SetImageByFilename(Assets, Asset_ImageRyoji, "assets/ryoji.png");
        SetImageByFilename(Assets, Asset_ImageYuu, "assets/yuu.png");
        SetSpritesheet(Assets, Asset_SpritesheetKaru, "assets/karu.png", 4, 3);
        
        SetImageByRaw(Assets, Asset_ImageAtlasDefault, AtlasWidth, AtlasHeight, AtlasChannels, AtlasBitmap);
        
        
        // NOTE(Momo): Read Atlas for user data
        for (u32 i = 0 ; i < Atlas->EntryCount; ++i ){
            auto* Entry = Atlas->Entries + i;
            auto Rect = *(Atlas->Rects + Entry->RectIndex);
            
            auto Type = *(load_type*)Entry->UserContext; 
            switch(Type) {
                case LoadType_Image: {
                    auto* UserData = (load_context_image*)Entry->UserContext;
                    SetAtlasRect(Assets, UserData->AssetId, Rect, UserData->AtlasAssetId);
                } break;
                case LoadType_Font: {
                    auto* UserData = (load_context_font*)Entry->UserContext;
                    SetAtlasRect(Assets, UserData->AssetId, Rect, UserData->AtlasAssetId);
                } break;
            }
            
        }
        
#endif
        
    }
    Write(Assets, "yuu");
    
    return 0;
    
}

