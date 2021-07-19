


static inline b8 
Arena_Init(Arena* arena, void* mem, u32 cap) {
    if (!mem || cap == 0) {
        return false;
    }
    arena->memory = (u8*)mem;
    arena->used = 0; 
    arena->capacity = cap;
    
    return true;
}

static inline void
Arena_Clear(Arena* arena) {
    arena->used = 0;
}

static inline u32 
Arena_Remaining(Arena* arena) {
    return arena->capacity - arena->used;
}

static inline void* 
Arena_Push_Block(Arena* arena, u32 size, u8 alignment = alignof(void*)) {
    if (size == 0 || alignment == 0) {
        return nullptr;
    }
    u8 adjust = AlignMemoryForwardDiff((u8*)arena->memory + arena->used, alignment);
    
    // if not enough space, return 
    u8* memory_end = (u8*)arena->memory + arena->capacity;
    u8* block_end = (u8*)arena->memory + arena->used + adjust + size;
    if (block_end > memory_end) {
        return nullptr;
    }
    
    void* ret = (u8*)arena->memory + arena->used + adjust;
    arena->used += adjust + size;
    return ret;
}

template<class T>
static inline T* 
Arena_Push(Arena* arena) {
    return (T*)Arena_Push_Block(arena, sizeof(T), alignof(T));
}

template<class T>
static inline T*
Arena_PushArray(Arena* arena, u32 count) {
    return (T*)Arena_Push_Block(arena, sizeof(T) * count, alignof(T));
}

static inline void* 
Arena_Boot_Block(u32 struct_size,
                 u32 offset_to_arena,
                 void* memory,
                 u32 memory_size)
{
    if (struct_size > memory_size) {
        return nullptr;
    }
    void* arena_memory = (u8*)memory + struct_size; 
    u32 arena_memory_size = memory_size - struct_size;
    Arena* arena_ptr = (Arena*)((u8*)memory + offset_to_arena);
    if (!Arena_Init(arena_ptr, arena_memory, arena_memory_size)) {
        return nullptr;
    }
    return memory;
}

static inline Arena_Marker
Arena_Mark(Arena* arena) {
    Arena_Marker ret = {0};
    ret.arena = arena;
    ret.old_used = arena->used;
    
    return ret;
}

void
Arena_Revert(Arena_Marker* mark) {
    mark->arena->used = mark->old_used;
}

// NOTE(Momo): It's a little sad that we can't run away from macros sometimes...
#define Arena_Boot(Type, Member, Memory, MemorySize) (Type*)Arena_Boot_Block(sizeof(Type), OffsetOf(Type, Member), (Memory), (MemorySize)) 