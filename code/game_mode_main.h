#ifndef GAME_MODE_MAIN_H
#define GAME_MODE_MAIN_H

#define CircleCap 1024
#define DotCap 1024
#define ParticleCap 256

//#define ZLayScore 9.f
#define ZLayPlayer 10.f
#define ZLayBullet 11.f
#define ZLayEnemy 12.f
#define ZLayParticles 13.f
#define ZLayDeathBomb 14.f
#define ZLayScore 15.f
#define ZLayDebug 16.f

enum Mood_Type {
    MOOD_TYPE_DOT,
    MOOD_TYPE_CIRCLE,
    
    MOOD_TYPE_COUNT,
};



struct Player {
    // NOTE(Momo): Rendering
    f32 dot_image_alpha;
    f32 dot_image_alpha_target;
    f32 dot_image_transition_timer;
    f32 dot_image_transition_duration;
    
    static constexpr f32 max_size = 32.f;
    f32 size;
    
	// Collision
    circle2f hit_circle;
    
    // Physics
    v2f position;
    v2f prev_position;
    
    // Gameplay
    Mood_Type mood_type;
    
    b8 is_dead;
};

struct Bullet {
    v2f Size;
    Mood_Type mood_type;
    v2f direction;
	v2f position;
	f32 speed;
	circle2f hit_circle; 
};

struct Particle {
    constexpr static f32 duration = 0.25f;
    constexpr static f32 alpha = 0.8f;
    constexpr static f32 size = 10.f;
    constexpr static f32 min_speed = 10.f;
    constexpr static f32 max_speed = 20.f;
    
    f32 timer;
    v2f position;
    v2f direction;
    f32 speed;
};


#include "game_mode_main_enemy.h"
#include "game_mode_main_wave.h"


enum game_mode_main_state_type {
    Main_StateType_Spawning,
    Main_StateType_Normal,
    Main_StateType_PlayerDied,
    Main_StateType_Cleanup,
};


struct Death_Bomb {
    static constexpr f32 growth_speed = 750.f;
    f32 radius;
    v2f position;
};


struct game_mode_main {
    game_mode_main_state_type State;
    
    // TODO: we might do like a tagged union state thing but
    // I don't really know the problem space for that yet
    // because not all states have variables atm
    f32 spawn_timer;
    static constexpr f32 spawn_duration = 1.f;
    
    Player player;
    Game_Camera camera;
    
    List<Bullet> circle_bullets;
    List<Bullet> dot_bullets;
    List<Enemy> enemies;
    Queue<Particle> particles;
    Death_Bomb death_bomb;
    
    Wave wave;
    Rng_Series rng;
    Big_Int score;
    
    // Audio handles
    Game_Audio_Mixer_Handle BgmHandle;
    
    
};

#endif //GAME_MODE_H
