#ifndef GAME_MODE_MAIN_H
#define GAME_MODE_MAIN_H

#define CircleCap 128
#define DotCap 128
#define EnemyCap 128

#define ZLayPlayer 0.f
#define ZLayDotBullet 10.f
#define ZLayCircleBullet 15.f
#define ZLayEnemy 20.f
#define ZLayParticles 25.f

#define ZLayDebug 40.f


#define Particle_Duration 3.0f
struct particle {
    f32 Timer;
    image_id ImageId;
    MM_V2f Position;
    MM_V2f Direction;
    f32 Speed;
};


enum mood_type {
    MoodType_Dot,
    MoodType_Circle,
    
    MoodType_Count,
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
    b32 IsDone;
};

struct player {
    // NOTE(Momo): Rendering
    f32 DotImageAlpha;
    f32 DotImageAlphaTarget;
    f32 DotImageTransitionTimer;
    f32 DotImageTransitionDuration;
    
    MM_V2f Size;
    
	// Collision
    MM_Circle2f HitCircle;
    
    // Physics
    MM_V2f Position;
    MM_V2f PrevPosition;
    
    // Gameplay
    mood_type MoodType;
};

struct bullet {
    MM_V2f Size;
    mood_type MoodType;
    MM_V2f Direction;
	MM_V2f Position;
	f32 Speed;
	MM_Circle2f HitCircle; 
};

struct enemy {
	MM_V2f Size; 
	MM_V2f Position;
    
    enemy_firing_pattern_type FiringPatternType;
    enemy_mood_pattern_type MoodPatternType;
    enemy_movement_type MovementType;
    
    f32 FireTimer;
	f32 FireDuration;
    
    f32 LifeTimer;
    f32 LifeDuration;
};


struct game_mode_main {
    MM_ArenaMark ArenaMark;
    
    player Player;
    game_camera Camera;
    
    
    MM_List<bullet> CircleBullets;
    MM_List<bullet> DotBullets;
    MM_List<enemy> Enemies;
    MM_Queue<particle> Particles;
    
    wave Wave;
    MM_Rng Rng;
    
    // Audio handles
    game_audio_mixer_handle BgmHandle;
};


#include "game_mode_main_spawn.h"
#include "game_mode_main_update.h"
#include "game_mode_main_render.h"

static inline b32 
InitMainMode(permanent_state* PermState,
             transient_state* TranState,
             debug_state* DebugState) 
{
    game_mode_main* Mode = PermState->MainMode;
    
    // NOTE(Momo): Init camera
    {
        Mode->Camera.Position = MM_V3f_Create(0.f, 0.f, 0.f);
        Mode->Camera.Anchor = MM_V3f_Create(0.5f, 0.5f, 0.5f);
        Mode->Camera.Color = Color_Grey2;
        Mode->Camera.Dimensions = MM_V3f_Create(Game_DesignWidth,
                                             Game_DesignHeight,
                                             Game_DesignDepth);
    }
    
    b32 Success = False;
    Mode->ArenaMark = MM_Arena_Mark(&PermState->ModeArena);
    
    Success = MM_List_InitFromArena(&Mode->DotBullets, Mode->ArenaMark, DotCap);
    if (!Success) {
        MM_Arena_Revert(&Mode->ArenaMark);
        return False;
    }
    
    Success = MM_List_InitFromArena(&Mode->CircleBullets, Mode->ArenaMark, CircleCap);
    if (!Success) {
        MM_Arena_Revert(&Mode->ArenaMark);
        return False;
    }
    
    Success = MM_List_InitFromArena(&Mode->Enemies, Mode->ArenaMark, EnemyCap);
    if (!Success) {
        MM_Arena_Revert(&Mode->ArenaMark);
        return False;
    }
    
    Mode->Wave.IsDone = True;
    Mode->Rng = MM_Rng_Seed(0); // TODO: Used system clock for seed.
    
    assets* Assets = &TranState->Assets;
    player* Player = &Mode->Player;
    {
        Player->Position = {};
        Player->PrevPosition = {};
        Player->Size = MM_V2f_Create( 32.f, 32.f );
        Player->HitCircle = { MM_V2f{}, 16.f};
        
        // NOTE(Momo): We start as Dot
        Player->MoodType = MoodType_Dot;
        Player->DotImageAlpha = 1.f;
        Player->DotImageAlphaTarget = 1.f;
        
        Player->DotImageTransitionDuration = 0.1f;
        Player->DotImageTransitionTimer = Player->DotImageTransitionDuration;
    }
    Mode->Wave.IsDone = True;
    
    Success = MM_Queue_InitFromArena(&Mode->Particles, Mode->ArenaMark, 128);
    if (!Success) {
        MM_Arena_Revert(&Mode->ArenaMark);
        return False;
    }
    
    Success = AudioMixer_Play(&TranState->Mixer, Sound_Test, False, &Mode->BgmHandle);
    if (!Success) {
        MM_Arena_Revert(&Mode->ArenaMark);
        return False;
    }
    
    return True; 
    
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
