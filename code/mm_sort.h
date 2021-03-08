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

static inline usize
Sort_QuickSortPartition(sort_entry* Arr,
                        usize LeftIndex, 
                        usize OnePastRightIndex) 
{
    // Save the rightmost index as pivot
    // This frees up the right most index as a slot
    sort_entry PivotValue = Arr[OnePastRightIndex - 1]; 
    usize SmallIndex = LeftIndex;
    usize LargeIndex = OnePastRightIndex - 1;
    
    while(LargeIndex > SmallIndex) {
        // Check the value left of LargerIndex
        // If it is bigger than PivotValue, shift it right
        if (Arr[LargeIndex-1].Key < PivotValue.Key) {
            Arr[LargeIndex] = Arr[LargeIndex - 1];
            --LargeIndex;
        }
        
        // If the value left of LargeIndex is smaller than pivot,
        // swap positions with the value in SmallIndex
        else {
            Swap(Arr + LargeIndex - 1, Arr + SmallIndex);
            ++SmallIndex;
        }
    }
    
    Arr[LargeIndex] = PivotValue;
    return LargeIndex;
    
}

// NOTE(Momo): This is done inplace
static inline void 
Sort_QuickSort(sort_entry* Arr, 
               usize LeftIndex, 
               usize OnePastRightIndex) 
{
    if (OnePastRightIndex - LeftIndex <= 1) {
        return;
    }
    usize PivotIndex = Sort_QuickSortPartition(Arr, 
                                               LeftIndex, 
                                               OnePastRightIndex);
    
    Sort_QuickSort(Arr, LeftIndex, PivotIndex);
    Sort_QuickSort(Arr, PivotIndex + 1, OnePastRightIndex);
    
    // Don't need to concatenate! O_o
}

static inline void
Sort_QuickSort(sort_entry* Arr, 
               usize ArrCount) 
{
    Sort_QuickSort(Arr, 0, ArrCount);
}



template<typename type, typename comparer>
static inline usize
QuickSortPartition(type* Arr,
                   usize LeftIndex, 
                   usize OnePastRightIndex,
                   comparer Comparer) 
{
    
    // Save the rightmost index as pivot
    // This frees up the right most index as a slot
    type PivotValue = Arr[OnePastRightIndex - 1]; 
    usize SmallIndex = LeftIndex;
    usize LargeIndex = OnePastRightIndex - 1;
    
    while(LargeIndex > SmallIndex) {
        // Check the value left of LargerIndex
        // If it is bigger than PivotValue, shift it right
        b32 Result = Comparer(Arr[LargeIndex-1], PivotValue);
        if (Result) {
            Arr[LargeIndex] = Arr[LargeIndex - 1];
            --LargeIndex;
        }
        
        // If the value left of LargeIndex is smaller than pivot,
        // swap positions with the value in SmallIndex
        else {
            Swap(Arr + LargeIndex - 1, Arr + SmallIndex);
            ++SmallIndex;
        }
    }
    
    Arr[LargeIndex] = PivotValue;
    return LargeIndex;
    
}

// NOTE(Momo): This is done inplace
template<typename type, typename comparer>
static inline void 
QuickSort(type* Arr, 
          usize LeftIndex, 
          usize OnePastRightIndex,
          comparer Comparer) 
{
    if (OnePastRightIndex - LeftIndex <= 1) {
        return;
    }
    usize PivotIndex = QuickSortPartition(Arr, 
                                          LeftIndex, 
                                          OnePastRightIndex,
                                          Comparer);
    
    QuickSort(Arr, LeftIndex, PivotIndex, Comparer);
    QuickSort(Arr, PivotIndex + 1, OnePastRightIndex, Comparer);
    
    // Don't need to concatenate! O_o
}

template<typename type, typename comparer>
static inline void
QuickSort(type* Arr, 
          usize ArrCount, 
          comparer Comparer) 
{
    QuickSort<type, comparer>(Arr, 0, ArrCount, Comparer);
}

template<typename type, typename comparer>
static inline void
QuickSort(array<type> Arr, 
          comparer Comparer) 
{
    QuickSort<type, comparer>(Arr.Elements, Arr.Count, Comparer);
}


#endif //MM_SORT_H
