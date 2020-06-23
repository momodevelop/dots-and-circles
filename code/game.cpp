// Interface between the game and platform layer

#ifndef __INTERFACE__
#define __INTERFACE__

#include "ryoji.cpp"


// Region Start: PLATFORM INTERFACE.
// TODO(Momo): Shift to another file when it gets too big?

global void PlatformLog(const char * str, ...);

struct PlatformGetFileSizeRes {
    bool ok;
    u64 Size;
    
};

global PlatformGetFileSizeRes PlatformGetFileSize(const char* path);
global bool PlatformReadBinaryFileToMemory(void* dest, u64 destSize, const char* path);
// Region End: PLATFORM INTERFACE

// Region Start: GAME
// TODO(Momo):
//     [] Graphics
//     [] Input
//     [] Sound

struct GameMemory {
    void* permanentStore;
    u64 permanentStoreSize;
    bool isInitialized;
};

internal
void
Init(GameMemory* gm, void* memory, u64 size) {
    gm->permanentStore = memory;
    gm->permanentStoreSize = size;
    gm->isInitialized = false;
}

struct GameState {
    f32 timeElapsed;
};


internal
void
GameUpdate(GameMemory* gameMemory, f32 delta) {
    GameState* state = (GameState*)gameMemory->permanentStore;
    if(!gameMemory->isInitialized) {
        // NOTE(Momo): Initialize game state here
        ZeroStruct(state);
        gameMemory->isInitialized = true;
    }
    
    state->timeElapsed += delta;
    PlatformLog("Updating: %f", state->timeElapsed);
    
}

// Region End: GAME

#endif __GAME__
