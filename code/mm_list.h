#ifndef __LIST__
#define __LIST__

// This is an ordered list, means removal is O(N);

#include "mm_core.h"

template<typename T>
struct mml_list {
    T* Elements;     // Expects pointer to object
    usize Used;     // Amount of objects currently borrowed
    usize Capacity; // Total number of borrowable objects.
    
    inline auto& operator[](usize I) {
        Assert(I < Used);
        return Elements[I];
    }

    inline const auto& operator[](usize I) const {
        Assert(I < Used);
        return Elements[I];
    }

};

template<typename T>
struct mml_it {
    mml_list<T>* List;
    usize Index;

    T* operator->() {
        return &List->Elements[Index];
    }
};



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
    return L.List->Elements[L.Index];
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
    Ret.Elements = Arr;
    Ret.Capacity= Capacity;
    return Ret;
}

template<typename T>
static inline void
mml_Push(mml_list<T>* List, T Obj) {
    Assert(List->Used < List->Capacity);
    List->Elements[List->Used++] = Obj;
}

template<typename T>
static inline void
mml_Pop(mml_list<T>* List) {
    Assert(List->Used > 0);
    --List->Used;
}

template<typename T, typename unary_comparer>
static inline mml_it<T>
mml_Find(mml_list<T>* List, unary_comparer UnaryComparer) {
    for(usize i = 0; i < List->Used; ++i) {
        if(UnaryComparer(&List->Elements[i])) {
            return { List, i };
        }
    }
    return mml_End(List);
}

template<typename T>
static inline mml_it<T>
mml_Remove(mml_list<T>* List, mml_it<T> It) {
    for (usize Index = It.Index; Index < List->Used - 1; ++Index) {
        List->Elements[Index] = List->Elements[Index + 1];
    }
    --List->Used;
    return It; 
}

template<typename T, typename unary_comparer> 
static inline mml_it<T>
mml_RemoveIf(mml_list<T>* List, unary_comparer UnaryComparer) {
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
mml_List(mmarn_arena* Arena, usize Capacity) {
    mml_list<T> Ret = {};
    Ret.Elements = mmarn_PushArray<T>(Arena, Capacity);
    Ret.Capacity = Capacity;
    return Ret;
}

#endif
