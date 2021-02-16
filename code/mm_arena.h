#ifndef __MOMO_ARENA__
#define __MOMO_ARENA__

#include "mm_core.h"
#include "mm_bitwise.h"

struct Memory_Arena {
    u8* Memory;
    usize Used;
    usize Capacity;
};

static inline Memory_Arena 
Arena(void* Memory, usize Capacity) {
    Assert(Capacity);
    return { (u8*)Memory, 0, Capacity};
}

static inline void 
Clear(Memory_Arena* Arena) {
    Arena->Used = 0;
}

static inline usize 
Remaining(Memory_Arena Arena) {
    return Arena.Capacity - Arena.Used;
}


static inline void* 
PushBlock(Memory_Arena* Arena, usize Size, u8 Alignment = alignof(void*)) {
    Assert(Size && Alignment);
    u8 Adjust = AlignForwardDiff((u8*)Arena->Memory + Arena->Used, Alignment);
    
    // if not enough space, return 
    u8* MemoryEnd = (u8*)Arena->Memory + Arena->Capacity;
    u8* BlockEnd = (u8*)Arena->Memory + Arena->Used + Adjust + Size;
    if (BlockEnd > MemoryEnd) {
        return nullptr;
    }
    
    void* ret = (u8*)Arena->Memory + Arena->Used + Adjust;
    Arena->Used += Adjust + Size;
    return ret;
}

template<typename type>
static inline type*
PushStruct(Memory_Arena* Arena) {
    return (type*)PushBlock(Arena, sizeof(type), alignof(type));
}

template<typename type>
static inline type*
PushSiArray(Memory_Arena* Arena, usize Count) {
    return (type*)PushBlock(Arena, sizeof(type) * Count, alignof(type));
}

// NOTE(Momo): "Temporary Memory" API
struct scratch {
    Memory_Arena* Arena;
    usize OldUsed;

    // Allow conversion to arena* to use functions associated with it
    operator Memory_Arena*() const { 
        return Arena; 
    }
};

static inline scratch
BeginScratch(Memory_Arena* Arena) {
    scratch Ret = {};
    Ret.Arena = Arena;
    Ret.OldUsed = Arena->Used;
    return Ret;
}

// NOTE: Conceptually, you can choose not to call this after
// BeginScratch() to permanently 'imprint' what was done in scratch
// onto the memory.
static inline void
EndScratch(scratch* Scratch) {
    Scratch->Arena->Used = Scratch->OldUsed;
}


static inline Memory_Arena
SubArena(Memory_Arena* SrcArena, usize Capacity) {
    Assert(Capacity);
    Memory_Arena Ret = {};

    // We don't care about alignment, so it should be 1.
    Ret.Memory = (u8*)PushBlock(SrcArena, Capacity, 1);
    Assert(Ret.Memory);
    Ret.Capacity = Capacity;
    return Ret;
}

static inline void*
BootstrapBlock(usize StructSize,
               usize OffsetToArena,
               void* Memory,
              usize MemorySize) 
{
    Assert(StructSize < MemorySize);
    void* ArenaMemory = (u8*)Memory + StructSize; 
    usize ArenaMemorySize = MemorySize - StructSize;
    Memory_Arena* ArenaPtr = (Memory_Arena*)((u8*)Memory + OffsetToArena);
    (*ArenaPtr) = Arena(ArenaMemory, ArenaMemorySize);
    
    return Memory;
}


#define BootstrapStruct(Type, Member, Memory, MemorySize) (Type*)BootstrapBlock(sizeof(Type), OffsetOf(Type, Member), (Memory), (MemorySize)) 

#endif
