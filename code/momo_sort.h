/* date = March 6th 2021 3:51 pm */

#ifndef MM_SORT_H
#define MM_SORT_H

// NOTE(Momo): There is a issue I have with using templates and that is:
// When doing any kind of swap in sorting, if the type is bigger than a certain
// amount, we are bound to bust the cache size at some point if used carelessly
//
// Instead, I think it's preferable to keep the things we want to swap small by 
// introducing a struct containing the index of the item they want to swap 
// (in their hypothetical array) and a value contain the key to compare with
// for sorting. 
//
// All sorting algorithms will therefore just sort an array of these structs
// in ascending order (from lowest key to biggest key). 
// If they need to go from biggest to smallest, they can just iterate backwards.

// NOTE(Momo): 64-bits!
struct MM_Sort_Entry {
    f32 key;
    u32 index;
};

static inline u32
MM_Sort_Quick__Partition(MM_Sort_Entry* entries,
                   u32 left_index, 
                   u32 one_past_right_index) 
{
    // Save the rightmost index as pivot
    // This frees up the right most index as a slot
    MM_Sort_Entry pivot_value = entries[one_past_right_index - 1]; 
    u32 smol_index = left_index;
    u32 large_index = one_past_right_index - 1;
    
    while(large_index > smol_index) {
        // Check the value left of LargerIndex
        // If it is bigger than pivot_value, shift it right
        if (entries[large_index-1].key > pivot_value.key) {
            entries[large_index] = entries[large_index - 1];
            --large_index;
        }
        
        // If the value left of large_index is smaller than pivot,
        // swap positions with the value in smol_index
        else {
            Swap(MM_Sort_Entry, entries[large_index-1], entries[smol_index]);
            ++smol_index;
        }
    }
    
    entries[large_index] = pivot_value;
    return large_index;
    
}

// NOTE(Momo): This is done inplace
static inline void 
MM_Sort_QuickSortRange(MM_Sort_Entry* entries, 
               u32 left_index, 
               u32 one_past_right_index) 
{
    if (one_past_right_index - left_index <= 1) {
        return;
    }
    u32 pivot_index = MM_Sort_Quick__Partition(entries, 
                                        left_index, 
                                        one_past_right_index);
    
    MM_Sort_QuickSortRange(entries, left_index, pivot_index);
    MM_Sort_QuickSortRange(entries, pivot_index + 1, one_past_right_index);
    
    // Don't need to concatenate! O_o
}

static inline void
MM_Sort_QuickSort(MM_Sort_Entry* entries, 
          u32 entry_count) 
{
    MM_Sort_QuickSortRange(entries, 0, entry_count);
}

#endif //MM_SORT_H
