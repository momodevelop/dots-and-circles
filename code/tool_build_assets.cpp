#include "tool_build_assets.h"



int main() {
    atlas_builder Atlas_ = {};
    atlas_builder* Atlas = &Atlas_;
    {
        AddImage(Atlas, "assets/ryoji.png");
        AddImage(Atlas, "assets/yuu.png");
    }
    Assert(Build(Atlas, 128, 4096));
    Defer { Clean(Atlas); };
    
    
    // NOTE(Momo): Gather all the assets we need to load
    asset_builder Assets_ = {};
    asset_builder* Assets = &Assets_;
    {
        
        AddImage(Assets, "assets/ryoji.png", Asset_ImageRyoji);
        AddImage(Assets, "assets/yuu.png", Asset_ImageYuu);
        AddSpritesheet(Assets, "assets/karu.png", Asset_SpritesheetKaru, 4, 3);
        AddAtlas(Assets, Atlas, Asset_AtlasDefault);
    }
    Write(Assets, "yuu");
    
    return 0;
    
}

