#ifndef __MM_AABB_PACKER__
#define __MM_AABB_PACKER__

// TODO(Momo): Very low priority, but we should rethink this
enum aabb_packer_sort_type {
    AabbPackerSortType_Width,
    AabbPackerSortType_Height,
    AabbPackerSortType_Area,
    AabbPackerSortType_Perimeter,
    AabbPackerSortType_Pathological,
    AabbPackerSortType_BiggerSide,
};

struct aabb_packer_node {
    u32 X, Y, W, H;
};

struct aabb_packer_aabb {
    // NOTE(Momo): Input
    u32 W, H; 
    
    // NOTE(Momo): For user's use
    void* UserData;
    
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
    
    
    usize CurrentNodeCount = 0;
    
    auto* Nodes = Arena_PushArray(aabb_packer_node, Arena, AabbCount+1);
    Nodes[CurrentNodeCount++] = { 0, 0, TotalWidth, TotalHeight };
    
    for (u32 i = 0; i < AabbCount; ++i) {
        aabb_packer_aabb* Aabb = Aabbs + SortEntries[i].Index;
        
        // NOTE(Momo): Iterate the empty spaces backwards to find the best fit index
        usize ChosenSpaceIndex = CurrentNodeCount;
        for (usize  j = 0; j < ChosenSpaceIndex ; ++j ) {
            usize Index = ChosenSpaceIndex - j - 1;
            aabb_packer_node Space = Nodes[Index];
            // NOTE(Momo): Check if the image fits
            if (Aabb->W <= Space.W && Aabb->H <= Space.H) {
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
        aabb_packer_node ChosenSpace = Nodes[ChosenSpaceIndex];
        if (CurrentNodeCount > 0) {
            Nodes[ChosenSpaceIndex] = Nodes[CurrentNodeCount-1];
            --CurrentNodeCount;
        }
        
        // NOTE(Momo): Split if not perfect fit
        if (ChosenSpace.W != Aabb->W && ChosenSpace.H == Aabb->H) {
            // Split right
            aabb_packer_node SplitSpaceRight = {
                ChosenSpace.X + Aabb->W,
                ChosenSpace.Y,
                ChosenSpace.W - Aabb->W,
                ChosenSpace.H
            };
            Nodes[CurrentNodeCount++] = SplitSpaceRight;
        }
        else if (ChosenSpace.W == Aabb->W && ChosenSpace.H != Aabb->H) {
            // Split down
            aabb_packer_node SplitSpaceDown = {
                ChosenSpace.X,
                ChosenSpace.Y + Aabb->H,
                ChosenSpace.W,
                ChosenSpace.H - Aabb->H
            };
            Nodes[CurrentNodeCount++] = SplitSpaceDown;
        }
        else if (ChosenSpace.W != Aabb->W && ChosenSpace.H != Aabb->H) {
            // Split right
            aabb_packer_node SplitSpaceRight = {
                ChosenSpace.X + Aabb->W,
                ChosenSpace.Y,
                ChosenSpace.W - Aabb->W,
                Aabb->H,
            };
            
            // Split down
            aabb_packer_node SplitSpaceDown = {
                ChosenSpace.X,
                ChosenSpace.Y + Aabb->H,
                ChosenSpace.W,
                ChosenSpace.H - Aabb->H,
            };
            
            // Choose to insert the bigger one first before the smaller one
            u32 RightArea = SplitSpaceRight.W * SplitSpaceRight.H;
            u32 DownArea = SplitSpaceDown.W * SplitSpaceDown.H;
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
        Aabb->X = ChosenSpace.X;
        Aabb->Y = ChosenSpace.Y;
    }
    
    return true;
}

#endif
