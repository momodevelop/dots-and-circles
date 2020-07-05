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
struct platform_work_queue;
struct render_group;
struct game_memory;
struct platform_api;

// NOTE(Momo): Function typedefs and helpers
#define GAME_UPDATE(name) void name(game_memory* GameMemory, f32 DeltaTime)
typedef GAME_UPDATE(game_update);
#define PLATFORM_WORK_QUEUE_ENTRY_CALLBACK(name) void name(platform_work_queue * Queue, void * Data)
typedef PLATFORM_WORK_QUEUE_ENTRY_CALLBACK(platform_work_queue_entry_callback);
#define PLATFORM_ADD_WORK(name) void name (platform_work_queue* Queue, platform_work_queue_entry_callback * Callback, void* Data)
typedef PLATFORM_ADD_WORK(platform_add_work);
#define PLATFORM_WAIT_FOR_ALL_WORK_TO_COMPLETE(name) void name(platform_work_queue* Queue)
typedef PLATFORM_WAIT_FOR_ALL_WORK_TO_COMPLETE(platform_wait_for_all_work_to_complete);

#define PLATFORM_LOG(name) void name(const char* Format, ...)
typedef PLATFORM_LOG(platform_log);
#define PLATFORM_GL_PROCESS_RENDER_GROUP(name) void name(render_group* RenderGroup)
typedef PLATFORM_GL_PROCESS_RENDER_GROUP(platform_gl_process_render_group);



enum struct renderer_type {
    SOFTWARE,
    OPENGL,
    DIRECTX
};


struct render_group {
    m44f Transforms[1024];
    m44f Colors[1024];
    usize Count;
};

struct platform_api {
    platform_log* Log;
    platform_add_work* EnqueueWork;
    platform_gl_process_render_group* GlProcessRenderGroup;
};

struct game_memory {
    void* PermanentStore;
    u64 PermanentStoreSize;
    bool IsInitialized;
    renderer_type RendererType; 
    platform_api PlatformApi;
};




struct platform_get_file_size_res {
    bool Ok;
    u64 Size;
    
};


static platform_get_file_size_res 
PlatformGetFileSize(const char* path);

static bool 
PlatformReadBinaryFileToMemory(void* dest, u64 destSize, const char* path);





#endif //PLATFORM_H
