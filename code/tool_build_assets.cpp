#include <stdio.h>
#include <stdlib.h>
#include "ryoji.h"

#define STB_IMAGE_IMPLEMENTATION
#include "thirdparty/stb/stb_image.h"


#include "game_assets.h"

#pragma pack(push, 1)
struct yuu_header {
    u32 Type;
    u32 OffsetToData;
};

struct yuu_image {
    u32 Width;
    u32 Height;
    u32 Channels;
    game_bitmap_handle Handle;
};
#pragma pack(pop)



struct asset_builder_entry_image {
    const char* Filename;
    game_bitmap_handle Handle;
};

struct asset_builder_entry_sound {
    // TODO(Momo): 
};

struct asset_builder_entry_font {
    // TODO(Momo): 
};

struct asset_builder_entry_spritesheet {
    // TODO(Momo): 
};

struct asset_builder_entry  {
    asset_type Type;
    union {
        asset_builder_entry_image Image;
        asset_builder_entry_sound Sound;
        asset_builder_entry_font Font;
        asset_builder_entry_spritesheet Spritesheet;
    };
    
};

#define BIG_NUMBER 1024
struct asset_builder {
    // NOTE(Momo): For preparation before writing
    asset_builder_entry Entries[BIG_NUMBER]; // TODO(Momo): Make this dynamic?
    u32 EntryCount;
};

static inline void
Init(asset_builder* Assets) {
    Assets->EntryCount = 0;
}

static inline asset_builder_entry*
AddAssetEntry(asset_builder* Assets) {
    Assert(Assets->EntryCount != BIG_NUMBER);
    asset_builder_entry* Ret = &Assets->Entries[Assets->EntryCount];
    ++Assets->EntryCount;
    return Ret;
}


static inline void
AddImage(asset_builder* Assets, const char* Filename, game_bitmap_handle Handle) {
    asset_builder_entry* Entry = AddAssetEntry(Assets);
    Entry->Type = asset_type::Image;
    Entry->Image.Filename = Filename;
    Entry->Image.Handle = Handle;
}


static inline b32 
WriteToFile(asset_builder* Assets, const char* Filename) {
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
    fwrite(&Assets->EntryCount, sizeof(Assets->EntryCount), sizeof(Assets->EntryCount), pFile);
    HeaderAt += sizeof(Assets->EntryCount);
    
    
    usize DataAt = HeaderAt + (sizeof(yuu_header) * Assets->EntryCount);
    // NOTE(Momo): Fill the file with stuff until DataAt 
    for (u32 i = 0; i < 100; ++i ) {
        fputc(0, pFile);
    }
    
    
    // NOTE(Momo): Write the data
    for (u32 i = 0; i < Assets->EntryCount; ++i)
    {
        asset_builder_entry* Entry = &Assets->Entries[i];
        
        switch(Entry->Type) {
            case asset_type::Image: {
                const char* ImageFilename = Entry->Image.Filename;
                // TODO(Momo): Maybe we can support ImageComp on game side?
                i32 ImageWidth, ImageHeight, ImageComp;
                u8* LoadedImage = stbi_load(ImageFilename, &ImageWidth, &ImageHeight, &ImageComp, 0);
                if (LoadedImage == nullptr) {
                    printf("Error loading image: %s\n", ImageFilename);
                    return false;
                }
                
                // NOTE(Momo): Write Header
                {
                    fseek(pFile, (i32)HeaderAt, SEEK_SET);
                    
                    yuu_header Header = { 
                        (u32)asset_type::Image,
                        (u32)DataAt,
                    };
                    
                    fwrite(&Header, sizeof(yuu_header), 1, pFile); 
                    HeaderAt += sizeof(yuu_header);
                }
                
                // NOTE(Momo): Write Data
                {
                    fseek(pFile, (i32)DataAt, SEEK_SET);
                    
                    yuu_image Image = {
                        (u32)ImageWidth,
                        (u32)ImageHeight,
                        (u32)ImageComp,
                        Entry->Image.Handle,
                    };
                    fwrite(&Image, sizeof(yuu_image), 1, pFile); 
                    DataAt += sizeof(yuu_image);
                    
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
            } break;
            case asset_type::Animation: {
            } break;
            case asset_type::Sound: {
            } break;
        }
        
    }
    
    return true;
}


int main() {
    // TODO(Momo): Options for this? (different assets for different renderers?)
    stbi_set_flip_vertically_on_load(true);
    
    
    // NOTE(Momo): Gather all the assets we need to load
    asset_builder Assets_ = {};
    asset_builder* Assets = &Assets_;
    
    
    {
        Init(Assets);
        
        AddImage(Assets, "assets/ryoji.png", GameBitmapHandle_Ryoji);
        AddImage(Assets, "assets/yuu.png", GameBitmapHandle_Yuu);
        
    }
    
    WriteToFile(Assets, "yuu");
    
    
    return 0;
    
}

