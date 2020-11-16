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
mmarn_PushBlock(mmarn_arena* Arena, usize Size, u8 Alignment = alignof(void*)) {
    Assert(Size && Alignment);
    u8 Adjust = mmbw_AlignForwardDiff(Arena->Memory, Alignment);
    
    // if not enough space, return 
    if ((u8*)Arena->Memory + Arena->Used + Adjust + Size > (u8*)Arena->Memory + Arena->Capacity ) {
        return nullptr;
    }
    
    void* ret = (u8*)Arena->Memory + Arena->Used + Adjust;
    Arena->Used += Adjust + Size;
    return ret;
}

template<typename type>
static inline type*
mmarn_PushStruct(mmarn_arena* Arena) {
    return (type*)mmarn_PushBlock(Arena, sizeof(type), alignof(type));
}

// Push with a given constructor function
template<typename type, typename ctr, typename... args>
static inline type*
mmarn_PushCtr(mmarn_arena* Arena, ctr Constructor, args&&... Args) {
    type* Ret = mmarn_PushStruct<type>(Arena);
    
    // Perfect forwarding!
    (*Ret) = Constructor(static_cast<args&&>(Args)...);

    return Ret;
}

// Same as PushConstruct, but passes the Arena down as first variable.
// This is because there are many cases where you want to allocate with the same
// allocator.
template<typename type, typename ctr, typename... args>
static inline type*
mmarn_PushCtrPassThru(mmarn_arena* Arena, ctr Constructor, args&&... Args) {
    type* Ret = mmarn_PushStruct<type>(Arena);
    (*Ret) = Constructor(Arena, static_cast<args&&>(Args)...);
    return Ret;
}

template<typename type>
static inline type*
mmarn_PushArray(mmarn_arena* Arena, usize Count) {
    return (type*)mmarn_PushBlock(Arena, sizeof(type) * Count, alignof(type));
}

// NOTE(Momo): temporary memory API
struct mmarn_scratch {
    mmarn_arena* Arena;
    usize OldUsed;

    // Allow conversion to mmarn_arena* to use functions associated with it
    operator mmarn_arena*() const { 
        return Arena; 
    }
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


static inline mmarn_arena
mmarn_SubArena(mmarn_arena* SrcArena, usize Capacity) {
    Assert(Capacity);
    mmarn_arena Ret = {};

    // We don't care about alignment, so it should be 1.
    Ret.Memory = (u8*)mmarn_PushBlock(SrcArena, Capacity, 1);
    Assert(Ret.Memory);
    Ret.Capacity = Capacity;
    return Ret;
}



#endif
