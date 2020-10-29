// This is a pool with a return policy that is done by 'swap removal'.
// That is, when an object is returned, it will simply copy the last used object onto it.
// Thus a few things are not garaunteed:
// 1. The object iteration order is mostly likely not in order of borrowing.
// 2. The pointer returned to the use with borrow might be invalid.
//
// However, the advantage is that the borrowed objects are contiguous
//

#ifndef MM_SWAP_REMOVE_POOL_H
#define MM_SWAP_REMOVE_POOL_H

#include "mm_core.h"

template<typename T>
struct mmsrp_pool {
    T* Objects;     // Expects pointer to object
    usize Used;     // Amount of objects currently borrowed
    usize Capacity; // Total number of borrowable objects.

    T& operator[](usize Index) {
        Assert(Index < Capacity);
        return Objects[Index];
    }
};

template<typename T>
static inline void 
mmsrp_Clear(mmsrp_pool<T>* Pool) {
    Pool->Used = 0;
}


template<typename T>
static inline mmsrp_pool<T>
mmsrp_CreatePool(T* Arr, usize Capacity) {
    mmsrp_pool Ret = {};
    Ret.Objects = Arr;
    Ret.Capacity= Capacity;
    return Ret;
}

template<typename T>
static inline T*
mmsrp_Borrow(mmsrp_pool<T>* Pool) {
    Assert(Pool->Used < Pool->Capacity);
    return Pool->Objects + Pool->Used++;
}

// This might actually be faster than the one above?
template<typename T>
static inline void
mmsrp_Return(mmsrp_pool<T>* Pool, T* Obj) {
    (*Obj) = Pool->Objects[Pool->Used - 1];
    --Pool->Used;
}


#include "mm_arena.h"
template<typename T>
static inline mmsrp_pool<T>
mmsrp_PushPool(mmarn_arena* Arena, usize Capacity) {
    mmsrp_pool<T> Ret = {};
    Ret.Objects = mmarn_PushArray<T>(Arena, Capacity);
    Ret.Capacity = Capacity;
    return Ret;
}




#endif // MM_POOL_H
