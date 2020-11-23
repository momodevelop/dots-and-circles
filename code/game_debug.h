#ifndef GAME_DEBUG
#define GAME_DEBUG

#include "mm_list.h"
#include "mm_colors.h"

#include "game_renderer.h"
#include "game_assets.h"
#include "game_text.h"
#include "game_input.h"

using debug_callback = void (*)(void* Context, string Arguments);
struct debug_command {
    string Key;
    debug_callback Callback;
    void* Context;
};

struct debug_string {
    string_buffer Buffer;
    v4f Color;
};


struct debug_console {
    v4f InfoBgColor;
    v4f InputBgColor;
    v4f InputColor;
    v4f InfoDefaultColor;
    v3f Position;
    v2f Dimensions;

    // Buffers
    array<debug_string> InfoBuffers;
    string_buffer InputBuffer;

    list<debug_command> Commands;
};




static inline debug_console
CreateDebugConsole(
        arena* Arena, 
        usize InfoBufferLines, 
        usize BufferCapacity, 
        usize CommandsCapacity,
        v4f InfoBgColor,
        v4f InputBgColor,
        v4f InputColor,
        v4f InfoDefaultColor,
        v2f Dimensions,
        v3f Position) 
{
    debug_console Ret = {};


    Ret.InputBuffer = StringBuffer(Arena, BufferCapacity);
    Ret.InfoBgColor = InfoBgColor;
    Ret.InputBgColor = InputBgColor;
    Ret.InfoDefaultColor = InfoDefaultColor;
    Ret.InputColor = InputColor;
    Ret.Position = Position;
    Ret.Dimensions = Dimensions;

   
    
    Ret.InfoBuffers = Array<debug_string>(Arena, InfoBufferLines);
    for (usize I = 0; I < Ret.InfoBuffers.Length; ++I) {
        debug_string* InfoBuffer = Ret.InfoBuffers + I;
        InfoBuffer->Buffer = StringBuffer(Arena, BufferCapacity);
        InfoBuffer->Color = InfoDefaultColor;
    }

    Ret.Commands = List<debug_command>(Arena, CommandsCapacity);
    return Ret;
}


static inline void
PushDebugInfo(debug_console* DebugConsole, string String, v4f Color) {
    for (usize I = 0; I < DebugConsole->InfoBuffers.Length - 1; ++I) {
        usize J = DebugConsole->InfoBuffers.Length - 1 - I;
        debug_string* Dest = DebugConsole->InfoBuffers + J;
        debug_string* Src = DebugConsole->InfoBuffers + J - 1;
        Copy(&Dest->Buffer, Src->Buffer.Array);
        Dest->Color = Src->Color;

    }
    DebugConsole->InfoBuffers[0].Color = Color;
    Clear(&DebugConsole->InfoBuffers[0].Buffer);
    Copy(&DebugConsole->InfoBuffers[0].Buffer, String);
}


static inline void 
Register(debug_console* DebugConsole, string Key, debug_callback Callback, void* Context)
{
    Push(&DebugConsole->Commands, { Key, Callback, Context } );
}

static inline void 
Unregister(debug_console* DebugConsole, string Key) {
    RemoveIf(&DebugConsole->Commands, [Key](const debug_command* Cmd) { 
        return Cmd->Key == Key; 
    });
}

static inline void 
Update(debug_console* DebugConsole, game_input* Input) {

    if (Input->DebugTextInputBuffer.Length > 0 && 
        Input->DebugTextInputBuffer.Length <= Remaining(DebugConsole->InputBuffer)) 
    {  
        Push(&DebugConsole->InputBuffer, Input->DebugTextInputBuffer.Array);
    }
    
    // Remove character
    if (IsPoked(Input->DebugKeys[GameDebugKey_Backspace])) {
        if (!IsEmpty(DebugConsole->InputBuffer.Array))
            Pop(&DebugConsole->InputBuffer);
    }

    if (IsPoked(Input->DebugKeys[GameDebugKey_Return])) {
        PushDebugInfo(DebugConsole, DebugConsole->InputBuffer.Array, ColorWhite);
        
        // Assume that the first token is the command 
        string StrToParse = DebugConsole->InputBuffer.Array;
        
        range<usize> Range = { 0, Find(StrToParse, ' ') };

        string CommandStr = SubString(StrToParse, Range); 
            
        // Send a command to a callback
        for (usize i = 0; i < DebugConsole->Commands.Length; ++i) {
            auto* Command = DebugConsole->Commands + i;
            if (Command->Key == CommandStr) {
                 Command->Callback(Command->Context, DebugConsole->InputBuffer.Array);
                 break;
            }
        }

        Clear(&DebugConsole->InputBuffer);
    }

}


static inline void
Render(debug_console* Console, mmcmd_commands* RenderCommands, game_assets* Assets) {
    font* Font = Assets->Fonts + Font_Default;
    
    f32 Bottom = Console->Position.Y - Console->Dimensions.H * 0.5f;
    f32 Left = Console->Position.X - Console->Dimensions.W * 0.5f;
    f32 LineHeight = Console->Dimensions.H / (Console->InfoBuffers.Length + 1);
    f32 FontSize = LineHeight * 0.9f;
    f32 FontHeight = Height(Font) * FontSize;
    f32 PaddingHeight = (LineHeight - FontHeight) * 0.5f  + Abs(Font->Descent) * FontSize; 
    f32 PaddingWidth = Console->Dimensions.W * 0.005f;
    {
        m44f ScaleMatrix = M44F_Scale(Console->Dimensions);
        m44f PositionMatrix = M44F_Translation(Console->Position);
        m44f InfoBgTransform = PositionMatrix * ScaleMatrix;
        PushCommandDrawQuad(RenderCommands, Console->InfoBgColor, InfoBgTransform);
    }

    {
        m44f ScaleMatrix = M44F_Scale(V2F(Console->Dimensions.W, LineHeight));
        m44f PositionMatrix = M44F_Translation(
                Console->Position.X, 
                Bottom + LineHeight * 0.5f,
                Console->Position.Z + 0.01f
        );

        m44f InputBgTransform = PositionMatrix * ScaleMatrix;
        PushCommandDrawQuad(RenderCommands, Console->InputBgColor, InputBgTransform);
    }

    // Draw text
    {


        for (u32 I = 0; I < Console->InfoBuffers.Length ; ++I) {
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
