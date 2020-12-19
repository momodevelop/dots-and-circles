#include <windows.h>
#include <ShellScalingAPI.h>

#include "mm_core.h"
#include "mm_maths.h"
#include "mm_string.h"
#include "platform.h"
#include "game_input.h"

#define STB_SPRINTF_IMPLEMENTATION
#include "stb_sprintf.h"

#if INTERNAL
HANDLE GlobalStdOut;
static inline void
Win32WriteConsole(const char* Message) {
    WriteConsoleA(GlobalStdOut, Message, SiStrLen(Message), 0, NULL);
}
#endif

static inline void 
Win32Log(const char* Message, ...) {
    char Buffer[256];

    va_list VaList;
    va_start(VaList, Message);
   
    // TODO: Change to stb_sprintf
    stbsp_vsprintf(Buffer, Message, VaList);

#if INTERNAL
    Win32WriteConsole(Buffer);
#endif
    // TODO: Logging to text file?

    va_end(VaList);

}

struct win32_state {
    char ExeFullPath[MAX_PATH];
    char* OnePastLastExePathSlash;
};

static inline void
Win32BuildExePath(win32_state* State) {
    GetModuleFileNameA(0, State->ExeFullPath, sizeof(State->ExeFullPath));

    char* OnePastLastExePathSlash;
    for( char* Itr = State->ExeFullPath; *Itr; ++Itr) {
        if (*Itr == '\\') {
            OnePastLastExePathSlash = Itr + 1;
        }
    }
}

static inline void
Win32BuildExePathFilename(win32_state* State, char* Dest, const char* Filename) {
    for(const char *Itr = State->ExeFullPath; 
        Itr != State->OnePastLastExePathSlash; 
        ++Itr, ++Dest) 
    {
        (*Dest) = (*Itr);
    }
    
    for (const char* Itr = Filename;
         (*Itr) != 0;
         ++Itr, ++Dest) 
    {
        (*Dest) = (*Filename);
    }

    (*Dest) = 0;
}


struct win32_game_code {
    HMODULE Dll;
    game_update* GameUpdate;
    FILETIME LastWriteTime;
    b32 IsValid;
};


static inline FILETIME 
Win32GetLastWriteTime(const char* Filename) {
    WIN32_FILE_ATTRIBUTE_DATA Data;
    FILETIME LastWriteTime = {};

    if(GetFileAttributesEx(Filename, GetFileExInfoStandard, &Data)) {
        LastWriteTime = Data.ftLastWriteTime;
    }
    return LastWriteTime; 
}


static inline win32_game_code
Win32LoadGameCode(const char* SourceDllFilename,
                  const char* TempDllFilename,
                  const char* LockFilename) 
{
    win32_game_code Ret = {};
    WIN32_FILE_ATTRIBUTE_DATA Ignored; 
    if(!GetFileAttributesEx(LockFilename, GetFileExInfoStandard, &Ignored)) {
        Ret.LastWriteTime = Win32GetLastWriteTime(SourceDllFilename);
        CopyFile(SourceDllFilename, TempDllFilename, FALSE);    
        Ret.Dll = LoadLibraryA(TempDllFilename);
        if(Ret.Dll) {
            Ret.GameUpdate = (game_update*)GetProcAddress(Ret.Dll, "GameUpdate");
            Ret.IsValid = Ret.GameUpdate != 0;
        }
    }
    return Ret;
}

static inline void 
Win32UnloadGameCode(win32_game_code* GameCode) {
    if (GameCode->Dll) {
        FreeLibrary(GameCode->Dll);
        GameCode->Dll = 0;
    }
    GameCode->IsValid = false;
    GameCode->GameUpdate = 0;
}

LRESULT CALLBACK
Win32WindowCallback(HWND Window, 
                    UINT Message, 
                    WPARAM WParam,
                    LPARAM LParam) 
{
    LRESULT Result = 0;
    switch(Message) {
        case WM_CLOSE: {
        } break;
        case WM_SETCURSOR: {
        } break;
        case WM_DESTROY: {
            // Error?
        } break;
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYDOWN:
        case WM_KEYUP: {
        } break;
        case WM_PAINT: {
        } break;
        default: {
            // Log message?
            Result = DefWindowProcA(Window, Message, WParam, LParam);
        };   
    }
    return Result;
}


int CALLBACK
WinMain(HINSTANCE Instance,
        HINSTANCE PrevInstance,
        LPSTR CommandLine,
        int ShowCode)
{
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
 
    WNDCLASSA WindowClass = {};
    WindowClass.style = CS_HREDRAW | CS_VREDRAW;
    WindowClass.lpfnWndProc = Win32WindowCallback;
    WindowClass.hInstance = Instance;
    WindowClass.hCursor = LoadCursor(0, IDC_ARROW);
    WindowClass.lpszClassName = "DnCWindowClass";

    // TODO: Maybe for internal debug only?
    // Not exactly sure what to do on release
    AllocConsole();    
    GlobalStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    Win32Log("Hello Windows!\n");

    if (RegisterClassA(&WindowClass)) {
        Win32Log("Registering class\n");
        HWND Window; 
        {
            i32 WindowW = (i32)DesignWidth;
            i32 WindowH = (i32)DesignHeight;
            i32 WindowX = GetSystemMetrics(SM_CXSCREEN) / 2 - WindowW / 2;
            i32 WindowY = GetSystemMetrics(SM_CYSCREEN) / 2 - WindowH / 2;
            

            Window = CreateWindowExA(
                    0,
                    WindowClass.lpszClassName,
                    "Dots And Circles",
                    WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                    WindowX,
                    WindowY,
                    WindowW,
                    WindowH,
                    0,
                    0,
                    Instance,
                    0);
        }

        if (!Window) {
            Win32Log("Window failed to initialize\n");
            return 1;
        }
        
        Win32Log("Window Initialized\n");

        // Refresh rate 
        u32 RefreshRate = GlobalDefaultRefreshRate;
        {
            HDC Dc = GetDC(Window);
            i32 DisplayRefreshRate = GetDeviceCaps(Dc, VREFRESH);
            ReleaseDC(Window, Dc);
            // It is possible for the refresh rate to be 0 or less
            // because of something called 'adaptive vsync
            if (DisplayRefreshRate > 1) {
                RefreshRate = DisplayRefreshRate;
            }
        }
        
        f32 TargetMsPerFrame = 1.f / RefreshRate * 1000.f; 
        Win32Log("Target Ms Per Frame: %.2f", TargetMsPerFrame);

        // Create and initialize game related stuff
        win32_state Win32State = {};
        Win32BuildExePath(&Win32State);

        char SourceGameCodeDllFullPath[MAX_PATH];
        Win32BuildExePathFilename(&Win32State, SourceGameCodeDllFullPath, "game.dll");

        char TempGameCodeDllFullPath[MAX_PATH];        
        Win32BuildExePathFilename(&Win32State, SourceGameCodeDllFullPath, "temp_game.dll");

        char GameCodeLockFullPath[MAX_PATH];
        Win32BuildExePathFilename(&Win32State, SourceGameCodeDllFullPath, "lock");

        Win32Log("%s\n%s\n%s\n", 
                SourceGameCodeDllFullPath,
                TempGameCodeDllFullPath,
                GameCodeLockFullPath);
        

    }

     
    return 0;
}
