#ifndef __GAME_PLATFORM__
#define __GAME_PLATFORM__

#include "ryoji.h"
#include "ryoji_maths.h"


// NOTE(Momo): Forward declarations 
struct render_commands;
struct game_memory;
struct platform_api;
struct platform_read_file_result;


// NOTE(Momo): Function typedefs and helpers
#define GAME_UPDATE(name) void name(game_memory* GameMemory, render_commands* RenderCommands, f32 DeltaTime)
typedef GAME_UPDATE(game_update);
#define PLATFORM_LOG(name) void name(const char* Format, ...)
typedef PLATFORM_LOG(platform_log);

// TODO(Momo): Is there a way to remove this?
#define PLATFORM_READ_FILE(name) platform_read_file_result name(const char* path)
typedef PLATFORM_READ_FILE(platform_read_file);


struct platform_read_file_result {
    void * Content;
    u32 ContentSize;
};


struct platform_api {
    platform_log* Log;
    platform_read_file* ReadFile;
};

struct game_memory {
    void* PermanentStore;
    u64 PermanentStoreSize;
    bool IsInitialized;
    platform_api PlatformApi;
};


#endif //PLATFORM_H
