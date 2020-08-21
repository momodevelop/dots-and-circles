#include <stdio.h>
#include <stdlib.h>
#include "ryoji.h"

#define STB_IMAGE_IMPLEMENTATION
#include "thirdparty/stb/stb_image.h"


#include "game_assets.h"
char Signature[] = { 'M', 'O', 'M', 'O' };


enum struct asset_type : u32 {
    Image,
    Font,
    Spritesheet,
    Animation,
    Sound,
};


struct asset_header {
    asset_type Type;
    u32 OffsetToData;
    u32 Handle;
};

struct asset_headers {
    asset_header Headers[1];
};

struct asset_bitmap {
    u32 Width;
    u32 Height;
};

/*
template<typename T> 
Write(FILE* File) {
    T item = 
        fwrite(
}*/

int main() {
    const char* filename = "asset.yuu";
    FILE* pFile = fopen(filename, "wb");
    if (pFile == nullptr) {
        printf("Cannot open %s\n", filename);
        return 1;
    }
    Defer { fclose(pFile); };
    printf("%s opened!\n", filename);
    fwrite(Signature, sizeof(u8), sizeof(Signature), pFile);
    
    i32 HeaderStart = ftell(pFile);
    i32 HeaderAt = HeaderStart;
    i32 DataStart = HeaderStart + sizeof(u32)*3 * 1;
    i32 DataAt = DataStart;
    
    printf("%d\n", DataAt);
    
    // NOTE(Momo): Image 
    {
        const char* Filename = "assets/test.png";
        i32 ImageWidth, ImageHeight, ImageComp;
        u8* LoadedImage = stbi_load(Filename, &ImageWidth, &ImageHeight, &ImageComp, 0);
        if (LoadedImage == nullptr) {
            printf("Error loading image: %s\n", filename);
            return 1;
        }
        Defer { stbi_image_free(LoadedImage); };
        
        printf("Loaded %s: Width = %d, Height = %d, Comp = %d\n", Filename, ImageWidth, ImageHeight, ImageComp);
        
        // NOTE(Momo): Write Header 
        {
            fseek(pFile, HeaderAt, SEEK_SET);
            
            u32 Type = (u32)asset_type::Image;
            fwrite(&Type, sizeof(Type), 1, pFile); 
            HeaderAt += (i32)sizeof(Type);
            
            u32 OffsetToData = DataAt;
            fwrite(&OffsetToData, sizeof(OffsetToData), 1, pFile); 
            HeaderAt += (i32)sizeof(OffsetToData);
            
            u32 Handle = GameBitmapHandle_Blank;
            fwrite(&Handle, sizeof(Handle), 1, pFile); 
            HeaderAt += (i32)sizeof(Handle);
        }
        
        // NOTE(Momo): Write Data 
        {
            fseek(pFile, DataAt, SEEK_SET);
            
            u32 Width = ImageWidth;
            fwrite(&Width, sizeof(Width), 1, pFile); 
            DataAt += (i32)sizeof(Width);
            
            u32 Height = ImageHeight;
            fwrite(&Height, sizeof(Height), 1, pFile); 
            DataAt += (i32)sizeof(Height);
        }
        
        
        for (u8* Itr = LoadedImage; Itr < LoadedImage + (ImageWidth * ImageHeight * 4); ++Itr) 
        {
            fwrite(Itr, 1, 1, pFile); 
            ++DataAt;
        }
    }  
    
    
    return 0;
    
}

