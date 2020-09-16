#include "tool_build_assets.h"



int main() {
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
        
    }
    Assert(Build(Atlas, 128, 4096));
    
    
    
    // NOTE(Momo): Gather all the assets we need to load
    asset_builder Assets_ = {};
    asset_builder* Assets = &Assets_;
    Init(Assets, Asset_Count);
    Defer { Free(Assets); };
    {
        AddImage(Assets, "assets/ryoji.png", Asset_ImageRyoji);
        AddImage(Assets, "assets/yuu.png", Asset_ImageYuu);
        AddSpritesheet(Assets, "assets/karu.png", Asset_SpritesheetKaru, 4, 3);
        AddAtlas(Assets, Atlas, Asset_AtlasDefault);
        
        for(u32 i = 0; i < ListCount(Atlas->Entries); ++i) {
            auto* AtlasEntry = Atlas->Entries + i;
            rect2u Rect = *(Atlas->Rects + AtlasEntry->Image.RectIndex);
            AddAtlasRect(Assets, Rect, Asset_AtlasDefault, (asset_id)AtlasEntry->Id);
        }
        
        
    }
    Write(Assets, "yuu");
    
    return 0;
    
}

