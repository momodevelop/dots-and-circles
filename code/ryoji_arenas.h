#ifndef __RYOJI_ARENAS__
#define __RYOJI_ARENAS__

#include "ryoji.h"
#include "ryoji_bitmanip.h"


struct memory_arena {
    u8* Memory;
    usize Used;
    usize Capacity;
};


static inline void
Init(memory_arena* Arena, void* Memory, usize Capacity) {
    Assert(Capacity);
    Arena->Memory = (u8*)Memory;
    Arena->Capacity = Capacity;
    Arena->Used = 0;
}

// NOTE(Momo): Linear allocation
static inline void* 
Allocate(memory_arena* a, usize size, u8 alignment) {
    Assert(size && alignment);
    u8 adjust = AlignForwardDiff(a->Memory, alignment);
    
    // if not enough space, return 
    if ((u8*)a->Memory + a->Used + adjust + size > (u8*)a->Memory + a->Capacity ) {
        return nullptr;
    }
    
    void* ret = (u8*)a->Memory + a->Used + adjust;
    
    a->Used += adjust + size;
    return ret;
}

#define New(type, arena)   (T*)Allocate(arena, sizeof(T), alignof(T))
#define NewArray(type, arena, count) (T*)Allocate(arena, sizeof(T)*count, alignof(T))




#endif