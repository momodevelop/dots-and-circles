/* date = April 12th 2021 7:00 pm */

#ifndef GAME_MODE_H
#define GAME_MODE_H

enum game_mode_type {
    GameModeType_Splash,
    GameModeType_Main,
    GameModeType_Menu,
    GameModeType_Sandbox,
    GameModeType_AnimeTest,
    GameModeType_None
};

// Stuff in this state can be zero initialized with no problems
// I.e. This state contains stuff that does not need to exist past a frame
struct transient_state {
    b8 IsInitialized;
    
    Arena arena;
    Assets assets; 
    Game_Audio_Mixer Mixer;
};

struct permanent_state {
    b8 IsInitialized;
    b8 IsPaused;
    f32 GameSpeed;
    Game_Camera Camera;
    
    game_mode_type CurrentGameMode;
    game_mode_type NextGameMode;
    union {
        struct game_mode_splash* SplashMode;
        struct game_mode_main* MainMode;
        struct game_mode_sandbox* SandboxMode;
        struct Game_Mode_Anime_Test* AnimeTestMode;
    };
    
    Arena arena;
    Arena_Mark ModeArena;
    
};

struct debug_state {
    b8 IsInitialized;
    Arena arena;
    
    debug_inspector Inspector;
    Debug_Console Console;
    
    struct permanent_state* PermanentState;
    struct transient_state* TransientState;
};

// NOTE(Momo): Things that require states' definitions

#endif //GAME_MODE_H
