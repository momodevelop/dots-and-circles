#ifndef __RYOJI_RENDER_COMMANDS_H__
#define __RYOJI_RENDER_COMMANDS_H__

#include "ryoji_maths.h"
#include "ryoji_arenas.h"

struct render_command_entry {
    u32 Type;
    u32 OffsetToData;
};


struct render_commands {
    u8* Memory;
    u8* DataMemoryAt;
    u8* EntryMemoryStart;
    u8* EntryMemoryAt;   
    u32 MemorySize;
    u32 EntryCount;
};


static inline void
Clear(render_commands* Commands) {
    Commands->DataMemoryAt = Commands->Memory;
    
    u8* EntryMemoryStart = Commands->Memory + Commands->MemorySize - sizeof(render_command_entry);
    u8 Adjust = AlignBackwardDiff(EntryMemoryStart, alignof(render_command_entry));
    EntryMemoryStart -= Adjust;
    
    Commands->EntryMemoryStart = EntryMemoryStart;
    Commands->EntryMemoryAt = EntryMemoryStart;
    
    Commands->EntryCount = 0;
}

static inline void
Init(render_commands* Commands, void* Memory, u32 MemorySize) {
    Commands->Memory = (u8*)Memory;
    Commands->MemorySize = MemorySize;
    Clear(Commands);
}


// NOTE(Momo): Accessors and Iterators
static inline render_command_entry* 
GetEntry(render_commands* Commands, usize Index) {
    Assert(Index < Commands->EntryCount);
    return (render_command_entry*)(Commands->EntryMemoryStart - Index * sizeof(render_command_entry));
}


static inline void*
GetDataFromEntry(render_commands* Commands, render_command_entry* Entry) {
    return (Commands->Memory + Entry->OffsetToData);
}


// NOTE(Momo): Push functions
template<typename T>
static inline T*
PushCommand(render_commands* Commands) 
{
    // NOTE(Momo): Allocate Data
    u8 DataAdjust = AlignForwardDiff(Commands->DataMemoryAt, alignof(T));
    u32 DataSize = sizeof(T);
    
    // NOTE(Momo): Allocate Entry
    u8 EntryAdjust = AlignBackwardDiff(Commands->EntryMemoryAt, alignof(render_command_entry));
    u32 EntrySize = sizeof(render_command_entry);
    
    if (Commands->EntryMemoryAt - EntrySize - EntryAdjust < Commands->DataMemoryAt + DataSize +  DataAdjust) {
        return nullptr; 
    }
    
    auto* Data = (T*)((u8*)Commands->DataMemoryAt + DataAdjust);
    Commands->DataMemoryAt += DataSize + DataAdjust;
    
    auto* Entry = (render_command_entry*)((u8*)Commands->EntryMemoryAt + EntryAdjust);
    Entry->OffsetToData = (u32)((u8*)Data - Commands->Memory);
    Entry->Type = T::TypeId; 
    Commands->EntryMemoryAt -= EntrySize;
    ++Commands->EntryCount;
    return Data;
}

#endif //RYOJI_RENDERER_H
