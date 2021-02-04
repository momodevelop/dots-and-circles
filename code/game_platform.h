// This layer is basically a bunch of settings required by both 
// the game and platform.

#ifndef __PLATFORM__
#define __PLATFORM__

#include <stdarg.h>
#include "mm_core.h"
#include "mm_string.h"
#include "mm_mailbox.h"
#include "game_renderer.h"

// NOTE(Momo): Global Settings
static constexpr f32 Global_DesignWidth = 1600.f;
static constexpr f32 Global_DesignHeight = 900.f;
static constexpr f32 Global_DesignDepth = 200.f;


// Input API /////////////////////////////////////////
struct game_input_button {
    bool Before : 1;
    bool Now : 1;
};

struct game_input {
    string_buffer Characters;
    union {
        game_input_button Buttons[8];
        struct {
            game_input_button ButtonUp;
            game_input_button ButtonDown;
            game_input_button ButtonRight;
            game_input_button ButtonLeft;
            game_input_button ButtonConfirm;
            game_input_button ButtonSwitch;
            game_input_button ButtonBack;
            game_input_button ButtonConsole;
        };
    };
    

};


static inline game_input
Input(string_buffer Buffer) 
{
    game_input Ret = {};
    Ret.Characters = Buffer;
    return Ret;
}


static inline b32
TryPushCharacterInput(game_input* Input, char C) {
    if (C >= 32 && C <= 126) {
        Push(&Input->Characters, C);
        return true;
    }
    return false;
}

static inline void
Update(game_input* Input) {
    Clear(&Input->Characters);
    for (auto&& itr : Input->Buttons) {
        itr.Before = itr.Now;
    }
}

// before: 0, now: 1
static inline b32 
IsPoked(game_input_button Button) {
    return !Button.Before && Button.Now;
}

// before: 1, now: 0
static inline b32
IsReleased(game_input_button Button) {
    return Button.Before && !Button.Now;
}


// before: X, now: 1
static inline b32
IsDown(game_input_button Button) {
    return Button.Now;
}

// before: 1, now: 1
static inline b32
IsHeld(game_input_button Button) {
    return Button.Before && Button.Now;
}

// Platform Api ////////////////////////////////////////////////////
struct platform_file_handle {
    u32 Id;
    u32 Error; // 0 is always no error!
};


#define PlatformLogFunc(Name) void Name(const char* Format, ...)
typedef PlatformLogFunc(platform_log);

#define PlatformOpenAssetFileFunc(Name) platform_file_handle Name(void)
typedef PlatformOpenAssetFileFunc(platform_open_asset_file);

#define PlatformCloseFileFunc(Name) void Name(platform_file_handle* Handle)
typedef PlatformCloseFileFunc(platform_close_file);

#define PlatformReadFileFunc(Name) void Name(platform_file_handle* Handle, usize Offset, usize Size, void* Dest)
typedef PlatformReadFileFunc(platform_read_file);

#define PlatformGetFileSizeFunc(Name) u32 Name(const char* Path)
typedef PlatformGetFileSizeFunc(platform_get_file_size);

#define PlatformLogFileErrorFunc(Name) void Name(platform_file_handle* Handle)
typedef PlatformLogFileErrorFunc(platform_log_file_error);

#define PlatformAddTextureFunc(Name) renderer_texture_handle Name(u32 Width, u32 Height, void* Pixels)
typedef PlatformAddTextureFunc(platform_add_texture);

#define PlatformClearTexturesFunc(Name) void Name()
typedef PlatformClearTexturesFunc(platform_clear_textures);


struct platform_api {
    platform_log* Log;
    platform_get_file_size* GetFileSize;
    platform_add_texture* AddTexture;
    platform_clear_textures* ClearTextures;
    platform_open_asset_file* OpenAssetFile;
    platform_close_file* CloseFile;
    platform_log_file_error* LogFileError;
    platform_read_file* ReadFile;
};


// Memory required by the game to get it running
// Note that Transient Memory, once set, should not be moved!
struct game_memory {
    void* PermanentMemory;
    usize PermanentMemorySize;
    
    void* TransientMemory;
    usize TransientMemorySize;

    void* DebugMemory;
    usize DebugMemorySize;
};

struct game_audio {
};

// Game function typedefs
// Returns true if still running, false if need to quit.
#define GameUpdateFunc(Name) b32 Name(game_memory* GameMemory, \
                                      platform_api* Platform, \
                                      mailbox* RenderCommands, \
                                      game_input* Input, \
                                      game_audio* Audio\
                                      f32 DeltaTime)
typedef GameUpdateFunc(game_update);


#endif //PLATFORM_H
