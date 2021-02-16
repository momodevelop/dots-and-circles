#ifndef __MM_POOL__
#define __MM_POOL__

#include "mm_core.h"
#include "mm_list.h"
#include "mm_array.h"
#include "mm_arena.h"

template<typename T>
struct Pool {
    Array<T> slots;
    List<usize> free_list;

    static Pool create(Memory_Arena* arena, usize cap);
    T* get(usize id);
    usize remainder() const;
};

template<typename T>
Pool<T> Pool<T>::create(Memory_Arena* arena, usize cap) {
    Pool<T> Ret = {};
    Ret.slots = Array<T>(arena, cap); 
    Ret.free_list = list<usize>(arena, cap);
    for (usize I = 0; I < cap; ++I) {
        Push(&Ret.free_list, I);
    }
    return Ret;
}

template<typename T>
T* Pool<T>::get(usize Id) {
    return &this->slots[Id]; 
}

// For use with smaller types
template<typename T>
static inline T
GetCopy(Pool<T>* Pool, usize Id) {
    return Pool->slots[Id]; 
}

template<typename T>
usize Pool<T>::remainder() const {
    return this->free_list.count;
}

// Analogous to 'booking a reservation'
template<typename t>
static inline usize
Reserve(Pool<t>* Pool, t Construct = {})
{
    usize Ret = Back(&Pool->free_list);
    Pop(&Pool->free_list);
    Pool->slots[Ret] = Construct;

    return Ret;
}

template<typename t>
static inline void
Release(Pool<t>* Pool, usize Id) {
    Push(&Pool->free_list, Id);
}

#endif
