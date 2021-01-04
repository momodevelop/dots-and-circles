#include <windows.h>
#include <ShellScalingAPI.h>

#include "mm_core.h"
#include "mm_maths.h"
#include "mm_string.h"
#include "mm_mailbox.h"
#include "platform.h"
#include "renderer_opengl.h"

#define STB_SPRINTF_IMPLEMENTATION
#include "stb_sprintf.h"


// Opengl/WGL Bindings
#define WGL_CONTEXT_MAJOR_VERSION_ARB           0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB           0x2092
#define WGL_CONTEXT_LAYER_PLANE_ARB             0x2093
#define WGL_CONTEXT_FLAGS_ARB                   0x2094
#define WGL_CONTEXT_PROFILE_MASK_ARB            0x9126

#define WGL_DRAW_TO_WINDOW_ARB                  0x2001
#define WGL_ACCELERATION_ARB                    0x2003
#define WGL_SUPPORT_OPENGL_ARB                  0x2010
#define WGL_DOUBLE_BUFFER_ARB                   0x2011
#define WGL_PIXEL_TYPE_ARB                      0x2013
#define WGL_TYPE_RGBA_ARB                       0x202B
#define WGL_FULL_ACCELERATION_ARB               0x2027
#define WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB        0x20A9
#define WGL_CONTEXT_FLAG_ARB                    0x2094
#define WGL_CONTEXT_DEBUG_BIT_ARB               0x0001
#define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB  0x0002
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB        0x00000001

// TODO: Use defines for better readability
#define WglFunction(Name) wgl_func_##Name
#define WglFunctionPtr(Name) WglFunction(Name)* Name
typedef BOOL WINAPI WglFunction(wglChoosePixelFormatARB)(HDC hdc,
                                                         const int* piAttribIList,
                                                         const FLOAT* pfAttribFList,
                                                         UINT nMaxFormats,
                                                         int* piFormats,
                                                         UINT* nNumFormats);
typedef BOOL WINAPI WglFunction(wglSwapIntervalEXT)(int interval);
typedef HGLRC WINAPI WglFunction(wglCreateContextAttribsARB)(HDC hdc, 
                                                             HGLRC hShareContext,
                                                            const int* attribList);
typedef const char* WINAPI WglFunction(wglGetExtensionsStringEXT)();



static WglFunctionPtr(wglCreateContextAttribsARB);
static WglFunctionPtr(wglChoosePixelFormatARB);
static WglFunctionPtr(wglSwapIntervalEXT);
static WglFunctionPtr(wglGetExtensionsStringEXT);

// Globals
static const char* Global_GameCodeDllFileName = "game.dll";
static const char* Global_TempGameCodeDllFileName = "temp_game.dll";
static const char* Global_GameCodeLockFileName = "lock";



b32 Global_IsRunning;
u32 Global_PerformanceFrequency;
char Global_ExeFullPath[MAX_PATH];
char* Global_OnePastExeDirectory;

char Global_SourceGameCodeDllFullPath[MAX_PATH];
char Global_GameCodeLockFullPath[MAX_PATH];
char Global_TempGameCodeDllFullPath[MAX_PATH];        

renderer_opengl Global_Opengl;

#if INTERNAL
    HANDLE Global_StdOut;
#endif

static inline LONG
Width(RECT Value) {
    return Value.right - Value.left;
}

static inline LONG
Height(RECT Value) {
    return Value.bottom - Value.top;
}

// TODO: Shift this into win32 state?
#if INTERNAL
static inline void
Win32WriteConsole(const char* Message) {
    WriteConsoleA(Global_StdOut, Message, SiStrLen(Message), 0, NULL);
}
#endif

static inline
PlatformLogFunc(Win32Log) {
    char Buffer[256];

    va_list VaList;
    va_start(VaList, Format);
   
    stbsp_vsprintf(Buffer, Format, VaList);

#if INTERNAL
    Win32WriteConsole(Buffer);
#endif
    // TODO: Logging to text file?

    va_end(VaList);

}
#define Win32RuntimeAssert(Cond, Msg) if(!(Cond)) { Win32Log(Msg); ExitProcess(1); }

static inline LARGE_INTEGER
Win32GetCurrentCounter(void) {
    LARGE_INTEGER Result;
    QueryPerformanceCounter(&Result);
    return Result;
}

static inline f32
Win32GetSecondsElapsed(LARGE_INTEGER Start, 
                       LARGE_INTEGER End) 
{
    return (f32(End.QuadPart - Start.QuadPart)) / Global_PerformanceFrequency; 
}

static inline
PlatformReadFileFunc(Win32ReadFile) {
    HANDLE FileHandle = CreateFileA(Path, 
                                    GENERIC_READ, 
                                    FILE_SHARE_READ,
                                    0,
                                    OPEN_EXISTING,
                                    0,
                                    0);
    Defer { CloseHandle(FileHandle); };

    if(FileHandle == INVALID_HANDLE_VALUE) {
        Win32Log("[Win32ReadFile] Cannot open file: %s\n", Path);
        return false;
    } else {
        LARGE_INTEGER FileSize;
        if (GetFileSizeEx(FileHandle, &FileSize)) {
            u32 FileSize32 = SafeCastU64ToU32(FileSize.QuadPart);
            if (FileSize.QuadPart > DestSize) {
                Win32Log("[Win32ReadFile] DestSize too smol for file: %s!\n", Path); 
                return false;
            }

            DWORD BytesRead;
            if(ReadFile(FileHandle, Dest, FileSize32, &BytesRead, 0) &&
               FileSize.QuadPart == BytesRead) {
                return true;
            }
            else {
                Win32Log("[Win32ReadFile] Cannot read file: %s!\n", Path);
                return false;
            }

        }
        else {
            Win32Log("[Win32ReadFile] Cannot get file size: %s!\n", Path);
            return false;
        }
    }

    
}

static inline u32
Win32GetFileSize(const char* Path) 
{
    HANDLE FileHandle = CreateFileA(Path, 
                                    GENERIC_READ, 
                                    FILE_SHARE_READ,
                                    0,
                                    OPEN_EXISTING,
                                    0,
                                    0);
    Defer { CloseHandle(FileHandle); };

    if(FileHandle == INVALID_HANDLE_VALUE) {
        Win32Log("[Win32GetFileSize] Cannot open file: %s\n", Path);
        return 0;
    } else {
        LARGE_INTEGER FileSize;
        if (!GetFileSizeEx(FileHandle, &FileSize)) {
            Win32Log("[Win32GetFileSize] Problems getting file size: %s\n", Path);
            return 0;
        }

        return (u32)FileSize.QuadPart;
    }
}


static inline void
Win32BuildExePathFilename(char* Dest, const char* Filename) {
    for(const char *Itr = Global_ExeFullPath; 
        Itr != Global_OnePastExeDirectory; 
        ++Itr, ++Dest) 
    {
        (*Dest) = (*Itr);
    }
    
    for (const char* Itr = Filename;
         (*Itr) != 0;
         ++Itr, ++Dest) 
    {
        (*Dest) = (*Itr);
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

static inline platform_api 
Win32LoadPlatformApi() {
    platform_api Ret = {};
    Ret.Log = Win32Log;
    Ret.ReadFile = Win32ReadFile;
    Ret.GetFileSize = Win32GetFileSize;
    return Ret;
}


static inline void
Win32OpenglSetPixelFormat(HDC DeviceContext) {
    i32 SuggestedPixelFormatIndex = 0;
    u32 ExtendedPick = 0;
    
    if (wglChoosePixelFormatARB) {
        i32 IntAttribList[] = {
            WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
            WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
            WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
            WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
            WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
            WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB, GL_TRUE,
            0,
        };
    
        wglChoosePixelFormatARB(DeviceContext, IntAttribList, 0, 1,
                &SuggestedPixelFormatIndex, &ExtendedPick);

    }
    
    if (!ExtendedPick) {
        PIXELFORMATDESCRIPTOR DesiredPixelFormat = {};
        DesiredPixelFormat.nSize = sizeof(DesiredPixelFormat);
        DesiredPixelFormat.nVersion = 1;
        DesiredPixelFormat.iPixelType = PFD_TYPE_RGBA;
        DesiredPixelFormat.dwFlags = 
            PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER; 
        DesiredPixelFormat.cColorBits = 32;
        DesiredPixelFormat.cAlphaBits = 8;
        DesiredPixelFormat.iLayerType = PFD_MAIN_PLANE;

        // Here, we ask windows to find the best supported pixel format based on our
        // desired format.
        SuggestedPixelFormatIndex = ChoosePixelFormat(DeviceContext, &DesiredPixelFormat);
    }
    PIXELFORMATDESCRIPTOR SuggestedPixelFormat = {};
    
    DescribePixelFormat(DeviceContext, SuggestedPixelFormatIndex, 
                        sizeof(SuggestedPixelFormat), 
                        &SuggestedPixelFormat);
    SetPixelFormat(DeviceContext, SuggestedPixelFormatIndex, &SuggestedPixelFormat);
}


static inline b32
Win32OpenglLoadWglExtensions() {
    WNDCLASSA WindowClass = {};
    input GameInput = {};
    // Er yeah...we have to create a 'fake' Opengl context to load the extensions lol.
    WindowClass.lpfnWndProc = DefWindowProcA;
    WindowClass.hInstance = GetModuleHandle(0);
    WindowClass.lpszClassName = "WGLLoader";

    if (RegisterClassA(&WindowClass)) {
        HWND Window = CreateWindowExA( 
                0,
                WindowClass.lpszClassName,
                "WGL Loader",
                0,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                0,
                0,
                WindowClass.hInstance,
                0);
        Defer { DestroyWindow(Window); };

        HDC Dc = GetDC(Window);
        Defer { ReleaseDC(Window, Dc); };
        
        Win32OpenglSetPixelFormat(Dc);
        
        HGLRC OpenglContext = wglCreateContext(Dc);
        Defer { wglDeleteContext(OpenglContext); };


        if (wglMakeCurrent(Dc, OpenglContext)) {

#define Win32SetWglFunction(Name) Name = (WglFunction(Name)*)wglGetProcAddress(#Name); if (!Name) { Win32Log("[OpenGL] Cannot load: " #Name " \n"); }

            Win32SetWglFunction(wglChoosePixelFormatARB);
            Win32SetWglFunction(wglCreateContextAttribsARB);
            Win32SetWglFunction(wglSwapIntervalEXT);

            wglMakeCurrent(0, 0);
        }
        else {
            Win32Log("[OpenGL] Cannot begin to load wgl extensions\n");
            return false;
        }

        return true;
    }
    else {
        Win32Log("[OpenGL] Cannot register class to load wgl extensions\n");
        return false;
    }
}

static inline u32
Win32DetermineIdealRefreshRate(HDC DeviceContext) {
    u32 RefreshRate = Global_DefaultRefreshRate;
    {
        i32 DisplayRefreshRate = GetDeviceCaps(DeviceContext, VREFRESH);
        // It is possible for the refresh rate to be 0 or less
        // because of something called 'adaptive vsync
        if (DisplayRefreshRate > 1) {
            RefreshRate = DisplayRefreshRate;
        }
    }
    return RefreshRate;
    
}

static inline void* 
Win32TryGetOpenglFunction(const char* Name, HMODULE FallbackModule)
{
    void* p = (void*)wglGetProcAddress(Name);
    if ((p == 0) || 
        (p == (void*)0x1) || 
        (p == (void*)0x2) || 
        (p == (void*)0x3) || 
        (p == (void*)-1))
    {
        p = (void*)GetProcAddress(FallbackModule, Name);
    }
    return p;

}
// TODO: Maybe return the context?
static inline b32
Win32OpenglInit(HDC DeviceContext, 
                u32 WindowWidth, 
                u32 WindowHeight) 
{

    if (!Win32OpenglLoadWglExtensions()) {
        return false;
    }
    Win32OpenglSetPixelFormat(DeviceContext);


    i32 Win32OpenglAttribs[] {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
        WGL_CONTEXT_MINOR_VERSION_ARB, 5,
        WGL_CONTEXT_FLAG_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB
#if INTERNAL
        | WGL_CONTEXT_DEBUG_BIT_ARB
#endif
        ,
        WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        0,
    };
    HGLRC OpenglContext = 0;
    
    // Mordern
    if(wglCreateContextAttribsARB) {
        OpenglContext = wglCreateContextAttribsARB(DeviceContext, 0, Win32OpenglAttribs); 
    }

    // Not modernw
    if (!OpenglContext) {
        OpenglContext = wglCreateContext(DeviceContext);
    }

    if(wglMakeCurrent(DeviceContext, OpenglContext)) {
        HMODULE Module = LoadLibraryA("opengl32.dll");
        // TODO: Log functions that are not loaded
#define Win32SetOpenglFunction(Name) Global_Opengl.Name = (OpenglFunction(Name)*)Win32TryGetOpenglFunction(#Name, Module); \
        if (!Global_Opengl.Name) { Win32Log("[Opengl] Cannot load " #Name " \n"); return false; }

        Win32SetOpenglFunction(glEnable);
        Win32SetOpenglFunction(glDisable); 
        Win32SetOpenglFunction(glViewport);
        Win32SetOpenglFunction(glScissor);
        Win32SetOpenglFunction(glCreateShader);
        Win32SetOpenglFunction(glCompileShader);
        Win32SetOpenglFunction(glShaderSource);
        Win32SetOpenglFunction(glAttachShader);
        Win32SetOpenglFunction(glDeleteShader);
        Win32SetOpenglFunction(glClear);
        Win32SetOpenglFunction(glClearColor);
        Win32SetOpenglFunction(glCreateBuffers);
        Win32SetOpenglFunction(glNamedBufferStorage);
        Win32SetOpenglFunction(glCreateVertexArrays);
        Win32SetOpenglFunction(glVertexArrayVertexBuffer);
        Win32SetOpenglFunction(glEnableVertexArrayAttrib);
        Win32SetOpenglFunction(glVertexArrayAttribFormat);
        Win32SetOpenglFunction(glVertexArrayAttribBinding);
        Win32SetOpenglFunction(glVertexArrayBindingDivisor);
        Win32SetOpenglFunction(glBlendFunc);
        Win32SetOpenglFunction(glCreateProgram);
        Win32SetOpenglFunction(glLinkProgram);
        Win32SetOpenglFunction(glGetProgramiv);
        Win32SetOpenglFunction(glGetProgramInfoLog);
        Win32SetOpenglFunction(glVertexArrayElementBuffer);
        Win32SetOpenglFunction(glCreateTextures);
        Win32SetOpenglFunction(glTextureStorage2D);
        Win32SetOpenglFunction(glTextureSubImage2D);
        Win32SetOpenglFunction(glBindTexture);
        Win32SetOpenglFunction(glTexParameteri);
        Win32SetOpenglFunction(glBindVertexArray);
        Win32SetOpenglFunction(glDrawElementsInstancedBaseInstance);
        Win32SetOpenglFunction(glGetUniformLocation);
        Win32SetOpenglFunction(glProgramUniformMatrix4fv);
        Win32SetOpenglFunction(glNamedBufferSubData);
        Win32SetOpenglFunction(glUseProgram);
    }
    OpenglInit(&Global_Opengl, WindowWidth, WindowHeight, 128);

    return true;
}

static inline v2u
Win32GetMonitorDimensions() {
    v2u Ret = {};
    Ret.W = SafeCastI32ToU32(GetSystemMetrics(SM_CXSCREEN));
    Ret.H = SafeCastI32ToU32(GetSystemMetrics(SM_CYSCREEN));
    return Ret;
}

static inline v2u
Win32GetWindowDimensions(HWND Window) {
    RECT Rect = {};
    GetWindowRect(Window, &Rect);
    return v2u { 
        u32(Rect.right - Rect.left),
        u32(Rect.bottom - Rect.top)
    };

}

static inline v2u
Win32GetClientDimensions(HWND Window) {
    RECT Rect = {};
    GetClientRect(Window, &Rect);
    return v2u { 
        u32(Rect.right - Rect.left),
        u32(Rect.bottom - Rect.top)
    };

}
static inline void
Win32ProcessMessages(HWND Window, 
                     input* Input)
{
    MSG Msg = {};
    while(PeekMessage(&Msg, Window, 0, 0, PM_REMOVE)) {
        switch(Msg.message) {
            case WM_CLOSE: {
                Global_IsRunning = false;
            } break;
            case WM_CHAR: {
                char C = (char)Msg.wParam;
                TryPushCharacterInput(Input, C);
            } break;
            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
            case WM_KEYDOWN:
            case WM_KEYUP: {
                u32 KeyCode = (u32)Msg.wParam;
                bool IsDown = Msg.message == WM_KEYDOWN;
                switch(KeyCode) {
                    case 'W':
                        Input->ButtonUp.Now = IsDown;
                        break;
                    case 'A':
                        Input->ButtonLeft.Now = IsDown;
                        break;
                    case 'S':
                        Input->ButtonDown.Now = IsDown;
                        break;
                    case 'D':
                        Input->ButtonRight.Now = IsDown;
                        break;
                    case VK_SPACE:
                        Input->ButtonSwitch.Now = IsDown;
                        break;
                    case VK_RETURN:
                        Input->ButtonConfirm.Now = IsDown;
                        break;
                    case VK_F1:
                        Input->ButtonConsole.Now = IsDown;
                        break;
                    case VK_BACK:
                        Input->ButtonBack.Now = IsDown;
                        break;
                }
                
                TranslateMessage(&Msg);

            } break;
            default: 
            {
                TranslateMessage(&Msg);
                DispatchMessage(&Msg);
            } break;
        }
    }
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
            Global_IsRunning = false;
        } break;
        case WM_DESTROY: {
            Global_IsRunning = false;
        } break;
        case WM_WINDOWPOSCHANGED: {
            if(Global_Opengl.Header.IsInitialized) {
                v2u WindowWH = Win32GetWindowDimensions(Window);
                v2u ClientWH = Win32GetClientDimensions(Window);

                Win32Log("ClientWH: %d x %d\n", ClientWH.W, ClientWH.H);
                Win32Log("WindowWH: %d x %d\n", WindowWH.W, WindowWH.H);

                OpenglResize(&Global_Opengl, (u32)ClientWH.W, (u32)ClientWH.H);
            }
        } break;

        default: {
            // Log message?
            Result = DefWindowProcA(Window, Message, WParam, LParam);
        };   
    }
    return Result;
}

static inline void
Win32InitGlobals() {
    // Initialize performance frequency
    {
        LARGE_INTEGER PerfCountFreq;
        QueryPerformanceFrequency(&PerfCountFreq);
        Global_PerformanceFrequency = SafeCastI64ToU32(PerfCountFreq.QuadPart);
        Global_IsRunning = true;
    }

    // Initialize paths
    {
        GetModuleFileNameA(0, 
                           Global_ExeFullPath, 
                           sizeof(Global_ExeFullPath));

        Global_OnePastExeDirectory = Global_ExeFullPath;
        for( char* Itr = Global_ExeFullPath; *Itr; ++Itr) {
            if (*Itr == '\\') {
                Global_OnePastExeDirectory = Itr + 1;
            }
        }

        Win32BuildExePathFilename(Global_SourceGameCodeDllFullPath, 
                                  Global_GameCodeDllFileName);
        Win32BuildExePathFilename(Global_TempGameCodeDllFullPath, 
                                  Global_TempGameCodeDllFileName);
        Win32BuildExePathFilename(Global_GameCodeLockFullPath, 
                                  Global_GameCodeLockFileName);

        Win32Log("Src Game Code DLL: %s\n", Global_SourceGameCodeDllFullPath);
        Win32Log("Tmp Game Code DLL: %s\n", Global_TempGameCodeDllFullPath);
        Win32Log("Game Code Lock: %s\n", Global_GameCodeLockFullPath);
    }
}


int CALLBACK
WinMain(HINSTANCE Instance,
        HINSTANCE PrevInstance,
        LPSTR CommandLine,
        int ShowCode)
{
    Win32InitGlobals();
    input GameInput = {};
    {
        // TODO: We should really use an arena for this but I'm lazy af.
        char _Characters[10];
        Init(&GameInput, _Characters, 10);
    }

#if INTERNAL
    // Allocate game_console and stdout
    AllocConsole();    
    Defer { FreeConsole(); };
    Global_StdOut = GetStdHandle(STD_OUTPUT_HANDLE);

    platform_api PlatformApi = Win32LoadPlatformApi();
#endif 


    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
 
    WNDCLASSA WindowClass = {};
    WindowClass.style = CS_HREDRAW | CS_VREDRAW;
    WindowClass.lpfnWndProc = Win32WindowCallback;
    WindowClass.hInstance = Instance;
    WindowClass.hCursor = LoadCursor(0, IDC_ARROW);
    WindowClass.lpszClassName = "DnCWindowClass";

    Win32RuntimeAssert(RegisterClassA(&WindowClass), "Failed to register class");

    Win32Log("Window class registered\n");
    HWND Window;
    {
        RECT WindowRect = {};
        v2u MonitorDimensions = Win32GetMonitorDimensions();
        WindowRect.left = MonitorDimensions.W / 2 - (u32)Global_DesignWidth / 2;
        WindowRect.right = MonitorDimensions.W / 2 + (u32)Global_DesignWidth / 2;
        WindowRect.top = MonitorDimensions.H / 2 - (u32)Global_DesignHeight / 2;
        WindowRect.bottom = MonitorDimensions.H / 2 + (u32)Global_DesignHeight / 2;
        
        DWORD Style = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
        AdjustWindowRectEx(&WindowRect,
                           Style,
                           FALSE,
                           0);
        

        // TODO: Adaptively create 'best' window resolution based on desktop reso.
        Window = CreateWindowExA(
                    0,
                    WindowClass.lpszClassName,
                    "Dots And Circles",
                    Style,
                    WindowRect.left,
                    WindowRect.top,
                    Width(WindowRect),
                    Height(WindowRect),
                    0,
                    0,
                    Instance,
                    0);
    }

    Win32RuntimeAssert(Window, "Window failed to initialize\n");

    Win32Log("Window Initialized\n");
    // Log the dimensions of window and client area
    {
        v2u WindowWH = Win32GetWindowDimensions(Window);
        v2u ClientWH = Win32GetClientDimensions(Window);
        Win32Log("Window Dimensions: %d x %d\n", WindowWH.W, WindowWH.H);
        Win32Log("Client Dimensions: %d x %d\n", ClientWH.W, ClientWH.H);
    }
    



    HDC DeviceContext = GetDC(Window); 

    u32 RefreshRate = Win32DetermineIdealRefreshRate(DeviceContext); 
    f32 TargetSecsPerFrame = 1.f / RefreshRate; 
    Win32Log("Target Secs Per Frame: %.2f\n", TargetSecsPerFrame);
    Win32Log("Monitor Refresh Rate: %d", RefreshRate);

    // Create and initialize game code and DLL file paths
    // Load the game code DLL
    // TODO: Make game code global?
    win32_game_code GameCode = 
        Win32LoadGameCode(
            Global_SourceGameCodeDllFullPath,
            Global_TempGameCodeDllFullPath,
            Global_GameCodeLockFullPath);

    // Allocate memory for the entire program
    void* ProgramMemory = VirtualAllocEx(GetCurrentProcess(),
                                         0, Global_TotalMemorySize, 
                                         MEM_RESERVE | MEM_COMMIT, 
                                         PAGE_READWRITE);

    
    Win32RuntimeAssert(ProgramMemory, "Cannot allocate program memory\n");
    Defer { VirtualFreeEx(GetCurrentProcess(), ProgramMemory, 0, MEM_RELEASE); }; 
    arena Win32Arena = Arena(ProgramMemory, Global_TotalMemorySize);
    
    // Initialize game memory
    game_memory GameMemory = {};
    GameMemory.MainMemory = PushBlock(&Win32Arena, Global_GameMainMemorySize); 
    Win32RuntimeAssert(GameMemory.MainMemory, "Cannot allocate game main memory\n");
    GameMemory.MainMemorySize = Global_GameMainMemorySize;
    
    // Intialize Render commands
    void* RenderCommandsMemory = PushBlock(&Win32Arena, 
                                           Global_RenderCommandsMemorySize);
    mailbox RenderCommands = Mailbox(RenderCommandsMemory, 
                                     Global_RenderCommandsMemorySize);
    
 
    // Initialize OpenGL
    {
        v2u Dimensions = Win32GetClientDimensions(Window);
        b32 Success = Win32OpenglInit(DeviceContext, 
                                      Dimensions.W, 
                                      Dimensions.H);
        Win32RuntimeAssert(Success, "Cannot initialize Opengl");
    }

    // Set sleep granularity to 1ms
    b32 SleepIsGranular = timeBeginPeriod(1) == TIMERR_NOERROR;

    // Game Loop
    LARGE_INTEGER LastCount = Win32GetCurrentCounter(); 
    while (Global_IsRunning) {
        Update(&GameInput);
        Win32ProcessMessages(Window, 
                             &GameInput);

        if (GameCode.GameUpdate) {
            GameCode.GameUpdate(&GameMemory,
                       &PlatformApi,
                       &RenderCommands,
                       &GameInput,
                       TargetSecsPerFrame);
        }

        OpenglRender(&Global_Opengl, &RenderCommands);
        Clear(&RenderCommands);
        
        f32 SecondsElapsed = Win32GetSecondsElapsed(LastCount, 
                                                    Win32GetCurrentCounter());
        if (TargetSecsPerFrame > SecondsElapsed) {
            if (SleepIsGranular) {
                DWORD MsToSleep = (DWORD)(1000.f * (TargetSecsPerFrame - SecondsElapsed)) - 1;
                if (MsToSleep > 0) {
                    Sleep(MsToSleep);
                }
            }
            while(TargetSecsPerFrame > Win32GetSecondsElapsed(LastCount, 
                                                              Win32GetCurrentCounter()));

        }
        else {
            Win32Log("Frame rate missed!\n");
        }

        LastCount = Win32GetCurrentCounter();
        // Swap buffers
        {
            HDC Dc = GetDC(Window);
            SwapBuffers(Dc);
            ReleaseDC(Window, Dc); 
        }

    }


    return 0;
}
