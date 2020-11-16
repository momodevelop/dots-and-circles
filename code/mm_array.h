#ifndef MOMO_ARRAY
#define MOMO_ARRAY

#include "mm_core.h"

// A container for arrays, with bounds checking assert.
// The point of this, I guess, is the option to store the Capacity on the stack
// and the Elements on the heap.
//

template<typename type>
struct mma_array {
    usize Capacity;
    type* Elements;

    inline auto& operator[](usize I) {
        Assert(I < Capacity);
        return Elements[I];
    }

    inline const auto& operator[](usize I) const {
        Assert(I < Capacity);
        return Elements[I];
    }
};

template<typename type>
static inline mma_array<type>
mma_Array(type* Elements, usize Capacity) {
    mma_array<type> Ret = {};
    Ret.Elements = Elements;
    Ret.Capacity = Capacity;

    return Ret;
}

#include "mm_arena.h"
template<typename type>
static inline mma_array<type> 
mma_Array(mmarn_arena* Arena, usize Capacity) {
    type* Buffer = mmarn_PushArray<type>(Arena, Capacity);
    return mma_Array(Buffer, Capacity);
    
}

template<typename type>
static inline mma_array<type>*
mma_PushArray(mmarn_arena* Arena, usize Capacity) {
    return mmarn_PushConstruct(Arena, mma_Array, Capacity);
}

template<typename type>
static inline type*
operator+(const mma_array<type>& L, usize I) {
    return L.Elements + I;
}

#endif


