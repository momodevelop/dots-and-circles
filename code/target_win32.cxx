
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



//~
#define WIN32_RECORD_STATE_FILE "record_state"
#define WIN32_RECORD_INPUT_FILE "record_input"
#define WIN32_SAVE_STATE_FILE "game_state"


struct Win32_State {
    Arena arena;
    b8 is_running;
    
    
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
    
    Opengl* opengl;
    
};
Win32_State* g_state = {};

#if INTERNAL
HANDLE g_std_out;
static inline void
win32_write_console(const char* Message) {
    WriteConsoleA(g_std_out,
                  Message, 
                  cstr_length(Message), 
                  0, 
                  NULL);
}
#endif

//~ NOTE(Momo): Helper functions that extends winapi
static inline
PLATFORM_LOG_DECL(win32_log) {
    char buffer[256];
    
    va_list valist;
    va_start(valist, format);
    
    stbsp_vsprintf(buffer, format, valist);
    
#if INTERNAL
    win32_write_console(buffer);
#endif
    // TODO: Logging to text file?
    va_end(valist);
}



static inline u32
win32_determine_ideal_refresh_rate(HWND window, u32 default_refresh_rate) {
    // Do we want to cap this?
    HDC dc = GetDC(window);
    defer { ReleaseDC(window, dc); };
    
    u32 refresh_rate = default_refresh_rate;
    {
        s32 display_refresh_rate = GetDeviceCaps(dc, VREFRESH);
        // It is possible for the refresh rate to be 0 or less
        // because of something called 'adaptive vsync'
        if (display_refresh_rate > 1) {
            refresh_rate = display_refresh_rate;
        }
    }
    return refresh_rate;
    
}

static inline LARGE_INTEGER
win32_file_time_to_large_int(FILETIME file_time) {
    LARGE_INTEGER ret = {};
    ret.LowPart = file_time.dwLowDateTime;
    ret.HighPart = file_time.dwHighDateTime;
    
    return ret;
}

static inline LONG
width(RECT value) {
    return value.right - value.left;
}

static inline LONG
height(RECT value) {
    return value.bottom - value.top;
}


static inline FILETIME 
win32_get_file_last_write_time(const char* filename) {
    WIN32_FILE_ATTRIBUTE_DATA data;
    FILETIME last_write_time = {};
    
    if(GetFileAttributesEx(filename, GetFileExInfoStandard, &data)) {
        last_write_time = data.ftLastWriteTime;
    }
    return last_write_time; 
}


#if INTERNAL
static inline void*
win32_allocate_memory_at_address(umi memory_size, LPVOID address ) {
    return VirtualAllocEx(GetCurrentProcess(),
                          address, 
                          memory_size,
                          MEM_RESERVE | MEM_COMMIT, 
                          PAGE_READWRITE);
    
}
#endif

static inline void*
win32_allocate_memory(umi memory_size) {
    return VirtualAllocEx(GetCurrentProcess(),
                          0, 
                          memory_size,
                          MEM_RESERVE | MEM_COMMIT, 
                          PAGE_READWRITE);
    
}

static inline void
win32_free_memory(void* memory) {
    if(memory) {
        VirtualFreeEx(GetCurrentProcess(), 
                      memory,    
                      0, 
                      MEM_RELEASE); 
    }
}

static inline LARGE_INTEGER
win32_get_performance_counter(void) {
    LARGE_INTEGER result;
    QueryPerformanceCounter(&result);
    return result;
}

#include "target_win32_game_memory.h"
#include "target_win32_record_playback.h"
#include "target_win32_game_code.h"

Win32_Game_Memory* g_game_memory = {};
Win32_Input_Recorder* g_input_recorder;
Win32_Input_Playbacker* g_input_playbacker;




//~ NOTE(Momo): Win32 state related
static inline f32
win32_get_seconds_elapsed(Win32_State* state,
                          LARGE_INTEGER start, 
                          LARGE_INTEGER end) 
{
    return (f32(end.QuadPart - start.QuadPart)) / state->performance_frequency; 
}

static inline void
win32_build_exe_path_filename(Win32_State* state,
                              char* dest, 
                              const char* filename) {
    for(const char *itr = state->exe_full_path; 
        itr != state->one_past_exe_dir; 
        ++itr, ++dest) 
    {
        (*dest) = (*itr);
    }
    
    for (const char* itr = filename;
         (*itr) != 0;
         ++itr, ++dest) 
    {
        (*dest) = (*itr);
    }
    
    (*dest) = 0;
}

static inline Win32_State*
win32_init() {
    // This is kinda what we wanna do if we ever want Renderer 
    // to be its own DLL...
    // NOTE(Momo): Arena 
    u32 platform_memory_size = KIBIBYTES(256);
    void* platform_memory = win32_allocate_memory(platform_memory_size);
    if(!platform_memory) {
        win32_log("[Win32::state] Failed to allocate memory\n"); 
        return 0;
    } 
    Win32_State* state = ARENA_BOOT_STRUCT(Win32_State,
                                           arena,
                                           platform_memory, 
                                           platform_memory_size);
    if (!state) {
        win32_log("[Win32::state] Failed to allocate state\n"); 
        return 0;
    }
    
    // NOTE(Momo): initialize paths
    GetModuleFileNameA(0, state->exe_full_path, 
                       sizeof(state->exe_full_path));
    state->one_past_exe_dir = state->exe_full_path;
    for( char* itr = state->exe_full_path; *itr; ++itr) {
        if (*itr == '\\') {
            state->one_past_exe_dir = itr + 1;
        }
    }
    
    // NOTE(Momo): Performance Frequency 
    LARGE_INTEGER perf_count_freq;
    QueryPerformanceFrequency(&perf_count_freq);
    state->performance_frequency = u32(perf_count_freq.QuadPart);
    
    // NOTE(Momo): initialize file handle store
    //Globalfile_handles = CreatePool<HANDLE>(&g_state->Arena, 8);
    for (u32 i = 0; i < ARRAY_COUNT(state->handles); ++i) {
        state->handle_free_list[i] = i;
    }
    state->handle_free_count = ARRAY_COUNT(state->handles);
    
#if INTERNAL
    // NOTE(Momo): initialize console
    AllocConsole();    
    g_std_out = GetStdHandle(STD_OUTPUT_HANDLE);
#endif
    
    state->is_running = true;
    return state;
}


static inline void
win32_free(Win32_State* state) {
    FreeConsole();
    win32_free_memory(state); 
}

static inline void
win32_swap_buffers(HWND window) {
    HDC DeviceContext = GetDC(window); 
    defer { ReleaseDC(window, DeviceContext); };
    SwapBuffers(DeviceContext);
}




//~ NOTE(Momo) Opengl-related

static inline void
win32_opengl_set_pixel_format(HDC dc) {
    s32 suggested_pixel_format_index = 0;
    u32 extended_pick = 0;
    
    if (wglChoosePixelFormatARB) {
        s32 attrib_list[] = {
            WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
            WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
            WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
            WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
            WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
            WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB, GL_TRUE,
            0,
        };
        
        wglChoosePixelFormatARB(dc, attrib_list, 0, 1,
                                &suggested_pixel_format_index, &extended_pick);
        
    }
    
    if (!extended_pick) {
        PIXELFORMATDESCRIPTOR desired_pixel_format = {};
        desired_pixel_format.nSize = sizeof(desired_pixel_format);
        desired_pixel_format.nVersion = 1;
        desired_pixel_format.iPixelType = PFD_TYPE_RGBA;
        desired_pixel_format.dwFlags = 
            PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER; 
        desired_pixel_format.cColorBits = 32;
        desired_pixel_format.cAlphaBits = 8;
        desired_pixel_format.iLayerType = PFD_MAIN_PLANE;
        
        // Here, we ask windows to find the best supported pixel 
        // format based on our desired format.
        suggested_pixel_format_index = 
            ChoosePixelFormat(dc, &desired_pixel_format);
    }
    PIXELFORMATDESCRIPTOR suggested_pixel_format = {};
    
    DescribePixelFormat(dc, suggested_pixel_format_index, 
                        sizeof(suggested_pixel_format), 
                        &suggested_pixel_format);
    SetPixelFormat(dc, 
                   suggested_pixel_format_index, 
                   &suggested_pixel_format);
}


static inline b8
win32_opengl_load_wgl_extensions() {
    WNDCLASSA window_class = {};
    // Er yeah...we have to create a 'fake' Opengl context 
    // to load the extensions lol.
    window_class.lpfnWndProc = DefWindowProcA;
    window_class.hInstance = GetModuleHandle(0);
    window_class.lpszClassName = "WGLLoader";
    
    if (RegisterClassA(&window_class)) {
        HWND window = CreateWindowExA( 
                                      0,
                                      window_class.lpszClassName,
                                      "WGL Loader",
                                      0,
                                      CW_USEDEFAULT,
                                      CW_USEDEFAULT,
                                      CW_USEDEFAULT,
                                      CW_USEDEFAULT,
                                      0,
                                      0,
                                      window_class.hInstance,
                                      0);
        defer { DestroyWindow(window); };
        
        HDC dc = GetDC(window);
        defer { ReleaseDC(window, dc); };
        
        win32_opengl_set_pixel_format(dc);
        
        HGLRC opengl_context = wglCreateContext(dc);
        defer { wglDeleteContext(opengl_context); };
        
        
        if (wglMakeCurrent(dc, opengl_context)) {
            
#define WIN32_SET_WGL_FUNCTION_DECL(Name) \
Name = (WGL_FUNCTION_DECL(Name)*)wglGetProcAddress(#Name); \
if (!Name) { \
win32_log("[Win32::OpenGL] Cannot load wgl function: " #Name " \n"); \
return false; \
}
            
            WIN32_SET_WGL_FUNCTION_DECL(wglChoosePixelFormatARB);
            WIN32_SET_WGL_FUNCTION_DECL(wglCreateContextAttribsARB);
            WIN32_SET_WGL_FUNCTION_DECL(wglSwapIntervalEXT);
            
            wglMakeCurrent(0, 0);
            return true;
        }
        else {
            win32_log("[Win32::OpenGL] Cannot begin to load wgl extensions\n");
            return false;
        }
        
    }
    else {
        win32_log("[Win32::Opengl] Cannot register class to load wgl extensions\n");
        return false;
    }
}

static inline void* 
win32_try_get_opengl_function(const char* name, HMODULE fallback_module)
{
    void* p = (void*)wglGetProcAddress(name);
    if ((p == 0) || 
        (p == (void*)0x1) || 
        (p == (void*)0x2) || 
        (p == (void*)0x3) || 
        (p == (void*)-1))
    {
        p = (void*)GetProcAddress(fallback_module, name);
    }
    return p;
    
}

#if INTERNAL
static inline
OPENGL_DEBUG_CALLBACK_FUNC(win32_opengl_debug_callback) {
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
    
    win32_log("[OpenGL] %d: %s of %s severity, raised from %s: %s\n",
              id, _type, _severity, _source, msg);
    
};
#endif

static inline b8
win32_init_opengl(Win32_State* state,
                  HWND window, 
                  v2u window_dimensions) 
{
    HDC DeviceContext = GetDC(window); 
    defer { ReleaseDC(window, DeviceContext); };
    
    Opengl* opengl = state->arena.push_struct<Opengl>();
    
    if (!opengl) {
        win32_log("[Win32::Opengl] Failed to allocate opengl\n"); 
        return false;
    }
    
    if (!win32_opengl_load_wgl_extensions()) {
        return false;
    }
    
    win32_opengl_set_pixel_format(DeviceContext);
    
    
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
        win32_log("[Win32::Opengl] Cannot create opengl context");
        return false;
    }
    
    if(wglMakeCurrent(DeviceContext, OpenglContext)) {
        HMODULE Module = LoadLibraryA("opengl32.dll");
        // TODO: Log functions that are not loaded
#define Win32_SetOpenglFunction(Name) \
opengl->Name = (OPENGL_FUNCTION_DECL(Name)*)win32_try_get_opengl_function(#Name, Module); \
if (!opengl->Name) { \
win32_log("[Win32::Opengl] Cannot load opengl function '" #Name "' \n"); \
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
    opengl->init(&state->arena, window_dimensions.w, window_dimensions.h);
    
#if INTERNAL
    opengl->glEnable(GL_DEBUG_OUTPUT);
    opengl->glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    opengl->glDebugMessageCallbackARB(win32_opengl_debug_callback, nullptr);
#endif
    
    state->opengl = opengl;
    return true;
}


//~ NOTE(Momo): audio related
struct Win32_Audio {
    // Wasapi
    IAudioClient2* client;
    IAudioRenderClient* render_client;
    
    // "Secondary" buffer
    u32 buffer_size;
    s16* buffer;
    
    // Other variables for tracking purposes
    u32 latency_sample_count;
    u32 samples_per_second;
    u16 bits_per_sample;
    u16 channels;
    
};

static inline void
win32_audio_free(Win32_Audio* audio) {
    audio->client->Stop();
    audio->client->Release();
    audio->render_client->Release();
    win32_free_memory(audio->buffer);
}


static inline b8
win32_audio_init(Win32_Audio* audio,
                 u32 samples_per_second, 
                 u16 bits_per_sample,
                 u16 channels,
                 u32 latency_frames,
                 u32 refresh_rate)
{
    audio->channels = channels;
    audio->bits_per_sample = bits_per_sample;
    audio->samples_per_second = samples_per_second;
    audio->latency_sample_count = (samples_per_second / refresh_rate) * latency_frames;
    
    HRESULT hr = CoInitializeEx(0, COINIT_SPEED_OVER_MEMORY);
    if (FAILED(hr)) {
        win32_log("[Win32::Audio] Failed CoInitializeEx\n");
        return false;
    }
    
    IMMDeviceEnumerator* device_enum;
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), 
                          nullptr,
                          CLSCTX_ALL, 
                          __uuidof(IMMDeviceEnumerator),
                          (LPVOID*)(&device_enum));
    
    if (FAILED(hr)) {
        win32_log("[Win32::Audio] Failed to create IMMDeviceEnumerator\n");
        return false;
    }
    defer { device_enum->Release(); };
    
    
    IMMDevice* Device;
    hr = device_enum->GetDefaultAudioEndpoint(eRender, 
                                              eConsole, 
                                              &Device);
    if (FAILED(hr)) {
        win32_log("[Win32::Audio] Failed to get audio endpoint\n");
        return false;
    }
    defer { Device->Release(); };
    
    hr = Device->Activate(__uuidof(IAudioClient2), 
                          CLSCTX_ALL, 
                          nullptr, 
                          (LPVOID*)&audio->client);
    if(FAILED(hr)) {
        win32_log("[Win32::Audio] Failed to create IAudioClient\n");
        return false;
    }
    
    WAVEFORMATEX wave_format = {};
    wave_format.wFormatTag = WAVE_FORMAT_PCM;
    wave_format.wBitsPerSample = bits_per_sample;
    wave_format.nChannels = channels;
    wave_format.nSamplesPerSec = samples_per_second;
    wave_format.nBlockAlign = (wave_format.nChannels * wave_format.wBitsPerSample / 8);
    wave_format.nAvgBytesPerSec = wave_format.nSamplesPerSec * wave_format.nBlockAlign;
    
    // buffer size in 100 nanoseconds
#if 0
    REFERENCE_TIME buffer_duration = 10000000ULL * audio->buffer_size / samples_per_second; 
#else
    
#if 0
    const int64_t REFTIMES_PER_SEC = 10000000; // hundred nanoseconds
    REFERENCE_TIME buffer_duration = REFTIMES_PER_SEC * 2;
#endif
    REFERENCE_TIME buffer_duration = 0;
    hr = audio->client->GetDevicePeriod(nullptr, &buffer_duration);
    
#endif
    
    DWORD stream_flags = ( AUDCLNT_STREAMFLAGS_RATEADJUST 
                          | AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM
                          | AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY );
    
    hr = audio->client->Initialize(AUDCLNT_SHAREMODE_SHARED, 
                                   stream_flags, 
                                   buffer_duration,
                                   0, 
                                   &wave_format, 
                                   nullptr);
    if (FAILED(hr))
    {
        win32_log("[Win32::audio] Failed to initialize audio client\n");
        return false;
    }
    
    if (FAILED(audio->client->GetService(__uuidof(IAudioRenderClient),
                                         (LPVOID*)(&audio->render_client))))
    {
        win32_log("[Win32::audio] Failed to create IAudioClient\n");
        return false;
    }
    
    UINT32 sound_frame_count;
    hr = audio->client->GetBufferSize(&sound_frame_count);
    if (FAILED(hr))
    {
        win32_log("[Win32::audio] Failed to get buffer size\n");
        return false;
    }
    
    audio->buffer_size = sound_frame_count;
    audio->buffer = (s16*)win32_allocate_memory(audio->buffer_size);
    if (!audio->buffer) {
        win32_log("[Win32::audio] Failed to allocate secondary buffer\n");
        return false;
    }
    
    
    win32_log("[Win32::audio] Loaded!\n");
    
    audio->client->Start();
    
    return true;
}

static inline Platform_Audio
win32_audio_prepare(Win32_Audio* audio) {
    Platform_Audio ret = {};
    
    UINT32 sound_padding_size;
    UINT32 samples_to_write = 0;
    
    // Padding is how much valid data is queued up in the sound buffer
    // if there's enough padding then we could skip writing more data
    HRESULT hr = audio->client->GetCurrentPadding(&sound_padding_size);
    if (SUCCEEDED(hr)) {
        samples_to_write = (UINT32)audio->buffer_size - sound_padding_size;
        
        // Cap the samples to write to how much latency is allowed.
        if (samples_to_write > audio->latency_sample_count) {
            samples_to_write = audio->latency_sample_count;
        }
    }
    
    ret.sample_buffer = audio->buffer;
    ret.sample_count = samples_to_write; 
    ret.channels = audio->channels;
    
    return ret;
}

static inline void
win32_flush_audio(Win32_Audio* audio, 
                  Platform_Audio output) 
{
    // NOTE(Momo): Kinda assumes 16-bit Sound
    BYTE* sound_buffer_data;
    if (SUCCEEDED(audio->render_client->GetBuffer((UINT32)output.sample_count, &sound_buffer_data))) 
    {
        s16* src_sample = output.sample_buffer;
        s16* dest_sample = (s16*)sound_buffer_data;
        // buffer structure for stereo:
        // s16   s16    s16  s16   s16  s16
        // [LEFT RIGHT] LEFT RIGHT LEFT RIGHT....
        for(u32 I = 0; I < output.sample_count; ++I ){
            for (u32 J = 0; J < audio->channels; ++J) {
                *dest_sample++ = *src_sample++;
            }
            
        }
        
        audio->render_client->ReleaseBuffer((UINT32)output.sample_count, 0);
    }
}


static inline v2u
win32_get_monitor_dimensions() {
    v2u ret = {};
    ret.w = u32(GetSystemMetrics(SM_CXSCREEN));
    ret.h = u32(GetSystemMetrics(SM_CYSCREEN));
    return ret;
}

static inline v2u
win32_get_window_dimensions(HWND window) {
    RECT rect = {};
    GetWindowRect(window, &rect);
    return { u16(rect.right - rect.left), u16(rect.bottom - rect.top) };
    
}

static inline v2u
win32_get_client_dimensions(HWND window) {
    RECT rect = {};
    GetClientRect(window, &rect);
    return { u32(rect.right - rect.left), u32(rect.bottom - rect.top) };
    
}

static inline void
win32_process_messages(HWND window, 
                       Win32_State* state,
                       Platform_Input* input,
                       Win32_Game_Memory* game_memory,
                       Win32_Input_Recorder* input_recorder,
                       Win32_Input_Playbacker * input_playbacker)
{
    MSG msg = {};
    while(PeekMessage(&msg, window, 0, 0, PM_REMOVE)) {
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
                            game_memory->save(WIN32_SAVE_STATE_FILE);
                        }
                    } break;
                    case VK_F4:{
                        if (msg.message == WM_KEYDOWN) {
                            game_memory->save(WIN32_SAVE_STATE_FILE);
                        }
                    } break;
                    case VK_F5:{
                        input->button_pause.now = is_down;
                    } break;
                    case VK_F6:{
                        if (msg.message == WM_KEYDOWN) {
                            if(input_recorder->is_recording_input) {
                                input_recorder->end();
                            }
                            else {
                                game_memory->save(WIN32_RECORD_STATE_FILE);
                                input_recorder->begin(WIN32_RECORD_INPUT_FILE);
                            }
                        }
                        
                    } break;
                    case VK_F7:{
                        if (msg.message == WM_KEYDOWN) {
                            if(input_playbacker->is_playback_input) {
                                input_playbacker->end();
                            }
                            else {
                                game_memory->load(WIN32_RECORD_STATE_FILE);
                                input_playbacker->begin(WIN32_RECORD_INPUT_FILE);
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
                //win32_log("[Win32::ProcessMessages] %d\n", msg.message);
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            } break;
        }
    }
}

LRESULT CALLBACK
Win32_WindowCallback(HWND window, 
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
                v2u WindowWH = win32_get_window_dimensions(window);
                win32_log("[Win32::Resize] window: %d x %d\n", WindowWH.w, WindowWH.h);
#endif
                
                v2u ClientWH = win32_get_client_dimensions(window);
                if (opengl->window_dimensions.w == ClientWH.w  &&
                    opengl->window_dimensions.h == ClientWH.h ) {
                    return Result;
                }
                win32_log("[Win32::Resize] client: %d x %d\n", ClientWH.w, ClientWH.h);
                opengl->resize((u16)ClientWH.w, (u16)ClientWH.h);
            }
        } break;
        default: {
            //TODO: Log message?
            Result = DefWindowProcA(window, Message, WParam, LParam);
        };   
    }
    return Result;
}


static inline HWND 
win32_create_window(HINSTANCE instance,
                    u32 window_width,
                    u32 window_height,
                    const char* Title) 
{
    WNDCLASSA window_class = {};
    window_class.style = CS_HREDRAW | CS_VREDRAW;
    window_class.lpfnWndProc = Win32_WindowCallback;
    window_class.hInstance = instance;
    window_class.hCursor = LoadCursor(0, IDC_ARROW);
    window_class.lpszClassName = "MainWindowClass";
    
    if(!RegisterClassA(&window_class)) {
        win32_log("[Win32::window] Failed to create class\n");
        return NULL;
    }
    
    HWND window = {};
    RECT WindowRect = {};
    v2u monitor_dimensions = win32_get_monitor_dimensions();
    WindowRect.left = monitor_dimensions.w / 2 - window_width / 2;
    WindowRect.right = monitor_dimensions.w / 2 + window_width / 2;
    WindowRect.top = monitor_dimensions.h / 2 - window_height / 2;
    WindowRect.bottom = monitor_dimensions.h / 2 + window_height / 2;
    
    DWORD Style = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
    AdjustWindowRectEx(&WindowRect,
                       Style,
                       FALSE,
                       0);
    
    // TODO: Adaptively create 'best' window resolution based on current desktop reso.
    window = CreateWindowExA(
                             0,
                             window_class.lpszClassName,
                             Title,
                             Style,
                             WindowRect.left,
                             WindowRect.top,
                             width(WindowRect),
                             height(WindowRect),
                             0,
                             0,
                             instance,
                             0);
    
    if (!window) {
        win32_log("[Win32::window] Failed to create window\n");
        return NULL;
    }
    win32_log("[Win32::window] window created successfully\n");
    v2u WindowWH = win32_get_window_dimensions(window);
    v2u ClientWH = win32_get_client_dimensions(window);
    win32_log("[Win32::window] client: %d x %d\n", ClientWH.w, ClientWH.h);
    win32_log("[Win32::window] window: %d x %d\n", WindowWH.w, WindowWH.h);
    return window;
    
}

// Platform Functions ////////////////////////////////////////////////////
enum Win32_File_Error_Type {
    WIN32_FILE_ERROR_NONE,
    WIN32_FILE_ERROR_NO_HANDLERS,
    WIN32_FILE_ERROR_CANNOT_OPEN,   
    WIN32_FILE_ERROR_ALREADY_CLOSED,
    WIN32_FILE_ERROR_READ_FILE_FAILED,
};

static inline
PLATFORM_GET_PERFORMANCE_COUNTER_DECL(win32_get_performance_counter_u64) {
    return (u64)win32_get_performance_counter().QuadPart;
}

static inline 
PLATFORM_OPEN_ASSET_FILE_DECL(win32_open_asset_file) {
    Platform_File_Handle ret = {}; 
    const char* Path = GAME_ASSET_FILENAME;
    
    // Check if there are free handlers to go around
    if (g_state->handle_free_count == 0) {
        ret.error = WIN32_FILE_ERROR_NO_HANDLERS;
        return ret;
    }    
    
    HANDLE Win32_Handle = CreateFileA(Path, 
                                      GENERIC_READ, 
                                      FILE_SHARE_READ,
                                      0,
                                      OPEN_EXISTING,
                                      0,
                                      0);
    
    
    if(Win32_Handle == INVALID_HANDLE_VALUE) {
        win32_log("[Win32::OpenAssetFile] Cannot open file: %s\n", Path);
        ret.error = WIN32_FILE_ERROR_CANNOT_OPEN;
        return ret;
    } 
    
    u32 FreeSlotIndex = g_state->handle_free_list[g_state->handle_free_count-1];
    g_state->handles[FreeSlotIndex] = Win32_Handle;
    --g_state->handle_free_count;
    ret.id = FreeSlotIndex;
    
    return ret; 
}

static inline 
PLATFORM_LOG_FILE_ERROR_DECL(win32_log_file_error) {
    switch(handle->error) {
        case WIN32_FILE_ERROR_NONE: {
            win32_log("[Win32::File] There is no file error\n");
        } break;
        case WIN32_FILE_ERROR_NO_HANDLERS: {
            win32_log("[Win32::File] There is not enough handlers\n");
        } break;
        case WIN32_FILE_ERROR_CANNOT_OPEN:{
            win32_log("[Win32::File] Cannot open file\n");
        } break;
        case WIN32_FILE_ERROR_ALREADY_CLOSED:{
            win32_log("[Win32::File] File is already closed\n");
        } break;
        case WIN32_FILE_ERROR_READ_FILE_FAILED: {
            win32_log("[Win32::File] File read failed\n");
        } break;
        default: {
            win32_log("[Win32::File] Undefined error!\n");
        };
    }
}

static inline
PLATFORM_CLOSE_FILE_DECL(win32_close_file) {
    ASSERT(handle->id < ARRAY_COUNT(g_state->handles));
    HANDLE win32_handle = g_state->handles[handle->id];
    if (win32_handle != INVALID_HANDLE_VALUE) {
        CloseHandle(win32_handle); 
    }
    g_state->handle_free_list[g_state->handle_free_count++] = handle->id;
    ASSERT(g_state->handle_free_count <= ARRAY_COUNT(g_state->handles));
}
static inline
PLATFORM_ADD_TEXTURE_DECL(win32_add_texture) {
    return g_state->opengl->add_texture(width, height, pixels);
}

static inline 
PLATFORM_CLEAR_TEXTURES_DECL(win32_clear_textures) {
    return g_state->opengl->clear_textures();
}

static inline 
PLATFORM_READ_FILE_DECL(win32_read_file) {
    if (handle->error) {
        return;
    }
    ASSERT(handle->id < ARRAY_COUNT(g_state->handles));
    
    HANDLE win32_handle = g_state->handles[handle->id];
    OVERLAPPED overlapped = {};
    overlapped.Offset = (u32)((offset >> 0) & 0xFFFFFFFF);
    overlapped.OffsetHigh = (u32)((offset >> 32) & 0xFFFFFFFF);
    
    u32 file_size_32 = (u32)size;
    DWORD bytes_read;
    if(ReadFile(win32_handle, dest, file_size_32, &bytes_read, &overlapped) &&
       file_size_32 == bytes_read) 
    {
        // success;
    }
    else {
        handle->error = WIN32_FILE_ERROR_READ_FILE_FAILED; 
    }
}

static inline 
PLATFORM_HIDE_CURSOR_DECL(win32_hide_cursor) {
    ShowCursor(FALSE);
}

static inline
PLATFORM_SHOW_CURSOR_DECL(win32_show_cursor) {
    ShowCursor(TRUE);
}

static inline
PLATFORM_GET_FILE_SIZE_DECL(win32_get_file_size) 
{
    HANDLE file_handle = CreateFileA(path, 
                                     GENERIC_READ, 
                                     FILE_SHARE_READ,
                                     0,
                                     OPEN_EXISTING,
                                     0,
                                     0);
    defer { CloseHandle(file_handle); };
    
    if(file_handle == INVALID_HANDLE_VALUE) {
        win32_log("[Win32::GetFileSize] Cannot open file: %s\n", path);
        return 0;
    } else {
        LARGE_INTEGER file_size;
        if (!GetFileSizeEx(file_handle, &file_size)) {
            win32_log("[Win32::GetFileSize] Problems getting file size: %s\n", path);
            return 0;
        }
        
        return (u32)file_size.QuadPart;
    }
}

static inline Platform_API
win32_init_platform_api() {
    Platform_API platform_api = {};
    
    platform_api.log = win32_log;
    platform_api.read_file = win32_read_file;
    platform_api.get_file_size = win32_get_file_size;
    platform_api.clear_textures = win32_clear_textures;
    platform_api.add_texture = win32_add_texture;
    platform_api.open_asset_file = win32_open_asset_file;
    platform_api.close_file = win32_close_file;
    platform_api.log_file_error = win32_log_file_error;
    platform_api.show_cursor = win32_show_cursor;
    platform_api.hide_cursor = win32_hide_cursor;
    platform_api.get_performance_counter = win32_get_performance_counter_u64;
    
    return platform_api;
}

static inline void
win32_free_render_commands(Mailbox* render_commands) {
    win32_log("[Win32::render_commands] Freed\n"); 
    win32_free_memory(render_commands->memory);
}


static inline b8
win32_init_render_commands(Mailbox* render_commands,
                           u32 RenderCommandsMemorySize) {
    void* RenderCommandsMemory =
        win32_allocate_memory(RenderCommandsMemorySize); 
    if (!RenderCommandsMemory) {
        win32_log("[Win32::render_commands] Failed to allocate\n"); 
        return false;
    }
    render_commands->init(RenderCommandsMemory,
                          RenderCommandsMemorySize);
    win32_log("[Win32::render_commands] Allocated: %d bytes\n", RenderCommandsMemorySize);
    
    return true;
    
}

int CALLBACK
WinMain(HINSTANCE instance,
        HINSTANCE prev_instance,
        LPSTR command_line,
        int show_code)
{
    Win32_State* state = win32_init();
    if (!state) {
        win32_log("[Win32::Main] Cannot initialize win32 state");
        return 1;
    }
    defer { win32_free(state); };
    g_state = state;
    
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    ImmDisableIME((DWORD)-1);
    HWND window = win32_create_window(instance, 
                                      GAME_DESIGN_WIDTH,
                                      GAME_DESIGN_HEIGHT,
                                      "Dots and Circles");
    if (!window) { 
        win32_log("[Win32::Main] Cannot create window");
        return 1; 
    }
    
    u32 refresh_rate = win32_determine_ideal_refresh_rate(window, 60); 
    f32 target_secs_per_frame = 1.f / refresh_rate; 
    win32_log("[Win32::Main] Target Secs Per Frame: %.2f\n", target_secs_per_frame);
    win32_log("[Win32::Main] Monitor Refresh Rate: %d Hz\n", refresh_rate);
    
    // Load the game code DLL
    Win32_Game_Code game_code = {};
    game_code.init("compiled_game.dll", 
                   "active_game.dll", 
                   "lock");
    
    // Initialize game input
    Platform_Input game_input = {};
    if(!game_input.alloc(&state->arena)) {
        win32_log("[Win32::Main] Cannot initialize input");
        return 1;
    }
    
    
    // Initialize platform api
    Platform_API platform_api = win32_init_platform_api();
    
    
    Win32_Audio audio = {};
    if(!win32_audio_init(&audio,
                         GAME_AUDIO_SAMPLES_PER_SECOND,
                         GAME_AUDIO_BITS_PER_SAMPLE,
                         GAME_AUDIO_CHANNELS,
                         GAME_AUDIO_LATENCY_FRAMES,
                         refresh_rate)) 
    {
        win32_log("[Win32::Main] Cannot initialize audio");
        return 1;
    }
    defer { win32_audio_free(&audio); }; 
    
    // NOTE(Momo): Initialize game memory
    Win32_Game_Memory game_memory = {};
    if (!game_memory.init(MEBIBYTES(1),
                          MEBIBYTES(16),
                          MEBIBYTES(8),
                          MEBIBYTES(1))) 
    {
        win32_log("[Win32::Main] Cannot initialize game memory");
        return 1;
    }
    defer { game_memory.free(); };
    g_game_memory = &game_memory;
    
    // NOTE(Momo): Initialize record and playback
    Win32_Input_Recorder input_recorder = {};
    Win32_Input_Playbacker input_playbacker = {};
    g_input_recorder = &input_recorder;
    g_input_playbacker = &input_playbacker;
    
    
    
    // NOTE(Momo): Initialize render_commands
    Mailbox render_commands = {};
    if(!win32_init_render_commands(&render_commands, MEBIBYTES(64))) {
        win32_log("[Win32::Main] Cannot initialize render commands");
        return 1;
    }
    defer { win32_free_render_commands(&render_commands); };
    
    // Initialize OpenGL
    if(!win32_init_opengl(state,
                          window, 
                          win32_get_client_dimensions(window)))
    {
        return 1;
    }
    
    
    // Set sleep granularity to 1ms
    b8 is_sleep_granular = timeBeginPeriod(1) == TIMERR_NOERROR;
    
    
    // Game Loop
    LARGE_INTEGER last_count = win32_get_performance_counter(); 
    while (state->is_running) {
        if (game_code.is_outdated()) {
            win32_log("[Win32::Main] Reloading game code!\n");
            game_code.unload();
            game_code.load();
            zero_block(game_memory.head.transient_memory, 
                       game_memory.head.transient_memory_size);
        }
        
        
        game_input.update();
        win32_process_messages(window, 
                               state,
                               &game_input,
                               &game_memory,
                               &input_recorder,
                               &input_playbacker);
        
        // NOTE(Momo): Recording/Playback input
        if (input_recorder.is_recording_input) {
            input_recorder.update(&game_input);
        }
        if (input_playbacker.is_playback_input) {
            // NOTE(Momo): This will actually modify game_input
            if (input_playbacker.update(&game_input)) {
                input_playbacker.end();
                game_memory.load(WIN32_RECORD_STATE_FILE);
                input_playbacker.begin(WIN32_RECORD_INPUT_FILE);
            }
        }
        
        
        // Compute how much sound to write and where
        // TODO: Functionize this
        Platform_Audio platform_audio_output = win32_audio_prepare(&audio);
        
        if (game_code.game_update) 
        {
            f32 game_dt = target_secs_per_frame;
            b8 is_game_running = game_code.game_update(&game_memory.head,
                                                       &platform_api,
                                                       &render_commands,
                                                       &game_input,
                                                       &platform_audio_output,
                                                       game_dt);
            state->is_running = is_game_running && state->is_running;
        }
        
        
        
        state->opengl->render(&render_commands);
        render_commands.clear();
        
        win32_flush_audio(&audio, platform_audio_output);
        
        f32 secs_elapsed = 
            win32_get_seconds_elapsed(state, last_count, win32_get_performance_counter());
        
        // NOTE(Momo): Sleep time
        if (target_secs_per_frame > secs_elapsed) {
            if (is_sleep_granular) {
                DWORD ms_to_sleep = 
                    (DWORD)(1000.f * (target_secs_per_frame - secs_elapsed));
                
                // We cut the sleep some slack, so we sleep 1 sec less.
                if (ms_to_sleep > 1) {
                    Sleep(ms_to_sleep - 1);
                }
            }
            while(target_secs_per_frame > secs_elapsed) {
                secs_elapsed = win32_get_seconds_elapsed(state, last_count, win32_get_performance_counter());
            }
        }
        
        last_count = win32_get_performance_counter();
        
        win32_swap_buffers(window);
    }
    
    return 0;
}



