//
// A container for arrays, with bounds checking assert.
// The point of this, I guess, is the option to store the Count on the stack
// and the Elements on the heap.
//
#ifndef MOMO_ARRAY
#define MOMO_ARRAY

#include "mm_core.h"
#include "mm_arena.h"


#define boot_array(name, T, count) T AnonVar(__LINE__)[count] = {}; Array<T> name = array(AnonVar(__LINE__), count)

template<typename T>
struct Array {
    usize count;
    T* elements;

    b8 is_empty() const;
    void reverse();
    
    template<typename Unary_Compare>
    usize find(Unary_Compare unary_compare, usize start_index = 0) const; 
    usize find(T item, usize start_index = 0) const;

    b8 operator==(const Array<T>& rhs);
    auto& operator[](usize index);
    const auto& operator[](usize index) const;

    static Array<T> create(T* elements, usize count);
    static Array<T> create(Memory_Arena* arena, usize count);
    static Array<T> create(Array<T> src, Range<usize> slice); 
};

template<typename T>
auto& Array<T>::operator[](usize index) {
    Assert(index < this->count);
    return this->elements[index];
}

template<typename T>
const auto& Array<T>::operator[](usize index) const {
    Assert(index < this->count);
    return this->elements[index];
}


template<typename T>
Array<T> Array<T>::create(T* elements, usize count) {
    Array<T> ret = {};
    ret.elements = elements;
    ret.count = count;
    return ret;
}

template<typename T>
Array<T> Array<T>::create(Memory_Arena* arena, usize count) {
    T* buffer = PushSiArray<T>(arena, count);
    return Array<T>::create(buffer, count);
}

template<typename T>
Array<T> Array<T>::create(Array<T> src, Range<usize> slice) {
    Assert(slice.start <= slice.end);
    return Array(src.elements + slice.start, slice.end - slice.start);
};


template<typename T>
b8 Array<T>::is_empty() const {
    return array.count == 0;
}

template<typename T>
void Array<T>::reverse() {
    for (usize i = 0; i < dest->count/2; ++i) {
        Swap(dest->elements[i], dest->elements[dest->count-1-i]);
    }
}

template <typename T>
template <typename Unary_Compare>
usize Array<T>::find(Unary_Compare unary_compare, usize start_index) const 
{
    for (usize i = start_index; I < this->count; ++I) {
        if (unary_compare(array.elements + I)) {
            return I;
        }
    }
    return array.count;
}

template<typename T>
usize Array<T>::find(T item, usize start_index) const 
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


