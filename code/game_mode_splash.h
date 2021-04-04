#ifndef GAME_MODE_SPLASH_H
#define GAME_MODE_SPLASH_H

#include "game.h"
#include "mm_easing.h"

struct splash_image_entity {
    v3f Scale;
    v3f Position;
    c4f Colors;
    
    atlas_aabb_id TextureAabb;
    
    f32 CountdownTimer;
    f32 CountdownDuration;
    
    f32 Timer;
    f32 Duration;
    f32 StartX;
    f32 EndX;
};

static inline void
UpdateSplashImageEntity(splash_image_entity* Entity, 
                        assets* Assets,
                        mailbox* RenderCommands, 
                        f32 DeltaTime) 
{
    Entity->CountdownTimer += DeltaTime;
    if (Entity->CountdownTimer <= Entity->CountdownDuration) 
        return;
    
    // NOTE(Momo): Update
    f32 Ease = EaseOutBounce(Clamp(Entity->Timer/Entity->Duration, 0.f, 1.f));
    
    Entity->Position.X = Entity->StartX + (Entity->EndX - Entity->StartX) * Ease; 
    Entity->Timer += DeltaTime;
    
    // NOTE(Momo): Render
    m44f T = M44f_Translation(Entity->Position.X,
                              Entity->Position.Y,
                              Entity->Position.Z);
    m44f S = M44f_Scale(Entity->Scale.X,
                        Entity->Scale.Y,
                        1.f);
    Draw_TexturedQuadFromAtlasAabb(RenderCommands,
                                   Assets,
                                   Entity->TextureAabb,
                                   M44f_Concat(T,S),
                                   Entity->Colors);
    
    
}

struct splash_blackout_entity {
    v3f Scale;
    v3f Position;
    c4f Colors;
    
    f32 CountdownTimer;
    f32 CountdownDuration;
    
    f32 Timer;
    f32 Duration;
};

static inline void
UpdateSplashBlackout(splash_blackout_entity* Entity, 
                     assets* Assets, 
                     mailbox* RenderCommands,
                     f32 DeltaTime) 
{
    Entity->CountdownTimer += DeltaTime;
    if (Entity->CountdownTimer <= Entity->CountdownDuration) 
        return;
    
    Entity->Colors.A = EaseInSine(Clamp(Entity->Timer/Entity->Duration, 0.f, 1.f));
    Entity->Timer += DeltaTime;
    
    // NOTE(Momo): Render
    m44f T = M44f_Translation(Entity->Position.X,
                              Entity->Position.Y,
                              Entity->Position.Z);
    m44f S = M44f_Scale(Entity->Scale.X, Entity->Scale.Y, 1.f);
    m44f TS = M44f_Concat(T,S);
    Renderer_DrawQuad(RenderCommands, Entity->Colors, TS);
    
}

struct game_mode_splash {
    splash_image_entity SplashImg[2];
    splash_blackout_entity SplashBlackout;
};


static inline void
InitSplashMode(permanent_state* PermState) {
    game_mode_splash* Mode = PermState->SplashMode;
    // NOTE(Momo): Create entities
    {
        Mode->SplashImg[0].Position = V3f_Create( 0.f, 0.f, 0.f );
        Mode->SplashImg[0].Scale = V3f_Create(400.f, 400.f, 1.f);
        Mode->SplashImg[0].Colors = C4f_Create(1.f, 1.f, 1.f, 1.f);
        Mode->SplashImg[0].TextureAabb = AtlasAabb_Ryoji;
        Mode->SplashImg[0].CountdownTimer = 0.f;
        Mode->SplashImg[0].CountdownDuration = 1.f;
        Mode->SplashImg[0].Timer = 0.f;
        Mode->SplashImg[0].Duration = 2.f;
        Mode->SplashImg[0].StartX = -1000.f;
        Mode->SplashImg[0].EndX = -200.f;
        
        Mode->SplashImg[1].Position = {};
        Mode->SplashImg[1].Scale = V3f_Create(400.f, 400.f, 1.f);
        Mode->SplashImg[1].Colors = C4f_Create(1.f, 1.f, 1.f, 1.f);
        Mode->SplashImg[1].TextureAabb = AtlasAabb_Yuu;
        Mode->SplashImg[1].CountdownTimer = 0.f;
        Mode->SplashImg[1].CountdownDuration = 1.f;
        Mode->SplashImg[1].Timer = 0.f;
        Mode->SplashImg[1].Duration = 2.f;
        Mode->SplashImg[1].StartX = 1000.f;
        Mode->SplashImg[1].EndX = 200.f;
        
        Mode->SplashBlackout.Position = V3f_Create(0.f, 0.f, 1.0f);
        Mode->SplashBlackout.Scale = V3f_Create(1600.f, 900.f, 1.f);
        Mode->SplashBlackout.Colors = C4f_Create(0.f, 0.f, 0.f, 0.0f);
        Mode->SplashBlackout.CountdownTimer = 0.f;
        Mode->SplashBlackout.CountdownDuration = 3.f;
        Mode->SplashBlackout.Timer = 0.f;
        Mode->SplashBlackout.Duration = 1.f;
        
    }
}

static inline void
UpdateSplashMode(permanent_state* PermState,
                 transient_state* TranState,
                 mailbox* RenderCommands,
                 game_input* Input,
                 f32 DeltaTime)
{
    SwitchToGameCoords(RenderCommands);
    game_mode_splash* Mode = PermState->SplashMode;
    assets* Assets = &TranState->Assets;
    
    
    for (u32 I = 0; I < ArrayCount(Mode->SplashImg); ++I) {
        UpdateSplashImageEntity(Mode->SplashImg + I, 
                                Assets, 
                                RenderCommands, 
                                DeltaTime);
    }
    
    UpdateSplashBlackout(&Mode->SplashBlackout,
                         Assets,
                         RenderCommands,
                         DeltaTime);
    
    // NOTE(Momo): Exit 
    if (Mode->SplashBlackout.Timer >= Mode->SplashBlackout.Duration) {
        PermState->NextGameMode = GameModeType_Main;
    }
    
}

#endif //GAME_MODE_SPLASH_H
