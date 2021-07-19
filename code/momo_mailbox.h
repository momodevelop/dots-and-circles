// Message bus system
#ifndef __MOMO_MAILBOX_H__
#define __MOMO_MAILBOX_H__

struct Mailbox_EntryHeader {
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
};


static inline void
Mailbox_Clear(Mailbox* m) {
    m->data_memory_at = m->memory;
    
    u8* start = m->memory + m->memory_size - sizeof(Mailbox_EntryHeader);
    u8 adjust = AlignMemoryBackwardDiff(start, alignof(Mailbox_EntryHeader));
    start-= adjust;
    
    m->entry_memory_start = start;
    m->entry_memory_at = start;
    
    m->entry_count = 0;
}



static inline b8
Mailbox_Init(Mailbox* m, void* memory, u32 memory_size) {
    if (!memory || memory_size == 0) {
        return false;
    }
    
    m->memory = (u8*)memory;
    m->memory_size = memory_size;
    
    Mailbox_Clear(m);
    
    return true;
}


// NOTE(Momo): Accessors and Iterators
static inline Mailbox_EntryHeader*
Mailbox_GetEntry(Mailbox* m, u32 index) {
    ASSERT(index < m->entry_count);
    return (Mailbox_EntryHeader*)(m->entry_memory_start - index * sizeof(Mailbox_EntryHeader));
}


static inline void*
Mailbox_GetEntryData(Mailbox* m, Mailbox_EntryHeader* entry) {
    return (m->memory + entry->offset_to_data);
}


static inline void*
_Mailbox_PushBlock(Mailbox* m, u32 size, u8 alignment,u32 id) 
{
    // Allocate data
    u8 data_adjust = AlignMemoryForwardDiff(m->data_memory_at, alignment);
    u32 data_size = size;
    
    // Allocate entry
    u8 entry_adjust = AlignMemoryBackwardDiff(m->entry_memory_at, alignof(Mailbox_EntryHeader));
    u32 entry_size = sizeof(Mailbox_EntryHeader);
    
    if (m->entry_memory_at - entry_size - entry_adjust < m->data_memory_at + data_size +  data_adjust) 
    {
        return nullptr; 
    }
    
    void* data = ((u8*)m->data_memory_at + data_adjust);
    m->data_memory_at += data_size + data_adjust;
    
    auto* entry = (Mailbox_EntryHeader*)((u8*)m->entry_memory_at + entry_adjust);
    
    entry->offset_to_data = (u32)((u8*)data - m->memory);
    
    entry->type = id; 
    m->entry_memory_at -= entry_size;
    ++m->entry_count;
    
    return data;
}

template<typename T>
static inline T*
Mailbox_Push(Mailbox* m, u32 id) {
    return (T*)_Mailbox_PushBlock(m, sizeof(T), alignof(T), id);
}
#endif
