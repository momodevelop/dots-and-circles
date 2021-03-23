#ifndef __GAME_DEBUG_INSPECTOR_H__
#define __GAME_DEBUG_INSPECTOR_H__

enum debug_inspector_var_type {
    DebugVariableType_B8,
    DebugVariableType_B32,
    DebugVariableType_I32,
    DebugVariableType_U32,
    DebugVariableType_F32,
    // TODO: Vector types
};

struct debug_inspector_var {
    debug_inspector_var_type Type;
    u8_cstr Name;
    union {
        void* Data;
        b8* B8;
        b32* B32;
        i32* I32;
        u32* U32;
        f32* F32;
    };
}; 


struct debug_inspector {
    b32 IsActive;
    debug_inspector_var* Vars;
    u32 VarCount;
    u32 VarCap;
};


static inline debug_inspector
DebugInspector_Create(arena* Arena, u32 VariableCap) {
    debug_inspector Ret = {};
    Ret.VarCap = VariableCap;
    Ret.VarCount = 0;
    Ret.Vars = Arena_PushArray(debug_inspector_var, 
                               Arena,
                               Ret.VarCap);
    Ret.IsActive = False;
    return Ret;
}

static inline void
DebugInspector_HookVariable(debug_inspector* Inspector, 
                            u8_cstr Name, 
                            void* Address, 
                            debug_inspector_var_type Type) {
    debug_inspector_var Var = {};
    Var.Data = Address;
    Var.Type = Type;
    Var.Name = Name;
    
    Assert(Inspector->VarCount < Inspector->VarCap);
    Inspector->Vars[Inspector->VarCount++] =  Var;
}

static inline void
DebugInspector_HookU32Variable(debug_inspector* Inspector,
                               u8_cstr Name,
                               void* Address) 
{
    DebugInspector_HookVariable(Inspector, Name, Address, DebugVariableType_U32);
}

static inline void
DebugInspector_UnhookVariable(debug_inspector* State, u8_cstr Name) {
    for (u32 I = 0; I < State->VarCount; ++I) {
        if (U8CStr_Compare(State->Vars[I].Name, Name)) {
            State->Vars[I] = State->Vars[State->VarCount-1];
            --State->VarCount;
            return;
        }
    }
}



static inline void
DebugInspector_Render(debug_inspector* Inspector,
                      arena* Arena,
                      game_assets* Assets,
                      mailbox* RenderCommands) 

{
    // For each variable, render:
    // Name: Data
    f32 OffsetY = 0.f;
    for (u32 I = 0; I < Inspector->VarCount; ++I) {
        arena_mark Scratch = Arena_Mark(Arena);
        Defer{ Arena_Revert(&Scratch); };
        
        u8_str Buffer = U8Str_CreateFromArena(Scratch.Arena, 256);
        U8Str_PushCStr(&Buffer, Inspector->Vars[I].Name);
        U8Str_PushCStr(&Buffer, U8CStr_FromSiStr(": "));
        
        Assert(Inspector->Vars[I].Data);
        switch(Inspector->Vars[I].Type) {
            case DebugVariableType_B32: {
            } break;
            case DebugVariableType_I32: {
                U8Str_PushI32(&Buffer, *Inspector->Vars[I].I32);
            } break;
            case DebugVariableType_F32: {
                Assert(false); 
            } break;
            case DebugVariableType_U32: {
                U8Str_PushU32(&Buffer, *Inspector->Vars[I].U32);
            } break;
            default: {
                // Unsupported type
                Assert(false);
            }
        }
        
        DrawText(RenderCommands, 
                 Assets, 
                 Font_Default, 
                 v3f{ -800.f + 10.f, 450.f - 32.f + OffsetY, 0.f }, 
                 Buffer.Str,
                 32.f, 
                 Color_White);
        U8Str_Clear(&Buffer);
        OffsetY -= 32.f;
    }
}

#endif 