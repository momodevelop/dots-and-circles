#include "tool_build_assets.h"



int main() {
    // NOTE(Momo): Gather all the assets we need to load
    asset_builder Assets_ = {};
    asset_builder* Assets = &Assets_;
    {
        AddImage(Assets, "assets/ryoji.png", Asset_ImageRyoji);
        AddSpritesheet(Assets, "assets/karu.png", Asset_SpritesheetKaru, 4, 3);
        AddImage(Assets, "assets/yuu.png", Asset_ImageYuu);
        AddFont(Assets, "assets/CodeNewRoman.otf", Asset_FontDefault, 36.f, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");
        AddAtlas(Assets, "assets/yuu.atlas", Asset_AtlasDefault);
    }
    Write(Assets, "yuu");
    
    return 0;
    
}

