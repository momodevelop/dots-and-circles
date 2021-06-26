// Message bus system
#ifndef __MOMO_MAILBOX_H__
#define __MOMO_MAILBOX_H__

struct Mailbox_Entry_Header {
    u32 type;
    u32 offset_to_data;
};

struct Mailbox {
    
    u8* memory;
    u8* data_memory_at;
    u8* entry_memory_start;
    u8* entry_memory_at;   
    u32 memory_size;
    u32 entry_count;
    
    b8 init(void * mem, u32 mem_size);
    void clear();
    Mailbox_Entry_Header* get_entry(u32 index);
    void* get_entry_data(Mailbox_Entry_Header* entry);
    void* push_block(u32 size, u8 alignment, u32 id);
    
    template<typename T>
        T* push_struct(u32);
};

b8
Mailbox::init(void* mem, u32 mem_size) {
    if (!mem || mem_size == 0) {
        return false;
    }
    
    memory = (u8*)mem;
    memory_size = mem_size;
    clear();
    
    return true;
}



void
Mailbox::clear() {
    data_memory_at = memory;
    
    u8* start = memory + memory_size - sizeof(Mailbox_Entry_Header);
    u8 adjust = align_backward_diff(start, alignof(Mailbox_Entry_Header));
    start-= adjust;
    
    entry_memory_start = start;
    entry_memory_at = start;
    
    entry_count = 0;
}

// NOTE(Momo): Accessors and Iterators
Mailbox_Entry_Header*
Mailbox::get_entry(u32 index) {
    Assert(index < entry_count);
    return (Mailbox_Entry_Header*)(entry_memory_start - index * sizeof(Mailbox_Entry_Header));
}


void*
Mailbox::get_entry_data(Mailbox_Entry_Header* entry) {
    return (memory + entry->offset_to_data);
}


void*
Mailbox::push_block(u32 size, 
                    u8 alignment,
                    u32 id) 
{
    // Allocate data
    u8 data_adjust = AlignForwardDiff(data_memory_at, alignment);
    u32 data_size = size;
    
    // Allocate entry
    u8 entry_adjust = align_backward_diff(entry_memory_at, alignof(Mailbox_Entry_Header));
    u32 entry_size = sizeof(Mailbox_Entry_Header);
    
    if (entry_memory_at - entry_size - entry_adjust < data_memory_at + data_size +  data_adjust) 
    {
        return nullptr; 
    }
    
    void* data = ((u8*)data_memory_at + data_adjust);
    data_memory_at += data_size + data_adjust;
    
    auto* entry = (Mailbox_Entry_Header*)((u8*)entry_memory_at + entry_adjust);
    
    entry->offset_to_data = (u32)((u8*)data - memory);
    
    entry->type = id; 
    entry_memory_at -= entry_size;
    ++entry_count;
    
    return data;
}

template<typename T>
T*
Mailbox::push_struct(u32 id) {
    return (T*)push_block(sizeof(T), alignof(T), id);
}
#endif
