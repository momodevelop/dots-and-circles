#ifndef __MM_AABB_PACKER__
#define __MM_AABB_PACKER__

// TODO: Do I want these stuff in an object?
enum AABB_Packer_Sort_Type {
    AABB_PACKER_SORT_WIDTH,
    AABB_PACKER_SORT_HEIGHT,
    AABB_PACKER_SORT_AREA,
    AABB_PACKER_SORT_PERIMETER,
    AABB_PACKER_SORT_BIGGER_SIZE,
    AABB_PACKER_SORT_PATHOLOGICAL,
};

static inline void
sort_aabbs_for_packer(aabb2u* aabbs,
                      Sort_Entry* sort_entries,
                      u32 sort_entry_count,
                      AABB_Packer_Sort_Type sort_type)
{
    switch (sort_type) {
        case AABB_PACKER_SORT_WIDTH: {
            for (u32 i = 0; i < sort_entry_count; ++i) {
                u32 aabb_w = width(aabbs[i]);
                f32 key = -(f32)aabb_w;
                sort_entries[i].key = key;
                sort_entries[i].index = i;
            }
        } break;
        case AABB_PACKER_SORT_HEIGHT: {
            for (u32 i = 0; i < sort_entry_count; ++i) {
                u32 aabb_h = height(aabbs[i]);
                f32 key = -(f32)aabb_h;
                sort_entries[i].key = key;
                sort_entries[i].index = i;
            }
        } break;
        case AABB_PACKER_SORT_AREA: {
            for (u32 i = 0; i < sort_entry_count; ++i) {
                u32 aabb_w = width(aabbs[i]);
                u32 aabb_h = height(aabbs[i]);
                f32 key = -(f32)(aabb_w * aabb_h);
                sort_entries[i].key = key;
                sort_entries[i].index = i;
            }
        } break;
        case AABB_PACKER_SORT_PERIMETER: {
            for (u32 i = 0; i < sort_entry_count; ++i) {
                u32 aabb_w = width(aabbs[i]);
                u32 aabb_h = height(aabbs[i]);
                f32 key = -(f32)(aabb_w + aabb_h);
                sort_entries[i].key = key;
                sort_entries[i].index = i;
            }
        } break;
        case AABB_PACKER_SORT_BIGGER_SIZE: {
            for (u32 i = 0; i < sort_entry_count; ++i) {
                u32 aabb_w = width(aabbs[i]);
                u32 aabb_h = height(aabbs[i]);
                f32 key = -(f32)(MAX(aabb_w, aabb_h));
                sort_entries[i].key = key;
                sort_entries[i].index = i;
            }
        } break;
        case AABB_PACKER_SORT_PATHOLOGICAL: {
            for (u32 i = 0; i < sort_entry_count; ++i) {
                u32 aabb_w = width(aabbs[i]);
                u32 aabb_h = height(aabbs[i]);
                
                u32 wh_max = MAX(aabb_w, aabb_h);
                u32 wh_min = MIN(aabb_w, aabb_h);
                f32 key = -(f32)(wh_max/wh_min * aabb_w * aabb_h);
                sort_entries[i].key = key;
                sort_entries[i].index = i;
            }
        } break;
        
    }
    
    quick_sort(sort_entries, sort_entry_count);
    
}

// NOTE(Momo): Aabbs WILL be sorted after this function
static inline b8
pack_aabbs(Arena* arena,
           u32 total_width,
           u32 total_height,
           aabb2u* aabbs, 
           u32 aabb_count, 
           AABB_Packer_Sort_Type sort_type) 
{
    Arena_Marker scratch = Arena_Mark(arena);
    defer { Arena_Revert(&scratch); };
    auto* sort_entries = Arena_PushArray<Sort_Entry>(arena, aabb_count);
    
    sort_aabbs_for_packer(aabbs, sort_entries, aabb_count, sort_type);
    
    
    u32 current_node_count = 0;
    
    auto* nodes = Arena_PushArray<aabb2u>(arena, aabb_count+1);
    nodes[current_node_count++] = aabb2u::create(0, 0, total_width, total_height);
    
    for (u32 i = 0; i < aabb_count; ++i) {
        aabb2u* Aabb = aabbs + sort_entries[i].index;
        u32 aabb_width = width(*Aabb);
        u32 aabb_height = height(*Aabb);
        
        // NOTE(Momo): Iterate the empty spaces backwards to find the best fit index
        u32 chosen_space_index = current_node_count;
        for (u32  j = 0; j < chosen_space_index ; ++j ) {
            u32 Index = chosen_space_index - j - 1;
            aabb2u Space = nodes[Index];
            u32 SpaceW = width(Space);
            u32 SpaceH = height(Space);
            
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
        u32 chosen_space_w = width(chosen_space);
        u32 chosen_space_h = height(chosen_space);
        
        if (current_node_count > 0) {
            nodes[chosen_space_index] = nodes[current_node_count-1];
            --current_node_count;
        }
        
        // NOTE(Momo): Split if not perfect fit
        if (chosen_space_w != aabb_width && chosen_space_h == aabb_height) {
            // Split right
            aabb2u split_space_right = 
                aabb2u::create_xywh(chosen_space.min.x + aabb_width,
                                    chosen_space.min.y,
                                    chosen_space_w - aabb_width,
                                    chosen_space_h);
            nodes[current_node_count++] = split_space_right;
        }
        else if (chosen_space_w == aabb_width && chosen_space_h != aabb_height) {
            // Split down
            aabb2u split_space_down = 
                aabb2u::create_xywh(chosen_space.min.x,
                                    chosen_space.min.y + aabb_height,
                                    chosen_space_w,
                                    chosen_space_h - aabb_height);
            nodes[current_node_count++] = split_space_down;
        }
        else if (chosen_space_w != aabb_width && chosen_space_h != aabb_height) {
            // Split right
            aabb2u split_space_right = 
                aabb2u::create_xywh(chosen_space.min.x + aabb_width,
                                    chosen_space.min.y,
                                    chosen_space_w - aabb_width,
                                    aabb_height);
            
            // Split down
            aabb2u split_space_down = 
                aabb2u::create_xywh(chosen_space.min.x,
                                    chosen_space.min.y + aabb_height,
                                    chosen_space_w,
                                    chosen_space_h - aabb_height);
            
            // Choose to insert the bigger one first before the smaller one
            u32 split_space_right_w = width(split_space_right);
            u32 split_splace_right_h = height(split_space_right);
            u32 split_space_downW = width(split_space_down);
            u32 split_space_downH = height(split_space_down);
            
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
        (*Aabb) = translate((*Aabb),
                            chosen_space.min.x,
                            chosen_space.min.y);
    }
    
    return true;
}


#endif
