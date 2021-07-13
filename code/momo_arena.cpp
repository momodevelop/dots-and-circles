
b8 
Arena::init(void* mem, umi cap) {
    if (!mem || cap == 0) {
        return false;
    }
    this->memory = (u8*)mem;
    this->used = 0; 
    this->capacity = cap;
    
    return true;
}

void
Arena::clear() {
    this->used = 0;
}

umi 
Arena::remaining() {
    return this->capacity - this->used;
}

void* 
Arena::push_block(umi size, u8 alignment) {
    if (size == 0 || alignment == 0) {
        return nullptr;
    }
    u8 adjust = align_memory_forward_diff((u8*)this->memory + this->used, alignment);
    
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
    return (T*)push_block(sizeof(T), alignof(T));
}

template<class T>
T*
Arena::push_array(umi count) {
    return (T*)push_block(sizeof(T) * count, alignof(T));
}

void* 
Arena::boot_block(umi struct_size,
                  umi offset_to_arena,
                  void* memory,
                  umi memory_size)
{
    if (struct_size > memory_size) {
        return nullptr;
    }
    void* arena_memory = (u8*)memory + struct_size; 
    umi arena_memory_size = memory_size - struct_size;
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

void
Arena_Mark::revert() {
    this->arena->used = this->old_used;
}

// NOTE(Momo): It's a little sad that we can't run away from macros sometimes...
#define ARENA_BOOT_STRUCT(Type, Member, Memory, MemorySize) (Type*)Arena::boot_block(sizeof(Type), OFFSET_OF(Type, Member), (Memory), (MemorySize)) 