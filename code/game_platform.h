#ifndef __GAME_PLATFORM__
#define __GAME_PLATFORM__

#include "ryoji.h"
#include "ryoji_maths.h"


// NOTE(Momo): simple entity system
// Nothing weird, just simple TRS transform
// TODO(Momo): Maybe move to ryoji?? Since it's gonna be generic
#if 0 
enum struct render_cmd_type {
    Clear,
    Bitmap,
    // TODO(Momo):  Colored Rectangle?
};


struct render_cmd_data_bitmap {
    m44f Transform;
    m44f Color;
    u32 TextureHandle;
};

struct render_cmd {
    render_cmd_type RenderCmdType;
    void* Data;
};

struct render_cmd_list {
    render_cmd* Commands; 
    usize Size;
};
#endif

// NOTE(Momo): Forward declarations 
struct render_info;
struct game_memory;
struct platform_api;

// NOTE(Momo): Function typedefs and helpers
#define GAME_UPDATE(name) void name(game_memory* GameMemory, f32 DeltaTime)
typedef GAME_UPDATE(game_update);
#define PLATFORM_LOG(name) void name(const char* Format, ...)
typedef PLATFORM_LOG(platform_log);
#define PLATFORM_SET_RENDER_INFO(name) void name(render_info* RenderInfo)
typedef PLATFORM_SET_RENDER_INFO(platform_set_render_info);



enum struct renderer_type {
    SOFTWARE,
    OPENGL,
    DIRECTX
};


struct render_info {
    m44f Transforms[1024];
    m44f Colors[1024];
    usize Count;
};

struct platform_api {
    platform_log* Log;
    platform_set_render_info* SetRenderInfo;
};

struct game_memory {
    void* PermanentStore;
    u64 PermanentStoreSize;
    bool IsInitialized;
    renderer_type RendererType; 
    platform_api PlatformApi;
};


#endif //PLATFORM_H
