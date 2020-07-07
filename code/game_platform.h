#ifndef __GAME_PLATFORM__
#define __GAME_PLATFORM__

#include "ryoji.h"
#include "ryoji_maths.h"


// NOTE(Momo): Forward declarations 
struct render_info;
struct game_memory;
struct platform_api;

// NOTE(Momo): Function typedefs and helpers
#define GAME_UPDATE(name) void name(game_memory* GameMemory, render_info* RenderInfo, f32 DeltaTime)
typedef GAME_UPDATE(game_update);
#define PLATFORM_LOG(name) void name(const char* Format, ...)
typedef PLATFORM_LOG(platform_log);



struct render_info {
    m44f Transforms[1024];
    m44f Colors[1024];
    usize Count;
};

struct platform_api {
    platform_log* Log;
};

struct game_memory {
    void* PermanentStore;
    u64 PermanentStoreSize;
    bool IsInitialized;
    platform_api PlatformApi;
};


#endif //PLATFORM_H
