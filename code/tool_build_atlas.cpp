#include <stdio.h>
#include <stdlib.h>


#include "ryoji.h"
#include "ryoji_list.h"


#define STB_IMAGE_IMPLEMENTATION
#include "thirdparty/stb/stb_image.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "thirdparty/stb/stb_truetype.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "thirdparty/stb/stb_image_write.h"

#define DEBUG_PNG 0

// NOTE(Momo): File Structures  //////////////////////////////////////////////////////////////
#pragma pack(push, 1)

struct atlas_file_header {
    u32 EntryCount;
    u32 EntryDataOffset;
    
    u32 RectCount;
    u32 RectsDataOffset;
    
    u32 BitmapWidth;
    u32 BitmapHeight;
    u32 BitmapDataOffset;
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
    atlas_builder_entry_type Type;
    union {
        atlas_builder_entry_image Image;
        // TODO(Momo): Other types here
    };
};

struct atlas_builder_rect {
    i32 X, Y, W, H;
};

struct atlas_builder {
    atlas_builder_rect* Rects; // List
    atlas_builder_entry* Entries; //List
    
    const char* OutputFilename;
    b32 Started;
};


static inline u8* 
AllocateBitmap(atlas_builder* Builder, u32 Width, u32 Height, u32 Channels = 4) {
    u32 BitmapSize = Width * Height * Channels;
    u8* BitmapMemory = (u8*)calloc(BitmapSize, sizeof(u8));
    if (!BitmapMemory) {
        return nullptr;
    }
    
    // NOTE(Momo): Check. Total area of Rects must match 
    
    
    u32 EntryCount = ListCount(Builder->Entries);
    for (u32 i = 0; i <  EntryCount; ++i) {
        auto* Entry = Builder->Entries + i;
        
        auto* Image = &Entry->Image;
        auto* Rect = Builder->Rects + Image->RectIndex;
        
        i32 W,H,C;
        u8* LoadedImage = stbi_load(Image->Filename, &W, &H, &C, 0);
        if (LoadedImage == nullptr) {
            return nullptr;
        }
        Defer { stbi_image_free(LoadedImage); };
        
        Assert(W == Rect->W && H == Rect->H && C == (i32)Channels); 
        
        i32 j = 0;
        for (i32 y = Rect->Y; y < Rect->Y + Rect->H; ++y) {
            for (i32 x = Rect->X; x < Rect->X + Rect->W; ++x) {
                u32 Index = TwoToOne(y, x, Width) * Channels;
                Assert(Index < BitmapSize);
                for (u32 c = 0; c < Channels; ++c) {
                    BitmapMemory[Index + c] = LoadedImage[j++];
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
AddRect(atlas_builder* Builder, i32 W, i32 H) {
    atlas_builder_rect Rect = {0, 0, W, H};
    ListPush(Builder->Rects, Rect);
    
    
    return ListCount(Builder->Rects);
}

static inline atlas_builder_entry*
AddEntry(atlas_builder* Builder, atlas_builder_entry_type Type) {
    atlas_builder_entry Entry;
    Entry.Type = Type;
    
    ListPush(Builder->Entries, Entry);
    
    return &ListLast(Builder->Entries);
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
    const auto* L = (*(atlas_builder_rect**)(Lhs));
    const auto* R = (*(atlas_builder_rect**)(Rhs));
    
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
TryPack(atlas_builder* Builder, u32 Width, u32 Height) {
    atlas_builder_rect* Spaces = nullptr;
    {
        atlas_builder_rect Space;
        Space.X = 0;
        Space.Y = 0;
        Space.W = Width;
        Space.H = Height;
        ListPush(Spaces, Space);
    }
    
    
    {
        u32 RectCount = ListCount(Builder->Rects);
        for (u32 i = 0; i < RectCount; ++i ) {
            auto* Rect = Builder->Rects + i;
            
            // NOTE(Momo): Iterate the empty spaces backwards to find the best fit index
            u32 SpaceCount = ListCount(Spaces);
            u32 ChosenSpaceIndex = SpaceCount;
            {
                for (u32 j = 0; j < SpaceCount; ++j ) {
                    u32 Index = SpaceCount - j - 1;
                    auto* Space = Spaces + Index;
                    // NOTE(Momo): Check if the image fits
                    if (Rect->W <= Space->W && Rect->H <= Space->H) {
                        ChosenSpaceIndex = Index;
                        break;
                        
                    }
                }
            }
            
            
            // NOTE(Momo): If an empty space that can fit is found, we remove that space and split.
            if (ChosenSpaceIndex == SpaceCount) {
                return false;
            }
            
            // NOTE(Momo): Swap and pop the chosen space
            auto ChosenSpace = Spaces[ChosenSpaceIndex];
            if (SpaceCount > 0) {
                Spaces[ChosenSpaceIndex] = Spaces[SpaceCount - 1];
                --SpaceCount;
            }
            
            // NOTE(Momo): Split if not perfect fit
            if (ChosenSpace.W != Rect->W && ChosenSpace.H == Rect->H) {
                // Split right
                atlas_builder_rect SplitSpaceRight = {};
                SplitSpaceRight.X = ChosenSpace.X + Rect->W;
                SplitSpaceRight.Y = ChosenSpace.Y;
                SplitSpaceRight.W = ChosenSpace.W - Rect->W;
                SplitSpaceRight.H = Rect->H;
                Spaces[SpaceCount++] = SplitSpaceRight;
                
            }
            else if (ChosenSpace.W == Rect->W && ChosenSpace.H != Rect->H) {
                // Split down
                atlas_builder_rect SplitSpaceDown = {};
                SplitSpaceDown.X = ChosenSpace.X;
                SplitSpaceDown.Y = ChosenSpace.Y + Rect->H;
                SplitSpaceDown.W = ChosenSpace.W;
                SplitSpaceDown.H = ChosenSpace.H - Rect->H;
                Spaces[SpaceCount++] = SplitSpaceDown;
                
            }
            else if (ChosenSpace.W != Rect->W && ChosenSpace.H != Rect->H) {
                // Split right
                atlas_builder_rect SplitSpaceRight = {};
                SplitSpaceRight.X = ChosenSpace.X + Rect->W;
                SplitSpaceRight.Y = ChosenSpace.Y;
                SplitSpaceRight.W = ChosenSpace.W - Rect->W;
                SplitSpaceRight.H = Rect->H;
                
                // Split down
                atlas_builder_rect SplitSpaceDown = {};
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
    
    return true;
}


// NOTE(Momo): For now, we only support POT scaling.
static b32 
Build(atlas_builder* Builder, u32 StartSize, u32 SizeLimit) {
    Assert(StartSize > 0);
    Assert(SizeLimit > 0);
    
    printf("=== Building atlas '%s'\n", Builder->OutputFilename);
    
    // NOTE(Momo): Sort
    atlas_builder_rect** SortedRects = nullptr; // List
    Defer { ListFree(SortedRects); };
    printf("\t[Sort] Begin\n");
    {
        u32 RectCount = ListCount(Builder->Rects);
        for (u32 i = 0; i < RectCount; ++i) {
            ListPush(SortedRects, &Builder->Rects[i]);
        }
        qsort(SortedRects, RectCount, sizeof(atlas_builder_rect*), Comparer);
    }
    printf("\t[Sort] End\n");
    
    // NOTE(Momo): Pack
    u32 Width = StartSize;
    u32 Height = StartSize;
    printf("\t[Pack] Begin\n");
    {
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
    }
    printf("\t[Pack] End\n");
    
    
    // NOTE(Momo): Output
    const u32 Channels = 4;
    printf("\t[Output] Begin\n");
    {
        FILE * OutputFile = fopen(Builder->OutputFilename, "wb");
        if (!OutputFile) {
            printf("\t\tCannot open file '%s'\n", Builder->OutputFilename);
            return false;
        }
        
        u8 EntryDataAt = 0;
        
        // NOTE(Momo): Write Header
        {
            atlas_file_header Header = {}; 
            Header.EntryCount = ListCount(Builder->Entries);
            Header.EntryDataOffset = sizeof(Header); 
            
            Header.RectCount = ListCount(Builder->Rects);
            Header.RectDataOffset = Header.EntryDataOffset + sizeof(atlas_file_entry) * Header.EntryCount;
            
            Header.BitmapWidth = Width;
            Header.BitmapHeight = Height;
            
            fwrite(&Header, sizeof(Header), 1, OutputFile);
        }
        
        // NOTE(Momo): Write Entries
        {
            
        }
        
        // NOTE(Momo): Write Rects
        {
        }
        
        // NOTE(Momo): Write Bitmap
        
        
        
        
        
        
        
        
        
#if DEBUG_PNG
        printf("=== Writing to %s \n", Filename);
        stbi_write_png(Filename, Width, Height, Channels, Memory, Width * Channels);
        printf("\tWrite completed\n");
        
#endif
    }
    printf("\t[Output] End\n");
    
    return true;
}


static inline b32
WriteToAtlas(atlas_builder* Builder, const char* Filename) {
    printf("=== Writing to %s \n", Filename);
    return true;
}

static inline void
StartAtlas(atlas_builder* Builder, const char* OutputFilename) {
    Assert(!Builder->Started);
    atlas_builder NewBuilder = {};
    (*Builder) = NewBuilder;
    Builder->OutputFilename = OutputFilename;
    Builder->Started = true;
    
}

static inline void
EndAtlas(atlas_builder* Builder) {
    
    Build(Builder, 128, 4096);
    
    
    
    Assert(Builder->Started);
    ListFree(Builder->Rects);
    ListFree(Builder->Entries);
    Builder->Started = false;
}



int main() {
    atlas_builder Atlas_ = {};
    
    
    atlas_builder* Atlas = &Atlas_;
    StartAtlas(Atlas, "yuu");
    {
        AddImage(Atlas, "assets/ryoji.png");
        AddImage(Atlas, "assets/yuu.png");
        //AddFont(Atlas, "assets/font.odt", "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ", 36.f);
    }
    EndAtlas(Atlas);
    
    
    
    return 0;
    
}