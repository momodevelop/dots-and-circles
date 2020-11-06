// This is a list with a erase policy that is done by 'swap removal'.
// That is, when an object is erased, it will simply copy the last object onto it.
//
// Advantages: 
// 1. Borrowed objects are in a contiguous block. 
//    So iterating over borrowed objects like this is valid:
//          for (u32 i = 0; i < List.Used; ++i);  
//    
//    Returning in the middle of an iteration is little awkward, but possible.
//    Just don't increment the iterator when you return:
//          for (u32 i = 0; i < List.Used; ++i) {
//              mmp_Return(&List[i--]);    
//          }
//    This is because after returning, the slot of the object will be swapped with 
//    the last borrowed object. 
// 
// 2. Erase is very fast at O(1). It is just a copy/paste of 1 entry. 
// 
// Disadvantages:
// 1. The object iteration order is mostly likely not in order of borrowing.
// 2. The object returned to the user with borrow might be invalid after a return.
//
// However, the advantage is that the borrowed objects are contiguous
//
// TODO: Create an API for iteration?
//

#ifndef MM_UNORDERED_LIST_H
#define MM_UNORDERED_LIST_H

#include "mm_core.h"

template<typename T>
struct mmul_list {
    T* Objects;     // Expects pointer to object
    usize Used;     // Amount of objects currently borrowed
    usize Capacity; // Total number of borrowable objects.
};

template<typename T>
struct mmul_it {
    mmul_list<T>* List;
    usize Index;

    T* operator->() {
        return &List->Objects[Index];
    }
};


// Iterators
template<typename T>
static inline mmul_it<T>
mmul_Begin(mmul_list<T>* List) {
    return { List, 0 };
}
    
template<typename T>
static inline mmul_it<T>
mmul_End(mmul_list<T>* List) {
    return { List, List->Used };
}

template<typename T>
static inline b32
operator!=(mmul_it<T> L, mmul_it<T> R) {
    return L.Index != R.Index;
}

template<typename T>
static inline mmul_it<T>&
operator++(mmul_it<T>& L) {
    ++L.Index;
    return L;
}

template<typename T>
static inline T& 
operator*(mmul_it<T>& L) {
    return L.List->Objects[L.Index];
}

// Functions
template<typename T>
static inline void 
mmul_Clear(mmul_list<T>* List) {
    List->Used = 0;
}


template<typename T>
static inline mmul_list<T>
mmul_List(T* Arr, usize Capacity) {
    mmul_list<T> Ret = {};
    Ret.Objects = Arr;
    Ret.Capacity= Capacity;
    return Ret;
}

template<typename T>
static inline void
mmul_Add(mmul_list<T>* List, T Obj) {
    Assert(List->Used < List->Capacity);
    List->Objects[List->Used++] = Obj;
}

// This might actually be faster than the one above?
template<typename T>
static inline mmul_it<T>
mmul_Remove(mmul_list<T>* List, mmul_it<T> It) {
    //(*Obj) = List->Objects[List->Used - 1];
    List->Objects[It.Index] = List->Objects[List->Used - 1];
    --List->Used;
    return It; //lol?
}

// C++11 range-base for loop support
template<typename T>
static inline mmul_it<T>
begin(mmul_list<T>& List) {
    return mmul_Begin(&List);
}
    
template<typename T>
static inline mmul_it<T>
end(mmul_list<T>& List) {
    return mmul_End(&List);
}

#include "mm_arena.h"
template<typename T>
static inline mmul_list<T>
mmul_PushList(mmarn_arena* Arena, usize Capacity) {
    mmul_list<T> Ret = {};
    Ret.Objects = mmarn_PushArray<T>(Arena, Capacity);
    Ret.Capacity = Capacity;
    return Ret;
}




#endif // MM_POOL_H
