#ifndef MOMO_ARRAY
#define MOMO_ARRAY

#include "mm_core.h"
#include "mm_arena.h"
// A container for arrays, with bounds checking assert.
// The point of this, I guess, is the option to store the Count on the stack
// and the Elements on the heap.
//


#define boot_array(name, T, count) T AnonVar(__LINE__)[count] = {}; Array<T> name = array(AnonVar(__LINE__), count)

template<typename T>
struct Array {
    usize count;
    T* elements;

    inline auto& operator[](usize index) {
        Assert(index < count);
        return elements[index];
    }
};


template<typename T>
static inline Array<T>
array(T* elements, usize count) {
    Array<T> ret = {};
    ret.elements = elements;
    ret.count = count;
    return ret;
}

template<typename T>
static inline Array<T> 
array(Memory_Arena* arena, usize count) {
    T* buffer = PushSiArray<T>(arena, count);
    return array(buffer, count);
    
}

template<typename T>
static inline Array<T>
sub_array(Array<T> src, Range<usize> slice) {
    Assert(slice.start <= slice.end);
    return Array(src.elements + slice.start, slice.end - slice.start);
};


template<typename T>
static inline b32
is_empty(Array<T> array) {
    return array.count == 0;
}

template<typename T>
static inline void
reverse(Array<T>* dest) {
    for (usize i = 0; i < dest->count/2; ++i) {
        Swap(dest->elements[i], dest->elements[dest->count-1-i]);
    }
}


template<typename T, typename Unary_Compare>
static inline usize
find(Array<T> array, 
     Unary_Compare unary_compare, 
     usize StartIndex = 0) 
{
    for(usize I = StartIndex; I < array.count; ++I) {
        if(unary_compare(array.elements + I)) {
            return I;
        }
    }
    return array.count;
}


template<typename T>
static inline usize
find(Array<T> array, 
     T item, 
     usize start_index = 0) 
{
    return find(array, [item](T* It) {
        return (*It) == item;       
    }, start_index);
}

template<typename T>
static inline T*
operator+(Array<T> lhs, usize index) {
    return lhs.elements + index;
}

template<typename T>
static inline b32
operator==(Array<T> lhs, Array<T> rhs) { 
    if(lhs.count != rhs.count) {
        return false;
    }
    for (u32 i = 0; i < lhs.count; ++i) {
        if (lhs.elements[i] != rhs.elements[i]) {
            return false;
        }
    }
    return true;
}

#endif


