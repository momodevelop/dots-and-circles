#include "game.h"


static inline void
Initialize() {
    
}


// NOTE(Momo):  Exported Functions
extern "C" void
GameUpdate(game_memory* GameMemory, platform_api* PlatformApi, f32 DeltaTime) {
    game_state* state = (game_state*)GameMemory->PermanentStore;
    if(!GameMemory->IsInitialized) {
        GameMemory->IsInitialized = true;
    }
    
    state->TimeElapsed += DeltaTime;
    PlatformApi->Log("Updating: %f", state->TimeElapsed);
    
}