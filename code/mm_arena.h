#ifndef __MOMO_ARENA__
#define __MOMO_ARENA__

#include "mm_core.h"
#include "mm_bitwise.h"

struct mmarn_arena {
    u8* Memory;
    usize Used;
    usize Capacity;
};

static inline mmarn_arena 
mmarn_Arena(void* Memory, usize Capacity) {
    Assert(Capacity);
    return { (u8*)Memory, 0, Capacity};
}

static inline void 
mmarn_Clear(mmarn_arena* Arena) {
    Arena->Used = 0;
}

static inline usize 
mmarn_Remaining(mmarn_arena* Arena) {
    return Arena->Capacity - Arena->Used;
}


static inline void* 
mmarn_PushBlock(mmarn_arena* Arena, usize size, u8 alignment = alignof(void*)) {
    Assert(size && alignment);
    u8 adjust = mmbw_AlignForwardDiff(Arena->Memory, alignment);
    
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
mmarn_PushStruct(mmarn_arena* Arena) {
    return (T*)mmarn_PushBlock(Arena, sizeof(T), alignof(T));
}


template<typename T>
static inline T*
mmarn_PushArray(mmarn_arena* Arena, usize Count) {
    return (T*)mmarn_PushBlock(Arena, sizeof(T) * Count, alignof(T));
}

// NOTE(Momo): temporary memory API
struct mmarn_scratch {
    mmarn_arena* Arena;
    usize OldUsed;
};

static inline mmarn_scratch
mmarn_BeginScratch(mmarn_arena* Arena) {
    mmarn_scratch Ret;
    Ret.Arena = Arena;
    Ret.OldUsed = Arena->Used;
    return Ret;
}

static inline mmarn_scratch
mmarn_BeginScratch(mmarn_scratch* TempMemory) {
    return mmarn_BeginScratch(TempMemory->Arena);
}

static inline void
mmarn_EndScratch(mmarn_scratch* TempMemory) {
    TempMemory->Arena->Used = TempMemory->OldUsed;
}


static inline void* 
mmarn_PushBlock(mmarn_scratch* TempArena, usize size, u8 alignment = alignof(void*)) {
    return mmarn_PushBlock(TempArena->Arena, size, alignment);
}

template<typename T>
static inline T*
mmarn_PushStruct(mmarn_scratch* TempArena) {
    return PushStruct<T>(TempArena->Arena, sizeof(T), alignof(T));
}


template<typename T>
static inline T*
mmarn_PushArray(mmarn_scratch* TempArena, usize Count) {
    return PushArray<T>(TempArena->Arena, Count);
}

static inline mmarn_arena
mmarn_PushArena(mmarn_arena* SrcArena, usize Capacity) {
    Assert(Capacity);
    mmarn_arena Ret = {};
    Ret.Memory = (u8*)mmarn_PushBlock(SrcArena, Capacity);
    Assert(Ret.Memory);
    Ret.Capacity = Capacity;
    return Ret;
}

// Creates a sub arena with all remaining memory
static inline mmarn_arena
mmarn_PushArenaAll(mmarn_arena* SrcArena) { 
    return mmarn_PushArena(SrcArena, mmarn_Remaining(SrcArena));
}

#endif
