#ifndef __RYOJI_ARENAS__
#define __RYOJI_ARENAS__

#include "ryoji.cpp"
#include "ryoji_bitmanip.cpp"


struct LinearArena {
    u8* memory;
    usize used;
    usize capacity;
};


internal
bool 
Init(LinearArena* a, void* memory, usize capacity) {
    Assert(capacity);
    a->memory = memory;
    a->capacity = capacity;
    a->used = 0;
}

// NOTE(Momo): Linear allocation
internal
void* 
Allocate(LinearArena* a, usize size, u8 alignment) {
    Assert(size && alignment);
    u8 adjust = AlignForwardDiff(a->memory, alignment);
    
    // if not enough space, return 
    if ((u8*)a->memory + a->used + adjust + size > (u8*)a->memory + a->capacity ) {
        return nullptr;
    }
    
    void* ret = (u8*)a->memory + a->used + adjust;
    
    a->used += adjust + size;
    return ret;
}

#define New(type, arena)   (T*)Allocate(arena, sizeof(T), alignof(T))
#define NewArray(type, arena, count) (T*)Allocate(arena, sizeof(T)*count, alignof(T))




#endif