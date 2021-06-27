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
    String_Buffer Characters;
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
Input_Init(platform_input* Input, Arena* arena) {
    return Input->Characters.alloc(arena, 10);
}


static inline b8
Input_TryPushCharacterInput(platform_input* Input, char C) {
    if (C >= 32 && C <= 126) {
        Input->Characters.push(C);
        return true;
    }
    return false;
}

static inline void
Input_Update(platform_input* Input) {
    Input->Characters.clear();
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


#define PlatformLogDecl(Name) void Name(const char* Format, ...)
typedef PlatformLogDecl(platform_log);

#define PlatformOpenAssetFileDecl(Name) platform_file_handle Name(void)
typedef PlatformOpenAssetFileDecl(platform_open_asset_file);

#define PlatformCloseFileDecl(Name) void Name(platform_file_handle* Handle)
typedef PlatformCloseFileDecl(platform_close_file);

#define PlatformReadFileDecl(Name) void Name(platform_file_handle* Handle, usize Offset, usize Size, void* Dest)
typedef PlatformReadFileDecl(platform_read_file);

#define PlatformGetFileSizeDecl(Name) u32 Name(const char* Path)
typedef PlatformGetFileSizeDecl(platform_get_file_size);

#define PlatformLogFileErrorDecl(Name) void Name(platform_file_handle* Handle)
typedef PlatformLogFileErrorDecl(platform_log_file_error);

#define PlatformAddTextureDecl(Name) renderer_texture_handle Name(u32 Width, u32 Height, void* Pixels)
typedef PlatformAddTextureDecl(platform_add_texture);

#define PlatformClearTexturesDecl(Name) void Name()
typedef PlatformClearTexturesDecl(platform_clear_textures);

#define PlatformHideCursorDecl(Name) void Name()
typedef PlatformHideCursorDecl(platform_hide_cursor);

#define PlatformShowCursorDecl(Name) void Name()
typedef PlatformShowCursorDecl(platform_show_cursor);

#define PlatformGetPerformanceCounterDecl(Name) u64 Name()
typedef PlatformGetPerformanceCounterDecl(platform_get_performance_counter);



struct platform_api {
    platform_log* LogFp;
    platform_get_file_size* GetFileSizeFp;
    platform_add_texture* AddTextureFp;
    platform_clear_textures* ClearTexturesFp;
    platform_open_asset_file* OpenAssetFileFp;
    platform_close_file* CloseFileFp;
    platform_log_file_error* LogFileErrorFp;
    platform_read_file* ReadFileFp;
    platform_show_cursor* ShowCursorFp;
    platform_hide_cursor* HideCursorFp;
    
    platform_get_performance_counter* GetPerformanceCounterFp;
};

// Memory required by the game to get it running
// Note that Transient Memory, once set, should not be moved!
// TODO(Momo): should change name to "platform_memory" because it is
// memory FROM the platform
struct game_memory {
    void* PermanentMemory;
    u32 PermanentMemorySize;
    
    void* TransientMemory;
    u32 TransientMemorySize;
    
    void* ScratchMemory;
    u32 ScratchMemorySize;
    
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
platform_api* PlatformApi, \
Mailbox* RenderCommands, \
platform_input* PlatformInput, \
platform_audio* Audio,\
f32 DeltaTime)
typedef GameUpdateFunc(game_update);

#endif //PLATFORM_H
