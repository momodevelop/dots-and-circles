// This layer is basically a bunch of settings required by both 
// the game and platform.

#ifndef __PLATFORM__
#define __PLATFORM__

#include "mm_core.h"

// NOTE(Momo): Global Settings
static const char* Global_GameCodeDllFileName = "game.dll";
static const char* Global_TempGameCodeDllFileName = "tmp_game.dll";
static const char* Global_GameCodeLockFileName = "lock";
static constexpr u64 Global_GameMainMemorySize = Gigabytes(1);
static constexpr u64 Global_RenderCommandsMemorySize = Megabytes(64);
static constexpr u32 Global_DefaultRefreshRate = 60;
static constexpr f32 Global_DesignWidth = 1600.f;
static constexpr f32 Global_DesignHeight = 900.f;
static constexpr f32 Global_DesignDepth = 200.f;

#if INTERNAL
static constexpr u64 Global_GameDebugMemorySize = Megabytes(64);
static constexpr u64 Global_TotalMemorySize = Global_GameMainMemorySize + Global_RenderCommandsMemorySize + Global_GameDebugMemorySize;
#else // !INTERNAL
static constexpr u64 Global_TotalMemorySize = Global_GameMainMemorySize + Global_RenderCommandsMemorySize;
#endif

// Old, remove soon
static constexpr u64 GameMainMemorySize = Gigabytes(1);
static constexpr u64 RenderCommandsMemorySize = Megabytes(64);
static constexpr u64 GlobalDefaultRefreshRate = 60;

static constexpr f32 DesignWidth = 1600.f;
static constexpr f32 DesignHeight = 900.f;
static constexpr f32 DesignDepth = 200.f;

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
                         f32 DeltaTime);

// TODO: Might want to rethink about better API for these, like better return values
// and structures?
typedef void platform_log(const char * Format, ...);
typedef u32  platform_get_file_size(const char * Path);
typedef b32 platform_read_file(void* Dest, u32 DestSize, const char* Path);

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
