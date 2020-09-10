#include "tool_build_assets.h"



int main() {
    // NOTE(Momo): Gather all the assets we need to load
    asset_builder Assets_ = {};
    asset_builder* Assets = &Assets_;
    {
        AddImage(Assets, "assets/ryoji.png", Asset_ImageRyoji);
        AddImage(Assets, "assets/yuu.png", Asset_ImageYuu);
        AddSpritesheet(Assets, "assets/karu.png", Asset_SpritesheetKaru, 4, 3);
        AddFont(Assets, "assets/CodeNewRoman.otf", Asset_FontDefault, 36.f, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");
    }
    
    WriteToFile(Assets, "yuu");
    
    return 0;
    
}

