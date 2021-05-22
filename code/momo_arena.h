#ifndef __MOMO_ARENA__
#define __MOMO_ARENA__

struct arena {
    u8* Memory;
    u32 Used;
    u32 Capacity;
};

static inline b8
Arena_Init(arena* A, void* Memory, u32 Capacity) {
    if (!Memory || Capacity == 0) {
        return false;
    }
    A->Memory = (u8*)Memory;
    A->Used = 0; 
    A->Capacity = Capacity;
    
    return true;
}

static inline void 
Arena_Clear(arena* Arena) {
    Arena->Used = 0;
}

static inline u32
Arena_Remaining(arena* Arena) {
    return Arena->Capacity - Arena->Used;
}


static inline void* 
Arena_PushBlock(arena* Arena, u32 Size, u8 Alignment = alignof(void*)) {
    if (Size == 0 || Alignment == 0) {
        return nullptr;
    }
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
Arena_PushArray(arena* A, u32 Count) {
    return (type*)Arena_PushBlock(A, sizeof(type)*Count, alignof(type));
}

template<typename type>
static inline type*
Arena_PushStruct(arena* A) {
    return (type*)Arena_PushBlock(A, sizeof(type), alignof(type));
}


#if 0
static inline arena
Arena_SubArena(arena* SrcArena, u32 Capacity) {
    Assert(Capacity);
    arena Ret = {};
    
    // We don't care about alignment, so it should be 1.
    Ret.Memory = (u8*)Arena_PushBlock(SrcArena, Capacity, 1);
    Assert(Ret.Memory);
    Ret.Capacity = Capacity;
    return Ret;
}
#endif

static inline void*
Arena_BootupBlock(u32 StructSize,
                  u32 OffsetToArena,
                  void* Memory,
                  u32 MemorySize) 
{
    if (StructSize > MemorySize) {
        return nullptr;
    }
    void* ArenaMemory = (u8*)Memory + StructSize; 
    u32 ArenaMemorySize = MemorySize - StructSize;
    arena* ArenaPtr = (arena*)((u8*)Memory + OffsetToArena);
    if (!Arena_Init(ArenaPtr, ArenaMemory, ArenaMemorySize)) {
        return nullptr;
    }
    return Memory;
}

#define Arena_BootupStruct(Type, Member, Memory, MemorySize) (Type*)Arena_BootupBlock(sizeof(Type), OffsetOf(Type, Member), (Memory), (MemorySize)) 


// NOTE(Momo): "Temporary Memory" API
struct arena_mark {
    arena* Arena;
    u32 OldUsed;
    
    operator arena*() {
        return Arena;
    }
};

static inline arena_mark
Arena_Mark(arena* Arena) {
    arena_mark Ret = {};
    Ret.Arena = Arena;
    Ret.OldUsed = Arena->Used;
    return Ret;
}

static inline void
Arena_Revert(arena_mark* Mark) {
    Mark->Arena->Used = Mark->OldUsed;
}
#endif
