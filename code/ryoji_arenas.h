#ifndef __RYOJI_ARENAS__
#define __RYOJI_ARENAS__

#include "ryoji.h"
#include "ryoji_bitmanip.h"

struct arena {
    u8* Memory;
    usize Used;
    usize Capacity;
};

static inline void 
Init(arena* Arena, void* Memory, usize Capacity) {
    Assert(Capacity);
    Arena->Memory = (u8*)Memory;
    Arena->Capacity = Capacity;
}

static inline void 
Clear(arena* Arena) {
    Arena->Used = 0;
}

static inline usize
GetRemainingCapacity(arena* Arena) {
    return Arena->Capacity - Arena->Used;
}

static inline void* 
PushBlock(arena* Arena, usize size, u8 alignment = alignof(void*)) {
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
PushStruct(arena* Arena) {
    return (T*)PushBlock(Arena, sizeof(T), alignof(T));
}


template<typename T>
static inline T*
PushArray(arena* Arena, usize Count) {
    return (T*)PushBlock(Arena, sizeof(T) * Count, alignof(T));
}

// NOTE(Momo): temporary memory API
struct temp_arena {
    arena* Arena;
    usize OldUsed;
};

static inline temp_arena
BeginTempArena(arena *Arena) {
    temp_arena Ret;
    Ret.Arena = Arena;
    Ret.OldUsed = Arena->Used;
    return Ret;
}

static inline temp_arena
BeginTempArena(temp_arena *TempMemory) {
    temp_arena Ret;
    Ret.Arena = TempMemory->Arena;
    Ret.OldUsed = TempMemory->Arena->Used;
    return Ret;
}

static inline void
EndTempArena(temp_arena TempMemory) {
    TempMemory.Arena->Used = TempMemory.OldUsed;
}


static inline void* 
PushBlock(temp_arena* TempArena, usize size, u8 alignment = alignof(void*)) {
    return PushBlock(TempArena->Arena, size, alignment);
}

template<typename T>
static inline T*
PushStruct(temp_arena* TempArena) {
    return PushStruct<T>(TempArena->Arena, sizeof(T), alignof(T));
}


template<typename T>
static inline T*
PushArray(temp_arena* TempArena, usize Count) {
    return PushArray<T>(TempArena->Arena, Count);
}

static inline void
SubArena(arena* DstArena, arena* SrcArena, usize Capacity) {
    Assert(Capacity);
    DstArena->Memory = (u8*)PushBlock(SrcArena, Capacity);
    Assert(DstArena->Memory);
    DstArena->Capacity = Capacity;
}

#endif