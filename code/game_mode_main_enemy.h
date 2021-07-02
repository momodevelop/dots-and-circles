/* date = May 22nd 2021 7:08 pm */

#ifndef GAME_MODE_MAIN_ENEMY_H
#define GAME_MODE_MAIN_ENEMY_H

#define EnemyCap 128

//~ NOTE(Momo): Enemy States
enum Enemy_State {
    EnemyState_Spawning,
    EnemyState_Active,
    EnemyState_Dying
        
};
struct Enemy_State_Spawn {
    f32 timer;
    constexpr static f32 duration = 0.5f;
};

struct Enemy_State_Dying {
    f32 timer;
    constexpr static f32 duration = 0.5f;
};

struct Enemy_State_Active {
    f32 timer;
    constexpr static f32 duration = 10.f;
};

//~ NOTE(Momo): Enemy movement patterns
enum Enemy_Movement_Type {
    ENEMY_MOVEMENT_TYPE_STATIC,
    
    ENEMY_MOVEMENT_TYPE_COUNT,
};

struct Enemy_Movement_None{};

//~ NOTE(Momo): Enemy shoot patterns

enum Enemy_Shoot_Type  {
    ENEMY_SHOOT_TYPE_HOMING,
    ENEMY_SHOOT_TYPE_8_DIR,
    
    ENEMY_SHOOT_TYPE_COUNT,
};

struct Enemy_Shoot_Homing {
    f32 timer;
    f32 duration;
    Mood_Type mood;
};

struct Enemy_Shoot_8_Dir {
    f32 timer;
    f32 duration;
    Mood_Type mood;
};

//~ 
struct Enemy {
    constexpr static f32 size = 32.f; 
    constexpr static f32 spawn_state_rotation_speed = 100.f;
    constexpr static f32 active_state_rotation_speed = 2.f;
    constexpr static f32 die_state_rotation_speed = 1.f;
    
    f32 rotation;
    f32 rotation_speed;
    
	v2f position;
    
    Enemy_State state;
    union {
        Enemy_State_Spawn state_spawn;
        Enemy_State_Active state_active;
        Enemy_State_Dying state_dying;
    };
    
    Enemy_Movement_Type movement_type;
    union {
        Enemy_Movement_None movement_none;
    };
    Enemy_Shoot_Type shoot_type;
    union {
        Enemy_Shoot_Homing shoot_homing;
        Enemy_Shoot_8_Dir shoot_8_dir;
    };
};

#endif 