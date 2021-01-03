// This layer is basically a bunch of settings required by both 
// the game and platform.

#ifndef __PLATFORM__
#define __PLATFORM__

#include <stdarg.h>
#include "mm_core.h"
#include "mm_string.h"
#include "mm_mailbox.h"

// NOTE(Momo): Global Settings
static constexpr u64 Global_GameMainMemorySize = Gigabytes(1);
static constexpr u64 Global_RenderCommandsMemorySize = Megabytes(64);
static constexpr u32 Global_DefaultRefreshRate = 60;
static constexpr f32 Global_DesignWidth = 1600.f;
static constexpr f32 Global_DesignHeight = 900.f;
static constexpr f32 Global_DesignDepth = 200.f;
static constexpr u64 Global_TotalMemorySize = Global_GameMainMemorySize + Global_RenderCommandsMemorySize;


// Input API /////////////////////////////////////////
struct input_button {
    bool Before : 1;
    bool Now : 1;
};

struct input {
    string_buffer Characters;
    union {
        input_button Buttons[6];
        struct {
            input_button ButtonUp;
            input_button ButtonDown;
            input_button ButtonRight;
            input_button ButtonLeft;
            input_button ButtonConfirm;
            input_button ButtonSwitch;
            input_button ButtonBack;

            // Meta keys
            input_button ButtonConsole;

        };
    };
};


static inline void
Init(input* Input,
     char* Memory,
     usize Capacity) 
{
    Input->Characters = StringBuffer(Memory, Capacity);
}


static inline b32
TryPushCharacterInput(input* Input, char C) {
    if (C >= 32 && C <= 126) {
        Push(&Input->Characters, C);
        return true;
    }
    return false;
}

static inline void
Update(input* Input) {
    Clear(&Input->Characters);
    for (auto&& itr : Input->Buttons) {
        itr.Before = itr.Now;
    }
}

// before: 0, now: 1
static inline b32 
IsPoked(input_button Button) {
    return !Button.Before && Button.Now;
}

// before: 1, now: 0
static inline b32
IsReleased(input_button Button) {
    return Button.Before && !Button.Now;
}


// before: X, now: 1
static inline b32
IsDown(input_button Button) {
    return Button.Now;
}

// before: 1, now: 1
static inline b32
IsHeld(input_button Button) {
    return Button.Before && Button.Now;
}

// Platform Api ////////////////////////////////////////////////////


#define PlatformLogFunc(Name) void Name(const char* Format, ...)
typedef PlatformLogFunc(platform_log);

#define PlatformGetFileSizeFunc(Name) u32 Name(const char* Path)
typedef PlatformGetFileSizeFunc(platform_get_file_size);

#define PlatformReadFileFunc(Name) b32 Name(void* Dest, u32 DestSize, const char* Path)
typedef PlatformReadFileFunc(platform_read_file);

struct platform_api {
    platform_log* Log;
    platform_get_file_size* GetFileSize;
    platform_read_file* ReadFile;
};


// Memory required by the game to get it running
struct game_memory {
    void* MainMemory;
    usize MainMemorySize;
    
    void* DebugMemory;
    usize DebugMemorySize;
};

// NOTE(Momo): Function typedefs and helpers
#define GameUpdateFunc(Name) void Name(game_memory* GameMemory, \
                                       platform_api* Platform, \
                                       mailbox* RenderCommands, \
                                       input* Input, \
                                       f32 DeltaTime)
typedef GameUpdateFunc(game_update);


#endif //PLATFORM_H
