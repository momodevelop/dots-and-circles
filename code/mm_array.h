//
// A container for arrays, with bounds checking assert.
// The point of this, I guess, is the option to store the Count on the stack
// and the Elements on the heap.
//
#ifndef MOMO_ARRAY
#define MOMO_ARRAY

#include "mm_core.h"
#include "mm_arena.h"

#define boot_array(name, T, count) T AnonVar(__LINE__)[count] = {}; Array<T> name = create_array(AnonVar(__LINE__), count)

template<typename T>
struct Array {
    usize length;
    T* elements;

    inline T& operator[](usize index) {
        Assert(index < length);
        return elements[index];
    }

    inline const T& operator[](usize index) const {
        Assert(index < length);
        return elements[index];
    }
};

template<typename T>
static inline Array<T> 
create_array(T* elements, usize length) {
    Array<T> ret = {};
    ret.elements = elements;
    ret.length = length;
    return ret;
}

template<typename T>
static inline Array<T> 
create_array(Memory_Arena* arena, usize length) {
    T* buffer = PushSiArray<T>(arena, length);
    return create_array(buffer, length);
}

template<typename T>
static inline Array<T> 
create_array(Array<T>* src, Range<usize> slice) {
    Assert(slice.start <= slice.end);
    return create_array(src->elements + slice.start, slice.end - slice.start);
};


template<typename T>
static inline b8 
is_empty(const Array<T>* arr) {
    return arr->count == 0;
}

template<typename T>
static inline void 
reverse(Array<T>* arr) {
    for (usize i = 0; i < arr->length/2; ++i) {
        Swap(arr->elements[i], arr->elements[arr->length-1-i]);
    }
}

template<typename T, typename Unary_Compare>
static inline usize
find(const Array<T>* arr, 
     Unary_Compare unary_compare, 
     usize start_index) 
{
    for (usize i = start_index; i < arr->length; ++i) {
        if (unary_compare(arr->elements + i)) {
            return i;
        }
    }
    return arr->length;
}

template<typename T>
static inline usize 
find(const Array<T>* arr, T item, usize start_index) {
    return find(arr, [item](T* It) {
        return (*It) == item;       
    }, start_index);
}

template<typename T>
static inline T*
operator+(Array<T>* lhs, usize index) {
    return lhs->elements + index;
}

template<typename T>
static inline b8
operator==(const Array<T>* lhs, const Array<T>* rhs) { 
    if(lhs->length != rhs->length) {
        return false;
    }
    for (u32 i = 0; i < lhs->length; ++i) {
        if (lhs->elements[i] != rhs->elements[i]) {
            return false;
        }
    }
    return true;
}

#endif


