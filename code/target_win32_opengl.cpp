#include <windows.h>
#include <ShellScalingAPI.h>

#include "mm_arena.h"
#include "mm_core.h"
#include "mm_maths.h"
#include "mm_string.h"
#include "mm_list.h"
#include "mm_mailbox.h"
#include "game_platform.h"
#include "game_opengl.h"

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
typedef BOOL WINAPI 
WglFunction(wglChoosePixelFormatARB)(HDC hdc,
                                     const int* piAttribIList,
                                     const FLOAT* pfAttribFList,
                                     UINT nMaxFormats,
                                     int* piFormats,
                                     UINT* nNumFormats);

typedef BOOL WINAPI 
WglFunction(wglSwapIntervalEXT)(int interval);

typedef HGLRC WINAPI 
WglFunction(wglCreateContextAttribsARB)(HDC hdc, 
                                        HGLRC hShareContext,
                                        const int* attribList);

typedef const char* WINAPI 
WglFunction(wglGetExtensionsStringEXT)();

static WglFunctionPtr(wglCreateContextAttribsARB);
static WglFunctionPtr(wglChoosePixelFormatARB);
static WglFunctionPtr(wglSwapIntervalEXT);
static WglFunctionPtr(wglGetExtensionsStringEXT);


struct win32_state {
    b32 IsRunning;
    u32 PerformanceFrequency;
    char ExeFullPath[MAX_PATH];
    char* OnePastExeDirectory;

    opengl* Opengl;

    array<HANDLE> FileHandles;
    list<u32> FileHandleFreeIds;

    // memory
    arena Arena;

};

win32_state* Global_Win32State;

#if INTERNAL
    HANDLE Global_StdOut;
#endif

static inline LARGE_INTEGER
FiletimeToLargeInt(FILETIME Filetime) {
    LARGE_INTEGER Ret = {};
    Ret.LowPart = Filetime.dwLowDateTime;
    Ret.HighPart = Filetime.dwHighDateTime;

    return Ret;
}

static inline LONG
Width(RECT Value) {
    return Value.right - Value.left;
}

static inline LONG
Height(RECT Value) {
    return Value.bottom - Value.top;
}




static inline void*
Win32AllocateMemory(usize MemorySize) {
    return VirtualAllocEx(GetCurrentProcess(),
                          0, 
                          MemorySize,
                          MEM_RESERVE | MEM_COMMIT, 
                          PAGE_READWRITE);
}

static inline void
Win32FreeMemory(void* Memory) {
    VirtualFreeEx(GetCurrentProcess(), 
                  Memory,    
                  0, 
                  MEM_RELEASE); 
}
// TODO: Shift this into win32 state?
#if INTERNAL
static inline void
Win32WriteConsole(const char* Message) {
    WriteConsoleA(Global_StdOut,
                  Message, 
                  SiStrLen(Message), 
                  0, 
                  NULL);
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
    return (f32(End.QuadPart - Start.QuadPart)) / 
            Global_Win32State->PerformanceFrequency; 
}

static inline void
Win32BuildExePathFilename(char* Dest, const char* Filename) {
    for(const char *Itr = Global_Win32State->ExeFullPath; 
        Itr != Global_Win32State->OnePastExeDirectory; 
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

    char SrcFileName[MAX_PATH];
    char TempFileName[MAX_PATH];
    char LockFileName[MAX_PATH];        
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
Win32GameCode(const char* SrcFileName,
              const char* TempFileName,
              const char* LockFileName) 
{
    Assert(SrcFileName && TempFileName && LockFileName);

    win32_game_code Ret = {};
    Win32BuildExePathFilename(Ret.SrcFileName, SrcFileName);
    Win32BuildExePathFilename(Ret.TempFileName, TempFileName);
    Win32BuildExePathFilename(Ret.LockFileName, LockFileName);

    Win32Log("Src Game Code DLL: %s\n", Ret.SrcFileName);
    Win32Log("Tmp Game Code DLL: %s\n", Ret.TempFileName);
    Win32Log("Game Code Lock: %s\n", Ret.LockFileName);
    
    return Ret;
}

static inline void
Win32LoadGameCode(win32_game_code* Code) 
{
    WIN32_FILE_ATTRIBUTE_DATA Ignored; 
    if(!GetFileAttributesEx(Code->LockFileName, 
                            GetFileExInfoStandard, 
                            &Ignored)) 
    {
        Code->LastWriteTime = Win32GetLastWriteTime(Code->SrcFileName);
        CopyFile(Code->SrcFileName, Code->TempFileName, FALSE);    
        Code->Dll = LoadLibraryA(Code->TempFileName);
        if(Code->Dll) {
            Code->GameUpdate = 
                (game_update*)GetProcAddress(Code->Dll, "GameUpdate");
            Code->IsValid = (Code->GameUpdate != 0);
        }
    }
}

static inline void 
Win32UnloadGameCode(win32_game_code* Code) {
    if (Code->Dll) {
        FreeLibrary(Code->Dll);
        Code->Dll = 0;
    }
    Code->IsValid = false;
    Code->GameUpdate = 0;
}

static inline void
Win32ReloadGameCodeIfOutdated(win32_game_code* Code) 
{
    // Check last modified date
    LARGE_INTEGER CurrentLastWriteTime = 
        FiletimeToLargeInt(Win32GetLastWriteTime(Code->SrcFileName)); 
    LARGE_INTEGER GameCodeLastWriteTime =
        FiletimeToLargeInt(Code->LastWriteTime);

    if (CurrentLastWriteTime.QuadPart > GameCodeLastWriteTime.QuadPart) {
        Win32UnloadGameCode(Code);
        Win32LoadGameCode(Code);
    }
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

        // Here, we ask windows to find the best supported pixel 
        // format based on our desired format.
        SuggestedPixelFormatIndex = 
            ChoosePixelFormat(DeviceContext, &DesiredPixelFormat);
    }
    PIXELFORMATDESCRIPTOR SuggestedPixelFormat = {};
    
    DescribePixelFormat(DeviceContext, SuggestedPixelFormatIndex, 
                        sizeof(SuggestedPixelFormat), 
                        &SuggestedPixelFormat);
    SetPixelFormat(DeviceContext, 
                   SuggestedPixelFormatIndex, 
                   &SuggestedPixelFormat);
}


static inline b32
Win32OpenglLoadWglExtensions() {
    WNDCLASSA WindowClass = {};
    input GameInput = {};
    // Er yeah...we have to create a 'fake' Opengl context 
    // to load the extensions lol.
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

#define Win32SetWglFunction(Name) \
            Name = (WglFunction(Name)*)wglGetProcAddress(#Name); \
            if (!Name) { \
                Win32Log("[OpenGL] Cannot load: " #Name " \n"); \
                return false; \
            }

            Win32SetWglFunction(wglChoosePixelFormatARB);
            Win32SetWglFunction(wglCreateContextAttribsARB);
            Win32SetWglFunction(wglSwapIntervalEXT);

            wglMakeCurrent(0, 0);
            return true;
        }
        else {
            Win32Log("[OpenGL] Cannot begin to load wgl extensions\n");
            return false;
        }

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

#if INTERNAL
static inline
OpenglDebugCallbackFunc(Win32OpenglDebugCallback) {
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
        _severity = "LOW";
        break;

        case GL_DEBUG_SEVERITY_NOTIFICATION:
        _severity = "NOTIFICATION";
        break;
        
        default:
        _severity = "UNKNOWN";
        break;
    }
    
    Win32Log("[OpenGL] %d: %s of %s severity, raised from %s: %s\n",
              id, _type, _severity, _source, msg);
    
};
#endif

static inline void
Win32FreeOpengl(opengl* Opengl) {
    Win32FreeMemory(Opengl);
}

static inline opengl*
Win32AllocateOpengl(HDC DeviceContext, 
                    v2u WindowDimensions) 
{
    // This is kinda what we wanna do if we ever want Renderer to be its own DLL...
    usize RendererMemorySize = sizeof(opengl) + Kilobytes(128); 
    void* RendererMemory = Win32AllocateMemory(RendererMemorySize);
    opengl* Opengl = BootstrapStruct(opengl,
                                     Arena,
                                     RendererMemory, 
                                     RendererMemorySize);

    if (!Opengl) {
        Win32Log("Cannot allocate renderer memory"); 
        return nullptr;
    }

    if (!Win32OpenglLoadWglExtensions()) {
        Win32FreeOpengl(Opengl);
        return nullptr;
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
    
    // Modern
    OpenglContext = wglCreateContextAttribsARB(DeviceContext, 
                                               0, 
                                               Win32OpenglAttribs); 

    // Not modern
    //if (!OpenglContext) {
    //    OpenglContext = wglCreateContext(DeviceContext);
    //}

    if(wglMakeCurrent(DeviceContext, OpenglContext)) {
        HMODULE Module = LoadLibraryA("opengl32.dll");
        // TODO: Log functions that are not loaded
#define Win32SetOpenglFunction(Name) \
        Opengl->Name = (OpenglFunction(Name)*)Win32TryGetOpenglFunction(#Name, Module); \
        if (!Opengl->Name) { \
            Win32Log("[Opengl] Cannot load " #Name " \n"); \
            Win32FreeOpengl(Opengl); \
            return nullptr; \
        }

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
        Win32SetOpenglFunction(glDeleteTextures);
        Win32SetOpenglFunction(glDebugMessageCallbackARB);
    }
    Init(Opengl, 
         WindowDimensions, 
         128,
         8);

#if INTERNAL
    Opengl->glEnable(GL_DEBUG_OUTPUT);
    Opengl->glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    Opengl->glDebugMessageCallbackARB(Win32OpenglDebugCallback, nullptr);
#endif

    return Opengl;
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
    return v2u{ 
        u16(Rect.right - Rect.left),
        u16(Rect.bottom - Rect.top)
    };

}

static inline v2u
Win32GetClientDimensions(HWND Window) {
    RECT Rect = {};
    GetClientRect(Window, &Rect);
    return v2u{ 
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
                Global_Win32State->IsRunning = false;
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
            Global_Win32State->IsRunning = false;
        } break;
        case WM_DESTROY: {
            Global_Win32State->IsRunning = false;
        } break;
        case WM_WINDOWPOSCHANGED: {
            if(Global_Win32State->Opengl && 
               Global_Win32State->Opengl->Header.IsInitialized) {
                v2u WindowWH = Win32GetWindowDimensions(Window);
                v2u ClientWH = Win32GetClientDimensions(Window);
                //Win32Log("ClientWH: %d x %d\n", ClientWH.W, ClientWH.H);
                //Win32Log("WindowWH: %d x %d\n", WindowWH.W, WindowWH.H);
                Resize(Global_Win32State->Opengl, (u16)ClientWH.W, (u16)ClientWH.H);
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
Win32Init(win32_state* State) {
    // Initialize performance frequency
    {
        LARGE_INTEGER PerfCountFreq;
        QueryPerformanceFrequency(&PerfCountFreq);
        State->PerformanceFrequency = SafeCastI64ToU32(PerfCountFreq.QuadPart);
        State->IsRunning = true;
    }

    // Initialize paths
    {
        GetModuleFileNameA(0, 
                           State->ExeFullPath, 
                           sizeof(State->ExeFullPath));

        State->OnePastExeDirectory = Global_Win32State->ExeFullPath;
        for( char* Itr = Global_Win32State->ExeFullPath; *Itr; ++Itr) {
            if (*Itr == '\\') {
                State->OnePastExeDirectory = Itr + 1;
            }
        }

    }

    // Initialize file handle store
    {
        const u32 FileHandleCap = 8;
        State->FileHandles = Array<HANDLE>(&State->Arena, FileHandleCap); 
        State->FileHandleFreeIds = List<u32>(&State->Arena, FileHandleCap);
        for (u32 I = 0; I < FileHandleCap; ++I) {
            Push(&State->FileHandleFreeIds, I);
        }
    }
}

// Platform Functions ////////////////////////////////////////////////////
enum platform_file_error {
    PlatformFileError_None,
    PlatformFileError_NotEnoughHandlers,
    PlatformFileError_CannotOpenFile,   
    PlatformFileError_Closed,
};

static inline 
PlatformOpenAssetFileFunc(Win32OpenAssetFile) {
    platform_file_handle Ret = {}; 
    const char* Path = "yuu";
    list<u32>* FreeIds = &Global_Win32State->FileHandleFreeIds;
    
    // Check if there are free handlers to go around
    if (FreeIds->Count == 0) {
        Ret.Error = PlatformFileError_NotEnoughHandlers;
        return Ret;
    }    

    HANDLE Win32Handle = CreateFileA(Path, 
                                    GENERIC_READ, 
                                    FILE_SHARE_READ,
                                    0,
                                    OPEN_EXISTING,
                                    0,
                                    0);
    
     
    if(Win32Handle == INVALID_HANDLE_VALUE) {
        Win32Log("[Win32ReadFile] Cannot open file: %s\n", Path);
        Ret.Error = PlatformFileError_CannotOpenFile;
        return Ret;
    } 

    Ret.Id = Back(FreeIds);
    Pop(FreeIds);
    Global_Win32State->FileHandles[Ret.Id] = Win32Handle; 

    return Ret; 
}


static inline 
PlatformLogFileErrorFunc(Win32LogFileError) {
    switch(Handle->Error) {
        case PlatformFileError_None: {
            Win32Log("[File] There is no file error\n");
        } break;
        case PlatformFileError_NotEnoughHandlers: {
            Win32Log("[File] There is not enough handlers\n");
        } break;
        case PlatformFileError_CannotOpenFile:{
            Win32Log("[File] Cannot open file\n");
        } break;
        case PlatformFileError_Closed:{
            Win32Log("[File] File is already closed");
        } break;
    }

}

static inline
PlatformCloseFileFunc(Win32CloseFile) {
    HANDLE Win32Handle = Global_Win32State->FileHandles[Handle->Id];
    Push(&Global_Win32State->FileHandleFreeIds, Handle->Id);
    if (Win32Handle != INVALID_HANDLE_VALUE) {
        CloseHandle(Win32Handle); 
    }
}
static inline
PlatformAddTextureFunc(Win32AddTexture) {
    return AddTexture(Global_Win32State->Opengl, Width, Height, Pixels);
}

static inline 
PlatformClearTexturesFunc(Win32ClearTextures) {
    return ClearTextures(Global_Win32State->Opengl);
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

static inline
PlatformGetFileSizeFunc(Win32GetFileSize) 
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


static inline platform_api 
Win32LoadPlatformApi() {
    platform_api Ret = {};
    Ret.Log = Win32Log;
    Ret.ReadFile = Win32ReadFile;
    Ret.GetFileSize = Win32GetFileSize;
    Ret.ClearTextures = Win32ClearTextures;
    Ret.AddTexture = Win32AddTexture;
    Ret.OpenAssetFile = Win32OpenAssetFile;
    Ret.CloseFile = Win32CloseFile;
    return Ret;
}

int CALLBACK
WinMain(HINSTANCE Instance,
        HINSTANCE PrevInstance,
        LPSTR CommandLine,
        int ShowCode)
{
#if INTERNAL
    AllocConsole();    
    Defer { FreeConsole(); };
    Global_StdOut = GetStdHandle(STD_OUTPUT_HANDLE);
#endif 
    usize PlatformMemorySize = Kilobytes(256);
    void* PlatformMemory = Win32AllocateMemory(PlatformMemorySize);
    if (!PlatformMemory) {
        Win32Log("Cannot allocate platform memory\n");
        return 1;
    }
    Defer { Win32FreeMemory(PlatformMemory); };

    Global_Win32State = BootstrapStruct(win32_state,
                                        Arena,
                                        PlatformMemory,
                                        PlatformMemorySize);

    Win32Init(Global_Win32State);
    
    input GameInput = {};
    {
        char _Characters[10];
        Init(&GameInput, _Characters, 10);
    }


    platform_api PlatformApi = Win32LoadPlatformApi();
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
 
    WNDCLASSA WindowClass = {};
    WindowClass.style = CS_HREDRAW | CS_VREDRAW;
    WindowClass.lpfnWndProc = Win32WindowCallback;
    WindowClass.hInstance = Instance;
    WindowClass.hCursor = LoadCursor(0, IDC_ARROW);
    WindowClass.lpszClassName = "DnCWindowClass";

    if(!RegisterClassA(&WindowClass)) {
        Win32Log("Failed to register class");
        return 1;
    }

    Win32Log("Window class registered\n");
    HWND Window;
    {
        RECT WindowRect = {};
        v2u MonitorDimensions = Win32GetMonitorDimensions();
        WindowRect.left = MonitorDimensions.W / 2 - (u16)Global_DesignWidth / 2;
        WindowRect.right = MonitorDimensions.W / 2 + (u16)Global_DesignWidth / 2;
        WindowRect.top = MonitorDimensions.H / 2 - (u16)Global_DesignHeight / 2;
        WindowRect.bottom = MonitorDimensions.H / 2 + (u16)Global_DesignHeight / 2;
        
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

    if (!Window) {
        Win32Log("Window failed to intialize\n");
        return 1;
    }

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
    Win32Log("Monitor Refresh Rate: %d\n", RefreshRate);

    // Create and initialize game code and DLL file paths
    // Load the game code DLL
    // TODO: Make game code global?
    win32_game_code GameCode = 
        Win32GameCode("game.dll", "temp_game.dll", "lock");

    // Initialize game memory
    game_memory GameMemory = {};
    GameMemory.PermanentMemorySize = Megabytes(256);
    GameMemory.PermanentMemory = Win32AllocateMemory(GameMemory.PermanentMemorySize); 
    if (!GameMemory.PermanentMemory) {
        Win32Log("Cannot allocate game permanent memory\n");
        return 1;
    }
    Defer { Win32FreeMemory(GameMemory.PermanentMemory); };

    GameMemory.TransientMemorySize = Gigabytes(1);
    GameMemory.TransientMemory = Win32AllocateMemory(GameMemory.TransientMemorySize);
    if (!GameMemory.TransientMemory) {
        Win32Log("Cannot allocate game transient memory\n");
        return 1;
    }
    Defer { Win32FreeMemory(GameMemory.TransientMemory); };
    
    // Intialize Render commands
    usize RenderCommandsMemorySize = Megabytes(64);
    void* RenderCommandsMemory = Win32AllocateMemory(RenderCommandsMemorySize); 
    if (!RenderCommandsMemory) {
        Win32Log("Cannot allocate render commands memory\n");
        return 1;
    }
    mailbox RenderCommands = Mailbox(RenderCommandsMemory,
                                     RenderCommandsMemorySize);
 
    // Initialize OpenGL
    Global_Win32State->Opengl = Win32AllocateOpengl(DeviceContext, 
                                        Win32GetClientDimensions(Window));
    if (!Global_Win32State->Opengl) {
        Win32Log("Cannot initialize opengl\n");
        return 1;
    }
    Defer { Win32FreeOpengl(Global_Win32State->Opengl); };

    // Set sleep granularity to 1ms
    b32 SleepIsGranular = timeBeginPeriod(1) == TIMERR_NOERROR;

    // Game Loop
    LARGE_INTEGER LastCount = Win32GetCurrentCounter(); 
    while (Global_Win32State->IsRunning) {
        Win32ReloadGameCodeIfOutdated(&GameCode);

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

        Render(Global_Win32State->Opengl, &RenderCommands);
        Clear(&RenderCommands);
        
        f32 SecsElapsed = 
            Win32GetSecondsElapsed(LastCount, Win32GetCurrentCounter());
        
        if (TargetSecsPerFrame > SecsElapsed) {
            if (SleepIsGranular) {
                DWORD MsToSleep = (DWORD)(1000.f * (TargetSecsPerFrame - SecsElapsed));
                // We cut the sleep some slack, so we sleep 1 sec less.
                if (MsToSleep > 1) {
                    Sleep(MsToSleep - 1);
                }
            }
            while(TargetSecsPerFrame > 
                    Win32GetSecondsElapsed(LastCount, Win32GetCurrentCounter()));

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
