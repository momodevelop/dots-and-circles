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
    usize count;
    T* elements;

    inline auto& operator[](usize index) {
        Assert(index < count);
        return elements[index];
    }
};

template<typename T>
Array<T> create_array(T* elements, usize count) {
    Array<T> ret = {};
    ret.elements = elements;
    ret.count = count;
    return ret;
}

template<typename T>
Array<T> create_array(Memory_Arena* arena, usize count) {
    T* buffer = PushSiArray<T>(arena, count);
    return create_array(buffer, count);
}

template<typename T>
Array<T> create_array(Array<T>* src, Range<usize> slice) {
    Assert(slice.start <= slice.end);
    return create_array(src->elements + slice.start, slice.end - slice.start);
};


template<typename T>
b8 is_empty(Array<T>* arr) {
    return arr->count == 0;
}

template<typename T>
void reverse(Array<T>* arr) {
    for (usize i = 0; i < arr->count/2; ++i) {
        Swap(arr->elements[i], arr->elements[arr->count-1-i]);
    }
}

template<typename T, typename Unary_Compare>
usize find(Array<T> arr, Unary_Compare unary_compare, usize start_index) 
{
    for (usize i = start_index; i < arr.count; ++i) {
        if (unary_compare(array.elements + i)) {
            return i;
        }
    }
    return array.count;
}

template<typename T>
usize find(T item, usize start_index) {
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


