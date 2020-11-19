#include <stdlib.h>


#include "mm_core.h"
#include "mm_maths.h"
#include "mm_arena.h"

#include "thirdparty/sdl2/include/SDL.h"
#include "thirdparty/glad/glad.c"

#include "platform.h"
#include "game_renderer_opengl.h"
#include "game_input.h"


static const char* GameDllFilename = "game.dll";
static const char* TempGameDllFilename = "temp_game.dll"; 


#if INTERNAL
static inline void
SdlGlDebugCallback(GLenum source,
                   GLenum type,
                   GLuint id,
                   GLenum severity,
                   GLsizei length,
                   const GLchar* msg,
                   const void* userParam) {
    // Ignore NOTIFICATION severity
    if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) 
        return;
    
    const char* _source;
    const char* _type;
    const char* _severity;
    switch (source) {
        case GL_DEBUG_SOURCE_API:
        _source = "API";
        break;
        
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
        _source = "WINDOW SYSTEM";
        break;
        
        case GL_DEBUG_SOURCE_SHADER_COMPILER:
        _source = "SHADER COMPILER";
        break;
        
        case GL_DEBUG_SOURCE_THIRD_PARTY:
        _source = "THIRD PARTY";
        break;
        
        case GL_DEBUG_SOURCE_APPLICATION:
        _source = "APPLICATION";
        break;
        
        case GL_DEBUG_SOURCE_OTHER:
        _source = "UNKNOWN";
        break;
        
        default:
        _source = "UNKNOWN";
        break;
    }
    
    switch (type) {
        case GL_DEBUG_TYPE_ERROR:
        _type = "ERROR";
        break;
        
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        _type = "DEPRECATED BEHAVIOR";
        break;
        
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        _type = "UDEFINED BEHAVIOR";
        break;
        
        case GL_DEBUG_TYPE_PORTABILITY:
        _type = "PORTABILITY";
        break;
        
        case GL_DEBUG_TYPE_PERFORMANCE:
        _type = "PERFORMANCE";
        break;
        
        case GL_DEBUG_TYPE_OTHER:
        _type = "OTHER";
        break;
        
        case GL_DEBUG_TYPE_MARKER:
        _type = "MARKER";
        break;
        
        default:
        _type = "UNKNOWN";
        break;
    }
    
    switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH:
        _severity = "HIGH";
        break;
        
        case GL_DEBUG_SEVERITY_MEDIUM:
        _severity = "MEDIUM";
        break;
        
        case GL_DEBUG_SEVERITY_LOW:
        //_severity = "LOW";
        return;
        
        case GL_DEBUG_SEVERITY_NOTIFICATION:
        //_severity = "NOTIFICATION";
        return;
        
        default:
        _severity = "UNKNOWN";
        break;
    }
    
    SDL_Log("[OpenGL] %d: %s of %s severity, raised from %s: %s\n",
            id, _type, _severity, _source, msg);
    
};
#endif

// TODO: This is INTERNAL only?
static inline b32
CopyFile(const char* DestFilename, const char* SrcFilename) {
    SDL_RWops* DestFile = SDL_RWFromFile(DestFilename, "wb");
    SDL_RWops* SrcFile = SDL_RWFromFile(SrcFilename, "rb");
    if (DestFile == nullptr) {
        SDL_Log("Cannot open DestFilename: %s", DestFilename);
        return false;
    }
    Defer { SDL_RWclose(DestFile); };
    if (SrcFile == nullptr) {
        SDL_Log("Cannot open SrcFilename: %s", SrcFilename);
        return false;
    }
    Defer {SDL_RWclose(SrcFile); };

    SDL_RWseek(SrcFile, 0, RW_SEEK_END);
    auto SrcFilesize = SDL_RWtell(SrcFile);
    SDL_RWseek(SrcFile, 0, RW_SEEK_SET);

    void* FileMemory = malloc(SrcFilesize);    
    Defer { free(FileMemory); };

    SDL_RWread(SrcFile, FileMemory, 1, SrcFilesize);
    SDL_RWwrite(DestFile, FileMemory, 1, SrcFilesize);

    return true;
}

// Timer
struct sdl_timer {
    u64 CountFrequency;
    u64 PrevFrameCounter;
    u64 EndFrameCounter;
    u64 CountsElapsed;
};



static inline 
void Start(sdl_timer* Timer) {
    Timer->CountFrequency = SDL_GetPerformanceFrequency();
    Timer->PrevFrameCounter = SDL_GetPerformanceCounter();
    Timer->EndFrameCounter = 0;
    Timer->CountsElapsed = 0;
}


static inline 
u64 GetTicksElapsed(sdl_timer* Timer) {
    Timer->EndFrameCounter = SDL_GetPerformanceCounter();
    Timer->CountsElapsed = Timer->EndFrameCounter - Timer->PrevFrameCounter;
    
    Timer->PrevFrameCounter = Timer->EndFrameCounter; 
    
    // NOTE(Momo): 
    // PerformanceCounter(C) gives how many count has elapsed.
    // PerformanceFrequency(F) gives how many counts/second.
    // Thus: seconds = C / F, and milliseconds = seconds * 1000
    return (1000 * Timer->CountsElapsed) / Timer->CountFrequency;
}


// NOTE(Momo): sdl_game_code
struct sdl_game_code {
    void* DLL; 
    game_update* Update;
};

static inline void
Unload(sdl_game_code* GameCode) {
    SDL_UnloadObject(GameCode->DLL);
    GameCode->Update = nullptr;
}

static inline b32
Load(sdl_game_code* GameCode, const char* SrcDllFilename, const char* TempDllFilename)
{    
    // TODO: Check src file last written time?
    if (!CopyFile(TempDllFilename, SrcDllFilename)) {
        return false;
    }

    GameCode->DLL = SDL_LoadObject(TempDllFilename);
    if (!GameCode->DLL) {
        SDL_Log("Failed to open %s", TempDllFilename);
        return false;
    }
    
    GameCode->Update = (game_update*)SDL_LoadFunction(GameCode->DLL, "GameUpdate");
    
    return true;
}




static bool gIsRunning = true;
static bool gIsPaused = false;
constexpr u64 ScreenTicks60FPS = 1000/60;
constexpr u64 ScreenTicks30FPS = 1000/30;



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
                                          SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
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
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    //SDL_GL_SetSwapInterval(1);    
#if INTERNAL
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif  
    
    
    // NOTE(Momo): Load game code
    sdl_game_code GameCode;
    Load(&GameCode, GameDllFilename, TempGameDllFilename);
    
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
    i32 windowWidth, windowHeight;
    SDL_GetWindowSize(window, &windowWidth, &windowHeight);

    renderer_opengl RendererOpenGL = {};
    Init(&RendererOpenGL, windowWidth, windowHeight, 1000000);
    
    void* ProgramMemory = calloc(TotalMemorySize, sizeof(u8));
    if (ProgramMemory == nullptr){
        SDL_Log("Cannot allocate memory");
        return 1;
    }
    Defer { free(ProgramMemory); };
    
    // NOTE(Momo): Memory Arena for platform
    mmarn_arena PlatformArena = mmarn_Arena(ProgramMemory, TotalMemorySize);
    
    // NOTE(Momo): Game
    game_memory GameMemory = {};
    GameMemory.MainMemory = mmarn_PushBlock(&PlatformArena, GameMainMemorySize);
    GameMemory.MainMemorySize = GameMainMemorySize;

#if INTERNAL
    GameMemory.DebugMemory = mmarn_PushBlock(&PlatformArena, DebugMemorySize);
    GameMemory.DebugMemorySize = DebugMemorySize;
#endif
    
    if ( !GameMemory.MainMemory ) {
        SDL_Log("Cannot allocate game memory");
        return 1;
    }

#if INTERNAL
    if ( !GameMemory.DebugMemory) {
        SDL_Log("Cannot allocate debug memory");
        return 1;
    }
#endif
    
    // NOTE(Momo): PlatformAPI
    platform_api PlatformApi;
    PlatformApi.Log = PlatformLog;
    PlatformApi.ReadFile = PlatformReadFile;
    PlatformApi.GetFileSize = PlatformGetFileSize;
    
    
    // NOTE(Momo): Render commands/queue
    void* RenderCommandsMemory = mmarn_PushBlock(&PlatformArena, RenderCommandsMemorySize);
    mmcmd_commands RenderCommands = mmcmd_Commands(RenderCommandsMemory, RenderCommandsMemorySize);
    
    // NOTE(Momo): Input
    game_input Input = {};
#if INTERNAL
    char DebugTextInputBuffer[10];
    Input.DebugTextInputBuffer = mms_StringBuffer(DebugTextInputBuffer, 10);
#endif 
    
    // NOTE(Momo): Timestep related
    // TODO(Momo): What if we can't hit 60fps?
    f32 TimeStepMultiplier = 1.f;
    u64 TargetTicksElapsed = 16; // 60FPS
    f32 TargetDeltaTime = TargetTicksElapsed / 1000.f;
    u64 ActualTicksElapsed = 0;
    // NOTE(Momo): Timer
    sdl_timer timer;
    Start(&timer);
    
    b32 IsLeftShiftDown = false;
    // NOTE(Momo): Game Loop
    while(gIsRunning) {
        Update(&Input);
        
        SDL_Event e;
        while(SDL_PollEvent(&e)) {
            switch(e.type) {
                case SDL_QUIT: {
                    gIsRunning = false;
                    SDL_Log("Quit triggered\n");
                } break; 
                case SDL_WINDOWEVENT: {
                    if (e.window.event == SDL_WINDOWEVENT_RESIZED) {
                        SDL_Log("Resizing: %d %d", e.window.data1, e.window.data2);
                        SetWindowResolution(&RendererOpenGL, e.window.data1, e.window.data2);
                        
                    }
                } break;
               
#if INTERNAL
                case SDL_TEXTINPUT: {
                    mms_Concat(&Input.DebugTextInputBuffer, mms_String(e.text.text));
                } break;
#endif                

                // NOTE(Momo): Handle keyboard
                case SDL_KEYDOWN: {
                    switch(e.key.keysym.sym) {
                        case SDLK_w: {
                            Input.ButtonUp.Now = true; 
                        }break;
                        case SDLK_a: {
                            Input.ButtonLeft.Now = true;
                        }break;
                        case SDLK_s: {
                            Input.ButtonDown.Now = true;
                        }break;
                        case SDLK_d: {
                            Input.ButtonRight.Now = true;
                        }break;
                        case SDLK_RETURN: {
                            Input.ButtonConfirm.Now = true;
                        }break;
                        case SDLK_SPACE: {
                            Input.ButtonSwitch.Now = true;
                        }break;
                    }
                    // Cries
#if INTERNAL
                    switch(e.key.keysym.sym) {
                        case SDLK_F1:
                            Input.DebugKeys[GameDebugKey_F1].Now = true;
                            break;
                        case SDLK_F2:
                            Input.DebugKeys[GameDebugKey_F2].Now = true;
                            break;
                        case SDLK_F3:
                            Input.DebugKeys[GameDebugKey_F3].Now = true;
                            break;
                        case SDLK_F4:
                            Input.DebugKeys[GameDebugKey_F4].Now = true;
                            break;
                        case SDLK_F5:
                            Input.DebugKeys[GameDebugKey_F5].Now = true;
                            break;
                        case SDLK_F6:
                            Input.DebugKeys[GameDebugKey_F6].Now = true;
                            break;
                        case SDLK_F7:
                            Input.DebugKeys[GameDebugKey_F7].Now = true;
                            break;
                        case SDLK_F8:
                            Input.DebugKeys[GameDebugKey_F8].Now = true;
                            break;
                        case SDLK_F9:
                            Input.DebugKeys[GameDebugKey_F9].Now = true;
                            break;
                        case SDLK_F10:
                            Input.DebugKeys[GameDebugKey_F10].Now = true;
                            break;
                        case SDLK_F11:
                            Input.DebugKeys[GameDebugKey_F11].Now = true;
                            break;
                        case SDLK_F12:
                            Input.DebugKeys[GameDebugKey_F12].Now = true;
                            SDL_Log("Reloading game code...");
                            
                            // TODO: Shift this to game loop?
                            Unload(&GameCode);
                            Load(&GameCode, GameDllFilename, TempGameDllFilename);
                            break;
                        case SDLK_RETURN:
                            Input.DebugKeys[GameDebugKey_Return].Now = true;
                            break;
                        case SDLK_BACKSPACE:
                            Input.DebugKeys[GameDebugKey_Backspace].Now = true;
                            break;
                    }
#endif
                } break;
                case SDL_KEYUP: {
                    switch(e.key.keysym.sym) {
                        case SDLK_w: {
                            Input.ButtonUp.Now = false;
                        }break;
                        case SDLK_a: {
                            Input.ButtonLeft.Now = false;
                        }break;
                        case SDLK_s: {
                            Input.ButtonDown.Now = false;
                        }break;
                        case SDLK_d: {
                            Input.ButtonRight.Now = false;
                        }break;
                        case SDLK_RETURN: {
                            Input.ButtonConfirm.Now = false;
                        }break;
                        case SDLK_SPACE: {
                            Input.ButtonSwitch.Now = false;
                        }break;
                    }
#if INTERNAL
                    switch(e.key.keysym.sym) {
                        case SDLK_F1: {
                            Input.DebugKeys[GameDebugKey_F1].Now = false;
                        } break;
                        case SDLK_F2:
                            Input.DebugKeys[GameDebugKey_F2].Now = false;
                            break;
                        case SDLK_F3:
                            Input.DebugKeys[GameDebugKey_F3].Now = false;
                            break;
                        case SDLK_F4:
                            Input.DebugKeys[GameDebugKey_F4].Now = false;
                            break;
                        case SDLK_F5:
                            Input.DebugKeys[GameDebugKey_F5].Now = false;
                            break;
                        case SDLK_F6:
                            Input.DebugKeys[GameDebugKey_F6].Now = false;
                            break;
                        case SDLK_F7:
                            Input.DebugKeys[GameDebugKey_F7].Now = false;
                            break;
                        case SDLK_F8:
                            Input.DebugKeys[GameDebugKey_F8].Now = false;
                            break;
                        case SDLK_F9:
                            Input.DebugKeys[GameDebugKey_F9].Now = false;
                            break;
                        case SDLK_F10:
                            Input.DebugKeys[GameDebugKey_F10].Now = false;
                            break;
                        case SDLK_F11:
                            Input.DebugKeys[GameDebugKey_F11].Now = false;
                            break;
                        case SDLK_F12:
                            Input.DebugKeys[GameDebugKey_F12].Now = false;
                            break;
                        case SDLK_RETURN:
                            Input.DebugKeys[GameDebugKey_Return].Now = false;
                            break; 
                        case SDLK_BACKSPACE:
                            Input.DebugKeys[GameDebugKey_Backspace].Now = false;
                            break;

                    }
#endif

                }
            }
            
        }
        
        
        if (GameCode.Update) {
            GameCode.Update(&GameMemory, &PlatformApi, &RenderCommands, &Input, TargetDeltaTime, ActualTicksElapsed); 
        }
       
        Render(&RendererOpenGL, RenderCommands); 
        mmcmd_Clear(&RenderCommands);
        
        // NOTE(Momo): Timer update
        SDL_GL_SwapWindow(window);

        ActualTicksElapsed = GetTicksElapsed(&timer);
        if (TargetTicksElapsed > ActualTicksElapsed) {
            SDL_Delay((Uint32)(TargetTicksElapsed - ActualTicksElapsed)); // 60fps?
        }
        
//        SDL_Log("%lld vs %lld  ms\n", TargetTicksElapsed, ActualTicksElapsed);
    }
    
    
    return 0;
    
}
