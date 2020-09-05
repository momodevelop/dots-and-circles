#include <stdio.h>
#include <stdlib.h>
#include "ryoji.h"

#define STB_IMAGE_IMPLEMENTATION
#include "thirdparty/stb/stb_image.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "thirdparty/stb/stb_truetype.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "thirdparty/stb/stb_image_write.h"


#include "game_asset_types.h"


#define MAX_ENTRIES 1024


#pragma pack(push, 1)
struct asset_file_entry {
    asset_type Type;
    u32 OffsetToData;
    union {
        image_id ImageId;
        spritesheet_id SpritesheetId;
        //font_id FontId;
        //sound_id SoundId;
    };
};

struct asset_file_data_image {
    u32 Width;
    u32 Height;
    u32 Channels;
};


struct asset_file_data_spritesheet {
    u32 Width;
    u32 Height;
    u32 Channels;
    
    u32 Rows;
    u32 Cols;
};
#pragma pack(pop)


struct asset_builder_entry_image {
    const char* Filename;
};

struct asset_builder_entry_sound {
    // TODO(Momo): 
};

struct asset_builder_entry_font {
    // TODO(Momo): 
};

struct asset_builder_entry_spritesheet {
    const char* Filename;
    u32 Rows;
    u32 Cols;
};

struct asset_builder_entry  {
    asset_type Type;
    union {
        image_id ImageId;
        spritesheet_id SpritesheetId;
        //font_id FontId;
        //sound_id SoundId;
    };
    
    union {
        asset_builder_entry_image Image;
        asset_builder_entry_sound Sound;
        asset_builder_entry_font Font;
        asset_builder_entry_spritesheet Spritesheet;
    };
    
};


struct asset_builder {
    asset_builder_entry Entries[MAX_ENTRIES];
    u32 EntryCount;
    
    u32 ImageCount;
    u32 SpritesheetCount;
    u32 FontCount;
    u32 SoundCount;
};


static inline asset_builder_entry*
AddAssetEntry(asset_builder* Assets) {
    Assert(Assets->EntryCount != MAX_ENTRIES);
    asset_builder_entry* Ret = &Assets->Entries[Assets->EntryCount];
    ++Assets->EntryCount;
    
    return Ret;
}


static inline void
AddImage(asset_builder* Assets, const char* Filename, image_id Id) {
    asset_builder_entry* Entry = AddAssetEntry(Assets);
    Entry->Type = asset_type::Image;
    Entry->ImageId = Id;
    Entry->Image.Filename = Filename;
    ++Assets->ImageCount;
}

static inline void 
AddSpritesheet(asset_builder* Assets, 
               const char* Filename, 
               spritesheet_id Id,  
               u32 Rows, 
               u32 Columns) 
{
    asset_builder_entry* Entry = AddAssetEntry(Assets);
    Entry->Type = asset_type::Spritesheet;
    Entry->SpritesheetId = Id;
    Entry->Spritesheet.Filename = Filename;
    Entry->Spritesheet.Rows = Rows;
    Entry->Spritesheet.Cols = Columns;
    ++Assets->SpritesheetCount;
}


static inline b32 
WriteToFile(asset_builder* Assets, const char* Filename) {
    // TODO(Momo): Maybe write a writer struct for all these states
    FILE* pFile = fopen(Filename, "wb");
    if (pFile == nullptr) {
        printf("Cannot open %s\n", Filename);
        return false;
    }
    Defer { fclose(pFile); };
    printf("%s opened!\n", Filename);
    
    usize HeaderAt = (usize)ftell(pFile);
    
    
    // NOTE(Momo): Write signature
    fwrite(AssetSignature, sizeof(u8), sizeof(AssetSignature), pFile);
    HeaderAt += sizeof(AssetSignature);
    
    
    // NOTE(Momo): Write the amount of items
    // TODO(Momo): Write a helper function for this?
    fwrite(&Assets->ImageCount, sizeof(Assets->ImageCount), 1, pFile);
    HeaderAt += sizeof(Assets->ImageCount);
    
    fwrite(&Assets->SpritesheetCount, sizeof(Assets->SpritesheetCount), 1, pFile);
    HeaderAt += sizeof(Assets->SpritesheetCount);
    
#if 0
    fwrite(&Assets->FontCount, sizeof(Assets->FontCount), 1, pFile);
    HeaderAt += sizeof(Assets->FontCount);
    
    fwrite(&Assets->SoundCount, sizeof(Assets->SoundCount), 1, pFile);
    HeaderAt += sizeof(Assets->SoundCount);
#endif
    
    usize DataAt = HeaderAt + (sizeof(asset_file_entry) * Assets->EntryCount);
    
    
    // NOTE(Momo): Write the data
    for (u32 i = 0; i < Assets->EntryCount; ++i)
    {
        asset_builder_entry* Entry = &Assets->Entries[i];
        
        switch(Entry->Type) {
            case asset_type::Image: {
                const char* ImageFilename = Entry->Image.Filename;
                i32 ImageWidth, ImageHeight, ImageComp;
                u8* LoadedImage = stbi_load(ImageFilename, &ImageWidth, &ImageHeight, &ImageComp, 0);
                if (LoadedImage == nullptr) {
                    printf("Error loading image: %s\n", ImageFilename);
                    return false;
                }
                Defer { stbi_image_free(LoadedImage); };
                
                // NOTE(Momo): Write Header
                {
                    fseek(pFile, (i32)HeaderAt, SEEK_SET);
                    
                    asset_file_entry Header = {};
                    Header.Type = asset_type::Image;
                    Header.OffsetToData = (u32)DataAt;
                    Header.ImageId = Entry->ImageId;
                    
                    fwrite(&Header, sizeof(asset_file_entry), 1, pFile); 
                    HeaderAt += sizeof(asset_file_entry);
                }
                
                // NOTE(Momo): Write Data
                {
                    fseek(pFile, (i32)DataAt, SEEK_SET);
                    
                    asset_file_data_image Image = {};
                    Image.Width = ImageWidth;
                    Image.Height = ImageHeight;
                    Image.Channels = ImageComp;
                    
                    fwrite(&Image, sizeof(asset_file_data_image), 1, pFile); 
                    DataAt += sizeof(asset_file_data_image);
                    
                    u32 ImageSize = (ImageWidth * ImageHeight * ImageComp);
                    for (u8* Itr = LoadedImage; Itr < LoadedImage + ImageSize; ++Itr) 
                    {
                        fwrite(Itr, 1, 1, pFile); 
                        ++DataAt;
                    }
                }
                
                printf("Loaded Image '%s': Width = %d, Height = %d, Comp = %d\n", ImageFilename, ImageWidth, ImageHeight, ImageComp);
                
            } break;
            case asset_type::Font: {
            } break;
            case asset_type::Spritesheet: {
                const char* ImageFilename = Entry->Image.Filename;
                // TODO(Momo): Maybe we can support ImageComp on game side?
                i32 ImageWidth, ImageHeight, ImageComp;
                u8* LoadedImage = stbi_load(ImageFilename, &ImageWidth, &ImageHeight, &ImageComp, 0);
                if (LoadedImage == nullptr) {
                    printf("Error loading image: %s\n", ImageFilename);
                    return false;
                }
                Defer { stbi_image_free(LoadedImage); };
                
                
                // NOTE(Momo): Write Header
                {
                    fseek(pFile, (i32)HeaderAt, SEEK_SET);
                    
                    asset_file_entry Header = {};
                    Header.Type = asset_type::Spritesheet;
                    Header.OffsetToData = (u32)DataAt;
                    Header.SpritesheetId = Entry->SpritesheetId;
                    
                    fwrite(&Header, sizeof(asset_file_entry), 1, pFile); 
                    HeaderAt += sizeof(asset_file_entry);
                }
                
                // NOTE(Momo): Write Data
                {
                    fseek(pFile, (i32)DataAt, SEEK_SET);
                    
                    asset_file_data_spritesheet Spritesheet = {};
                    Spritesheet.Width = (u32)ImageWidth;
                    Spritesheet.Height = (u32)ImageHeight;
                    Spritesheet.Channels = (u32)ImageComp;
                    Spritesheet.Rows = Entry->Spritesheet.Rows;
                    Spritesheet.Cols = Entry->Spritesheet.Cols;
                    
                    fwrite(&Spritesheet, sizeof(asset_file_data_spritesheet), 1, pFile); 
                    DataAt += sizeof(asset_file_data_spritesheet);
                    
                    u32 ImageSize = (ImageWidth * ImageHeight * ImageComp);
                    for (u8* Itr = LoadedImage; Itr < LoadedImage + ImageSize; ++Itr) 
                    {
                        fwrite(Itr, 1, 1, pFile); 
                        ++DataAt;
                    }
                }
                
                printf("Loaded Spritesheet '%s': Width = %d, Height = %d, Comp = %d, Rows = %d, Cols = %d\n", ImageFilename, ImageWidth, ImageHeight, ImageComp, Entry->Spritesheet.Rows, Entry->Spritesheet.Cols);
            } break;
            case asset_type::Sound: {
            } break;
        }
        
    }
    
    return true;
}


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
    // TODO(Momo): Options for this? (different assets for different renderers?)
    stbi_set_flip_vertically_on_load(true);
    
    
    // NOTE(Momo): Gather all the assets we need to load
    asset_builder Assets_ = {};
    asset_builder* Assets = &Assets_;
    {
        AddImage(Assets, "assets/ryoji.png", image_id::Ryoji);
        AddImage(Assets, "assets/yuu.png", image_id::Yuu);
        
        AddSpritesheet(Assets, "assets/karu.png", spritesheet_id::Karu, 4, 3);
    }
    
    WriteToFile(Assets, "yuu");
#endif
    
    return 0;
    
}

