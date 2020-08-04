/* date = July 30th 2020 0:41 pm */

#ifndef GAME_MODE_MAIN_H
#define GAME_MODE_MAIN_H

#include "game.h"
#include "game_mode_splash.h"

// NOTE(Momo): Entities ///////////////////////////////////
struct main_image_entity {
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

static inline void
Update(main_image_entity* Entity, 
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
                            {
                                0.5f, 0.5f, // top right
                                0.5f, 0.f, // bottom right
                                0.f, 0.f, // bottom left
                                0.f, 0.5f,  // top left
                            });
}

struct main_blackout_entity {
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
Update(main_blackout_entity* Entity, 
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
                                0.5f, 0.5f, // top right
                                0.5f, 0.f, // bottom right
                                0.f, 0.f, // bottom left
                                0.f, 0.5f,  // top left
                            });
    
}


// NOTE(Momo): Mode /////////////////////////////////////////////
struct game_mode_main {
    static constexpr u8 TypeId = 1;
    main_image_entity SplashImg[2];
    main_blackout_entity SplashBlackout;
    
};

static inline void 
InitMode(game_mode_main* Mode, game_state* GameState) {
    // NOTE(Momo): Create entities
    {
        Mode->SplashImg[0].Position = { 0.f, 0.f, 0.f };
        Mode->SplashImg[0].Scale = { 200.f, 200.f };
        Mode->SplashImg[0].Colors = { 1.f, 1.f, 1.f, 1.f };
        Mode->SplashImg[0].TextureHandle = GameBitmapHandle_Yuu;
        Mode->SplashImg[0].CountdownTimer = 0.f;
        Mode->SplashImg[0].CountdownDuration = 5.f;
        Mode->SplashImg[0].Timer = 0.f;
        Mode->SplashImg[0].Duration = 2.f;
        Mode->SplashImg[0].StartX = -1000.f;
        Mode->SplashImg[0].EndX = -200.f;
        
        Mode->SplashImg[1].Position = { 0.f };
        Mode->SplashImg[1].Scale = { 200.f, 200.f };
        Mode->SplashImg[1].Colors = { 1.f, 1.f, 1.f, 1.f };
        Mode->SplashImg[1].TextureHandle = GameBitmapHandle_Ryoji;
        Mode->SplashImg[1].CountdownTimer = 0.f;
        Mode->SplashImg[1].CountdownDuration = 5.f;
        Mode->SplashImg[1].Timer = 0.f;
        Mode->SplashImg[1].Duration = 2.f;
        Mode->SplashImg[1].StartX = 1000.f;
        Mode->SplashImg[1].EndX = 200.f;
        
        Mode->SplashBlackout.Position = { 0.f, 0.f, 1.0f };
        Mode->SplashBlackout.Scale = { 800.f, 450.f };
        Mode->SplashBlackout.Colors = { 0.f, 0.f, 0.f, 0.0f };
        Mode->SplashBlackout.TextureHandle = GameBitmapHandle_Blank;
        Mode->SplashBlackout.CountdownTimer = 0.f;
        Mode->SplashBlackout.CountdownDuration = 8.f;
        Mode->SplashBlackout.Timer = 0.f;
        Mode->SplashBlackout.Duration = 1.f;
    }
    Log("Main state initialized!");
}

static inline void
UpdateMode(game_mode_main* Mode,
           game_state* GameState, 
           render_commands* RenderCommands, 
           game_input* Input,
           f32 DeltaTime) 
{
    
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
        Log("Main state exit");
        SetGameMode<game_mode_splash>(GameState);
        
    }
}



#endif //GAME_MODE_MAIN_H
