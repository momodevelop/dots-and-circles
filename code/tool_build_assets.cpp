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


int main() {
    
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

