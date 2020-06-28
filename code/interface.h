#ifndef __INTERFACE__
#define __INTERFACE__

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

struct render_group {
    m44f Transforms[1024];
    m44f Colors[1024];
    usize Count;
};


struct platform_api {
    void (*Log)(const char* str, ...);
    
    //u32 (*LoadTexture)(const char* path);
    void (*GlProcessRenderGroup)(render_group*); 
    
};

struct game_memory {
    void* PermanentStore;
    u64 PermanentStoreSize;
    bool IsInitialized;
    
    platform_api PlatformApi;
};


// NOTE(Momo): game functions from DLL
typedef void game_update(game_memory* GameMemory,  f32 DeltaTime);


struct platform_get_file_size_res {
    bool Ok;
    u64 Size;
    
};


static platform_get_file_size_res 
PlatformGetFileSize(const char* path);

static bool 
PlatformReadBinaryFileToMemory(void* dest, u64 destSize, const char* path);






#endif //PLATFORM_H
