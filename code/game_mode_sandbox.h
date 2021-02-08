#ifndef GAME_MODE_SANDBOX
#define GAME_MODE_SANDBOX

#include "game.h"

// NOTE(Momo): Mode /////////////////////////////////////////////
struct game_mode_sandbox_entity {
    v2f Position;
    v2f Direction; 
};

struct game_mode_sandbox {
    game_mode_sandbox_entity Entity;
};

static inline void 
InitSandboxMode(permanent_state* PermState) {
    game_mode_sandbox* Mode = PermState->SandboxMode;     
    Mode->Entity.Position = v2f{ -800.f, 0.f };
}

static inline void
UpdateInput(game_mode_sandbox* Mode,
            game_input* Input)
{
    v2f Direction = {};
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
        Player->Direction = Normalize(Direction);
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
    game_mode_sandbox* Mode = PermState->SandboxMode;
    game_assets* Assets = TranState->Assets;
    PushClearColor(RenderCommands, { 0.0f, 0.3f, 0.3f, 0.f });
    PushOrthoCamera(RenderCommands, 
            v3f{}, 
            CenteredAabb( 
                v3f{ Global_DesignWidth, Global_DesignHeight, Global_DesignDepth }, 
                v3f{ 0.5f, 0.5f, 0.5f }
            )
    );
    
    UpdateInput(Mode, Input);

    // Entity
    game_mode_sandbox_entity * Entity = &Mode->Entity;
    m44f Transform = M44fTranslation(Mode->Entity.Position) * 
                     M44fScale(64.f, 64.f, 1.f);
    auto* AtlasAabb = Assets->AtlasAabbs + AtlasAabb_PlayerDot;
    f32 Speed = 300.f; 
    Entity->Position += Entity->Direction * Speed * DeltaTime;
    PushDrawTexturedQuad(RenderCommands, 
                         Color_White, 
                         Transform,
                         GetTexture(Assets, Texture_AtlasDefault).Handle,
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
