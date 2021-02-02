#ifndef __MOMO_ARENA__
#define __MOMO_ARENA__

#include "mm_core.h"
#include "mm_bitwise.h"

struct arena {
    u8* Memory;
    usize Used;
    usize Capacity;
};

static inline arena 
Arena(void* Memory, usize Capacity) {
    Assert(Capacity);
    return { (u8*)Memory, 0, Capacity};
}

static inline void 
Clear(arena* Arena) {
    Arena->Used = 0;
}

static inline usize 
Remaining(arena Arena) {
    return Arena.Capacity - Arena.Used;
}


static inline void* 
PushBlock(arena* Arena, usize Size, u8 Alignment = alignof(void*)) {
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
PushStruct(arena* Arena) {
    return (type*)PushBlock(Arena, sizeof(type), alignof(type));
}

// Consider removal because the functions below opens a bag of worms
// regarding whether to forward the arena down and in 
// what order of arguments.
// 
#if 0
// Push with a given constructor function
template<typename type, typename ctr, typename... args>
static inline type*
PushCtr(arena* Arena, ctr Constructor, args&&... Args) {
    type* Ret = PushStruct<type>(Arena);
    
    // Perfect forwarding!
    (*Ret) = Constructor(static_cast<args&&>(Args)...);

    return Ret;
}

// Same as PushConstruct, but passes the Arena down as first variable.
// This is because there are many cases where you want to 
// allocate with the same allocator.
template<typename type, typename ctr, typename... args>
static inline type*
PushCtrPassThru(arena* Arena, ctr Constructor, args&&... Args) {
    type* Ret = PushStruct<type>(Arena);
    (*Ret) = Constructor(Arena, static_cast<args&&>(Args)...);
    return Ret;
}
#endif

template<typename type>
static inline type*
PushSiArray(arena* Arena, usize Count) {
    return (type*)PushBlock(Arena, sizeof(type) * Count, alignof(type));
}

// NOTE(Momo): "Temporary Memory" API
struct scratch {
    arena* Arena;
    usize OldUsed;

    // Allow conversion to arena* to use functions associated with it
    operator arena*() const { 
        return Arena; 
    }
};

static inline scratch
BeginScratch(arena* Arena) {
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


static inline arena
SubArena(arena* SrcArena, usize Capacity) {
    Assert(Capacity);
    arena Ret = {};

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
    arena* ArenaPtr = (arena*)((u8*)Memory + OffsetToArena);
    (*ArenaPtr) = Arena(ArenaMemory, ArenaMemorySize);
    
    return Memory;
}


#define BootstrapStruct(Type, Member, Memory, MemorySize) (Type*)BootstrapBlock(sizeof(Type), OffsetOf(Type, Member), (Memory), (MemorySize)) 

#endif
