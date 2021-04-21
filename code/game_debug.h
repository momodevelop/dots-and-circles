#if 0

#ifndef __GAME_DEBUG__
#define __GAME_DEBUG__



struct debug_state {
    b32 IsInitialized;
    arena Arena;
    
    sdw
        debug_console Console;
    
    struct permanent_state* PermanentState;
    struct transient_state* TransientState;
};

static inline void
Debug_HookVariable(debug_state* State, 
                   u8_cstr Name, 
                   void* Address, 
                   debug_variable_type Type) {
    debug_variable Var = {};
    Var.Data = Address;
    Var.Type = Type;
    Var.Name = Name;
    
    Assert(State->VariableCount < State->VariableCapacity);
    State->Variables[State->VariableCount++] =  Var;
}

static inline void
Debug_HookU32Variable(debug_state* State,
                      u8_cstr Name,
                      void* Address) 
{
    Debug_HookVariable(State, Name, Address, DebugVariableType_U32);
}

static inline void
Debug_UnhookAllVariables(debug_state* State) {
    State->VariableCount = 0;
}

static inline void
Debug_Render(debug_state* State,
             assets* Assets,
             mailbox* RenderCommands) 

{
    // Render console system
    DebugConsole_Render(&State->Console, RenderCommands, Assets);
    
    
    // For each variable, render:
    // Name: Data
    for (u32 I = 0; I < State->VariableCount; ++I) {
        arena_mark Scratch = Arena_Mark(&State->Arena);
        Defer{ Arena_Revert(&Scratch); };
        
        u8_str Buffer = U8Str_CreateFromArena(Scratch.Arena, 256);
        U8Str_PushCStr(&Buffer, State->Variables[I].Name);
        U8Str_PushCStr(&Buffer, U8CStr_FromSiStr(": "));
        
        Assert(State->Variables[I].Data);
        switch(State->Variables[I].Type) {
            case DebugVariableType_B32: {
            } break;
            case DebugVariableType_I32: {
                U8Str_PushI32(&Buffer, *State->Variables[I].s32);
            } break;
            case DebugVariableType_F32: {
                Assert(false); 
            } break;
            case DebugVariableType_U32: {
                U8Str_PushU32(&Buffer, *State->Variables[I].U32);
            } break;
            default: {
                // Unsupported type
                Assert(false);
            }
        }
        
        DrawText(RenderCommands, 
                 Assets, 
                 Font_Default, 
                 v3f{ -800.f + 10.f, 450.f - 32.f, 0.f }, 
                 Buffer.Str,
                 32.f, 
                 Color_White);
        U8Str_Clear(&Buffer);
    }
}

#endif
#endif