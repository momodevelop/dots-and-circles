#ifndef __GAME_DEBUG__
#define __GAME_DEBUG__

#include "mm_core.h"
#include "mm_mailbox.h"
#include "mm_string.h"
#include "mm_arena.h"
#include "game_assets.h"
#include "game_text.h"
#include "game_debug_console.h"

enum debug_variable_type {
    DebugInspectorVariableType_B32,
    DebugInspectorVariableType_I32,
    DebugInspectorVariableType_U32,
    DebugInspectorVariableType_F32,
    // TODO: Vector types
};

struct debug_variable {
    debug_variable_type Type;
    string Name;
    union {
        void* Data;
        b32* B32;
        i32* I32;
        u32* U32;
        f32* F32;
    };
}; 

struct debug_state {
    b32 IsInitialized;
    arena Arena;
   
    list<debug_variable> Variables; 
    debug_console Console;

    struct permanent_state* PermanentState;
    struct transient_state* TransientState;
};

static inline void
HookVariable(debug_state* State) {
    // TODO
}

static inline void
UnhookVariable(debug_state* State) {
    // TODO
}

static inline void
Render(debug_state* State,
       game_assets* Assets,
       mailbox* RenderCommands,
       arena* Arena) 

{
    Render(&State->Console, RenderCommands, Assets);

    // For each variable, render:
    // Name: Data
    for (usize I = 0; I < State->Variables.Count; ++I) {
        scratch Scratchpad = BeginScratch(Arena);
        Defer{ EndScratch(&Scratchpad); };

        string_buffer Buffer = StringBuffer(Scratchpad, 256);
        Push(&Buffer, State->Variables[I].Name);
        Push(&Buffer, String(": "));

        //PushI32(&Buffer, (i32)Mode->Bullets.Count);
        DrawText(RenderCommands, 
                 Assets, 
                 v3f{ -800.f + 10.f, 450.f - 32.f, 0.f }, 
                 Color_White, 
                 Font_Default, 
                 32.f, 
                 Buffer.Array);
        Clear(&Buffer);
    }
}

#endif
