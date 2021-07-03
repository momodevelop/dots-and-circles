
#include <windows.h>
#include <windowsx.h>
#include <ShellScalingAPI.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <imm.h>

// NOTE(Momo): Because MS has baggage :(
#undef near
#undef far

#include "momo.h"

#include "game_config.h"
#include "game_renderer.h"
#include "game_platform.h"
#include "game_opengl.h"

#define STB_SPRINTF_IMPLEMENTATION
#include "stb_sprintf.h"

//~ WGL Bindings
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

#define WGL_FUNCTION_DECL(Name) wgl_func_##Name
#define WGL_FUNCTION_DECL_PTR(Name) WGL_FUNCTION_DECL(Name)* Name
typedef BOOL WINAPI 
WGL_FUNCTION_DECL(wglChoosePixelFormatARB)(HDC hdc,
                                           const int* piAttribIList,
                                           const FLOAT* pfAttribFList,
                                           UINT nMaxFormats,
                                           int* piFormats,
                                           UINT* nNumFormats);

typedef BOOL WINAPI 
WGL_FUNCTION_DECL(wglSwapIntervalEXT)(int interval);

typedef HGLRC WINAPI 
WGL_FUNCTION_DECL(wglCreateContextAttribsARB)(HDC hdc, 
                                              HGLRC hShareContext,
                                              const int* attribList);

typedef const char* WINAPI 
WGL_FUNCTION_DECL(wglGetExtensionsStringEXT)();

static WGL_FUNCTION_DECL_PTR(wglCreateContextAttribsARB);
static WGL_FUNCTION_DECL_PTR(wglChoosePixelFormatARB);
static WGL_FUNCTION_DECL_PTR(wglSwapIntervalEXT);
//static WGL_FUNCTION_DECL_PTR(wglGetExtensionsStringEXT);
//~ NOTE(Momo): Consts
#define WIN32_RECORD_STATE_FILE "record_state"
#define WIN32_RECORD_INPUT_FILE "record_input"
#define WIN32_SAVE_STATE_FILE "game_state"

//~ NOTE(Momo): Structs

struct Win32_Game_Memory {
    Game_Memory head;
    
    void* data;
    u32 data_size;
};

struct win32_state {
    Arena arena;
    b8 is_running;
    
    b8 is_recording_input;
    HANDLE recording_input_handle;
    b8 is_playback_input;
    HANDLE playback_input_handle;
    
    void* platform_memory_block;
    u32 platform_memory_block_size;
    
    
    // File paths 
    u32 performance_frequency;
    char exe_full_path[MAX_PATH];
    char* one_past_exe_dir;
    
    // Handle pool
    HANDLE handles[8];
    u32 handle_free_list[ARRAY_COUNT(handles)];
    u32 handle_free_count;
    
#if INTERNAL
    HANDLE std_out;
#endif
    
    Opengl* opengl;
    Win32_Game_Memory game_memory;
    
};


//~ Globals
win32_state* g_state = {};

//~ NOTE(Momo): Helper functions and globals


static inline u32
Win32_DetermineIdealRefreshRate(HWND Window, u32 DefaultRefreshRate) {
    // Do we want to cap this?
    HDC DeviceContext = GetDC(Window);
    defer { ReleaseDC(Window, DeviceContext); };
    
    u32 RefreshRate = DefaultRefreshRate;
    {
        s32 DisplayRefreshRate = GetDeviceCaps(DeviceContext, VREFRESH);
        // It is possible for the refresh rate to be 0 or less
        // because of something called 'adaptive vsync'
        if (DisplayRefreshRate > 1) {
            RefreshRate = DisplayRefreshRate;
        }
    }
    return RefreshRate;
    
}

static inline LARGE_INTEGER
Win32_FiletimeToLargeInt(FILETIME Filetime) {
    LARGE_INTEGER Ret = {};
    Ret.LowPart = Filetime.dwLowDateTime;
    Ret.HighPart = Filetime.dwHighDateTime;
    
    return Ret;
}

static inline LONG
RECT_Width(RECT Value) {
    return Value.right - Value.left;
}

static inline LONG
RECT_Height(RECT Value) {
    return Value.bottom - Value.top;
}


static inline FILETIME 
Win32_GetLastWriteTime(const char* Filename) {
    WIN32_FILE_ATTRIBUTE_DATA data;
    FILETIME LastWriteTime = {};
    
    if(GetFileAttributesEx(Filename, GetFileExInfoStandard, &data)) {
        LastWriteTime = data.ftLastWriteTime;
    }
    return LastWriteTime; 
}

#if INTERNAL
static inline void
Win32_WriteConsole(const char* Message) {
    WriteConsoleA(g_state->std_out,
                  Message, 
                  cstr_length(Message), 
                  0, 
                  NULL);
}
#endif

static inline
PLATFORM_LOG_DECL(Win32_Log) {
    char Buffer[256];
    
    va_list VaList;
    va_start(VaList, Format);
    
    stbsp_vsprintf(Buffer, Format, VaList);
    
#if INTERNAL
    Win32_WriteConsole(Buffer);
#endif
    // TODO: Logging to text file?
    va_end(VaList);
}

#if INTERNAL
static inline void*
Win32_AllocateMemoryAtAddress(usize MemorySize, LPVOID Address ) {
    return VirtualAllocEx(GetCurrentProcess(),
                          Address, 
                          MemorySize,
                          MEM_RESERVE | MEM_COMMIT, 
                          PAGE_READWRITE);
    
}
#endif

static inline void*
Win32_AllocateMemory(usize MemorySize) {
    return VirtualAllocEx(GetCurrentProcess(),
                          0, 
                          MemorySize,
                          MEM_RESERVE | MEM_COMMIT, 
                          PAGE_READWRITE);
    
}

static inline void
Win32_FreeMemory(void* Memory) {
    if(Memory) {
        VirtualFreeEx(GetCurrentProcess(), 
                      Memory,    
                      0, 
                      MEM_RELEASE); 
    }
}

static inline LARGE_INTEGER
Win32_GetPerformanceCounter(void) {
    LARGE_INTEGER Result;
    QueryPerformanceCounter(&Result);
    return Result;
}

//~ NOTE(Momo): Win32_ State related
static inline f32
Win32_GetSecondsElapsed(win32_state* State,
                        LARGE_INTEGER Start, 
                        LARGE_INTEGER End) 
{
    return (f32(End.QuadPart - Start.QuadPart)) / State->performance_frequency; 
}

static inline void
Win32_BuildExePathFilename(win32_state* State,
                           char* Dest, 
                           const char* Filename) {
    for(const char *Itr = State->exe_full_path; 
        Itr != State->one_past_exe_dir; 
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

static inline win32_state*
Win32_Init() {
    // This is kinda what we wanna do if we ever want Renderer 
    // to be its own DLL...
    // NOTE(Momo): Arena 
    u32 PlatformMemorySize = KIBIBYTES(256);
    void* PlatformMemory = Win32_AllocateMemory(PlatformMemorySize);
    if(!PlatformMemory) {
        Win32_Log("[Win32::State] Failed to allocate memory\n"); 
        return 0;
    } 
    win32_state* State = ARENA_BOOT_STRUCT(win32_state,
                                           arena,
                                           PlatformMemory, 
                                           PlatformMemorySize);
    if (!State) {
        Win32_Log("[Win32::State] Failed to allocate state\n"); 
        return 0;
    }
    
    // NOTE(Momo): Initialize paths
    GetModuleFileNameA(0, State->exe_full_path, 
                       sizeof(State->exe_full_path));
    State->one_past_exe_dir = State->exe_full_path;
    for( char* Itr = State->exe_full_path; *Itr; ++Itr) {
        if (*Itr == '\\') {
            State->one_past_exe_dir = Itr + 1;
        }
    }
    
    // NOTE(Momo): Performance Frequency 
    LARGE_INTEGER PerfCountFreq;
    QueryPerformanceFrequency(&PerfCountFreq);
    State->performance_frequency = u32(PerfCountFreq.QuadPart);
    
    // NOTE(Momo): Initialize file handle store
    //GlobalFileHandles = CreatePool<HANDLE>(&g_state->Arena, 8);
    for (u32 I = 0; I < ARRAY_COUNT(State->handles); ++I) {
        State->handle_free_list[I] = I;
    }
    State->handle_free_count = ARRAY_COUNT(State->handles);
    
#if INTERNAL
    // NOTE(Momo): Initialize console
    AllocConsole();    
    State->std_out = GetStdHandle(STD_OUTPUT_HANDLE);
    
#endif
    
    State->is_running = true;
    return State;
}


static inline void
Win32_Free(win32_state* State) {
    FreeConsole();
    Win32_FreeMemory(State); 
}

static inline void
Win32_BeginRecordingInput(win32_state* State, const char* Path) {
    ASSERT(!State->is_recording_input);
    HANDLE RecordFileHandle = CreateFileA(Path,
                                          GENERIC_WRITE,
                                          FILE_SHARE_WRITE,
                                          0,
                                          CREATE_ALWAYS,
                                          0,
                                          0);
    
    if (RecordFileHandle == INVALID_HANDLE_VALUE) {
        Win32_Log("[Win32::BeginRecordingInput] Cannot open file: %s\n", Path);
        return;
    }
    State->recording_input_handle = RecordFileHandle;
    State->is_recording_input = true;
    Win32_Log("[Win32::BeginRecordingInput] Recording has begun: %s\n", Path);
}

static inline void
Win32_EndRecordingInput(win32_state* State) {
    ASSERT(State->is_recording_input);
    CloseHandle(State->recording_input_handle);
    State->is_recording_input = false;
    Win32_Log("[Win32::EndRecordingInput] Recording has ended\n");
}

static inline void
Win32_RecordInput(win32_state* State, Platform_Input* Input) {
    ASSERT(State->is_recording_input);
    DWORD BytesWritten;
    if(!WriteFile(State->recording_input_handle,
                  Input,
                  sizeof(Platform_Input),
                  &BytesWritten, 0)) 
    {
        Win32_Log("[Win32::RecordInput] Cannot write file\n");
        Win32_EndRecordingInput(State);
        return;
    }
    
    if (BytesWritten != sizeof(Platform_Input)) {
        Win32_Log("[Win32::RecordInput] Did not complete writing\n");
        Win32_EndRecordingInput(State);
        return;
    }
}

static inline void 
Win32_EndPlaybackInput(win32_state* State) {
    ASSERT(State->is_playback_input);
    CloseHandle(State->playback_input_handle);
    State->is_playback_input = false;
    Win32_Log("[Win32::EndPlaybackInput] Playback has ended\n");
}

static inline void
Win32_BeginPlaybackInput(win32_state* State, const char* Path) {
    ASSERT(!State->is_playback_input);
    HANDLE RecordFileHandle = CreateFileA(Path,
                                          GENERIC_READ,
                                          FILE_SHARE_READ,
                                          0,
                                          OPEN_EXISTING,
                                          0,
                                          0);
    
    if (RecordFileHandle == INVALID_HANDLE_VALUE) {
        Win32_Log("[Win32::BeginPlaybackInput] Cannot open file: %s\n", Path);
        return;
    }
    State->playback_input_handle = RecordFileHandle;
    State->is_playback_input = true;
    Win32_Log("[Win32::BeginPlaybackInput] Playback has begun: %s\n", Path);
}

// NOTE(Momo): returns true if 'done' reading all input, false otherwise
static inline b8 
Win32_PlaybackInput(win32_state* State, Platform_Input* Input) {
    DWORD BytesRead;
    BOOL Success = ReadFile(State->playback_input_handle, 
                            Input,
                            sizeof(Platform_Input),
                            &BytesRead,
                            0);
    if(!Success || BytesRead != sizeof(Platform_Input)) {
        return true;
    }
    return false;
}

//~ NOTE(Momo): game code related
struct win32_game_code {
    HMODULE Dll;
    game_update* GameUpdate;
    FILETIME LastWriteTime;
    b8 IsValid;
    
    char SrcFileName[MAX_PATH];
    char TempFileName[MAX_PATH];
    char LockFileName[MAX_PATH];        
};


static inline win32_game_code 
Win32_InitGameCode(win32_state* State,
                   const char* SrcFileName,
                   const char* TempFileName,
                   const char* LockFileName) 
{
    win32_game_code Ret = {};
    Win32_BuildExePathFilename(State, Ret.SrcFileName, SrcFileName);
    Win32_BuildExePathFilename(State, Ret.TempFileName, TempFileName);
    Win32_BuildExePathFilename(State, Ret.LockFileName, LockFileName);
    
    return Ret;
}

static inline void
Win32_LoadGameCode(win32_game_code* Code) 
{
    WIN32_FILE_ATTRIBUTE_DATA Ignored; 
    if(!GetFileAttributesEx(Code->LockFileName, 
                            GetFileExInfoStandard, 
                            &Ignored)) 
    {
        Code->LastWriteTime = Win32_GetLastWriteTime(Code->SrcFileName);
        BOOL Success = FALSE;
        do {
            Success = CopyFile(Code->SrcFileName, Code->TempFileName, FALSE); 
        } while (!Success); 
        Code->Dll = LoadLibraryA(Code->TempFileName);
        if(Code->Dll) {
            Code->GameUpdate = 
                (game_update*)GetProcAddress(Code->Dll, "GameUpdate");
            Code->IsValid = (Code->GameUpdate != 0);
        }
    }
}

static inline void 
Win32_UnloadGameCode(win32_game_code* Code) {
    if (Code->Dll) {
        FreeLibrary(Code->Dll);
        Code->Dll = 0;
    }
    Code->IsValid = false;
    Code->GameUpdate = 0;
}

static inline b8
Win32_IsGameCodeOutdated(win32_game_code* Code) {    
    // Check last modified date
    FILETIME LastWriteTime = Win32_GetLastWriteTime(Code->SrcFileName);
    LARGE_INTEGER CurrentLastWriteTime = Win32_FiletimeToLargeInt(LastWriteTime); 
    LARGE_INTEGER GameCodeLastWriteTime = Win32_FiletimeToLargeInt(Code->LastWriteTime);
    
    return (CurrentLastWriteTime.QuadPart > GameCodeLastWriteTime.QuadPart); 
}


//~ NOTE(Momo) Opengl-related

static inline void
Win32_OpenglSetPixelFormat(HDC DeviceContext) {
    s32 SuggestedPixelFormatIndex = 0;
    u32 ExtendedPick = 0;
    
    if (wglChoosePixelFormatARB) {
        s32 IntAttribList[] = {
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


static inline b8
Win32_OpenglLoadWglExtensions() {
    WNDCLASSA WindowClass = {};
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
        defer { DestroyWindow(Window); };
        
        HDC Dc = GetDC(Window);
        defer { ReleaseDC(Window, Dc); };
        
        Win32_OpenglSetPixelFormat(Dc);
        
        HGLRC OpenglContext = wglCreateContext(Dc);
        defer { wglDeleteContext(OpenglContext); };
        
        
        if (wglMakeCurrent(Dc, OpenglContext)) {
            
#define Win32_SetWGL_FUNCTION_DECL(Name) \
Name = (WGL_FUNCTION_DECL(Name)*)wglGetProcAddress(#Name); \
if (!Name) { \
Win32_Log("[Win32::OpenGL] Cannot load wgl function: " #Name " \n"); \
return false; \
}
            
            Win32_SetWGL_FUNCTION_DECL(wglChoosePixelFormatARB);
            Win32_SetWGL_FUNCTION_DECL(wglCreateContextAttribsARB);
            Win32_SetWGL_FUNCTION_DECL(wglSwapIntervalEXT);
            
            wglMakeCurrent(0, 0);
            return true;
        }
        else {
            Win32_Log("[Win32::OpenGL] Cannot begin to load wgl extensions\n");
            return false;
        }
        
    }
    else {
        Win32_Log("[Win32::Opengl] Cannot register class to load wgl extensions\n");
        return false;
    }
}

static inline void* 
Win32_TryGetOpenglFunction(const char* Name, HMODULE FallbackModule)
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
OpenglDebugCallbackFunc(Win32_OpenglDebugCallback) {
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
    
    Win32_Log("[OpenGL] %d: %s of %s severity, raised from %s: %s\n",
              id, _type, _severity, _source, msg);
    
};
#endif

static inline b8
Win32_InitOpengl(win32_state* State,
                 HWND Window, 
                 v2u WindowDimensions) 
{
    HDC DeviceContext = GetDC(Window); 
    defer { ReleaseDC(Window, DeviceContext); };
    
    Opengl* opengl = State->arena.push_struct<Opengl>();
    
    if (!opengl) {
        Win32_Log("[Win32::Opengl] Failed to allocate opengl\n"); 
        return false;
    }
    
    if (!Win32_OpenglLoadWglExtensions()) {
        return false;
    }
    
    Win32_OpenglSetPixelFormat(DeviceContext);
    
    
    s32 Win32_OpenglAttribs[] {
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
    HGLRC OpenglContext = wglCreateContextAttribsARB(DeviceContext, 0, 
                                                     Win32_OpenglAttribs); 
    
    if (!OpenglContext) {
        //OpenglContext = wglCreateContext(DeviceContext);
        Win32_Log("[Win32::Opengl] Cannot create opengl context");
        return false;
    }
    
    if(wglMakeCurrent(DeviceContext, OpenglContext)) {
        HMODULE Module = LoadLibraryA("opengl32.dll");
        // TODO: Log functions that are not loaded
#define Win32_SetOpenglFunction(Name) \
opengl->Name = (OPENGL_FUNCTION_DECL(Name)*)Win32_TryGetOpenglFunction(#Name, Module); \
if (!opengl->Name) { \
Win32_Log("[Win32::Opengl] Cannot load opengl function '" #Name "' \n"); \
return false; \
}
        
        Win32_SetOpenglFunction(glEnable);
        Win32_SetOpenglFunction(glDisable); 
        Win32_SetOpenglFunction(glViewport);
        Win32_SetOpenglFunction(glScissor);
        Win32_SetOpenglFunction(glCreateShader);
        Win32_SetOpenglFunction(glCompileShader);
        Win32_SetOpenglFunction(glShaderSource);
        Win32_SetOpenglFunction(glAttachShader);
        Win32_SetOpenglFunction(glDeleteShader);
        Win32_SetOpenglFunction(glClear);
        Win32_SetOpenglFunction(glClearColor);
        Win32_SetOpenglFunction(glCreateBuffers);
        Win32_SetOpenglFunction(glNamedBufferStorage);
        Win32_SetOpenglFunction(glCreateVertexArrays);
        Win32_SetOpenglFunction(glVertexArrayVertexBuffer);
        Win32_SetOpenglFunction(glEnableVertexArrayAttrib);
        Win32_SetOpenglFunction(glVertexArrayAttribFormat);
        Win32_SetOpenglFunction(glVertexArrayAttribBinding);
        Win32_SetOpenglFunction(glVertexArrayBindingDivisor);
        Win32_SetOpenglFunction(glBlendFunc);
        Win32_SetOpenglFunction(glCreateProgram);
        Win32_SetOpenglFunction(glLinkProgram);
        Win32_SetOpenglFunction(glGetProgramiv);
        Win32_SetOpenglFunction(glGetProgramInfoLog);
        Win32_SetOpenglFunction(glVertexArrayElementBuffer);
        Win32_SetOpenglFunction(glCreateTextures);
        Win32_SetOpenglFunction(glTextureStorage2D);
        Win32_SetOpenglFunction(glTextureSubImage2D);
        Win32_SetOpenglFunction(glBindTexture);
        Win32_SetOpenglFunction(glTexParameteri);
        Win32_SetOpenglFunction(glBindVertexArray);
        Win32_SetOpenglFunction(glDrawElementsInstancedBaseInstance);
        Win32_SetOpenglFunction(glGetUniformLocation);
        Win32_SetOpenglFunction(glProgramUniformMatrix4fv);
        Win32_SetOpenglFunction(glNamedBufferSubData);
        Win32_SetOpenglFunction(glUseProgram);
        Win32_SetOpenglFunction(glDeleteTextures);
        Win32_SetOpenglFunction(glDebugMessageCallbackARB);
    }
    Opengl_Init(opengl, &State->arena, WindowDimensions);
    
#if INTERNAL
    opengl->glEnable(GL_DEBUG_OUTPUT);
    opengl->glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    opengl->glDebugMessageCallbackARB(Win32_OpenglDebugCallback, nullptr);
#endif
    
    State->opengl = opengl;
    return true;
}


static inline void
Win32_GameMemory_Save(Win32_Game_Memory* game_memory, const char* Path) {
    // We just dump the whole game memory into a file
    HANDLE Win32_Handle = CreateFileA(Path,
                                      GENERIC_WRITE,
                                      FILE_SHARE_WRITE,
                                      0,
                                      CREATE_ALWAYS,
                                      0,
                                      0);
    if (Win32_Handle == INVALID_HANDLE_VALUE) {
        Win32_Log("[Win32::SaveState] Cannot open file: %s\n", Path);
        return;
    }
    defer { CloseHandle(Win32_Handle); }; 
    
    DWORD BytesWritten;
    if(!WriteFile(Win32_Handle, 
                  game_memory->data,
                  (DWORD)game_memory->data_size,
                  &BytesWritten,
                  0)) 
    {
        Win32_Log("[Win32::SaveState] Cannot write file: %s\n", Path);
        return;
    }
    
    if (BytesWritten != game_memory->data_size) {
        Win32_Log("[Win32::SaveState] Did not complete writing: %s\n", Path);
        return;
    }
    Win32_Log("[Win32::SaveState] State saved: %s\n", Path);
    
}

static inline void
Win32_GameMemory_Load(Win32_Game_Memory* game_memory, const char* Path) {
    HANDLE Win32_Handle = CreateFileA(Path,
                                      GENERIC_READ,
                                      FILE_SHARE_READ,
                                      0,
                                      OPEN_EXISTING,
                                      0,
                                      0);
    if (Win32_Handle == INVALID_HANDLE_VALUE) {
        Win32_Log("[Win32::LoadState] Cannot open file: %s\n", Path);
        return;
    }
    defer { CloseHandle(Win32_Handle); }; 
    DWORD BytesRead;
    
    BOOL Success = ReadFile(Win32_Handle, 
                            game_memory->data,
                            (DWORD)game_memory->data_size,
                            &BytesRead,
                            0);
    
    if (Success && game_memory->data_size == BytesRead) {
        Win32_Log("[Win32::LoadState] State loaded from: %s\n", Path);
        return;
    }
    Win32_Log("[Win32::LoadState] Could not read all bytes: %s\n", Path);
}


//~ NOTE(Momo): Audio related
struct win32_audio {
    // Wasapi
    IAudioClient2* Client;
    IAudioRenderClient* RenderClient;
    
    // "Secondary" buffer
    u32 BufferSize;
    s16* Buffer;
    
    // Other variables for tracking purposes
    u32 LatencySampleCount;
    u32 SamplesPerSecond;
    u16 BitsPerSample;
    u16 channels;
    
};

static inline void
Win32_AudioFree(win32_audio* Audio) {
    Audio->Client->Stop();
    Audio->Client->Release();
    Audio->RenderClient->Release();
    Win32_FreeMemory(Audio->Buffer);
}


static inline b8
Win32_AudioInit(win32_audio* Audio,
                u32 SamplesPerSecond, 
                u16 BitsPerSample,
                u16 channels,
                u32 LatencyFrames,
                u32 RefreshRate)
{
    Audio->channels = channels;
    Audio->BitsPerSample = BitsPerSample;
    Audio->SamplesPerSecond = SamplesPerSecond;
    Audio->LatencySampleCount = (SamplesPerSecond / RefreshRate) * LatencyFrames;
    
    HRESULT Hr = CoInitializeEx(0, COINIT_SPEED_OVER_MEMORY);
    if (FAILED(Hr)) {
        Win32_Log("[Win32::Audio] Failed CoInitializeEx\n");
        return false;
    }
    
    IMMDeviceEnumerator* DeviceEnumerator;
    Hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), 
                          nullptr,
                          CLSCTX_ALL, 
                          __uuidof(IMMDeviceEnumerator),
                          (LPVOID*)(&DeviceEnumerator));
    
    if (FAILED(Hr)) {
        Win32_Log("[Win32::Audio] Failed to create IMMDeviceEnumerator\n");
        return false;
    }
    defer { DeviceEnumerator->Release(); };
    
    
    IMMDevice* Device;
    Hr = DeviceEnumerator->GetDefaultAudioEndpoint(eRender, 
                                                   eConsole, 
                                                   &Device);
    if (FAILED(Hr)) {
        Win32_Log("[Win32::Audio] Failed to get audio endpoint\n");
        return false;
    }
    defer { Device->Release(); };
    
    Hr = Device->Activate(__uuidof(IAudioClient2), 
                          CLSCTX_ALL, 
                          nullptr, 
                          (LPVOID*)&Audio->Client);
    if(FAILED(Hr)) {
        Win32_Log("[Win32::Audio] Failed to create IAudioClient\n");
        return false;
    }
    
    WAVEFORMATEX WaveFormat = {};
    WaveFormat.wFormatTag = WAVE_FORMAT_PCM;
    WaveFormat.wBitsPerSample = BitsPerSample;
    WaveFormat.nChannels = channels;
    WaveFormat.nSamplesPerSec = SamplesPerSecond;
    WaveFormat.nBlockAlign = (WaveFormat.nChannels * WaveFormat.wBitsPerSample / 8);
    WaveFormat.nAvgBytesPerSec = WaveFormat.nSamplesPerSec * WaveFormat.nBlockAlign;
    
    // buffer size in 100 nanoseconds
#if 0
    REFERENCE_TIME BufferDuration = 10000000ULL * Audio->BufferSize / SamplesPerSecond; 
#else
    
#if 0
    const int64_t REFTIMES_PER_SEC = 10000000; // hundred nanoseconds
    REFERENCE_TIME BufferDuration = REFTIMES_PER_SEC * 2;
#endif
    REFERENCE_TIME BufferDuration = 0;
    Hr = Audio->Client->GetDevicePeriod(nullptr, &BufferDuration);
    
#endif
    
    DWORD StreamFlags = ( AUDCLNT_STREAMFLAGS_RATEADJUST 
                         | AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM
                         | AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY );
    
    Hr = Audio->Client->Initialize(AUDCLNT_SHAREMODE_SHARED, 
                                   StreamFlags, 
                                   BufferDuration,
                                   0, 
                                   &WaveFormat, 
                                   nullptr);
    if (FAILED(Hr))
    {
        Win32_Log("[Win32::Audio] Failed to initialize audio client\n");
        return false;
    }
    
    if (FAILED(Audio->Client->GetService(__uuidof(IAudioRenderClient),
                                         (LPVOID*)(&Audio->RenderClient))))
    {
        Win32_Log("[Win32::Audio] Failed to create IAudioClient\n");
        return false;
    }
    
    UINT32 SoundFrameCount;
    Hr = Audio->Client->GetBufferSize(&SoundFrameCount);
    if (FAILED(Hr))
    {
        Win32_Log("[Win32::Audio] Failed to get buffer size\n");
        return false;
    }
    
    Audio->BufferSize = SoundFrameCount;
    Audio->Buffer = (s16*)Win32_AllocateMemory(Audio->BufferSize);
    if (!Audio->Buffer) {
        Win32_Log("[Win32::Audio] Failed to allocate secondary buffer\n");
        return false;
    }
    
    
    Win32_Log("[Win32::Audio] Loaded!\n");
    
    Audio->Client->Start();
    
    return true;
}

static inline Platform_Audio
Win32_AudioPrepare(win32_audio* Audio) {
    Platform_Audio Ret = {};
    
    UINT32 SoundPaddingSize;
    UINT32 SamplesToWrite = 0;
    
    // Padding is how much valid data is queued up in the sound buffer
    // if there's enough padding then we could skip writing more data
    HRESULT Hr = Audio->Client->GetCurrentPadding(&SoundPaddingSize);
    if (SUCCEEDED(Hr)) {
        SamplesToWrite = (UINT32)Audio->BufferSize - SoundPaddingSize;
        
        // Cap the samples to write to how much latency is allowed.
        if (SamplesToWrite > Audio->LatencySampleCount) {
            SamplesToWrite = Audio->LatencySampleCount;
        }
    }
    
    Ret.sample_buffer = Audio->Buffer;
    Ret.sample_count = SamplesToWrite; 
    Ret.channels = Audio->channels;
    
    return Ret;
}

static inline void
Win32_AudioFlush(win32_audio* Audio, 
                 Platform_Audio Output) 
{
    // NOTE(Momo): Kinda assumes 16-bit Sound
    BYTE* SoundBufferData;
    if (SUCCEEDED(Audio->RenderClient->GetBuffer((UINT32)Output.sample_count, &SoundBufferData))) 
    {
        s16* SrcSample = Output.sample_buffer;
        s16* DestSample = (s16*)SoundBufferData;
        // Buffer structure for stereo:
        // s16   s16    s16  s16   s16  s16
        // [LEFT RIGHT] LEFT RIGHT LEFT RIGHT....
        for(u32 I = 0; I < Output.sample_count; ++I ){
            for (u32 J = 0; J < Audio->channels; ++J) {
                *DestSample++ = *SrcSample++;
            }
            
        }
        
        Audio->RenderClient->ReleaseBuffer((UINT32)Output.sample_count, 0);
    }
}


static inline v2u
Win32_GetMonitorDimensions() {
    v2u Ret = {};
    Ret.w = u32(GetSystemMetrics(SM_CXSCREEN));
    Ret.h = u32(GetSystemMetrics(SM_CYSCREEN));
    return Ret;
}

static inline v2u
Win32_GetWindowDimensions(HWND Window) {
    RECT Rect = {};
    GetWindowRect(Window, &Rect);
    return { u16(Rect.right - Rect.left), u16(Rect.bottom - Rect.top) };
    
}

static inline v2u
Win32_GetClientDimensions(HWND Window) {
    RECT Rect = {};
    GetClientRect(Window, &Rect);
    return { u32(Rect.right - Rect.left), u32(Rect.bottom - Rect.top) };
    
}

static inline void
Win32_ProcessMessages(HWND Window, 
                      win32_state* state,
                      Platform_Input* input)
{
    MSG msg = {};
    while(PeekMessage(&msg, Window, 0, 0, PM_REMOVE)) {
        switch(msg.message) {
            case WM_QUIT:
            case WM_CLOSE: {
                state->is_running = false;
            } break;
            case WM_CHAR: {
                input->try_push_character_input((u8)msg.wParam);
            } break;
            case WM_MOUSEMOVE: {
                // NOTE(Momo): This is the actual conversion from screen space to 
                // design space. I'm not 100% if this should be here but I guess
                // only time will tell.
                input->window_mouse_pos.x = (f32)GET_X_LPARAM(msg.lParam);
                input->window_mouse_pos.y = (f32)GET_Y_LPARAM(msg.lParam);
                
                v2u WindowDims = state->opengl->window_dimensions;
                aabb2u RenderRegion = state->opengl->render_region;
                
                input->render_mouse_pos.x = input->window_mouse_pos.x - RenderRegion.min.x;
                input->render_mouse_pos.y = input->window_mouse_pos.y - RenderRegion.min.y;
                
                v2f DesignDimsF = to_v2f(state->opengl->design_dimensions);
                v2u RenderDimsU = dimensions(RenderRegion);
                v2f RenderDimsF = to_v2f(RenderDimsU);
                v2f DesignToRenderRatio = ratio(DesignDimsF, RenderDimsF);
                
                input->design_mouse_pos.x = input->render_mouse_pos.x * DesignToRenderRatio.w;
                input->design_mouse_pos.y = input->render_mouse_pos.y * DesignToRenderRatio.h;
                
            } break;
            case WM_LBUTTONUP:
            case WM_LBUTTONDOWN: {
                u32 Code = (u32)msg.wParam;
                b8 is_down = msg.message == WM_LBUTTONDOWN;
                input->button_switch.now = is_down;
            } break;
            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
            case WM_KEYDOWN:
            case WM_KEYUP: {
                u32 KeyCode = (u32)msg.wParam;
                b8 is_down = msg.message == WM_KEYDOWN;
                switch(KeyCode) {
                    case VK_RETURN:{
                        input->button_confirm.now = is_down;
                    } break;
                    case VK_F1:{
                        input->button_console.now = is_down;
                    } break;
                    case VK_F2:{
                        input->button_inspector.now = is_down;
                    } break;
                    case VK_F3:{
                        if (msg.message == WM_KEYDOWN) {
                            Win32_GameMemory_Save(&state->game_memory, WIN32_SAVE_STATE_FILE);
                        }
                    } break;
                    case VK_F4:{
                        if (msg.message == WM_KEYDOWN) {
                            Win32_GameMemory_Load(&state->game_memory, WIN32_SAVE_STATE_FILE);
                        }
                    } break;
                    case VK_F5:{
                        input->button_pause.now = is_down;
                    } break;
                    case VK_F6:{
                        if (msg.message == WM_KEYDOWN) {
                            if(state->is_recording_input) {
                                Win32_EndRecordingInput(state);
                            }
                            else {
                                Win32_GameMemory_Save(&state->game_memory, WIN32_RECORD_STATE_FILE);
                                Win32_BeginRecordingInput(state, WIN32_RECORD_INPUT_FILE);
                            }
                        }
                        
                    } break;
                    case VK_F7:{
                        if (msg.message == WM_KEYDOWN) {
                            if(state->is_playback_input) {
                                Win32_EndPlaybackInput(state);
                            }
                            else {
                                Win32_GameMemory_Load(&state->game_memory, WIN32_RECORD_STATE_FILE);
                                Win32_BeginPlaybackInput(state, 
                                                         WIN32_RECORD_INPUT_FILE);
                            }
                        }
                        
                    } break;
                    case VK_F11:{
                        input->button_speed_down.now = is_down;
                    } break;
                    case VK_F12: {
                        input->button_speed_up.now = is_down;
                    } break;
                    case VK_BACK:{
                        input->button_back.now = is_down;
                    } break;
                } 
                TranslateMessage(&msg);
            } break;
            default: 
            {
                //Win32_Log("[Win32::ProcessMessages] %d\n", msg.message);
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            } break;
        }
    }
}

LRESULT CALLBACK
Win32_WindowCallback(HWND Window, 
                     UINT Message, 
                     WPARAM WParam,
                     LPARAM LParam) 
{
    LRESULT Result = 0;
    switch(Message) {
        case WM_CLOSE: {
            g_state->is_running = false;
        } break;
        case WM_DESTROY: {
            g_state->is_running = false;
        } break;
        case WM_WINDOWPOSCHANGED: {
            Opengl* opengl = g_state->opengl;
            if(opengl && 
               opengl->is_initialized) 
            {
#if 0
                v2u WindowWH = Win32_GetWindowDimensions(Window);
                Win32_Log("[Win32::Resize] Window: %d x %d\n", WindowWH.w, WindowWH.h);
#endif
                
                v2u ClientWH = Win32_GetClientDimensions(Window);
                if (opengl->window_dimensions.w == ClientWH.w  &&
                    opengl->window_dimensions.h == ClientWH.h ) {
                    return Result;
                }
                Win32_Log("[Win32::Resize] Client: %d x %d\n", ClientWH.w, ClientWH.h);
                Opengl_Resize(opengl, (u16)ClientWH.w, (u16)ClientWH.h);
            }
        } break;
        default: {
            //TODO: Log message?
            Result = DefWindowProcA(Window, Message, WParam, LParam);
        };   
    }
    return Result;
}


static inline HWND 
Win32_CreateWindow(HINSTANCE Instance,
                   u32 WindowWidth,
                   u32 WindowHeight,
                   const char* Title) 
{
    WNDCLASSA WindowClass = {};
    WindowClass.style = CS_HREDRAW | CS_VREDRAW;
    WindowClass.lpfnWndProc = Win32_WindowCallback;
    WindowClass.hInstance = Instance;
    WindowClass.hCursor = LoadCursor(0, IDC_ARROW);
    WindowClass.lpszClassName = "MainWindowClass";
    
    if(!RegisterClassA(&WindowClass)) {
        Win32_Log("[Win32::Window] Failed to create class\n");
        return NULL;
    }
    
    HWND Window = {};
    RECT WindowRect = {};
    v2u MonitorDimensions = Win32_GetMonitorDimensions();
    WindowRect.left = MonitorDimensions.w / 2 - WindowWidth / 2;
    WindowRect.right = MonitorDimensions.w / 2 + WindowWidth / 2;
    WindowRect.top = MonitorDimensions.h / 2 - WindowHeight / 2;
    WindowRect.bottom = MonitorDimensions.h / 2 + WindowHeight / 2;
    
    DWORD Style = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
    AdjustWindowRectEx(&WindowRect,
                       Style,
                       FALSE,
                       0);
    
    // TODO: Adaptively create 'best' window resolution based on current desktop reso.
    Window = CreateWindowExA(
                             0,
                             WindowClass.lpszClassName,
                             Title,
                             Style,
                             WindowRect.left,
                             WindowRect.top,
                             RECT_Width(WindowRect),
                             RECT_Height(WindowRect),
                             0,
                             0,
                             Instance,
                             0);
    
    if (!Window) {
        Win32_Log("[Win32::Window] Failed to create window\n");
        return NULL;
    }
    Win32_Log("[Win32::Window] Window created successfully\n");
    v2u WindowWH = Win32_GetWindowDimensions(Window);
    v2u ClientWH = Win32_GetClientDimensions(Window);
    Win32_Log("[Win32::Window] Client: %d x %d\n", ClientWH.w, ClientWH.h);
    Win32_Log("[Win32::Window] Window: %d x %d\n", WindowWH.w, WindowWH.h);
    return Window;
    
}

static inline void
Win32_SwapBuffers(HWND Window) {
    HDC DeviceContext = GetDC(Window); 
    defer { ReleaseDC(Window, DeviceContext); };
    SwapBuffers(DeviceContext);
}

// Platform Functions ////////////////////////////////////////////////////
enum platform_file_error {
    PlatformFileError_None,
    PlatformFileError_NotEnoughHandlers,
    PlatformFileError_CannotOpenFile,   
    PlatformFileError_Closed,
    PlatformFileError_ReadFileFailed,
};

static inline
PLATFORM_GET_PERFORMANCE_COUNTER_DECL(Win32_GetPerformanceCounterU64) {
    return (u64)Win32_GetPerformanceCounter().QuadPart;
}

static inline 
PLATFORM_OPEN_ASSET_FILE_DECL(Win32_OpenAssetFile) {
    Platform_File_Handle Ret = {}; 
    const char* Path = Game_AssetFileName;
    
    // Check if there are free handlers to go around
    if (g_state->handle_free_count == 0) {
        Ret.error = PlatformFileError_NotEnoughHandlers;
        return Ret;
    }    
    
    HANDLE Win32_Handle = CreateFileA(Path, 
                                      GENERIC_READ, 
                                      FILE_SHARE_READ,
                                      0,
                                      OPEN_EXISTING,
                                      0,
                                      0);
    
    
    if(Win32_Handle == INVALID_HANDLE_VALUE) {
        Win32_Log("[Win32::OpenAssetFile] Cannot open file: %s\n", Path);
        Ret.error = PlatformFileError_CannotOpenFile;
        return Ret;
    } 
    
    u32 FreeSlotIndex = g_state->handle_free_list[g_state->handle_free_count-1];
    g_state->handles[FreeSlotIndex] = Win32_Handle;
    --g_state->handle_free_count;
    Ret.id = FreeSlotIndex;
    
    return Ret; 
}

static inline 
PLATFORM_LOG_FILE_ERROR_DECL(Win32_LogFileError) {
    switch(Handle->error) {
        case PlatformFileError_None: {
            Win32_Log("[Win32::File] There is no file error\n");
        } break;
        case PlatformFileError_NotEnoughHandlers: {
            Win32_Log("[Win32::File] There is not enough handlers\n");
        } break;
        case PlatformFileError_CannotOpenFile:{
            Win32_Log("[Win32::File] Cannot open file\n");
        } break;
        case PlatformFileError_Closed:{
            Win32_Log("[Win32::File] File is already closed\n");
        } break;
        case PlatformFileError_ReadFileFailed: {
            Win32_Log("[Win32::File] File read failed\n");
        } break;
        default: {
            Win32_Log("[Win32::File] Undefined error!\n");
        };
    }
}

static inline
PLATFORM_CLOSE_FILE_DECL(Win32_CloseFile) {
    ASSERT(Handle->id < ARRAY_COUNT(g_state->handles));
    HANDLE Win32_Handle = g_state->handles[Handle->id];
    if (Win32_Handle != INVALID_HANDLE_VALUE) {
        CloseHandle(Win32_Handle); 
    }
    g_state->handle_free_list[g_state->handle_free_count++] = Handle->id;
    ASSERT(g_state->handle_free_count <= ARRAY_COUNT(g_state->handles));
}
static inline
PLATFORM_ADD_TEXTURE_DECL(Win32_AddTexture) {
    return Opengl_AddTexture(g_state->opengl, Width, Height, Pixels);
}

static inline 
PLATFORM_CLEAR_TEXTURES_DECL(Win32_ClearTextures) {
    return Opengl_ClearTextures(g_state->opengl);
}

static inline 
PLATFORM_READ_FILE_DECL(Win32_ReadFile) {
    if (Handle->error) {
        return;
    }
    ASSERT(Handle->id < ARRAY_COUNT(g_state->handles));
    
    HANDLE Win32_Handle = g_state->handles[Handle->id];
    OVERLAPPED Overlapped = {};
    Overlapped.Offset = (u32)((Offset >> 0) & 0xFFFFFFFF);
    Overlapped.OffsetHigh = (u32)((Offset >> 32) & 0xFFFFFFFF);
    
    u32 FileSize32 = (u32)Size;
    DWORD BytesRead;
    if(ReadFile(Win32_Handle, Dest, FileSize32, &BytesRead, &Overlapped) &&
       FileSize32 == BytesRead) 
    {
        // success;
    }
    else {
        Handle->error = PlatformFileError_ReadFileFailed; 
    }
}

static inline 
PLATFORM_HIDE_CURSOR_DECL(Win32_HideCursor) {
    ShowCursor(FALSE);
}

static inline
PLATFORM_SHOW_CURSOR_DECL(Win32_ShowCursor) {
    ShowCursor(TRUE);
}

static inline
PLATFORM_GET_FILE_SIZE_DECL(Win32_GetFileSize) 
{
    HANDLE FileHandle = CreateFileA(Path, 
                                    GENERIC_READ, 
                                    FILE_SHARE_READ,
                                    0,
                                    OPEN_EXISTING,
                                    0,
                                    0);
    defer { CloseHandle(FileHandle); };
    
    if(FileHandle == INVALID_HANDLE_VALUE) {
        Win32_Log("[Win32::GetFileSize] Cannot open file: %s\n", Path);
        return 0;
    } else {
        LARGE_INTEGER FileSize;
        if (!GetFileSizeEx(FileHandle, &FileSize)) {
            Win32_Log("[Win32::GetFileSize] Problems getting file size: %s\n", Path);
            return 0;
        }
        
        return (u32)FileSize.QuadPart;
    }
}

static inline Platform_API
Win32_InitPlatformApi() {
    Platform_API PlatformApi = {};
    PlatformApi.log = Win32_Log;
    PlatformApi.read_file = Win32_ReadFile;
    PlatformApi.get_file_size = Win32_GetFileSize;
    PlatformApi.clear_textures = Win32_ClearTextures;
    PlatformApi.add_texture = Win32_AddTexture;
    PlatformApi.open_asset_file = Win32_OpenAssetFile;
    PlatformApi.close_file = Win32_CloseFile;
    PlatformApi.log_file_error = Win32_LogFileError;
    PlatformApi.show_cursor = Win32_ShowCursor;
    PlatformApi.hide_cursor = Win32_HideCursor;
    PlatformApi.get_performance_counter = Win32_GetPerformanceCounterU64;
    return PlatformApi;
}

static inline void
Win32_FreeGameMemory(Win32_Game_Memory* game_memory) {
    Win32_Log("[Win32::game_memory] Freed\n");
    Win32_FreeMemory(game_memory->data);
}

static inline b8
Win32_InitGameMemory(Win32_Game_Memory* game_memory,
                     u32 permanent_memory_size,
                     u32 transient_memory_size,
                     u32 scratch_memory_size,
                     u32 debug_memory_size) 
{
    game_memory->data_size = permanent_memory_size + transient_memory_size + scratch_memory_size + debug_memory_size;
    
#if INTERNAL
    SYSTEM_INFO SystemInfo;
    GetSystemInfo(&SystemInfo);
    
    game_memory->data = 
        Win32_AllocateMemoryAtAddress(game_memory->data_size, 
                                      SystemInfo.lpMinimumApplicationAddress);
#else
    game_memory->data = Win32_AllocateMemory(game_memory->data_size);
#endif
    if (!game_memory->data) {
        Win32_Log("[Win32::game_memory] Failed to allocate\n");
        return false;
    }
    
    u8* MemoryPtr = (u8*)game_memory->data;
    
    game_memory->head.permanent_memory_size = permanent_memory_size;
    game_memory->head.permanent_memory = game_memory->data;
    MemoryPtr += permanent_memory_size;
    
    game_memory->head.transient_memory_size = transient_memory_size;
    game_memory->head.transient_memory = MemoryPtr;;
    MemoryPtr += transient_memory_size;
    
    game_memory->head.scratch_memory_size = scratch_memory_size;
    game_memory->head.scratch_memory = MemoryPtr;
    MemoryPtr += scratch_memory_size;
    
    game_memory->head.debug_memory_size = debug_memory_size;
    game_memory->head.debug_memory = MemoryPtr;
    
    Win32_Log("[Win32::game_memory] Allocated\n");
    Win32_Log("[Win32::game_memory] Permanent Memory Size: %d bytes\n", permanent_memory_size);
    Win32_Log("[Win32::game_memory] Transient Memory Size: %d bytes\n", transient_memory_size);
    Win32_Log("[Win32::game_memory] Scratch Memory Size: %d bytes\n", scratch_memory_size);
    Win32_Log("[Win32::game_memory] Debug Memory Size: %d bytes\n", debug_memory_size);
    
    return true;
}

static inline void
Win32_FreeRenderCommands(Mailbox* RenderCommands) {
    Win32_Log("[Win32::RenderCommands] Freed\n"); 
    Win32_FreeMemory(RenderCommands->memory);
}


static inline b8
Win32_InitRenderCommands(Mailbox* RenderCommands,
                         u32 RenderCommandsMemorySize) {
    void* RenderCommandsMemory =
        Win32_AllocateMemory(RenderCommandsMemorySize); 
    if (!RenderCommandsMemory) {
        Win32_Log("[Win32::RenderCommands] Failed to allocate\n"); 
        return false;
    }
    RenderCommands->init(RenderCommandsMemory,
                         RenderCommandsMemorySize);
    Win32_Log("[Win32::RenderCommands] Allocated: %d bytes\n", RenderCommandsMemorySize);
    
    return true;
    
}

int CALLBACK
WinMain(HINSTANCE Instance,
        HINSTANCE PrevInstance,
        LPSTR CommandLine,
        int ShowCode)
{
    win32_state* State = Win32_Init();
    if (!State) {
        Win32_Log("[Win32::Main] Cannot initialize win32 state");
        return 1;
    }
    defer { Win32_Free(State); };
    g_state = State;
    
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    ImmDisableIME((DWORD)-1);
    HWND Window = Win32_CreateWindow(Instance, 
                                     Game_DesignWidth,
                                     Game_DesignHeight,
                                     "Dots and Circles");
    if (!Window) { 
        Win32_Log("[Win32::Main] Cannot create window");
        return 1; 
    }
    
    u32 RefreshRate = Win32_DetermineIdealRefreshRate(Window, 60); 
    f32 TargetSecsPerFrame = 1.f / RefreshRate; 
    Win32_Log("[Win32::Main] Target Secs Per Frame: %.2f\n", TargetSecsPerFrame);
    Win32_Log("[Win32::Main] Monitor Refresh Rate: %d Hz\n", RefreshRate);
    
    // Load the game code DLL
    win32_game_code GameCode = Win32_InitGameCode(State,
                                                  "compiled_game.dll", 
                                                  "active_game.dll", 
                                                  "lock");
    
    // Initialize game input
    Platform_Input GameInput = {};
    if(!GameInput.alloc(&State->arena)) {
        Win32_Log("[Win32::Main] Cannot initialize input");
        return 1;
    }
    
    
    // Initialize platform api
    Platform_API PlatformApi = Win32_InitPlatformApi();
    
    
    win32_audio Audio = {};
    if(!Win32_AudioInit(&Audio,
                        Game_AudioSamplesPerSecond,
                        Game_AudioBitsPerSample,
                        Game_AudioChannels,
                        Game_AudioLatencyFrames,
                        RefreshRate)) 
    {
        Win32_Log("[Win32::Main] Cannot initialize audio");
        return 1;
    }
    defer { Win32_AudioFree(&Audio); }; 
    
    // Initialize game memory
    if (!Win32_InitGameMemory(&State->game_memory,
                              MEBIBYTES(1),
                              MEBIBYTES(16),
                              MEBIBYTES(8),
                              MEBIBYTES(1))) 
    {
        Win32_Log("[Win32::Main] Cannot initialize game memory");
        return 1;
    }
    defer { Win32_FreeGameMemory(&State->game_memory); };
    
    // Initialize RenderCommands
    Mailbox RenderCommands = {};
    if(!Win32_InitRenderCommands(&RenderCommands, MEBIBYTES(64))) {
        Win32_Log("[Win32::Main] Cannot initialize render commands");
        return 1;
    }
    defer { Win32_FreeRenderCommands(&RenderCommands); };
    
    // Initialize OpenGL
    if(!Win32_InitOpengl(State,
                         Window, 
                         Win32_GetClientDimensions(Window)))
    {
        return 1;
    }
    
    
    // Set sleep granularity to 1ms
    b8 SleepIsGranular = timeBeginPeriod(1) == TIMERR_NOERROR;
    
    
    // Game Loop
    LARGE_INTEGER LastCount = Win32_GetPerformanceCounter(); 
    while (State->is_running) {
        if (Win32_IsGameCodeOutdated(&GameCode)) {
            Win32_Log("[Win32::Main] Reloading game code!\n");
            Win32_UnloadGameCode(&GameCode);
            Win32_LoadGameCode(&GameCode);
            zero_block(State->game_memory.head.transient_memory, 
                       State->game_memory.head.transient_memory_size);
        }
        
        
        GameInput.update();
        Win32_ProcessMessages(Window, 
                              State,
                              &GameInput);
        
        // NOTE(Momo): Recording/Playback input
        if (State->is_recording_input) {
            Win32_RecordInput(State, &GameInput);
        }
        if (State->is_playback_input) {
            // NOTE(Momo): This will actually modify GameInput
            if (Win32_PlaybackInput(State, &GameInput)) {
                Win32_EndPlaybackInput(State);
                Win32_GameMemory_Load(&State->game_memory, WIN32_RECORD_STATE_FILE);
                Win32_BeginPlaybackInput(State, WIN32_RECORD_INPUT_FILE);
            }
        }
        
        
        // Compute how much sound to write and where
        // TODO: Functionize this
        Platform_Audio GameAudioOutput = Win32_AudioPrepare(&Audio);
        
        if (GameCode.GameUpdate) 
        {
            f32 GameDeltaTime = TargetSecsPerFrame;
            b8 IsGameRunning = GameCode.GameUpdate(&State->game_memory.head,
                                                   &PlatformApi,
                                                   &RenderCommands,
                                                   &GameInput,
                                                   &GameAudioOutput,
                                                   GameDeltaTime);
            State->is_running = IsGameRunning && State->is_running;
        }
        
        
        
        Opengl_Render(State->opengl, &RenderCommands);
        RenderCommands.clear();
        
        Win32_AudioFlush(&Audio, GameAudioOutput);
        
        f32 SecsElapsed = 
            Win32_GetSecondsElapsed(State, LastCount, Win32_GetPerformanceCounter());
        
        // NOTE(Momo): Sleep time
        if (TargetSecsPerFrame > SecsElapsed) {
            if (SleepIsGranular) {
                DWORD MsToSleep = 
                    (DWORD)(1000.f * (TargetSecsPerFrame - SecsElapsed));
                
                // We cut the sleep some slack, so we sleep 1 sec less.
                if (MsToSleep > 1) {
                    Sleep(MsToSleep - 1);
                }
            }
            while(TargetSecsPerFrame > 
                  Win32_GetSecondsElapsed(State, LastCount, Win32_GetPerformanceCounter()));
            
        }
        
        LastCount = Win32_GetPerformanceCounter();
        
        Win32_SwapBuffers(Window);
    }
    
    return 0;
}



