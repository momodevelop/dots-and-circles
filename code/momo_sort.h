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
struct sort_entry {
    f32 Key;
    u32 Index;
};

static inline u32
QuickSortPartition(sort_entry* Arr,
                   u32 LeftIndex, 
                   u32 OnePastRightIndex) 
{
    // Save the rightmost index as pivot
    // This frees up the right most index as a slot
    sort_entry PivotValue = Arr[OnePastRightIndex - 1]; 
    u32 SmallIndex = LeftIndex;
    u32 LargeIndex = OnePastRightIndex - 1;
    
    while(LargeIndex > SmallIndex) {
        // Check the value left of LargerIndex
        // If it is bigger than PivotValue, shift it right
        if (Arr[LargeIndex-1].Key > PivotValue.Key) {
            Arr[LargeIndex] = Arr[LargeIndex - 1];
            --LargeIndex;
        }
        
        // If the value left of LargeIndex is smaller than pivot,
        // swap positions with the value in SmallIndex
        else {
            Swap(sort_entry, Arr[LargeIndex-1], Arr[SmallIndex]);
            ++SmallIndex;
        }
    }
    
    Arr[LargeIndex] = PivotValue;
    return LargeIndex;
    
}

// NOTE(Momo): This is done inplace
static inline void 
QuickSortRange(sort_entry* Arr, 
               u32 LeftIndex, 
               u32 OnePastRightIndex) 
{
    if (OnePastRightIndex - LeftIndex <= 1) {
        return;
    }
    u32 PivotIndex = QuickSortPartition(Arr, 
                                        LeftIndex, 
                                        OnePastRightIndex);
    
    QuickSortRange(Arr, LeftIndex, PivotIndex);
    QuickSortRange(Arr, PivotIndex + 1, OnePastRightIndex);
    
    // Don't need to concatenate! O_o
}

static inline void
QuickSort(sort_entry* Arr, 
          u32 ArrCount) 
{
    QuickSortRange(Arr, 0, ArrCount);
}

#endif //MM_SORT_H
