#ifndef __MOMO_ARENA__
#define __MOMO_ARENA__

struct Arena;
struct Arena_Mark {
    // NOTE(Momo): Regarding reverting, an arena status back to an Arena_Mark,
    // I considered using destructors, to return the arena but...
    // there are still cases where I feel that you do *not* want to destruct
    Arena* arena;
    umi old_used;
    
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
    umi used;
    umi capacity;
    
    b8 init(void* memory, umi capacity);
    void clear();
    umi remaining();
    
    void* push_block(umi size, u8 alignment = alignof(void*));
    template<class T> T* push_struct();
    template<class T> T* push_array(umi count);
    
    static void* boot_block(umi struct_size,
                            umi offset_to_arena,
                            void* memory,
                            umi memory_size);
    
    Arena_Mark mark();
};

#include "momo_arena.cpp"

#endif
