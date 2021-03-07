#ifndef __GAME_CONSOLE__
#define __GAME_CONSOLE__

#include "mm_easing.h"
#include "mm_list.h"
#include "mm_colors.h"
#include "mm_timer.h"
#include "game_platform.h"
#include "game_renderer.h"
#include "game_assets.h"
#include "game_draw.h"

using debug_console_callback = void (*)(struct debug_console* Console, void* Context, string Args);

struct debug_console_command {
    string Key;
    debug_console_callback Callback;
    void* Context;
};

struct debug_console_string {
    string_buffer Buffer;
    c4f Color;
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
    array<debug_console_string> InfoBuffers;
    string_buffer InputBuffer;
    string_buffer CommandBuffer;
    
    // Backspace (to delete character) related
    // Maybe make an easing system?
    timer StartPopRepeatTimer;
    timer PopRepeatTimer;
    b32 IsStartPop;
    
    // Enter and Exit transitions for swag!
    v3f TransitStartPos;
    v3f TransitEndPos;
    timer TransitTimer;
    
    list<debug_console_command> Commands;
};

static inline void 
RegisterCommand(debug_console* Console, 
                string Key, 
                debug_console_callback Callback, 
                void* Context)
{
    debug_console_command Command = { Key, Callback, Context };
    Push(&Console->Commands, Command);
}

static inline void 
UnregisterCommand(debug_console* Console, string Key) {
    auto Lamb = [Key](const debug_console_command* Cmd) { 
        return Cmd->Key == Key; 
    };
    RemoveIf(&Console->Commands, Lamb);
}

static inline debug_console
DebugConsole_Create(arena* Arena, 
                    usize InfoLineCount, 
                    usize CharactersPerLine, 
                    usize CommandsCapacity,
                    v3f TransitStartPos,
                    v3f TransitEndPos,
                    f32 TransitDuration,
                    v2f Dimensions,
                    f32 StartPopRepeatDuration,
                    f32 PopRepeatDuration)
{
    debug_console Ret = {};
    
    Ret.InfoBuffers = CreateArray<debug_console_string>(Arena, InfoLineCount);
    for (usize I = 0; I < Ret.InfoBuffers.Count; ++I) {
        debug_console_string* InfoBuffer = Ret.InfoBuffers + I;
        InfoBuffer->Buffer = CreateStringBuffer(Arena, CharactersPerLine);
    }
    
    Ret.InputBuffer = CreateStringBuffer(Arena, CharactersPerLine);
    Ret.CommandBuffer = CreateStringBuffer(Arena, CharactersPerLine);
    
    Ret.Commands = CreateList<debug_console_command>(Arena, CommandsCapacity);
    Ret.TransitTimer = CreateTimer(TransitDuration);
    
    Ret.InfoBgColor = Color_Grey3;
    Ret.InfoTextDefaultColor = Color_White;
    Ret.InputBgColor = Color_Grey2;
    Ret.InputTextColor = Color_White;
    Ret.Dimensions = Dimensions;
    Ret.Position = TransitStartPos;
    Ret.TransitStartPos = TransitStartPos;
    Ret.TransitEndPos = TransitEndPos;
    Ret.StartPopRepeatTimer = CreateTimer(StartPopRepeatDuration);
    Ret.PopRepeatTimer = CreateTimer(PopRepeatDuration); 
    
    return Ret;
}


static inline void
PushInfo(debug_console* Console, string String, c4f Color) {
    for (usize I = 0; I < Console->InfoBuffers.Count - 1; ++I) {
        usize J = Console->InfoBuffers.Count - 1 - I;
        debug_console_string* Dest = Console->InfoBuffers + J;
        debug_console_string* Src = Console->InfoBuffers + J - 1;
        Copy(&Dest->Buffer, &Src->Buffer);
        Dest->Color = Src->Color;
    }
    Console->InfoBuffers[0].Color = Color;
    Clear(&Console->InfoBuffers[0].Buffer);
    Copy(&Console->InfoBuffers[0].Buffer, &String);
}

static inline void 
DebugConsole_Pop(debug_console* Console) {
    if (!IsEmpty(&Console->InputBuffer))
        Pop(&Console->InputBuffer);
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
        f32 P = EaseInQuad(Percent(Console->TransitTimer));
        v3f Delta = Console->TransitEndPos - Console->TransitStartPos; 
        Console->Position = Console->TransitStartPos + (P * Delta); 
    }
    
    if (Console->IsActive) {
        Tick(&Console->TransitTimer, DeltaTime);
        if (Input->Characters.Count > 0 && 
            Input->Characters.Count <= Remaining(Console->InputBuffer)) 
        {  
            Push(&Console->InputBuffer, Input->Characters);
        }
        
        // Remove character backspace logic
        if (IsDown(Input->ButtonBack)) {
            if(!Console->IsStartPop) {
                DebugConsole_Pop(Console);
                Console->IsStartPop = true;
                Reset(&Console->StartPopRepeatTimer);
                Reset(&Console->PopRepeatTimer);
            }
            else {
                if (IsEnd(Console->StartPopRepeatTimer)) {
                    if(IsEnd(Console->PopRepeatTimer)) {
                        DebugConsole_Pop(Console);
                        Reset(&Console->PopRepeatTimer);
                    }
                    Tick(&Console->PopRepeatTimer, DeltaTime);
                }
                Tick(&Console->StartPopRepeatTimer, DeltaTime);
            }
        }
        else {
            Console->IsStartPop = false; 
        }
        
        // Execute command
        if (IsPoked(Input->ButtonConfirm)) {
            PushInfo(Console, Console->InputBuffer, Color_White);
            Copy(&Console->CommandBuffer, &Console->InputBuffer);
            Clear(&Console->InputBuffer);
            
            range<usize> Range = { 0, Find(&Console->CommandBuffer, ' ') };
            string CommandStr = SubString(Console->CommandBuffer, Range); 
            
            // Send a command to a callback
            for (usize I = 0; I < Console->Commands.Count; ++I) {
                debug_console_command* Command = &Console->Commands[I];
                if (Command->Key == CommandStr) {
                    Command->Callback(Console, Command->Context, Console->CommandBuffer);
                }
            }
        }
    }
    else if (!Console->IsActive) {
        Untick(&Console->TransitTimer, DeltaTime);
    }
    
    
}

static inline void
Render(debug_console* Console, 
       mailbox* RenderCommands,
       game_assets* Assets) 
{
    if (IsBegin(Console->TransitTimer)) {
        return;
    }
    game_asset_font* Font = Assets->Fonts + Font_Default;
    
    f32 Bottom = Console->Position.Y - Console->Dimensions.H * 0.5f;
    f32 Left = Console->Position.X - Console->Dimensions.W * 0.5f;
    f32 LineHeight = Console->Dimensions.H / (Console->InfoBuffers.Count + 1);
    f32 FontSize = LineHeight * 0.9f;
    f32 FontHeight = GetHeight(Font) * FontSize;
    f32 PaddingHeight = (LineHeight - FontHeight) * 0.5f  + Abs(Font->Descent) * FontSize; 
    f32 PaddingWidth = Console->Dimensions.W * 0.005f;
    {
        m44f ScaleMatrix = M44f_Scale(Console->Dimensions.X, 
                                      Console->Dimensions.Y, 
                                      1.f);
        
        m44f PositionMatrix = M44f_Translation(Console->Position.X,
                                               Console->Position.Y,
                                               Console->Position.Z);
        m44f InfoBgTransform = PositionMatrix * ScaleMatrix;
        PushDrawQuad(RenderCommands, 
                     Console->InfoBgColor, 
                     InfoBgTransform);
    }
    
    {
        m44f ScaleMatrix = M44f_Scale(Console->Dimensions.W, LineHeight, 0.f);
        m44f PositionMatrix = M44f_Translation(Console->Position.X, 
                                               Bottom + LineHeight * 0.5f,
                                               Console->Position.Z + 0.01f);
        
        m44f InputBgTransform = PositionMatrix * ScaleMatrix;
        PushDrawQuad(RenderCommands, 
                     Console->InputBgColor, 
                     InputBgTransform);
    }
    
    // Draw text
    {
        for (u32 I = 0; I < Console->InfoBuffers.Count ; ++I) {
            v3f Position = {};
            Position.X = Left + PaddingWidth;
            Position.Y = Bottom + ((I+1) * LineHeight) + PaddingHeight;
            Position.Z = Console->Position.Z + 0.02f;
            
            DrawText(RenderCommands,
                     Assets,
                     Font_Default, 
                     Position,
                     Console->InfoBuffers[I].Buffer,
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
                     Console->InputBuffer,
                     FontSize,
                     Console->InputTextColor);
            
        }
        
    }
}





#endif
