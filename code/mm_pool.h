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

};

template<typename T>
Pool<T> create_pool(Memory_Arena* arena, usize cap) {
    Pool<T> ret = {};
    ret.slots = create_array<T>(arena, cap); 
    ret.free_list = list<usize>(arena, cap);
    for (usize i = 0; i < cap; ++i) {
        Push(&ret.free_list, i);
    }
    return Ret;
}

template<typename T>
static inline T*
get(usize id) {
    return &this->slots[id]; 
}

template<typename T>
static inline usize
remainder(Pool<T>* p) {
    return p->free_list.count;
}

// Analogous to 'booking a reservation'
template<typename t>
static inline usize
reserve(Pool<t>* p, t item = {})
{
    usize ret = Back(&p->free_list);
    Pop(&p->free_list);
    p->slots[ret] = item;

    return ret;
}

template<typename t>
static inline void
release(Pool<t>* Pool, usize Id) {
    Push(&Pool->free_list, Id);
}

#endif
