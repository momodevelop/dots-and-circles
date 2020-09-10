#include <stdio.h>
#include <stdlib.h>
#include "ryoji.h"

// TODO(Momo): Don't use this somehow?
#define MAX_RECTS 1024
#define MAX_ENTRIES 1024 
#define ATLAS_WIDTH_LIMIT 4096
#define ATLAS_HEIGHT_LIMIT 4096


#define STB_IMAGE_IMPLEMENTATION
#include "thirdparty/stb/stb_image.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "thirdparty/stb/stb_truetype.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "thirdparty/stb/stb_image_write.h"


// NOTE(Momo): File Structures  //////////////////////////////////////////////////////////////
#pragma pack(push, 1)

struct atlas_file_header {
    u32 Width;
    u32 Height;
    u32 Channels;
    
    u32 EntryCount;
    u32 OffsetToEntries;
    
    u32 RectCount;
    u32 OffsetToRects;
};

struct atlas_file_entry {
    u32 Id;
    u32 Type;
};
#pragma pack(pop)


// NOTE(Momo): Atlas Builder //////////////////////////////////////////////////////////////
enum atlas_builder_entry_type {
    AtlasEntryType_Image,
};

struct atlas_builder_entry_image {
    const char* Filename;
    u32 RectIndex;
};

struct atlas_builder_entry {
    const char* Name;
    atlas_builder_entry_type Type;
    union {
        atlas_builder_entry_image Image;
    };
};

struct atlas_builder_rect {
    i32 X, Y, W, H;
};

struct atlas_builder {
    atlas_builder_rect Rects[MAX_RECTS];
    u32 RectCount;
    
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

static inline u32
AddRect(atlas_builder* Builder, i32 W, i32 H) {
    auto* Rect = Builder->Rects +  Builder->RectCount;
    (*Rect) = {0, 0, W, H};
    
    
    ++Builder->RectCount;
    Assert(Builder->RectCount < LARGE_NUMBER);
    
    return Builder->RectCount - 1;
}

static inline atlas_builder_entry*
AddEntry(atlas_builder* Builder, atlas_builder_entry_type Type) {
    auto* Entry = Builder->Entries +  Builder->EntryCount++;
    Assert(Builder->EntryCount < LARGE_NUMBER);
    Entry->Type = Type;
    return Entry;
}

static inline void 
AddImage(atlas_builder* Builder, const char* Filename) {
    auto* Entry = AddEntry(Builder,  AtlasEntryType_Image);
    Entry->Image.Filename = Filename;
    
    i32 W, H;
    stbi_info(Filename, &W, &H, 0);
    Entry->Image.RectIndex = AddRect(Builder, W, H);
}



// NOTE(Momo): Sort by area. Maybe sort by other methods?
static inline i32
Comparer(const void* Lhs, const void* Rhs) {
    const auto* L = (atlas_builder_rect**)Lhs;
    const auto* R = (atlas_builder_rect**)Rhs;
    
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


static inline b32 
TryPack(atlas_builder* Builder) {
    atlas_builder_rect Spaces[MAX_RECTS];
    u32 SpaceCount = 0;
    
    Spaces->X = 0;
    Spaces->Y = 0;
    Spaces->W = Builder->Width;
    Spaces->H = Builder->Height;
    ++SpaceCount;
    
    printf("=== Trying to pack at: W = %d, H = %d\n", Builder->Width, Builder->Height);
    {
        for (u32 i = 0; i < Builder->RectCount; ++i ) {
            auto* Rect = Builder->Rects + i;
            
            // NOTE(Momo): Iterate the empty spaces backwards to find the best fit index
            
            u32 ChosenSpaceIndex = SpaceCount;
            {
                for (u32 j = 0; j < SpaceCount; ++j ) {
                    u32 Index = SpaceCount - j - 1;
                    rect* Space = Spaces + Index;
                    // NOTE(Momo): Check if the image fits
                    if (Rect->W <= Space->W && Rect->H <= Space->H) {
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
            if (ChosenSpace.W != Rect->W && ChosenSpace.H == Rect->H) {
                // Split right
                rect SplitSpaceRight = {};
                SplitSpaceRight.X = ChosenSpace.X + Rect->W;
                SplitSpaceRight.Y = ChosenSpace.Y;
                SplitSpaceRight.W = ChosenSpace.W - Rect->W;
                SplitSpaceRight.H = Rect->H;
                Spaces[SpaceCount++] = SplitSpaceRight;
                
            }
            else if (ChosenSpace.W == Rect->W && ChosenSpace.H != Rect->H) {
                // Split down
                rect SplitSpaceDown = {};
                SplitSpaceDown.X = ChosenSpace.X;
                SplitSpaceDown.Y = ChosenSpace.Y + Rect->H;
                SplitSpaceDown.W = ChosenSpace.W;
                SplitSpaceDown.H = ChosenSpace.H - Rect->H;
                Spaces[SpaceCount++] = SplitSpaceDown;
                
            }
            else if (ChosenSpace.W != Rect->W && ChosenSpace.H != Rect->H) {
                // Split right
                rect SplitSpaceRight = {};
                SplitSpaceRight.X = ChosenSpace.X + Rect->W;
                SplitSpaceRight.Y = ChosenSpace.Y;
                SplitSpaceRight.W = ChosenSpace.W - Rect->W;
                SplitSpaceRight.H = Rect->H;
                
                // Split down
                rect SplitSpaceDown = {};
                SplitSpaceDown.X = ChosenSpace.X;
                SplitSpaceDown.Y = ChosenSpace.Y + Rect->H;
                SplitSpaceDown.W = ChosenSpace.W;
                SplitSpaceDown.H = ChosenSpace.H - Rect->H;
                
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
            Rect->X = ChosenSpace.X;
            Rect->Y = ChosenSpace.Y;
        }
    }
    
    // Debug purposes
#if 0
    printf("--- Results:\n");
    for ( u32 i = 0; i < Builder->EntryCount; ++i ) {
        auto* Entry = Builder->Entries + i;
        printf("\tPacked %s at: X = %d, Y = %d, W = %d, H = %d\n", Rect->Filename, Rect->X, Rect->Y, Rect->W, Rect->H);
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
    
    // NOTE(Momo): Sort
    atlas_builder_rect* SortedRects[MAX_RECTS];
    {
        for (u32 i = 0; i < Builder->RectCount; ++i) 
            SortedRects[i] = &Builder->Rects[i];
        qsort(SortedRects, Builder->RectCount, sizeof(atlas_builder_rect*), Comparer);
    }
    
    Builder->Width = StartAtlasWidth;
    Builder->Height = StartAtlasHeight;
    Builder->Channels = Channels;
    
    // NOTE(Momo): POT scaling.
    for (;;) {
        b32 Success = TryPack(Builder);
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
AllocateTexture(atlas_builder* Builder) {
    u32 MemorySize = Builder->Width * Builder->Height * Builder->Channels;
    u8* Memory = (u8*)calloc(MemorySize, sizeof(u8));
    if (!Memory) {
        return nullptr;
    }
    
    for (u32 i = 0; i < Builder->EntryCount; ++i) {
        auto* Entry = Builder->Entries + i;
        
        switch(Entry->Type) {
            auto* Image = &Entry->Image;
            auto* Rect = Builder->Rects + Image->RectIndex;
            
            i32 W,H,C;
            u8* LoadedImage = stbi_load(Image->Filename, &W, &H, &C, 0);
            if (LoadedImage == nullptr) {
                return nullptr;
            }
            Defer { stbi_image_free(LoadedImage); };
            
            // TODO(Momo): Limitation: Color Channels of all images must be same as builder channels
            Assert(W == Rect->W && H == Rect->H && C == (i32)Builder->Channels); 
            
            i32 j = 0;
            for (i32 y = Rect->Y; y < Rect->Y + Rect->H; ++y) {
                for (i32 x = Rect->X; x < Rect->X + Rect->W; ++x) {
                    i32 Index = TwoToOne(y, x, Builder->Width) * Builder->Channels;
                    for (u32 c = 0; c < Builder->Channels; ++c) {
                        Memory[Index + c] = LoadedImage[j++];
                    }
                }
            } break;
            
        }
        
    }
    
    return Memory;
}

static inline void
FreeTexture(u8* Memory) {
    free(Memory);
}

// NOTE(Momo): Debug no tame
static inline b32
WriteToPng(atlas_builder* Builder, const char* Filename) 
{
    printf("=== Writing to %s \n", Filename);
    u8* Memory = AllocateTexture(Builder);
    if (!Memory) {
        return false;
    }
    Defer { FreeTexture(Memory); };
    
    stbi_write_png(Filename, Builder->Width, Builder->Height, Builder->Channels, Memory, Builder->Width * Builder->Channels);
    printf("\tWrite completed\n");
    return true;
}

static inline b32
WriteToAtlas(atlas_builder* Builder, const char* Filename) {
    printf("=== Writing to %s \n", Filename);
    return true;
}


int main() {
    atlas_builder Atlas_ = {};
    atlas_builder* Atlas = &Atlas_;
    {
        AddImage(Atlas, "assets/ryoji.png");
        AddImage(Atlas, "assets/yuu.png");
        //AddFont(Atlas, "assets/font.odt", "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ", 36.f);
        
        
    }
    
    if (!Pack(Atlas, 128, 128, 4)) {
        printf("Pack failed\n");
        return 1;
    }
    printf("Final Atlas Dimension: W = %d, H = %d\n", Atlas->Width, Atlas->Height);
    WriteToPng(Atlas, "assets/atlas.png");
    WriteToAtlas(Atlas, "assets/ryoji.atlas");
    
    
    return 0;
    
}