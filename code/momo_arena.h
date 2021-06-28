#ifndef __MOMO_ARENA__
#define __MOMO_ARENA__

struct Arena;
struct Arena_Mark {
    // NOTE(Momo): Regarding reverting, an arena status back to an Arena_Mark,
    // I considered using destructors, to return the arena but...
    // there are still cases where I feel that you do *not* want to destruct
    Arena* arena;
    u32 old_used;
    
    // TODO: Can we remove this?
    // Can't we just create more free functions specific to Arena_Mark itself? 
    // There are some functions from an API pov where
    // Arena_Mark shouldn't act like Arean, for example: clear()
    operator Arena*() {
        return this->arena;
    }
    
    void revert();
};

struct Arena {
    u8* memory;
    u32 used;
    u32 capacity;
    
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
};

#include "momo_arena.cpp"

#endif
