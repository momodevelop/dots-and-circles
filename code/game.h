#ifndef GAME_H
#define GAME_H

#include "platform.h"


#if INTERNAL
#include "game_console.h"
#endif


#include "renderer.h"
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


struct game_state {
    game_mode_type ModeType;
    game_mode_type NextModeType;
    union {
        struct game_mode_splash* SplashMode;
        struct game_mode_main* MainMode;
        struct game_mode_menu* MenuMode;
        struct game_mode_sandbox* SandboxMode;
    };
    
    game_assets Assets;
    
    arena MainArena;
    arena ModeArena;

    b32 IsInitialized;

    b32 IsShowInfo;
    game_console Console;

};

#endif //GAME_H
