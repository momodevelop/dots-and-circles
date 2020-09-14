#include "tool_build_assets.h"



int main() {
    atlas_builder Atlas_ = {};
    atlas_builder* Atlas = &Atlas_;
    {
        AddImage(Atlas, "assets/ryoji.png", AtlasEntry_Default_Ryoji);
        AddImage(Atlas, "assets/yuu.png", AtlasEntry_Default_Yuu);
        
        AddImage(Atlas, "assets/karu00.png", AtlasEntry_Default_Karu00);
        AddImage(Atlas, "assets/karu01.png", AtlasEntry_Default_Karu01);
        AddImage(Atlas, "assets/karu02.png", AtlasEntry_Default_Karu02);
        AddImage(Atlas, "assets/karu10.png", AtlasEntry_Default_Karu10);
        AddImage(Atlas, "assets/karu11.png", AtlasEntry_Default_Karu11);
        AddImage(Atlas, "assets/karu12.png", AtlasEntry_Default_Karu12);
        AddImage(Atlas, "assets/karu20.png", AtlasEntry_Default_Karu20);
        AddImage(Atlas, "assets/karu21.png", AtlasEntry_Default_Karu21);
        AddImage(Atlas, "assets/karu22.png", AtlasEntry_Default_Karu22);
        AddImage(Atlas, "assets/karu30.png", AtlasEntry_Default_Karu30);
        AddImage(Atlas, "assets/karu31.png", AtlasEntry_Default_Karu31);
        AddImage(Atlas, "assets/karu32.png", AtlasEntry_Default_Karu32);
        
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

