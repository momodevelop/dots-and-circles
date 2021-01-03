// This layer is basically a bunch of settings required by both 
// the game and platform.

#ifndef __PLATFORM__
#define __PLATFORM__

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


// Input API /////////////////////////////////////////
#if INTERNAL
enum input_debug_key {
    GameDebugKey_F1,
    GameDebugKey_F2,
    GameDebugKey_F3,
    GameDebugKey_F4,
    GameDebugKey_F5,
    GameDebugKey_F6,
    GameDebugKey_F7,
    GameDebugKey_F8,
    GameDebugKey_F9,
    GameDebugKey_F10,
    GameDebugKey_F11,
    GameDebugKey_F12,
    GameDebugKey_Return,
    GameDebugKey_Backspace,

    GameDebugKey_Count
};
#endif

struct input_button {
    bool Before : 1;
    bool Now : 1;
};

struct input {
    union {
        input_button Buttons[6];
        struct {
            input_button ButtonUp;
            input_button ButtonDown;
            input_button ButtonRight;
            input_button ButtonLeft;
            input_button ButtonConfirm;
            input_button ButtonSwitch;
        };
    };
};



static inline void
Update(input* Input) {
    for (auto&& itr : Input->Buttons) {
        itr.Before = itr.Now;
    }
}

// before: 0, now: 1
bool IsPoked(input_button Button) {
    return !Button.Before && Button.Now;
}

// before: 1, now: 0
bool IsReleased(input_button Button) {
    return Button.Before && !Button.Now;
}


// before: X, now: 1
bool IsDown(input_button Button) {
    return Button.Now;
}

// before: 1, now: 1
bool IsHeld(input_button Button) {
    return Button.Before && Button.Now;
}

// Platform Api ////////////////////////////////////////////////////
typedef void platform_log(const char * Format, ...);
typedef u32  platform_get_file_size(const char * Path);
typedef b32 platform_read_file(void* Dest, u32 DestSize, const char* Path);

struct platform_state {
#if INTERNAL 
    string_buffer DebugCharInput;
    input_button DebugKeys[GameDebugKey_Count];
#endif
};
struct platform_api {
    platform_state* State;

    platform_log* Log;
    platform_get_file_size* GetFileSize;
    platform_read_file* ReadFile;
};

#if INTERNAL
static inline void
UpdateDebugState(platform_state* DebugState) {
    Clear(&DebugState->DebugCharInput);
    for (auto&& itr : DebugState->DebugKeys) {
        itr.Before = itr.Now;
    }
}

static inline b32
TryPushCharacterInput(platform_state* Debug, char C) {
    if (C >= 32 && C <= 126) {
        Push(&Debug->DebugCharInput, C);
        return true;
    }
    return false;
}
#endif 


struct game_memory {
    void* MainMemory;
    usize MainMemorySize;
    
    void* DebugMemory;
    usize DebugMemorySize;
};

// NOTE(Momo): Function typedefs and helpers
#define GAME_UPDATE(Name) void Name(game_memory* GameMemory, \
                                    platform_api* Platform, \
                                    mailbox* RenderCommands, \
                                    input* Input, \
                                    f32 DeltaTime)
typedef GAME_UPDATE(game_update);


#endif //PLATFORM_H
