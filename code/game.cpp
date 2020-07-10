#include "game_platform.h"
#include "game_renderer.h"

struct entity {
    f32 Rotation;
    v3f Scale;
    v3f Position;
    c4f Colors;
    
    f32 RotationSpeed;
    
};



static inline void
UpdateAndRender(entity* Entity, render_commands * RenderCommands, f32 DeltaTime) {
    // NOTE(Momo): Update
    Entity->Rotation += Entity->RotationSpeed * DeltaTime;
    Wrap(Entity->Rotation, -Pi32, Pi32);
    
    // NOTE(Momo): Render
    PushCommandClear(RenderCommands, { 0.0f, 0.3f, 0.3f, 0.f });
    m44f T = MakeTranslationMatrix(Entity->Position);
    m44f R = MakeRotationZMatrix(Entity->Rotation);
    m44f S = MakeScaleMatrix(Entity->Scale);
    
    // TODO(Momo): This part should be done by renderer?
    PushCommandColoredQuad(RenderCommands, 
                           Entity->Colors, 
                           T*R*S);
}

struct game_state {
    entity Entities[3];
};


static inline void 
Init(game_state * GameState) {
    GameState->Entities[0].Position = {0, 0, 0};
    GameState->Entities[0].Rotation = 0.f;
    GameState->Entities[0].Scale = {100, 100 , 100};
    GameState->Entities[0].Colors = {1, 0, 0, 0};
    GameState->Entities[0].RotationSpeed = 1.0f;
    
    GameState->Entities[1].Position = {100, 100, 1};
    GameState->Entities[1].Rotation = 0.f;
    GameState->Entities[1].Scale = {100, 100 , 100};
    GameState->Entities[1].Colors  = { 0, 1, 0, 0 };
    GameState->Entities[1].RotationSpeed = -1.0f;
    
    
    GameState->Entities[2].Position = {-100, -100, 2};
    GameState->Entities[2].Rotation = 0.f;
    GameState->Entities[2].Scale = {100, 100 , 100 };
    GameState->Entities[2].Colors  = { 0, 0, 1, 0 };
    GameState->Entities[2].RotationSpeed = 10.0f;
    
}

// NOTE(Momo):  Exported Functions
extern "C" 
GAME_UPDATE(GameUpdate) {
    
    game_state* GameState = (game_state*)GameMemory->PermanentStore;
    if(!GameMemory->IsInitialized) {
        Init(GameState);
        GameMemory->IsInitialized = true;
    }
    
    for (u32 i = 0; i < 3; ++i) {
        UpdateAndRender(&GameState->Entities[i], RenderCommands, DeltaTime );
    }
}