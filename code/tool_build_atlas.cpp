#include <stdio.h>

#include "ryoji_atlas_builder.h"

#define STB_IMAGE_IMPLEMENTATION
#include "thirdparty/stb/stb_image.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "thirdparty/stb/stb_truetype.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "thirdparty/stb/stb_image_write.h"

#include "game_assets_file_formats.h" 


// Extensions
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
AddImage(atlas_builder* Builder, const char* Filename) {
    i32 W, H, C;
    unsigned char* BitmapData = stbi_load(Filename, &W, &H, &C, 0);
    Defer { stbi_image_free(BitmapData); };
    
    AddEntry(Builder, (u32)W, (u32)H, (u32)C, BitmapData);
}




int main() {
    
    atlas_builder Atlas_ = {};
    atlas_builder* Atlas = &Atlas_;
    Defer { Free(Atlas); };
    Begin(Atlas);
    {
        AddImage(Atlas, "assets/ryoji.png");
        AddImage(Atlas, "assets/yuu.png");
        AddImage(Atlas, "assets/karu00.png");
        AddImage(Atlas, "assets/karu01.png");
        AddImage(Atlas, "assets/karu02.png");
        AddImage(Atlas, "assets/karu10.png");
        AddImage(Atlas, "assets/karu11.png");
        AddImage(Atlas, "assets/karu12.png");
        AddImage(Atlas, "assets/karu20.png");
        AddImage(Atlas, "assets/karu21.png");
        AddImage(Atlas, "assets/karu22.png");
        AddImage(Atlas, "assets/karu30.png");
        AddImage(Atlas, "assets/karu31.png");
        AddImage(Atlas, "assets/karu32.png");
    }
    End(Atlas);
    WriteBitmapToPng(Atlas, "atlas.png");
}