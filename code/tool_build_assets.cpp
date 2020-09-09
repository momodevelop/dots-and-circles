#include "tool_build_assets.h"



int main() {
#if 0
    //https://github.com/justinmeiners/stb-truetype-example/blob/master/main.c
    const char* Filename = "assets/CodeNewRoman.otf";
    u8* FontBuffer = nullptr; 
    {
        FILE* pFile = fopen(Filename, "rb");
        if (pFile == nullptr) {
            printf("Cannot open %s\n", Filename);
            return false;
        }
        Defer { fclose(pFile); };
        printf("%s opened!\n", Filename);
        
        fseek(pFile, 0, SEEK_END);
        u32 size = ftell(pFile); /* how long is the file ? */
        fseek(pFile, 0, SEEK_SET); /* reset */
        
        FontBuffer = (u8*)malloc(size);
        fread(FontBuffer, size, 1, pFile);
    }
    Defer { free(FontBuffer); };
    
    stbtt_fontinfo FontInfo;
    if (!stbtt_InitFont(&FontInfo, FontBuffer, 0)) {
        printf("failed");
        return 0;
    }
    
    f32 Scale = stbtt_ScaleForPixelHeight(&FontInfo, 24); 
    const char* Characters = "abc";
    char OutFileBuffer[6];
    for (u32 i = 0; i < ArrayCount(Characters) - 1; ++i) {
        u32 Width = 0;
        u32 Height = 0;
        {
            i32 x1, y1, x2, y2;
            stbtt_GetCodepointBitmapBox(&FontInfo, Characters[i], Scale, Scale, &x1, &y1, &x2, &y2);
            Width = x2 - x1;
            Height = y2 - y1;
        }
        
        u8* Bitmap = (u8*)calloc(Width * Height, sizeof(u8));
        Defer { free(Bitmap); };
        
        stbtt_MakeCodepointBitmap(&FontInfo, Bitmap, Width, Height, Width, Scale, Scale, Characters[i]);
        
        OutFileBuffer[0] = Characters[i];
        OutFileBuffer[1] = '.';
        OutFileBuffer[2] = 'p';
        OutFileBuffer[3] = 'n';
        OutFileBuffer[4] = 'g';
        OutFileBuffer[5] = 0;
        stbi_write_png(OutFileBuffer, Width, Height, 1, Bitmap, Width);
    }
    
#else 
    // NOTE(Momo): Gather all the assets we need to load
    asset_builder Assets_ = {};
    asset_builder* Assets = &Assets_;
    {
        AddImage(Assets, "assets/ryoji.png", Asset_ImageRyoji);
        AddImage(Assets, "assets/yuu.png", Asset_ImageYuu);
        AddSpritesheet(Assets, "assets/karu.png", Asset_SpritesheetKaru, 4, 3);
        AddFont(Assets, "assets/font.otf", Asset_FontDefault, 36.f, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");
    }
    
    WriteToFile(Assets, "yuu");
#endif
    
    return 0;
    
}

