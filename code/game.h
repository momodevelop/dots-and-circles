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

#if INTERNAL
static inline b8
ProcessMetaInput(game_state* GameState, game_input* Input) {
    if (IsDown(Input->ButtonDebug[1])) {
        SetGameMode<game_mode_splash>(GameState);
        Log("Jumping to splash state");
        return true;
    }
    else if (IsDown(Input->ButtonDebug[2])) {
        SetGameMode<game_mode_main>(GameState);
        Log("Jumping to main state");
        
        return true;
    }
    
    else if (IsDown(Input->ButtonDebug[0])) {
        SetGameMode<game_mode_sandbox>(GameState);
        Log("Jumping to sandbox state");
        return true;
    }
    
    return false;
}
#endif

#endif //GAME_H
