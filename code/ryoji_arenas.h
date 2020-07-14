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
Allocate(memory_arena* a, usize size, u8 alignment = 4) {
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
#define AllocateStruct(Arena, Type) (Type*)Allocate(Arena, sizeof(Type), alignof(Type))
#define AllocateArray(Arena, Count, Type) (T*)Allocate(Arena, sizeof(Type)*Count, alignof(Type))



#endif