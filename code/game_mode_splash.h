#ifndef GAME_MODE_SPLASH_H
#define GAME_MODE_SPLASH_H

#include "ryoji_easing.h"

#include "game.h"
#include "game_mode_main.h"

struct splash_image_entity {
    v3f Scale;
    v3f Position;
    c4f Colors;
    
    quad2f TextureCoords;
    
    u32 TextureHandle;
    
    f32 CountdownTimer;
    f32 CountdownDuration;
    
    f32 Timer;
    f32 Duration;
    f32 StartX;
    f32 EndX;
};

static inline void
Update(splash_image_entity* Entity, 
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
    m44f T = TranslationMatrix(Entity->Position);
    m44f S = ScaleMatrix(Entity->Scale);
    
    // TODO(Momo): This part should be done by renderer?
    PushCommandTexturedQuad(RenderCommands, 
                            Entity->Colors, 
                            T*S,
                            Entity->TextureHandle,
                            Entity->TextureCoords);
}

struct splash_blackout_entity {
    v3f Scale;
    v3f Position;
    c4f Colors;
    u32 TextureHandle;
    
    f32 CountdownTimer;
    f32 CountdownDuration;
    
    f32 Timer;
    f32 Duration;
};

static inline void
Update(splash_blackout_entity* Entity, 
       game_assets* Assets, 
       render_commands* RenderCommands,
       f32 DeltaTime) 
{
    Entity->CountdownTimer += DeltaTime;
    if (Entity->CountdownTimer <= Entity->CountdownDuration) 
        return;
    
    Entity->Colors.A = EaseInSine(Clamp(Entity->Timer/Entity->Duration, 0.f, 1.f));
    Entity->Timer += DeltaTime;
    
    // NOTE(Momo): Render
    m44f T = TranslationMatrix(Entity->Position);
    m44f S = ScaleMatrix(Entity->Scale);
    
    // TODO(Momo): This part should be done by renderer?
    PushCommandTexturedQuad(RenderCommands, 
                            Entity->Colors, 
                            T*S,
                            Entity->TextureHandle,
                            {
                                1.f, 1.f, // top right
                                1.f, 0.f, // bottom right
                                0.f, 0.f, // bottom left
                                0.f, 1.f,  // top left
                            });
    
}

struct game_mode_splash {
    static constexpr u8 TypeId = 0;
    splash_image_entity SplashImg[2];
    splash_blackout_entity SplashBlackout;
};


static inline void
InitMode(game_mode_splash* Mode, game_state* GameState) {
    // NOTE(Momo): Create entities
    {
        Mode->SplashImg[0].Position = { 0.f, 0.f, 0.f };
        Mode->SplashImg[0].Scale = { 400.f, 400.f };
        Mode->SplashImg[0].Colors = { 1.f, 1.f, 1.f, 1.f };
        Mode->SplashImg[0].TextureHandle = GameBitmapHandle_Ryoji;
        Mode->SplashImg[0].CountdownTimer = 0.f;
        Mode->SplashImg[0].CountdownDuration = 5.f;
        Mode->SplashImg[0].Timer = 0.f;
        Mode->SplashImg[0].Duration = 2.f;
        Mode->SplashImg[0].StartX = -1000.f;
        Mode->SplashImg[0].EndX = -200.f;
        Mode->SplashImg[0].TextureCoords = {
            0.0f, 0.0f,  // top left
            1.0f, 0.0f, // top right
            0.f, 1.f, // bottom left
            1.0f, 1.f, // bottom right
        };
        
        Mode->SplashImg[1].Position = { 0.f };
        Mode->SplashImg[1].Scale = { 400.f, 400.f };
        Mode->SplashImg[1].Colors = { 1.f, 1.f, 1.f, 1.f };
        Mode->SplashImg[1].TextureHandle = GameBitmapHandle_Yuu;
        Mode->SplashImg[1].CountdownTimer = 0.f;
        Mode->SplashImg[1].CountdownDuration = 5.f;
        Mode->SplashImg[1].Timer = 0.f;
        Mode->SplashImg[1].Duration = 2.f;
        Mode->SplashImg[1].StartX = 1000.f;
        Mode->SplashImg[1].EndX = 200.f;
        Mode->SplashImg[1].TextureCoords = {
            0.0f, 0.0f,  // top left
            1.0f, 0.0f, // top right
            0.f, 1.f, // bottom left
            1.0f, 1.f, // bottom right
        };
        
        Mode->SplashBlackout.Position = { 0.f, 0.f, 1.0f };
        Mode->SplashBlackout.Scale = { 1600.f, 900.f };
        Mode->SplashBlackout.Colors = { 0.f, 0.f, 0.f, 0.0f };
        Mode->SplashBlackout.TextureHandle = GameBitmapHandle_Blank;
        Mode->SplashBlackout.CountdownTimer = 0.f;
        Mode->SplashBlackout.CountdownDuration = 8.f;
        Mode->SplashBlackout.Timer = 0.f;
        Mode->SplashBlackout.Duration = 1.f;
    }
    Log("Splash state initialized!");
}

static inline void
UpdateMode(game_mode_splash* Mode,
           game_state* GameState, 
           render_commands* RenderCommands, 
           game_input* Input,
           f32 DeltaTime) {
    
#if INTERNAL
    if (ProcessMetaInput(GameState, Input)) {
        return;
    }
#endif
    
    PushCommandClear(RenderCommands, { 0.0f, 0.3f, 0.3f, 0.f });
    for (u32 i = 0; i < 2; ++i) {
        Update(&Mode->SplashImg[i], 
               GameState->Assets, 
               RenderCommands, 
               DeltaTime);
    }
    
    Update(&Mode->SplashBlackout,
           GameState->Assets,
           RenderCommands,
           DeltaTime);
    
    // NOTE(Momo): Exit 
    if (Mode->SplashBlackout.Timer >= Mode->SplashBlackout.Duration) {
        Log("Splash state exit");
        SetGameMode<game_mode_main>(GameState);
    }
    
    
    
}

#endif //GAME_MODE_SPLASH_iH