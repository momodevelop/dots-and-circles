#include "tool_build_assets.h"

#include "ryoji_atlas_builder.h"
#include "ryoji_bitmanip.h"

static inline void 
ReadAtlasInfo(asset_builder* Assets, const char * Filename) {
    FILE* File = fopen(Filename, "rb");
    Assert(File);
    Defer { fclose(File); };
    
    
    atlas_builder_info_header Header; 
    fread(&Header, sizeof(Header), 1, File);
    
    
    for(u32 i = 0; i < Header.EntryCount; ++i ) {
        atlas_builder_info_entry Entry; 
        fread(&Entry, sizeof(Entry), 1, File);
        
        // peeking
        yuu_atlas_ud_type Type = {};
        {
            auto RecallPoint = ftell(File);
            fread(&Type, sizeof(Type), 1, File);
            fseek(File, RecallPoint, SEEK_SET);
        }
        
        switch(Type) {
            case YuuAtlasUserDataType_Image: {
                yuu_atlas_ud_image ImageData = {};
                fread(&ImageData, sizeof(ImageData), 1, File);
                SetAtlasRect(Assets, ImageData.AssetId, Entry.Rect, ImageData.AtlasAssetId);
            } break;
        }
        
    }
}

#include "ryoji_atlas_builder.h"

// NOTE(Momo): Bitmap
struct loaded_bitmap {
    u32 Width;
    u32 Height;
    u32 Channels;
    u8* Data;
};

static inline loaded_bitmap
AllocateBitmapFromFile(const char* Filename) {
    i32 W, H, C;
    u8* BitmapData = stbi_load(Filename, &W, &H, &C, 0);
    return { (u32)W, (u32)H, (u32)Channels, BitmapData };
}

static inline void
FreeBitmap(loaded_bitmap Bitmap) {
    stbi_image_free(Bitmap.Data);
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
    Defer { fclose(FontFile); }
    
    fseek(FontFile, 0, SEEK_END);
    auto Size = ftell(FontFile);
    fseek(FontFile, 0, SEEK_SET);
    
    void* Buffer = malloc(Size);
    fread(Buffer, Size, 1, FontFile);
    stbtt_InitFont(&Font, Buffer, 0);
    
    return { Font, Buffer };
}

static inline void
FreeFont(loaded_font Font) {
    free(Font.Data);
}

int main() {
#if 0 
    atlas_builder<128> Atlas;
    {
        // NOTE(Momo): Load resources
        loaded_font Font = AllocateFontFromFile("assets/DroidSansMono.ttf");
        loaded_bitmap Bitmaps[128];
        u32 BitmapCount = 0;
        
        Bitmaps[BitmapCount++] = AllocateBitmapFromFile();
        Defer { 
            FreeFont(Font); 
            for (u32 i = 0; i < BitmapCount; ++i ) 
                FreeBitmap(Bitmaps[i]);
            
        };
        
        // NOTE(Momo): Add to atlas
        
        Begin(Atlas);
        
        
        
        End(Atlas);
        
        
        
        
    }
#endif
    
    
    
    
    
    // NOTE(Momo): Gather all the assets we need to load
    asset_builder Assets_ = {};
    asset_builder* Assets = &Assets_;
    Init(Assets, Asset_Count);
    Defer { Free(Assets); };
    {
        SetImage(Assets, Asset_ImageRyoji, "assets/ryoji.png");
        SetImage(Assets, Asset_ImageYuu, "assets/yuu.png");
        SetSpritesheet(Assets, Asset_SpritesheetKaru, "assets/karu.png", 4, 3);
        SetImage(Assets, Asset_ImageAtlasDefault, "assets/atlas.png");
        
        ReadAtlasInfo(Assets, "assets/atlas_info.dat");
        
    }
    Write(Assets, "yuu");
    
    return 0;
    
}

