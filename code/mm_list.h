#ifndef __LIST__
#define __LIST__

#include "mm_core.h"
#include "mm_array.h"

template<typename type>
struct list {
    union {
        struct {
            usize Count;     // Amount of objects currently borrowed
            type* Elements;     // Expects pointer to object
        };
        array<type> Array;
    };


    usize Capacity; // Total number of borrowable objects.
    
    inline type& operator[](usize I) {
        Assert(I < Count);
        return Elements[I];
    }
};

// Constructors
template<typename type>
static inline list<type>
List(type* Arr, usize Capacity) {
    list<type> Ret = {};
    Ret.Elements = Arr;
    Ret.Capacity= Capacity;
    return Ret;
}

// Functions
template<typename type>
static inline void 
Clear(list<type>* List) {
    List->Count = 0;
}

template<typename type>
static inline void
Copy(list<type>* Dest, array<type> Src) {
    Assert(Src.Count <= Dest->Capacity);
    for (u32 i = 0; i < Src.Count; ++i ) {
        Dest->Elements[i] = Src.Elements[i];
    }
    Dest->Count = Src.Count;
}

template<typename type>
static inline usize
Remaining(list<type> List) {
    return List.Capacity - List.Count;
}

template<typename type>
static inline void
Push(list<type>* List, type Obj) {
    Assert(List->Count < List->Capacity);
    List->Elements[List->Count++] = Obj;
}

template<typename type>
static inline void
Push(list<type>* Dest, array<type> Src) {
    Assert(Dest->Count + Src.Count <= Dest->Capacity);
    for ( u32 i = 0; i < Src.Count; ++i ) {
        Dest->Elements[Dest->Count++] = Src.Elements[i];
    }
}


template<typename type>
static inline void
Pop(list<type>* List) {
    Assert(List->Count > 0);
    --List->Count;
}

template<typename type>
static inline type&
Front(list<type>* List) {
    Assert(List->Count > 0);
    return List->Elements[0];
}

template<typename type>
static inline type&
Back(list<type>* List) {
    Assert(List->Count > 0);
    return List->Elements[List->Count-1];
}

template<typename type>
static inline void
Remove(list<type>* List, usize Index) {
    Assert(Index < List->Count);
    for (; Index < List->Count - 1; ++Index) {
        List->Elements[Index] = List->Elements[Index + 1];
    }
    --List->Count;
}

template<typename type, typename unary_comparer> 
static inline usize
RemoveIf(list<type>* List, unary_comparer UnaryComparer) {
    Remove(List, Find(List->Array, UnaryComparer));
}

// Faster version of Remove by swapping the last element into the current element
// This is O(1), but usually messes up the order of the list. 
template<typename type>
static inline usize
SwapRemove(list<type>* List, usize Index) {
    //(*Obj) = List->Elements[List->Count - 1];
    List->Elements[Index] = List->Elements[List->Count - 1];
    --List->Count;
    return Index;
}

// Specialized struct and functions for ascii string
template<typename type>
static inline type*
operator+(list<type> L, usize I) {
    return L.Elements + I;
}

#include "mm_arena.h"
template<typename type>
static inline list<type>
List(arena* Arena, usize Capacity) {
    list<type> Ret = {};
    Ret.Elements = PushSiArray<type>(Arena, Capacity);
    Ret.Capacity = Capacity;
    return Ret;
}

#endif
