#ifndef GAME_H
#define GAME_H


#include "yuu_platform.h"
#include "yuu_input.h"
#include "yuu_renderer.h"
#include "yuu_assets.h"

#include "game_entities.h"

static platform_api* gPlatform;

struct game_state_splash {
    static constexpr u32 TypeId = 0;
    splash_image_entity SplashImg[2];
    splash_blackout_entity SplashBlackout;
};

struct game_state_main {
    static constexpr u32 TypeId = 1;
};

union game_state {
    game_state_splash Splash;
    game_state_main Main;
};
struct game {
    game_state GameState;
    u32 CurrentStateType;
    bool IsStateInitialized;
    
    game_assets Assets;
    
    memory_arena MainArena;
};



#endif //GAME_H
