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

// NOTE(Momo): "2D" Orthographic camera. 
// Saves the current camera state.
// No direction because we are assuming 2D
struct game_camera {
    v3f Position;
    v3f Anchor; 
    v3f Dimensions;
    c4f Color;
    
};

static inline void
Camera_Set(game_camera* C, mailbox* RenderCommands) {
    aabb3f CenterBox = Aabb3f_CreateCentered(C->Dimensions, C->Anchor);
    Renderer_ClearColor(RenderCommands, C->Color);
    Renderer_SetOrthoCamera(RenderCommands, 
                            C->Position, 
                            CenterBox);
}

static inline void
Camera_ScreenToWorld(game_camera* C) {
    // TODO(Momo): 
    
}

// NOTE(Momo): Things that require states' definitions
#include "game_mode_main.h"
#include "game_mode_splash.h"
#include "game_mode_sandbox.h"

#endif //GAME_H
