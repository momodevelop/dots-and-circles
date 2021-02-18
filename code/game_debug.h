#ifndef __GAME_DEBUG__
#define __GAME_DEBUG__

#include "mm_core.h"
#include "mm_mailbox.h"
#include "mm_string.h"
#include "mm_arena.h"
#include "game_assets.h"
#include "game_draw.h"
#include "game_debug_console.h"

enum debug_variable_type {
    DebugVariableType_B8,
    DebugVariableType_B32,
    DebugVariableType_I32,
    DebugVariableType_U32,
    DebugVariableType_F32,
    // TODO: Vector types
};

struct debug_variable {
    debug_variable_type Type;
    string Name;
    union {
        void* Data;
        b8* B8;
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
HookVariable(debug_state* State, 
             string Name, 
             void* Address, 
             debug_variable_type Type) {
    debug_variable Var = {};
    Var.Data = Address;
    Var.Type = Type;
    Var.Name = Name;
    Push(&State->Variables, Var);
}

static inline void
HookU32Variable(debug_state* State,
                string Name,
                void* Address) 
{
    HookVariable(State, Name, Address, DebugVariableType_U32);
}

static inline void
UnhookVariable(debug_state* State, string Name) {
    RemoveIf(&State->Variables, [=](debug_variable* Var) {
        return Name == Var->Name; 
    });

}

static inline void
UnhookAllVariables(debug_state* State) {
    Clear(&State->Variables);
}

static inline void
Render(debug_state* State,
       game_assets* Assets,
       mailbox* RenderCommands) 

{
    // Render console system
    Render(&State->Console, RenderCommands, Assets);

    // For each variable, render:
    // Name: Data
    for (usize I = 0; I < State->Variables.Count; ++I) {
        scratch Scratchpad = BeginScratch(&State->Arena);
        Defer{ EndScratch(&Scratchpad); };

        string_buffer Buffer = StringBuffer(Scratchpad, 256);
        Push(&Buffer, State->Variables[I].Name);
        Push(&Buffer, String(": "));

        Assert(State->Variables[I].Data);
        switch(State->Variables[I].Type) {
            case DebugVariableType_B32: {
            } break;
            case DebugVariableType_I32: {
                PushI32(&Buffer, *State->Variables[I].I32);
            } break;
            case DebugVariableType_F32: {
                Assert(false); 
            } break;
            case DebugVariableType_U32: {
                PushU32(&Buffer, *State->Variables[I].U32);
            } break;
            default: {
                // Unsupported type
                Assert(false);
            }
        }

        DrawText(
             RenderCommands, 
             Assets, 
             Font_Default, 
             v3f{ -800.f + 10.f, 450.f - 32.f, 0.f }, 
             Buffer,
             32.f, 
             Color_White 
        );
        Clear(&Buffer);
    }
}

#endif
