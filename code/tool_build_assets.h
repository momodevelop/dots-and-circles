#ifndef TOOL_BUILD_ASSETS_H
#define TOOL_BUILD_ASSETS_H

#include <stdio.h>
#include <stdlib.h>
#include "ryoji.h"
#include "ryoji_maths.h"
#include "ryoji_dynamic_buffer.h"

#define DEBUG_PNG 1

#define STB_IMAGE_IMPLEMENTATION
#include "thirdparty/stb/stb_image.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "thirdparty/stb/stb_truetype.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "thirdparty/stb/stb_image_write.h"


#include "game_asset_types.h"
#include "game_atlas_types.h"

// NOTE(Momo): Atlas Builder ///////////////////////////////////////////////////////////

struct atlas_builder_entry_image {
    const char* Filename;
    u32 RectIndex;
};

struct atlas_builder_entry {
    atlas_entry_type Type;
    usize Id;
    union {
        atlas_builder_entry_image Image;
        // TODO(Momo): Other types here?
    };
};

struct atlas_builder {
    rect2u* Rects; // List
    rect2u** SortedRects; // List
    atlas_builder_entry* Entries; // List
    
    u32 Width;
    u32 Height;
    u32 Channels;
};


static inline u8* 
AllocateBitmap(atlas_builder* Builder) {
    u32 BitmapSize = Builder->Width * Builder->Height * Builder->Channels;
    u8* BitmapMemory = (u8*)calloc(BitmapSize, sizeof(u8));
    if (!BitmapMemory) {
        return nullptr;
    }
    
    // NOTE(Momo): Check. Total area of Rects must match 
    
    
    u32 EntryCount = DynBufferCount(Builder->Entries);
    for (u32 i = 0; i < EntryCount; ++i) {
        auto* Entry = Builder->Entries + i;
        
        // TODO(Momo): Cater for different types
        {
            auto* Image = &Entry->Image;
            rect2u Rect = *(Builder->Rects + Image->RectIndex);
            
            i32 W,H,C;
            
            u8* LoadedImage = stbi_load(Image->Filename, &W, &H, &C, 0);
            if (LoadedImage == nullptr) {
                return nullptr;
            }
            Defer { stbi_image_free(LoadedImage); };
            
            
            Assert((u32)W == GetWidth(Rect));
            Assert((u32)H == GetHeight(Rect));
            Assert((u32)C == Builder->Channels); 
            
            i32 j = 0;
            for (u32 y = Rect.Min.Y; y < Rect.Min.Y + GetHeight(Rect); ++y) {
                for (u32 x = Rect.Min.X; x < Rect.Min.X + GetWidth(Rect); ++x) {
                    u32 Index = TwoToOne(y, x, Builder->Width) * Builder->Channels;
                    Assert(Index < BitmapSize);
                    for (u32 c = 0; c < Builder->Channels; ++c) {
                        BitmapMemory[Index + c] = LoadedImage[j++];
                    }
                }
            }
        }
        
    }
    
    return BitmapMemory;
}

static inline void
FreeBitmap(u8* Memory) {
    free(Memory);
}

static inline u32
AddRect(atlas_builder* Builder, u32 W, u32 H) {
    rect2u Rect = {0, 0, W, H};
    DynBufferPush(Builder->Rects, Rect);
    return DynBufferCount(Builder->Rects) - 1;
}

static inline atlas_builder_entry*
AddEntry(atlas_builder* Builder, atlas_entry_type Type, usize Id) {
    atlas_builder_entry Entry = {};
    Entry.Id = Id;
    Entry.Type = Type;
    
    DynBufferPush(Builder->Entries, Entry);
    return &DynBufferLast(Builder->Entries);
}

static inline void 
AddImage(atlas_builder* Builder, const char* Filename, usize Id) {
    auto* Entry = AddEntry(Builder, AtlasEntryType_Image, Id);
    Entry->Image.Filename = Filename;
    
    i32 W, H;
    stbi_info(Filename, &W, &H, 0);
    Entry->Image.RectIndex = AddRect(Builder, (u32)W, (u32)H);
}



// NOTE(Momo): Sort by area. Maybe sort by other methods?
static inline i32
AtlasBuilderComparer(const void* Lhs, const void* Rhs) {
    const rect2u L = (**(rect2u**)(Lhs));
    const rect2u R = (**(rect2u**)(Rhs));
    
    
    auto LW = GetWidth(L);
    auto LH = GetHeight(L);
    auto RW = GetWidth(R);
    auto RH = GetHeight(R);
    
    auto LhsArea = LW * LH;
    auto RhsArea = RW * RH;
    if (LhsArea != RhsArea)
        return RhsArea - LhsArea;
    
    auto LhsPerimeter = LW + LH;
    auto RhsPerimeter = RW + RH;
    if (LhsPerimeter != RhsPerimeter)
        return RhsPerimeter - LhsPerimeter;
    
    // By bigger side
    auto LhsBiggerSide = Maximum(LW, LH);
    auto RhsBiggerSide = Maximum(RW, RH);
    if (LhsBiggerSide != RhsBiggerSide) 
        return RhsBiggerSide - LhsBiggerSide;
    
    // By Width
    if (LW != RW)
        return RW - LW;
    
    // By right
    if (LH != RH)
        return RH - LH;
    
    // pathological multipler
    auto LhsMultipler = Maximum(LW, LH)/Minimum(LW, LH) * LW * LH;
    auto RhsMultipler = Maximum(RW, RH)/Minimum(RW, RH) * RW * RH;
    return RhsMultipler - LhsMultipler;
}


static inline b32 
TryPack(atlas_builder* Builder, u32 Width, u32 Height) {
    rect2u* Spaces = nullptr;
    {
        rect2u Space{ 0, 0, Width, Height };
        DynBufferPush(Spaces, Space);
    }
    
    
    for (u32 i = 0; i < DynBufferCount(Builder->SortedRects); ++i ) {
        rect2u Rect = **(Builder->SortedRects + i);
        
        // NOTE(Momo): Iterate the empty spaces backwards to find the best fit index
        u32 ChosenSpaceIndex = DynBufferCount(Spaces);
        {
            for (u32 j = 0; j < ChosenSpaceIndex ; ++j ) {
                u32 Index = ChosenSpaceIndex - j - 1;
                rect2u Space = Spaces[Index];
                // NOTE(Momo): Check if the image fits
                if (GetWidth(Rect) <= GetWidth(Space) && GetHeight(Rect) <= GetHeight(Space)) {
                    ChosenSpaceIndex = Index;
                    break;
                    
                }
            }
        }
        
        
        // NOTE(Momo): If an empty space that can fit is found, we remove that space and split.
        if (ChosenSpaceIndex == DynBufferCount(Spaces)) {
            return false;
        }
        
        // NOTE(Momo): Swap and pop the chosen space
        rect2u ChosenSpace = Spaces[ChosenSpaceIndex];
        if (DynBufferCount(Spaces) > 0) {
            
            Spaces[ChosenSpaceIndex] = DynBufferLast(Spaces);
            DynBufferPop(Spaces);
        }
        
        // NOTE(Momo): Split if not perfect fit
        if (GetWidth(ChosenSpace) != GetWidth(Rect) && GetHeight(ChosenSpace) == GetHeight(Rect)) {
            // Split right
            rect2u SplitSpaceRight = {
                ChosenSpace.Min.X + GetWidth(Rect),
                ChosenSpace.Min.Y,
                ChosenSpace.Max.X, 
                ChosenSpace.Max.Y,
            };
            DynBufferPush(Spaces, SplitSpaceRight);
            
        }
        else if (GetWidth(ChosenSpace) == GetWidth(Rect) && GetHeight(ChosenSpace) != GetHeight(Rect)) {
            // Split down
            rect2u SplitSpaceDown = {
                ChosenSpace.Min.X,
                ChosenSpace.Min.Y + GetHeight(Rect),
                ChosenSpace.Max.X,
                ChosenSpace.Max.X
            };
            DynBufferPush(Spaces,SplitSpaceDown);
        }
        else if (GetWidth(ChosenSpace) != GetWidth(Rect) && GetHeight(ChosenSpace) != GetHeight(Rect)) {
            // Split right
            rect2u SplitSpaceRight = {
                ChosenSpace.Min.X + GetWidth(Rect),
                ChosenSpace.Min.Y,
                ChosenSpace.Max.X,
                ChosenSpace.Min.Y + GetHeight(Rect),
            };
            
            // Split down
            rect2u SplitSpaceDown = {
                ChosenSpace.Min.X,
                ChosenSpace.Min.Y + GetHeight(Rect),
                ChosenSpace.Max.X,
                ChosenSpace.Max.Y,
            };
            
            // Choose to insert the bigger one first before the smaller one
            u32 RightArea = GetWidth(SplitSpaceRight) * GetHeight(SplitSpaceRight);
            u32 DownArea = GetWidth(SplitSpaceDown) * GetHeight(SplitSpaceDown);
            if (RightArea > DownArea) {
                DynBufferPush(Spaces, SplitSpaceRight);
                DynBufferPush(Spaces, SplitSpaceDown);
            }
            else {
                DynBufferPush(Spaces, SplitSpaceDown);
                DynBufferPush(Spaces, SplitSpaceRight);
            }
            
        }
        
        // NOTE(Momo): Translate the rect
        *(Builder->SortedRects[i]) = Translate(Rect, ChosenSpace.Min);
    }
    
    return true;
}


static void
Sort(atlas_builder* Builder) {
    printf("\t[Sort] Begin\n");
    
    u32 RectCount = DynBufferCount(Builder->Rects);
    for (u32 i = 0; i < RectCount; ++i) {
        DynBufferPush(Builder->SortedRects, &Builder->Rects[i]);
    }
    
    // TODO(Momo): Is there really no way to delare something that accepts a lambda?
    qsort(Builder->SortedRects, RectCount, sizeof(rect2u*), AtlasBuilderComparer);
    
    printf("\t[Sort] End\n");
}

// NOTE(Momo): For now, we only support POT scaling.
static b32
Pack(atlas_builder* Builder, u32 StartSize, u32 SizeLimit) {
    Assert(StartSize > 0);
    Assert(SizeLimit > 0);
    
    u32 Width = StartSize;
    u32 Height = StartSize;
    
    printf("\t[Pack] Begin\n");
    for (;;) {
        printf("\t\tTrying to pack at: W = %d, H = %d\n", Width, Height);
        b32 Success = TryPack(Builder, Width, Height);
        if (!Success) {
            printf("\t\tFit not found.\n");
            if(Width >= SizeLimit || Height >= SizeLimit) {
                printf("\t\tExceeded size limit! Terminating...\n");
                return false;
            }
        }
        else {
            break;
        }
        
        // NOTE(Momo): POT scaling. 
        Width *= 2;
        Height *= 2;
    }
    printf("\t[Pack] End\n");
    
    Builder->Width = Width;
    Builder->Height = Height;
    Builder->Channels = 4; // TODO(Momo): Support for more channels?
    
    return true;
}

static inline b32
Build(atlas_builder* Builder, u32 StartSize, u32 SizeLimit) {
    Assert(DynBufferCount(Builder->Entries) > 0);
    
    printf("[Build] Atlas Building started \n");
    Defer { printf("[Build] Atlas Building complete\n"); };
    
    Sort(Builder);
    
    return Pack(Builder, StartSize, SizeLimit);
}

static inline void 
Init(atlas_builder* Builder, u32 Capacity) {
    // TODO(Momo): Remove?
}

static inline void 
Free(atlas_builder* Builder) {
    DynBufferFree(Builder->Rects);
    DynBufferFree(Builder->Entries);
    DynBufferFree(Builder->SortedRects);
}

// NOTE(Momo): Asset Builder //////////////////////////////////////////////////////////
enum asset_source_image_type {
    AssetSourceImageType_File,
    AssetSourceImageType_AtlasBuilder,
};

struct asset_source_image {
    asset_source_image_type Type;
    union {
        const char* Filename;
        atlas_builder * AtlasBuilder;
    };
};


struct asset_source_atlas {
    atlas_builder* Builder;
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
        asset_source_atlas Atlas;
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
    Entry->Image.Type = AssetSourceImageType_File;
}

static inline void 
SetAtlas(asset_builder* Assets, asset_id Id, atlas_builder* Builder) 
{
    asset_source* Entry = SetEntry(Assets, Id, AssetType_Atlas);
    Entry->Atlas.Builder = Builder;
}

static inline void 
SetAtlasRect(asset_builder* Assets, asset_id Id, rect2u Rect, asset_id AtlasAssetId) {
    asset_source* Entry = SetEntry(Assets, Id, AssetType_AtlasRect);
    Entry->AtlasRect.Rect = Rect;
    Entry->AtlasRect.AtlasAssetId = AtlasAssetId;
}

static inline void 
SetAtlasImage(asset_builder* Assets, asset_id Id, atlas_builder* Builder) {
    asset_source* Entry = SetEntry(Assets, Id, AssetType_Image);
    Entry->Image.AtlasBuilder = Builder;
    Entry->Image.Type = AssetSourceImageType_AtlasBuilder;
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
    switch (Image->Type) {
        case AssetSourceImageType_File: {
            i32 W, H, C;
            LoadedImage = stbi_load(Image->Filename, &W, &H, &C, 0);
            Assert(LoadedImage != nullptr);
            Width = (u32)W;
            Height = (u32)H; 
            Channels = (u32)C;
        } break;
        case AssetSourceImageType_AtlasBuilder: {
            LoadedImage = AllocateBitmap(Image->AtlasBuilder);
            Width = Image->AtlasBuilder->Width;
            Height = Image->AtlasBuilder->Height;
            Channels = Image->AtlasBuilder->Channels;
        } break;
        default: {
            Assert(false);
        }
    }
    
    Defer {
        switch (Image->Type) {
            case AssetSourceImageType_File: {
                stbi_image_free(LoadedImage);
            } break;
            case AssetSourceImageType_AtlasBuilder: {
                FreeBitmap(LoadedImage);
            } break;
        }
    };
    
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
WriteAtlasData(asset_builder* Assets, FILE* File, asset_source_atlas* Atlas) {
    atlas_builder* Builder = Atlas->Builder;
    usize AtlasStart = ftell(File);
    
    // NOTE(Momo): Write Header
    yuu_atlas Header = {}; 
    {
        Header.Width = Builder->Width;
        Header.Height = Builder->Height;
        Header.Channels = Builder->Channels;
        Header.EntryCount = DynBufferCount(Builder->Entries);
        Header.RectCount = DynBufferCount(Builder->Rects);
        
        fwrite(&Header, sizeof(Header), 1, File);
    }
    // NOTE(Momo): Write Rects
    {
        fwrite(Builder->Rects, sizeof(rect2u), Header.RectCount, File);
    }
    // NOTE(Momo): Write Bitmap
    {
        u8 * BitmapMemory = AllocateBitmap(Builder);
        Assert(BitmapMemory);
        Defer { FreeBitmap(BitmapMemory); };
        u32 BitmapSize = Header.Width * Header.Height * Header.Channels;
        fwrite(BitmapMemory, 1, BitmapSize, File);
#if DEBUG_PNG
        {
            // TODO(Momo): Use dynamic string
            //char Buffer[256] = {};
            const char * PngFilename = "debug.png";
            printf("\tWriting to '%s'... \n", PngFilename);
            if (!stbi_write_png(PngFilename, Builder->Width, Builder->Height, Builder->Channels, BitmapMemory, Builder->Width * Builder->Channels)) {
                printf("\tWrite failed\n");
                Assert(false);
            }
            printf("\tWriting to '%s' completed\n", PngFilename);
        }
#endif
    }
    // NOTE(Momo): Write Entries
    {
        for (u32 i = 0; i < DynBufferCount(Builder->Entries); ++i) {
            auto* Entry = Builder->Entries + i;
            
            // NOTE(Momo): Write file entry
            {
                yuu_atlas_entry FileEntry = {};
                FileEntry.Type = Entry->Type;
                
                fwrite(&FileEntry, sizeof(FileEntry), 1, File); 
            }
            
            // NOTE(Momo): Write Data
            switch(Entry->Type) {
                case AtlasEntryType_Image: {
                    yuu_atlas_image ImageData = {};
                    ImageData.RectIndex = Entry->Image.RectIndex;
                    
                    fwrite(&ImageData, sizeof(ImageData), 1, File); 
                } break;
                default: {
                    Assert(false);
                }
            }
            
        }
        
    }
    
    printf("Loaded Atlas: Rects = %d, Entries = %d, Width = %d, Height = %d\n",  Header.RectCount, Header.EntryCount, Header.Width, Header.Height);
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
        asset_source* Entry = &Assets->Entries[i];
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
            case AssetType_Atlas: {
                WriteAtlasData(Assets, OutFile, &Entry->Atlas);
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
