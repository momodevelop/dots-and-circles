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
    
    // TODO(Momo): Zero memory? 
}


static inline void 
Clear(memory_arena* Arena) {
    Arena->Used = 0;
}

static inline usize
GetRemainingCapacity(memory_arena* Arena) {
    return Arena->Capacity - Arena->Used;
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

// NOTE(Momo): temporary memory definitions
struct temporary_memory {
    memory_arena* Arena;
    usize OldUsed;
};

static inline temporary_memory
BeginTemporaryMemory(memory_arena *Arena) {
    temporary_memory Ret;
    Ret.Arena = Arena;
    Ret.OldUsed = Arena->Used;
    return Ret;
}

static inline temporary_memory
BeginTemporaryMemory(temporary_memory *TempMemory) {
    temporary_memory Ret;
    Ret.Arena = TempMemory->Arena;
    Ret.OldUsed = TempMemory->Arena->Used;
    return Ret;
}

static inline void
EndTemporaryMemory(temporary_memory TempMemory) {
    TempMemory.Arena->Used = TempMemory.OldUsed;
}

static inline void
SubArena(memory_arena* DstArena, memory_arena* SrcArena, usize Capacity) {
    Assert(Capacity);
    DstArena->Memory = (u8*)PushBlock(SrcArena, Capacity);
    Assert(DstArena->Memory);
    DstArena->Capacity = Capacity;
}

#endif