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


struct render_command_header {
    u32 Type;
    render_command_header* Next;
    void* Entry;
};

struct render_commands {
    // NOTE(Momo): Linked List of commands
    render_command_header* Head;
    render_command_header* Tail;
    
    // NOTE(Momo): Memory for linked list above
    u8* Memory;
    u8* MemoryAt;
    u32 MemorySize;
    
    // TODO(Momo): Sorting?
};


static inline void
Init(render_commands* Commands, void* Memory, u32 MemorySize) {
    Commands->Head = nullptr;
    Commands->Tail = nullptr;
    Commands->Memory = (u8*)Memory;
    Commands->MemorySize = MemorySize;
    Commands->MemoryAt = (u8*)Memory;
}

static inline void
Clear(render_commands* Commands) {
    Commands->Head = nullptr;
    Commands->Tail = nullptr;
    Commands->MemoryAt = Commands->Memory;
}

static inline void *
PushBlockFront(render_commands* Commands, u32 Size, u8 Alignment) {
    Assert(Size && Alignment);
    u8 Adjust = AlignForwardDiff(Commands->Memory, Alignment);
    
    u8* End = Commands->Memory + Commands->MemorySize;
    if (Commands->MemoryAt + Size + Adjust > End) {
        return nullptr; 
    }
    u8* Result = Commands->MemoryAt;
    Commands->MemoryAt += Size;
    
    return Result;
}

template<typename T>
static inline T*
PushStructFront(render_commands* Commands) {
    return (T*)PushBlockFront(Commands, sizeof(T), alignof(T));
}

template<typename T>
static inline T*
PushCommand(render_commands* Commands) 
{
    // NOTE(Momo): Allocate header first
    auto* Header = PushStructFront<render_command_header>(Commands);
    Assert(Header);
    Header->Type = T::TypeId;
    Header->Next = nullptr;
    
    if( Commands->Tail != nullptr ) {
        render_command_header* PrevHeader = Commands->Tail;
        PrevHeader->Next = Header;
    }
    Commands->Tail = Header;
    
    if (Commands->Head == nullptr) {
        Commands->Head = Commands->Tail;
    }
    
    // NOTE(Momo): Then allocate 
    T* Ret  = PushStructFront<T>(Commands);
    Assert(Ret);
    Header->Entry = Ret;
    return Ret;
}

struct render_command_entry_clear {
    static constexpr u32 TypeId = 0;
    c4f Colors;
};

struct render_command_entry_textured_quad {
    static constexpr u32 TypeId = 1;
    game_texture Texture;
    c4f Colors;
    m44f Transform;
};


static inline void
PushCommandClear(render_commands* Commands, c4f Colors) {
    using T = render_command_entry_clear;
    auto Entry = PushCommand<T>(Commands);
    Entry->Colors = Colors;
}


// TODO(Momo): Change TextureHandle and TextureData to use game_texture
static inline void
PushCommandTexturedQuad(render_commands* Commands, c4f Colors, m44f Transform, game_texture Texture) 
{
    using T = render_command_entry_textured_quad;
    auto Entry = PushCommand<T>(Commands);
    
    Entry->Colors = Colors;
    Entry->Transform = Transform;
    Entry->Texture = Texture;
}

#endif //RYOJI_RENDERER_H
