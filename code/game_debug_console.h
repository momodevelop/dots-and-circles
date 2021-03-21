#ifndef __GAME_CONSOLE__
#define __GAME_CONSOLE__

#include "mm_easing.h"
#include "mm_colors.h"
#include "mm_timer.h"
#include "game_platform.h"
#include "game_renderer.h"
#include "game_assets.h"
#include "game_draw.h"


typedef void (*debug_console_callback) (struct debug_console* Console, void* Context, u8_cstr Args);

struct debug_console_command {
    u8_str Key;
    debug_console_callback Callback;
    void* Context;
};

struct debug_console_buffer {
    u8_str Buffer;
    c4f Color;
};

struct debug_console_command_list {
    u32 Capacity;
    u32 Count;
    debug_console_command* E;
};


struct debug_console {
    b32 IsActive;
    
    c4f InfoBgColor;
    c4f InputBgColor;
    c4f InputTextColor;
    c4f InfoTextDefaultColor;
    v3f Position;
    v2f Dimensions;
    
    // Buffers
    debug_console_buffer* InfoBuffers;
    u32 InfoBufferCount;
    
    
    u8_str InputBuffer;
    u8_str CommandBuffer;
    
    // Backspace (to delete character) related
    // Maybe make an easing system?
    timer StartPopRepeatTimer;
    timer PopRepeatTimer;
    b32 IsStartPop;
    
    // Enter and Exit transitions for swag!
    v3f TransitStartPos;
    v3f TransitEndPos;
    timer TransitTimer;
    
    // List of commands
    debug_console_command* Commands;
    u32 CommandCount;
    u32 CommandCapacity;
};

static inline void 
DebugConsole_AddCmd(debug_console* C, 
                    u8_str Key, 
                    debug_console_callback Callback, 
                    void* Context)
{
    debug_console_command Command = { Key, Callback, Context };
    
    Assert(C->CommandCount < C->CommandCapacity);
    C->Commands[C->CommandCount] = Command; 
    ++C->CommandCount;
}


static inline void 
DebugConsole_RemoveCmd(debug_console* Console, u8_cstr Key) {
    for (u32 I = 0; I = Console->CommandCount; ++I) {
        if (U8CStr_Compare(Console->Commands[I].Key.Str, Key)) {
            // Swap with last element
            Console->Commands[I] = Console->Commands[Console->CommandCount-1];
            --Console->CommandCount;
            return;
        }
    }
    
}

static inline debug_console
DebugConsole_Create(arena* Arena, 
                    u32 InfoLineCount, 
                    u32 CharactersPerLine, 
                    u32 CommandsCapacity,
                    v3f TransitStartPos,
                    v3f TransitEndPos,
                    f32 TransitDuration,
                    v2f Dimensions,
                    f32 StartPopRepeatDuration,
                    f32 PopRepeatDuration)
{
    debug_console Ret = {};
    
    Ret.InfoBuffers = Arena_PushArray(debug_console_buffer, Arena, InfoLineCount);
    Ret.InfoBufferCount = InfoLineCount;
    
    for (usize I = 0; I < Ret.InfoBufferCount; ++I) {
        debug_console_buffer* InfoBuffer = Ret.InfoBuffers + I;
        InfoBuffer->Buffer = U8Str_CreateFromArena(Arena, CharactersPerLine);
    }
    
    Ret.InputBuffer = U8Str_CreateFromArena(Arena, CharactersPerLine);
    Ret.CommandBuffer = U8Str_CreateFromArena(Arena, CharactersPerLine);
    
    Ret.Commands = Arena_PushArray(debug_console_command, Arena, CommandsCapacity);
    Ret.CommandCapacity = CommandsCapacity;
    Ret.CommandCount = 0;
    
    Ret.TransitTimer = Timer_Create(TransitDuration);
    
    Ret.InfoBgColor = Color_Grey3;
    Ret.InfoTextDefaultColor = Color_White;
    Ret.InputBgColor = Color_Grey2;
    Ret.InputTextColor = Color_White;
    Ret.Dimensions = Dimensions;
    Ret.Position = TransitStartPos;
    Ret.TransitStartPos = TransitStartPos;
    Ret.TransitEndPos = TransitEndPos;
    Ret.StartPopRepeatTimer = Timer_Create(StartPopRepeatDuration);
    Ret.PopRepeatTimer = Timer_Create(PopRepeatDuration); 
    
    return Ret;
}


static inline void
PushInfo(debug_console* Console, u8_cstr String, c4f Color) {
    for (u32 I = 0; I < Console->InfoBufferCount - 1; ++I) {
        u32 J = Console->InfoBufferCount - 1 - I;
        debug_console_buffer* Dest = Console->InfoBuffers + J;
        debug_console_buffer* Src = Console->InfoBuffers + J - 1;
        U8Str_Copy(&Dest->Buffer, Src->Buffer.Str);
        Dest->Color = Src->Color;
    }
    Console->InfoBuffers[0].Color = Color;
    U8Str_Clear(&Console->InfoBuffers[0].Buffer);
    U8Str_Copy(&Console->InfoBuffers[0].Buffer, String);
}

static inline void 
DebugConsole_Pop(debug_console* Console) {
    if (Console->InputBuffer.Size != 0)
        U8Str_Pop(&Console->InputBuffer);
}

// Returns true if there is a new command
static inline void 
DebugConsole_Update(debug_console* Console, 
                    game_input* Input,
                    f32 DeltaTime) 
{
    if (IsPoked(Input->ButtonConsole)) {
        Console->IsActive = !Console->IsActive; 
    }
    
    // Transition
    {
        f32 P = EaseInQuad(Timer_Percent(Console->TransitTimer));
        v3f Delta = V3f_Sub(Console->TransitEndPos, Console->TransitStartPos); 
        v3f DeltaP = V3f_Mul(Delta, P);
        Console->Position = V3f_Add(Console->TransitStartPos, DeltaP); 
    }
    
    if (Console->IsActive) {
        Timer_Tick(&Console->TransitTimer, DeltaTime);
        if (Input->Characters.Size > 0 && 
            Input->Characters.Size <= U8Str_Remaining(&Console->InputBuffer)) 
        {  
            U8Str_PushCStr(&Console->InputBuffer, Input->Characters.Str);
        }
        
        // Remove character backspace logic
        if (IsDown(Input->ButtonBack)) {
            if(!Console->IsStartPop) {
                DebugConsole_Pop(Console);
                Console->IsStartPop = true;
                Timer_Reset(&Console->StartPopRepeatTimer);
                Timer_Reset(&Console->PopRepeatTimer);
            }
            else {
                if (Timer_IsEnd(Console->StartPopRepeatTimer)) {
                    if(Timer_IsEnd(Console->PopRepeatTimer)) {
                        DebugConsole_Pop(Console);
                        Timer_Reset(&Console->PopRepeatTimer);
                    }
                    Timer_Tick(&Console->PopRepeatTimer, DeltaTime);
                }
                Timer_Tick(&Console->StartPopRepeatTimer, DeltaTime);
            }
        }
        else {
            Console->IsStartPop = false; 
        }
        
        // Execute command
        if (IsPoked(Input->ButtonConfirm)) {
            PushInfo(Console, Console->InputBuffer.Str, Color_White);
            U8Str_Copy(&Console->CommandBuffer, Console->InputBuffer.Str);
            U8Str_Clear(&Console->InputBuffer);
            
            u32 Min = 0;
            u32 Max = 0;
            for (u32 I = 0; I < Console->CommandBuffer.Size; ++I) {
                if (Console->CommandBuffer.Data[I] == ' ') {
                    Max = I;
                    break;
                }
            }
            u8_cstr CommandStr = U8Str_SubString(Console->CommandBuffer.Str, Min, Max); 
            
            // Send a command to a callback
            for (usize I = 0; I < Console->CommandCount; ++I) {
                debug_console_command* Command = Console->Commands + I;
                if (U8CStr_Compare(Command->Key.Str, CommandStr)) {
                    Command->Callback(Console, Command->Context, Console->CommandBuffer.Str);
                }
            }
        }
    }
    else if (!Console->IsActive) {
        Timer_Untick(&Console->TransitTimer, DeltaTime);
    }
    
    
}

static inline void
Render(debug_console* Console, 
       mailbox* RenderCommands,
       game_assets* Assets) 
{
    if (Timer_IsBegin(Console->TransitTimer)) {
        return;
    }
    game_asset_font* Font = Assets->Fonts + Font_Default;
    
    f32 Bottom = Console->Position.Y - Console->Dimensions.H * 0.5f;
    f32 Left = Console->Position.X - Console->Dimensions.W * 0.5f;
    f32 LineHeight = Console->Dimensions.H / (Console->InfoBufferCount + 1);
    f32 FontSize = LineHeight * 0.9f;
    f32 FontHeight = GetHeight(Font) * FontSize;
    f32 PaddingHeight = (LineHeight - FontHeight) * 0.5f  + AbsOf(Font->Descent) * FontSize; 
    f32 PaddingWidth = Console->Dimensions.W * 0.005f;
    {
        m44f ScaleMatrix = M44f_Scale(Console->Dimensions.X, 
                                      Console->Dimensions.Y, 
                                      1.f);
        
        m44f PositionMatrix = M44f_Translation(Console->Position.X,
                                               Console->Position.Y,
                                               Console->Position.Z);
        m44f InfoBgTransform = M44f_Concat(PositionMatrix, ScaleMatrix);
        PushDrawQuad(RenderCommands, 
                     Console->InfoBgColor, 
                     InfoBgTransform);
    }
    
    {
        m44f ScaleMatrix = M44f_Scale(Console->Dimensions.W, LineHeight, 0.f);
        m44f PositionMatrix = M44f_Translation(Console->Position.X, 
                                               Bottom + LineHeight * 0.5f,
                                               Console->Position.Z + 0.01f);
        
        m44f InputBgTransform = M44f_Concat(PositionMatrix, ScaleMatrix);
        PushDrawQuad(RenderCommands, 
                     Console->InputBgColor, 
                     InputBgTransform);
    }
    
    // Draw text
    {
        for (u32 I = 0; I < Console->InfoBufferCount ; ++I) {
            v3f Position = {};
            Position.X = Left + PaddingWidth;
            Position.Y = Bottom + ((I+1) * LineHeight) + PaddingHeight;
            Position.Z = Console->Position.Z + 0.02f;
            
            DrawText(RenderCommands,
                     Assets,
                     Font_Default, 
                     Position,
                     Console->InfoBuffers[I].Buffer.Str,
                     FontSize,
                     Console->InfoBuffers[I].Color);
        }
        
        {
            v3f Position = {};
            Position.X = Left + PaddingWidth;
            Position.Y = Bottom + PaddingHeight;
            Position.Z = Console->Position.Z + 0.02f;
            
            DrawText(RenderCommands, 
                     Assets, 
                     Font_Default, 
                     Position,
                     Console->InputBuffer.Str,
                     FontSize,
                     Console->InputTextColor);
            
        }
        
    }
}





#endif
