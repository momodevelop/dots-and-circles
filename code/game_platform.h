#ifndef __YUU_PLATFORM__
#define __YUU_PLATFORM__

#include "ryoji.h"

// NOTE(Momo): Global Settings
#define GameMemorySize Megabytes(100)
#define RenderCommandsMemorySize Megabytes(64)
#define TotalMemorySize GameMemorySize + RenderCommandsMemorySize


// NOTE(Momo): Forward declarations 
struct render_command_queue;
struct game_memory;
struct platform_api;
struct platform_read_file_result;
union game_input;

// NOTE(Momo): Function typedefs and helpers
typedef void game_update(platform_api* Platform, 
                         game_memory* GameMemory,  
                         render_command_queue* RenderCommands, 
                         game_input* Input, 
                         f32 DeltaTime);
typedef void platform_log(const char* Format, ...);
typedef platform_read_file_result platform_read_file(const char* path);
typedef void platform_free_file(platform_read_file_result File);
typedef void* platform_allocate_memory(usize Size);
typedef void platform_deallocate_memory(void* Memory);

struct platform_read_file_result {
    void* Content;
    u32 ContentSize;
};

struct platform_api {
    platform_log* Log;
    platform_read_file* ReadFile;
    platform_free_file* FreeFile;
    platform_allocate_memory* AllocateMemory;
    platform_deallocate_memory* DeallocateMemory;
};


struct game_memory {
    void* Memory;
    u64 MemorySize;
};


#endif //PLATFORM_H
