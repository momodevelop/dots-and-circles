#ifndef __YUU_PLATFORM__
#define __YUU_PLATFORM__

#include "ryoji.h"

// NOTE(Momo): Global Settings
#define GameMainMemorySize Megabytes(100)
#define RenderCommandsMemorySize Megabytes(64)
#define TotalMemorySize GameMainMemorySize + RenderCommandsMemorySize


// NOTE(Momo): Forward declarations 
struct render_command_queue;
struct game_memory;
struct platform_api;
struct platform_read_file_result;
union game_input;

// NOTE(Momo): Function typedefs and helpers
typedef void game_update(game_memory* GameMemory,  
                         platform_api* Platform, 
                         render_command_queue* RenderCommands, 
                         game_input* Input, 
                         f32 DeltaTime);
typedef void platform_log(const char* Format, ...);
typedef u32  platform_get_file_size(const char* Path);
typedef void platform_read_file(void* Dest, u32 DestSize, const char* Path);

struct platform_read_file_result {
    void* Content;
    u32 ContentSize;
};

struct platform_api {
    platform_log* Log;
    platform_get_file_size* GetFileSize;
    platform_read_file* ReadFile;
};


struct game_memory {
    void* MainMemory;
    u64 MainMemorySize;
};


#endif //PLATFORM_H
