#ifndef GAME_MODE_MAIN_H
#define GAME_MODE_MAIN_H

#define CircleCap 1024
#define DotCap 1024
#define ParticleCap 256

#define ZLayPlayer 0.f
#define ZLayBullet 10.f
#define ZLayEnemy 20.f
#define ZLayParticles 25.f

#define ZLayDebug 40.f


enum mood_type {
    MoodType_Dot,
    MoodType_Circle,
    
    MoodType_Count,
};

#include "game_mode_main_enemy.h"
#include "game_mode_main_player.h"
#include "game_mode_main_bullet.h"
#include "game_mode_main_wave.h"
#include "game_mode_main_particle.h"

struct game_mode_main {
    player Player;
    game_camera Camera;
    
    
    list<bullet> CircleBullets;
    list<bullet> DotBullets;
    list<enemy> Enemies;
    queue<particle> Particles;
    
    wave Wave;
    rng_series Rng;
    
    // Audio handles
    game_audio_mixer_handle BgmHandle;
};


#include "game_mode_main_player.cpp"
#include "game_mode_main_bullet.cpp"
#include "game_mode_main_enemy.cpp"
#include "game_mode_main_wave.cpp"
#include "game_mode_main_particle.cpp"
#include "game_mode_main_collision.cpp"
#include "game_mode_main_debug.cpp"

static inline b8 
Main_Init(permanent_state* PermState,
          transient_state* TranState,
          debug_state* DebugState) 
{
    game_mode_main* Mode = PermState->MainMode;
    arena* ModeArena = PermState->ModeArena;
    
    // NOTE(Momo): Init camera
    {
        Mode->Camera.Position = V3f_Create(0.f, 0.f, 0.f);
        Mode->Camera.Anchor = V3f_Create(0.5f, 0.5f, 0.5f);
        Mode->Camera.Color = Color_Grey2;
        Mode->Camera.Dimensions = V3f_Create(Game_DesignWidth,
                                             Game_DesignHeight,
                                             Game_DesignDepth);
    }
    
    b8 Success = false;
    
    Success = List_New(&Mode->DotBullets, ModeArena, DotCap);
    if (!Success) {
        return false;
    }
    
    Success = List_New(&Mode->CircleBullets, ModeArena, CircleCap);
    if (!Success) {
        return false;
    }
    
    Success = List_New(&Mode->Enemies, ModeArena, EnemyCap);
    if (!Success) {
        return false;
    }
    
    Success = Queue_New(&Mode->Particles, ModeArena, ParticleCap);
    if (!Success) {
        return false;
    }
    
    Mode->Wave.IsDone = true;
    Mode->Rng = Rng_Seed(0); // TODO: Used system clock for seed.
    
    assets* Assets = &TranState->Assets;
    player* Player = &Mode->Player;
    {
        Player->Position = {};
        Player->PrevPosition = {};
        Player->Size = V2f_Create( 32.f, 32.f );
        Player->HitCircle = { v2f{}, 16.f};
        
        // NOTE(Momo): We start as Dot
        Player->MoodType = MoodType_Dot;
        Player->DotImageAlpha = 1.f;
        Player->DotImageAlphaTarget = 1.f;
        
        Player->DotImageTransitionDuration = 0.1f;
        Player->DotImageTransitionTimer = Player->DotImageTransitionDuration;
    }
    Mode->Wave.IsDone = true;
    
#if 0    
    Success = AudioMixer_Play(&TranState->Mixer, Sound_Test, false, &Mode->BgmHandle);
    if (!Success) {
        return false;
    }
#endif
    
    return true; 
    
}


static inline void
Main_Update(permanent_state* PermState, 
            transient_state* TranState,
            debug_state* DebugState,
            mailbox* RenderCommands, 
            platform_input* Input,
            f32 DeltaTime) 
{
    game_mode_main* Mode = PermState->MainMode;
    Camera_Set(&Mode->Camera, RenderCommands);
    
    assets* Assets = &TranState->Assets;
    
    // NOTE(Momo): Update
    Main_UpdateInput(Mode, Input);
    Main_UpdatePlayer(Mode, DeltaTime);    
    Main_UpdateBullets(Mode, DeltaTime);
    Main_UpdateWaves(Mode, Assets, DeltaTime);
    Main_UpdateEnemies(Mode, Assets, DeltaTime); 
    Main_UpdateCollision(Mode, Assets, DeltaTime);
    Main_UpdateParticles(Mode, DeltaTime);
    
    
    // NOTE(Momo): Render
    Main_RenderPlayer(Mode, Assets, RenderCommands);
    Main_RenderBullets(Mode, Assets, RenderCommands);
    Main_RenderEnemies(Mode, Assets, RenderCommands);
    Main_RenderParticles(Mode, Assets, RenderCommands);
    //Main_RenderDebugLines(Mode, RenderCommands);
    
    u8_cstr Buffer = {};
    U8CStr_InitFromSiStr(&Buffer, "Dots: ");
    DebugInspector_PushU32(&DebugState->Inspector,
                           Buffer,
                           Mode->DotBullets.Count);
    U8CStr_InitFromSiStr(&Buffer, "Circles: ");
    DebugInspector_PushU32(&DebugState->Inspector, 
                           Buffer, 
                           Mode->CircleBullets.Count);
    
    U8CStr_InitFromSiStr(&Buffer, "Bullets: ");
    DebugInspector_PushU32(&DebugState->Inspector, 
                           Buffer, 
                           Mode->DotBullets.Count + Mode->CircleBullets.Count);
    
    
    U8CStr_InitFromSiStr(&Buffer, "Enemies: ");
    DebugInspector_PushU32(&DebugState->Inspector, 
                           Buffer, 
                           Mode->Enemies.Count);
}

#endif //GAME_MODE_H
