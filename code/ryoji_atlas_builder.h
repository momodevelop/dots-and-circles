#ifndef RYOJI_ATLAS_BUILDER_H
#define RYOJI_ATLAS_BUILDER_H

#include <stdlib.h>

#include "ryoji.h"
#include "ryoji_maths.h"
#include "ryoji_bitmanip.h"

typedef void (*atlas_builder_get_bitmap_info_cb)(void* UserContext, u32* BitmapWidth, u32* BitmapHeight, u32* BitmapChannels);
typedef void (*atlas_builder_load_bitmap_cb)(void* UserContext, u8** BitmapMemory, u32* BitmapWidth, u32* BitmapHeight, u32* BitmapChannels);
typedef void (*atlas_builder_unload_bitmap_cb)(void* UserContext);

struct atlas_builder_entry {
    void* UserContext;
    atlas_builder_get_bitmap_info_cb  GetInfoCb;
    atlas_builder_load_bitmap_cb LoadCb;
    atlas_builder_unload_bitmap_cb UnloadCb;
    usize RectIndex;
};

template<usize N>
struct atlas_builder {
    rect2u Rects[N];
    u32 RectCount;
    
    rect2u* SortedRects[N];
    u32 SortedRectCount;
    
    atlas_builder_entry Entries[N];
    u32 EntryCount;
    
    u32 Width;
    u32 Height;
    u32 Channels;
    
    b32 IsBegin;
};


template<usize N> static inline void
AddEntry(atlas_builder<N>* Builder, 
         atlas_builder_get_bitmap_info_cb GetInfoCb, 
         atlas_builder_load_bitmap_cb LoadCb,
         atlas_builder_unload_bitmap_cb UnloadCb,
         void* UserContext) 
{
    Assert(GetInfoCb != nullptr);
    Assert(LoadCb != nullptr);
    Assert(UnloadCb != nullptr);
    
    atlas_builder_entry Entry = {};
    Entry.LoadCb = LoadCb;
    Entry.UnloadCb = UnloadCb;
    Entry.GetInfoCb = GetInfoCb;
    Entry.UserContext = UserContext;
    
    u32 W, H, C;
    Entry.GetInfoCb(UserContext, &W, &H, &C);
    Entry.RectIndex = AddRect(Builder, W, H);
    Builder->Entries[Builder->EntryCount++] = Entry;
}

template<usize N> static inline usize
AddRect(atlas_builder<N>* Builder, u32 W, u32 H) {
    Builder->Rects[Builder->RectCount++] = {0, 0, W, H};
    return Builder->RectCount - 1;
}

template<usize N> static inline usize
GetAtlasBitmapSize(atlas_builder<N>* Builder) {
    Assert(!Builder->IsBegin);
    return Builder->Width * Builder->Height * Builder->Channels;
}

template<usize N> static inline void
GetAtlasBitmap(atlas_builder<N>* Builder, void* Memory, u32* Width, u32* Height, u32* Channels) {
    u32 BitmapSize = Builder->Width * Builder->Height * Builder->Channels;
    u8* BitmapMemory = (u8*)Memory;
    (*Width) = Builder->Width;
    (*Height) = Builder->Height;
    (*Channels) = Builder->Channels;
    
    
    for (u32 i = 0; i < Builder->EntryCount; ++i) {
        auto* Entry = Builder->Entries + i;
        rect2u Rect = *(Builder->Rects + Entry->RectIndex);
        
        // Load bitmap here
        u32 EntryBitmapWidth, EntryBitmapHeight, EntryBitmapChannels;
        u8* EntryBitmapMemory;
        
        Entry->LoadCb(Entry->UserContext, &EntryBitmapMemory, &EntryBitmapWidth, &EntryBitmapHeight, &EntryBitmapChannels);
        
        Defer { Entry->UnloadCb(Entry->UserContext); };
        
        Assert(EntryBitmapWidth > 0);
        Assert(EntryBitmapHeight > 0);
        Assert(EntryBitmapChannels > 0);
        Assert(EntryBitmapWidth == GetWidth(Rect));
        Assert(EntryBitmapHeight == GetHeight(Rect));
        Assert(EntryBitmapChannels == Builder->Channels); 
        
        i32 j = 0;
        for (u32 y = Rect.Min.Y; y < Rect.Min.Y + GetHeight(Rect); ++y) {
            for (u32 x = Rect.Min.X; x < Rect.Min.X + GetWidth(Rect); ++x) {
                u32 Index = TwoToOne(y, x, Builder->Width) * Builder->Channels;
                Assert(Index < BitmapSize);
                for (u32 c = 0; c < Builder->Channels; ++c) {
                    BitmapMemory[Index + c] = EntryBitmapMemory[j++];
                }
            }
        }
        
    }
    
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


template<usize N> static inline b32 
TryPack(atlas_builder<N>* Builder, u32 Width, u32 Height) {
    rect2u Spaces[N+1];
    u32 SpaceCount = 0;
    Spaces[SpaceCount++] = { 0, 0, Width, Height };
    
    for (u32 i = 0; i < Builder->SortedRectCount; ++i ) {
        rect2u Rect = **(Builder->SortedRects + i);
        
        // NOTE(Momo): Iterate the empty spaces backwards to find the best fit index
        u32 ChosenSpaceIndex = SpaceCount;
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
        if (ChosenSpaceIndex == SpaceCount) {
            return false;
        }
        
        // NOTE(Momo): Swap and pop the chosen space
        rect2u ChosenSpace = Spaces[ChosenSpaceIndex];
        if (SpaceCount > 0) {
            Spaces[ChosenSpaceIndex] = Spaces[SpaceCount-1];
            --SpaceCount;
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
            Spaces[SpaceCount++] = SplitSpaceRight;
        }
        else if (GetWidth(ChosenSpace) == GetWidth(Rect) && GetHeight(ChosenSpace) != GetHeight(Rect)) {
            // Split down
            rect2u SplitSpaceDown = {
                ChosenSpace.Min.X,
                ChosenSpace.Min.Y + GetHeight(Rect),
                ChosenSpace.Max.X,
                ChosenSpace.Max.X
            };
            Spaces[SpaceCount++] = SplitSpaceDown;
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
                Spaces[SpaceCount++] = SplitSpaceRight;
                Spaces[SpaceCount++] = SplitSpaceDown;
            }
            else {
                Spaces[SpaceCount++] = SplitSpaceDown;
                Spaces[SpaceCount++] = SplitSpaceRight;
            }
            
        }
        
        // NOTE(Momo): Translate the rect
        *(Builder->SortedRects[i]) = Translate(Rect, ChosenSpace.Min);
    }
    
    return true;
}


template<usize N> static void
Sort(atlas_builder<N>* Builder) {
    for (u32 i = 0; i < Builder->RectCount; ++i) {
        Builder->SortedRects[Builder->SortedRectCount++] =  &Builder->Rects[i];
    }
    
    // TODO(Momo): Is there really no way to delare something that accepts a lambda?
    qsort(Builder->SortedRects, Builder->RectCount, sizeof(rect2u*), AtlasBuilderComparer);
}

// NOTE(Momo): For now, we only support POT scaling.
template<usize N> static inline void 
Pack(atlas_builder<N>* Builder, u32 StartSize, u32 SizeLimit) {
    Assert(StartSize > 0);
    Assert(SizeLimit > 0);
    
    u32 Width = StartSize;
    u32 Height = StartSize;
    
    for (;;) {
        b32 Success = TryPack(Builder, Width, Height);
        if (!Success) {
            if(Width >= SizeLimit || Height >= SizeLimit) {
                Assert(false);
            }
        }
        else {
            break;
        }
        
        // NOTE(Momo): POT scaling. 
        Width *= 2;
        Height *= 2;
    }
    
    Builder->Width = Width;
    Builder->Height = Height;
    Builder->Channels = 4; // TODO(Momo): Support for more channels?
}



template<usize N> static inline void 
Begin(atlas_builder<N>* Builder) {
    Assert(!Builder->IsBegin);
    Builder->EntryCount = 0;
    Builder->RectCount = 0;
    Builder->SortedRectCount = 0;
    Builder->Width = 0;
    Builder->Height = 0;
    Builder->Channels = 0;
    
    Builder->IsBegin = true;
}

template<usize N> static inline void 
End(atlas_builder<N>* Builder, u32 StartSize = 128, u32 SizeLimit = 4096) {
    Assert(Builder->IsBegin);
    Assert(Builder->EntryCount > 0);
    Sort(Builder);
    Pack(Builder, StartSize, SizeLimit);
    Builder->IsBegin = false;
}




#endif //RYOJI_ATLAS_BUILDER_H
