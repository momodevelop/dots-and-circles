#ifndef __GAME_PLATFORM__
#define __GAME_PLATFORM__

#include "ryoji.h"

// NOTE(Momo): Global Settings
#define GameMemorySize Megabytes(100)
#define RenderCommandsMemorySize Megabytes(64)
#define TotalMemorySize GameMemorySize + RenderCommandsMemorySize


// NOTE(Momo): Forward declarations 
struct render_commands;
struct game_memory;
struct platform_api;
struct platform_read_file_result;
union game_input;

// NOTE(Momo): Function typedefs and helpers
#define GAME_UPDATE(name) void name(platform_api * Platform, game_memory* GameMemory,  render_commands* RenderCommands, game_input * Input, f32 DeltaTime)
typedef GAME_UPDATE(game_update);
#define PLATFORM_LOG(name) void name(const char* Format, ...)
typedef PLATFORM_LOG(platform_log);


// TODO(Momo): This is just temp code! To be replaced by asset loading code
#define PLATFORM_READ_FILE(name) platform_read_file_result name(const char* path)
typedef PLATFORM_READ_FILE(platform_read_file);

#define PLATFORM_FREE_FILE(name) void name(platform_read_file_result File)
typedef PLATFORM_FREE_FILE(platform_free_file);


struct platform_read_file_result {
    void* Content;
    u32 ContentSize;
};

struct platform_api {
    platform_log* Log;
    
    // TODO(Momo): Maybe replace with proper asset management?
    platform_read_file* ReadFile;
    platform_free_file* FreeFile;
};

struct game_memory {
    void* Memory;
    u64 MemorySize;
    bool IsInitialized;
};


#endif //PLATFORM_H
