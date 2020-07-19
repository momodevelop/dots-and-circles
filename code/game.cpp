#include "game_platform.h"
#include "game_renderer.h"
#include "game_assets.h"
#include "ryoji_easing.h"


struct entity {
    f32 Rotation;
    v3f Scale;
    v3f Position;
    c4f Colors;
    u32 TextureHandle;
    
    f32 Timer;
    f32 Duration;
    f32 StartX;
    f32 EndX;
};

static inline void
UpdateAndRender(platform_api* Platform,
                game_assets* Assets,
                entity* Entity, 
                render_commands * RenderCommands, 
                f32 DeltaTime) {
    Assert(Entity->Duration != 0.f);
    
    
    // NOTE(Momo): Update
    
    f32 ease = EaseOutBounce(Clamp(Entity->Timer/Entity->Duration, 0.f, 1.f));
    
    Entity->Position.X = Entity->StartX + (Entity->EndX - Entity->StartX) * ease; 
    Entity->Timer += DeltaTime;
    //Platform->Log("%f\n", Clamp(Entity->Timer/Entity->Duration, 0.f, 1.f));
    
    //Entity->Rotation += Entity->RotationSpeed * DeltaTime;
    //Wrap(Entity->Rotation, -Pi32, Pi32);
    
    // NOTE(Momo): Render
    m44f T = MakeTranslationMatrix(Entity->Position);
    m44f R = MakeRotationZMatrix(Entity->Rotation);
    m44f S = MakeScaleMatrix(Entity->Scale);
    
    // TODO(Momo): This part should be done by renderer?
    PushCommandTexturedQuad(RenderCommands, 
                            Entity->Colors, 
                            T*R*S,
                            Assets->Textures[Entity->TextureHandle]);
}

#define MAX_ENTITIES 128
struct game_state {
    game_assets Assets;
    entity Entities[MAX_ENTITIES];
};

static inline void 
Init(game_state * GameState, platform_api* Platform) {
    auto Result = Platform->ReadFile("assets/ryoji.bmp");
    LoadTexture(&GameState->Assets, GameTextureType_ryoji, Result.Content);
    Platform->FreeFile(Result);
    
    Result = Platform->ReadFile("assets/yuu.bmp");
    LoadTexture(&GameState->Assets, GameTextureType_yuu, Result.Content);
    Platform->FreeFile(Result);
    
    Result = Platform->ReadFile("assets/blank.bmp");
    LoadTexture(&GameState->Assets, GameTextureType_blank, Result.Content);
    Platform->FreeFile(Result);
    
    GameState->Entities[0].Position = { 0.f, 0.f, 0.f };
    GameState->Entities[0].Rotation = 0.f;
    GameState->Entities[0].Scale = { 100.f, 100.f };
    GameState->Entities[0].Colors = { 1.f, 1.f, 1.f, 1.f };
    GameState->Entities[0].TextureHandle = GameTextureType_ryoji;
    GameState->Entities[0].Timer = 0.f;
    GameState->Entities[0].Duration = 1.f;
    GameState->Entities[0].StartX = -850.f;
    GameState->Entities[0].EndX = -100.f;
    
    GameState->Entities[1].Position = { 0.f, 0.f, 0.f };
    GameState->Entities[1].Rotation = 0.f;
    GameState->Entities[1].Scale = { 100.f, 100.f };
    GameState->Entities[1].Colors = { 1.f, 1.f, 1.f, 1.f };
    GameState->Entities[1].TextureHandle = GameTextureType_yuu;
    GameState->Entities[1].Timer = 0.f;
    GameState->Entities[1].Duration = 1.f;
    GameState->Entities[1].StartX = 850.f;
    GameState->Entities[1].EndX = 100.f;
    
}

// NOTE(Momo):  Exported Functions
extern "C" 
GAME_UPDATE(GameUpdate) {
    game_state* GameState = (game_state*)GameMemory->PermanentStore;
    if(!GameMemory->IsInitialized) {
        Init(GameState, Platform);
        GameMemory->IsInitialized = true;
    }
    
    // NOTE(Momo): Test Input
    
    if (IsPoked(Input->ButtonUp)) {
        Platform->Log("Button Up is Poked");
    }
    if (IsDown(Input->ButtonUp)) {
        Platform->Log("Button Up is Down");
    }
    if (IsReleased(Input->ButtonUp)) {
        Platform->Log("Button Up is Released");
    }
    if (IsHeld(Input->ButtonUp)) {
        Platform->Log("Button Up is Held");
    }
    
    PushCommandClear(RenderCommands, { 0.0f, 0.3f, 0.3f, 0.f });
    for (u32 i = 0; i < 2; ++i) {
        UpdateAndRender(Platform,
                        &GameState->Assets, 
                        &GameState->Entities[i], 
                        RenderCommands, 
                        DeltaTime);
    }
    
    
    
    
    
    
}