#ifndef GAME_MODE_MAIN_H
#define GAME_MODE_MAIN_H

#define CircleCap 1024
#define DotCap 1024
#define ParticleCap 256

#define ZLayPlayer 0.f
#define ZLayBullet 10.f
#define ZLayEnemy 20.f
#define ZLayParticles 25.f
#define ZLayDeathBomb 26.f
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

enum game_mode_main_state {
    MainState_Spawning,
    MainState_Normal,
    MainState_PlayerDied,
    MainState_Cleanup,
};


struct death_bomb {
    static constexpr f32 GrowthSpeed = 1000.f;
    f32 Radius;
    v2f Position;
};




struct game_mode_main {
    game_mode_main_state State;
    player Player;
    
    game_camera Camera;
    
    list<bullet> CircleBullets;
    list<bullet> DotBullets;
    list<enemy> Enemies;
    queue<particle> Particles;
    death_bomb DeathBomb;
    
    wave Wave;
    rng_series Rng;
    
    // Audio handles
    game_audio_mixer_handle BgmHandle;
};

static inline void
Main_UpdateDeathBomb(game_mode_main* Mode, f32 DeltaTime) {
    death_bomb* DeathBomb = &Mode->DeathBomb;
    DeathBomb->Radius += DeathBomb->GrowthSpeed * DeltaTime;
    
    // NOTE(Momo)
}

static inline void
Main_RenderDeathBomb(game_mode_main* Mode, mailbox* RenderCommands)
{
    death_bomb* DeathBomb = &Mode->DeathBomb;
    // Circle?
    Renderer_DrawCircle2f(RenderCommands,
                          Circle2f_Create(DeathBomb->Position, DeathBomb->Radius),
                          5.f, 32, Color_White, ZLayDeathBomb);
}


#include "game_mode_main_player.cpp"
#include "game_mode_main_bullet.cpp"
#include "game_mode_main_enemy.cpp"
#include "game_mode_main_wave.cpp"
#include "game_mode_main_particle.cpp"
#include "game_mode_main_collision.cpp"
#include "game_mode_main_debug.cpp"


// TODO: split state logic into files?
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
        Player->IsDead = false;
    }
    Mode->Wave.IsDone = true;
    
#if 0    
    Success = AudioMixer_Play(&TranState->Mixer, Sound_Test, false, &Mode->BgmHandle);
    if (!Success) {
        return false;
        
    }
#endif
    
    
    Mode->State = MainState_Normal;
    return true; 
    
}

static inline void
Main_UpdateNormal(permanent_state* PermState, 
                  transient_state* TranState,
                  debug_state* DebugState,
                  mailbox* RenderCommands, 
                  platform_input* Input,
                  f32 DeltaTime) 
{
    assets* Assets = &TranState->Assets;
    game_mode_main* Mode = PermState->MainMode;
    Main_UpdateInput(Mode, Input);
    Main_UpdatePlayer(Mode, DeltaTime);    
    Main_UpdateBullets(Mode, DeltaTime);
    Main_UpdateWaves(Mode, Assets, DeltaTime);
    Main_UpdateEnemies(Mode, Assets, DeltaTime); 
    Main_UpdatePlayerBulletCollision(Mode, Assets, DeltaTime);
    Main_UpdateParticles(Mode, DeltaTime);
    
    
    // NOTE(Momo): if player's dead, do dead stuff
    if(Mode->Player.IsDead) 
    {
        // NOTE(Momo): Drop the death bomb
        Mode->DeathBomb.Radius = 0.f;
        Mode->DeathBomb.Position = Mode->Player.Position;
        
        Mode->State = MainState_PlayerDied;
        Mode->Player.Position = V2f_Create(-1000.f, -1000.f);
    }
    
    
    Main_RenderPlayer(Mode, Assets, RenderCommands);
    Main_RenderBullets(Mode, Assets, RenderCommands);
    Main_RenderEnemies(Mode, Assets, RenderCommands);
    Main_RenderParticles(Mode, Assets, RenderCommands);
    
}

static inline void
Main_UpdatePlayerDied(permanent_state* PermState, 
                      transient_state* TranState,
                      debug_state* DebugState,
                      mailbox* RenderCommands, 
                      platform_input* Input,
                      f32 DeltaTime) 
{
    // Everything stops
    game_mode_main* Mode = PermState->MainMode;
    assets* Assets = &TranState->Assets;
    
    Main_UpdateDeathBomb(Mode, DeltaTime);
    Main_UpdateParticles(Mode, DeltaTime);
    
    Main_RenderPlayer(Mode, Assets, RenderCommands);
    Main_RenderBullets(Mode, Assets, RenderCommands);
    Main_RenderEnemies(Mode, Assets, RenderCommands);
    Main_RenderParticles(Mode, Assets, RenderCommands);
    Main_RenderDeathBomb(Mode, RenderCommands);
    
}

static inline void
Main_UpdateSpawning() {
}

static inline void
Main_UpdateCleaning() {
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
    
    switch(Mode->State) {
        case MainState_Spawning: {
        } break;
        case MainState_Normal: {
            Main_UpdateNormal(PermState, TranState, DebugState, RenderCommands, Input, DeltaTime);
        }break;
        case MainState_PlayerDied: {
            Main_UpdatePlayerDied(PermState, TranState, DebugState, RenderCommands, Input, DeltaTime);
        } break;
        case MainState_Cleanup: {
        } break;
        
    }
    // NOTE(Momo): Render
    
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
