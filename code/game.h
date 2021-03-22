#ifndef GAME_H
#define GAME_H

#include "mm_core.h"

#include "game_renderer.h"
#include "game_platform.h"
#include "game_assets_types.h"
#include "game_assets.h"
#include "game_draw.h"
#include "game_debug_console.h"
#include "game_debug.h"




enum game_mode_type {
    GameModeType_Splash,
    GameModeType_Main,
    GameModeType_Menu,
    GameModeType_Sandbox,
    
    GameModeType_None
};

// Stuff in this state can be zero initialized with no problems
// I.e. This state contains stuff that does not need to exist past a frame
struct transient_state {
    b32 IsInitialized;
    
    arena Arena;
    game_assets* Assets; 
};

struct permanent_state {
    b32 IsInitialized;
    b32 IsRunning;
    b32 IsPaused;
    
    game_mode_type CurrentGameMode;
    game_mode_type NextGameMode;
    union {
        struct game_mode_splash* SplashMode;
        struct game_mode_main* MainMode;
        struct game_mode_sandbox* SandboxMode;
    };
    
    arena MainArena;
    arena ModeArena;
    
};


// Common functions
static inline void
SwitchToGameCoords(mailbox* RenderCommands) {
    aabb3f CenterBox = Aabb3f_Centered(Global_DesignSpace, Global_DesignSpaceAnchor);
    PushClearColor(RenderCommands, { 0.0f, 0.3f, 0.3f, 0.f });
    PushOrthoCamera(RenderCommands, 
                    v3f{}, 
                    CenterBox);
}

static inline void
SwitchToUICoords(mailbox* RenderCommands) {
    aabb3f CenterBox = Aabb3f_Centered(Global_DesignSpace, v3f{});
    PushClearColor(RenderCommands, { 0.0f, 0.3f, 0.3f, 0.f });
    PushOrthoCamera(RenderCommands, 
                    v3f{}, 
                    CenterBox);
}

#endif //GAME_H
