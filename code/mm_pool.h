#ifndef __MM_POOL__
#define __MM_POOL__

#include "mm_core.h"
#include "mm_list.h"
#include "mm_array.h"
#include "mm_arena.h"

template<typename t>
struct pool {
    array<t> Slots;
    list<usize> FreeList;
};

template<typename t>
static inline pool<t>
CreatePool(arena* Arena, usize Capacity) {
    pool<t> Ret = {};
    Ret.Slots = Array_Create<t>(Arena, Capacity); 
    Ret.FreeList = CreateList<usize>(Arena, Capacity);
    for (usize I = 0; I < Capacity; ++I) {
        Push(&Ret.FreeList, I);
    }
    return Ret;
}

template<typename t>
static inline t*
Get(pool<t>* Pool, usize Id) {
    return &Pool->Slots[Id]; 
}

// For use with smaller types
template<typename t>
static inline t
GetCopy(pool<t>* Pool, usize Id) {
    return Pool->Slots[Id]; 
}

template<typename t>
static inline usize 
Remainder(pool<t>* Pool) {
    return Pool->FreeList.Count;
}

// Analogous to 'booking a reservation'
template<typename t>
static inline usize
Reserve(pool<t>* Pool, t Construct = {})
{
    usize Ret = Back(&Pool->FreeList);
    Pop(&Pool->FreeList);
    Pool->Slots[Ret] = Construct;
    
    return Ret;
}

template<typename t>
static inline void
Release(pool<t>* Pool, usize Id) {
    Push(&Pool->FreeList, Id);
}

#endif
