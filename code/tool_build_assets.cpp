#include <stdio.h>
#include <stdlib.h>
#include "ryoji.h"

// TODO(Momo): Don't use this somehow?
#define MAX_ENTRIES 1024 
#define ATLAS_WIDTH_LIMIT 4096
#define ATLAS_HEIGHT_LIMIT 4096



#define STB_IMAGE_IMPLEMENTATION
#include "thirdparty/stb/stb_image.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "thirdparty/stb/stb_truetype.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "thirdparty/stb/stb_image_write.h"


#include "game_asset_types.h"


// NOTE(Momo): File Structures  //////////////////////////////////////////////////////////////
#pragma pack(push, 1)

struct asset_file_entry {
    asset_type Type;
    u32 OffsetToData;
    asset_id Id;
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

struct asset_file_data_atlas {
    
    
};
#pragma pack(pop)



// NOTE(Momo): Atlas Builder //////////////////////////////////////////////////////////////
struct atlas_builder_entry {
    const char* Filename;
    atlas_image_id Id;
    i32 X, Y, W, H;
};

struct atlas_builder {
    atlas_builder_entry Entries[MAX_ENTRIES];
    u32 EntryCount;
    u32 Width;
    u32 Height;
    u32 Channels;
};

static inline void
Init(atlas_builder* Builder, u32 StartWidth, u32 StartHeight, u32 Channels) {
    Assert(StartWidth > 0);
    Assert(StartHeight > 0);
    Assert(Channels > 0);
    
    Builder->Width = StartWidth;
    Builder->Height = StartHeight;
    Builder->Channels = Channels;
}

static inline void 
AddImage(atlas_builder* Builder, const char* Filename) {
    auto* Entry = Builder->Entries +  Builder->EntryCount;
    Entry->Filename = Filename;
    stbi_info(Filename, &Entry->W, &Entry->H, 0);
    ++Builder->EntryCount;
}

// NOTE(Momo): Sort by area. Maybe sort by other methods?
static inline i32
Comparer(const void* Lhs, const void* Rhs) {
    const auto* L = (atlas_builder_entry*)Lhs;
    const auto* R = (atlas_builder_entry*)Rhs;
    
    auto LhsArea = L->W * L->H;
    auto RhsArea = R->W * R->H;
    if (LhsArea != RhsArea)
        return  RhsArea - LhsArea;
    
    auto LhsPerimeter = L->W + L->H;
    auto RhsPerimeter = R->W + R->H;
    if (LhsPerimeter != RhsPerimeter)
        return RhsPerimeter - LhsPerimeter;
    
    // By bigger side
    auto LhsBiggerSide = Maximum(L->W, L->H);
    auto RhsBiggerSide = Maximum(R->W, R->H);
    if (LhsBiggerSide != RhsBiggerSide) 
        return RhsBiggerSide - LhsBiggerSide;
    
    // By Width
    if (L->W != R->W)
        return R->W - L->W;
    
    // By right
    if (L->H != R->H)
        return R->H - L->H;
    
    // pathological multipler
    auto LhsMultipler = Maximum(L->W, L->H)/Minimum(L->W, L->H) * L->W * L->H;
    auto RhsMultipler = Maximum(R->W, R->H)/Minimum(R->W, R->H) * R->W * R->H;
    return RhsMultipler - LhsMultipler;
}

static inline void 
Sort(atlas_builder* Builder) {
    qsort(Builder->Entries, Builder->EntryCount, sizeof(atlas_builder_entry), Comparer);
}

static inline b32 
PackSub(atlas_builder* Builder) {
    struct rect {
        i32 X, Y, W, H;
    } Spaces[MAX_ENTRIES];
    u32 SpaceCount = 0;
    
    Spaces->X = 0;
    Spaces->Y = 0;
    Spaces->W = Builder->Width;
    Spaces->H = Builder->Height;
    ++SpaceCount;
    
    printf("=== Trying to pack at: W = %d, H = %d\n", Builder->Width, Builder->Height);
    {
        for (u32 i = 0; i < Builder->EntryCount; ++i ) {
            auto* Entry = Builder->Entries + i;
            printf("\tPacking %s...\n", Entry->Filename);
            
            // NOTE(Momo): Iterate the empty spaces backwards to find the best fit index
            
            u32 ChosenSpaceIndex = SpaceCount;
            {
                for (u32 j = 0; j < SpaceCount; ++j ) {
                    u32 Index = SpaceCount - j - 1;
                    rect* Space = Spaces + Index;
                    // NOTE(Momo): Check if the image fits
                    if (Entry->W <= Space->W && Entry->H <= Space->H) {
                        ChosenSpaceIndex = Index;
                        
                        break;
                        
                    }
                }
            }
            
            
            // NOTE(Momo): If an empty space that can fit is found, we remove that space and split.
            if (ChosenSpaceIndex == SpaceCount) {
                printf("\t[FAILED] Fit not found, terminating\n");
                return false;
            }
            
            // NOTE(Momo): Swap and pop the chosen space
            rect ChosenSpace = Spaces[ChosenSpaceIndex];
            //printf("Chosen space %d: %d %d %d %d\n",ChosenSpaceIndex, ChosenSpace.X, ChosenSpace.Y, ChosenSpace.W, ChosenSpace.H );
            
            
            
            if (SpaceCount > 0) {
                Spaces[ChosenSpaceIndex] = Spaces[SpaceCount - 1];
                --SpaceCount;
            }
            
            // NOTE(Momo): Split if not perfect fit
            if (ChosenSpace.W != Entry->W && ChosenSpace.H == Entry->H) {
                // Split right
                rect SplitSpaceRight = {};
                SplitSpaceRight.X = ChosenSpace.X + Entry->W;
                SplitSpaceRight.Y = ChosenSpace.Y;
                SplitSpaceRight.W = ChosenSpace.W - Entry->W;
                SplitSpaceRight.H = Entry->H;
                Spaces[SpaceCount++] = SplitSpaceRight;
                
            }
            else if (ChosenSpace.W == Entry->W && ChosenSpace.H != Entry->H) {
                // Split down
                rect SplitSpaceDown = {};
                SplitSpaceDown.X = ChosenSpace.X;
                SplitSpaceDown.Y = ChosenSpace.Y + Entry->H;
                SplitSpaceDown.W = ChosenSpace.W;
                SplitSpaceDown.H = ChosenSpace.H - Entry->H;
                Spaces[SpaceCount++] = SplitSpaceDown;
                
            }
            else if (ChosenSpace.W != Entry->W && ChosenSpace.H != Entry->H) {
                // Split right
                rect SplitSpaceRight = {};
                SplitSpaceRight.X = ChosenSpace.X + Entry->W;
                SplitSpaceRight.Y = ChosenSpace.Y;
                SplitSpaceRight.W = ChosenSpace.W - Entry->W;
                SplitSpaceRight.H = Entry->H;
                
                // Split down
                rect SplitSpaceDown = {};
                SplitSpaceDown.X = ChosenSpace.X;
                SplitSpaceDown.Y = ChosenSpace.Y + Entry->H;
                SplitSpaceDown.W = ChosenSpace.W;
                SplitSpaceDown.H = ChosenSpace.H - Entry->H;
                
                // Choose to insert the bigger one first before the smaller one
                u32 RightArea = SplitSpaceRight.W * SplitSpaceRight.H;
                u32 DownArea = SplitSpaceDown.W * SplitSpaceDown.H;
                if (RightArea > DownArea) {
                    Spaces[SpaceCount++] = SplitSpaceRight;
                    Spaces[SpaceCount++] = SplitSpaceDown;
                }
                else {
                    Spaces[SpaceCount++] = SplitSpaceDown;
                    Spaces[SpaceCount++] = SplitSpaceRight;
                }
                
            }
            
            // NOTE(Momo): Add to packed images
            Entry->X = ChosenSpace.X;
            Entry->Y = ChosenSpace.Y;
        }
    }
    
    // Debug purposes
#if 0
    printf("--- Results:\n");
    for ( u32 i = 0; i < Builder->EntryCount; ++i ) {
        auto* Entry = Builder->Entries + i;
        printf("\tPacked %s at: X = %d, Y = %d, W = %d, H = %d\n", Entry->Filename, Entry->X, Entry->Y, Entry->W, Entry->H);
    }
    
    printf("--- Space Left:\n");
    for ( u32 i = 0; i < SpaceCount; ++i ) {
        auto* Space = Spaces + i;
        printf("\tSpace %d: X = %d, Y = %d, W = %d, H = %d\n", i,  Space->X, Space->Y, Space->W, Space->H);
    }
#endif
    printf("=== Pack completed!\n");
    return true;
}

static inline b32 
Pack(atlas_builder* Builder, u32 StartAtlasWidth, u32 StartAtlasHeight, u32 Channels) {
    Assert(StartAtlasWidth > 0);
    Assert(StartAtlasHeight > 0);
    Assert(Channels > 0);
    
    Builder->Width = StartAtlasWidth;
    Builder->Height = StartAtlasHeight;
    Builder->Channels = Channels;
    
    // NOTE(Momo): POT scaling.
    for (;;) {
        b32 Success = PackSub(Builder);
        if (!Success) {
            if(Builder->Width >= ATLAS_WIDTH_LIMIT || Builder->Height >= ATLAS_HEIGHT_LIMIT) {
                return false;
            }
            
            Builder->Width *= 2;
            Builder->Height *= 2;
        }
        else {
            break;
        }
    }
    
    return true;
}

static inline u8* 
AllocateAtlasData(atlas_builder* Builder) {
    u32 MemorySize = Builder->Width * Builder->Height * Builder->Channels;
    u8* Memory = (u8*)calloc(MemorySize, sizeof(u8));
    if (!Memory) {
        return nullptr;
    }
    
    for (u32 i = 0; i < Builder->EntryCount; ++i) {
        auto* Entry = Builder->Entries + i;
        //printf("\tLoading %s\n", Entry->Filename);
        
        i32 W,H,C;
        u8* LoadedImage = stbi_load(Entry->Filename, &W, &H, &C, 0);
        if (LoadedImage == nullptr) {
            return nullptr;
        }
        Defer { stbi_image_free(LoadedImage); };
        
        // TODO(Momo): Limitation: Color Channels of all images must be same as builder channels
        Assert(W == Entry->W && H == Entry->H && C == (i32)Builder->Channels); 
        
        i32 j = 0;
        for (i32 y = Entry->Y; y < Entry->Y + Entry->H; ++y) {
            for (i32 x = Entry->X; x < Entry->X + Entry->W; ++x) {
                i32 Index = TwoToOne(y, x, Builder->Width) * Builder->Channels;
                for (u32 c = 0; c < Builder->Channels; ++c) {
                    Memory[Index + c] = LoadedImage[j++];
                }
            }
        }
        
    }
    
    return Memory;
}

static inline void
FreeAtlasData(u8* Memory) {
    free(Memory);
}

// NOTE(Momo): Debug no tame
static inline b32
OutputToPng(atlas_builder* Builder, const char* Filename) 
{
    printf("=== Writing to %s \n", Filename);
    u8* Memory = AllocateAtlasData(Builder);
    if (!Memory) {
        return false;
    }
    Defer { FreeAtlasData(Memory); };
    
    stbi_write_png(Filename, Builder->Width, Builder->Height, Builder->Channels, Memory, Builder->Width * Builder->Channels);
    printf("=== Write to %s completed\n", Filename);
    return true;
}


// NOTE(Momo): Asset Builder //////////////////////////////////////////////////////////
struct asset_builder_entry_image {
    const char* Filename;
};

struct asset_builder_entry_sound {
    // TODO(Momo): 
};

struct asset_builder_entry_atlas {
    atlas_builder* AtlasBuilder;
};



struct asset_builder_entry_font {
    const char* Filename;
    const char* Characters;
    u32 CharacterCount;
    f32 Size;
};

struct asset_builder_entry_spritesheet {
    const char* Filename;
    u32 Rows;
    u32 Cols;
};

struct asset_builder_entry  {
    asset_type Type;
    asset_id Id;
    
    union {
        asset_builder_entry_image Image;
        asset_builder_entry_sound Sound;
        asset_builder_entry_spritesheet Spritesheet;
        asset_builder_entry_atlas Atlas;
    };
    
};


struct asset_builder {
    asset_builder_entry Entries[MAX_ENTRIES];
    u32 EntryCount;
};


static inline asset_builder_entry*
AddAssetEntry(asset_builder* Assets, asset_id Id, asset_type Type) {
    Assert(Assets->EntryCount != MAX_ENTRIES);
    asset_builder_entry* Ret = &Assets->Entries[Assets->EntryCount++];
    Ret->Id = Id;
    Ret->Type = Type;
    return Ret;
}


static inline void
AddImage(asset_builder* Assets, const char* Filename, asset_id Id) {
    asset_builder_entry* Entry = AddAssetEntry(Assets, Id, AssetType_Image);
    Entry->Image.Filename = Filename;
}

static inline void 
AddSpritesheet(asset_builder* Assets, 
               const char* Filename, 
               asset_id Id,  
               u32 Rows, 
               u32 Columns) 
{
    asset_builder_entry* Entry = AddAssetEntry(Assets, Id, AssetType_Spritesheet);
    Entry->Spritesheet.Filename = Filename;
    Entry->Spritesheet.Rows = Rows;
    Entry->Spritesheet.Cols = Columns;
}


static inline void 
AddAtlas(asset_builder* Assets, atlas_builder* Atlas, asset_id Id) 
{
    asset_builder_entry* Entry = AddAssetEntry(Assets, Id, AssetType_Atlas);
    // TODO(Momo): Maybe copy over required things?
    
    Entry->Atlas.AtlasBuilder = Atlas;
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
    fwrite(&Assets->EntryCount, sizeof(Assets->EntryCount), 1, pFile);
    HeaderAt += sizeof(Assets->EntryCount);
    
    
    usize DataAt = HeaderAt + (sizeof(asset_file_entry) * Assets->EntryCount);
    
    
    // NOTE(Momo): Write the data
    for (u32 i = 0; i < Assets->EntryCount; ++i)
    {
        asset_builder_entry* Entry = &Assets->Entries[i];
        
        switch(Entry->Type) {
            case AssetType_Image: {
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
                    Header.Type = Entry->Type;
                    Header.OffsetToData = (u32)DataAt;
                    Header.Id = Entry->Id;
                    
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
            case AssetType_Spritesheet: {
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
                    Header.Type = AssetType_Spritesheet;
                    Header.OffsetToData = (u32)DataAt;
                    Header.Id = Entry->Id;
                    
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
    
    atlas_builder Atlas_ = {};
    atlas_builder* Atlas = &Atlas_;
    {
        AddImage(Atlas, "assets/ryoji.png");
        AddImage(Atlas, "assets/yuu.png");
        
        Sort(Atlas);
        
        if (!Pack(Atlas, 128, 128, 4)) {
            printf("Pack failed\n");
            return 1;
        }
        printf("Final Atlas Dimension: W = %d, H = %d\n", Atlas->Width, Atlas->Height);
        
#if 1
        OutputToPng(Atlas, "test.png");
#endif
    }
    
    
    // NOTE(Momo): Gather all the assets we need to load
    asset_builder Assets_ = {};
    asset_builder* Assets = &Assets_;
    {
        AddImage(Assets, "assets/ryoji.png", Asset_ImageRyoji);
        AddImage(Assets, "assets/yuu.png", Asset_ImageYuu);
        
        AddSpritesheet(Assets, "assets/karu.png", Asset_SpritesheetKaru, 4, 3);
        
        
        
    }
    
    WriteToFile(Assets, "yuu");
#endif
    
    return 0;
    
}

