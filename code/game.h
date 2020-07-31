#ifndef GAME_H
#define GAME_H



#include "game_renderer.h"
#include "game_platform.h"
#include "game_input.h"
#include "game_assets.h"
#include "game_entities.h"

#if  INTERNAL
static platform_log* gLog;
#define Log(fmt, ...) gLog(fmt, __VA_ARGS__);
#else 
#define Log(fmt, ...)
#endif

struct game_state {
    union {
        void* Mode;
        struct game_mode_splash* SplashMode;
        struct game_mode_main* MainMode;
        struct game_mode_sandbox* SandboxMode;
    };
    
    u32 CurrentMode;
    
    game_assets* Assets;
    
    memory_arena MainArena;
    memory_arena ModeArena;
    
    b8 IsInitialized;
};

template<typename T>
static inline void
SetGameMode(game_state* GameState) {
    memory_arena* ModeArena = &GameState->ModeArena;
    Clear(ModeArena);
    GameState->CurrentMode = T::TypeId;
    T* Mode = PushStruct<T>(ModeArena); 
    InitMode(Mode, GameState); // all modes must have init function
    GameState->Mode = Mode;
}

#endif //GAME_H
