#ifndef MOMO_ARRAY_Char
#define MOMO_ARRAY_Char

#include "mm_core.h"
#include "mm_arena.h"

// A container for arrays, with bounds checking assert.
// The point of this, I guess, is the option to store the Count on the stack
// and the Elements on the heap.
//

#define BootArrayChar(name, count) char AnonVar(__LINE__)[count] = {}; array_char name = ArrayChar(AnonVar(__LINE__), count)

struct array_char {
    usize Count;
    char* Elements;

    inline auto& operator[](usize I) {
        Assert(I < Count);
        return Elements[I];
    }
};


static inline array_char
ArrayChar(char* Elements, usize Count) {
    array_char Ret = {};
    Ret.Elements = Elements;
    Ret.Count = Count;

    return Ret;
}

static inline array_char
ArrayChar(arena* Arena, usize Count) {
    char* Buffer = PushSiArray<char>(Arena, Count);
    return ArrayChar(Buffer, Count);
    
}

static inline array_char
SubArray(array_char Src, range<usize> Slice) {
    Assert(Slice.Start <= Slice.End);
    return ArrayChar(Src.Elements + Slice.Start, Slice.End - Slice.Start);
};


static inline void
Reverse(array_char* Dest) {
    for (usize i = 0; i < Dest->Count/2; ++i) {
        Swap(Dest->Elements[i], Dest->Elements[Dest->Count-1-i]);
    }
}


template<typename unary_comparer>
static inline usize
Find(array_char* Array, 
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
Find(array_char* Array, 
     char Item, 
     usize StartIndex = 0) 
{
    return Find(Array, [Item](char* It) {
        return (*It) == Item;       
    }, StartIndex);
}


static inline char*
operator+(array_char L, usize I) {
    return L.Elements + I;
}

static inline b32
operator==(array_char Lhs, array_char Rhs) { 
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
 