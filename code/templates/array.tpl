#ifndef MOMO_ARRAY_$1
#define MOMO_ARRAY_$1

#include "mm_core.h"
#include "mm_arena.h"

// A container for arrays, with bounds checking assert.
// The point of this, I guess, is the option to store the Count on the stack
// and the Elements on the heap.
//

#define BootArray$1(name, count) $0 AnonVar(__LINE__)[count] = {}; array_$0 name = Array$1(AnonVar(__LINE__), count)

struct array_$0 {
    usize Count;
    $0* Elements;

    inline auto& operator[](usize I) {
        Assert(I < Count);
        return Elements[I];
    }
};


static inline array_$0
Array$1($0* Elements, usize Count) {
    array_$0 Ret = {};
    Ret.Elements = Elements;
    Ret.Count = Count;

    return Ret;
}

static inline array_$0
Array$1(arena* Arena, usize Count) {
    $0* Buffer = PushSiArray<$0>(Arena, Count);
    return Array$1(Buffer, Count);
    
}

static inline array_$0
SubArray(array_$0 Src, range<usize> Slice) {
    Assert(Slice.Start <= Slice.End);
    return Array$1(Src.Elements + Slice.Start, Slice.End - Slice.Start);
};


static inline void
Reverse(array_$0* Dest) {
    for (usize i = 0; i < Dest->Count/2; ++i) {
        Swap(Dest->Elements[i], Dest->Elements[Dest->Count-1-i]);
    }
}


template<typename unary_comparer>
static inline usize
Find(array_$0* Array, 
     unary_comparer UnaryCompare, 
     usize StartIndex = 0) 
{
    for(usize I = StartIndex; I < Array->Count; ++I) {
        if(UnaryCompare(Array->Elements + I)) {
            return I;
        }
    }
    return Array->Count;
}


static inline usize
Find(array_$0* Array, 
     $0 Item, 
     usize StartIndex = 0) 
{
    return Find(Array, [Item]($0* It) {
        return (*It) == Item;       
    }, StartIndex);
}


static inline $0*
operator+(array_$0 L, usize I) {
    return L.Elements + I;
}

static inline b32
operator==(array_$0 Lhs, array_$0 Rhs) { 
    if(Lhs.Count != Rhs.Count) {
        return false;
    }
    for (usize i = 0; i < Lhs.Count; ++i) {
        if (Lhs.Elements[i] != Rhs.Elements[i]) {
            return false;
        }
    }
    return true;
}

#endif
