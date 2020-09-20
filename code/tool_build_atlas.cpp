#include <stdio.h>

#include "ryoji_atlas_builder.h"

#define STB_IMAGE_IMPLEMENTATION
#include "thirdparty/stb/stb_image.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "thirdparty/stb/stb_truetype.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "thirdparty/stb/stb_image_write.h"

#include "game_assets_file_formats.h" 


static inline void 
WriteBitmapToPng(atlas_builder* Builder, const char* PngFilename) {
    printf("[Write] Write to file started \n");
    Defer { printf("[Write] Write to file complete\n"); };
    
    auto Result = AllocateBitmap(Builder);
    Defer { FreeBitmap(Result); };
    
    Assert(Result.Ok);
    printf("\tWriting to %s\n", PngFilename); 
    stbi_write_png(PngFilename, Result.Width, Result.Height, Result.Channels, Result.Bitmap, Builder->Width * Builder->Channels);
}

static inline void 
WriteInfo(atlas_builder* Builder, const char* InfoFilename) {
    printf("[Write] Write to file started \n");
    Defer { printf("[Write] Write to file complete\n"); };
    
    auto Result = AllocateInfo(Builder);
    Assert(Result.Ok);
    Defer { FreeInfo(Result); };
    
    printf("\tWriting to %s\n", InfoFilename); 
    fwrite(Result.Memory, Result.Size, 1, fopen(InfoFilename, "wb"));
}


static inline void
AddImage(atlas_builder* Builder, const char* Filename, asset_id AssetId, asset_id TargetAtlasId) 
{
    yuu_atlas_ud_image Data = {};
    Data.Base.AssetId = AssetId;
    Data.Base.AtlasAssetId = TargetAtlasId;
    Data.Base.Type = YuuAtlasUserDataType_Image;
    
    i32 W, H, C;
    unsigned char* BitmapData = stbi_load(Filename, &W, &H, &C, 0);
    Defer { stbi_image_free(BitmapData); };
    
    AddEntry(Builder, (u32)W, (u32)H, (u32)C, BitmapData, &Data);
}

int main() {
    
    atlas_builder Atlas_ = {};
    atlas_builder* Atlas = &Atlas_;
    Defer { Free(Atlas); };
    
    Begin(Atlas);
    {
        asset_id TargetAtlasAssetId = Asset_ImageAtlasDefault;
        AddImage(Atlas, "assets/ryoji.png", Asset_RectRyoji, TargetAtlasAssetId);
        AddImage(Atlas, "assets/yuu.png", Asset_RectYuu, TargetAtlasAssetId);
        
        AddImage(Atlas, "assets/karu00.png", Asset_RectKaru00, TargetAtlasAssetId);
        AddImage(Atlas, "assets/karu01.png", Asset_RectKaru01, TargetAtlasAssetId);
        AddImage(Atlas, "assets/karu02.png", Asset_RectKaru02, TargetAtlasAssetId);
        AddImage(Atlas, "assets/karu10.png", Asset_RectKaru10, TargetAtlasAssetId);
        AddImage(Atlas, "assets/karu11.png", Asset_RectKaru11, TargetAtlasAssetId);
        AddImage(Atlas, "assets/karu12.png", Asset_RectKaru12, TargetAtlasAssetId);
        AddImage(Atlas, "assets/karu20.png", Asset_RectKaru20, TargetAtlasAssetId);
        AddImage(Atlas, "assets/karu21.png", Asset_RectKaru21, TargetAtlasAssetId);
        AddImage(Atlas, "assets/karu22.png", Asset_RectKaru22, TargetAtlasAssetId);
        AddImage(Atlas, "assets/karu30.png", Asset_RectKaru30, TargetAtlasAssetId);
        AddImage(Atlas, "assets/karu31.png", Asset_RectKaru31, TargetAtlasAssetId);
        AddImage(Atlas, "assets/karu32.png", Asset_RectKaru32, TargetAtlasAssetId);
    }
    End(Atlas);
    WriteBitmapToPng(Atlas, "assets/atlas.png");
    WriteInfo(Atlas, "assets/atlas_info.dat");
}