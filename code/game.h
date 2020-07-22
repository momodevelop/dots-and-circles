#ifndef GAME_H
#define GAME_H





#include "ryoji_easing.h"

#include "yuu_platform.h"
#include "yuu_input.h"
#include "yuu_renderer.h"
#include "yuu_assets.h"

static platform_api* gPlatform;

//#include "game_state_splash.h"
//#include "game_state_main.h"



struct splash_blackout_entity {
    v3f Scale;
    v3f Position;
    c4f Colors;
    u32 TextureHandle;
    
    f32 CountdownTimer;
    f32 CountdownDuration;
    
    f32 Timer;
    f32 Duration;
    f32 StartY;
    f32 EndY;
};


// NOTE(Momo): State Splash Stuff
struct splash_image_entity {
    v3f Scale;
    v3f Position;
    c4f Colors;
    u32 TextureHandle;
    
    f32 CountdownTimer;
    f32 CountdownDuration;
    
    
    f32 Timer;
    f32 Duration;
    f32 StartX;
    f32 EndX;
};


struct game_state_splash {
    static constexpr u32 TypeId = 0;
    splash_image_entity SplashImg[2];
    splash_blackout_entity SplashBlackout;
};


struct game_state_main {
    static constexpr u32 TypeId = 1;
};



union game_state {
    game_state_splash Splash;
    game_state_main Main;
};
struct game {
    game_state GameState;
    u32 CurrentStateType;
    bool IsStateInitialized;
    
    game_assets Assets;
    
    memory_arena MainArena;
};

static inline void
UpdateAndRender(splash_image_entity* Entity, 
                game_assets* Assets,
                render_commands * RenderCommands, 
                f32 DeltaTime) {
    Entity->CountdownTimer += DeltaTime;
    if (Entity->CountdownTimer <= Entity->CountdownDuration) 
        return;
    
    // NOTE(Momo): Update
    f32 ease = EaseOutBounce(Clamp(Entity->Timer/Entity->Duration, 0.f, 1.f));
    
    Entity->Position.X = Entity->StartX + (Entity->EndX - Entity->StartX) * ease; 
    Entity->Timer += DeltaTime;
    
    // NOTE(Momo): Render
    m44f T = MakeTranslationMatrix(Entity->Position);
    m44f S = MakeScaleMatrix(Entity->Scale);
    
    // TODO(Momo): This part should be done by renderer?
    PushCommandTexturedQuad(RenderCommands, 
                            Entity->Colors, 
                            T*S,
                            Assets->Textures[Entity->TextureHandle]);
}


static inline void
UpdateAndRender(splash_blackout_entity* Entity, 
                game_assets* Assets, 
                render_commands* RenderCommands,
                f32 DeltaTime) 
{
    Entity->CountdownTimer += DeltaTime;
    if (Entity->CountdownTimer <= Entity->CountdownDuration) 
        return;
    
    f32 ease = EaseInSine(Clamp(Entity->Timer/Entity->Duration, 0.f, 1.f));
    Entity->Position.Y = Entity->StartY + (Entity->EndY - Entity->StartY) * ease; 
    Entity->Timer += DeltaTime;
    
    // NOTE(Momo): Render
    m44f T = MakeTranslationMatrix(Entity->Position);
    m44f S = MakeScaleMatrix(Entity->Scale);
    
    // TODO(Momo): This part should be done by renderer?
    PushCommandTexturedQuad(RenderCommands, 
                            Entity->Colors, 
                            T*S,
                            Assets->Textures[Entity->TextureHandle]);
    
}

static inline void
UpdateAndRenderSplashState(game* Game, 
                           platform_api* Platform, 
                           render_commands* RenderCommands, 
                           f32 DeltaTime) {
    game_state_splash* State = &Game->GameState.Splash; 
    if(!Game->IsStateInitialized) {
        // NOTE(Momo): Create entities
        {
            State->SplashImg[0].Position = { 0.f, 0.f, 0.f };
            State->SplashImg[0].Scale = { 200.f, 200.f };
            State->SplashImg[0].Colors = { 1.f, 1.f, 1.f, 1.f };
            State->SplashImg[0].TextureHandle = GameTextureType_ryoji;
            State->SplashImg[0].CountdownTimer = 0.f;
            State->SplashImg[0].CountdownDuration = 3.f;
            State->SplashImg[0].Timer = 0.f;
            State->SplashImg[0].Duration = 2.f;
            State->SplashImg[0].StartX = -1000.f;
            State->SplashImg[0].EndX = -200.f;
            
            State->SplashImg[1].Position = { 0.f };
            State->SplashImg[1].Scale = { 200.f, 200.f };
            State->SplashImg[1].Colors = { 1.f, 1.f, 1.f, 1.f };
            State->SplashImg[1].TextureHandle = GameTextureType_yuu;
            State->SplashImg[1].CountdownTimer = 0.f;
            State->SplashImg[1].CountdownDuration = 3.f;
            State->SplashImg[1].Timer = 0.f;
            State->SplashImg[1].Duration = 2.f;
            State->SplashImg[1].StartX = 1000.f;
            State->SplashImg[1].EndX = 200.f;
            
            State->SplashBlackout.Position = { 0.f, 900.f, 1.0f };
            State->SplashBlackout.Scale = { 800.f, 450.f };
            State->SplashBlackout.Colors = { 0.f, 0.f, 0.f, 0.f };
            State->SplashBlackout.TextureHandle = GameTextureType_blank;
            State->SplashBlackout.CountdownTimer = 0.f;
            State->SplashBlackout.CountdownDuration = 5.f;
            State->SplashBlackout.Timer = 0.f;
            State->SplashBlackout.Duration = 1.f;
            State->SplashBlackout.StartY = 900.f;
            State->SplashBlackout.EndY = 0.f;
        }
        Game->IsStateInitialized = true;
        Platform->Log("Splash state initialized!");
    }
    
    PushCommandClear(RenderCommands, { 0.0f, 0.3f, 0.3f, 0.f });
    
    for (u32 i = 0; i < 2; ++i) {
        UpdateAndRender(&State->SplashImg[i], 
                        &Game->Assets, 
                        RenderCommands, 
                        DeltaTime);
    }
    UpdateAndRender(&State->SplashBlackout,
                    &Game->Assets,
                    RenderCommands,
                    DeltaTime);
    
}



#endif //GAME_H
