#ifndef GAME_H
#define GAME_H

#include "game_platform.h"
#include "game_console.h"
#include "game_renderer.h"
#include "game_assets.h"
#include "mm_list.h"


// NOTE(Momo): How much do we care to make this generic and use CRTP + std::variant?
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
   
    arena TransArena;
};

struct permanent_state {
    b32 IsInitialized;

    game_mode_type CurrentGameMode;
    game_mode_type NextGameMode;
    union {
        struct game_mode_splash* SplashMode;
        struct game_mode_main* MainMode;
        struct game_mode_menu* MenuMode;
        struct game_mode_sandbox* SandboxMode;
    };
    
    game_assets Assets;
    
    arena MainArena;
    arena ModeArena;

    b32 IsShowInfo;
    game_console Console;


};

#endif //GAME_H
