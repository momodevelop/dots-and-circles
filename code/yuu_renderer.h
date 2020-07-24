#ifndef __GAME_RENDER_COMMANDS__
#define __GAME_RENDER_COMMANDS__

#include "ryoji_maths.h"
#include "ryoji_colors.h"
#include "ryoji_arenas.h"


#include "yuu_assets.h"

// TODO(Momo): Consider shifting this part to Ryoji?
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
    memory_arena Arena;
    
    // TODO(Momo): Camera transforms?
    // TODO(Momo): Sorting?
};


static inline void
Init(render_commands* Commands, void* Memory, usize MemorySize) {
    Commands->Head = nullptr;
    Commands->Tail = nullptr;
    
    Init(&Commands->Arena, Memory, MemorySize);
}

static inline void
Clear(render_commands* Commands) {
    Clear(&Commands->Arena);
    Commands->Head = nullptr;
    Commands->Tail = nullptr;
}

template<typename T>
static inline T*
PushCommand(render_commands* Commands) 
{
    // NOTE(Momo): Allocate header first
    auto Header = PushStruct<render_command_header>(&Commands->Arena);
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
    T* Ret  = PushStruct<T>(&Commands->Arena);
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

#endif //GAME_RENDERER_H
