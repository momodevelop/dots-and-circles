
b8 
Arena::init(void* mem, u32 cap) {
    if (!mem || cap == 0) {
        return false;
    }
    this->memory = (u8*)memory;
    this->used = 0; 
    this->capacity = capacity;
    
    return true;
}

void
Arena::clear() {
    this->used = 0;
}

u32 
Arena::remaining() {
    return this->capacity - this->used;
}

void* 
Arena::push_block(u32 size, u8 alignment) {
    if (size == 0 || alignment == 0) {
        return nullptr;
    }
    u8 adjust = AlignForwardDiff((u8*)this->memory + this->used, alignment);
    
    // if not enough space, return 
    u8* memory_end = (u8*)this->memory + this->capacity;
    u8* block_end = (u8*)this->memory + this->used + adjust + size;
    if (block_end > memory_end) {
        return nullptr;
    }
    
    void* ret = (u8*)this->memory + this->used + adjust;
    this->used += adjust + size;
    return ret;
}

template<class T>
T* 
Arena::push_struct() {
    return (T)push_block(sizeof(T), alignof(T));
}

template<class T>
T*
Arena::push_array(u32 count) {
    return (T*)push_block(sizeof(T) * count, alignof(T));
}

void* 
Arena::boot_block(u32 struct_size,
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
    if (!arena_ptr->init(arena_memory, arena_memory_size)) {
        return nullptr;
    }
    return memory;
}

Arena_Mark
Arena::mark() {
    Arena_Mark ret = {0};
    ret.arena = this;
    ret.old_used = this->used;
    
    return ret;
}

b8
Arena::revert(Arena_Mark mark) {
    if (mark.arena != this) {
        return false;
    }
    
    this->used = mark.old_used;
    return true;
}

// NOTE(Momo): It's a little sad that we can't run away from macros sometimes...
#define ARENA_BOOT_STRUCT(Type, Member, Memory, MemorySize) (Type*)Arena_BootupBlock(sizeof(Type), OffsetOf(Type, Member), (Memory), (MemorySize)) 