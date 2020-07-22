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
typedef void game_update(platform_api* Platform, 
                         game_memory* GameMemory,  
                         render_commands* RenderCommands, 
                         game_input* Input, 
                         f32 DeltaTime);
typedef void platform_log(const char* Format, ...);


// TODO(Momo): This is just temp code! To be replaced by asset loading code
typedef platform_read_file_result platform_read_file(const char* path);
typedef void platform_free_file(platform_read_file_result File);


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
