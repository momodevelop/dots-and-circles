#ifndef __MM_AABB_PACKER__
#define __MM_AABB_PACKER__

// NOTE(Momo): Requires "mm_maths.h"

enum aabb_packer_sort_type {
    AabbPackerSortType_Width,
    AabbPackerSortType_Height,
    AabbPackerSortType_Area,
    AabbPackerSortType_Perimeter,
    AabbPackerSortType_Pathological,
    AabbPackerSortType_BiggerSide,
};

static inline void
__AabbPacker_Sort(aabb2u* Aabbs,
                  sort_entry* SortEntries,
                  u32 SortEntryCount,
                  aabb_packer_sort_type SortType)
{
    switch (SortType) {
        case AabbPackerSortType_Width: {
            for (u32 I = 0; I < SortEntryCount; ++I) {
                u32 AabbW = Aabb2u_Width(Aabbs[I]);
                f32 Key = -(f32)AabbW;
                SortEntries[I].Key = Key;
                SortEntries[I].Index = I;
            }
        } break;
        case AabbPackerSortType_Height: {
            for (u32 I = 0; I < SortEntryCount; ++I) {
                u32 AabbH = Aabb2u_Height(Aabbs[I]);
                f32 Key = -(f32)AabbH;
                SortEntries[I].Key = Key;
                SortEntries[I].Index = I;
            }
        } break;
        case AabbPackerSortType_Area: {
            for (u32 I = 0; I < SortEntryCount; ++I) {
                u32 AabbW = Aabb2u_Width(Aabbs[I]);
                u32 AabbH = Aabb2u_Height(Aabbs[I]);
                f32 Key = -(f32)(AabbW * AabbH);
                SortEntries[I].Key = Key;
                SortEntries[I].Index = I;
            }
        } break;
        case AabbPackerSortType_Perimeter: {
            for (u32 I = 0; I < SortEntryCount; ++I) {
                u32 AabbW = Aabb2u_Width(Aabbs[I]);
                u32 AabbH = Aabb2u_Height(Aabbs[I]);
                f32 Key = -(f32)(AabbW + AabbH);
                SortEntries[I].Key = Key;
                SortEntries[I].Index = I;
            }
        } break;
        case AabbPackerSortType_BiggerSide: {
            for (u32 I = 0; I < SortEntryCount; ++I) {
                u32 AabbW = Aabb2u_Width(Aabbs[I]);
                u32 AabbH = Aabb2u_Height(Aabbs[I]);
                f32 Key = -(f32)(MaxOf(AabbW, AabbH));
                SortEntries[I].Key = Key;
                SortEntries[I].Index = I;
            }
        } break;
        case AabbPackerSortType_Pathological: {
            for (u32 I = 0; I < SortEntryCount; ++I) {
                u32 AabbW = Aabb2u_Width(Aabbs[I]);
                u32 AabbH = Aabb2u_Height(Aabbs[I]);
                
                u32 MaxOfWH = MaxOf(AabbW, AabbH);
                u32 MinOfWH = MinOf(AabbW, AabbH);
                f32 Key = -(f32)(MaxOfWH/MinOfWH * AabbW * AabbH);
                SortEntries[I].Key = Key;
                SortEntries[I].Index = I;
            }
        } break;
        
    }
    
    QuickSort(SortEntries, SortEntryCount);
    
}

// NOTE(Momo): Aabbs WILL be sorted after this function
static inline b32
AabbPacker_Pack(arena* Arena,
                u32 TotalWidth,
                u32 TotalHeight,
                aabb2u* Aabbs, 
                u32 AabbCount, 
                aabb_packer_sort_type SortType) 
{
    arena_mark Scratch = MM_Arena_Mark(Arena);
    Defer { MM_Arena_Revert(&Scratch); };
    auto* SortEntries = MM_Arena_PushArray(sort_entry, Arena, AabbCount);
    
    __AabbPacker_Sort(Aabbs, SortEntries, AabbCount, SortType);
    
    
    u32 CurrentNodeCount = 0;
    
    auto* Nodes = MM_Arena_PushArray(aabb2u, Arena, AabbCount+1);
    Nodes[CurrentNodeCount++] = Aabb2u_Create(0, 0, TotalWidth, TotalHeight);
    
    for (u32 i = 0; i < AabbCount; ++i) {
        aabb2u* Aabb = Aabbs + SortEntries[i].Index;
        u32 AabbW = Aabb2u_Width(*Aabb);
        u32 AabbH = Aabb2u_Height(*Aabb);
        
        // NOTE(Momo): Iterate the empty spaces backwards to find the best fit index
        u32 ChosenSpaceIndex = CurrentNodeCount;
        for (u32  j = 0; j < ChosenSpaceIndex ; ++j ) {
            u32 Index = ChosenSpaceIndex - j - 1;
            aabb2u Space = Nodes[Index];
            u32 SpaceW = Aabb2u_Width(Space);
            u32 SpaceH = Aabb2u_Height(Space);
            
            // NOTE(Momo): Check if the image fits
            if (AabbW <= SpaceW && AabbH <= SpaceH) {
                ChosenSpaceIndex = Index;
                break;
            }
        }
        
        
        // NOTE(Momo): If an empty space that can fit is found, 
        // we remove that space and split.
        if (ChosenSpaceIndex == CurrentNodeCount) {
            return false;
        }
        
        // NOTE(Momo): Swap and pop the chosen space
        aabb2u ChosenSpace = Nodes[ChosenSpaceIndex];
        u32 ChosenSpaceW = Aabb2u_Width(ChosenSpace);
        u32 ChosenSpaceH = Aabb2u_Height(ChosenSpace);
        
        if (CurrentNodeCount > 0) {
            Nodes[ChosenSpaceIndex] = Nodes[CurrentNodeCount-1];
            --CurrentNodeCount;
        }
        
        // NOTE(Momo): Split if not perfect fit
        if (ChosenSpaceW != AabbW && ChosenSpaceH == AabbH) {
            // Split right
            aabb2u SplitSpaceRight = 
                Aabb2u_CreateXYWH(ChosenSpace.Min.X + AabbW,
                                  ChosenSpace.Min.Y,
                                  ChosenSpaceW - AabbW,
                                  ChosenSpaceH);
            Nodes[CurrentNodeCount++] = SplitSpaceRight;
        }
        else if (ChosenSpaceW == AabbW && ChosenSpaceH != AabbH) {
            // Split down
            aabb2u SplitSpaceDown = 
                Aabb2u_CreateXYWH(ChosenSpace.Min.X,
                                  ChosenSpace.Min.Y + AabbH,
                                  ChosenSpaceW,
                                  ChosenSpaceH - AabbH);
            Nodes[CurrentNodeCount++] = SplitSpaceDown;
        }
        else if (ChosenSpaceW != AabbW && ChosenSpaceH != AabbH) {
            // Split right
            aabb2u SplitSpaceRight = 
                Aabb2u_CreateXYWH(ChosenSpace.Min.X + AabbW,
                                  ChosenSpace.Min.Y,
                                  ChosenSpaceW - AabbW,
                                  AabbH);
            
            // Split down
            aabb2u SplitSpaceDown = 
                Aabb2u_CreateXYWH(ChosenSpace.Min.X,
                                  ChosenSpace.Min.Y + AabbH,
                                  ChosenSpaceW,
                                  ChosenSpaceH - AabbH);
            
            // Choose to insert the bigger one first before the smaller one
            u32 SplitSpaceRightW = Aabb2u_Width(SplitSpaceRight);
            u32 SplitSpaceRightH = Aabb2u_Height(SplitSpaceRight);
            u32 SplitSpaceDownW = Aabb2u_Width(SplitSpaceDown);
            u32 SplitSpaceDownH = Aabb2u_Height(SplitSpaceDown);
            
            u32 RightArea = SplitSpaceRightW * SplitSpaceRightH;
            u32 DownArea = SplitSpaceDownW * SplitSpaceDownH;
            
            if (RightArea > DownArea) {
                Nodes[CurrentNodeCount++] = SplitSpaceRight;
                Nodes[CurrentNodeCount++] = SplitSpaceDown;
            }
            else {
                Nodes[CurrentNodeCount++] = SplitSpaceDown;
                Nodes[CurrentNodeCount++] = SplitSpaceRight;
            }
            
        }
        
        // NOTE(Momo): Translate the Aabb
        (*Aabb) = Aabb2u_Translate((*Aabb),
                                   ChosenSpace.Min.X,
                                   ChosenSpace.Min.Y);
    }
    
    return true;
}

#endif
