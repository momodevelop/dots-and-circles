#ifndef __MM_AABB_PACKER__
#define __MM_AABB_PACKER__

enum MM_AabbPacker_SORT_TYPE {
    MM_AabbPacker_SORT_TYPE_WIDTH,
    MM_AabbPacker_SORT_TYPE_HEIGHT,
    MM_AabbPacker_SORT_TYPE_AREA,
    MM_AabbPacker_SORT_TYPE_PERIMETER,
    MM_AabbPacker_SORT_TYPE_PATHOLOGICAL,
    MM_AabbPacker_SORT_TYPE_BIGGER_SIDE,
};
static inline void
MM_AabbPacker__Sort(MM_Aabb2u* aabbs,
                    MM_Sort_Entry* sort_entries,
                    u32 sort_entry_count,
                    MM_AabbPacker_SORT_TYPE sort_type)
{
    switch (sort_type) {
        case MM_AabbPacker_SORT_TYPE_WIDTH: {
            for (u32 I = 0; I < sort_entry_count; ++I) {
                u32 aabb_width = MM_Aabb2u_Width(aabbs[I]);
                f32 Key = -(f32)aabb_width;
                sort_entries[I].key = Key;
                sort_entries[I].index = I;
            }
        } break;
        case MM_AabbPacker_SORT_TYPE_HEIGHT: {
            for (u32 I = 0; I < sort_entry_count; ++I) {
                u32 aabb_height = MM_Aabb2u_Height(aabbs[I]);
                f32 Key = -(f32)aabb_height;
                sort_entries[I].key = Key;
                sort_entries[I].index = I;
            }
        } break;
        case MM_AabbPacker_SORT_TYPE_AREA: {
            for (u32 I = 0; I < sort_entry_count; ++I) {
                u32 aabb_width = MM_Aabb2u_Width(aabbs[I]);
                u32 aabb_height = MM_Aabb2u_Height(aabbs[I]);
                f32 Key = -(f32)(aabb_width * aabb_height);
                sort_entries[I].key = Key;
                sort_entries[I].index = I;
            }
        } break;
        case MM_AabbPacker_SORT_TYPE_PERIMETER: {
            for (u32 I = 0; I < sort_entry_count; ++I) {
                u32 aabb_width = MM_Aabb2u_Width(aabbs[I]);
                u32 aabb_height = MM_Aabb2u_Height(aabbs[I]);
                f32 Key = -(f32)(aabb_width + aabb_height);
                sort_entries[I].key = Key;
                sort_entries[I].index = I;
            }
        } break;
        case MM_AabbPacker_SORT_TYPE_BIGGER_SIDE: {
            for (u32 I = 0; I < sort_entry_count; ++I) {
                u32 aabb_width = MM_Aabb2u_Width(aabbs[I]);
                u32 aabb_height = MM_Aabb2u_Height(aabbs[I]);
                f32 Key = -(f32)(MaxOf(aabb_width, aabb_height));
                sort_entries[I].key = Key;
                sort_entries[I].index = I;
            }
        } break;
        case MM_AabbPacker_SORT_TYPE_PATHOLOGICAL: {
            for (u32 I = 0; I < sort_entry_count; ++I) {
                u32 aabb_width = MM_Aabb2u_Width(aabbs[I]);
                u32 aabb_height = MM_Aabb2u_Height(aabbs[I]);
                
                u32 MaxOfWH = MaxOf(aabb_width, aabb_height);
                u32 MinOfWH = MinOf(aabb_width, aabb_height);
                f32 Key = -(f32)(MaxOfWH/MinOfWH * aabb_width * aabb_height);
                sort_entries[I].key = Key;
                sort_entries[I].index = I;
            }
        } break;
        
    }
    
    MM_Sort_QuickSort(sort_entries, sort_entry_count);
    
}

// NOTE(Momo): aabbs WILL be sorted after this function
static inline b32
MM_AabbPacker_Pack(MM_Arena* arena,
                   u32 total_width,
                   u32 total_height,
                   MM_Aabb2u* aabbs, 
                   u32 aabb_count, 
                   MM_AabbPacker_SORT_TYPE sort_type) 
{
    MM_ArenaMark scratch = MM_Arena_Mark(arena);
    Defer { MM_Arena_Revert(&scratch); };
    auto* sort_entries = MM_Arena_PushArray(MM_Sort_Entry, arena, aabb_count);
    
    MM_AabbPacker__Sort(aabbs, sort_entries, aabb_count, sort_type);
    
    
    u32 cur_node_count = 0;
    
    auto* nodes = MM_Arena_PushArray(MM_Aabb2u, arena, aabb_count+1);
    nodes[cur_node_count++] = MM_Aabb2u_Create(0, 0, total_width, total_height);
    
    for (u32 i = 0; i < aabb_count; ++i) {
        MM_Aabb2u* aabb = aabbs + sort_entries[i].index;
        u32 aabb_width = MM_Aabb2u_Width(*aabb);
        u32 aabb_height = MM_Aabb2u_Height(*aabb);
        
        // NOTE(Momo): Iterate the empty spaces backwards to find the best fit index
        u32 chosen_space_idx = cur_node_count;
        for (u32  j = 0; j < chosen_space_idx ; ++j ) {
            u32 index = chosen_space_idx - j - 1;
            MM_Aabb2u space = nodes[index];
            u32 space_width = MM_Aabb2u_Width(space);
            u32 space_height = MM_Aabb2u_Height(space);
            
            // NOTE(Momo): Check if the image fits
            if (aabb_width <= space_width && aabb_height <= space_height) {
                chosen_space_idx = index;
                break;
            }
        }
        
        
        // NOTE(Momo): If an empty space that can fit is found, 
        // we remove that space and split.
        if (chosen_space_idx == cur_node_count) {
            return false;
        }
        
        // NOTE(Momo): Swap and pop the chosen space
        MM_Aabb2u chosen_space = nodes[chosen_space_idx];
        u32 chosen_space_width = MM_Aabb2u_Width(chosen_space);
        u32 chosen_space_height = MM_Aabb2u_Height(chosen_space);
        
        if (cur_node_count > 0) {
            nodes[chosen_space_idx] = nodes[cur_node_count-1];
            --cur_node_count;
        }
        
        // NOTE(Momo): Split if not perfect fit
        if (chosen_space_width != aabb_width && chosen_space_height == aabb_height) {
            // Split right
            MM_Aabb2u split_space_right = 
                MM_Aabb2u_CreateXYWH(chosen_space.min.x + aabb_width,
                                     chosen_space.min.y,
                                     chosen_space_width - aabb_width,
                                     chosen_space_height);
            nodes[cur_node_count++] = split_space_right;
        }
        else if (chosen_space_width == aabb_width && chosen_space_height != aabb_height) {
            // Split down
            MM_Aabb2u split_space_down = 
                MM_Aabb2u_CreateXYWH(chosen_space.min.x,
                                     chosen_space.min.y + aabb_height,
                                     chosen_space_width,
                                     chosen_space_height - aabb_height);
            nodes[cur_node_count++] = split_space_down;
        }
        else if (chosen_space_width != aabb_width && chosen_space_height != aabb_height) {
            // Split right
            MM_Aabb2u split_space_right = 
                MM_Aabb2u_CreateXYWH(chosen_space.min.x + aabb_width,
                                     chosen_space.min.y,
                                     chosen_space_width - aabb_width,
                                     aabb_height);
            
            // Split down
            MM_Aabb2u split_space_down = 
                MM_Aabb2u_CreateXYWH(chosen_space.min.x,
                                     chosen_space.min.y + aabb_height,
                                     chosen_space_width,
                                     chosen_space_height - aabb_height);
            
            // Choose to insert the bigger one first before the smaller one
            u32 split_space_right_w = MM_Aabb2u_Width(split_space_right);
            u32 split_space_right_h = MM_Aabb2u_Height(split_space_right);
            u32 split_space_down_w = MM_Aabb2u_Width(split_space_down);
            u32 split_space_down_h = MM_Aabb2u_Height(split_space_down);
            
            u32 RightArea = split_space_right_w * split_space_right_h;
            u32 DownArea = split_space_down_w * split_space_down_h;
            
            if (RightArea > DownArea) {
                nodes[cur_node_count++] = split_space_right;
                nodes[cur_node_count++] = split_space_down;
            }
            else {
                nodes[cur_node_count++] = split_space_down;
                nodes[cur_node_count++] = split_space_right;
            }
            
        }
        
        // NOTE(Momo): Translate the aabb
        (*aabb) = MM_Aabb2u_Translate((*aabb),
                                      chosen_space.min.x,
                                      chosen_space.min.y);
    }
    
    return true;
}

#endif
