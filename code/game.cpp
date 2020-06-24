#include "game.h"


extern "C" GAME_UPDATE(GameUpdate) {
    game_state* state = (game_state*)GameMemory->PermanentStore;
    if(!GameMemory->IsInitialized) {
        GameMemory->IsInitialized = true;
    }
    
    state->TimeElapsed += DeltaTime;
    //PlatformLog("Updating: %f", state->TimeElapsed);
    
}