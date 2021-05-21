#ifndef __MOMO_ARENA__
#define __MOMO_ARENA__

struct MM_Arena {
    u8* memory;
    u32 used;
    u32 capacity;
};

static inline b8
MM_Arena_Init(MM_Arena* A, void* memory, u32 capacity) {
    if (!memory || !capacity) {
        return false;
    }
    A->memory = (u8*)memory;
    A->used = 0; 
    A->capacity = capacity;
    return true;
}


static inline void 
MM_Arena_Clear(MM_Arena* a) {
    a->used = 0;
}

static inline u32
MM_Arena_Remaining(MM_Arena a) {
    return a.capacity - a.used;
}


static inline void* 
MM_Arena_PushBlock(MM_Arena* a, u32 size, u8 alignment = alignof(void*)) {
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


static inline MM_Arena
MM_Arena_SubArena(MM_Arena* srcArena, u32 capacity) {
    Assert(capacity);
    MM_Arena ret = {};
    
    // We don't care about alignment, so it should be 1.
    ret.memory = (u8*)MM_Arena_PushBlock(srcArena, capacity, 1);
    Assert(ret.memory);
    ret.capacity = capacity;
    return ret;
}

static inline void*
MM_Arena_BootupBlock(u32 struct_size,
                     u32 arena_offset,
                     void* memory,
                     u32 memory_size) 
{
    Assert(struct_size < memory_size);
    void* arena_memory = (u8*)memory + struct_size; 
    u32 arena_memory_size = memory_size - struct_size;
    MM_Arena* arena_ptr = (MM_Arena*)((u8*)memory + arena_offset);
    
    MM_Arena_Init(arena_ptr, arena_memory, arena_memory_size);
    
    return memory;
}


#define Arena_BootupStruct(Type, Member, memory, memorySize) (Type*)MM_Arena_BootupBlock(sizeof(Type), OffsetOf(Type, Member), (memory), (memorySize)) 


// NOTE(Momo): "Temporary memory" API
struct MM_ArenaMark {
    MM_Arena* arena;
    u32 old_used;
    
    operator MM_Arena*() {
        return arena;
    }
};

static inline MM_ArenaMark
MM_Arena_Mark(MM_Arena* a) {
    MM_ArenaMark ret = {};
    ret.arena = a;
    ret.old_used = a->used;
    return ret;
}

static inline void
MM_Arena_Revert(MM_ArenaMark* mark) {
    mark->arena->used = mark->old_used;
    mark->arena = 0;
}
#endif
