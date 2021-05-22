#include "thirdparty/sdl2/include/SDL_timer.h"
#if _WIN32
#pragma comment(lib, "User32.lib")
#include <windows.h>
#include <ShellScalingAPI.h>
#endif


#include <stdlib.h>

#include "mm_core.h"
#include "mm_maths.h"
#include "mm_arena.h"

#include "game_opengl.h"
#include "game_platform.h"
#include "thirdparty/sdl2/include/SDL.h"
#include "thirdparty/glad/glad.c"


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



static inline b8 
SdlCopyFile(const char* DestFilename, const char* SrcFilename) {
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

// NOTE(Momo): sdl_game_code
struct sdl_game_code {
    void* DLL; 
    game_update* Update;
};

static inline void
SdlUnloadGameCode(sdl_game_code* GameCode) {
    SDL_UnloadObject(GameCode->DLL);
    GameCode->Update = nullptr;
}

static inline b8
SdlLoadGameCode(sdl_game_code* GameCode, 
        const char* SrcDllFilename, 
        const char* TempDllFilename)
{    
    // TODO: Check src file last written time?
    if (!SdlCopyFile(TempDllFilename, SrcDllFilename)) {
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

// NOTE(Momo): Platform API code
static inline void
SdlLog(const char* Format, ...) {
    va_list va;
    va_start(va, Format);
    SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO, Format, va);
    va_end(va);
}

static inline u32
SdlGetFileSize(const char* Path) {
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

static inline b8
SdlReadFile(void* Dest, u32 DestSize, const char* Path) {
    SDL_RWops * File = SDL_RWFromFile(Path, "rb");
    if (File == nullptr) {
        return false;
    }
    Defer{ SDL_RWclose(File); };
    SDL_RWread(File, Dest, 1, DestSize);
    return true;
}


f32 SdlGetMsElapsed(u64 Start, u64 End) {
    return (End - Start)/(f32)SDL_GetPerformanceFrequency() * 1000.f;
}



// NOTE(Momo): entry point
int main(int argc, char* argv[]) {

#if _WIN32
    // https://seabird.handmade.network/blogs/p/2460-be_aware_of_high_dpi
    // Handling high dpi
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
#endif 

    SDL_Log("SDL initializing\n");
    if (SDL_Init(SDL_INIT_VIDEO) < 0 ) {
        SDL_Log("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }
    Defer{
        SDL_Log("SDL shutting down\n");
        SDL_Quit();
    }; 

    SDL_GL_LoadLibrary(nullptr);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
#if INTERNAL
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif  
    
    SDL_DisplayMode DisplayMode = {};
    if (SDL_GetDesktopDisplayMode(0, &DisplayMode) != 0) {
       SDL_Log("Problems getting display mode: %s", SDL_GetError());
       return 1;
    }
        
    u32 RefreshRate = DisplayMode.refresh_rate == 0 ? 60 : DisplayMode.refresh_rate;
    SDL_Log("Monitor Refresh Rate: %d", RefreshRate);

    f32 TargetMsPerFrame = 1.f/RefreshRate * 1000.f;
    SDL_Log("Target Ms Per Frame: %.2f", TargetMsPerFrame);

    SDL_Log("Resolution detected: %d x %d", DisplayMode.w, DisplayMode.h);
    u32 WindowWidth = 0, WindowHeight = 0;
    if constexpr(DesignWidth > DesignHeight) { 
        if (DisplayMode.w >= DesignWidth) {
            WindowWidth = (u32)DesignWidth;
            WindowHeight = (u32)DesignHeight;
        }

        // If we can't fit the width, then we need to 'try our best'.
        else {
            if (DisplayMode.w > DisplayMode.h) {
                // If the resolution is too small, guess a size will show the entire window.
                // For now, the guess is 85% of the height.
                f32 DesignAspect = DesignWidth / DesignHeight;
                WindowHeight = (u32)(DisplayMode.h - (DisplayMode.h * 0.15f));
                WindowWidth = (u32)(WindowHeight * DesignAspect);
            }
            else {
                f32 InvDesignAspect = DesignHeight / DesignWidth;
                WindowWidth = (u32)(DisplayMode.w - (DisplayMode.w * 0.15f));
                WindowHeight = (u32)(WindowWidth * InvDesignAspect);
            }
        }
   
    }
    else {
        // TODO
        SDL_Log("Not implemented");
        return 1;
    }

    if (WindowWidth == 0 || WindowHeight == 0) {
        SDL_Log("Invalid resolution: %d x %d", WindowWidth, WindowHeight);
        return 1;
    }
     
    // Create Window
    SDL_Log("Creating %d x %d resolution window", WindowWidth, WindowHeight); 
    SDL_Window* Window = SDL_CreateWindow("Vigil", 
                                          SDL_WINDOWPOS_UNDEFINED, 
                                          SDL_WINDOWPOS_UNDEFINED, 
                                          WindowWidth, 
                                          WindowHeight, 
                                          SDL_WINDOW_SHOWN | 
                                          SDL_WINDOW_OPENGL | 
                                          SDL_WINDOW_RESIZABLE | 
                                          SDL_WINDOW_ALLOW_HIGHDPI);
 
    if (Window == nullptr) {
        SDL_Log("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }
    Defer { SDL_DestroyWindow(Window); };


    // Opengl Context
    SDL_Log("Creating Opengl context");
    SDL_GLContext OpenglContext = SDL_GL_CreateContext(Window);
    if (OpenglContext == nullptr) { 
        SDL_Log("Failed to create OpenGL context! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }
    Defer { SDL_GL_DeleteContext(OpenglContext); };

    gladLoadGLLoader(SDL_GL_GetProcAddress);

    // VSync
    SDL_Log("[OpenGL] Loaded!\n");
    if (SDL_GL_SetSwapInterval(-1) < 0) {
        if (SDL_GL_SetSwapInterval(1) < 0) {
            SDL_GL_SetSwapInterval(0);
            SDL_Log("[OpenGL] No VSync Enabled");
        }
        else {
            SDL_Log("[OpenGL] Normal VSync Enabled");
        }
    }
    else {
        SDL_Log("[OpenGL] Adaptive VSync Enabled");
    }

    SDL_Log("[OpenGL] Vendor:   %s\n", glGetString(GL_VENDOR));
    SDL_Log("[OpenGL] Renderer: %s\n", glGetString(GL_RENDERER));
    SDL_Log("[OpenGL] Version:  %s\n", glGetString(GL_VERSION));
    
#ifdef INTERNAL
    glDebugMessageCallback(SdlGlDebugCallback, nullptr);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, true);
#endif

    // Renderer
    opengl Renderer = {};
    Init(&Renderer, WindowWidth, WindowHeight, 10000);
    

    // Load game code
    sdl_game_code GameCode = {};
    SdlLoadGameCode(&GameCode, GameDllFilename, TempGameDllFilename);
   

    // Allocation of memory
    void* ProgramMemory = calloc(TotalMemorySize, sizeof(u8));
    if (ProgramMemory == nullptr){
        SDL_Log("Cannot allocate memory");
        return 1;
    }
    Defer { free(ProgramMemory); };
    
    // NOTE(Momo): Memory Arena for platform
    arena PlatformArena = Arena(ProgramMemory, TotalMemorySize);
    
    // NOTE(Momo): Game
    game_memory GameMemory = {};
    GameMemory.MainMemory = PushBlock(&PlatformArena, GameMainMemorySize);
    GameMemory.MainMemorySize = GameMainMemorySize;

    if ( !GameMemory.MainMemory ) {
        SDL_Log("Cannot allocate game memory");
        return 1;
    }

#if INTERNAL
    GameMemory.DebugMemory = PushBlock(&PlatformArena, DebugMemorySize);
    GameMemory.DebugMemorySize = DebugMemorySize; 
    if ( !GameMemory.DebugMemory) {
        SDL_Log("Cannot allocate debug memory");
        return 1;
    }

#endif
    
    // NOTE(Momo): PlatformAPI
    platform_api PlatformApi;
    PlatformApi.Log = SdlLog;
    PlatformApi.ReadFile = SdlReadFile;
    PlatformApi.GetFileSize = SdlGetFileSize;
    
    
    // NOTE(Momo): Render commands/queue
    void* RenderCommandsMemory = PushBlock(&PlatformArena, RenderCommandsMemorySize);
    mailbox RenderCommands = Mailbox(RenderCommandsMemory, RenderCommandsMemorySize);
    
    // NOTE(Momo): Input
    platform_input Input = {};
#if INTERNAL
    char DebugTextInputBuffer[10];
    Input.DebugTextInputBuffer = StringBuffer(DebugTextInputBuffer, 10);
#endif 
    
    // NOTE(Momo): Timestep related
    // TODO(Momo): What if we can't hit 60fps?

    // NOTE(Momo): Game Loop
    b8 IsRunning = true;
    u64 LastCounter = SDL_GetPerformanceCounter();
    f32 TimeStepMultiplier = 1.f;
    while(IsRunning) {
                
        Update(&Input);

        SDL_Event Event;
        while(SDL_PollEvent(&Event)) {
            switch(Event.type) {
                case SDL_QUIT: {
                    IsRunning = false;
                    SDL_Log("Quit triggered\n");
                } break; 
                case SDL_WINDOWEVENT: {
                    switch(Event.window.event) {
                        case SDL_WINDOWEVENT_RESIZED: {
                            SDL_Log("Resizing: %d %d", Event.window.data1, Event.window.data2);
                            OpenglResize(&Renderer, Event.window.data1, Event.window.data2);
                        } break;
                    }
                } break;
               
#if INTERNAL
                case SDL_TEXTINPUT: {
                    Push(&Input.DebugTextInputBuffer, String(Event.text.text));
                } break;
#endif                

                // NOTE(Momo): Handle keyboard
                case SDL_KEYDOWN: 
                case SDL_KEYUP: 
                {
                    SDL_Keycode KeyCode = Event.key.keysym.sym;
                    b8 IsDown = (Event.key.state == SDL_PRESSED);
                    switch(KeyCode) {
                        case SDLK_w: {
                            Input.ButtonUp.Now = IsDown; 
                        }break;
                        case SDLK_a: {
                            Input.ButtonLeft.Now = IsDown;
                        }break;
                        case SDLK_s: {
                            Input.ButtonDown.Now = IsDown;
                        }break;
                        case SDLK_d: {
                            Input.ButtonRight.Now = IsDown;
                        }break;
                        case SDLK_RETURN: {
                            Input.ButtonConfirm.Now = IsDown;
                        }break;
                        case SDLK_SPACE: {
                            Input.ButtonSwitch.Now = IsDown;
                        }break;
                    }
#if INTERNAL
                    switch(KeyCode) {
                        case SDLK_F1:
                            Input.DebugKeys[GameDebugKey_F1].Now = IsDown;
                            break;
                        case SDLK_F2:
                            Input.DebugKeys[GameDebugKey_F2].Now = IsDown;
                            break;
                        case SDLK_F3:
                            Input.DebugKeys[GameDebugKey_F3].Now = IsDown;
                            break;
                        case SDLK_F4:
                            Input.DebugKeys[GameDebugKey_F4].Now = IsDown;
                            break;
                        case SDLK_F5:
                            Input.DebugKeys[GameDebugKey_F5].Now = IsDown;
                            break;
                        case SDLK_F6:
                            Input.DebugKeys[GameDebugKey_F6].Now = IsDown;
                            break;
                        case SDLK_F7:
                            Input.DebugKeys[GameDebugKey_F7].Now = IsDown;
                            break;
                        case SDLK_F8:
                            Input.DebugKeys[GameDebugKey_F8].Now = IsDown;
                            break;
                        case SDLK_F9:
                            Input.DebugKeys[GameDebugKey_F9].Now = IsDown;
                            break;
                        case SDLK_F10:
                            Input.DebugKeys[GameDebugKey_F10].Now = IsDown;
                            break;
                        case SDLK_F11:
                            Input.DebugKeys[GameDebugKey_F11].Now = IsDown;
                            break;
                        case SDLK_F12:
                            Input.DebugKeys[GameDebugKey_F12].Now = IsDown;
                            SDL_Log("Reloading game code...");
                            
                            // TODO: Shift this to game loop?
                            SdlUnloadGameCode(&GameCode);
                            SdlLoadGameCode(&GameCode, GameDllFilename, TempGameDllFilename);
                            break;
                        case SDLK_RETURN:
                            Input.DebugKeys[GameDebugKey_Return].Now = IsDown;
                            break;
                        case SDLK_BACKSPACE:
                            Input.DebugKeys[GameDebugKey_Backspace].Now = IsDown;
                            break;
                    }
#endif
                } break;

            }
            
        }
        
       

        if (GameCode.Update) {
            GameCode.Update(&GameMemory, &PlatformApi, &RenderCommands, &Input, TargetMsPerFrame/1000.f); 
        }
       
        OpenglRender(&Renderer, &RenderCommands);
        Clear(&RenderCommands);

        if (TargetMsPerFrame > SdlGetMsElapsed(LastCounter, SDL_GetPerformanceCounter())) {

            // -1 because there is a chance we might overshoot the frame...
            f32 TimeToSleep = TargetMsPerFrame - SdlGetMsElapsed(LastCounter, SDL_GetPerformanceCounter()) - 1;
            if (TimeToSleep > 0) {
                SDL_Delay((s32)TimeToSleep);
            }
            //Assert(SdlGetMsElapsed(LastCounter, SDL_GetPerformanceCounter()) < TargetMsPerFrame);
            while(SdlGetMsElapsed(LastCounter, SDL_GetPerformanceCounter()) < TargetMsPerFrame);
        }
        else {
            SDL_Log("Frame rate missed!");
        }

        LastCounter = SDL_GetPerformanceCounter();
        SDL_GL_SwapWindow(Window);


    }
    
    
    return 0;
    
}
