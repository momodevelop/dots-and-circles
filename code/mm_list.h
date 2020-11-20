#ifndef __LIST__
#define __LIST__

#include "mm_core.h"

template<typename T>
struct list {
    T* Elements;     // Expects pointer to object
    usize Count;     // Amount of objects currently borrowed
    usize Capacity; // Total number of borrowable objects.
    
    inline auto& operator[](usize I) {
        Assert(I < Count);
        return Elements[I];
    }

};

template<typename T>
struct list_it {
    list<T>* List;
    usize Index;

    T* operator->() {
        return &List->Elements[Index];
    }
};



// Iterators
template<typename T>
static inline list_it<T>
Begin(list<T>* List) {
    return { List, 0 };
}
    
template<typename T>
static inline list_it<T>
End(list<T>* List) {
    return { List, List->Count };
}

template<typename T>
static inline b32
operator!=(list_it<T> L, list_it<T> R) {
    return L.Index != R.Index;
}

template<typename T>
static inline list_it<T>&
operator++(list_it<T>& L) {
    ++L.Index;
    return L;
}

template<typename T>
static inline T& 
operator*(list_it<T> L) {
    return L.List->Elements[L.Index];
}

// Functions
template<typename T>
static inline void 
Clear(list<T>* List) {
    List->Count = 0;
}


template<typename T>
static inline list<T>
List(T* Arr, usize Capacity) {
    list<T> Ret = {};
    Ret.Elements = Arr;
    Ret.Capacity= Capacity;
    return Ret;
}

template<typename T>
static inline void
Push(list<T>* List, T Obj) {
    Assert(List->Count < List->Capacity);
    List->Elements[List->Count++] = Obj;
}

template<typename T>
static inline void
Pop(list<T>* List) {
    Assert(List->Count > 0);
    --List->Count;
}

template<typename T, typename unary_comparer>
static inline list_it<T>
Find(list<T>* List, unary_comparer UnaryComparer) {
    for(usize i = 0; i < List->Count; ++i) {
        if(UnaryComparer(&List->Elements[i])) {
            return { List, i };
        }
    }
    return End(List);
}

template<typename T>
static inline list_it<T>
Remove(list<T>* List, list_it<T> It) {
    for (usize Index = It.Index; Index < List->Count - 1; ++Index) {
        List->Elements[Index] = List->Elements[Index + 1];
    }
    --List->Count;
    return It; 
}

template<typename T, typename unary_comparer> 
static inline list_it<T>
RemoveIf(list<T>* List, unary_comparer UnaryComparer) {
    return Remove(List, Find(List, UnaryComparer));
}

// Faster version of Remove by swapping the last element into the current element
// This is O(1), but usually messes up the order of the list. 
template<typename T>
static inline list_it<T>
SwapRemove(list<T>* List, list_it<T> It) {
    //(*Obj) = List->Elements[List->Count - 1];
    List->Elements[It.Index] = List->Elements[List->Count - 1];
    --List->Count;
    return It; //lol?
}

// C++11 range-base for loop support
template<typename T>
static inline list_it<T>
begin(list<T>& List) {
    return Begin(&List);
}
    
template<typename T>
static inline list_it<T>
end(list<T>& List) {
    return End(&List);
}

#include "mm_arena.h"
template<typename T>
static inline list<T>
List(arena* Arena, usize Capacity) {
    list<T> Ret = {};
    Ret.Elements = PushSiArray<T>(Arena, Capacity);
    Ret.Capacity = Capacity;
    return Ret;
}

#endif
