#ifndef __RYOJI_ARENAS__
#define __RYOJI_ARENAS__

#include "ryoji.cpp"
#include "ryoji_bitmanip"


struct Arena {
    u8* Memory;
    usize Used;
    usize Capacity;
};

bool Init(Arena* a, void* memory, usize capacity) {
    Assert(capacity);
    a->Memory = memory;
    a->Capacity = capacity;
    a->Used = 0;
}

void* zawarudo_AllocLinear(Arena* a, usize size, u8 alignment) {
    Assert(size && alignment);
    u8 adjust = AlignForward(a->Memory, alignment);
    
    // if not enough space, return 
    if (a->Memory + Used + adjust + size > a->Memory + a->Capacity ) {
        return nullptr;
    }
    
    void* ret = a->Memory + Used + adjust;
    
    a->Used += adjust + size;
    return ret;
}
#define AllocLinear(type, arena) 
#define AllocLinearArray(type, arena, count)




#endif