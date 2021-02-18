#ifndef __MOMO_LIST__
#define __MOMO_LIST__

#include "mm_array.h"

template<typename T>
struct List {
    Array<T> array;
    usize count; 
   
    inline T& operator[](usize index) {
        return array[index];
    }

    inline const T& operator[](usize index) const {
        return array[index];
    }
};

#define boot_list(name, T, count) T AnonVar(__LINE__)[count] = {}; list<T> name = create_list(AnonVar(__LINE__), count)

// Constructors
template<typename T>
static inline List<T>
create_list(T* arr, usize capacity) {
    List<T> ret = {};
    ret.array = create_array(arr, capacity);
    return ret;
}

template<typename T>
static inline List<T>
create_list(Memory_Arena* arena, usize capacity) {
    List<T> ret = {};
    ret.array = create_array(arena, capacity);
    return ret;
}

// Functions
template<typename T>
static inline void 
clear(List<T>* l) {
    l->count = 0;
}

template<typename T>
static inline void
copy(List<T>* l, const Array<T>* src) {
    Assert(src->count <= l->capacity);
    for (u32 i = 0; i < src->count; ++i ) {
        l->elements[i] = src->elements[i];
    }
    l->count = src->count;
}

template<typename T>
static inline usize
remaining(const List<T>* l) {
    return l->array.length - l->count;
}

template<typename T>
static inline void
push(List<T>* list, T obj) {
    Assert(list->count < list->array.capacity);
    list->elements[list->count++] = obj;
}

template<typename T>
static inline void
push(List<T>* dest, Array<T> src) {
    Assert(dest->count + src.count <= dest->capacity);
    for (u32 i = 0; i < src.count; ++i ) {
        dest->elements[dest->count++] = src.elements[i];
    }
}


template<typename T>
static inline void
pop(List<T>* list) {
    Assert(list->count > 0);
    --list->count;
}

template<typename T>
static inline T&
front(List<T>* list) {
    Assert(list->count > 0);
    return list->elements[0];
}

template<typename T>
static inline T&
back(List<T>* list) {
    Assert(list->count > 0);
    return list->elements[list->count-1];
}

template<typename T>
static inline void
remove(List<T>* list, usize index) {
    Assert(index < list->count);
    for (; index < list->count - 1; ++index) {
        list->elements[index] = list->elements[index + 1];
    }
    --list->count;
}

template<typename T, typename Unary_Compare> 
static inline usize
remove_if(List<T>* list, Unary_Compare unary_compare) {
    usize Index = find(list->array, unary_compare);
    Remove(list, Index);
    return Index;
}

// Faster version of Remove by swapping the last element into the current element
// This is O(1), but usually messes up the order of the list. 
template<typename T>
static inline usize
swap_remove(List<T>* list, usize Index) {
    //(*Obj) = List->Elements[List->Count - 1];
    list->elements[Index] = list->elements[list->count - 1];
    --list->count;
    return Index;
}

// Specialized struct and functions for ascii string
template<typename T>
static inline T*
operator+(List<T> lhs, usize index) {
    return lhs.index + index;
}


#endif
