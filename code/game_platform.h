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
    StringBuffer characters;
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
    return StringBuffer_Alloc(&characters, arena, 10);
}


b8
Platform_Input::try_push_character_input(u8 c) {
    if (c >= 32 && c <= 126) {
        StringBuffer_Push(&characters, c);
        return true;
    }
    return false;
}

void
Platform_Input::update() {
    StringBuffer_Clear(&characters);
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
struct Platform_File_Handle {
    u32 id;
    u32 error; // 0 is always no error!
};


#define PLATFORM_LOG_DECL(Name) void Name(const char* format, ...)
typedef PLATFORM_LOG_DECL(Platform_Log);

#define PLATFORM_OPEN_ASSET_FILE_DECL(Name) Platform_File_Handle Name(void)
typedef PLATFORM_OPEN_ASSET_FILE_DECL(Platform_Open_Asset_File);

#define PLATFORM_CLOSE_FILE_DECL(Name) void Name(Platform_File_Handle* handle)
typedef PLATFORM_CLOSE_FILE_DECL(Platform_Close_File);

#define PLATFORM_READ_FILE_DECL(Name) void Name(Platform_File_Handle* handle, umi offset, umi size, void* dest)
typedef PLATFORM_READ_FILE_DECL(Platform_Read_File);

#define PLATFORM_GET_FILE_SIZE_DECL(Name) u32 Name(const char* path)
typedef PLATFORM_GET_FILE_SIZE_DECL(Platform_Get_File_Size);

#define PLATFORM_LOG_FILE_ERROR_DECL(Name) void Name(Platform_File_Handle* handle)
typedef PLATFORM_LOG_FILE_ERROR_DECL(Platform_Log_File_Error);

#define PLATFORM_ADD_TEXTURE_DECL(Name) Renderer_Texture_Handle Name(u32 width, u32 height, void* pixels)
typedef PLATFORM_ADD_TEXTURE_DECL(Platform_Add_Texture);

#define PLATFORM_CLEAR_TEXTURES_DECL(Name) void Name()
typedef PLATFORM_CLEAR_TEXTURES_DECL(Platform_Clear_Textures);

#define PLATFORM_HIDE_CURSOR_DECL(Name) void Name()
typedef PLATFORM_HIDE_CURSOR_DECL(Platform_Hide_Cursor);

#define PLATFORM_SHOW_CURSOR_DECL(Name) void Name()
typedef PLATFORM_SHOW_CURSOR_DECL(Platform_Show_Cursor);

#define PLATFORM_GET_PERFORMANCE_COUNTER_DECL(Name) u64 Name()
typedef PLATFORM_GET_PERFORMANCE_COUNTER_DECL(Platform_Get_Performance_Counter);



struct Platform_API {
    Platform_Log* log;
    Platform_Get_File_Size* get_file_size;
    Platform_Add_Texture* add_texture;
    Platform_Clear_Textures* clear_textures;
    Platform_Open_Asset_File* open_asset_file;
    Platform_Close_File* close_file;
    Platform_Log_File_Error* log_file_error;
    Platform_Read_File* read_file;
    Platform_Show_Cursor* show_cursor;
    Platform_Hide_Cursor* hide_cursor;
    
    Platform_Get_Performance_Counter* get_performance_counter;
};

// Memory required by the game to get it running
// Note that Transient Memory, once set, should not be moved!
// TODO(Momo): should change name to "platform_memory" because it is
// memory FROM the platform
struct Game_Memory {
    void* permanent_memory;
    u32 permanent_memory_size;
    
    void* transient_memory;
    u32 transient_memory_size;
    
    void* scratch_memory;
    u32 scratch_memory_size;
    
    void* debug_memory;
    u32 debug_memory_size;
};

struct Platform_Audio {
    s16* sample_buffer;
    u32 sample_count;
    u32 channels;
};

// Game function typedefs
// Returns true if still running, false if need to quit.
#define GameUpdateFunc(Name) b8 Name(Game_Memory* game_memory, \
Platform_API* platform_api, \
Mailbox* render_commands, \
Platform_Input* platform_input, \
Platform_Audio* platform_audio,\
f32 dt)
typedef GameUpdateFunc(Game_Update);

#endif //PLATFORM_H
