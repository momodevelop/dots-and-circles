// Message bus system
#ifndef __MOMO_MAILBOX_H__
#define __MOMO_MAILBOX_H__

#include "mm_maths.h"
#include "mm_arena.h"

struct mailbox_entry_header {
    u32 Type;
    u32 OffsetToData;
};

struct mailbox {
    u8* Memory;
    u8* DataMemoryAt;
    u8* EntryMemoryStart;
    u8* EntryMemoryAt;   
    usize MemorySize;
    u32 EntryCount;
};

static inline void
Clear(mailbox* Mailbox) {
    Mailbox->DataMemoryAt = Mailbox->Memory;
    
    u8* EntryMemoryStart = Mailbox->Memory + Mailbox->MemorySize - sizeof(mailbox_entry_header);
    u8 Adjust = AlignBackwardDiff(EntryMemoryStart, alignof(mailbox_entry_header));
    EntryMemoryStart -= Adjust;
    
    Mailbox->EntryMemoryStart = EntryMemoryStart;
    Mailbox->EntryMemoryAt = EntryMemoryStart;
    
    Mailbox->EntryCount = 0;
}

static inline mailbox
Mailbox(void* Memory, usize MemorySize) {
    mailbox Ret = {};
    Ret.Memory = (u8*)Memory;
    Ret.MemorySize = MemorySize;
    Clear(&Ret);
    return Ret;
}


// NOTE(Momo): Accessors and Iterators
static inline mailbox_entry_header*
GetEntry(mailbox* Mailbox, usize Index) {
    Assert(Index < Mailbox->EntryCount);
    return (mailbox_entry_header*)(Mailbox->EntryMemoryStart - Index * sizeof(mailbox_entry_header));
}


static inline void*
GetDataFromEntry(mailbox* Mailbox, mailbox_entry_header* Entry) {
    return (Mailbox->Memory + Entry->OffsetToData);
}


template<typename T>
static inline T*
Push(mailbox* Mailbox) 
{
    // Allocate Data
    u8 DataAdjust = AlignForwardDiff(Mailbox->DataMemoryAt, alignof(T));
    u32 DataSize = sizeof(T);
    
    // Allocate Entry
    u8 EntryAdjust = AlignBackwardDiff(Mailbox->EntryMemoryAt, alignof(mailbox_entry_header));
    u32 EntrySize = sizeof(mailbox_entry_header);
   
    if (Mailbox->EntryMemoryAt - EntrySize - EntryAdjust < Mailbox->DataMemoryAt + DataSize +  DataAdjust) {
        return nullptr; 
    }
    
    auto* Data = (T*)((u8*)Mailbox->DataMemoryAt + DataAdjust);
    Mailbox->DataMemoryAt += DataSize + DataAdjust;
    
    auto* Entry = (mailbox_entry_header*)((u8*)Mailbox->EntryMemoryAt + EntryAdjust);
    Entry->OffsetToData = (u32)((u8*)Data - Mailbox->Memory);
    Entry->Type = T::TypeId; 
    Mailbox->EntryMemoryAt -= EntrySize;
    ++Mailbox->EntryCount;
    
    return Data;
}

#endif
