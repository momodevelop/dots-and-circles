#ifndef __GAME_CONSOLE__
#define __GAME_CONSOLE__

#include "mm_list.h"
#include "mm_colors.h"
#include "game_platform.h"
#include "game_renderer.h"
#include "game_assets.h"
#include "game_text.h"

using game_console_callback = void (*)(void* Context, string Arguments);
struct game_console_command {
    string Key;
    game_console_callback Callback;
    void* Context;
};

struct game_console_string {
    string_buffer Buffer;
    c4f Color;
};


struct game_console {
    b32 IsActive;
    c4f InfoBgColor;
    c4f InputBgColor;
    c4f InputTextColor;
    c4f InfoTextDefaultColor;
    v3f Position;
    v2f Dimensions;

    // Buffers
    array<game_console_string> InfoBuffers;
    string_buffer InputBuffer;
    string_buffer CommandBuffer;

    list<game_console_command> Commands;
};



static inline void 
RegisterCommand(game_console* Console, 
                string Key, 
                game_console_callback Callback, 
                void* Context)
{
    game_console_command Command = { Key, Callback, Context };
    Push(&Console->Commands, Command);
}

static inline void 
UnregisterCommand(game_console* Console, string Key) {
    RemoveIf(&Console->Commands, [Key](const game_console_command* Cmd) { 
        return Cmd->Key == Key; 
    });
}

static inline b32 
Execute(game_console* Console, string Arguments) {
    // Assume that the first token is the command     
    range<usize> Range = { 0, Find(Arguments, ' ') };
    string CommandStr = SubString(Arguments, Range); 

    // Send a command to a callback
    for (usize I = 0; I < Console->Commands.Count; ++I) {
        game_console_command* Command = &Console->Commands[I];
        if (Command->Key == CommandStr) {
             Command->Callback(Command->Context, Arguments);
             return true;
        }
    }

    return false;
}


static inline game_console
GameConsole(arena* Arena, 
            usize InfoLines, 
            usize CharactersPerLine, 
            usize CommandsCapacity,
            c4f InfoBgColor,
            c4f InfoTextDefaultColor,
            c4f InputBgColor,
            c4f InputTextColor,
            v2f Dimensions,
            v3f Position)
{
    game_console Ret = {};

    Ret.InfoBuffers = Array<game_console_string>(Arena, InfoLines);
    for (usize I = 0; I < Ret.InfoBuffers.Count; ++I) {
        game_console_string* InfoBuffer = Ret.InfoBuffers + I;
        InfoBuffer->Buffer = StringBuffer(Arena, CharactersPerLine);
    }

    Ret.InputBuffer = StringBuffer(Arena, CharactersPerLine);
    Ret.CommandBuffer = StringBuffer(Arena, CharactersPerLine);
    Ret.InfoBgColor = InfoBgColor;
    Ret.InfoTextDefaultColor = InfoTextDefaultColor;
    Ret.InputBgColor = InputBgColor;
    Ret.InputTextColor = InputTextColor;
    Ret.Dimensions = Dimensions;
    Ret.Position = Position;

    Ret.Commands = List<game_console_command>(Arena, CommandsCapacity);
    return Ret;
}


static inline void
PushInfo(game_console* Console, string String, c4f Color) {
    for (usize I = 0; I < Console->InfoBuffers.Count - 1; ++I) {
        usize J = Console->InfoBuffers.Count - 1 - I;
        game_console_string* Dest = Console->InfoBuffers + J;
        game_console_string* Src = Console->InfoBuffers + J - 1;
        Copy(&Dest->Buffer, Src->Buffer.Array);
        Dest->Color = Console->InfoTextDefaultColor;

    }
    Console->InfoBuffers[0].Color = Color;
    Clear(&Console->InfoBuffers[0].Buffer);
    Copy(&Console->InfoBuffers[0].Buffer, String);
}


// Returns true if there is a new command
static inline void 
Update(game_console* Console, 
       input* Input) 
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
    
    // Remove character
    if (IsPoked(Input->ButtonBack)) {
        if (!IsEmpty(Console->InputBuffer.Array))
            Pop(&Console->InputBuffer);
    }

    if (IsPoked(Input->ButtonConfirm)) {
        PushInfo(Console, Console->InputBuffer.Array, Color_White);
        Copy(&Console->CommandBuffer, Console->InputBuffer.Array);
        Clear(&Console->InputBuffer);
        Execute(Console, Console->CommandBuffer.Array);
    }

}

static inline void
Render(game_console* Console, 
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
