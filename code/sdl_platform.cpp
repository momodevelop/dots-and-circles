#include <stdlib.h>

#include "ryoji_maths.h"
#include "ryoji_arenas.h"

#include "thirdparty/sdl2/include/SDL.h"
#include "thirdparty/glad/glad.c"


#include "game_platform.h"
#include "game_renderer_opengl.h"

#include "sdl_platform_timer.h"
#include "sdl_platform_gldebug.h"
#include "sdl_platform_utils.h"

// TODO(Momo): Shift for game settings?
#define gGameMemorySize Megabytes(64)
#define gRenderCommandsMemorySize Megabytes(64)
#define gTotalMemorySize Gigabytes(1)


// NOTE(Momo): sdl_game_code
struct sdl_game_code {
    game_update* Update;
};

static inline void
Unload(sdl_game_code* GameCode) {
    GameCode->Update = nullptr;
}

static inline bool
Load(sdl_game_code* GameCode)
{
    Unload(GameCode);
    
    void* GameCodeDLL = SDL_LoadObject("game.dll");
    if (!GameCodeDLL) {
        SDL_Log("Failed to open game.dll");
        return false;
    }
    
    GameCode->Update = (game_update*)SDL_LoadFunction(GameCodeDLL, "GameUpdate");
    
    return true;
}

static bool gIsRunning = true;

static inline
PLATFORM_LOG(PlatformLog) {
    va_list va;
    va_start(va, Format);
    SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO, Format, va);
    va_end(va);
}



static inline 
PLATFORM_READ_FILE(PlatformReadFile) {
    SDL_RWops * File = SDL_RWFromFile(path, "rb");
    if (File == nullptr) {
        return {};
    }
    Defer{ SDL_RWclose(File); };
    
    platform_read_file_result Ret = {};
    SDL_RWseek(File, 0, RW_SEEK_END);
    Ret.ContentSize = (u32)SDL_RWtell(File);
    SDL_RWseek(File, 0, RW_SEEK_SET);
    
    Ret.Content = malloc(Ret.ContentSize);
    SDL_RWread(File, Ret.Content, 1, Ret.ContentSize);
    return Ret;
}

static inline 
PLATFORM_FREE_FILE(PlatformFreeFile) {
    free(File.Content);
}

static inline void 
DebugPrintMatrix(m44f M) {
    SDL_Log("%02f %02f %02f %02f\n%02f %02f %02f %02f\n%02f %02f %02f %02f\n%02f %02f %02f %02f", 
            M[0], M[1], M[2], M[3],
            M[4], M[5], M[6], M[7],
            M[8], M[9], M[10], M[11],
            M[12], M[13], M[14], M[15]
            );
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
    
#ifdef SLOW_MODE
    glDebugMessageCallback(SdlGlDebugCallback, nullptr);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, true);
#endif
    
    renderer_opengl RendererOpenGL = {};
    auto [windowWidth, windowHeight] = SDLGetWindowSize(window);
    Init(&RendererOpenGL, windowWidth, windowHeight,  1024);
    
    void* Memory = malloc(gTotalMemorySize);
    if (Memory == nullptr ){
        SDL_Log("Cannot allocate memory");
        return 1;
    }
    Defer { free(Memory); };
    
    
    memory_arena MainMemory = {};
    Init(&MainMemory, Memory, gTotalMemorySize);
    
    
    // NOTE(Momo): Game Init
    game_memory GameMemory = {};
    GameMemory.IsInitialized = false;
    GameMemory.PermanentStore = Allocate(&MainMemory, gGameMemorySize, alignof(void*));
    GameMemory.PermanentStoreSize = gGameMemorySize;
    
    if ( !GameMemory.PermanentStore ) {
        SDL_Log("Cannot allocate for PermanentStore");
        return 1;
    }
    
    // NOTE(Momo): PlatformAPI
    platform_api PlatformApi;
    PlatformApi.Log = PlatformLog;
    PlatformApi.ReadFile = PlatformReadFile;
    PlatformApi.FreeFile = PlatformFreeFile;
    
    // NOTE(Momo): Timer
    sdl_timer timer;
    Start(&timer);
    
    // NOTE(Momo): Render commands/queue
    void* RenderCommandsMemory = Allocate(&MainMemory, gRenderCommandsMemorySize, alignof(void*));
    
    // NOTE(Momo): Input
    game_input GameInput = {};
    
    
    // NOTE(Momo): Game Loop
    while(gIsRunning) {
        Update(&GameInput);
        
        SDL_Event e;
        while(SDL_PollEvent(&e)) {
            
            
            switch(e.type) {
                case SDL_QUIT: {
                    gIsRunning = false;
                    SDL_Log("Quit triggered\n");
                } break; 
                
                // NOTE(Momo): Handle keyboard
                case SDL_KEYDOWN: {
                    switch(e.key.keysym.sym) {
                        case SDLK_w: {
                            GameInput.ButtonUp.Now = true; 
                        }break;
                        case SDLK_a: {
                            GameInput.ButtonLeft.Now = true;
                        }break;
                        case SDLK_s: {
                            GameInput.ButtonDown.Now = true;
                        }break;
                        case SDLK_d: {
                            GameInput.ButtonRight.Now = true;
                        }break;
                        case SDLK_RETURN: {
                            GameInput.ButtonConfirm.Now = true;
                        }break;
                        
                    }
                } break;
                case SDL_KEYUP: {
                    switch(e.key.keysym.sym) {
                        case SDLK_w: {
                            GameInput.ButtonUp.Now = false;
                        }break;
                        case SDLK_a: {
                            GameInput.ButtonLeft.Now = false;
                        }break;
                        case SDLK_s: {
                            GameInput.ButtonDown.Now = false;
                        }break;
                        case SDLK_d: {
                            GameInput.ButtonRight.Now = false;
                        }break;
                        case SDLK_RETURN: {
                            GameInput.ButtonConfirm.Now = false;
                        }break;
                    } break;
                }
            }
            
        }
        
        u64 timeElapsed = TimeElapsed(&timer);
        f32 deltaTime = timeElapsed / 1000.f;
        
        
        render_commands Commands;
        Init(&Commands, RenderCommandsMemory, gRenderCommandsMemorySize);
        
        GameCode.Update(&PlatformApi, &GameMemory, &Commands, &GameInput, deltaTime); 
        Render(&RendererOpenGL, &Commands); 
        
        
        // NOTE(Momo): Timer update
        Tick(&timer);
        SDL_Log("%lld  ms\n", timeElapsed);
        SDL_GL_SwapWindow(window);
    }
    
    
    return 0;
    
}
