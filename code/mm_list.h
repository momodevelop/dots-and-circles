#ifndef __MOMO_LIST__
#define __MOMO_LIST__

#include "mm_array.h"

template<typename T>
struct List {
    union {
        struct {
            usize count; // Amount of objects currently borrowed
            T* elements;   // Expects pointer to object
        };
        Array<T> array;
    };


    usize capacity; // Total number of borrowable objects.
    
    inline T& operator[](usize index) {
        Assert(index < count);
        return elements[index];
    }
};

#define boot_list(name, T, count) T AnonVar(__LINE__)[count] = {}; list<T> name = List(AnonVar(__LINE__), count)

// Constructors
template<typename T>
static inline List<T>
list(T* arr, usize capacity) {
    List<T> ret = {};
    ret.elements = arr;
    ret.capacity = capacity;
    return ret;
}

template<typename T>
static inline List<T>
list(Memory_Arena* arena, usize capacity) {
    List<T> ret = {};
    ret.elements = PushSiArray<T>(arena, capacity);
    ret.capacity = capacity;
    return ret;
}

// Functions
template<typename T>
static inline void 
clear(List<T>* list) {
    list->Count = 0;
}

template<typename T>
static inline void
copy(List<T>* dest, Array<T> src) {
    Assert(src.count <= dest->capacity);
    for (u32 i = 0; i < src.count; ++i ) {
        dest->elements[i] = src.elements[i];
    }
    dest->count = src.count;
}

template<typename T>
static inline usize
remaining(List<T> list) {
    return list.capacity - list.count;
}

template<typename T>
static inline void
push(List<T>* list, T obj) {
    Assert(list->count < list->capacity);
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
