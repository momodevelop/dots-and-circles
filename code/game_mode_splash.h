#ifndef GAME_MODE_SPLASH_H
#define GAME_MODE_SPLASH_H

#include "game.h"
#include "mm_easing.h"

struct splash_image_entity {
    v3f Scale;
    v3f Position;
    v4f Colors;
    
    bitmap_id BitmapHandle;
    
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
       mailbox* RenderCommands, 
       f32 DeltaTime) {
    Entity->CountdownTimer += DeltaTime;
    if (Entity->CountdownTimer <= Entity->CountdownDuration) 
        return;
    
    // NOTE(Momo): Update
    f32 ease = EaseOutBounce(Clamp(Entity->Timer/Entity->Duration, 0.f, 1.f));
    
    Entity->Position.X = Entity->StartX + (Entity->EndX - Entity->StartX) * ease; 
    Entity->Timer += DeltaTime;
    
    // NOTE(Momo): Render
    m44f T = M44fTranslation(Entity->Position);
    m44f S = M44fScale(Entity->Scale);
    
    // TODO(Momo): This part should be done by renderer?
    PushCommandDrawTexturedQuad(RenderCommands, Entity->Colors, T*S,  Entity->BitmapHandle);
}

struct splash_blackout_entity {
    v3f Scale;
    v3f Position;
    v4f Colors;
    
    f32 CountdownTimer;
    f32 CountdownDuration;
    
    f32 Timer;
    f32 Duration;
};

static inline void
Update(splash_blackout_entity* Entity, 
       game_assets* Assets, 
       mailbox* RenderCommands,
       f32 DeltaTime) 
{
    Entity->CountdownTimer += DeltaTime;
    if (Entity->CountdownTimer <= Entity->CountdownDuration) 
        return;
    
    Entity->Colors.A = EaseInSine(Clamp(Entity->Timer/Entity->Duration, 0.f, 1.f));
    Entity->Timer += DeltaTime;
    
    // NOTE(Momo): Render
    m44f T = M44fTranslation(Entity->Position);
    m44f S = M44fScale(Entity->Scale);
    
    // TODO(Momo): This part should be done by renderer?
    PushCommandDrawQuad(RenderCommands, Entity->Colors, T*S);
    
}

struct game_mode_splash {
    splash_image_entity SplashImg[2];
    splash_blackout_entity SplashBlackout;
};


static inline void
InitSplashMode(game_state* GameState) {
    game_mode_splash* Mode = GameState->SplashMode;
    // NOTE(Momo): Create entities
    {
        Mode->SplashImg[0].Position = { 0.f, 0.f, 0.f };
        Mode->SplashImg[0].Scale = { 400.f, 400.f };
        Mode->SplashImg[0].Colors = { 1.f, 1.f, 1.f, 1.f };
        Mode->SplashImg[0].BitmapHandle = Bitmap_Ryoji;
        Mode->SplashImg[0].CountdownTimer = 0.f;
        Mode->SplashImg[0].CountdownDuration = 1.f;
        Mode->SplashImg[0].Timer = 0.f;
        Mode->SplashImg[0].Duration = 2.f;
        Mode->SplashImg[0].StartX = -1000.f;
        Mode->SplashImg[0].EndX = -200.f;
        
        Mode->SplashImg[1].Position = { 0.f };
        Mode->SplashImg[1].Scale = { 400.f, 400.f };
        Mode->SplashImg[1].Colors = { 1.f, 1.f, 1.f, 1.f };
        Mode->SplashImg[1].BitmapHandle = Bitmap_Yuu;
        Mode->SplashImg[1].CountdownTimer = 0.f;
        Mode->SplashImg[1].CountdownDuration = 1.f;
        Mode->SplashImg[1].Timer = 0.f;
        Mode->SplashImg[1].Duration = 2.f;
        Mode->SplashImg[1].StartX = 1000.f;
        Mode->SplashImg[1].EndX = 200.f;
        
        Mode->SplashBlackout.Position = { 0.f, 0.f, 1.0f };
        Mode->SplashBlackout.Scale = { 1600.f, 900.f };
        Mode->SplashBlackout.Colors = { 0.f, 0.f, 0.f, 0.0f };
        Mode->SplashBlackout.CountdownTimer = 0.f;
        Mode->SplashBlackout.CountdownDuration = 3.f;
        Mode->SplashBlackout.Timer = 0.f;
        Mode->SplashBlackout.Duration = 1.f;

#if INTERNAL
        

#endif 
    }
}

static inline void
UpdateSplashMode(game_state* GameState, 
       mailbox* RenderCommands,
       game_input* Input,
       f32 DeltaTime)
{
    game_mode_splash* Mode = GameState->SplashMode;
    PushCommandClearColor(RenderCommands, { 0.0f, 0.3f, 0.3f, 0.f });

    PushCommandOrthoCamera(RenderCommands, 
            v3f{}, 
            CenteredRect( 
                v3f{ DesignWidth, DesignHeight, DesignDepth }, 
                v3f{ 0.5f, 0.5f, 0.5f }
            )
    );
    
    for (u32 I = 0; I < ArrayCount(Mode->SplashImg); ++I) {
        Update(Mode->SplashImg + I, 
               &GameState->Assets, 
               RenderCommands, 
               DeltaTime);
    }
    
    Update(&Mode->SplashBlackout,
            &GameState->Assets,
            RenderCommands,
            DeltaTime);
        
    // NOTE(Momo): Exit 
    if (Mode->SplashBlackout.Timer >= Mode->SplashBlackout.Duration) {
        GameState->NextModeType = GameModeType_Splash;
    }

}

#endif //GAME_MODE_SPLASH_H
