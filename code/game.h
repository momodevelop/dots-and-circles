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


typedef void (*debug_callback)(void* Context);
struct debug_command {
    mms_const_string Key;
    debug_callback Callback;
    void* Context;
};

struct debug_string {
    mms_string Buffer;
    mmm_v4f Color;
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
    
    game_assets* Assets;
    
    mmarn_arena MainArena;
    mmarn_arena ModeArena;
    
    b32 IsInitialized;

#if INTERNAL
    b32 IsDebug;
    b32 IsShowTicksElapsed;

    debug_string DebugInfoBuffers[5];
    mms_string DebugInputBuffer;

    mmarn_arena DebugArena;

    // TODO: Implement a Hashmap??
    mml_list<debug_command> DebugCallbacks;
#endif
};

static inline void
PushDebugInfo(game_state* GameState, mms_const_string String, mmm_v4f Color) {
    for(i32 i = ArrayCount(GameState->DebugInfoBuffers) - 2; i >= 0 ; --i) {
        debug_string* Dest = GameState->DebugInfoBuffers + i + 1;
        debug_string* Src = GameState->DebugInfoBuffers + i;
        mms_Copy(&Dest->Buffer, &Src->Buffer);
        Dest->Color = Src->Color;

    }
    GameState->DebugInfoBuffers[0].Color = Color;
    mms_Clear(&GameState->DebugInfoBuffers[0].Buffer);
    mms_Copy(&GameState->DebugInfoBuffers[0].Buffer, String);
}

#endif //GAME_H
