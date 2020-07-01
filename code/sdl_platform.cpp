#include <stdlib.h>

#include "ryoji_maths.h"

#include "thirdparty/sdl2/include/SDL.h"
#include "thirdparty/glad/glad.c"


#include "game_platform.h"
#include "game_asset_loader.h"
#include "game_render_gl.h"

#include "sdl_platform_timer.cpp"
#include "sdl_platform_gldebug.cpp"
#include "sdl_platform_game_code.cpp"



static bool gIsRunning = true;

static inline 
sdl_window_size SDLGetWindowSize(SDL_Window* Window) {
    i32 w, h;
    SDL_GetWindowSize(Window, &w, &h);
    return { w, h };
}



// TODO(Momo): export as function pointer to game code
void PlatformLog(const char * str, ...) {
    va_list va;
    va_start(va, str);
    SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO, str, va);
    va_end(va);
}

#if 0
static
PlatformGetFileSizeRes
PlatformGetFileSize(const char* path) {
    SDL_RWops * file = SDL_RWFromFile(path, "r");
    if (file == nullptr) {
        return { false, 0 };
    }
    Defer {
        SDL_RWclose(file);
    };
    
    Sint64 filesize = SDL_RWsize(file);
    if(filesize < 0) {
        return { false, 0 };
    }
    u64 ret = (u64)filesize; 
    
    return { true, ret };
}


#endif



static inline
bool ReadFileStr(char* dest, u64 destSize, const char * path) {
    SDL_RWops* file = SDL_RWFromFile(path, "r");
    if (file == nullptr) {
        return false;
    }
    Defer{
        SDL_RWclose(file);
    };
    
    // Get file size
    u64 filesize = SDL_RWsize(file); // Does not include EOF
    
    if ((filesize + 1) > destSize) {
        return false;
    }
    
    SDL_RWread(file, dest, sizeof(char), filesize);
    
    // Don't forget null terminating value
    dest[filesize] = 0;
    
    return true;
}



// TODO(Momo): Test Thread Code
struct work_queue_entry {
    char * StringToPrint;
};

struct thread_context {
    u32 Index;
};

static u32 NextEntryToDo;
static u32 EntryCount;
work_queue_entry Entries[256];


static inline int 
TestThread(void *ptr) {
    thread_context* Info = (thread_context*)ptr;
    
    for (;;) {
        if ( NextEntryToDo < EntryCount) {
            work_queue_entry* Entry = Entries + NextEntryToDo++;
            SDL_Log("Thread %d: %s", Info->Index, Entry->StringToPrint);
        }
    }
}

static inline void
PushString(char* String) {
    Assert(EntryCount < ArrayCount(Entries));
    work_queue_entry* Entry = Entries + EntryCount++;
    Entry->StringToPrint = String;
}



// NOTE(Momo): entry point
int main(int argc, char* argv[]) {
    // TODO(Momo): Test Thread Code
    /*thread_context ThreadContext[15];
    for (int i = 0; i < ArrayCount(ThreadContext); ++i) {
        thread_context * Context = ThreadContext + i;
        Context->Index = i;
        
        auto Thread = SDL_CreateThread(TestThread, "TestThread", Context);
        SDL_Log("Thread %d created", i);
        SDL_DetachThread(Thread);
    }
    
    PushString("1");*/
    
    
    
    SDL_Log("SDL initializing\n");
    if (SDL_Init(SDL_INIT_VIDEO) < 0 ) {
        SDL_Log("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }
    Defer{
        SDL_Log("SDL shutting down\n");
        SDL_Quit();
    };
    
    // NOTE(Momo): Create Window
    SDL_Log("SDL creating Window\n");
    SDL_Window* window = SDL_CreateWindow("Vigil", 
                                          SDL_WINDOWPOS_UNDEFINED, 
                                          SDL_WINDOWPOS_UNDEFINED, 
                                          1600, 
                                          900, 
                                          SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
    if (window == nullptr) {
        SDL_Log("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }
    Defer{
        SDL_Log("SDL destroying window\n");
        SDL_DestroyWindow(window);
    };
    
    // NOTE(Momo): Set OpenGL attributes
    SDL_GL_LoadLibrary(nullptr);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    
#if SLOW_MODE
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif  
    
    
    // NOTE(Momo): Load game code
    sdl_game_code GameCode;
    Load(&GameCode);
    
    // NOTE(Momo) Initialize OpenGL context  core)
    SDL_Log("SDL creating context\n");
    SDL_GLContext context = SDL_GL_CreateContext(window);
    if (context == nullptr) { 
        SDL_Log("Failed to create OpenGL context! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }
    Defer {
        SDL_Log("SDL deleting context\b");
        SDL_GL_DeleteContext(context);
    };
    
    gladLoadGLLoader(SDL_GL_GetProcAddress);
    
    SDL_Log("OpenGL loaded!\n");
    SDL_Log("[OpenGL] Vendor:   %s\n", glGetString(GL_VENDOR));
    SDL_Log("[OpenGL] Renderer: %s\n", glGetString(GL_RENDERER));
    SDL_Log("[OpenGL] Version:  %s\n", glGetString(GL_VERSION));
    
    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //glDisable(GL_DEPTH_TEST);
    
#ifdef SLOW_MODE
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(SdlGlDebugCallback, nullptr);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, true);
#endif
    
    auto [windowWidth, windowHeight] = SDLGetWindowSize(window);
    glViewport(0,0,windowWidth,windowHeight);
    glClearColor(0.0f, 0.3f, 0.3f, 0.0f);
    
    
    
    loaded_bitmap TestBitmap = DebugMakeBitmapFromBmp("assets/ryoji.bmp");
    Defer{ free(TestBitmap.Pixels); };
    
    GlRendererInit((f32)windowWidth, (f32)windowHeight,  1024, &TestBitmap);
    
    // NOTE(Momo): Game Init
    game_memory GameMemory = {};
    GameMemory.IsInitialized = false;
    GameMemory.PermanentStore = malloc(Megabytes(64));
    
    if ( !GameMemory.PermanentStore ) {
        SDL_Log("Cannot allocate for PermanentStore");
        return 1;
    }
    Defer { free(GameMemory.PermanentStore); };
    
    GameMemory.PermanentStoreSize = Megabytes(64);
    
    // NOTE(Momo): PlatformAPI
    GameMemory.PlatformApi.Log = PlatformLog;
    GameMemory.PlatformApi.GlProcessRenderGroup = GlProcessRenderGroup;
    
    sdl_timer timer;
    Start(&timer);
    
    // NOTE(Momo): Game Loop
    while(gIsRunning) {
        SDL_Event e;
        while(SDL_PollEvent(&e)) {
            switch(e.type) {
                case SDL_QUIT: {
                    gIsRunning = false;
                    SDL_Log("Quit triggered\n");
                } break; 
            }
        }
        u64 timeElapsed = TimeElapsed(&timer);
        f32 deltaTime = timeElapsed / 1000.f;
        
        
        GameCode.Update(&GameMemory, deltaTime); 
        
        // NOTE(Momo): Timer update
        Tick(&timer);
        //SDL_Log("%lld  ms\n", timeElapsed);
        SDL_GL_SwapWindow(window);
        
        
    }
    
    
    return 0;
    
}
