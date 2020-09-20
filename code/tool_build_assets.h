#ifndef TOOL_BUILD_ASSETS_H
#define TOOL_BUILD_ASSETS_H

#include <stdio.h>
#include <stdlib.h>
#include "ryoji.h"
#include "ryoji_maths.h"
#include "ryoji_dynamic_buffer.h"

#define STB_IMAGE_IMPLEMENTATION
#include "thirdparty/stb/stb_image.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "thirdparty/stb/stb_truetype.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "thirdparty/stb/stb_image_write.h"

#include "game_assets_file_formats.h"  

struct asset_source_image {
    const char* Filename;
};

struct asset_source_atlas_rect {
    rect2u Rect;
    asset_id AtlasAssetId;
};

struct asset_source_spritesheet {
    const char* Filename;
    u32 Rows;
    u32 Cols;
};

struct asset_source  {
    asset_type Type;
    asset_id Id;
    union {
        asset_source_image Image;
        asset_source_spritesheet Spritesheet;
        asset_source_atlas_rect AtlasRect;
    };
    
};


struct asset_builder {
    asset_source* Entries; // List
};

static inline void 
Init(asset_builder* Builder, u32 Capacity) {
    DynBufferAdd(Builder->Entries, Capacity);
}

static inline void
Free(asset_builder* Builder) {
    DynBufferFree(Builder->Entries);
}

static inline asset_source*
SetEntry(asset_builder* Assets, asset_id Id, asset_type Type) {
    asset_source* Ret = Assets->Entries + Id;
    Ret->Type = Type;
    Ret->Id = Id;
    return Ret;
}

static inline void
SetImage(asset_builder* Assets, asset_id Id, const char* Filename) {
    asset_source* Entry = SetEntry(Assets, Id, AssetType_Image);
    Entry->Image.Filename = Filename;
}

static inline void 
SetAtlasRect(asset_builder* Assets, asset_id Id, rect2u Rect, asset_id AtlasAssetId) {
    asset_source* Entry = SetEntry(Assets, Id, AssetType_AtlasRect);
    Entry->AtlasRect.Rect = Rect;
    Entry->AtlasRect.AtlasAssetId = AtlasAssetId;
}

static inline void 
SetSpritesheet(asset_builder* Assets, 
               asset_id Id,
               const char* Filename, 
               u32 Rows, 
               u32 Columns) 
{
    asset_source* Entry = SetEntry(Assets, Id, AssetType_Spritesheet);
    Entry->Spritesheet.Filename = Filename;
    Entry->Spritesheet.Rows = Rows;
    Entry->Spritesheet.Cols = Columns;
}

static inline void 
WriteSpritesheetData(asset_builder* Assets, FILE* File, asset_source_spritesheet* Spritesheet) 
{
    i32 Width, Height, Comp;
    u8* Loaded = stbi_load(Spritesheet->Filename, &Width, &Height, &Comp, 0);
    Assert(Loaded != nullptr);
    Defer { stbi_image_free(Loaded); };
    
    // NOTE(Momo): Write Data
    yuu_spritesheet FileSpritesheet = {};
    FileSpritesheet.Width = (u32)Width;
    FileSpritesheet.Height = (u32)Height;
    FileSpritesheet.Channels = (u32)Comp;
    FileSpritesheet.Rows = Spritesheet->Rows;
    FileSpritesheet.Cols = Spritesheet->Cols;
    
    fwrite(&FileSpritesheet, sizeof(yuu_spritesheet), 1, File); 
    
    u32 Size = (Width * Height * Comp);
    for (u8* Itr = Loaded; Itr < Loaded + Size; ++Itr) 
    {
        fwrite(Itr, 1, 1, File); 
    }
    
    printf("Loaded Spritesheet '%s': Width = %d, Height = %d, Comp = %d, Rows = %d, Cols = %d\n", Spritesheet->Filename, Width, Height, Comp, Spritesheet->Rows, Spritesheet->Cols);
    
}

static inline void 
WriteImageData(asset_builder* Assets, FILE* File, asset_source_image* Image) 
{
    u8* LoadedImage = nullptr;
    u32 Width = 0, Height = 0, Channels = 0;
    {
        i32 W, H, C;
        LoadedImage = stbi_load(Image->Filename, &W, &H, &C, 0);
        Assert(LoadedImage != nullptr);
        Width = (u32)W;
        Height = (u32)H; 
        Channels = (u32)C;
    }
    Defer { stbi_image_free(LoadedImage); };
    
    yuu_image FileImage = {};
    FileImage.Width = Width;
    FileImage.Height = Height;
    FileImage.Channels = Channels;
    
    // NOTE(Momo): Write the data info
    fwrite(&FileImage, sizeof(yuu_image), 1, File); 
    
    
    // NOTE(Momo): Write raw data
    u32 ImageSize = Width * Height * Channels;
    for (u8* Itr = LoadedImage; Itr < LoadedImage + ImageSize; ++Itr) {
        fwrite(Itr, 1, 1, File); 
    }
    
    printf("Loaded Image:  Width = %d, Height = %d, Channels = %d\n",  Width, Height, Channels);
    
}


static inline void
WriteAtlasRectData(asset_builder* Assets, FILE* File, asset_source_atlas_rect* AtlasRect) {
    
    yuu_atlas_rect YuuAtlasRect  = {};
    YuuAtlasRect.Rect = AtlasRect->Rect;
    YuuAtlasRect.AtlasAssetId = AtlasRect->AtlasAssetId;
    
    // NOTE(Momo): Write the data info
    fwrite(&YuuAtlasRect, sizeof(yuu_atlas_rect), 1, File); 
    
    
    printf("Loaded Atlas Rect: MinX = %d, MinY = %d, MaxX = %d, MaxY = %d\n", AtlasRect->Rect.Min.X, AtlasRect->Rect.Min.Y, AtlasRect->Rect.Max.X, AtlasRect->Rect.Max.Y);
    
}

static inline b32 
Write(asset_builder* Assets, const char* Filename) {
    FILE* OutFile = fopen(Filename, "wb");
    if (OutFile == nullptr) {
        printf("Cannot open %s\n", Filename);
        return false;
    }
    Defer { fclose(OutFile); };
    printf("%s opened!\n", Filename);
    
    usize HeaderAt = (usize)ftell(OutFile);
    
    // NOTE(Momo): Write signature
    fwrite(AssetSignature, sizeof(u8), sizeof(AssetSignature), OutFile);
    HeaderAt += sizeof(AssetSignature);
    
    // NOTE(Momo): Write the amount of items
    u32 EntryCount = DynBufferCount(Assets->Entries);
    fwrite(&EntryCount, sizeof(EntryCount), 1, OutFile);
    HeaderAt += sizeof(EntryCount);
    
    usize DataAt = HeaderAt + (sizeof(yuu_entry) * EntryCount);
    
    // NOTE(Momo): Write the data
    for (u32 i = 0; i < EntryCount; ++i)
    {
        asset_source* Entry = Assets->Entries + i;
        // NOTE(Momo): Write Header
        fseek(OutFile, (i32)HeaderAt, SEEK_SET);
        {
            yuu_entry Header = {};
            Header.Type = Entry->Type;
            Header.OffsetToData = (u32)DataAt;
            Header.Id = Entry->Id;
            
            fwrite(&Header, sizeof(yuu_entry), 1, OutFile); 
            HeaderAt += sizeof(yuu_entry);
        }
        
        
        // NOTE(Momo): Write Data
        fseek(OutFile, (i32)DataAt, SEEK_SET);
        switch(Entry->Type) {
            case AssetType_Image: {
                WriteImageData(Assets, OutFile, &Entry->Image);
            } break;
            case AssetType_Spritesheet: {
                WriteSpritesheetData(Assets, OutFile, &Entry->Spritesheet);
            } break;
            case AssetType_AtlasRect: {
                WriteAtlasRectData(Assets, OutFile, &Entry->AtlasRect);
            } break;
            default: {
                printf("Unknown Type = %d\n", Entry->Type);
                Assert(false);
            }
        }
        DataAt = ftell(OutFile);
    }
    
    return true;
}

#endif //TOOL_BUILD_ASSETS_H
