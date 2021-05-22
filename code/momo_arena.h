#ifndef __MOMO_ARENA__
#define __MOMO_ARENA__

struct arena {
    u8* memory;
    u32 used;
    u32 capacity;
};

static inline b8
MM_Arena_Init(arena* A, void* memory, u32 capacity) {
    if (!memory || !capacity) {
        return false;
    }
    A->memory = (u8*)memory;
    A->used = 0; 
    A->capacity = capacity;
    return true;
}


static inline void 
MM_Arena_Clear(arena* a) {
    a->used = 0;
}

static inline u32
MM_Arena_Remaining(arena a) {
    return a.capacity - a.used;
}


static inline void* 
MM_Arena_PushBlock(arena* a, u32 size, u8 alignment = alignof(void*)) {
    if (size == 0 || alignment == 0) {
        return Null;
    }
    u8 adjust = AlignForwardDiff((u8*)a->memory + a->used, alignment);
    
    // if not enough space, return 
    u8* memoryEnd = (u8*)a->memory + a->capacity;
    u8* blockEnd = (u8*)a->memory + a->used + adjust + size;
    if (blockEnd > memoryEnd) {
        return Null;
    }
    
    void* ret = (u8*)a->memory + a->used + adjust;
    a->used += adjust + size;
    return ret;
}

#define MM_Arena_PushArray(Type, Arena, Count) \
(Type*)MM_Arena_PushBlock((Arena), sizeof(Type)*(Count), alignof(Type));

#define MM_Arena_PushStruct(Type, Arena) \
(Type*)MM_Arena_PushBlock((Arena), sizeof(Type), alignof(Type));


static inline arena
MM_Arena_SubArena(arena* srcArena, u32 capacity) {
    Assert(capacity);
    arena ret = {};
    
    // We don't care about alignment, so it should be 1.
    ret.memory = (u8*)MM_Arena_PushBlock(srcArena, capacity, 1);
    Assert(ret.memory);
    ret.capacity = capacity;
    return ret;
}

static inline void*
MM_Arena_BootupBlock(u32 structSize,
                     u32 offsetToArena,
                     void* memory,
                     u32 memorySize) 
{
    Assert(structSize < memorySize);
    void* arenaMemory = (u8*)memory + structSize; 
    u32 arenaMemorySize = memorySize - structSize;
    arena* arenaPtr = (arena*)((u8*)memory + offsetToArena);
    
    MM_Arena_Init(arenaPtr, arenaMemory, arenaMemorySize);
    
    return memory;
}


#define Arena_BootupStruct(Type, Member, memory, memorySize) (Type*)MM_Arena_BootupBlock(sizeof(Type), OffsetOf(Type, Member), (memory), (memorySize)) 


// NOTE(Momo): "Temporary memory" API
struct arena_mark {
    arena* Arena;
    u32 Oldused;
    
    operator arena*() {
        return Arena;
    }
};

static inline arena_mark
MM_Arena_Mark(arena* Arena) {
    arena_mark Ret = {};
    Ret.Arena = Arena;
    Ret.Oldused = Arena->used;
    return Ret;
}

static inline void
MM_Arena_Revert(arena_mark* Mark) {
    Mark->Arena->used = Mark->Oldused;
    Mark->Arena = 0;
}
#endif
