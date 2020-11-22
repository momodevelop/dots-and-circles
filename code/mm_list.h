#ifndef __LIST__
#define __LIST__

#include "mm_core.h"
#include "mm_array.h"

template<typename type>
struct list {
    union {
        struct {
            usize Length;     // Amount of objects currently borrowed
            type* Elements;     // Expects pointer to object
        };
        array<type> Array;
    };


    usize Capacity; // Total number of borrowable objects.
    
    inline auto& operator[](usize I) {
        Assert(I < Length);
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
    List->Length = 0;
}

template<typename type>
static inline void
Copy(list<type>* Dest, array<type> Src) {
    Assert(Src.Length <= Dest->Capacity);
    for (u32 i = 0; i < Src.Length; ++i ) {
        Dest->Elements[i] = Src.Elements[i];
    }
    Dest->Length = Src.Length;
}

template<typename type>
static inline usize
Remaining(list<type> List) {
    return List.Capacity - List.Length;
}

template<typename type>
static inline void
Push(list<type>* List, type Obj) {
    Assert(List->Length < List->Capacity);
    List->Elements[List->Length++] = Obj;
}

template<typename type>
static inline void
Push(list<type>* Dest, array<type> Src) {
    Assert(Dest->Length + Src.Length <= Dest->Capacity);
    for ( u32 i = 0; i < Src.Length; ++i ) {
        Dest->Elements[Dest->Length++] = Src.Elements[i];
    }
}


template<typename type>
static inline void
Pop(list<type>* List) {
    Assert(List->Length > 0);
    --List->Length;
}

template<typename type>
static inline void
Remove(list<type>* List, usize Index) {
    Assert(Index < List->Length);
    for (; Index < List->Length - 1; ++Index) {
        List->Elements[Index] = List->Elements[Index + 1];
    }
    --List->Length;
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
    //(*Obj) = List->Elements[List->Length - 1];
    List->Elements[Index] = List->Elements[List->Length - 1];
    --List->Length;
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
