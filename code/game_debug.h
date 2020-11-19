#ifndef GAME_DEBUG
#define GAME_DEBUG

#include "mm_string.h"
#include "mm_list.h"
#include "mm_colors.h"

#include "game_renderer.h"
#include "game_assets.h"
#include "game_text.h"
#include "game_input.h"

typedef void (*debug_callback)(void* Context);
struct debug_command {
    mms_string Key;
    debug_callback Callback;
    void* Context;
};

struct debug_string {
    mms_string_buffer Buffer;
    mmm_v4f Color;
};

struct debug_console {
    debug_string InfoBuffers[5];
    mms_string_buffer InputBuffer;

    mml_list<debug_command> Commands;
};

static inline debug_console
CreateDebugConsole(mmarn_arena* Arena) {
    debug_console Ret = {};

    Ret.InputBuffer = mms_StringBuffer(Arena, 110);

    for (auto&& DebugInfoBuffer : Ret.InfoBuffers) {
        char* Memory = mmarn_PushArray<char>(Arena, 110);
        DebugInfoBuffer.Buffer = mms_StringBuffer(Memory, 110);
        DebugInfoBuffer.Color = { 1.f, 1.f, 1.f, 1.f };
    }

    Ret.Commands = mml_List<debug_command>(Arena, 32);
    return Ret;
}


static inline void
PushDebugInfo(debug_console* DebugConsole, mms_string String, mmm_v4f Color) {
    for(i32 i = ArrayCount(DebugConsole->InfoBuffers) - 2; i >= 0 ; --i) {
        debug_string* Dest = DebugConsole->InfoBuffers + i + 1;
        debug_string* Src = DebugConsole->InfoBuffers + i;
        mms_Copy(&Dest->Buffer, Src->Buffer.String);
        Dest->Color = Src->Color;

    }
    DebugConsole->InfoBuffers[0].Color = Color;
    mms_Clear(&DebugConsole->InfoBuffers[0].Buffer);
    mms_Copy(&DebugConsole->InfoBuffers[0].Buffer, String);
}


static inline void 
Register(debug_console* DebugConsole, mms_string Key, debug_callback Callback, void* Context)
{
    mml_Push(&DebugConsole->Commands, { Key, Callback, Context } );
}

static inline void 
Unregister(debug_console* DebugConsole, mms_string Key) {
    mml_RemoveIf(&DebugConsole->Commands, [Key](const debug_command* Cmd) { 
        return mms_Compare(Cmd->Key, Key); 
    });
}



static inline void 
Update(debug_console* DebugConsole, game_input* Input) {

    if (Input->DebugTextInputBuffer.Length > 0) { 
         mms_ConcatSafely(&DebugConsole->InputBuffer, Input->DebugTextInputBuffer.String);
    }
    
    // Remove character
    if (IsPoked(Input->DebugKeys[GameDebugKey_Backspace])) {
        // TODO: Create an pop safely function? 
        // Returns an iterator? Returns a tuple? Returns an Option?
        mms_PopSafely(&DebugConsole->InputBuffer);
    }

    if (IsPoked(Input->DebugKeys[GameDebugKey_Return])) {
        PushDebugInfo(DebugConsole, DebugConsole->InputBuffer.String, mmc_ColorWhite);
        
        mms_string StringToParse = DebugConsole->InputBuffer.String;
        {
            // strtok and strsep is nice, but it modifies the existing string. 
            range Slice = {};
            while(Slice.OnePastEnd < StringToParse.Length) {
                Slice = mms_FindNextToken(StringToParse, ' ', Slice); 
            }

        }

        // Send a command to a callback
        for (auto&& Command : DebugConsole->Commands) {
            if (mms_Compare(Command.Key, DebugConsole->InputBuffer.String)) {
                 Command.Callback(Command.Context);
                 break;
            }
        }

        mms_Clear(&DebugConsole->InputBuffer);
    }

}


static inline void
Render(debug_console* DebugConsole, mmcmd_commands* RenderCommands, game_assets* Assets) {
    PushCommandSetBasis(RenderCommands, 
            mmm_Orthographic(-1.f, 1.f, -1.f, 1.f, -1.f, 1.f, 
                        0.f, 1600.f, 0.f, 900.f, -1000.f, 1000.f, true));

    // Background
    {
        mmm_m44f A = mmm_Translation(0.5f, 0.5f, 0.f);
        mmm_m44f S = mmm_Scale(1600.f, 240.f, 1.f);
        mmm_m44f T = mmm_Translation(0.f, 0.f, 500.f);
        PushCommandDrawQuad(RenderCommands, { 0.3f, 0.3f, 0.3f, 1.f }, T*S*A);
        
        S = mmm_Scale(1600.f, 40.f, 1.f);
        T = mmm_Translation(0.f, 0.f, 501.f);
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
            mmc_ColorWhite,
            Font_Default, 32.f, 
            DebugConsole->InputBuffer.String);
        


    }
}





#endif
