#include <stdlib.h>

#include "ryoji_maths.h"
#include "ryoji_arenas.h"

#include "thirdparty/sdl2/include/SDL.h"
#include "thirdparty/glad/glad.c"

#include "game_platform.h"
#include "ryoji_renderer_opengl.h"
#include "game_input.h"

#include "sdl_platform_timer.h"
#include "sdl_platform_gldebug.h"
#include "sdl_platform_utils.h"


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

// NOTE(Momo): Platform API code
static inline void
PlatformLog(const char* Format, ...) {
    va_list va;
    va_start(va, Format);
    SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO, Format, va);
    va_end(va);
}

static inline u32
PlatformGetFileSize(const char* Path) {
    SDL_RWops * File = SDL_RWFromFile(Path, "rb");
    if (File == nullptr) {
        return {};
    }
    Defer{ SDL_RWclose(File); };
    
    u32 Ret = 0;
    SDL_RWseek(File, 0, RW_SEEK_END);
    Ret = (u32)SDL_RWtell(File);
    SDL_RWseek(File, 0, RW_SEEK_SET);
    
    return Ret;
}

static inline void
PlatformReadFile(void* Dest, u32 DestSize, const char* Path) {
    SDL_RWops * File = SDL_RWFromFile(Path, "rb");
    if (File == nullptr) {
        return;
    }
    Defer{ SDL_RWclose(File); };
    SDL_RWread(File, Dest, 1, DestSize);
}


// NOTE(Momo): entry point
int main(int argc, char* argv[]) {
    
    SDL_Log("SDL initializing\n");
    if (SDL_Init(SDL_INIT_VIDEO) < 0 ) {
        SDL_Log("SDL could not initialize! SDL_Errgor: %s\n", SDL_GetError());
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
    
#if INTERNAL
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
    
#ifdef INTERNAL
    glDebugMessageCallback(SdlGlDebugCallback, nullptr);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, true);
#endif
    
    // NOTE(Momo): Renderer Init
    auto [windowWidth, windowHeight] = SDLGetWindowSize(window);
    renderer_opengl RendererOpenGL = {};
    Init(&RendererOpenGL, windowWidth, windowHeight, 1000000);
    
    void* ProgramMemory = malloc(TotalMemorySize);
    if (ProgramMemory == nullptr){
        SDL_Log("Cannot allocate memory");
        return 1;
    }
    Defer { free(ProgramMemory); };
    
    // NOTE(Momo): Memory Arena for platform
    memory_arena PlatformArena = {};
    Init(&PlatformArena, ProgramMemory, TotalMemorySize);
    
    // NOTE(Momo): Game Init
    game_memory GameMemory = {};
    GameMemory.MainMemory = PushBlock(&PlatformArena, GameMainMemorySize);
    GameMemory.MainMemorySize = GameMainMemorySize;
    
    if ( !GameMemory.MainMemory ) {
        SDL_Log("Cannot allocate game memory");
        return 1;
    }
    
    // NOTE(Momo): PlatformAPI
    platform_api PlatformApi;
    PlatformApi.Log = PlatformLog;
    PlatformApi.ReadFile = PlatformReadFile;
    PlatformApi.GetFileSize = PlatformGetFileSize;
    
    // NOTE(Momo): Timer
    sdl_timer timer;
    Start(&timer);
    
    // NOTE(Momo): Render commands/queue
    void* RenderCommandsMemory = PushBlock(&PlatformArena, RenderCommandsMemorySize);
    render_command_queue RenderCommands = {}; 
    Init(&RenderCommands, RenderCommandsMemory, RenderCommandsMemorySize);
    
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
        
        u64 TimeElapsed = GetTimeElapsed(&timer);
        f32 DeltaTime = TimeElapsed / 1000.f;
        
        
        if (GameCode.Update) {
            GameCode.Update(&GameMemory, &PlatformApi, &RenderCommands, &GameInput, DeltaTime); 
        }
        
        Render(&RendererOpenGL, &RenderCommands); 
        Clear(&RenderCommands);
        
        
        // NOTE(Momo): Timer update
        Tick(&timer);
        SDL_Log("%lld  ms\n", TimeElapsed);
        SDL_GL_SwapWindow(window);
    }
    
    
    return 0;
    
}
