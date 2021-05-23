/* date = May 22nd 2021 7:12 pm */

#ifndef GAME_MODE_MAIN_WAVE_H
#define GAME_MODE_MAIN_WAVE_H
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

#endif //GAME_MODE_MAIN_WAVE_H
