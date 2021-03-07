#ifndef __MOMO_ARENA__
#define __MOMO_ARENA__

struct arena {
    u8* Memory;
    usize Used;
    usize Capacity;
};

static inline arena 
Arena_Create(void* Memory, usize Capacity) {
    Assert(Capacity);
    return { (u8*)Memory, 0, Capacity};
}

static inline void 
Arena_Clear(arena* Arena) {
    Arena->Used = 0;
}

static inline usize 
Arena_Remaining(arena Arena) {
    return Arena.Capacity - Arena.Used;
}


static inline void* 
Arena_PushBlock(arena* Arena, usize Size, u8 Alignment = alignof(void*)) {
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
Arena_PushStruct(arena* Arena) {
    return (type*)Arena_PushBlock(Arena, sizeof(type), alignof(type));
}

template<typename type>
static inline type*
Arena_PushSiArray(arena* Arena, usize Count) {
    return (type*)Arena_PushBlock(Arena, sizeof(type) * Count, alignof(type));
}

// NOTE(Momo): "Temporary Memory" API
struct scratch {
    arena* Arena;
    usize OldUsed;
    
    // Allow conversion to arena* to use functions associated with it
    operator arena*() { 
        return Arena; 
    }
};

static inline scratch
Arena_BeginScratch(arena* Arena) {
    scratch Ret = {};
    Ret.Arena = Arena;
    Ret.OldUsed = Arena->Used;
    return Ret;
}

// NOTE: Conceptually, you can choose not to call this after
// BeginScratch() to permanently 'imprint' what was done in scratch
// onto the memory.
static inline void
Arena_EndScratch(scratch* Scratch) {
    Scratch->Arena->Used = Scratch->OldUsed;
}

// TODO: Is this function really needed?
// Doesn't scratch already fulfill everything we need?
static inline arena
Arena_SubArena(arena* SrcArena, usize Capacity) {
    Assert(Capacity);
    arena Ret = {};
    
    // We don't care about alignment, so it should be 1.
    Ret.Memory = (u8*)Arena_PushBlock(SrcArena, Capacity, 1);
    Assert(Ret.Memory);
    Ret.Capacity = Capacity;
    return Ret;
}

static inline void*
Arena_BootstrapBlock(usize StructSize,
                     usize OffsetToArena,
                     void* Memory,
                     usize MemorySize) 
{
    Assert(StructSize < MemorySize);
    void* ArenaMemory = (u8*)Memory + StructSize; 
    usize ArenaMemorySize = MemorySize - StructSize;
    arena* ArenaPtr = (arena*)((u8*)Memory + OffsetToArena);
    (*ArenaPtr) = Arena_Create(ArenaMemory, ArenaMemorySize);
    
    return Memory;
}


#define BootstrapStruct(Type, Member, Memory, MemorySize) (Type*)Arena_BootstrapBlock(sizeof(Type), OffsetOf(Type, Member), (Memory), (MemorySize)) 

#endif
