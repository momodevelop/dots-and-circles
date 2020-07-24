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
}

static inline void 
Clear(memory_arena* Arena) {
    Arena->Used = 0;
}

static inline void* 
PushBlock(memory_arena* Arena, usize size, u8 alignment = alignof(void*)) {
    Assert(size && alignment);
    u8 adjust = AlignForwardDiff(Arena->Memory, alignment);
    
    // if not enough space, return 
    if ((u8*)Arena->Memory + Arena->Used + adjust + size > (u8*)Arena->Memory + Arena->Capacity ) {
        return nullptr;
    }
    
    void* ret = (u8*)Arena->Memory + Arena->Used + adjust;
    Arena->Used += adjust + size;
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