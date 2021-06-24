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
static inline b8
AabbPacker_Pack(Arena* arena,
                u32 total_width,
                u32 total_height,
                aabb2u* Aabbs, 
                u32 aabb_count, 
                aabb_packer_sort_type SortType) 
{
    Arena_Mark scratch = arena->mark();
    Defer { scratch.revert(); };
    auto* sort_entries = arena->push_array<sort_entry>(aabb_count);
    
    __AabbPacker_Sort(Aabbs, sort_entries, aabb_count, SortType);
    
    
    u32 current_node_count = 0;
    
    auto* nodes = arena->push_array<aabb2u>(aabb_count+1);
    nodes[current_node_count++] = Aabb2u_Create(0, 0, total_width, total_height);
    
    for (u32 i = 0; i < aabb_count; ++i) {
        aabb2u* Aabb = Aabbs + sort_entries[i].Index;
        u32 aabb_width = Aabb2u_Width(*Aabb);
        u32 aabb_height = Aabb2u_Height(*Aabb);
        
        // NOTE(Momo): Iterate the empty spaces backwards to find the best fit index
        u32 chosen_space_index = current_node_count;
        for (u32  j = 0; j < chosen_space_index ; ++j ) {
            u32 Index = chosen_space_index - j - 1;
            aabb2u Space = nodes[Index];
            u32 SpaceW = Aabb2u_Width(Space);
            u32 SpaceH = Aabb2u_Height(Space);
            
            // NOTE(Momo): Check if the image fits
            if (aabb_width <= SpaceW && aabb_height <= SpaceH) {
                chosen_space_index = Index;
                break;
            }
        }
        
        
        // NOTE(Momo): If an empty space that can fit is found, 
        // we remove that space and split.
        if (chosen_space_index == current_node_count) {
            return false;
        }
        
        // NOTE(Momo): Swap and pop the chosen space
        aabb2u chosen_space = nodes[chosen_space_index];
        u32 chosen_space_w = Aabb2u_Width(chosen_space);
        u32 chosen_space_h = Aabb2u_Height(chosen_space);
        
        if (current_node_count > 0) {
            nodes[chosen_space_index] = nodes[current_node_count-1];
            --current_node_count;
        }
        
        // NOTE(Momo): Split if not perfect fit
        if (chosen_space_w != aabb_width && chosen_space_h == aabb_height) {
            // Split right
            aabb2u split_space_right = 
                Aabb2u_CreateXYWH(chosen_space.Min.X + aabb_width,
                                  chosen_space.Min.Y,
                                  chosen_space_w - aabb_width,
                                  chosen_space_h);
            nodes[current_node_count++] = split_space_right;
        }
        else if (chosen_space_w == aabb_width && chosen_space_h != aabb_height) {
            // Split down
            aabb2u split_space_down = 
                Aabb2u_CreateXYWH(chosen_space.Min.X,
                                  chosen_space.Min.Y + aabb_height,
                                  chosen_space_w,
                                  chosen_space_h - aabb_height);
            nodes[current_node_count++] = split_space_down;
        }
        else if (chosen_space_w != aabb_width && chosen_space_h != aabb_height) {
            // Split right
            aabb2u split_space_right = 
                Aabb2u_CreateXYWH(chosen_space.Min.X + aabb_width,
                                  chosen_space.Min.Y,
                                  chosen_space_w - aabb_width,
                                  aabb_height);
            
            // Split down
            aabb2u split_space_down = 
                Aabb2u_CreateXYWH(chosen_space.Min.X,
                                  chosen_space.Min.Y + aabb_height,
                                  chosen_space_w,
                                  chosen_space_h - aabb_height);
            
            // Choose to insert the bigger one first before the smaller one
            u32 split_space_right_w = Aabb2u_Width(split_space_right);
            u32 split_splace_right_h = Aabb2u_Height(split_space_right);
            u32 split_space_downW = Aabb2u_Width(split_space_down);
            u32 split_space_downH = Aabb2u_Height(split_space_down);
            
            u32 right_area = split_space_right_w * split_splace_right_h;
            u32 down_area = split_space_downW * split_space_downH;
            
            if (right_area > down_area) {
                nodes[current_node_count++] = split_space_right;
                nodes[current_node_count++] = split_space_down;
            }
            else {
                nodes[current_node_count++] = split_space_down;
                nodes[current_node_count++] = split_space_right;
            }
            
        }
        
        // NOTE(Momo): Translate the Aabb
        (*Aabb) = Aabb2u_Translate((*Aabb),
                                   chosen_space.Min.X,
                                   chosen_space.Min.Y);
    }
    
    return true;
}

#endif
