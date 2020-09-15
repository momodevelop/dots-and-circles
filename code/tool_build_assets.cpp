#include "tool_build_assets.h"



int main() {
    atlas_builder Atlas_ = {};
    atlas_builder* Atlas = &Atlas_;
    Init(Atlas, AtlasDefault_Count);
    Defer { Free(Atlas); };
    {
        AddImage(Atlas, "assets/ryoji.png", AtlasDefault_Ryoji);
        AddImage(Atlas, "assets/yuu.png", AtlasDefault_Yuu);
        
        AddImage(Atlas, "assets/karu00.png", AtlasDefault_Karu00);
        AddImage(Atlas, "assets/karu01.png", AtlasDefault_Karu01);
        AddImage(Atlas, "assets/karu02.png", AtlasDefault_Karu02);
        AddImage(Atlas, "assets/karu10.png", AtlasDefault_Karu10);
        AddImage(Atlas, "assets/karu11.png", AtlasDefault_Karu11);
        AddImage(Atlas, "assets/karu12.png", AtlasDefault_Karu12);
        AddImage(Atlas, "assets/karu20.png", AtlasDefault_Karu20);
        AddImage(Atlas, "assets/karu21.png", AtlasDefault_Karu21);
        AddImage(Atlas, "assets/karu22.png", AtlasDefault_Karu22);
        AddImage(Atlas, "assets/karu30.png", AtlasDefault_Karu30);
        AddImage(Atlas, "assets/karu31.png", AtlasDefault_Karu31);
        AddImage(Atlas, "assets/karu32.png", AtlasDefault_Karu32);
        
    }
    Assert(Build(Atlas, 128, 4096));
    
    
    
    // NOTE(Momo): Gather all the assets we need to load
    asset_builder Assets_ = {};
    asset_builder* Assets = &Assets_;
    Init(Assets, Asset_Count);
    Defer { Free(Atlas); };
    {
        AddImage(Assets, "assets/ryoji.png", Asset_ImageRyoji);
        AddImage(Assets, "assets/yuu.png", Asset_ImageYuu);
        AddSpritesheet(Assets, "assets/karu.png", Asset_SpritesheetKaru, 4, 3);
        AddAtlas(Assets, Atlas, Asset_AtlasDefault);
    }
    Write(Assets, "yuu");
    
    return 0;
    
}

