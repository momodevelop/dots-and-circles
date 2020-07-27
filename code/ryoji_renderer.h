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

// TODO(Momo): Do I just put this into entry...?
struct render_command_header {
    u32 Type;
    u32 OffsetToData;
};

struct render_command_entry {
    u32 SortKey;
    u32 OffsetToHeader;
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

static inline render_command_header*
GetHeaderFromEntry(render_command_queue* Commands, render_command_entry* Entry) {
    return (render_command_header*)(Commands->Memory + Entry->OffsetToHeader);
}

static inline void*
GetDataFromHeader(render_command_queue* Commands, render_command_header* Header) {
    return (Commands->Memory + Header->OffsetToData);
}

static inline void
Sort(render_command_queue* Commands) {
    // TODO(Momo): Please use a better swap T_T
    // NOTE(Momo): poo-poo Bubble sort :(
    u32 N = Commands->EntryCount;
    for (u32 i = 0; i < N-1; i++)      
        for (u32 j = 0; j < N-i-1; j++) {
        auto* Lhs = GetEntry(Commands, j);
        auto* Rhs = GetEntry(Commands, j+1);
        if (Lhs->SortKey > Rhs->SortKey) {
            auto Temp = *Lhs;
            *Lhs = *Rhs;
            *Rhs = Temp;
        }
    }
}



// NOTE(Momo): Push functions
static inline void*
PushBlockFront(render_command_queue* Commands, u32 Size, u8 Alignment) {
    Assert(Size && Alignment);
    u8 Adjust = AlignForwardDiff(Commands->DataMemoryAt, Alignment);
    Size += Adjust;
    
    Assert(Commands->DataMemoryAt + Size <= Commands->EntryMemoryAt);
    if (Commands->DataMemoryAt + Size > Commands->EntryMemoryAt) {
        return nullptr; 
    }
    
    u8* Result = Commands->DataMemoryAt;
    Commands->DataMemoryAt += Size;
    
    return Result;
}


static inline void*
PushBlockBack(render_command_queue* Commands, u32 Size, u8 Alignment) {
    Assert(Size && Alignment);
    u8 Adjust = AlignBackwardDiff(Commands->EntryMemoryAt - Size, Alignment);
    Size += Adjust;
    
    Assert(Commands->EntryMemoryAt - Size > Commands->DataMemoryAt);
    if (Commands->EntryMemoryAt - Size < Commands->DataMemoryAt) {
        return nullptr; 
    }
    
    u8* Result = Commands->EntryMemoryAt;
    Commands->EntryMemoryAt -= Size;
    return Result;
    
}

template<typename T>
static inline T*
PushStructFront(render_command_queue* Commands) {
    return (T*)PushBlockFront(Commands, sizeof(T), alignof(T));
}


template<typename T>
static inline T*
PushStructBack(render_command_queue* Commands) {
    return (T*)PushBlockBack(Commands, sizeof(T), alignof(T));
}

template<typename T>
static inline T*
PushCommand(render_command_queue* Commands, u32 SortKey) 
{
#if 1
    // NOTE(Momo): Allocate Header
    u32 HeaderSize = sizeof(render_command_header);
    u8 HeaderAdjust = AlignForwardDiff(Commands->DataMemoryAt, alignof(render_command_header));
    HeaderSize += HeaderAdjust;
    
    // TODO(Momo): Put below
    auto* Header = (render_command_header*)Commands->DataMemoryAt;
    Commands->DataMemoryAt += HeaderSize;
    Header->Type = T::TypeId;
    
    
    // NOTE(Momo): Allocate Data
    u32 DataSize = sizeof(T);
    u8 DataAdjust = AlignForwardDiff(Commands->DataMemoryAt, alignof(T));
    DataSize += DataAdjust;
    
    // TODO(Momo): Put below
    auto* Data = (T*)Commands->DataMemoryAt;
    Commands->DataMemoryAt += DataSize;
    Header->OffsetToData = (u32)((u8*)Data - Commands->Memory);
    
    
    // NOTE(Momo): Allocate Entry
    u32 EntrySize = sizeof(render_command_entry);
    u8 EntryAdjust = AlignBackwardDiff(Commands->EntryMemoryAt, alignof(render_command_entry));
    EntrySize += EntryAdjust;
    
    /*
if (Commands->EntryMemoryAt - EntrySize - EntryAdjust < Commands->DataMemoryAt + DataSize + HeaderSize + HeaderAdjust + DataAdjust) {
        return nullptr; 
    }*/
    
    auto* Entry = (render_command_entry*)(Commands->EntryMemoryAt);
    Entry->OffsetToHeader = (u32)((u8*)Header - Commands->Memory);
    Entry->SortKey = SortKey;
    Commands->EntryMemoryAt -= EntrySize;
    
    ++Commands->EntryCount;
    
    return Data;
#else
    // NOTE(Momo): Allocate header first
    // TODO(Momo): Not really very nice. Each push might need to be reverted. 
    auto* Header = PushStructFront<render_command_header>(Commands);
    Assert(Header);
    Header->Type = T::TypeId;
    
    // NOTE(Momo): Allocate the command data (the entry) 
    T* Ret  = PushStructFront<T>(Commands);
    Assert(Ret);
    Header->OffsetToData = (u32)((u8*)Ret - Commands->Memory);
    
    
    // NOTE(Momo): Allocate the sort
    auto* Entry = PushStructBack<render_command_entry>(Commands);
    Assert(Entry);
    Entry->OffsetToHeader = (u32)((u8*)Header - Commands->Memory);
    Entry->SortKey = SortKey;
    
    ++Commands->EntryCount;
    return Ret;
#endif
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
