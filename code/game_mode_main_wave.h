/* date = May 22nd 2021 7:12 pm */

#ifndef GAME_MODE_MAIN_WAVE_H
#define GAME_MODE_MAIN_WAVE_H
// Wave
enum Wave_Pattern_Type {
    WAVE_PATTERN_TYPE_SPAWN_N_FOR_DURATION,   // Spawns N enemies at a time
};

struct Wave_Pattern_Spawn_N_For_Duration {
    u32 enemies_per_spawn;
    f32 spawn_timer;
    f32 spawn_duration;
    f32 timer;
    f32 duration;
};

struct Wave {
    Wave_Pattern_Type type;
    union {
        Wave_Pattern_Spawn_N_For_Duration pattern_spawn_n_for_duration;    
    };
    b8 is_done;
};

#endif //GAME_MODE_MAIN_WAVE_H
