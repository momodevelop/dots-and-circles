/* date = April 12th 2021 7:00 pm */

#ifndef GAME_MODE_H
#define GAME_MODE_H

enum Game_Mode_Type {
    GAME_MODE_SPLASH,
    GAME_MODE_MAIN,
    GAME_MODE_SANDBOX,
    GAME_MODE_ANIME_TEST,
    GAME_MODE_NONE
};

// Stuff in this state can be zero initialized with no problems
// I.e. This state contains stuff that does not need to exist past a frame
struct Transient_State {
    b8 is_initialized;
    
    Arena arena;
    Assets assets; 
    Game_Audio_Mixer mixer;
};

struct Permanent_State {
    b8 is_initialized;
    b8 is_paused;
    f32 game_speed;
    Game_Camera camera;
    
    Game_Mode_Type current_game_mode;
    Game_Mode_Type next_game_mode;
    union {
        struct Game_Mode_Splash* splash_mode;
        struct Game_Mode_Main* main_mode;
        struct Game_Mode_Sandbox* sandbox_mode;
        struct Game_Mode_Anime_Test* anime_test_mode;
    };
    
    Arena arena;
    Arena_Marker mode_arena;
    
};

struct Debug_State {
    b8 is_initialized;
    Arena arena;
    
    Debug_Inspector inspector;
    Debug_Console console;
    
    struct Permanent_State* perm_state;
    struct Transient_State* tran_state;
};

// NOTE(Momo): Things that require states' definitions

#endif //GAME_MODE_H
