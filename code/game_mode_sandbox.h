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
    // Animation?
}



#endif 
