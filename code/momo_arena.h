#ifndef __MOMO_ARENA__
#define __MOMO_ARENA__

struct Arena;
struct Arena_Marker {
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
    
};

struct Arena {
    u8* memory;
    u32 used;
    u32 capacity;
    
};



#include "momo_arena.cpp"

#endif
