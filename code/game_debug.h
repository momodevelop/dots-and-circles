#ifndef GAME_DEBUG
#define GAME_DEBUG

#include "mm_string.h"
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
    debug_string InfoBuffers[5];
    string_buffer InputBuffer;

    list<debug_command> Commands;
};

static inline debug_console
CreateDebugConsole(arena* Arena) {
    debug_console Ret = {};

    Ret.InputBuffer = StringBuffer(Arena, 110);

    for (auto&& DebugInfoBuffer : Ret.InfoBuffers) {
        DebugInfoBuffer.Buffer = StringBuffer(Arena, 110);
        DebugInfoBuffer.Color = { 1.f, 1.f, 1.f, 1.f };
    }

    Ret.Commands = List<debug_command>(Arena, 32);
    return Ret;
}


static inline void
PushDebugInfo(debug_console* DebugConsole, string String, v4f Color) {
    for(i32 i = ArrayCount(DebugConsole->InfoBuffers) - 2; i >= 0 ; --i) {
        debug_string* Dest = DebugConsole->InfoBuffers + i + 1;
        debug_string* Src = DebugConsole->InfoBuffers + i;
        Copy(&Dest->Buffer, Src->Buffer.String);
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
        return Compare(Cmd->Key, Key); 
    });
}



static inline void 
Update(debug_console* DebugConsole, game_input* Input) {

    if (Input->DebugTextInputBuffer.Length > 0 && 
        Input->DebugTextInputBuffer.Length <= Remaining(DebugConsole->InputBuffer)) 
    {  
        Concat(&DebugConsole->InputBuffer, Input->DebugTextInputBuffer.String);
    }
    
    // Remove character
    if (IsPoked(Input->DebugKeys[GameDebugKey_Backspace])) {
        if (!IsEmpty(DebugConsole->InputBuffer.String))
            Pop(&DebugConsole->InputBuffer);
    }

    if (IsPoked(Input->DebugKeys[GameDebugKey_Return])) {
        PushDebugInfo(DebugConsole, DebugConsole->InputBuffer.String, ColorWhite);
        
        // Assume that the first token is the command 
        string StrToParse = DebugConsole->InputBuffer.String;
        
        range<usize> Slice = { 0, Find(StrToParse, ' ') };
        string CommandStr = String(StrToParse, Slice); 
            

        // Send a command to a callback
        for (auto&& Command : DebugConsole->Commands) {
            if (Compare(Command.Key, CommandStr)) {
                 Command.Callback(Command.Context, DebugConsole->InputBuffer.String);
                 break;
            }
        }

        Clear(&DebugConsole->InputBuffer);
    }

}


static inline void
Render(debug_console* DebugConsole, mmcmd_commands* RenderCommands, game_assets* Assets) {
    PushCommandSetBasis(RenderCommands, 
            Orthographic(-1.f, 1.f, -1.f, 1.f, -1.f, 1.f, 
                        0.f, 1600.f, 0.f, 900.f, -1000.f, 1000.f, true));

    // Background
    {
        m44f A = Translation(0.5f, 0.5f, 0.f);
        m44f S = Scale(1600.f, 240.f, 1.f);
        m44f T = Translation(0.f, 0.f, 500.f);
        PushCommandDrawQuad(RenderCommands, { 0.3f, 0.3f, 0.3f, 1.f }, T*S*A);
        
        S = Scale(1600.f, 40.f, 1.f);
        T = Translation(0.f, 0.f, 501.f);
        PushCommandDrawQuad(RenderCommands, { 0.2f, 0.2f, 0.2f, 1.f }, T*S*A);
    }

    // Draw text
    {
        for (u32 i = 0; i < ArrayCount(DebugConsole->InfoBuffers); ++i) {
            DrawText(RenderCommands,
                    Assets,
                    { 10.f, 50.f + i * 40.f, 502.f },
                    DebugConsole->InfoBuffers[i].Color,
                    Font_Default, 32.f,
                    DebugConsole->InfoBuffers[i].Buffer.String);
        }

        DrawText(RenderCommands, 
            Assets, 
            { 10.f, 10.f, 600.f }, 
            ColorWhite,
            Font_Default, 32.f, 
            DebugConsole->InputBuffer.String);
        


    }
}





#endif
