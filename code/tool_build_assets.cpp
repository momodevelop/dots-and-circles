#include <stdio.h>
#include <stdlib.h>
#include "ryoji.h"

#define STB_IMAGE_IMPLEMENTATION
#include "thirdparty/stb/stb_image.h"


#include "game_assets.h"


#define MAX_ENTRIES 1024

#pragma pack(push, 1)
struct yuu_header {
    asset_type Type;
    u32 OffsetToData;
    asset_id AssetId;
};

struct yuu_image {
    u32 Width;
    u32 Height;
    u32 Channels;
};


struct yuu_spritesheet {
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
    asset_id AssetId;
    union {
        asset_builder_entry_image Image;
        asset_builder_entry_sound Sound;
        asset_builder_entry_font Font;
        asset_builder_entry_spritesheet Spritesheet;
    };
    
};


struct asset_builder {
    // NOTE(Momo): For preparation before writing
    asset_builder_entry Entries[MAX_ENTRIES]; 
    u32 EntryCount;
};

static inline void
Init(asset_builder* Assets) {
    Assets->EntryCount = 0;
}

static inline asset_builder_entry*
AddAssetEntry(asset_builder* Assets) {
    Assert(Assets->EntryCount != MAX_ENTRIES);
    asset_builder_entry* Ret = &Assets->Entries[Assets->EntryCount];
    ++Assets->EntryCount;
    return Ret;
}


static inline void
AddImage(asset_builder* Assets, const char* Filename, asset_id AssetId) {
    asset_builder_entry* Entry = AddAssetEntry(Assets);
    Entry->Type = asset_type::Image;
    Entry->AssetId = AssetId;
    Entry->Image.Filename = Filename;
    
}

static inline void 
AddSpritesheet(asset_builder* Assets, 
               const char* Filename, 
               asset_id AssetId,  
               u32 Rows, 
               u32 Columns) 
{
    asset_builder_entry* Entry = AddAssetEntry(Assets);
    Entry->Type = asset_type::Spritesheet;
    Entry->AssetId = AssetId;
    Entry->Spritesheet.Filename = Filename;
    Entry->Spritesheet.Rows = Rows;
    Entry->Spritesheet.Cols = Columns;
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
    fwrite(&Assets->EntryCount, sizeof(Assets->EntryCount), sizeof(Assets->EntryCount), pFile);
    HeaderAt += sizeof(Assets->EntryCount);
    
    
    usize DataAt = HeaderAt + (sizeof(yuu_header) * Assets->EntryCount);
    
    
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
                        (u32)Entry->AssetId,
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
                        (u32)asset_type::Spritesheet,
                        (u32)DataAt,
                        (u32)Entry->AssetId,
                    };
                    
                    fwrite(&Header, sizeof(yuu_header), 1, pFile); 
                    HeaderAt += sizeof(yuu_header);
                }
                
                // NOTE(Momo): Write Data
                {
                    fseek(pFile, (i32)DataAt, SEEK_SET);
                    
                    yuu_spritesheet Spritesheet = {
                        (u32)ImageWidth,
                        (u32)ImageHeight,
                        (u32)ImageComp,
                        Entry->Spritesheet.Rows,
                        Entry->Spritesheet.Cols,
                    };
                    fwrite(&Spritesheet, sizeof(yuu_spritesheet), 1, pFile); 
                    DataAt += sizeof(yuu_spritesheet);
                    
                    u32 ImageSize = (ImageWidth * ImageHeight * ImageComp);
                    for (u8* Itr = LoadedImage; Itr < LoadedImage + ImageSize; ++Itr) 
                    {
                        fwrite(Itr, 1, 1, pFile); 
                        ++DataAt;
                    }
                }
                
                printf("Loaded Spritesheet '%s': Width = %d, Height = %d, Comp = %d, Rows = %d, Cols = %d\n", ImageFilename, ImageWidth, ImageHeight, ImageComp, Entry->Spritesheet.Rows, Entry->Spritesheet.Cols);
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
        
        AddImage(Assets, "assets/ryoji.png", asset_id::Image_Ryoji);
        AddImage(Assets, "assets/yuu.png", asset_id::Image_Yuu);
        
        AddSpritesheet(Assets, "assets/karu.png", asset_id::Spritesheet_Karu, 4, 3);
        
    }
    
    WriteToFile(Assets, "yuu");
    
    
    return 0;
    
}
