/* date = May 22nd 2021 7:08 pm */

#ifndef GAME_MODE_MAIN_ENEMY_H
#define GAME_MODE_MAIN_ENEMY_H

#define EnemyCap 128

//~ NOTE(Momo): Enemy States
enum enemy_state {
    EnemyState_Spawning,
    EnemyState_Active,
    EnemyState_Dying
        
};
struct enemy_state_spawn {
    f32 Timer;
    constexpr static f32 Duration = 0.5f;
};

struct enemy_state_dying {
    f32 Timer;
    constexpr static f32 Duration = 0.5f;
};

struct enemy_state_active {
    f32 Timer;
    constexpr static f32 Duration = 10.f;
};

//~ NOTE(Momo): Enemy movement patterns
enum enemy_movement_type {
    EnemyMovementType_Static,
    
    EnemyMovementType_Count,
};

struct enemy_movement_none{};

//~ NOTE(Momo): Enemy shoot patterns

enum enemy_shoot_type  {
    EnemyShootType_Homing,
    EnemyShootType_8Directions,
    
    EnemyFiringPatternType_Count,
};

struct enemy_shoot_homing {
    f32 Timer;
    f32 Duration;
    mood_type Mood;
};

struct enemy_shoot_8dir {
    f32 Timer;
    f32 Duration;
    mood_type Mood;
};

//~ 
struct enemy {
    constexpr static f32 Size = 32.f; 
    constexpr static f32 SpawnStateRotationSpeed = 100.f;
    constexpr static f32 ActiveStateRotationSpeed = 2.f;
    constexpr static f32 DieStateRotationSpeed = 1.f;
    
    f32 Rotation;
    f32 RotationSpeed;
    
	v2f Position;
    
    enemy_state State;
    union {
        enemy_state_spawn StateSpawn;
        enemy_state_active StateActive;
        enemy_state_dying StateDying;
    };
    
    enemy_movement_type MovementType;
    union {
        enemy_movement_none MovementNone;
    };
    enemy_shoot_type ShootType;
    union {
        enemy_shoot_homing ShootHoming;
        enemy_shoot_8dir Shoot8Dir;
    };
};

#endif 