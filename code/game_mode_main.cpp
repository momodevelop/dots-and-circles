#ifndef GAME_MODE_MAIN_CPP
#define GAME_MODE_MAIN_CPP


#include "game_mode_main_player.cpp"
#include "game_mode_main_bullet.cpp"
#include "game_mode_main_enemy.cpp"
#include "game_mode_main_wave.cpp"
#include "game_mode_main_particle.cpp"
#include "game_mode_main_collision.cpp"
#include "game_mode_main_debug.cpp"
#include "game_mode_main_death_bomb.cpp"

static inline b8 
MainMode_Init(permanent_state* PermState,
              transient_state* TranState,
              debug_state* DebugState) 
{
    game_mode_main* Mode = PermState->MainMode;
    Arena* ModeArena = PermState->ModeArena;
    
    G_Platform->HideCursorFp();
    
    // NOTE(Momo): Init camera
    {
        Mode->Camera.Position = v3f::create(0.f, 0.f, 0.f);
        Mode->Camera.Anchor = v3f::create(0.5f, 0.5f, 0.5f);
        Mode->Camera.Color = C4F_GREY2;
        Mode->Camera.Dimensions = v3f::create(Game_DesignWidth,
                                              Game_DesignHeight,
                                              Game_DesignDepth);
    }
    
    b8 Success = false;
    
    Success = Mode->DotBullets.alloc(ModeArena, DotCap);
    if (!Success) {
        return false;
    }
    
    Success = Mode->CircleBullets.alloc(ModeArena, CircleCap);
    if (!Success) {
        return false;
    }
    
    Success = Mode->Enemies.alloc(ModeArena, EnemyCap);
    if (!Success) {
        return false;
    }
    
    Success = Mode->Particles.alloc(ModeArena, ParticleCap);
    if (!Success) {
        return false;
    }
    
    
    Success = Mode->Score.alloc(ModeArena, 128);
    if (!Success) {
        return false;
    }
    Mode->Wave.IsDone = true;
    Mode->rng = Rng_Series::create(0); // TODO: Used system clock for seed.
    
    assets* Assets = &TranState->Assets;
    player* Player = &Mode->Player;
    {
        Player->Position = {};
        Player->PrevPosition = {};
        Player->Size = Player->MaxSize;
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
    
    
    Camera_Set(&Mode->Camera);
    
    Mode->State = Main_StateType_Spawning;
    return true; 
    
}

static inline void
Main_StateNormal_Update(permanent_state* PermState, 
                        transient_state* TranState,
                        debug_state* DebugState,
                        f32 DeltaTime) 
{
    assets* Assets = &TranState->Assets;
    game_mode_main* Mode = PermState->MainMode;
    
    Main_UpdateInput(Mode);
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
        
        Mode->State = Main_StateType_PlayerDied;
        Mode->Player.Position = v2f::create(-1000.f, -1000.f);
    }
    
    Main_RenderPlayer(Mode);
    Main_RenderBullets(Mode);
    Main_RenderEnemies(Mode);
    Main_RenderParticles(Mode);
    Main_RenderScore(Mode);
    
}



static inline void
Main_StatePlayerDied_Update(permanent_state* PermState, 
                            transient_state* TranState,
                            debug_state* DebugState,
                            f32 DeltaTime) 
{
    // Everything stops
    game_mode_main* Mode = PermState->MainMode;
    assets* Assets = &TranState->Assets;
    
    Main_UpdateDeathBomb(Mode, DeltaTime);
    Main_UpdateParticles(Mode, DeltaTime);
    
    //Main_RenderScore(Mode, &TranState->Arena);
    Main_RenderPlayer(Mode);
    Main_RenderBullets(Mode);
    Main_RenderEnemies(Mode);
    Main_RenderParticles(Mode);
    Main_RenderDeathBomb(Mode);
    
    
    // NOTE: PlayerDied -> Spawning state
    // NOTE: Change state if enemy and bullet count is 0
    if (Mode->DeathBomb.Radius >= Game_DesignWidth * 2.f) 
    {
        Mode->State = Main_StateType_Spawning;
        Mode->SpawnTimer = 0.f;
        Mode->Player.IsDead = false;
    }
}

static inline void
Main_StateSpawning_Update(permanent_state* PermState, 
                          transient_state* TranState,
                          debug_state* DebugState,
                          f32 DeltaTime) 
{
    game_mode_main* Mode = PermState->MainMode;
    assets* Assets = &TranState->Assets;
    
    f32 Ease = ease_out_bounce(Clamp(Mode->SpawnTimer/Mode->SpawnDuration, 0.f, 1.f));
    Mode->Player.Size = Mode->Player.MaxSize * Ease;
    
    Main_UpdateInput(Mode);
    Main_UpdatePlayer(Mode, DeltaTime);    
    
    //Main_RenderScore(Mode, &TranState->Arena, Assets);
    Main_RenderPlayer(Mode);
    
    
    // NOTE(Momo): Spawning -> Normal state
    if (Mode->SpawnTimer >= Mode->SpawnDuration) {
        Mode->State = Main_StateType_Normal;
        Mode->Player.Size = Mode->Player.MaxSize;
    }
    Mode->SpawnTimer += DeltaTime;
    
    
}

static inline void
MainMode_Update(permanent_state* PermState, 
                transient_state* TranState,
                debug_state* DebugState,
                f32 DeltaTime) 
{
    game_mode_main* Mode = PermState->MainMode;
    
    switch(Mode->State) {
        case Main_StateType_Spawning: {
            Main_StateSpawning_Update(PermState, TranState, DebugState, DeltaTime);
        } break;
        case Main_StateType_Normal: {
            Main_StateNormal_Update(PermState, TranState, DebugState, DeltaTime);
        }break;
        case Main_StateType_PlayerDied: {
            Main_StatePlayerDied_Update(PermState, TranState, DebugState, DeltaTime);
        } break;
        case Main_StateType_Cleanup: {
        } break;
        
    }
    
    //Main_RenderDebugLines(Mode, RenderCommands);
    
    u8_cstr Buffer = {};
    U8CStr_InitFromSiStr(&Buffer, "Dots: ");
    DebugInspector_PushU32(&DebugState->Inspector,
                           Buffer,
                           Mode->DotBullets.count);
    U8CStr_InitFromSiStr(&Buffer, "Circles: ");
    DebugInspector_PushU32(&DebugState->Inspector, 
                           Buffer, 
                           Mode->CircleBullets.count);
    
    U8CStr_InitFromSiStr(&Buffer, "Bullets: ");
    DebugInspector_PushU32(&DebugState->Inspector, 
                           Buffer, 
                           Mode->DotBullets.count + Mode->CircleBullets.count);
    
    
    U8CStr_InitFromSiStr(&Buffer, "Enemies: ");
    DebugInspector_PushU32(&DebugState->Inspector, 
                           Buffer, 
                           Mode->Enemies.count);
}

#endif //GAME_MODE_H
