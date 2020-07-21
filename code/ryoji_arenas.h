#ifndef __RYOJI_ARENAS__
#define __RYOJI_ARENAS__

#include "ryoji.h"
#include "ryoji_bitmanip.h"


struct memory_arena {
    u8* Memory;
    usize Used;
    usize Capacity;
};

static inline memory_arena
MakeMemoryArena(void* Memory, usize Capacity) {
    Assert(Capacity);
    memory_arena Ret = { (u8*)Memory, 0, Capacity };
    return Ret;
}


static inline void* 
PushBlock(memory_arena* a, usize size, u8 alignment = alignof(void*)) {
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

template<typename T>
static inline T*
PushStruct(memory_arena* Arena) {
    return (T*)PushBlock(Arena, sizeof(T), alignof(T));
}


template<typename T>
static inline T*
PushArray(memory_arena* Arena, usize Count) {
    return (T*)PushBlock(Arena, sizeof(T) * Count, alignof(T));
}


#endif