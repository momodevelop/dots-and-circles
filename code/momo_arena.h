#ifndef __MOMO_ARENA__
#define __MOMO_ARENA__

struct Arena;
struct Arena_Mark {
    // NOTE(Momo): Regarding reverting, an arena status back to an Arena_Mark,
    // I considered using destructors, to return the arena but...
    // there are still cases where I feel that you do *not* want to destruct
    Arena* arena;
    u32 old_used;
    
    operator Arena*() {
        return this->arena;
    }
    
};

struct Arena {
    //- Member functions
    u8* memory;
    u32 used;
    u32 capacity;
    
    //- Public functions
    b8 init(void* memory, u32 capacity);
    void clear();
    u32 remaining();
    
    void* push_block(u32 size, u8 alignment = alignof(void*));
    template<class T> T* push_struct();
    template<class T> T* push_array(u32 count);
    
    static void* boot_block(u32 struct_size,
                            u32 offset_to_arena,
                            void* memory,
                            u32 memory_size);
    
    Arena_Mark mark();
    b8 revert(Arena_Mark mark);
};

#include "momo_arena.cpp"

#endif
