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
struct SortEntry {
    f32 key;
    u32 index;
};

static inline u32
_QuickSortPartition(SortEntry* arr,
                     u32 left_index, 
                     u32 one_past_right_index) 
{
    // Save the rightmost index as pivot
    // This frees up the right most index as a slot
    SortEntry pivot_value = arr[one_past_right_index - 1]; 
    u32 small_index = left_index;
    u32 large_index = one_past_right_index - 1;
    
    while(large_index > small_index) {
        // Check the value left of LargerIndex
        // If it is bigger than pivot_value, shift it right
        if (arr[large_index-1].key > pivot_value.key) {
            arr[large_index] = arr[large_index - 1];
            --large_index;
        }
        
        // If the value left of large_index is smaller than pivot,
        // swap positions with the value in small_index
        else {
            Swap(arr[large_index-1], arr[small_index]);
            ++small_index;
        }
    }
    
    arr[large_index] = pivot_value;
    return large_index;
    
}

// NOTE(Momo): This is done inplace
static inline void 
QuickSortRange(SortEntry* arr, 
                 u32 left_index, 
                 u32 one_past_right_index) 
{
    if (one_past_right_index - left_index <= 1) {
        return;
    }
    u32 pivot_index = _QuickSortPartition(arr, 
                                           left_index, 
                                           one_past_right_index);
    
    QuickSortRange(arr, left_index, pivot_index);
    QuickSortRange(arr, pivot_index + 1, one_past_right_index);
    
    // Don't need to concatenate! O_o
}

static inline void
QuickSort(SortEntry* arr, 
           u32 arr_count) 
{
    QuickSortRange(arr, 0, arr_count);
}

#endif //MM_SORT_H
