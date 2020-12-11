#include <stdlib.h>

#include "mm_core.h"
#include "mm_maths.h"
#include "mm_arena.h"

#include "platform.h"
#include "platform_sdl.h"
#include "platform_sdl_opengl.h"
#include "game_renderer_opengl.h"
#include "game_input.h"
#include "thirdparty/sdl2/include/SDL_video.h"


static const char* GameDllFilename = "game.dll";
static const char* TempGameDllFilename = "temp_game.dll"; 



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


f32 GetMsElapsed(u64 Start, u64 End) {
    return (End - Start)/(f32)SDL_GetPerformanceFrequency() * 1000.f;
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
   
    

    
    // NOTE(Momo): Load game code
    sdl_game_code GameCode;
    Load(&GameCode, GameDllFilename, TempGameDllFilename);
    

    // TODO: for now we'll just do OpenGL. We might expose this to DLL one day.
    sdl_api SdlApi = {};
    SdlApi.Load = SdlOpenglLoad;
    SdlApi.Unload = SdlOpenglUnload;
    SdlApi.Resize = SdlOpenglResize;
    SdlApi.Render = SdlOpenglRender;
    SdlApi.SwapBuffer = SdlOpenglSwapBuffer;

    // Creating the initial window
    sdl_context SdlContext;
    {
        SDL_DisplayMode DisplayMode = {};
        f32 Ddpi = 1, Hdpi = 1, Vdpi = 1;
        if (SDL_GetDesktopDisplayMode(0, &DisplayMode) != 0) {
           SDL_Log("Problems getting display mode: %s", SDL_GetError());
           return 1;
        }
        SDL_GetDisplayDPI(0, &Ddpi, &Hdpi, &Vdpi);
        
            
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
         
        SDL_Log("Creating %d x %d resolution window", WindowWidth, WindowHeight); 
        option<sdl_context> Op = SdlApi.Load(WindowWidth, WindowHeight);
        if( !Op ) {
            return 1;
        }
        SdlContext = Op.Item;
    }

    SDL_Log("Hello World"); 
    Defer { SdlApi.Unload(SdlContext); };

         
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
    PlatformApi.Log = PlatformLog;
    PlatformApi.ReadFile = PlatformReadFile;
    PlatformApi.GetFileSize = PlatformGetFileSize;
    
    
    // NOTE(Momo): Render commands/queue
    void* RenderCommandsMemory = PushBlock(&PlatformArena, RenderCommandsMemorySize);
    mailbox RenderCommands = Mailbox(RenderCommandsMemory, RenderCommandsMemorySize);
    
    // NOTE(Momo): Input
    game_input Input = {};
#if INTERNAL
    char DebugTextInputBuffer[10];
    Input.DebugTextInputBuffer = StringBuffer(DebugTextInputBuffer, 10);
#endif 
    
    // NOTE(Momo): Timestep related
    // TODO(Momo): What if we can't hit 60fps?
    SDL_DisplayMode DisplayMode = {};
    SDL_GetCurrentDisplayMode(SDL_GetWindowDisplayIndex(SdlContext.Window), &DisplayMode);
    u32 RefreshRate = DisplayMode.refresh_rate == 0 ? 60 : DisplayMode.refresh_rate;
    SDL_Log("Monitor Refresh Rate: %d", RefreshRate);

    f32 TimeStepMultiplier = 1.f;
    f32 TargetMsForFrame = 1.f/RefreshRate * 1000.f;

    // NOTE(Momo): Game Loop
    while(gIsRunning) {
        u64 StartCount = SDL_GetPerformanceCounter();
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
                        SdlApi.Resize(SdlContext.Renderer, e.window.data1, e.window.data2);
                        
                    }
                } break;
               
#if INTERNAL
                case SDL_TEXTINPUT: {
                    Push(&Input.DebugTextInputBuffer, String(e.text.text));
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
            GameCode.Update(&GameMemory, &PlatformApi, &RenderCommands, &Input, TargetMsForFrame/1000.f); 
        }
       
        SdlApi.Render(SdlContext.Renderer, &RenderCommands);

        u64 EndCount = SDL_GetPerformanceCounter();
        
        f32 CurrentMsForFrame = GetMsElapsed(StartCount, EndCount);
        if (TargetMsForFrame > CurrentMsForFrame) {
            SDL_Delay((Uint32)(TargetMsForFrame - CurrentMsForFrame)); // 60fps?
            f32 RemainingMsForFrame = CurrentMsForFrame - GetMsElapsed(EndCount, SDL_GetPerformanceCounter());
            if (RemainingMsForFrame > 0.f ){
                // Didn't sleep enough, so sleep some more?                
                SDL_Log("Target: %f, Remaining: %f", TargetMsForFrame, RemainingMsForFrame);
            }
            else {
                SDL_Log("Overslept?! Remaining: %f", RemainingMsForFrame);
            }
        }
        else {
            SDL_Log("Frame rate missed! Target: %f, Current: %f", TargetMsForFrame, CurrentMsForFrame);
        }

        SdlApi.SwapBuffer(SdlContext);
    }
    
    
    return 0;
    
}
