#ifndef __RYOJI_COMMANDS_H__
#define __RYOJI_COMMANDS_H__

#include "ryoji_maths.h"
#include "ryoji_arenas.h"

struct command_entry_header {
    u32 Type;
    u32 OffsetToData;
};


struct commands {
    u8* Memory;
    u8* DataMemoryAt;
    u8* EntryMemoryStart;
    u8* EntryMemoryAt;   
    u32 MemorySize;
    u32 EntryCount;
};


static inline void
Clear(commands* Commands) {
    Commands->DataMemoryAt = Commands->Memory;
    
    u8* EntryMemoryStart = Commands->Memory + Commands->MemorySize - sizeof(command_entry_header);
    u8 Adjust = AlignBackwardDiff(EntryMemoryStart, alignof(command_entry_header));
    EntryMemoryStart -= Adjust;
    
    Commands->EntryMemoryStart = EntryMemoryStart;
    Commands->EntryMemoryAt = EntryMemoryStart;
    
    Commands->EntryCount = 0;
}

static inline void
Init(commands* Commands, void* Memory, u32 MemorySize) {
    Commands->Memory = (u8*)Memory;
    Commands->MemorySize = MemorySize;
    Clear(Commands);
}


// NOTE(Momo): Accessors and Iterators
static inline command_entry_header* 
GetEntry(commands* Commands, usize Index) {
    Assert(Index < Commands->EntryCount);
    return (command_entry_header*)(Commands->EntryMemoryStart - Index * sizeof(command_entry_header));
}


static inline void*
GetDataFromEntry(commands* Commands, command_entry_header* Entry) {
    return (Commands->Memory + Entry->OffsetToData);
}


// NOTE(Momo): Push functions
template<typename T>
static inline T*
Push(commands* Commands) 
{
    // NOTE(Momo): Allocate Data
    u8 DataAdjust = AlignForwardDiff(Commands->DataMemoryAt, alignof(T));
    u32 DataSize = sizeof(T);
    
    // NOTE(Momo): Allocate Entry
    u8 EntryAdjust = AlignBackwardDiff(Commands->EntryMemoryAt, alignof(command_entry_header));
    u32 EntrySize = sizeof(command_entry_header);
    
    if (Commands->EntryMemoryAt - EntrySize - EntryAdjust < Commands->DataMemoryAt + DataSize +  DataAdjust) {
        return nullptr; 
    }
    
    auto* Data = (T*)((u8*)Commands->DataMemoryAt + DataAdjust);
    Commands->DataMemoryAt += DataSize + DataAdjust;
    
    auto* Entry = (command_entry_header*)((u8*)Commands->EntryMemoryAt + EntryAdjust);
    Entry->OffsetToData = (u32)((u8*)Data - Commands->Memory);
    Entry->Type = T::TypeId; 
    Commands->EntryMemoryAt -= EntrySize;
    ++Commands->EntryCount;
    
    return Data;
}

#endif //RYOJI_RENDERER_H
