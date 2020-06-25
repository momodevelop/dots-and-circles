// Interface between the game and platform layer
#ifndef __INTERFACE__
#define __INTERFACE__

#include "ryoji.h"

// Region Start: PLATFORM INTERFACE.
// TODO(Momo): Shift to another file when it gets too big?
struct platform_api {
    void (*Log)(const char* str, ...);
};

struct game_memory {
    void* PermanentStore;
    u64 PermanentStoreSize;
    bool IsInitialized;
};


// NOTE(Momo): game functions from DLL
typedef void game_update(game_memory* GameMemory, platform_api* PlatformApi, f32 DeltaTime);


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

struct game_state {
    f32 TimeElapsed;
};



// Region End: GAME

#endif __GAME__
