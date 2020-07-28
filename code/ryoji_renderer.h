#ifndef __RYOJI_RENDERER__
#define __RYOJI_RENDERER__

#include "ryoji_maths.h"
#include "ryoji_arenas.h"

// TODO(Momo): Remove this
#include "game_assets.h"

// NOTE(Momo): Renderer Types
struct render_bitmap {
    u32 Width;
    u32 Height;
    
    // TODO(Momo): Maybe seperate this? Might need to consider more use cases 
    struct pixel {
        u8 Red, Green, Blue, Alpha;
    };
    pixel* Pixels;
};

struct render_command_entry {
    u32 Type;
    u32 OffsetToData;
};


struct render_command_queue {
    u8* Memory;
    u8* DataMemoryAt;
    u8* EntryMemoryStart;
    u8* EntryMemoryAt;   
    u32 MemorySize;
    u32 EntryCount;
};


static inline void
Clear(render_command_queue* Commands) {
    Commands->DataMemoryAt = Commands->Memory;
    
    u8* EntryMemoryStart = Commands->Memory + Commands->MemorySize - sizeof(render_command_entry);
    u8 Adjust = AlignBackwardDiff(EntryMemoryStart, alignof(render_command_entry));
    EntryMemoryStart -= Adjust;
    
    Commands->EntryMemoryStart = EntryMemoryStart;
    Commands->EntryMemoryAt = EntryMemoryStart;
    
    Commands->EntryCount = 0;
}

static inline void
Init(render_command_queue* Commands, void* Memory, u32 MemorySize) {
    Commands->Memory = (u8*)Memory;
    Commands->MemorySize = MemorySize;
    Clear(Commands);
}


// NOTE(Momo): Accessors and Iterators
static inline render_command_entry* 
GetEntry(render_command_queue* Commands, usize Index) {
    Assert(Index < Commands->EntryCount);
    return (render_command_entry*)(Commands->EntryMemoryStart - Index * sizeof(render_command_entry));
}


static inline void*
GetDataFromEntry(render_command_queue* Commands, render_command_entry* Entry) {
    return (Commands->Memory + Entry->OffsetToData);
}


// NOTE(Momo): Push functions
template<typename T>
static inline T*
PushCommand(render_command_queue* Commands, u32 SortKey) 
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

// TODO(Momo): Set TypeId to some compile time counter
struct render_command_data_clear {
    static constexpr u32 TypeId = 0;
    c4f Colors;
};

struct render_command_data_textured_quad {
    static constexpr u32 TypeId = 1;
    game_texture Texture;
    c4f Colors;
    m44f Transform;
};

static inline void
PushCommandClear(render_command_queue* Commands, c4f Colors) {
    using data_t = render_command_data_clear;
    auto* Data = PushCommand<data_t>(Commands, 0);
    Data->Colors = Colors;
}


// TODO(Momo): Change TextureHandle and TextureData to use game_texture
static inline void
PushCommandTexturedQuad(render_command_queue* Commands, c4f Colors, m44f Transform, game_texture Texture) 
{
    using data_t = render_command_data_textured_quad;
    auto* Data = PushCommand<data_t>(Commands, Texture.Handle+1);
    
    Data->Colors = Colors;
    Data->Transform = Transform;
    Data->Texture = Texture;
    
}

#endif //RYOJI_RENDERER_H
