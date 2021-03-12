#ifndef MOMO_ARRAY
#define MOMO_ARRAY

// A container for arrays, with bounds checking assert.
// The point of this, I guess, is the option to store the Count on the stack
// and the Elements on the heap.
//
#define BootstrapArray(name, type, count) type AnonVar(__LINE__)[count] = {}; array<type> name = Array_Create(AnonVar(__LINE__), count)

template<typename type>
struct array {
    usize Count;
    type* Elements;
    
    inline auto& operator[](usize I) {
        Assert(I < Count);
        return Elements[I];
    }
};


template<typename type>
static inline array<type>
Array_Create(type* Elements, usize Count) {
    array<type> Ret = {};
    Ret.Elements = Elements;
    Ret.Count = Count;
    
    return Ret;
}

template<typename type>
static inline array<type> 
Array_Create(arena* Arena, u32 Count) {
    type* Buffer = Arena_PushArray(type, Arena, Count);
    return Array_Create(Buffer, Count);
    
}

template<typename type>
static inline array<type>
Array_SubArray(array<type> Src, range<usize> Slice) {
    Assert(Slice.Start <= Slice.End);
    return Array_Create(Src.Elements + Slice.Start, Slice.End - Slice.Start);
};


template<typename type>
static inline void
Reverse(array<type>* Dest) {
    for (usize i = 0; i < Dest->Count/2; ++i) {
        Swap(Dest->Elements[i], Dest->Elements[Dest->Count-1-i]);
    }
}


template<typename type, typename unary_comparer>
static inline usize
Find(array<type>* Array, 
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


template<typename type>
static inline usize
Find(array<type>* Array, 
     type Item, 
     usize StartIndex = 0) 
{
    return Find(Array, [Item](type* It) {
                    return (*It) == Item;       
                }, StartIndex);
}

template<typename type>
static inline type*
operator+(array<type> L, usize I) {
    return L.Elements + I;
}

template<typename type>
static inline b32
operator==(array<type> Lhs, array<type> Rhs) { 
    if(Lhs.Count != Rhs.Count) {
        return false;
    }
    for (u32 i = 0; i < Lhs.Count; ++i) {
        if (Lhs.Elements[i] != Rhs.Elements[i]) {
            return false;
        }
    }
    return true;
}

#endif


