#ifndef MOMO_ARRAY
#define MOMO_ARRAY

#include "mm_core.h"

// A container for arrays, with bounds checking assert.
// The point of this, I guess, is the option to store the Capacity on the stack
// and the Elements on the heap.
//

template<typename type>
struct array {
    usize Capacity;
    type* Elements;

    inline auto& operator[](usize I) {
        Assert(I < Capacity);
        return Elements[I];
    }
};

template<typename type>
static inline array<type>
Array(type* Elements, usize Capacity) {
    array<type> Ret = {};
    Ret.Elements = Elements;
    Ret.Capacity = Capacity;

    return Ret;
}


template<typename type>
static inline type*
operator+(array<type> L, usize I) {
    return L.Elements + I;
}

#include "mm_arena.h"
template<typename type>
static inline array<type> 
Array(arena* Arena, usize Capacity) {
    type* Buffer = PushSiArray<type>(Arena, Capacity);
    return Array(Buffer, Capacity);
    
}

#endif


