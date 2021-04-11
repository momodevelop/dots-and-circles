// This layer is basically a bunch of settings required by both 
// the game and platform.

#ifndef __PLATFORM__
#define __PLATFORM__


// NOTE(Momo): Global Settings

#define Game_DesignWidth 800
#define Game_DesignHeight 800
#define Game_DesignDepth 200

// Input API /////////////////////////////////////////
struct game_input_button {
    bool Before : 1;
    bool Now : 1;
};

struct game_input {
    u8_str Characters;
    union {
        // TODO(Momo): maybe we don't do buttons but do mouse?
        game_input_button Buttons[9];
        struct {
            game_input_button ButtonUp;
            game_input_button ButtonDown;
            game_input_button ButtonRight;
            game_input_button ButtonLeft;
            
            game_input_button ButtonConfirm;
            game_input_button ButtonSwitch;
            game_input_button ButtonBack;
            
            // NOTE(Momo): Kinda for in-game debugging
            game_input_button ButtonConsole;
            game_input_button ButtonInspector;
        };
    };
    
    
    v2f DesignMousePos;
    v2f WindowMousePos;
    v2f RenderMousePos;
};


static inline game_input
Input_Create(u8_str Buffer) {
    game_input Ret = {};
    Ret.Characters = Buffer;
    return Ret;
}


static inline b32
Input_TryPushCharacterInput(game_input* Input, char C) {
    if (C >= 32 && C <= 126) {
        U8Str_Push(&Input->Characters, C);
        return True;
    }
    return False;
}

static inline void
Input_Update(game_input* Input) {
    U8Str_Clear(&Input->Characters);
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

struct game_audio {
    i16* SampleBuffer;
    usize SampleCount;
};

// Game function typedefs
// Returns true if still running, false if need to quit.
#define GameUpdateFunc(Name) b32 Name(game_memory* GameMemory, \
platform_api* Platform, \
mailbox* RenderCommands, \
game_input* Input, \
game_audio* Audio,\
f32 DeltaTime)
typedef GameUpdateFunc(game_update);


#endif //PLATFORM_H
