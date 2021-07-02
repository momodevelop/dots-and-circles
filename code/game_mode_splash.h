#ifndef GAME_MODE_SPLASH_H
#define GAME_MODE_SPLASH_H

#include "game.h"

struct splash_image_entity {
    v3f Scale;
    v3f position;
    c4f Colors;
    
    Image_ID TextureAabb;
    
    f32 CountdownTimer;
    f32 CountdownDuration;
    
    f32 Timer;
    f32 duration;
    f32 StartX;
    f32 EndX;
};

static inline void
UpdateSplashImageEntity(splash_image_entity* Entity, 
                        f32 DeltaTime) 
{
    Entity->CountdownTimer += DeltaTime;
    if (Entity->CountdownTimer <= Entity->CountdownDuration) 
        return;
    
    // NOTE(Momo): Update
    f32 Ease = ease_out_bounce(CLAMP(Entity->Timer/Entity->duration, 0.f, 1.f));
    
    Entity->position.x = Entity->StartX + (Entity->EndX - Entity->StartX) * Ease; 
    Entity->Timer += DeltaTime;
    
    // NOTE(Momo): Render
    m44f T = m44f::create_translation(Entity->position.x,
                                      Entity->position.y,
                                      Entity->position.z);
    m44f S = m44f::create_scale(Entity->Scale.x,
                                Entity->Scale.y,
                                1.f);
    draw_textured_quad_from_image(Entity->TextureAabb,
                                  T*S,
                                  Entity->Colors);
    
    
}

struct splash_blackout_entity {
    v3f Scale;
    v3f position;
    c4f Colors;
    
    f32 CountdownTimer;
    f32 CountdownDuration;
    
    f32 Timer;
    f32 duration;
};

static inline void
UpdateSplashBlackout(splash_blackout_entity* Entity, 
                     f32 DeltaTime) 
{
    Entity->CountdownTimer += DeltaTime;
    if (Entity->CountdownTimer <= Entity->CountdownDuration) 
        return;
    
    Entity->Colors.a = ease_in_sine(CLAMP(Entity->Timer/Entity->duration, 0.f, 1.f));
    Entity->Timer += DeltaTime;
    
    // NOTE(Momo): Render
    m44f T = m44f::create_translation(Entity->position.x,
                                      Entity->position.y,
                                      Entity->position.z);
    m44f S = m44f::create_scale(Entity->Scale.x, Entity->Scale.y, 1.f);
    
    Renderer_DrawQuad(G_Renderer, Entity->Colors, T*S);
    
}

struct game_mode_splash {
    splash_image_entity SplashImg[2];
    splash_blackout_entity SplashBlackout;
};


static inline void
SplashMode_Init(permanent_state* PermState) {
    game_mode_splash* Mode = PermState->SplashMode;
    G_Platform->ShowCursorFp();
    
    // NOTE(Momo): Create entities
    {
        Mode->SplashImg[0].position = v3f::create( 0.f, 0.f, 0.f );
        Mode->SplashImg[0].Scale = v3f::create(400.f, 400.f, 1.f);
        Mode->SplashImg[0].Colors = c4f::create(1.f, 1.f, 1.f, 1.f);
        Mode->SplashImg[0].TextureAabb = IMAGE_RYOJI;
        Mode->SplashImg[0].CountdownTimer = 0.f;
        Mode->SplashImg[0].CountdownDuration = 1.f;
        Mode->SplashImg[0].Timer = 0.f;
        Mode->SplashImg[0].duration = 2.f;
        Mode->SplashImg[0].StartX = -1000.f;
        Mode->SplashImg[0].EndX = -200.f;
        
        Mode->SplashImg[1].position = {};
        Mode->SplashImg[1].Scale = v3f::create(400.f, 400.f, 1.f);
        Mode->SplashImg[1].Colors = c4f::create(1.f, 1.f, 1.f, 1.f);
        Mode->SplashImg[1].TextureAabb = IMAGE_YUU;
        Mode->SplashImg[1].CountdownTimer = 0.f;
        Mode->SplashImg[1].CountdownDuration = 1.f;
        Mode->SplashImg[1].Timer = 0.f;
        Mode->SplashImg[1].duration = 2.f;
        Mode->SplashImg[1].StartX = 1000.f;
        Mode->SplashImg[1].EndX = 200.f;
        
        Mode->SplashBlackout.position = v3f::create(0.f, 0.f, 1.0f);
        Mode->SplashBlackout.Scale = v3f::create(1600.f, 900.f, 1.f);
        Mode->SplashBlackout.Colors = c4f::create(0.f, 0.f, 0.f, 0.0f);
        Mode->SplashBlackout.CountdownTimer = 0.f;
        Mode->SplashBlackout.CountdownDuration = 3.f;
        Mode->SplashBlackout.Timer = 0.f;
        Mode->SplashBlackout.duration = 1.f;
        
    }
}

static inline void
SplashMode_Update(permanent_state* PermState,
                  transient_state* TranState,
                  f32 DeltaTime)
{
    game_mode_splash* Mode = PermState->SplashMode;
    
    // NOTE: Camera
    {
        v3f position = v3f::create(0.f, 0.f, 0.f);
        v3f Anchor = v3f::create(0.5f, 0.5f, 0.5f);
        c4f Color = c4f::create(0.f, 0.3f, 0.3f, 1.f);
        v3f Dimensions = v3f::create(800.f,
                                     800.f,
                                     800.f);
        
        Renderer_ClearColor(G_Renderer, Color);
        
        aabb3f Frustum = aabb3f::create_centered(Dimensions, Anchor);
        Renderer_SetOrthoCamera(G_Renderer, 
                                position, 
                                Frustum);
    }
    
    for (u32 I = 0; I < ARRAY_COUNT(Mode->SplashImg); ++I) {
        UpdateSplashImageEntity(Mode->SplashImg + I, 
                                DeltaTime);
    }
    
    UpdateSplashBlackout(&Mode->SplashBlackout,
                         DeltaTime);
    
    // NOTE(Momo): Exit 
    if (Mode->SplashBlackout.Timer >= Mode->SplashBlackout.duration) {
        PermState->NextGameMode = GameModeType_Main;
    }
    
}

#endif //GAME_MODE_SPLASH_H
