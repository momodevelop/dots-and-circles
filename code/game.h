#ifndef GAME_H
#define GAME_H

#include "mm_core.h"

#include "game_renderer.h"
#include "game_platform.h"
#include "game_assets_types.h"
#include "game_assets.h"
#include "game_draw.h"


#include "game_debug_inspector.h"
#include "game_debug_console.h"

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
    assets Assets; 
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

struct debug_state {
    b32 IsInitialized;
    arena Arena;
    
    debug_inspector Inspector;
    debug_console Console;
    
    struct permanent_state* PermanentState;
    struct transient_state* TransientState;
};


// Common functions
static inline void
SwitchToGameCoords(mailbox* RenderCommands) {
    v3f DesignSpace = V3f_Create(Game_DesignWidth, Game_DesignHeight, Game_DesignDepth);
    v3f DesignAnchor = V3f_Create(Game_DesignAnchorX, Game_DesignAnchorY, Game_DesignAnchorZ); 
    aabb3f CenterBox = Aabb3f_CreateCentered(DesignSpace, DesignAnchor);
    Renderer_ClearColor(RenderCommands, C4f_Create(0.0f, 0.3f, 0.3f, 0.f));
    Renderer_SetOrthoCamera(RenderCommands, 
                            v3f{}, 
                            CenterBox);
}

static inline void
SwitchToUICoords(mailbox* RenderCommands) {
    v3f DesignSpace = V3f_Create(Game_DesignWidth, Game_DesignHeight, Game_DesignDepth);
    aabb3f CenterBox = Aabb3f_CreateCentered(DesignSpace, v3f{});
    Renderer_ClearColor(RenderCommands, C4f_Create(0.0f, 0.3f, 0.3f, 0.f));
    Renderer_SetOrthoCamera(RenderCommands, 
                            v3f{}, 
                            CenterBox);
}

// NOTE(Momo): Things that require states' definitions
#include "game_mode_main.h"
#include "game_mode_splash.h"
#include "game_mode_sandbox.h"

#endif //GAME_H
