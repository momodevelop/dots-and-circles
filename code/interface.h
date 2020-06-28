#ifndef __INTERFACE__
#define __INTERFACE__

#include "ryoji.h"
#include "ryoji_maths.h"

// NOTE(Momo): simple entity system
// Nothing weird, just simple TRS transform
// TODO(Momo): Maybe move to ryoji?? Since it's gonna be generic
enum struct render_cmd_type {
    Clear,
    RenderBitmap,
};


struct render_cmd_entry {
    render_cmd_type RenderCmdType;
    m44f Transform;
    m44f Color;
    // TODO(Momo): Texture?
    u32 TextureHandle;
};

struct render_cmd {
    render_cmd_entry Entries;
};

struct platform_api {
    void (*Log)(const char* str, ...);
    
    u32 (*LoadTexture)(const char* path);
    
    
    // TODO(Momo): Change to render_group
    void (*PassRenderGroup)(); 
    
};

struct game_memory {
    void* PermanentStore;
    u64 PermanentStoreSize;
    bool IsInitialized;
};


// NOTE(Momo): game functions from DLL
typedef void game_update(game_memory* GameMemory, platform_api* PlatformApi, f32 DeltaTime);


struct platform_get_file_size_res {
    bool Ok;
    u64 Size;
    
};


static platform_get_file_size_res 
PlatformGetFileSize(const char* path);

static bool 
PlatformReadBinaryFileToMemory(void* dest, u64 destSize, const char* path);






#endif //PLATFORM_H
