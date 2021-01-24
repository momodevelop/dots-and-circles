#ifndef __GAME_CONSOLE__
#define __GAME_CONSOLE__

#include "mm_list.h"
#include "mm_colors.h"
#include "mm_timer.h"
#include "game_platform.h"
#include "game_renderer.h"
#include "game_assets.h"
#include "game_text.h"

using debug_console_callback = void (*)(struct debug_console* Console, void* Context, string Arguments);
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
    RemoveIf(&Console->Commands, [Key](const debug_console_command* Cmd) { 
        return Cmd->Key == Key; 
    });
}

static inline debug_console
DebugConsole(arena* Arena, 
             usize InfoLines, 
             usize CharactersPerLine, 
             usize CommandsCapacity)
{
    debug_console Ret = {};

    Ret.InfoBuffers = Array<debug_console_string>(Arena, InfoLines);
    for (usize I = 0; I < Ret.InfoBuffers.Count; ++I) {
        debug_console_string* InfoBuffer = Ret.InfoBuffers + I;
        InfoBuffer->Buffer = StringBuffer(Arena, CharactersPerLine);
    }

    Ret.InputBuffer = StringBuffer(Arena, CharactersPerLine);
    Ret.CommandBuffer = StringBuffer(Arena, CharactersPerLine);

    Ret.Commands = List<debug_console_command>(Arena, CommandsCapacity);
    return Ret;
}


static inline void
PushInfo(debug_console* Console, string String, c4f Color) {
    for (usize I = 0; I < Console->InfoBuffers.Count - 1; ++I) {
        usize J = Console->InfoBuffers.Count - 1 - I;
        debug_console_string* Dest = Console->InfoBuffers + J;
        debug_console_string* Src = Console->InfoBuffers + J - 1;
        Copy(&Dest->Buffer, Src->Buffer.Array);
        Dest->Color = Src->Color;
    }
    Console->InfoBuffers[0].Color = Color;
    Clear(&Console->InfoBuffers[0].Buffer);
    Copy(&Console->InfoBuffers[0].Buffer, String);
}

static inline void 
Pop(debug_console* Console) {
    if (!IsEmpty(Console->InputBuffer.Array))
        Pop(&Console->InputBuffer);
}

// Returns true if there is a new command
static inline void 
Update(debug_console* Console, 
       input* Input,
       f32 DeltaTime) 
{
    if (IsPoked(Input->ButtonConsole)) {
        Console->IsActive = !Console->IsActive; 
    }

    if (!Console->IsActive) {
        return;
    }

    if (Input->Characters.Count > 0 && 
        Input->Characters.Count <= Remaining(Console->InputBuffer)) 
    {  
        Push(&Console->InputBuffer, Input->Characters.Array);
    }
    
    // Remove character backspace logic
    if (IsDown(Input->ButtonBack)) {
        if(!Console->IsStartPop) {
            Pop(Console);
            Console->IsStartPop = true;
            Reset(&Console->StartPopRepeatTimer);
            Reset(&Console->PopRepeatTimer);
        }
        else {
            if (IsTimeUp(Console->StartPopRepeatTimer)) {
                if(IsTimeUp(Console->PopRepeatTimer)) {
                    Pop(Console);
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
        string Arguments = Console->InputBuffer.Array;
        PushInfo(Console, Console->InputBuffer.Array, Color_White);
        Copy(&Console->CommandBuffer, Arguments);
        Clear(&Console->InputBuffer);

        range<usize> Range = { 0, Find(Arguments, ' ') };
        string CommandStr = SubString(Arguments, Range); 

        // Send a command to a callback
        for (usize I = 0; I < Console->Commands.Count; ++I) {
            debug_console_command* Command = &Console->Commands[I];
            if (Command->Key == CommandStr) {
                 Command->Callback(Console, Command->Context, Arguments);
            }
        }
    }

}

static inline void
Render(debug_console* Console, 
       mailbox* RenderCommands,
       game_assets* Assets) 
{
    if (!Console->IsActive) {
        return;
    }
    font* Font = Assets->Fonts + Font_Default;
    
    f32 Bottom = Console->Position.Y - Console->Dimensions.H * 0.5f;
    f32 Left = Console->Position.X - Console->Dimensions.W * 0.5f;
    f32 LineHeight = Console->Dimensions.H / (Console->InfoBuffers.Count + 1);
    f32 FontSize = LineHeight * 0.9f;
    f32 FontHeight = Height(Font) * FontSize;
    f32 PaddingHeight = (LineHeight - FontHeight) * 0.5f  + Abs(Font->Descent) * FontSize; 
    f32 PaddingWidth = Console->Dimensions.W * 0.005f;
    {
        m44f ScaleMatrix = M44fScale(Console->Dimensions);
        m44f PositionMatrix = M44fTranslation(Console->Position);
        m44f InfoBgTransform = PositionMatrix * ScaleMatrix;
        PushDrawQuad(RenderCommands, 
                     Console->InfoBgColor, 
                     InfoBgTransform);
    }

    {
        m44f ScaleMatrix = M44fScale(v2f{ Console->Dimensions.W, LineHeight });
        m44f PositionMatrix = M44fTranslation(
                Console->Position.X, 
                Bottom + LineHeight * 0.5f,
                Console->Position.Z + 0.01f
        );

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
                    Position,
                    Console->InfoBuffers[I].Color,
                    Font_Default, 
                    FontSize,
                    Console->InfoBuffers[I].Buffer.Array);
        }

        {
            v3f Position = {};
            Position.X = Left + PaddingWidth;
            Position.Y = Bottom + PaddingHeight;
            Position.Z = Console->Position.Z + 0.02f;

            DrawText(
                RenderCommands, 
                Assets, 
                Position,
                Console->InputTextColor,
                Font_Default, FontSize, 
                Console->InputBuffer.Array
            );
        
        }

    }
}





#endif
