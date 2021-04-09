#ifndef GAME_MODE_SANDBOX
#define GAME_MODE_SANDBOX

#include "game.h"

// NOTE(Momo): Mode /////////////////////////////////////////////
struct game_mode_sandbox_entity {
    v3f Position;
    v3f Direction; 
};

struct game_mode_sandbox {
    game_mode_sandbox_entity Entity;
};

static inline void 
InitSandboxMode(permanent_state* PermState) {
    game_mode_sandbox* Mode = PermState->SandboxMode;     
    Mode->Entity.Position = V3f_Create(-800.f, 0.f, 0.f);
}

static inline void
UpdateInput(game_mode_sandbox* Mode,
            game_input* Input)
{
    v3f Direction = {};
    game_mode_sandbox_entity* Player = &Mode->Entity; 
    b8 IsMovementButtonDown = false;
    if(IsDown(Input->ButtonLeft)) {
        Direction.X = -1.f;
        IsMovementButtonDown = true;
    };
    
    if(IsDown(Input->ButtonRight)) {
        Direction.X = 1.f;
        IsMovementButtonDown = true;
    }
    
    if(IsDown(Input->ButtonUp)) {
        Direction.Y = 1.f;
        IsMovementButtonDown = true;
    }
    if(IsDown(Input->ButtonDown)) {
        Direction.Y = -1.f;
        IsMovementButtonDown = true;
    }
    
    if (IsMovementButtonDown) 
        Player->Direction = V3f_Normalize(Direction);
    else {
        Player->Direction = {};
    }
}

static inline void
UpdateSandboxMode(permanent_state* PermState, 
                  transient_state* TranState,
                  mailbox* RenderCommands, 
                  game_input* Input,
                  f32 DeltaTime) 
{
#if 0
    static inline void
        Win32PlatformStartRecord(win32_state* State) {
        const char* RecordPath = "record_inputs";
        
        
        // NOTE(Momo): We save the state 
        DWORD BytesWritten;
        if(!WriteFile(Win32Handle, 
                      G_GameMemory->Data,
                      (DWORD)G_GameMemory->DataSize,
                      &BytesWritten,
                      0)) 
        {
            Win32Log("[Win32::StartRecord] Cannot write file: %s\n", Path);
            CloseHandle(RecordFileHandle);
            return;
        }
        
        if (BytesWritten != G_GameMemory->DataSize) {
            Win32Log("[Win32::StartRecord] Did not complete writing: %s\n", Path);
            CloseHandle(RecordFileHandle);
            return;
        }
        Win32Log("[Win32::StartRecord] State saved: %s\n", Path);
        
        // NOTE(Momo): Initialize recording state
        G_State.IsRecording = True;
        G_State.RecordDuration = 0.f;
        G_State.RecordFileHandle = RecordFileHandle;
        
        
    }
    
    static inline 
        PlatformStopRecordFunc(Win32PlatformStopRecord) {
        if (!G_State.IsRecording) {
            return;
        }
        G_State.IsRecording = False;
        CloseHandle(G_State.RecordFileHandle);
    }
    
#endif
    
    // Animation?
}



#endif 
