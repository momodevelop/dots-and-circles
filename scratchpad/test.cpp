#include <stdio.h>
#include <stdlib.h>
#include "ryoji_common.cpp"

pure void* AlignForward(void* ptr, u8 align) {
    Assert(align > 0 && (align & (align - 1)) == 0); // power of 2 only
    return (void*)((uptr(ptr) + (align - 1)) & ~(align - 1));
}

pure void* AlignBackward(void* ptr, u8 align) {
    Assert(align > 0 && (align & (align - 1)) == 0); // power of 2 only
    return (void*)(uptr(ptr) & ~(align - 1));
}

pure u8 AlignBackwardDiff(void* ptr, u8 align)  {
    return u8((uptr)ptr - uptr(AlignBackward(ptr, align)));
}

pure u8 AlignForwardDiff(void* ptr, u8 align)  {
    return u8(uptr(AlignForward(ptr, align)) - uptr(ptr));
}

struct LinearArena {
    void* Memory;
    usize Used;
    usize Capacity;
};

void Init(LinearArena* a, void* memory, usize capacity) {
    Assert(capacity);
    a->Memory = memory;
    a->Capacity = capacity;
    a->Used = 0;
}

// NOTE(Momo): Linear allocation
void* Allocate(LinearArena* a, usize size, u8 alignment) {
    Assert(size && alignment);
    u8 adjust = AlignForwardDiff(a->Memory, alignment);
    
    // if not enough space, return 
    if ((u8*)a->Memory + a->Used + adjust + size > (u8*)a->Memory + a->Capacity ) {
        return nullptr;
    }
    
    void* ret = (u8*)a->Memory + a->Used + adjust;
    
    a->Used += adjust + size;
    return ret;
}


struct StackArena {
    void* Memory;
    usize Used;
    usize Capacity;
};

#define New(T, arena) (T*)Allocate(arena, sizeof(T), alignof(T))


struct Vector3 {
    float x, y, z;
};

int main() {
    void * mem = malloc(1024);
    LinearArena arena;
    Init(&arena, mem, 13);
    
    Vector3* test1 = New(Vector3, &arena);
    
    printf("Capacity: %lld\n", arena.Capacity);
    printf("Used: %lld\n", arena.Used);
    
    printf("test1: %p\n", test1);
    free(mem);
    
}


