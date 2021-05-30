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



struct player {
    // NOTE(Momo): Rendering
    f32 DotImageAlpha;
    f32 DotImageAlphaTarget;
    f32 DotImageTransitionTimer;
    f32 DotImageTransitionDuration;
    
    static constexpr f32 MaxSize = 32.f;
    f32 Size;
    
	// Collision
    circle2f HitCircle;
    
    // Physics
    v2f Position;
    v2f PrevPosition;
    
    // Gameplay
    mood_type MoodType;
    
    b8 IsDead;
};

struct bullet {
    v2f Size;
    mood_type MoodType;
    v2f Direction;
	v2f Position;
	f32 Speed;
	circle2f HitCircle; 
};

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


#include "game_mode_main_enemy.h"
#include "game_mode_main_wave.h"


enum game_mode_main_state_type {
    Main_StateType_Spawning,
    Main_StateType_Normal,
    Main_StateType_PlayerDied,
    Main_StateType_Cleanup,
};


struct death_bomb {
    static constexpr f32 GrowthSpeed = 750.f;
    f32 Radius;
    v2f Position;
};


struct game_mode_main_state_spawning {
    f32 Timer;
    static constexpr f32 Duration = 1.f;
};
struct game_mode_main_state_normal {
};
struct game_mode_main_state_player_died {
};

struct game_mode_main {
    // TODO: We could use pointers to save memory but ahhhhh...lazy
    game_mode_main_state_type State;
    union {
        game_mode_main_state_spawning SpawningState;
        game_mode_main_state_normal NormalState;
        game_mode_main_state_player_died PlayerDiedState;
    };
    player Player;
    
    game_camera Camera;
    
    list<bullet> CircleBullets;
    list<bullet> DotBullets;
    list<enemy> Enemies;
    queue<particle> Particles;
    death_bomb DeathBomb;
    
    wave Wave;
    rng_series Rng;
    big_int Score;
    
    // Audio handles
    game_audio_mixer_handle BgmHandle;
    
    
};

#endif //GAME_MODE_H
