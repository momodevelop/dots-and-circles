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


struct game_mode_splash {
    static constexpr u8 TypeId = 0;
    splash_image_entity SplashImg[2];
    splash_blackout_entity SplashBlackout;
};

struct game_mode_main {
    static constexpr u8 TypeId = 1;
    splash_image_entity SplashImg[2];
    splash_blackout_entity SplashBlackout;
    
};

struct game_mode_sandbox {
    static constexpr u8 TypeId = 2;
    static constexpr u32 TotalEntities = 2500;
    test_entity Entities[TotalEntities];
};


union game_mode {
    game_mode_splash Splash;
    game_mode_main Main;
    game_mode_sandbox Sandbox;
};

struct game_state {
    game_mode GameMode;
    u32 CurrentMode;
    
    game_assets Assets;
    
    memory_arena MainArena;
    memory_arena ModeArena;
    
    
    b8 IsInitialized;
    b8 IsStateInitialized;
    
    
};



#endif //GAME_H
