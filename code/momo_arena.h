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
    inline operator Arena*() {
        return this->arena;
    }
    
    void revert();
};

struct Arena {
    u8* memory;
    u32 used;
    u32 capacity;
    
    inline b8 init(void* memory, u32 capacity);
    inline void clear();
    inline u32 remaining();
    
    inline void* push_block(u32 size, u8 alignment = alignof(void*));
    template<class T> inline T* push_struct();
    template<class T> inline T* push_array(u32 count);
    
    static inline void* boot_block(u32 struct_size,
                                   u32 offset_to_arena,
                                   void* memory,
                                   u32 memory_size);
    
    inline Arena_Mark mark();
};

#include "momo_arena.cpp"

#endif
