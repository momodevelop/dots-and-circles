#ifndef __YUU_PLATFORM__
#define __YUU_PLATFORM__

#include "mm_core.h"

// NOTE(Momo): Global Settings
static constexpr u64 GameMainMemorySize = Gigabytes(1);
static constexpr u64 RenderCommandsMemorySize = Megabytes(64);

constexpr static f32 DesignWidth = 1600.f;
constexpr static f32 DesignHeight = 900.f;
constexpr static f32 DesignDepth = 200.f;

#if INTERNAL
static constexpr u64 DebugMemorySize = Megabytes(64);
static constexpr u64 TotalMemorySize = GameMainMemorySize + RenderCommandsMemorySize + DebugMemorySize;
#else 
static constexpr u64 TotalMemorySize = GameMainMemorySize + RenderCommandsMemorySize;
#endif 

// NOTE(Momo): Forward declarations 
struct mailbox;
struct game_memory;
struct platform_api;
struct game_debug_keyboard;
struct game_input;

// NOTE(Momo): Function typedefs and helpers
typedef void game_update(game_memory* GameMemory,  
                         platform_api* Platform, 
                         mailbox* RenderCommands, 
                         game_input* Input,
                         f32 DeltaTime,
                         u64 TicksElapsed);
typedef void platform_log(const char * Format, ...);
typedef u32  platform_get_file_size(const char * Path);
typedef void platform_read_file(void* Dest, u32 DestSize, const char* Path);

struct platform_api {
    platform_log* Log;
    platform_get_file_size* GetFileSize;
    platform_read_file* ReadFile;
};


struct game_memory {
    void* MainMemory;
    usize MainMemorySize;
    
    void* DebugMemory;
    usize DebugMemorySize;
};


#endif //PLATFORM_H
