#ifndef __GAME_RENDERER__
#define __GAME_RENDERER__

#include "ryoji_maths.h"
#include "ryoji_bitmanip.h"
#include "ryoji_colors.h"

#include "game_assets.h"

// TODO(Momo): Consider shifting this part to Ryoji?
struct render_command_header {
    u32 Type;
    render_command_header* Next;
    void* Entry;
};

struct render_commands {
    // NOTE(Momo): Linked List of commands
    render_command_header* Head;
    render_command_header* Tail ;
    
    // NOTE(Momo): Memory for linked list above
    u8* Memory;
    u8* MemoryCurrent;
    u32 MemorySize;
    
    // TODO(Momo): Camera transforms?
    // TODO(Momo): Sorting?
};

static inline void
Init(render_commands* Commands, void* Memory, u32 MemorySize) {
    Commands->Head = nullptr;
    Commands->Tail = nullptr;
    Commands->Memory = (u8*)Memory;
    Commands->MemorySize = MemorySize;
    Commands->MemoryCurrent = (u8*)Memory;
}

static inline void*
Allocate(render_commands* Commands, u32 Size, u8 Alignment) {
    Assert(Size && Alignment);
    u8 Adjust = AlignForwardDiff(Commands->Memory, Alignment);
    
    u8* End = Commands->Memory + Commands->MemorySize;
    if (Commands->MemoryCurrent + Size + Adjust > End) {
        return nullptr; 
    }
    u8* Result = Commands->MemoryCurrent;
    Commands->MemoryCurrent += Size;
    
    return Result;
}


static inline void*
Push(render_commands* Commands, u32 Type, u32 Size, u8 Alignment) 
{
    // NOTE(Momo): Allocate header first
    render_command_header* Header = (render_command_header*)Allocate(Commands, 
                                                                     sizeof(render_command_header), 
                                                                     alignof(render_command_header));
    Assert(Header);
    Header->Type = Type;
    Header->Next = nullptr;
    
    if( Commands->Tail != nullptr ) {
        render_command_header* PrevHeader = (render_command_header*)Commands->Tail;
        PrevHeader->Next = Header;
    }
    Commands->Tail = Header;
    
    if (Commands->Head == nullptr) {
        Commands->Head = Commands->Tail;
    }
    
    // NOTE(Momo): Then allocate 
    Header->Entry = (u8*)Allocate(Commands, Size, Alignment);
    Assert(Header->Entry);
    return Header->Entry;
}


// NOTE(Momo): From here it's game related code
enum render_command_type {
    RenderCommandType_Clear,
    RenderCommandType_ColoredQuad,
    RenderCommandType_TexturedQuad,
};

struct render_command_entry_clear {
    c4f Colors;
};



struct render_command_entry_textured_quad {
    game_texture Texture;
    c4f Colors;
    m44f Transform;
};

struct render_command_entry_colored_quad {
    c4f Colors;
    m44f Transform;
};


static inline void
PushCommandClear(render_commands* Commands, c4f Colors) {
    auto Entry = (render_command_entry_clear*)Push(Commands, 
                                                   RenderCommandType_Clear, 
                                                   sizeof(render_command_entry_clear), 
                                                   alignof(render_command_entry_clear));
    Entry->Colors = Colors;
}

static inline void
PushCommandColoredQuad(render_commands* Commands, c4f Colors, m44f Transform) {
    auto Entry = (render_command_entry_colored_quad*)Push(Commands, 
                                                          RenderCommandType_ColoredQuad, 
                                                          sizeof(render_command_entry_colored_quad), 
                                                          alignof(render_command_entry_colored_quad));
    
    Entry->Colors = Colors;
    Entry->Transform = Transform;
}



// TODO(Momo): Change TextureHandle and TextureData to use game_texture
static inline void
PushCommandTexturedQuad(render_commands* Commands, c4f Colors, m44f Transform, game_texture Texture) 
{
    auto Entry = (render_command_entry_textured_quad*)Push(Commands,
                                                           RenderCommandType_TexturedQuad,
                                                           sizeof(render_command_entry_textured_quad),
                                                           alignof(render_command_entry_textured_quad));
    Entry->Colors = Colors;
    Entry->Transform = Transform;
    Entry->Texture = Texture;
}

#endif //GAME_RENDERER_H
