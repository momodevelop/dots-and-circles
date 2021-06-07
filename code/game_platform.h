// This layer is basically a bunch of settings required by both 
// the game and platform.

#ifndef __PLATFORM__
#define __PLATFORM__



// Input API /////////////////////////////////////////
struct platform_input_button {
    bool Before : 1;
    bool Now : 1;
};

struct platform_input {
    u8_str Characters;
    union {
        platform_input_button Buttons[8];
        struct {
            platform_input_button ButtonSwitch;
            
            // NOTE(Momo): Kinda for in-game debugging
            platform_input_button ButtonConfirm;
            platform_input_button ButtonBack;
            platform_input_button ButtonConsole;
            platform_input_button ButtonInspector;
            platform_input_button ButtonPause;
            platform_input_button ButtonSpeedUp;
            platform_input_button ButtonSpeedDown;
        };
    };
    
    
    v2f DesignMousePos;
    v2f WindowMousePos;
    v2f RenderMousePos;
};


static inline b8
Input_Init(platform_input* Input, arena* Arena) {
    return U8Str_New(&Input->Characters, Arena, 10);
}


static inline b8
Input_TryPushCharacterInput(platform_input* Input, char C) {
    if (C >= 32 && C <= 126) {
        U8Str_Push(&Input->Characters, C);
        return true;
    }
    return false;
}

static inline void
Input_Update(platform_input* Input) {
    U8Str_Clear(&Input->Characters);
    for (auto&& itr : Input->Buttons) {
        itr.Before = itr.Now;
    }
}

// before: 0, now: 1
static inline b8 
Button_IsPoked(platform_input_button Button) {
    return !Button.Before && Button.Now;
}

// before: 1, now: 0
static inline b8
Button_IsReleased(platform_input_button Button) {
    return Button.Before && !Button.Now;
}


// before: X, now: 1
static inline b8
Button_IsDown(platform_input_button Button) {
    return Button.Now;
}

// before: 1, now: 1
static inline b8
Button_IsHeld(platform_input_button Button) {
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
    platform_log* LogFp;
    platform_get_file_size* GetFileSizeFp;
    platform_add_texture* AddTextureFp;
    platform_clear_textures* ClearTexturesFp;
    platform_open_asset_file* OpenAssetFileFp;
    platform_close_file* CloseFileFp;
    platform_log_file_error* LogFileErrorFp;
    platform_read_file* ReadFileFp;
    
};


// Memory required by the game to get it running
// Note that Transient Memory, once set, should not be moved!
struct game_memory {
    void* PermanentMemory;
    u32 PermanentMemorySize;
    
    void* TransientMemory;
    u32 TransientMemorySize;
    
    void* DebugMemory;
    u32 DebugMemorySize;
};

struct platform_audio {
    s16* SampleBuffer;
    u32 SampleCount;
    u32 Channels;
};

// Game function typedefs
// Returns true if still running, false if need to quit.
#define GameUpdateFunc(Name) b8 Name(game_memory* GameMemory, \
platform_api* Platform, \
mailbox* RenderCommands, \
platform_input* Input, \
platform_audio* Audio,\
f32 DeltaTime)
typedef GameUpdateFunc(game_update);

#endif //PLATFORM_H
