#ifndef GAME_H
#define GAME_H

#include "game_platform.h"
#include "game_debug_console.h"
#include "game_renderer.h"
#include "game_debug.h"
#include "game_assets.h"
#include "mm_list.h"


enum game_mode_type : u32 {
    GameModeType_Splash,
    GameModeType_Main,
    GameModeType_Menu,
    GameModeType_Sandbox,

    GameModeType_None
};

// Stuff in this state can be zero initialized with no problems
// I.e. This state contains stuff that does not need to exist past a frame
// TODO: Put game_arena in here
struct transient_state {
    b32 IsInitialized;
    
    arena Arena;
    game_assets* Assets; 
};

struct permanent_state {
    b32 IsInitialized;
    b32 IsRunning;
    b32 IsShowInfo;

    game_mode_type CurrentGameMode;
    game_mode_type NextGameMode;
    union {
        struct game_mode_splash* SplashMode;
        struct game_mode_main* MainMode;
        struct game_mode_sandbox* SandboxMode;
    };
    
    arena MainArena;
    arena ModeArena;

};




#endif //GAME_H
