#include <windows.h>
#include <ShellScalingAPI.h>
#include <mmdeviceapi.h>
#include <audioclient.h>

#include "mm_arena.h"
#include "mm_pool.h"
#include "mm_core.h"
#include "mm_maths.h"
#include "mm_string.h"
#include "mm_list.h"
#include "mm_mailbox.h"
#include "game_platform.h"
#include "game_opengl.h"

#define STB_SPRINTF_IMPLEMENTATION
#include "stb_sprintf.h"

// WGL Bindings
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

// Globals
b32 GlobalIsRunning;
opengl* GlobalOpengl;
u32 GlobalPerformanceFrequency;
char GlobalExeFullPath[MAX_PATH];
char* GlobalOnePastExeDirectory;
arena GlobalArena;
pool<HANDLE> GlobalFileHandles;


#if INTERNAL
HANDLE GlobalStdOut;
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
    if(Memory) {
        VirtualFreeEx(GetCurrentProcess(), 
                      Memory,    
                      0, 
                      MEM_RELEASE); 
    }
}


#if INTERNAL
static inline void
Win32WriteConsole(const char* Message) {
    WriteConsoleA(GlobalStdOut,
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
            GlobalPerformanceFrequency; 
}

static inline void
Win32BuildExePathFilename(char* Dest, const char* Filename) {
    for(const char *Itr = GlobalExeFullPath; 
        Itr != GlobalOnePastExeDirectory; 
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
    win32_game_code Ret = {};
    Win32BuildExePathFilename(Ret.SrcFileName, SrcFileName);
    Win32BuildExePathFilename(Ret.TempFileName, TempFileName);
    Win32BuildExePathFilename(Ret.LockFileName, LockFileName);
    
    return Ret;
}

static inline void
Load(win32_game_code* Code) 
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
Unload(win32_game_code* Code) {
    if (Code->Dll) {
        FreeLibrary(Code->Dll);
        Code->Dll = 0;
    }
    Code->IsValid = false;
    Code->GameUpdate = 0;
}

static inline b32
IsOutdated(win32_game_code* Code) {    
    // Check last modified date
    LARGE_INTEGER CurrentLastWriteTime = 
        FiletimeToLargeInt(Win32GetLastWriteTime(Code->SrcFileName)); 
    LARGE_INTEGER GameCodeLastWriteTime =
        FiletimeToLargeInt(Code->LastWriteTime);

    return (CurrentLastWriteTime.QuadPart > GameCodeLastWriteTime.QuadPart); 
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
    game_input GameInput = {};
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
                Win32Log("[Win32::OpenGL] Cannot load wgl function: " #Name " \n"); \
                return false; \
            }

            Win32SetWglFunction(wglChoosePixelFormatARB);
            Win32SetWglFunction(wglCreateContextAttribsARB);
            Win32SetWglFunction(wglSwapIntervalEXT);

            wglMakeCurrent(0, 0);
            return true;
        }
        else {
            Win32Log("[Win32::OpenGL] Cannot begin to load wgl extensions\n");
            return false;
        }

    }
    else {
        Win32Log("[Win32::Opengl] Cannot register class to load wgl extensions\n");
        return false;
    }
}

static inline u32
Win32DetermineIdealRefreshRate(HWND Window, u32 DefaultRefreshRate) {
    // Do we want to cap this?
    HDC DeviceContext = GetDC(Window);
    Defer { ReleaseDC(Window, DeviceContext); };

    u32 RefreshRate = DefaultRefreshRate;
    {
        i32 DisplayRefreshRate = GetDeviceCaps(DeviceContext, VREFRESH);
        // It is possible for the refresh rate to be 0 or less
        // because of something called 'adaptive vsync'
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
    Win32Log("[Win32::Opengl] Freeing opengl\n");
    Win32FreeMemory(Opengl);
}

static inline opengl*
Win32InitOpengl(HWND Window, 
                    v2u WindowDimensions,
                    usize ExtraMemory) 
{
    HDC DeviceContext = GetDC(Window); 
    Defer { ReleaseDC(Window, DeviceContext); };
    
    // This is kinda what we wanna do if we ever want Renderer 
    // to be its own DLL...
    usize RendererMemorySize = sizeof(opengl) + ExtraMemory; 
    void* RendererMemory = Win32AllocateMemory(RendererMemorySize);
    opengl* Opengl = BootstrapStruct(opengl,
                                     Arena,
                                     RendererMemory, 
                                     RendererMemorySize);

    if (!Opengl) {
        Win32Log("[Win32::Opengl] Failed to allocate\n"); 
        return nullptr;
    }

    Win32Log("[Win32::Opengl] Allocated: %d bytes\n", RendererMemorySize);

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
    HGLRC OpenglContext = wglCreateContextAttribsARB(DeviceContext, 0, 
                                                     Win32OpenglAttribs); 

    if (!OpenglContext) {
        //OpenglContext = wglCreateContext(DeviceContext);
        Win32Log("[Win32::Opengl] Cannot create opengl context");
        Win32FreeOpengl(Opengl);
        return nullptr;
    }

    if(wglMakeCurrent(DeviceContext, OpenglContext)) {
        HMODULE Module = LoadLibraryA("opengl32.dll");
        // TODO: Log functions that are not loaded
#define Win32SetOpenglFunction(Name) \
        Opengl->Name = (OpenglFunction(Name)*)Win32TryGetOpenglFunction(#Name, Module); \
        if (!Opengl->Name) { \
            Win32Log("[Win32::Opengl] Cannot load opengl function '" #Name "' \n"); \
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

static inline void
Win32InitGlobalPaths() {
    GetModuleFileNameA(0, GlobalExeFullPath, 
                       sizeof(GlobalExeFullPath));
    GlobalOnePastExeDirectory = GlobalExeFullPath;
    for( char* Itr = GlobalExeFullPath; *Itr; ++Itr) {
        if (*Itr == '\\') {
            GlobalOnePastExeDirectory = Itr + 1;
        }
    }
}

static inline void 
Win32InitGlobalPerformanceFrequency() {
    LARGE_INTEGER PerfCountFreq;
    QueryPerformanceFrequency(&PerfCountFreq);
    GlobalPerformanceFrequency = SafeCastU32(PerfCountFreq.QuadPart);
}

static inline void
Win32FreeGlobalArena() {
    Win32Log("[Win32::Arena] Freed platform arena\n");
    Win32FreeMemory(GlobalArena.Memory); 
}

static inline b32
Win32AllocateGlobalArena(u32 PlatformMemorySize) {
    void* PlatformMemory = Win32AllocateMemory(PlatformMemorySize);
    GlobalArena = Arena(PlatformMemory, PlatformMemorySize); 
    if(!PlatformMemory) {
        Win32Log("[Win32::Arena] Cannot allocate platform arena\n");
        return false;
    }
    Win32Log("[Win32::Arena] Allocated arena: %d bytes\n", PlatformMemorySize);
    return true;
}

struct win32_audio_output {
    usize BufferSize;
    i16* Buffer;

    u32 LatencySampleCount;
    u32 SamplesPerSecond;
    u16 BitsPerSample;
    u16 Channels;
};

static inline void
Win32FreeAudioOutput(win32_audio_output* AudioOutput) {
    Win32FreeMemory(AudioOutput->Buffer);
}

static inline maybe<win32_audio_output>
Win32InitAudioOutput(u32 SamplesPerSecond,
                     u16 Channels,
                     u16 BitsPerSample,
                     u32 LatencyFrames,
                     u32 RefreshRate)
{
    win32_audio_output Ret = {};
    Ret.Channels = Channels;
    Ret.BitsPerSample = BitsPerSample;
    Ret.SamplesPerSecond = SamplesPerSecond;
    Ret.LatencySampleCount = (SamplesPerSecond / RefreshRate) * LatencyFrames;
    Ret.BufferSize = SamplesPerSecond * (BitsPerSample / 8) * Channels;
    Ret.Buffer = (i16*)Win32AllocateMemory(Ret.BufferSize); 
    if (!Ret.Buffer) {
        Win32Log("[Win32::Audio] Failed to allocate secondary buffer\n");
        return No();
    }

    return Yes(Ret);
    
}

struct win32_wasapi {
    IMMDeviceEnumerator* Enumerator;
    IMMDevice* Device;
    IAudioClient* AudioClient;
    IAudioRenderClient* AudioRenderClient;
};

static inline void
Win32FreeWasapi(win32_wasapi* Wasapi) {
#define SAFE_RELEASE(Item) if(Item) (Item)->Release();
    SAFE_RELEASE(Wasapi->Enumerator);
    SAFE_RELEASE(Wasapi->AudioClient);
    SAFE_RELEASE(Wasapi->Device);
    SAFE_RELEASE(Wasapi->AudioRenderClient);
#undef SAFE_RELEASE
}
    

static inline maybe<win32_wasapi>
Win32InitWasapi(u32 SamplesPerSecond, 
                u16 BitsPerSample,
                u16 Channels,
                usize BufferSize) 
{
    win32_wasapi Ret = {};
    if (FAILED(CoInitializeEx(0, COINIT_SPEED_OVER_MEMORY))) {
        Win32Log("[Win32::Audio] Failed CoInitializeEx\n");
        return No();
    }


    if (FAILED(CoCreateInstance(__uuidof(MMDeviceEnumerator), 
                               NULL,
                               CLSCTX_ALL, 
                               IID_PPV_ARGS(&Ret.Enumerator))))
    {
        Win32Log("[Win32::Audio] Failed to create IMMDeviceEnumerator\n");
        return No();
    }

    if (FAILED(Ret.Enumerator->GetDefaultAudioEndpoint(
                    eRender, 
                    eConsole, 
                    &Ret.Device))) 
    {
        Win32Log("[Win32::Audio] Failed to get audio endpoint\n");
        return No();
    }

    if(FAILED(Ret.Device->Activate(__uuidof(IAudioClient), 
                               CLSCTX_ALL, 
                               NULL, 
                               (LPVOID*)&Ret.AudioClient))) {
        Win32Log("[Win32::Audio] Failed to create IAudioClient\n");
        return No();
    }

    WAVEFORMATEXTENSIBLE WaveFormat;
    WaveFormat.Format.cbSize = sizeof(WaveFormat);
    WaveFormat.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
    WaveFormat.Format.wBitsPerSample = BitsPerSample;
    WaveFormat.Format.nChannels = Channels;
    WaveFormat.Format.nSamplesPerSec = SamplesPerSecond;
    WaveFormat.Format.nBlockAlign = 
        (WaveFormat.Format.nChannels * WaveFormat.Format.wBitsPerSample / 8);
    WaveFormat.Format.nAvgBytesPerSec = 
        WaveFormat.Format.nSamplesPerSec * WaveFormat.Format.nBlockAlign;
    WaveFormat.Samples.wValidBitsPerSample = BitsPerSample;
    WaveFormat.dwChannelMask = KSAUDIO_SPEAKER_STEREO;
    WaveFormat.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
    

    // buffer size in 100 nanoseconds
    REFERENCE_TIME BufferDuration = 
        10000000ULL * BufferSize / SamplesPerSecond;         
    if (FAILED(Ret.AudioClient->Initialize(
                    AUDCLNT_SHAREMODE_SHARED, 
                    AUDCLNT_STREAMFLAGS_NOPERSIST, 
                    BufferDuration, 0, 
                    &WaveFormat.Format, nullptr)))
    {
        Win32Log("[Win32::Audio] Failed to initialize audio client\n");
        return No();
    }

    if (FAILED(Ret.AudioClient->GetService(IID_PPV_ARGS(&Ret.AudioRenderClient))))
    {
        Win32Log("[Win32::Audio] Failed to create IAudioClient\n");
        return No();
    }

    UINT32 SoundFrameCount;
    if (FAILED(Ret.AudioClient->GetBufferSize(&SoundFrameCount)))
    {
        Win32Log("[Win32::Audio] Failed to get buffer size\n");
        return No();
    }
    
    if (BufferSize != SoundFrameCount) {
        Win32Log("[Win32::Audio] Buffer size not expected\n");
        return No();
    }

    Win32Log("[Win32::Audio] Loaded!\n");
    return Yes(Ret);
}



static inline v2u
Win32GetMonitorDimensions() {
    v2u Ret = {};
    Ret.W = SafeCastU32(GetSystemMetrics(SM_CXSCREEN));
    Ret.H = SafeCastU32(GetSystemMetrics(SM_CYSCREEN));
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
                     game_input* Input)
{
    MSG Msg = {};
    while(PeekMessage(&Msg, Window, 0, 0, PM_REMOVE)) {
        switch(Msg.message) {
            case WM_CLOSE: {
                GlobalIsRunning = false;
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
            GlobalIsRunning = false;
        } break;
        case WM_DESTROY: {
            GlobalIsRunning = false;
        } break;
        case WM_WINDOWPOSCHANGED: {
            if(GlobalOpengl && 
               GlobalOpengl->Header.IsInitialized) 
            {
                v2u WindowWH = Win32GetWindowDimensions(Window);
                v2u ClientWH = Win32GetClientDimensions(Window);
                Win32Log("[Win32::Resize] Client: %d x %d\n", ClientWH.W, ClientWH.H);
                Win32Log("[Win32::Resize] Window: %d x %d\n", WindowWH.W, WindowWH.H);
                Resize(GlobalOpengl, (u16)ClientWH.W, (u16)ClientWH.H);
            }
        } break;

        default: {
            // Log message?
            Result = DefWindowProcA(Window, Message, WParam, LParam);
        };   
    }
    return Result;
}


static inline HWND 
Win32CreateWindow(HINSTANCE Instance,
                  u32 WindowWidth,
                  u32 WindowHeight,
                  const char* Title) 
{
    WNDCLASSA WindowClass = {};
    WindowClass.style = CS_HREDRAW | CS_VREDRAW;
    WindowClass.lpfnWndProc = Win32WindowCallback;
    WindowClass.hInstance = Instance;
    WindowClass.hCursor = LoadCursor(0, IDC_ARROW);
    WindowClass.lpszClassName = "MainWindowClass";

    if(!RegisterClassA(&WindowClass)) {
        Win32Log("[Win32::Window] Failed to create class\n");
        return NULL;
    }

    HWND Window = {};
    RECT WindowRect = {};
    v2u MonitorDimensions = Win32GetMonitorDimensions();
    WindowRect.left = MonitorDimensions.W / 2 - WindowWidth / 2;
    WindowRect.right = MonitorDimensions.W / 2 + WindowWidth / 2;
    WindowRect.top = MonitorDimensions.H / 2 - WindowHeight / 2;
    WindowRect.bottom = MonitorDimensions.H / 2 + WindowHeight / 2;
    
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
                Width(WindowRect),
                Height(WindowRect),
                0,
                0,
                Instance,
                0);

    if (!Window) {
        Win32Log("[Win32::Window] Failed to create window\n");
        return NULL;
    }
    Win32Log("[Win32::Window] Window created successfully\n");
    v2u WindowWH = Win32GetWindowDimensions(Window);
    v2u ClientWH = Win32GetClientDimensions(Window);
    Win32Log("[Win32::Window] Client: %d x %d\n", ClientWH.W, ClientWH.H);
    Win32Log("[Win32::Window] Window: %d x %d\n", WindowWH.W, WindowWH.H);
    return Window;

}

static inline void
Win32SwapBuffers(HWND Window) {
    HDC DeviceContext = GetDC(Window); 
    Defer { ReleaseDC(Window, DeviceContext); };
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
PlatformOpenAssetFileFunc(Win32OpenAssetFile) {
    platform_file_handle Ret = {}; 
    const char* Path = "yuu";
    
    // Check if there are free handlers to go around
    if (Remainder(&GlobalFileHandles) == 0) {
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
        Win32Log("[Win32::ReadFile] Cannot open file: %s\n", Path);
        Ret.Error = PlatformFileError_CannotOpenFile;
        return Ret;
    } 
    

    Ret.Id = (u32)Reserve(&GlobalFileHandles, Win32Handle);

    return Ret; 
}


static inline 
PlatformLogFileErrorFunc(Win32LogFileError) {
    switch(Handle->Error) {
        case PlatformFileError_None: {
            Win32Log("[Win32::File] There is no file error\n");
        } break;
        case PlatformFileError_NotEnoughHandlers: {
            Win32Log("[Win32::File] There is not enough handlers\n");
        } break;
        case PlatformFileError_CannotOpenFile:{
            Win32Log("[Win32::File] Cannot open file\n");
        } break;
        case PlatformFileError_Closed:{
            Win32Log("[Win32::File] File is already closed\n");
        } break;
        case PlatformFileError_ReadFileFailed: {
            Win32Log("[Win32::File] File read failed\n");
        } break;
        default: {
            Win32Log("[Win32::File] Undefined error!\n");
        };
    }
}

static inline
PlatformCloseFileFunc(Win32CloseFile) {
    HANDLE Win32Handle = GetCopy(&GlobalFileHandles, Handle->Id);
    if (Win32Handle != INVALID_HANDLE_VALUE) {
        CloseHandle(Win32Handle); 
    }
    Release(&GlobalFileHandles, Handle->Id);
}
static inline
PlatformAddTextureFunc(Win32AddTexture) {
    return AddTexture(GlobalOpengl, Width, Height, Pixels);
}

static inline 
PlatformClearTexturesFunc(Win32ClearTextures) {
    return ClearTextures(GlobalOpengl);
}

static inline 
PlatformReadFileFunc(Win32ReadFile) {
    if (Handle->Error) {
        return;
    }

    HANDLE Win32Handle = *(Get(&GlobalFileHandles, Handle->Id));
    OVERLAPPED Overlapped = {};
    Overlapped.Offset = (u32)((Offset >> 0) & 0xFFFFFFFF);
    Overlapped.OffsetHigh = (u32)((Offset >> 32) & 0xFFFFFFFF);

    u32 FileSize32 = (u32)Size;
    DWORD BytesRead;
    if(ReadFile(Win32Handle, Dest, FileSize32, &BytesRead, &Overlapped) &&
       FileSize32 == BytesRead) 
    {
        // success;
    }
    else {
        Handle->Error = PlatformFileError_ReadFileFailed; 
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
        Win32Log("[Win32::GetFileSize] Cannot open file: %s\n", Path);
        return 0;
    } else {
        LARGE_INTEGER FileSize;
        if (!GetFileSizeEx(FileHandle, &FileSize)) {
            Win32Log("[Win32::GetFileSize] Problems getting file size: %s\n", Path);
            return 0;
        }

        return (u32)FileSize.QuadPart;
    }
}

static inline platform_api
Win32InitPlatformApi() {
    platform_api PlatformApi = {};
    PlatformApi.Log = Win32Log;
    PlatformApi.ReadFile = Win32ReadFile;
    PlatformApi.GetFileSize = Win32GetFileSize;
    PlatformApi.ClearTextures = Win32ClearTextures;
    PlatformApi.AddTexture = Win32AddTexture;
    PlatformApi.OpenAssetFile = Win32OpenAssetFile;
    PlatformApi.CloseFile = Win32CloseFile;
    return PlatformApi;
}

static inline void
Win32FreeGameMemory(game_memory* GameMemory) {
    Win32FreeMemory(GameMemory->DebugMemory);
    Win32FreeMemory(GameMemory->PermanentMemory);
    Win32FreeMemory(GameMemory->TransientMemory);
    Win32Log("[Win32::GameMemory] Freed\n");
}

static inline maybe<game_memory>
Win32InitGameMemory(usize PermanentMemorySize,
                        usize TransientMemorySize,
                        usize DebugMemorySize) 
{
    game_memory GameMemory = {};
    GameMemory.PermanentMemorySize = PermanentMemorySize;
    GameMemory.PermanentMemory =                
        Win32AllocateMemory(PermanentMemorySize); 

    GameMemory.TransientMemorySize = TransientMemorySize;
    GameMemory.TransientMemory = 
        Win32AllocateMemory(TransientMemorySize);

    GameMemory.DebugMemorySize = DebugMemorySize;
    GameMemory.DebugMemory =
        Win32AllocateMemory(DebugMemorySize);

    if (!GameMemory.PermanentMemory ||
        !GameMemory.TransientMemory ||
        !GameMemory.DebugMemory) {
        Win32FreeGameMemory(&GameMemory);
        Win32Log("[Win32::GameMemory] Failed to allocate\n");
        return No();
    }
    Win32Log("[Win32::GameMemory] Allocated\n");
    Win32Log("[Win32::GameMemory] Permanent Memory Size: %d bytes\n", PermanentMemorySize);
    Win32Log("[Win32::GameMemory] Transient Memory Size: %d bytes\n", TransientMemorySize);
    Win32Log("[Win32::GameMemory] Debug Memory Size: %d bytes\n", DebugMemorySize);

    return Yes(GameMemory); 
}

static inline void
Win32FreeRenderCommands(mailbox* RenderCommands) {
    Win32Log("[Win32::RenderCommands] Freed\n"); 
    Win32FreeMemory(RenderCommands->Memory);
}


static inline maybe<mailbox>
Win32InitRenderCommands(usize RenderCommandsMemorySize) {
    void* RenderCommandsMemory =
        Win32AllocateMemory(RenderCommandsMemorySize); 
    if (!RenderCommandsMemory) {
        Win32Log("[Win32::RenderCommands] Failed to allocate\n"); 
        return No();
    }
    mailbox RenderCommands = Mailbox(RenderCommandsMemory,
                                     RenderCommandsMemorySize);
    Win32Log("[Win32::RenderCommands] Allocated: %d bytes\n", RenderCommandsMemorySize);

    return Yes(RenderCommands);

}


int CALLBACK
WinMain(HINSTANCE Instance,
        HINSTANCE PrevInstance,
        LPSTR CommandLine,
        int ShowCode)
{
#if INTERNAL
    // Initialize console
    AllocConsole();    
    Defer { FreeConsole(); };
    GlobalStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
#endif
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    GlobalIsRunning = true;

    Win32InitGlobalPaths();
    Win32InitGlobalPerformanceFrequency();
    
    if (!Win32AllocateGlobalArena(Kilobytes(256))) {
        return 1;
    }
    Defer { Win32FreeGlobalArena(); };

    // Initialize file handle store
    GlobalFileHandles = Pool<HANDLE>(&GlobalArena, 8);

    HWND Window = Win32CreateWindow(Instance, 
                                   (u32)Global_DesignSpace.W,
                                   (u32)Global_DesignSpace.H,
                                   "Dots and Circles");
    if (!Window) { return 1; }
   
    u32 RefreshRate = Win32DetermineIdealRefreshRate(Window, 60); 
    f32 TargetSecsPerFrame = 1.f / RefreshRate; 
    Win32Log("[Win32::Main] Target Secs Per Frame: %.2f\n", TargetSecsPerFrame);
    Win32Log("[Win32::Main] Monitor Refresh Rate: %d Hz\n", RefreshRate);

    // Load the game code DLL
    win32_game_code GameCode = 
        Win32GameCode("game.dll", "temp_game.dll", "lock");
    
    // Initialize game input
    game_input GameInput = Input(StringBuffer(&GlobalArena, 10));

    // Initialize platform api
    platform_api PlatformApi = Win32InitPlatformApi();

    // Initialize Audio-related stuff
    maybe<win32_audio_output> AudioOutput_ = 
        Win32InitAudioOutput(48000,
                             2,
                             16, // Can only be 8 or 16
                             2,  // Latency frames
                             RefreshRate);
    if (!AudioOutput_) { return 1; }
    win32_audio_output& AudioOutput = AudioOutput_.This;
    Defer{ Win32FreeAudioOutput(&AudioOutput); };

    // TODO: Should really initialize from Audio Ouput
    maybe<win32_wasapi> Wasapi_ = 
        Win32InitWasapi(AudioOutput.SamplesPerSecond,
                        AudioOutput.BitsPerSample,
                        AudioOutput.Channels,
                        AudioOutput.BufferSize); 
    if (!Wasapi_) { return 1; }
    win32_wasapi& Wasapi = Wasapi_.This;
    Defer { Win32FreeWasapi(&Wasapi); }; 
    Wasapi.AudioClient->Start();

    // Initialize game memory
    maybe<game_memory> GameMemory_ = 
        Win32InitGameMemory(Megabytes(256),
                            Megabytes(256),
                            Megabytes(256));
    if (!GameMemory_) {
        return 1;
    }
    game_memory& GameMemory = GameMemory_.This;
    Defer { Win32FreeGameMemory(&GameMemory); };


    // Initialize RenderCommands
    maybe<mailbox> RenderCommands_ = 
        Win32InitRenderCommands(Megabytes(64));
    if (!RenderCommands_) {
        return 1;
    }
    mailbox& RenderCommands = RenderCommands_.This;
    Defer { Win32FreeRenderCommands(&RenderCommands); };

    // Initialize OpenGL
    GlobalOpengl = Win32InitOpengl(Window, 
                                   Win32GetClientDimensions(Window),
                                   Kilobytes(128));
    if (!GlobalOpengl) {
        return 1;
    }
    Defer { Win32FreeOpengl(GlobalOpengl); };

    // Set sleep granularity to 1ms
    b32 SleepIsGranular = timeBeginPeriod(1) == TIMERR_NOERROR;


    // Game Loop
    LARGE_INTEGER LastCount = Win32GetCurrentCounter(); 
    while (GlobalIsRunning) {
        if (IsOutdated(&GameCode)) {
            Win32Log("[Win32] Reloading game code!\n");
            Unload(&GameCode);
            Load(&GameCode);
        }

        Update(&GameInput);
        Win32ProcessMessages(Window, 
                             &GameInput);

        // Compute how much sound to write and where
        // TODO: Functionize this
        game_audio GameAudio = {};
        {
            UINT32 SoundPaddingSize;
            UINT32 SamplesToWrite = 0;
            if (SUCCEEDED(Wasapi.AudioClient->
                    GetCurrentPadding(&SoundPaddingSize))) {
                SamplesToWrite = 
                    (UINT32)AudioOutput.BufferSize - SoundPaddingSize;

                // Cap the samples to write to how much latency is allowed.
                if (SamplesToWrite > AudioOutput.LatencySampleCount) {
                    SamplesToWrite = AudioOutput.LatencySampleCount;
                }
            }
            GameAudio.SampleBuffer = AudioOutput.Buffer;
            GameAudio.SampleCount = SamplesToWrite; 
        }

        if (GameCode.GameUpdate) 
        {
            GameCode.GameUpdate(&GameMemory,
                                &PlatformApi,
                                &RenderCommands,
                                &GameInput,
                                &GameAudio,
                                TargetSecsPerFrame);
        }


        Render(GlobalOpengl, &RenderCommands);
        Clear(&RenderCommands);
       
        // TODO: Functionize this
        {
            BYTE* SoundBufferData;
            if (SUCCEEDED(Wasapi.AudioRenderClient->
                        GetBuffer((UINT32)GameAudio.SampleCount, &SoundBufferData))) 
            {
                i16* SrcSample = AudioOutput.Buffer;
                i16* DestSample = (i16*)SoundBufferData;
                // Buffer structure:
                // i16   i16    i16  i16   i16  i16
                // [LEFT RIGHT] LEFT RIGHT LEFT RIGHT....
                for(usize I = 0; I < GameAudio.SampleCount; ++I ){
                    *DestSample++ = *SrcSample++; // Left
                    *DestSample++ = *SrcSample++; // Right
                }

                Wasapi.AudioRenderClient->ReleaseBuffer(
                        (UINT32)GameAudio.SampleCount, 0);
            }

        }

        f32 SecsElapsed = 
            Win32GetSecondsElapsed(LastCount, Win32GetCurrentCounter());
        
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
                  Win32GetSecondsElapsed(LastCount, Win32GetCurrentCounter()));

        }
        
        LastCount = Win32GetCurrentCounter();
        Win32SwapBuffers(Window);
    }

    return 0;
}



