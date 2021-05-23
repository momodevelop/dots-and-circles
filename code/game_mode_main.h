#ifndef GAME_MODE_MAIN_H
#define GAME_MODE_MAIN_H

#define CircleCap 128
#define DotCap 128
#define EnemyCap 128
#define ParticleCap 256

#define ZLayPlayer 0.f
#define ZLayBullet 10.f
#define ZLayEnemy 20.f
#define ZLayParticles 25.f

#define ZLayDebug 40.f

struct particle {
    constexpr static f32 Duration = 0.25f;
    constexpr static f32 Alpha = 0.8f;
    constexpr static f32 Size = 10.f;
    constexpr static f32 SpeedMin = 10.f;
    constexpr static f32 SpeedMax = 20.f;
    
    f32 Timer;
    v2f Position;
    v2f Direction;
    f32 Speed;
};


enum mood_type {
    MoodType_Dot,
    MoodType_Circle,
    
    MoodType_Count,
};

enum enemy_state {
    EnemyState_Spawning,
    EnemyState_Active,
    EnemyState_Dying
        
};
enum enemy_mood_pattern_type {
    EnemyMoodPatternType_Dot,
    EnemyMoodPatternType_Circle,
    EnemyMoodPatternType_Both,
    
    EnemyMoodPatternType_Count,
};

enum enemy_firing_pattern_type  {
    EnemyFiringPatternType_Homing,
    
    EnemyFiringPatternType_Count,
};

enum enemy_movement_type {
    EnemyMovementType_Static,
    
    EnemyMovementType_Count,
};


// Wave
enum wave_pattern_type {
    WavePatternType_SpawnNForDuration,   // Spawns N enemies at a time
};

struct wave_pattern_spawn_n_for_duration {
    u32 EnemiesPerSpawn;
    f32 SpawnTimer;
    f32 SpawnDuration;
    f32 Timer;
    f32 Duration;
};

struct wave {
    wave_pattern_type Type;
    union {
        wave_pattern_spawn_n_for_duration PatternSpawnNForDuration;    
    };
    b8 IsDone;
};

struct player {
    // NOTE(Momo): Rendering
    f32 DotImageAlpha;
    f32 DotImageAlphaTarget;
    f32 DotImageTransitionTimer;
    f32 DotImageTransitionDuration;
    
    v2f Size;
    
	// Collision
    circle2f HitCircle;
    
    // Physics
    v2f Position;
    v2f PrevPosition;
    
    // Gameplay
    mood_type MoodType;
};

struct bullet {
    v2f Size;
    mood_type MoodType;
    v2f Direction;
	v2f Position;
	f32 Speed;
	circle2f HitCircle; 
};

struct enemy {
    constexpr static f32 Size = 32.f; 
    constexpr static f32 SpawnStateRotationSpeed = 100.f;
    constexpr static f32 ActiveStateRotationSpeed = 2.f;
    constexpr static f32 DieStateRotationSpeed = 1.f;
    constexpr static f32 FireDuration = 0.1f;
    constexpr static f32 LifeDuration = 10.f;
    constexpr static f32 SpawnDuration = 0.5f;
    constexpr static f32 DieDuration = 0.5f;
    
    f32 Rotation;
    f32 RotationSpeed;
    
	v2f Position;
    
    enemy_firing_pattern_type FiringPatternType;
    enemy_mood_pattern_type MoodPatternType;
    enemy_movement_type MovementType;
    enemy_state State;
    
    f32 FireTimer;
    f32 LifeTimer;
    f32 SpawnTimer;
    f32 DieTimer;
    
};


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


#include "game_mode_main_input.h"
#include "game_mode_main_player.h"
#include "game_mode_main_bullet.h"
#include "game_mode_main_enemy.h"
#include "game_mode_main_wave.h"
#include "game_mode_main_particle.h"
#include "game_mode_main_collision.h"
#include "game_mode_main_debug.h"

static inline b8 
InitMainMode(permanent_state* PermState,
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
UpdateMainMode(permanent_state* PermState, 
               transient_state* TranState,
               debug_state* DebugState,
               mailbox* RenderCommands, 
               platform_input* Input,
               f32 DeltaTime) 
{// NOTE(Momo): jj
    game_mode_main* Mode = PermState->MainMode;
    Camera_Set(&Mode->Camera, RenderCommands);
    
    assets* Assets = &TranState->Assets;
    UpdateInput(Mode, Input);
    UpdatePlayer(Mode, DeltaTime);    
    UpdateBullets(Mode, DeltaTime);
    UpdateWaves(Mode, Assets, DeltaTime);
    UpdateEnemies(Mode, Assets, DeltaTime); 
    UpdateCollision(Mode, Assets, DeltaTime);
    UpdateParticles(Mode, DeltaTime);
    
    RenderPlayer(Mode, Assets, RenderCommands);
    RenderBullets(Mode, Assets, RenderCommands);
    RenderEnemies(Mode, Assets, RenderCommands);
    RenderParticles(Mode, Assets, RenderCommands);
    //RenderDebugLines(Mode, RenderCommands);
    
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
