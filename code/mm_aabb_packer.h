#ifndef __MM_AABB_PACKER__
#define __MM_AABB_PACKER__

// TODO(Momo): Remove aabb_packer_node, replace with aabbu
// TODO(Momo): Remove aabb_packer_aabb, replace with aabbu


enum aabb_packer_sort_type {
    AabbPackerSortType_Width,
    AabbPackerSortType_Height,
    AabbPackerSortType_Area,
    AabbPackerSortType_Perimeter,
    AabbPackerSortType_Pathological,
    AabbPackerSortType_BiggerSide,
};

struct aabb_packer_aabb {
    // NOTE(Momo): Input
    u32 W, H; 
    
    // NOTE(Momo): Output
    u32 X, Y;
};

static inline void
__AabbPacker_Sort(aabb_packer_aabb* Aabbs,
                  sort_entry* SortEntries,
                  u32 SortEntryCount,
                  aabb_packer_sort_type SortType)
{
    switch (SortType) {
        case AabbPackerSortType_Width: {
            for (u32 I = 0; I < SortEntryCount; ++I) {
                f32 Key = -(f32)Aabbs[I].W;
                SortEntries[I].Key = Key;
                SortEntries[I].Index = I;
            }
        } break;
        case AabbPackerSortType_Height: {
            for (u32 I = 0; I < SortEntryCount; ++I) {
                f32 Key = -(f32)Aabbs[I].H;
                SortEntries[I].Key = Key;
                SortEntries[I].Index = I;
            }
        } break;
        case AabbPackerSortType_Area: {
            for (u32 I = 0; I < SortEntryCount; ++I) {
                f32 Key = -(f32)(Aabbs[I].W * Aabbs[I].H);
                SortEntries[I].Key = Key;
                SortEntries[I].Index = I;
            }
        } break;
        case AabbPackerSortType_Perimeter: {
            for (u32 I = 0; I < SortEntryCount; ++I) {
                f32 Key = -(f32)(Aabbs[I].W + Aabbs[I].H);
                SortEntries[I].Key = Key;
                SortEntries[I].Index = I;
            }
        } break;
        case AabbPackerSortType_BiggerSide: {
            for (u32 I = 0; I < SortEntryCount; ++I) {
                f32 Key = -(f32)(MaxOf(Aabbs[I].W, Aabbs[I].H));
                SortEntries[I].Key = Key;
                SortEntries[I].Index = I;
            }
        } break;
        case AabbPackerSortType_Pathological: {
            for (u32 I = 0; I < SortEntryCount; ++I) {
                u32 MaxOfWH = MaxOf(Aabbs[I].W, Aabbs[I].H);
                u32 MinOfWH = MinOf(Aabbs[I].W, Aabbs[I].H);
                f32 Key = -(f32)(MaxOfWH/MinOfWH * Aabbs[I].W * Aabbs[I].H);
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
                aabb_packer_aabb* Aabbs, 
                u32 AabbCount, 
                aabb_packer_sort_type SortType) 
{
    arena_mark Scratch = Arena_Mark(Arena);
    Defer { Arena_Revert(&Scratch); };
    auto* SortEntries = Arena_PushArray(sort_entry, Arena, AabbCount);
    
    __AabbPacker_Sort(Aabbs, SortEntries, AabbCount, SortType);
    
    
    u32 CurrentNodeCount = 0;
    
    auto* Nodes = Arena_PushArray(aabb2u, Arena, AabbCount+1);
    Nodes[CurrentNodeCount++] = { 0, 0, TotalWidth, TotalHeight };
    
    for (u32 i = 0; i < AabbCount; ++i) {
        aabb_packer_aabb* Aabb = Aabbs + SortEntries[i].Index;
        
        // NOTE(Momo): Iterate the empty spaces backwards to find the best fit index
        u32 ChosenSpaceIndex = CurrentNodeCount;
        for (u32  j = 0; j < ChosenSpaceIndex ; ++j ) {
            u32 Index = ChosenSpaceIndex - j - 1;
            aabb2u Space = Nodes[Index];
            u32 SpaceW = Aabb2u_Width(Space);
            u32 SpaceH = Aabb2u_Height(Space);
            
            // NOTE(Momo): Check if the image fits
            if (Aabb->W <= SpaceW && Aabb->H <= SpaceH) {
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
        if (ChosenSpaceW != Aabb->W && ChosenSpaceH == Aabb->H) {
            // Split right
            aabb2u SplitSpaceRight = 
                Aabb2u_CreateXYWH(ChosenSpace.Min.X + Aabb->W,
                                  ChosenSpace.Min.Y,
                                  ChosenSpaceW - Aabb->W,
                                  ChosenSpaceH);
            Nodes[CurrentNodeCount++] = SplitSpaceRight;
        }
        else if (ChosenSpaceW == Aabb->W && ChosenSpaceH != Aabb->H) {
            // Split down
            aabb2u SplitSpaceDown = 
                Aabb2u_CreateXYWH(ChosenSpace.Min.X,
                                  ChosenSpace.Min.Y + Aabb->H,
                                  ChosenSpaceW,
                                  ChosenSpaceH - Aabb->H);
            Nodes[CurrentNodeCount++] = SplitSpaceDown;
        }
        else if (ChosenSpaceW != Aabb->W && ChosenSpaceH != Aabb->H) {
            // Split right
            aabb2u SplitSpaceRight = 
                Aabb2u_CreateXYWH(ChosenSpace.Min.X + Aabb->W,
                                  ChosenSpace.Min.Y,
                                  ChosenSpaceW - Aabb->W,
                                  Aabb->H);
            
            // Split down
            aabb2u SplitSpaceDown = 
                Aabb2u_CreateXYWH(ChosenSpace.Min.X,
                                  ChosenSpace.Min.Y + Aabb->H,
                                  ChosenSpaceW,
                                  ChosenSpaceH - Aabb->H);
            
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
        Aabb->X = ChosenSpace.Min.X;
        Aabb->Y = ChosenSpace.Min.Y;
    }
    
    return true;
}

#endif
