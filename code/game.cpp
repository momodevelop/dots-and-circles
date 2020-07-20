#include "interface.h"
#include "interface_input.h"

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


enum struct game_state_type {
    Splash,
    Main
};

#define MAX_ENTITIES 128
struct game {
    game_state_type CurrentState;
    bool IsStateInitialized;
    
    game_assets Assets;
    entity Entities[MAX_ENTITIES];
    memory_arena MainArena;
};

struct game_state_splash {
    entity Entities[MAX_ENTITIES];
    
};

static inline void
UpdateAndRenderSplashState(game* Game, platform_api* Platform, render_commands* RenderCommands, f32 DeltaTime) {
    if(!Game->IsStateInitialized) {
        // NOTE(Momo): Create entities
        {
            Game->Entities[0].Position = { 0.f, 0.f, 0.f };
            Game->Entities[0].Rotation = 0.f;
            Game->Entities[0].Scale = { 100.f, 100.f };
            Game->Entities[0].Colors = { 1.f, 1.f, 1.f, 1.f };
            Game->Entities[0].TextureHandle = GameTextureType_ryoji;
            Game->Entities[0].Timer = 0.f;
            Game->Entities[0].Duration = 1.f;
            Game->Entities[0].StartX = -850.f;
            Game->Entities[0].EndX = -100.f;
            
            Game->Entities[1].Position = { 0.f, 0.f, 0.f };
            Game->Entities[1].Rotation = 0.f;
            Game->Entities[1].Scale = { 100.f, 100.f };
            Game->Entities[1].Colors = { 1.f, 1.f, 1.f, 1.f };
            Game->Entities[1].TextureHandle = GameTextureType_yuu;
            Game->Entities[1].Timer = 0.f;
            Game->Entities[1].Duration = 1.f;
            Game->Entities[1].StartX = 850.f;
            Game->Entities[1].EndX = 100.f;
        }
        Game->IsStateInitialized = true;
        Platform->Log("Splash state initialized!");
    }
    
    
    PushCommandClear(RenderCommands, { 0.0f, 0.3f, 0.3f, 0.f });
    for (u32 i = 0; i < 2; ++i) {
        UpdateAndRender(Platform,
                        &Game->Assets, 
                        &Game->Entities[i], 
                        RenderCommands, 
                        DeltaTime);
    }
    
}




// NOTE(Momo):  Exported Functions
extern "C" 
GAME_UPDATE(GameUpdate) {
    game* Game = (game*)GameMemory->Memory;
    
    // NOTE(Momo): Initialization of the game
    if(!GameMemory->IsInitialized) {
        Game->CurrentState = game_state_type::Splash;
        Game->IsStateInitialized = false;
        
        Game->MainArena = MakeMemoryArena((u8*)GameMemory->Memory + sizeof(game), GameMemory->MemorySize - sizeof(game));
        
        // NOTE(Momo): Init Assets
        Init(&Game->Assets, &Game->MainArena);
        {
            auto Result = Platform->ReadFile("assets/ryoji.bmp");
            Assert(Result.Content);
            LoadTexture(&Game->Assets, GameTextureType_ryoji, Result.Content);
            Platform->FreeFile(Result);
        }
        {
            auto Result = Platform->ReadFile("assets/yuu.bmp");
            Assert(Result.Content);
            LoadTexture(&Game->Assets, GameTextureType_yuu, Result.Content);
            Platform->FreeFile(Result);
        }
        {
            auto Result = Platform->ReadFile("assets/blank.bmp");
            Assert(Result.Content);
            LoadTexture(&Game->Assets, GameTextureType_blank, Result.Content);
            Platform->FreeFile(Result);
        }
        GameMemory->IsInitialized = true;
    }
    
    
    
    
    switch(Game->CurrentState) {
        case game_state_type::Splash: {
            UpdateAndRenderSplashState(Game, Platform, RenderCommands, DeltaTime);
        } break;
        case game_state_type::Main: {
            Platform->Log("Main!");
        } break;
        
    }
}