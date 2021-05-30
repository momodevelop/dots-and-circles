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
        Mode->Camera.Color = C4f_Grey2;
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
    
    
    Success = BigInt_New(&Mode->Score, ModeArena, 128);
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
    
    
    Mode->State = Main_StateType_Spawning;
    return true; 
    
}

static inline void
Main_StateNormal_Update(permanent_state* PermState, 
                        transient_state* TranState,
                        debug_state* DebugState,
                        mailbox* RenderCommands, 
                        platform_input* Input,
                        f32 DeltaTime) 
{
    assets* Assets = &TranState->Assets;
    game_mode_main* Mode = PermState->MainMode;
    
    Camera_Set(&Mode->Camera, RenderCommands);
    
    Main_UpdateInput(Mode, Input);
    Main_UpdatePlayer(Mode, DeltaTime);    
    Main_UpdateBullets(Mode, DeltaTime);
    Main_UpdateWaves(Mode, Assets, DeltaTime);
    Main_UpdateEnemies(Mode, Assets, DeltaTime); 
    Main_UpdatePlayerBulletCollision(Mode, Assets, DeltaTime);
    Main_UpdateParticles(Mode, DeltaTime);
    
    Main_RenderPlayer(Mode, Assets, RenderCommands);
    Main_RenderBullets(Mode, Assets, RenderCommands);
    Main_RenderEnemies(Mode, Assets, RenderCommands);
    Main_RenderParticles(Mode, Assets, RenderCommands);
    
    // NOTE(Momo): if player's dead, do dead stuff
    if(Mode->Player.IsDead) 
    {
        // NOTE(Momo): Drop the death bomb
        Mode->DeathBomb.Radius = 0.f;
        Mode->DeathBomb.Position = Mode->Player.Position;
        
        Mode->State = Main_StateType_PlayerDied;
        Mode->Player.Position = V2f_Create(-1000.f, -1000.f);
    }
}



static inline void
Main_StatePlayerDied_Update(permanent_state* PermState, 
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
    
    Camera_Set(&Mode->Camera, RenderCommands);
    Main_RenderPlayer(Mode, Assets, RenderCommands);
    Main_RenderBullets(Mode, Assets, RenderCommands);
    Main_RenderEnemies(Mode, Assets, RenderCommands);
    Main_RenderParticles(Mode, Assets, RenderCommands);
    Main_RenderDeathBomb(Mode, RenderCommands);
    
    // NOTE: Change state if enemy and bullet count is 0
    if (Mode->DeathBomb.Radius >= Game_DesignWidth) 
    {
        Mode->State = Main_StateType_Spawning;
        Mode->SpawningState.Timer = 0.f;
        Mode->Player.IsDead = false;
    }
}

static inline void
Main_StateSpawning_Update(permanent_state* PermState, 
                          transient_state* TranState,
                          debug_state* DebugState,
                          mailbox* RenderCommands, 
                          platform_input* Input,
                          f32 DeltaTime) 
{
    game_mode_main* Mode = PermState->MainMode;
    assets* Assets = &TranState->Assets;
    
    f32 Ease = EaseOutBounce(Clamp(Mode->SpawningState.Timer/Mode->SpawningState.Duration, 0.f, 1.f));
    Mode->Player.Size = Mode->Player.MaxSize * Ease;
    
    
    Main_UpdateInput(Mode, Input);
    Main_UpdatePlayer(Mode, DeltaTime);    
    
    Camera_Set(&Mode->Camera, RenderCommands);
    
    Main_RenderPlayer(Mode, Assets, RenderCommands);
    
    if (Mode->SpawningState.Timer >= Mode->SpawningState.Duration) {
        Mode->State = Main_StateType_Normal;
        Mode->Player.Size = Mode->Player.MaxSize;
    }
    Mode->SpawningState.Timer += DeltaTime;
    
    
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
    
    switch(Mode->State) {
        case Main_StateType_Spawning: {
            Main_StateSpawning_Update(PermState, TranState, DebugState, RenderCommands, Input, DeltaTime);
        } break;
        case Main_StateType_Normal: {
            Main_StateNormal_Update(PermState, TranState, DebugState, RenderCommands, Input, DeltaTime);
        }break;
        case Main_StateType_PlayerDied: {
            Main_StatePlayerDied_Update(PermState, TranState, DebugState, RenderCommands, Input, DeltaTime);
        } break;
        case Main_StateType_Cleanup: {
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
