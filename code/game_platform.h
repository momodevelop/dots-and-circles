// This layer is basically a bunch of settings required by both 
// the game and platform.
#ifndef __PLATFORM__
#define __PLATFORM__



// Input API /////////////////////////////////////////
struct Platform_Input_Button {
    b8 before : 1;
    b8 now : 1;
    
    b8 is_poked();
    b8 is_released();
    b8 is_down();
    b8 is_held();
};

struct Platform_Input {
    String_Buffer characters;
    union {
        Platform_Input_Button buttons[8];
        struct {
            Platform_Input_Button button_switch;
            
            // NOTE(Momo): Kinda for in-game debugging
            Platform_Input_Button button_confirm;
            Platform_Input_Button button_back;
            Platform_Input_Button button_console;
            Platform_Input_Button button_inspector;
            Platform_Input_Button button_pause;
            Platform_Input_Button button_speed_up;
            Platform_Input_Button button_speed_down;
        };
    };
    
    
    v2f design_mouse_pos;
    v2f window_mouse_pos;
    v2f render_mouse_pos;
    
    //- functions
    b8 alloc(Arena* arena);
    b8 try_push_character_input(u8 c);
    void update();
};


b8
Platform_Input::alloc(Arena* arena) {
    return characters.alloc(arena, 10);
}


b8
Platform_Input::try_push_character_input(u8 c) {
    if (c >= 32 && c <= 126) {
        characters.push(c);
        return true;
    }
    return false;
}

void
Platform_Input::update() {
    characters.clear();
    for (auto&& itr : buttons) {
        itr.before = itr.now;
    }
}

// before: 0, now: 1
b8 
Platform_Input_Button::is_poked() {
    return !this->before && this->now;
}

// before: 1, now: 0
b8
Platform_Input_Button::is_released() {
    return this->before && !this->now;
}


// before: X, now: 1
b8
Platform_Input_Button::is_down() {
    return this->now;
}

// before: 1, now: 1
b8
Platform_Input_Button::is_held() {
    return this->before && this->now;
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
Platform_Input* platform_input, \
platform_audio* Audio,\
f32 DeltaTime)
typedef GameUpdateFunc(game_update);

#endif //PLATFORM_H
