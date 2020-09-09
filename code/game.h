#ifndef GAME_H
#define GAME_H

#include "game_platform.h"
#include "game_input.h"
#include "game_renderer.h"
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
        struct game_mode_menu* MenuMode;
        struct game_mode_atlas_test* AtlasTestMode;
    };
    
    u32 CurrentMode;
    
    void (*UpdateMode)(void* Mode, game_state* State, commands* RenderCommands, game_input* Input, f32 DeltaTime);
    
    game_assets* Assets;
    
    memory_arena MainArena;
    memory_arena ModeArena;
    
    b32 IsInitialized;
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
static inline b32
ProcessMetaInput(game_state* GameState, game_input* Input) {
    if (IsPoked(Input->ButtonDebug[1])) {
        SetGameMode<game_mode_splash>(GameState);
        Log("Jumping to splash state");
        return true;
    }
    else if (IsPoked(Input->ButtonDebug[2])) {
        SetGameMode<game_mode_menu>(GameState);
        Log("Jumping to menu state");
        return true;
    }
    else if (IsPoked(Input->ButtonDebug[9])) {
        SetGameMode<game_mode_atlas_test>(GameState);
        Log("Jumping to atlas test state");
        return true;
    }
    
    else if (IsPoked(Input->ButtonDebug[0])) {
        SetGameMode<game_mode_main>(GameState);
        Log("Jumping to main state");
        return true;
    }
    
    return false;
}
#endif

#endif //GAME_H
