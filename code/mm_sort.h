/* date = March 6th 2021 3:51 pm */

#ifndef MM_SORT_H
#define MM_SORT_H

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
