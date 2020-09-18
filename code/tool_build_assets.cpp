#include "tool_build_assets.h"



int main() {
    stbtt_fontinfo LoadedFont;
    {
        FILE* File = fopen("Default.ttf", "rb");
        if (File == nullptr) {
            return 0;
        }
        fseek(File, 0, SEEK_END);
        auto Size = ftell(File);
        fseek(File, 0, SEEK_SET);
        u8* Buffer = malloc(Size);
        fread(Buffer, 1, Size);
        stbtt_InitFont(&LoadedFont, Buffer, 0);
        free(Buffer);
    }
    
    atlas_builder Atlas_ = {};
    atlas_builder* Atlas = &Atlas_;
    Init(Atlas, AtlasDefault_Count);
    Defer { Free(Atlas); };
    {
        AddImage(Atlas, "assets/ryoji.png", Asset_RectRyoji);
        AddImage(Atlas, "assets/yuu.png", Asset_RectYuu);
        
        AddImage(Atlas, "assets/karu00.png", Asset_RectKaru00);
        AddImage(Atlas, "assets/karu01.png", Asset_RectKaru01);
        AddImage(Atlas, "assets/karu02.png", Asset_RectKaru02);
        AddImage(Atlas, "assets/karu10.png", Asset_RectKaru10);
        AddImage(Atlas, "assets/karu11.png", Asset_RectKaru11);
        AddImage(Atlas, "assets/karu12.png", Asset_RectKaru12);
        AddImage(Atlas, "assets/karu20.png", Asset_RectKaru20);
        AddImage(Atlas, "assets/karu21.png", Asset_RectKaru21);
        AddImage(Atlas, "assets/karu22.png", Asset_RectKaru22);
        AddImage(Atlas, "assets/karu30.png", Asset_RectKaru30);
        AddImage(Atlas, "assets/karu31.png", Asset_RectKaru31);
        AddImage(Atlas, "assets/karu32.png", Asset_RectKaru32);
        
        AddFont(Atlas, 'a', LoadedFont)
            
    }
    if (!Build(Atlas, 128, 4096)) {
        Assert(false);
    }
    
    
    
    // NOTE(Momo): Gather all the assets we need to load
    asset_builder Assets_ = {};
    asset_builder* Assets = &Assets_;
    Init(Assets, Asset_Count);
    Defer { Free(Assets); };
    {
        SetImage(Assets, Asset_ImageRyoji, "assets/ryoji.png");
        SetImage(Assets, Asset_ImageYuu, "assets/yuu.png");
        SetSpritesheet(Assets, Asset_SpritesheetKaru, "assets/karu.png", 4, 3);
        SetAtlasImage(Assets, Asset_ImageAtlasDefault, Atlas);
        
        for(u32 i = 0; i < DynBufferCount(Atlas->Entries); ++i) {
            auto* AtlasEntry = Atlas->Entries + i;
            rect2u Rect = *(Atlas->Rects + AtlasEntry->Image.RectIndex);
            SetAtlasRect(Assets, (asset_id)AtlasEntry->Id, Rect, Asset_ImageAtlasDefault);
        }
#endif
        
    }
    Write(Assets, "yuu");
    
    return 0;
    
}

