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


struct atlas_builder {
    rect2u* Rects; // List
    rect2u** SortedRects; // List
    atlas_builder_entry* Entries; // List
    
    u32 Width;
    u32 Height;
    u32 Channels;
};



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
Free(atlas_builder* Builder) {
    DynBufferFree(Builder->Rects);
    DynBufferFree(Builder->Entries);
    DynBufferFree(Builder->SortedRects);
}

static inline void 
WriteToFile(atlas_builder* Builder) {
    
}


int main() {
    atlas_builder Atlas_ = {};
    atlas_builder* Atlas = &Atlas_;
    Init(Atlas, AtlasDefault_Count);
    Defer { Free(Atlas); };
    {
        AddImage(Atlas, "assets/ryoji.png", Asset_RectRyoji);
        AddImage(Atlas, "assets/yuu.png", Asset_RectYuu);
        
        AddImage(Atlas, "assets/karu00.png", Asset_RectKaru00);
        AddImage(Atlas, "assets/karu01.png", Asset_RectKaru01);
        AddImage(Atlas, "assets/karu02.png", Asset_RectKaru02);
        AddImage(Atlas, "assets/karu10.png", Asset_RectKaru10);
        AddImage(Atlas, "assets/karu11.png", Asset_RectKaru11);
        AddImage(Atlas, "assets/karu12.png", Asset_RectKaru12);
        AddImage(Atlas, "assets/karu20.png", Asset_RectKaru20);
        AddImage(Atlas, "assets/karu21.png", Asset_RectKaru21);
        AddImage(Atlas, "assets/karu22.png", Asset_RectKaru22);
        AddImage(Atlas, "assets/karu30.png", Asset_RectKaru30);
        AddImage(Atlas, "assets/karu31.png", Asset_RectKaru31);
        AddImage(Atlas, "assets/karu32.png", Asset_RectKaru32);
        
    }
    if (!Build(Atlas, 128, 4096)) {
        printf("Failed to build\n");
        return 1;
    }
    
    
    stbi_write_png("atlas.png",);
}