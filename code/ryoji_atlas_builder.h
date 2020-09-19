#ifndef RYOJI_ATLAS_BUILDER_H
#define RYOJI_ATLAS_BUILDER_H

#include <stdlib.h>
#include "ryoji.h"
#include "ryoji_maths.h"
#include "ryoji_bitmanip.h"
#include "ryoji_dynamic_buffer.h"

struct atlas_builder_entry {
    u32 BitmapWidth, BitmapHeight, BitmapChannels;
    u32 StartSize, SizeLimit;
    u8* BitmapPixels;
    
    void* UserData;
    u32 UserDataSize;
    
    usize RectIndex;
};

struct atlas_builder {
    rect2u* Rects; // List
    rect2u** SortedRects; // List
    atlas_builder_entry* Entries; // List
    
    u32 Width;
    u32 Height;
    u32 Channels;
};

static inline u32
AddRect(atlas_builder* Builder, u32 W, u32 H) {
    rect2u Rect = {0, 0, W, H};
    DynBufferPush(Builder->Rects, Rect);
    return DynBufferCount(Builder->Rects) - 1;
}

static inline void
AddEntry(atlas_builder* Builder, u32 Width, u32 Height, u32 Channels, u8* Pixels, void* UserData, u32 UserDataSize) 
{
    Assert(Width > 0);
    Assert(Height > 0);
    Assert(Channels > 0);
    Assert(Pixels != nullptr);
    
    atlas_builder_entry Entry = {};
    
    Entry.BitmapWidth = Width;
    Entry.BitmapHeight = Height;
    Entry.BitmapChannels = Channels;
    u32 BitmapSize = Width * Height * Channels;
    Entry.BitmapPixels = (u8*)malloc(BitmapSize);
    Assert(Entry.BitmapPixels);
    CopyBlock(Entry.BitmapPixels, Pixels, BitmapSize);
    
    if (UserData != nullptr) {
        Assert(UserDataSize > 0);
        Entry.UserData = malloc(UserDataSize);
        Assert(Entry.UserData);
        Entry.UserDataSize = UserDataSize;
        CopyBlock(Entry.UserData, UserData, UserDataSize);
    }
    
    Entry.RectIndex = AddRect(Builder, Width, Height);
    
    DynBufferPush(Builder->Entries, Entry);
}

template<typename user_data_t>
static inline void 
AddEntry(atlas_builder* Builder, u32 Width, u32 Height, u32 Channels, u8* Pixels, user_data_t* UserData) {
    AddEntry(Builder, Width, Height, Channels, Pixels, UserData, sizeof(user_data_t));
}

static inline void 
AddEntry(atlas_builder* Builder, u32 Width, u32 Height, u32 Channels, u8* Pixels) {
    AddEntry(Builder, Width, Height, Channels, Pixels, nullptr, 0);
}

// NOTE(Momo): Memory contains all entries in the following format
/*
rect2u, user_data_size (u32), user_data...rect2u, user_data_size (u32), user_data...

if user_data_size is 0, there will not be user_data
*/
struct atlas_builder_allocate_info_result {
    u32 Size;
    void* Memory;
};

static inline atlas_builder_allocate_info_result 
AllocateInfo(atlas_builder* Builder) 
{
    // NOTE(Momo): Precalculate size
    u32 Size = DynBufferCount(Builder->Entries) * sizeof(rect2u);
    for (u32 i = 0; i < DynBufferCount(Builder->Entries); ++i ){
        auto* Entry = Builder->Entries + i;
        Size += sizeof(u32) + Entry->UserDataSize; 
    }
    
    void* Memory = malloc(Size);
    u8* Itr = (u8*)Memory;
    
    for (u32 i = 0; i < DynBufferCount(Builder->Entries); ++i ){
        auto* Entry = Builder->Entries + i;
        rect2u Rect = *(Builder->Rects + Entry->RectIndex);
        Write(&Itr, Rect);
        Write(&Itr, Entry->UserDataSize);
        CopyBlock(Itr, Entry->UserData, Entry->UserDataSize);
        Itr += Entry->UserDataSize;
    }
    
    Assert(Itr <= (u8*)Memory + Size);
    
    return { Size, Memory };
}

static inline void
FreeAtlasInfo(atlas_builder_allocate_info_result AllocateInfoResult) {
    free(AllocateInfoResult.Memory);
}

struct atlas_builder_allocate_bitmap_result {
    b32 Ok;
    u32 Width, Height, Channels;
    u8* Bitmap;
};

static inline atlas_builder_allocate_bitmap_result 
AllocateBitmap(atlas_builder* Builder) {
    u32 BitmapSize = Builder->Width * Builder->Height * Builder->Channels;
    u8* BitmapMemory = (u8*)calloc(BitmapSize, sizeof(u8));
    if (!BitmapMemory) {
        return {};
    }
    
    // NOTE(Momo): Check. Total area of Rects must match 
    
    
    u32 EntryCount = DynBufferCount(Builder->Entries);
    for (u32 i = 0; i < EntryCount; ++i) {
        auto* Entry = Builder->Entries + i;
        rect2u Rect = *(Builder->Rects + Entry->RectIndex);
        
        Assert((u32)Entry->BitmapWidth == GetWidth(Rect));
        Assert((u32)Entry->BitmapHeight == GetHeight(Rect));
        Assert((u32)Entry->BitmapChannels == Builder->Channels); 
        
        i32 j = 0;
        for (u32 y = Rect.Min.Y; y < Rect.Min.Y + GetHeight(Rect); ++y) {
            for (u32 x = Rect.Min.X; x < Rect.Min.X + GetWidth(Rect); ++x) {
                u32 Index = TwoToOne(y, x, Builder->Width) * Builder->Channels;
                Assert(Index < BitmapSize);
                for (u32 c = 0; c < Builder->Channels; ++c) {
                    BitmapMemory[Index + c] = Entry->BitmapPixels[j++];
                }
            }
        }
        
    }
    
    return { true, Builder->Width, Builder->Height, Builder->Channels, BitmapMemory };
}

static inline void
FreeBitmap(atlas_builder_allocate_bitmap_result AllocateBitmapResult) {
    free(AllocateBitmapResult.Bitmap);
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
    u32 RectCount = DynBufferCount(Builder->Rects);
    for (u32 i = 0; i < RectCount; ++i) {
        DynBufferPush(Builder->SortedRects, &Builder->Rects[i]);
    }
    
    // TODO(Momo): Is there really no way to delare something that accepts a lambda?
    qsort(Builder->SortedRects, RectCount, sizeof(rect2u*), AtlasBuilderComparer);
}

// NOTE(Momo): For now, we only support POT scaling.
static inline void 
Pack(atlas_builder* Builder, u32 StartSize, u32 SizeLimit) {
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



static inline void 
Free(atlas_builder* Builder) {
    for (u32 i = 0; i < DynBufferCount(Builder->Entries); ++i) {
        free(Builder->Entries[i].UserData);
        free(Builder->Entries[i].BitmapPixels);
    }
    DynBufferFree(Builder->Entries);
    DynBufferFree(Builder->Rects);
    DynBufferFree(Builder->SortedRects);
}




static inline void 
Begin(atlas_builder* Builder) {
    Free(Builder);
}

static inline void 
End(atlas_builder* Builder, u32 StartSize = 128, u32 SizeLimit = 4096) {
    Assert(DynBufferCount(Builder->Entries) > 0);
    Sort(Builder);
    Pack(Builder, StartSize, SizeLimit);
}




#endif //RYOJI_ATLAS_BUILDER_H
