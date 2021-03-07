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
    Mode->Entity.Position = v3f{ -800.f, 0.f, 0.f };
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
    SwitchToGameCoords(RenderCommands);
    game_mode_sandbox* Mode = PermState->SandboxMode;
    game_assets* Assets = TranState->Assets;
    
    UpdateInput(Mode, Input);
    
    // Entity
    game_mode_sandbox_entity * Entity = &Mode->Entity;
    m44f T = M44f_Translation(Mode->Entity.Position.X,
                              Mode->Entity.Position.Y,
                              Mode->Entity.Position.Z);
    m44f S = M44f_Scale(64.f, 64.f, 1.f);
    
    auto* AtlasAabb = Assets->AtlasAabbs + AtlasAabb_PlayerDot;
    f32 Speed = 300.f; 
    Entity->Position += Entity->Direction * Speed * DeltaTime;
    PushDrawTexturedQuad(RenderCommands, 
                         Color_White, 
                         T*S,
                         GetTexture(Assets, Texture_AtlasDefault)->Handle,
                         GetAtlasUV(Assets, AtlasAabb));
    
    // Line
    line2f Line = line2f {
        v2f { -200.f, -200.f },
        v2f { 200.f, 200.f }
    };
    
    PushDrawLine(RenderCommands,
                 Line,
                 16.f,
                 Color_White);
    
    // Line circle collision detection
    {
        
    }
    
}



#endif 
