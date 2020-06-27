#include "interface.h"

#include "game.h"
#include "ryoji_maths.h"


struct game_state {
    render_group RenderGroup;
    f32 TimeElapsed;
};



static inline void 
GameInit(game_memory * GameMemory) {
    GameMemory->IsInitialized = true;
}



// NOTE(Momo):  Exported Functions
extern "C" void
GameUpdate(game_memory* GameMemory, platform_api* PlatformApi, f32 DeltaTime) {
    game_state* GameState = (game_state*)GameMemory->PermanentStore;
    if(!GameMemory->IsInitialized) {
        GameInit(GameMemory);
    }
    
    GameState->TimeElapsed += DeltaTime;
    //PlatformApi->Log("Updating: %f", GameState->TimeElapsed);
    
    // TODO(Momo): Render?
    //PlatformApi->Render();
    
}