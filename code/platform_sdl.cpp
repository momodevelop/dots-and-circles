#include <stdlib.h>

#include "ryoji_maths.h"

#include "thirdparty/sdl2/include/SDL.h"
#include "thirdparty/glad/glad.c"

#include "bmp.cpp"

#include "interface.h"
#include "platform_sdl_timer.cpp"
#include "platform_sdl_gldebug.cpp"
#include "platform_sdl_game_code.cpp"
#include "platform_sdl_renderer_gl.cpp"

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

static
bool PlatformReadBinaryFileToMemory(void * dest, u64 destSize, const char * path) {
    SDL_RWops * file = SDL_RWFromFile(path, "rb");
    if (file == nullptr) {
        return false;
    }
    Defer{
        SDL_RWclose(file);
    };
    
    // Get file size
    u64 filesize = SDL_RWsize(file); // Does not include EOF
    
    if (filesize > destSize) {
        return false;
    }
    
    SDL_RWread(file, dest, 1, filesize);
    
    return true;
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


// TODO(Momo): Complete this function
static inline void
PlatformRenderOpenGL() {
    //Render(&GlRenderer);
}

// NOTE(Momo): entry point
int main(int argc, char* argv[]) {
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
    
    
    // Setup Textures
    // TODO(Momo): Probably move this to game code asset loading?
    Bmp bmp;
    if (auto err = Load(&bmp, "assets/ryoji.bmp"); err > 0) {
        SDL_Log("%s", BmpErrorStr(err));
        return 1;
    }
    SDL_Log("Bmp loaded");
    Defer{ Unload(&bmp); };
    
    gl_renderer GlRenderer;
    Init(&GlRenderer, (f32)windowWidth, (f32)windowHeight,  1024, &bmp);
    
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
    platform_api PlatformAPI = {};
    PlatformAPI.Log = PlatformLog;
    //PlatformAPI.Render = PlatformRenderOpenGL;
    
    
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
        
        
        GameCode.Update(&GameMemory, &PlatformAPI, deltaTime);
        
        Render(&GlRenderer);
        
        // NOTE(Momo): Timer update
        Tick(&timer);
        //SDL_Log("%lld  ms\n", timeElapsed);
        SDL_GL_SwapWindow(window);
        
        
    }
    
    
    return 0;
    
}
