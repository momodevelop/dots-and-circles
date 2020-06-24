// Interface between the game and platform layer
#ifndef __INTERFACE__
#define __INTERFACE__

#include "ryoji.h"


// Region Start: PLATFORM INTERFACE.
// TODO(Momo): Shift to another file when it gets too big?

void PlatformLog(const char * str, ...);

struct platform_get_file_size_res {
    bool Ok;
    u64 Size;
    
};


static platform_get_file_size_res 
PlatformGetFileSize(const char* path);

static bool 
PlatformReadBinaryFileToMemory(void* dest, u64 destSize, const char* path);


// Region End: PLATFORM INTERFACE

// Region Start: GAME
// TODO(Momo):
//     [] Graphics
//     [] Input
//     [] Sound

struct game_memory {
    void* PermanentStore;
    u64 PermanentStoreSize;
    bool IsInitialized;
};


struct game_state {
    f32 TimeElapsed;
};


#define GAME_UPDATE(name) void name(game_memory* GameMemory, f32 DeltaTime)
typedef GAME_UPDATE(game_update);
GAME_UPDATE(GameUpdateStub) {}

// Region End: GAME

#endif __GAME__
