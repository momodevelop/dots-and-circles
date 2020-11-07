#ifndef __LIST__
#define __LIST__

// This is an ordered list, means removal is O(N);

#include "mm_core.h"

template<typename T>
struct mml_list {
    T* Objects;     // Expects pointer to object
    usize Used;     // Amount of objects currently borrowed
    usize Capacity; // Total number of borrowable objects.
};

template<typename T>
struct mml_it {
    mml_list<T>* List;
    usize Index;

    T* operator->() {
        return &List->Objects[Index];
    }
};


template<typename T>
using mml_unary_compare_cb = b32 (*)(const T*);

// Iterators
template<typename T>
static inline mml_it<T>
mml_Begin(mml_list<T>* List) {
    return { List, 0 };
}
    
template<typename T>
static inline mml_it<T>
mml_End(mml_list<T>* List) {
    return { List, List->Used };
}

template<typename T>
static inline b32
operator!=(mml_it<T> L, mml_it<T> R) {
    return L.Index != R.Index;
}

template<typename T>
static inline mml_it<T>&
operator++(mml_it<T>& L) {
    ++L.Index;
    return L;
}

template<typename T>
static inline T& 
operator*(mml_it<T>& L) {
    return L.List->Objects[L.Index];
}

// Functions
template<typename T>
static inline void 
mml_Clear(mml_list<T>* List) {
    List->Used = 0;
}


template<typename T>
static inline mml_list<T>
mml_List(T* Arr, usize Capacity) {
    mml_list<T> Ret = {};
    Ret.Objects = Arr;
    Ret.Capacity= Capacity;
    return Ret;
}

template<typename T>
static inline void
mml_Push(mml_list<T>* List, T Obj) {
    Assert(List->Used < List->Capacity);
    List->Objects[List->Used++] = Obj;
}

template<typename T>
static inline void
mml_Pop(mml_list<T>* List) {
    Assert(List->Used > 0);
    --List->Used;
}

template<typename T>
static inline mml_it<T>
mml_Find(mml_list<T>* List, mml_unary_compare_cb<T> UnaryComparer) {
    for(usize i = 0; i < List->Used; ++i) {
        if(UnaryComparer(&List->Objects[i])) {
            return { List, i };
        }
    }
    return mml_End(List);
}

template<typename T>
static inline mml_it<T>
mml_Remove(mml_list<T>* List, mml_it<T> It) {
    for (usize Index = It.Index; Index < List->Used - 1; ++Index) {
        List->Objects[Index] = List->Objects[Index + 1];
    }
    --List->Used;
    return It; 
}

template<typename T> 
static inline mml_it<T>
mml_RemoveIf(mml_list<T>* List, mml_unary_compare_cb<T> UnaryComparer) {
    return mml_Remove(List, mml_Find(List, UnaryComparer));
}


// C++11 range-base for loop support
template<typename T>
static inline mml_it<T>
begin(mml_list<T>& List) {
    return mml_Begin(&List);
}
    
template<typename T>
static inline mml_it<T>
end(mml_list<T>& List) {
    return mml_End(&List);
}

#include "mm_arena.h"
template<typename T>
static inline mml_list<T>
mml_PushList(mmarn_arena* Arena, usize Capacity) {
    mml_list<T> Ret = {};
    Ret.Objects = mmarn_PushArray<T>(Arena, Capacity);
    Ret.Capacity = Capacity;
    return Ret;
}

#endif
