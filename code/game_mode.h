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
    b32 IsInitialized;
    
    arena Arena;
    assets Assets; 
};

struct permanent_state {
    b32 IsInitialized;
    b32 IsRunning;
    b32 IsPaused;
    
    game_mode_type CurrentGameMode;
    game_mode_type NextGameMode;
    union {
        struct game_mode_splash* SplashMode;
        struct game_mode_main* MainMode;
        struct game_mode_sandbox* SandboxMode;
        struct game_mode_anime_test* AnimeTestMode;
    };
    
    arena MainArena;
    arena ModeArena;
    
};

struct debug_state {
    b32 IsInitialized;
    arena Arena;
    
    debug_inspector Inspector;
    debug_console Console;
    
    struct permanent_state* PermanentState;
    struct transient_state* TransientState;
};

// NOTE(Momo): Things that require states' definitions

#endif //GAME_MODE_H
