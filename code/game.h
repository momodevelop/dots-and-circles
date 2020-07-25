#ifndef GAME_H
#define GAME_H

#include "ryoji_renderer.h"


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


struct game_state_splash {
    static constexpr u8 TypeId = 0;
    splash_image_entity SplashImg[2];
    splash_blackout_entity SplashBlackout;
};

struct game_state_main {
    static constexpr u8 TypeId = 1;
};

struct game_state_sandbox {
    static constexpr u8 TypeId = 2;
    static constexpr u32 TotalEntities = 25000;
    test_entity Entities[TotalEntities];
};


union game_state {
    game_state_splash Splash;
    game_state_main Main;
    game_state_sandbox Sandbox;
};

struct game {
    game_state GameState;
    u32 CurrentStateType;
    bool IsStateInitialized;
    
    game_assets Assets;
    memory_arena MainArena;
    
};



#endif //GAME_H
