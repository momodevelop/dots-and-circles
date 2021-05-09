#ifndef GAME_MODE_MAIN_H
#define GAME_MODE_MAIN_H

#define CircleCap 128
#define DotCap 128
#define EnemyCap 128

#define ZLayPlayer 0.f
#define ZLayDotBullet 10.f
#define ZLayCircleBullet 20.f
#define ZLayEnemy 30.f
#define ZLayDebug 40.f


#define Particle_Duration 3.0f
struct particle {
    f32 Timer;
    image_id ImageId;
    v2f Position;
    v2f Direction;
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
	v2f Size; 
	v2f Position;
    
    enemy_firing_pattern_type FiringPatternType;
    enemy_mood_pattern_type MoodPatternType;
    enemy_movement_type MovementType;
    
    f32 FireTimer;
	f32 FireDuration;
    
    f32 LifeTimer;
    f32 LifeDuration;
};


struct game_mode_main {
    arena_mark ArenaMark;
    
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


static inline void
SpawnEnemy(game_mode_main* Mode, 
           assets* Assets, 
           v2f Position,
           enemy_mood_pattern_type MoodPatternType,
           enemy_firing_pattern_type FiringPatternType, 
           enemy_movement_type MovementType, 
           f32 FireRate,
           f32 LifeDuration) 
{
    enemy Enemy = {}; 
    Enemy.Position = Position;
    Enemy.Size = V2f_Create(32.f, 32.f);
    
    Enemy.FireTimer = 0.f;
    Enemy.FireDuration = FireRate; 
    Enemy.LifeDuration = LifeDuration;
    
    Enemy.FiringPatternType = FiringPatternType;
    Enemy.MoodPatternType = MoodPatternType;
    Enemy.MovementType = MovementType;
    
    List_PushItem(&Mode->Enemies, Enemy);
}


static inline void
SpawnBullet(game_mode_main* Mode, assets* Assets, v2f Position, v2f Direction, f32 Speed, mood_type Mood) {
    bullet Bullet = {}; 
    Bullet.Position = Position;
	Bullet.Speed = Speed;
    Bullet.Size = V2f_Create(16.f, 16.f);
    
    Bullet.HitCircle = {
        V2f_Create(0.f, 0.f), 
        Bullet.Size.X * 0.5f 
    };
    
    if (V2f_LengthSq(Direction) > 0.f) {
	    Bullet.Direction = V2f_Normalize(Direction);
    }
    switch (Mood) {
        case MoodType_Dot: {
            List_PushItem(&Mode->DotBullets, Bullet);
        } break;
        case MoodType_Circle: {
            List_PushItem(&Mode->CircleBullets, Bullet);
        } break;
        default: {
            Assert(False);
        }
    }
    
}

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
        Mode->Camera.Position = V3f_Create(0.f, 0.f, 0.f);
        Mode->Camera.Anchor = V3f_Create(0.5f, 0.5f, 0.5f);
        Mode->Camera.Color = Color_Grey2;
        Mode->Camera.Dimensions = V3f_Create(Game_DesignWidth,
                                             Game_DesignHeight,
                                             Game_DesignDepth);
    }
    
    b32 Success = False;
    Mode->ArenaMark = Arena_Mark(&PermState->ModeArena);
    
    Success = List_InitFromArena(&Mode->DotBullets, Mode->ArenaMark, DotCap);
    if (!Success) {
        Arena_Revert(&Mode->ArenaMark);
        return False;
    }
    
    Success = List_InitFromArena(&Mode->CircleBullets, Mode->ArenaMark, CircleCap);
    if (!Success) {
        Arena_Revert(&Mode->ArenaMark);
        return False;
    }
    
    Success = List_InitFromArena(&Mode->Enemies, Mode->ArenaMark, EnemyCap);
    if (!Success) {
        Arena_Revert(&Mode->ArenaMark);
        return False;
    }
    
    Mode->Wave.IsDone = True;
    Mode->Rng = Seed(0); // TODO: Used system clock for seed.
    
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
    Mode->Wave.IsDone = True;
    
    Success = Queue_InitFromArena(&Mode->Particles, Mode->ArenaMark, 128);
    if (!Success) {
        Arena_Revert(&Mode->ArenaMark);
        return False;
    }
    
    Success = AudioMixer_Play(&TranState->Mixer, Sound_Test, False, &Mode->BgmHandle);
    if (!Success) {
        Arena_Revert(&Mode->ArenaMark);
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
    UpdateCollision(Mode, DeltaTime);
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
    
}

#endif //GAME_MODE_H
