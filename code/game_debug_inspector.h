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
    u8 LabelBuffer[256];
    u8_str Label;
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
    debug_inspector_var Vars[32];
    u32 VarCount;
};


static inline void
DebugInspector_Init(debug_inspector* Inspector) {
    Inspector->VarCount = 0;
    Inspector->IsActive = False;
    
    for (u32 I = 0; I < ArrayCount(Inspector->Vars); ++I) {
        debug_inspector_var* Var = Inspector->Vars + I;
        Var->Label = U8Str_Create(Var->LabelBuffer, ArrayCount(Var->LabelBuffer));
    }
}

static inline void
DebugInspector_HookVariable(debug_inspector* Inspector, 
                            u8_cstr Label, 
                            void* Address, 
                            debug_inspector_var_type Type) 
{
    Assert(Inspector->VarCount < ArrayCount(Inspector->Vars));
    
    debug_inspector_var* Var = Inspector->Vars + Inspector->VarCount++;
    Var->Data = Address;
    Var->Type = Type;
    U8Str_Copy(&Var->Label, Label);
}

static inline void
DebugInspector_HookU32Variable(debug_inspector* Inspector,
                               u8_cstr Label,
                               void* Address) 
{
    DebugInspector_HookVariable(Inspector, Label, Address, DebugVariableType_U32);
}

static inline void
DebugInspector_UnhookVariable(debug_inspector* State, u8_cstr Label) {
    for (u32 I = 0; I < State->VarCount; ++I) {
        if (U8CStr_Compare(State->Vars[I].Label.Str, Label)) {
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
        U8Str_PushCStr(&Buffer, Inspector->Vars[I].Label.Str);
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