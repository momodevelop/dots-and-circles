
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
    f32 RotationSpeed;
};

static inline void
UpdateAndRender(game_assets* Assets,
                entity* Entity, 
                render_commands * RenderCommands, 
                f32 DeltaTime) {
    // NOTE(Momo): Update
    Entity->Rotation += Entity->RotationSpeed * DeltaTime;
    Wrap(Entity->Rotation, -Pi32, Pi32);
    
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

#define MAX_ENTITIES  128
struct game_state {
    game_assets Assets;
    entity Entities[MAX_ENTITIES];
};

// TODO(Momo): Shift to ryoji_easing.h
static inline f32 
Lerp(f32 Start, f32 End, f32 Fraction) {
    return (1.f - Fraction) * Start + Fraction * End;
}


static inline void 
Init(game_state * GameState, platform_api* Platform) {
    auto Result = Platform->ReadFile("assets/ryoji.bmp");
    LoadTexture(&GameState->Assets, GameTextureType_ryoji, Result.Content);
    Platform->FreeFile(Result);
    
    Result = Platform->ReadFile("assets/yuu.bmp");
    LoadTexture(&GameState->Assets, GameTextureType_yuu, Result.Content);
    Platform->FreeFile(Result);
    
    Result = Platform->ReadFile("assets/yuu.bmp");
    LoadTexture(&GameState->Assets, GameTextureType_blank, Result.Content);
    Platform->FreeFile(Result);
    
    
    // NOTE(Momo): Initialize the entities
    f32 currentPositionX = -800.f;
    f32 currentPositionY = -400.f;
    
    for ( u32 i = 0; i < MAX_ENTITIES; ++i) {
        f32 currentColor = Lerp(0.f, 1.f, (f32)i/MAX_ENTITIES);
        GameState->Entities[i].Position = { currentPositionX, currentPositionY, 0 };
        GameState->Entities[i].Rotation = 0.f;
        GameState->Entities[i].Scale = { 1.f * i , 1.f * i , 1.f };
        GameState->Entities[i].Colors = { currentColor, currentColor, currentColor, currentColor };
        GameState->Entities[i].RotationSpeed = 0.01f * i;
        GameState->Entities[i].TextureHandle = i % GameTextureType_max;
        
        currentPositionX += 1.f * i * 2.f;
        
        if (currentPositionX > 800.f) {
            currentPositionX = -800.f;
            currentPositionY += 1.f * i * 2.f;
        }
    }
}

// NOTE(Momo):  Exported Functions
extern "C" 
GAME_UPDATE(GameUpdate) {
    game_state* GameState = (game_state*)GameMemory->PermanentStore;
    if(!GameMemory->IsInitialized) {
        Init(GameState, &GameMemory->PlatformApi);
        GameMemory->IsInitialized = true;
    }
    
    PushCommandClear(RenderCommands, { 0.0f, 0.3f, 0.3f, 0.f });
    for (u32 i = 0; i < MAX_ENTITIES; ++i) {
        UpdateAndRender(&GameState->Assets, &GameState->Entities[i], RenderCommands, DeltaTime);
    }
}