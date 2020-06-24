#ifndef __RYOJI_ARENAS__
#define __RYOJI_ARENAS__

#include "ryoji.cpp"
#include "ryoji_bitmanip.cpp"


struct linear_arena {
    u8* Memory;
    usize Used;
    usize capacity;
};


static inline bool 
Init(linear_arena* a, void* Memory, usize Capacity) {
    Assert(capacity);
    a->Memory = Memory;
    a->capacity = capacity;
    a->Used = 0;
}

// NOTE(Momo): Linear allocation
static inline void* 
Allocate(linear_arena* a, usize size, u8 alignment) {
    Assert(size && alignment);
    u8 adjust = AlignForwardDiff(a->Memory, alignment);
    
    // if not enough space, return 
    if ((u8*)a->Memory + a->Used + adjust + size > (u8*)a->Memory + a->capacity ) {
        return nullptr;
    }
    
    void* ret = (u8*)a->Memory + a->Used + adjust;
    
    a->Used += adjust + size;
    return ret;
}

#define New(type, arena)   (T*)Allocate(arena, sizeof(T), alignof(T))
#define NewArray(type, arena, count) (T*)Allocate(arena, sizeof(T)*count, alignof(T))




#endif