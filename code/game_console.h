#ifndef GAME_CONSOLE
#define GAME_CONSOLE

#include "mm_list.h"
#include "mm_colors.h"
#include "platform.h"
#include "renderer.h"
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
    v4f Color;
};


struct game_console {
    v4f InfoBgColor;
    v4f InputBgColor;
    v4f InputColor;
    v4f InfoDefaultColor;
    v3f Position;
    v2f Dimensions;

    // Buffers
    array<game_console_string> InfoBuffers;
    string_buffer InputBuffer;
    string_buffer CommandBuffer;
    b32 IsCommandRead;
};



static inline void 
Register(list<game_console_command>* Commands, 
         string Key, 
         game_console_callback Callback, 
         void* Context)
{
    Push(Commands, { Key, Callback, Context } );
}

static inline void 
Unregister(list<game_console_command>* Commands, string Key) {
    RemoveIf(Commands, [Key](const game_console_command* Cmd) { 
        return Cmd->Key == Key; 
    });
}

static inline b32 
Execute(list<game_console_command>* Commands, string Arguments) {
    // Assume that the first token is the command     
    range<usize> Range = { 0, Find(Arguments, ' ') };
    string CommandStr = SubString(Arguments, Range); 

    // Send a command to a callback
    for (usize I = 0; I < Commands->Count; ++I) {
        game_console_command* Command = (*Commands) + I;
        if (Command->Key == CommandStr) {
             Command->Callback(Command->Context, Arguments);
             return true;
        }
    }

    return false;
}


static inline game_console
GameConsole(arena* Arena, 
            usize InfoBufferLines, 
            usize BufferCapacity, 
            usize CommandsCapacity)
{
    game_console Ret = {};

    Ret.InfoBuffers = Array<game_console_string>(Arena, InfoBufferLines);
    for (usize I = 0; I < Ret.InfoBuffers.Count; ++I) {
        game_console_string* InfoBuffer = Ret.InfoBuffers + I;
        InfoBuffer->Buffer = StringBuffer(Arena, BufferCapacity);
    }

    Ret.InputBuffer = StringBuffer(Arena, BufferCapacity);
    Ret.CommandBuffer = StringBuffer(Arena, BufferCapacity);

    return Ret;
}


static inline void
PushDebugInfo(game_console* Console, string String, v4f Color) {
    for (usize I = 0; I < Console->InfoBuffers.Count - 1; ++I) {
        usize J = Console->InfoBuffers.Count - 1 - I;
        game_console_string* Dest = Console->InfoBuffers + J;
        game_console_string* Src = Console->InfoBuffers + J - 1;
        Copy(&Dest->Buffer, Src->Buffer.Array);
        Dest->Color = Console->InfoDefaultColor;

    }
    Console->InfoBuffers[0].Color = Color;
    Clear(&Console->InfoBuffers[0].Buffer);
    Copy(&Console->InfoBuffers[0].Buffer, String);
}

static inline string
GetCommandString(game_console* Console) {
    return Console->CommandBuffer.Array;
}

// Returns true if there is a new command
static inline b32 
Update(game_console* Console, 
       input* Input) 
{

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
        PushDebugInfo(Console, Console->InputBuffer.Array, ColorWhite);
        Copy(&Console->CommandBuffer, Console->InputBuffer.Array);
        Clear(&Console->InputBuffer);
        return true;
    }

    return false;

}

static inline void
Render(game_console* Console, 
       mailbox* RenderCommands,
       game_assets* Assets) 
{
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
        PushCommandDrawQuad(RenderCommands, Console->InfoBgColor, InfoBgTransform);
    }

    {
        m44f ScaleMatrix = M44fScale(v2f{ Console->Dimensions.W, LineHeight });
        m44f PositionMatrix = M44fTranslation(
                Console->Position.X, 
                Bottom + LineHeight * 0.5f,
                Console->Position.Z + 0.01f
        );

        m44f InputBgTransform = PositionMatrix * ScaleMatrix;
        PushCommandDrawQuad(RenderCommands, Console->InputBgColor, InputBgTransform);
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
                Console->InputColor,
                Font_Default, FontSize, 
                Console->InputBuffer.Array
            );
        
        }

    }
}





#endif
