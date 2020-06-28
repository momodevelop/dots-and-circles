#include "interface.h"

#include "game.h"
#include "ryoji_maths.h"


// TODO(Momo): Complete this function
#if 0
static inline void
Push(render_cmd_list* RenderCmdList,  render_cmd_type Type, void* Data) {
}

// TODO(Momo): Complete this function
static inline void 
Clear(render_cmd_list* RenderCmdList) {
    
}

#endif

struct game_state {
    render_group RenderGroup;
    f32 TimeElapsed;
    f32 Rotation;
};


static inline void 
GameInit(game_state * GameState) {
    GameState->TimeElapsed = 0.f;
    for (u32 i = 0 ; i < 1024; ++i ) {
        GameState->RenderGroup.Transforms[i] = Identity();
    }
    GameState->Rotation = 0.f;
}



// NOTE(Momo):  Exported Functions
extern "C" void
GameUpdate(game_memory* GameMemory, platform_api* PlatformApi, f32 DeltaTime) {
    game_state* GameState = (game_state*)GameMemory->PermanentStore;
    if(!GameMemory->IsInitialized) {
        GameInit(GameState);
        GameMemory->IsInitialized = true;
    }
    
    GameState->TimeElapsed += DeltaTime;
    
    f32 startX = -1600.f/2.f;
    f32 startY = -900.f/2.f;
    f32 xOffset = 200.f;
    f32 yOffset = 200.f;
    f32 currentXOffset = 0.f;
    f32 currentYOffset = 0.f;
    
    for (u32 i = 0 ; i < 1024; ++i ) {
        GameState->RenderGroup.Transforms[i] =
            Transpose(Translation(startX + currentXOffset, startY + currentYOffset, 0.f) *
                      RotationZ(GameState->Rotation) *
                      Scale(100.f, 100.f, 1.f));
        
        currentXOffset += xOffset;
        if (currentXOffset > 1600) {
            currentXOffset = 0.f;
            currentYOffset += yOffset;
        }
    }
    
    GameState->Rotation += DeltaTime;
    Wrap(GameState->Rotation, -Pi32, Pi32);
    
    //PlatformApi->Log("Updating: %f", GameState->TimeElapsed);
    
    PlatformApi->GlProcessRenderGroup(&GameState->RenderGroup);
    
}