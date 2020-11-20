// TODO we should really call this something else

#ifndef __MOMO_COMMANDS_H__
#define __MOMO_COMMANDS_H__


#include "mm_maths.h"
#include "mm_arena.h"

struct mmcmd_entry_header {
    u32 Type;
    u32 OffsetToData;
};

struct mmcmd_commands {
    u8* Memory;
    u8* DataMemoryAt;
    u8* EntryMemoryStart;
    u8* EntryMemoryAt;   
    u32 MemorySize;
    u32 EntryCount;
};

static inline void
mmcmd_Clear(mmcmd_commands* Commands) {
    Commands->DataMemoryAt = Commands->Memory;
    
    u8* EntryMemoryStart = Commands->Memory + Commands->MemorySize - sizeof(mmcmd_entry_header);
    u8 Adjust = AlignBackwardDiff(EntryMemoryStart, alignof(mmcmd_entry_header));
    EntryMemoryStart -= Adjust;
    
    Commands->EntryMemoryStart = EntryMemoryStart;
    Commands->EntryMemoryAt = EntryMemoryStart;
    
    Commands->EntryCount = 0;
}

static inline mmcmd_commands
mmcmd_Commands(void* Memory, u32 MemorySize) {
    mmcmd_commands Ret = {};
    Ret.Memory = (u8*)Memory;
    Ret.MemorySize = MemorySize;
    mmcmd_Clear(&Ret);
    return Ret;
}


// NOTE(Momo): Accessors and Iterators
static inline mmcmd_entry_header 
mmcmd_GetEntry(mmcmd_commands Commands, usize Index) {
    Assert(Index < Commands.EntryCount);
    return *(mmcmd_entry_header*)(Commands.EntryMemoryStart - Index * sizeof(mmcmd_entry_header));
}


static inline void*
mmcmd_GetDataFromEntry(mmcmd_commands Commands, mmcmd_entry_header Entry) {
    return (Commands.Memory + Entry.OffsetToData);
}


template<typename T>
static inline T*
mmcmd_Push(mmcmd_commands* Commands) 
{
    // Allocate Data
    u8 DataAdjust = AlignForwardDiff(Commands->DataMemoryAt, alignof(T));
    u32 DataSize = sizeof(T);
    
    // Allocate Entry
    u8 EntryAdjust = AlignBackwardDiff(Commands->EntryMemoryAt, alignof(mmcmd_entry_header));
    u32 EntrySize = sizeof(mmcmd_entry_header);
    
    if (Commands->EntryMemoryAt - EntrySize - EntryAdjust < Commands->DataMemoryAt + DataSize +  DataAdjust) {
        return nullptr; 
    }
    
    auto* Data = (T*)((u8*)Commands->DataMemoryAt + DataAdjust);
    Commands->DataMemoryAt += DataSize + DataAdjust;
    
    auto* Entry = (mmcmd_entry_header*)((u8*)Commands->EntryMemoryAt + EntryAdjust);
    Entry->OffsetToData = (u32)((u8*)Data - Commands->Memory);
    Entry->Type = T::TypeId; 
    Commands->EntryMemoryAt -= EntrySize;
    ++Commands->EntryCount;
    
    return Data;
}

#endif
