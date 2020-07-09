#ifndef __GAME_RENDERER__
#define __GAME_RENDERER__


#if 0
#include "ryoji_bitmanip.h"



enum struct render_command_type {
    Clear,
    Textured_Quads
        
};

struct render_command_textured_quads {
    u32 TextureHandle; // ???
    m44f Transform;
    m44f Color;
};

struct render_command_header {
    render_command_entry_type Type;
};

struct render_commands {
    u8* Memory;
    u32 Used;
    u32 Capacity;
};

static inline u8*
Push(render_commands* Commands, u32 Size) {
    
    u8* BufferEnd = Commands->BufferBase + Commands->BufferSize;
    if ((Commands->BufferAt + Size) > BufferEnd) {
        return {}; // assert?
    }
    u8* Result = Commands->BufferAt;
    Commands->BufferAt += Size;
    
    return Result;
}

static inline void
Push(render_commands* Commands, u32 Size, render_command_type Type) {
    Size += sizeof(render_command_header);
    
}
#define (RenderComm
#endif

#endif //GAME_RENDERER_H
