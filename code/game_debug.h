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
    m44f Basis;
    m44f InfoBgTransform;
    m44f InputBgTransform;
    v4f InfoBgColor;
    v4f InputBgColor;

    // Buffers
    array<debug_string> InfoBuffers;
    string_buffer InputBuffer;

    list<debug_command> Commands;
};




static inline debug_console
CreateDebugConsole(arena* Arena, 
        usize InfoBufferLines, 
        usize BufferCapacity, 
        usize CommandsCapacity,
        m44f Basis,
        m44f InfoBgTransform,
        m44f InputBgTransform,
        v4f InfoBgColor,
        v4f InputBgColor) 
{
    debug_console Ret = {};

    Ret.InputBuffer = StringBuffer(Arena, BufferCapacity);
    Ret.Basis = Basis;
    Ret.InfoBgTransform = InfoBgTransform;
    Ret.InputBgTransform = InputBgTransform;
    Ret.InfoBgColor = InfoBgColor;
    Ret.InputBgColor = InputBgColor;
    

    
    Ret.InfoBuffers = Array<debug_string>(Arena, InfoBufferLines);
    for (usize I = 0; I < Ret.InfoBuffers.Length; ++I) {
        debug_string* InfoBuffer = Ret.InfoBuffers + I;
        InfoBuffer->Buffer = StringBuffer(Arena, BufferCapacity);
        InfoBuffer->Color = { 1.f, 1.f, 1.f, 1.f };
    }

    Ret.Commands = List<debug_command>(Arena, CommandsCapacity);
    return Ret;
}


static inline void
PushDebugInfo(debug_console* DebugConsole, string String, v4f Color) {
    for(usize i = DebugConsole->InfoBuffers.Length - 2; i >= 0 ; --i) {
        debug_string* Dest = DebugConsole->InfoBuffers + i + 1;
        debug_string* Src = DebugConsole->InfoBuffers + i;
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
    PushCommandSetBasis(RenderCommands, Console->Basis);
    
    // Background
    PushCommandDrawQuad(RenderCommands, Console->InfoBgColor, Console->InfoBgTransform);
    PushCommandDrawQuad(RenderCommands, Console->InputBgColor, Console->InputBgTransform);
    

    // Draw text
    {
        for (u32 I = 0; I < Console->InfoBuffers.Length ; ++I) {
            DrawText(RenderCommands,
                    Assets,
                    { 10.f, 50.f + I * 40.f, 502.f },
                    Console->InfoBuffers[I].Color,
                    Font_Default, 32.f,
                    Console->InfoBuffers[I].Buffer.Array);
        }

        DrawText(RenderCommands, 
            Assets, 
            { 10.f, 10.f, 600.f }, 
            ColorWhite,
            Font_Default, 32.f, 
            Console->InputBuffer.Array);
        


    }
}





#endif
