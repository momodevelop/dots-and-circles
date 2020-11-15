#ifndef GAME_H
#define GAME_H

#include "platform.h"
#ifdef INTERNAL
static platform_log* gLog;
#define Log(fmt, ...) gLog(fmt, __VA_ARGS__);
#else 
#define Log(fmt, ...)
#endif

#include "game_input.h"


#if INTERNAL
#include "game_debug.h"
#endif


#include "game_renderer.h"
#include "game_assets.h"
#include "mm_string.h"
#include "mm_list.h"


// NOTE(Momo): How much do we care to make this generic and use CRTP + std::variant?
enum game_mode_type : u32 {
    GameModeType_Splash,
    GameModeType_Main,
    GameModeType_Menu,
    GameModeType_AtlasTest,

    GameModeType_None
};


struct game_state {
    game_mode_type ModeType;
    game_mode_type NextModeType;
    union {
        struct game_mode_splash* SplashMode;
        struct game_mode_main* MainMode;
        struct game_mode_menu* MenuMode;
        struct game_mode_atlas_test* AtlasTestMode;
    };
    
    game_assets Assets;
    
    mmarn_arena MainArena;
    mmarn_arena ModeArena;
    
    b32 IsInitialized;

#if INTERNAL
    b32 IsDebug;
    b32 IsShowTicksElapsed;

    mmarn_arena DebugArena;
    debug_console DebugConsole;
#endif
};

#endif //GAME_H
