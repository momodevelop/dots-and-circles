
#include <windows.h>
#include <windowsx.h>
#include <ShellScalingAPI.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <imm.h>

#include "momo.h"

#include "game_config.h"
#include "game_renderer.h"
#include "game_platform.h"

#define STB_SPRINTF_IMPLEMENTATION
#include "stb_sprintf.h"


//~ NOTE(Momo): Consts
#define Win32_RecordStateFile "record_state"
#define Win32_RecordInputFile "record_input"
#define Win32_SaveStateFile "game_state"

//~ NOTE(Momo): Structs
// File handle pool
struct win32_handle_pool {
    HANDLE Slots[8];
    s32 FreeList;
};

struct win32_screen_buffer {
    BITMAPINFO Info;
    void* Memory;
    u32 Width;
    u32 Height;
    static constexpr u32 BytesPerPixel = 4;
};

struct win32_game_memory {
    game_memory Head;
    
    void* Data;
    u32 DataSize;
};

struct win32_state {
    arena Arena;
    b8 IsRunning;
    
    b8 IsRecordingInput;
    HANDLE RecordingInputHandle;
    b8 IsPlaybackInput;
    HANDLE PlaybackInputHandle;
    
    void* PlatformMemoryBlock;
    u32 PlatformMemoryBlockSize;
    
    
    // File paths 
    u32 PerformanceFrequency;
    char ExeFullPath[MAX_PATH];
    char* OnePastExeDirectory;
    
    // Handle pool
    HANDLE Handles[8];
    u32 HandleFreeList[ArrayCount(Handles)];
    u32 HandleFreeCount;
    
#if INTERNAL
    HANDLE StdOut;
#endif

    win32_screen_buffer ScreenBuffer;
    win32_game_memory GameMemory;
    
};


//~ Globals
win32_state* G_State = {};

//~ NOTE(Momo): Helper functions and globals


static inline u32
Win32_DetermineIdealRefreshRate(HWND Window, u32 DefaultRefreshRate) {
    // Do we want to cap this?
    HDC DeviceContext = GetDC(Window);
    Defer { ReleaseDC(Window, DeviceContext); };
    
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
    WIN32_FILE_ATTRIBUTE_DATA Data;
    FILETIME LastWriteTime = {};
    
    if(GetFileAttributesEx(Filename, GetFileExInfoStandard, &Data)) {
        LastWriteTime = Data.ftLastWriteTime;
    }
    return LastWriteTime; 
}

#if INTERNAL
static inline void
Win32_WriteConsole(const char* Message) {
    WriteConsoleA(G_State->StdOut,
                  Message, 
                  SiStrLen(Message), 
                  0, 
                  NULL);
}
#endif

static inline
PlatformLogDecl(Win32_Log) {
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
    return (f32(End.QuadPart - Start.QuadPart)) / State->PerformanceFrequency; 
}

static inline void
Win32_BuildExePathFilename(win32_state* State,
                          char* Dest, 
                          const char* Filename) {
    for(const char *Itr = State->ExeFullPath; 
        Itr != State->OnePastExeDirectory; 
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

static inline void
Win32_DisplayBufferInWindow(HWND Window, win32_screen_buffer* Buffer) {
    HDC DeviceContext = GetDC(Window);
    Defer { ReleaseDC(Window, DeviceContext); };

    // Centering?

}

static inline void
Win32_FreeScreenBuffer(win32_screen_buffer* Buffer) {
    if (Buffer->Memory) {
        Win32_FreeMemory(Buffer->Memory);
    }
}
static inline b8 
Win32_InitScreenBuffer(win32_screen_buffer* Buffer, u32 Width, u32 Height) {
    if (Buffer->Memory) {
        Win32_FreeMemory(Buffer->Memory);
    }
    Buffer->Width = Width;
    Buffer->Height = Height;

    Buffer->Memory = Win32_AllocateMemory(Buffer->Width * Buffer->Height * Buffer->BytesPerPixel);
    if (!Buffer->Memory) {
        return false;
    }

    // NOTE(casey): When the biHeight field is negative, this is the clue to
    // Windows to treat this bitmap as top-down, not bottom-up, meaning that
    // the first three bytes of the image are the color for the top left pixel
    // in the bitmap, not the bottom left!
    Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
    Buffer->Info.bmiHeader.biWidth = Buffer->Width;
    Buffer->Info.bmiHeader.biHeight = Buffer->Height;
    Buffer->Info.bmiHeader.biPlanes = 1;
    Buffer->Info.bmiHeader.biBitCount = 32;
    Buffer->Info.bmiHeader.biCompression = BI_RGB;

    return true;
}


static inline win32_state*
Win32_Init() {
    // This is kinda what we wanna do if we ever want Renderer 
    // to be its own DLL...
    // NOTE(Momo): Arena 
    u32 PlatformMemorySize = Kibibytes(256);
    void* PlatformMemory = Win32_AllocateMemory(PlatformMemorySize);
    if(!PlatformMemory) {
        Win32_Log("[Win32_State] Failed to allocate memory\n"); 
        return 0;
    }
    win32_state* State = Arena_BootupStruct(win32_state,
                                            Arena,
                                            PlatformMemory, 
                                            PlatformMemorySize);
    if (!State) {
        Win32_Log("[Win32_State] Failed to allocate state\n"); 
        return 0;
    }
    
    // NOTE(Momo): Initialize paths
    GetModuleFileNameA(0, State->ExeFullPath, 
                       sizeof(State->ExeFullPath));
    State->OnePastExeDirectory = State->ExeFullPath;
    for( char* Itr = State->ExeFullPath; *Itr; ++Itr) {
        if (*Itr == '\\') {
            State->OnePastExeDirectory = Itr + 1;
        }
    }
    
    // NOTE(Momo): Performance Frequency 
    LARGE_INTEGER PerfCountFreq;
    QueryPerformanceFrequency(&PerfCountFreq);
    State->PerformanceFrequency = S64_ToU32(PerfCountFreq.QuadPart);
    
    // NOTE(Momo): Initialize file handle store
    //GlobalFileHandles = CreatePool<HANDLE>(&G_State->Arena, 8);
    for (u32 I = 0; I < ArrayCount(State->Handles); ++I) {
        State->HandleFreeList[I] = I;
    }
    State->HandleFreeCount = ArrayCount(State->Handles);
    
#if INTERNAL
    // NOTE(Momo): Initialize console
    AllocConsole();    
    State->StdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    
#endif
    
    State->IsRunning = true;
    return State;
}


static inline void
Win32_Free(win32_state* State) {
    FreeConsole();
    Win32_FreeMemory(State); 
}

static inline void
Win32_BeginRecordingInput(win32_state* State, const char* Path) {
    Assert(!State->IsRecordingInput);
    HANDLE RecordFileHandle = CreateFileA(Path,
                                          GENERIC_WRITE,
                                          FILE_SHARE_WRITE,
                                          0,
                                          CREATE_ALWAYS,
                                          0,
                                          0);
    
    if (RecordFileHandle == INVALID_HANDLE_VALUE) {
        Win32_Log("[Win32_BeginRecordingInput] Cannot open file: %s\n", Path);
        return;
    }
    State->RecordingInputHandle = RecordFileHandle;
    State->IsRecordingInput = true;
    Win32_Log("[Win32_BeginRecordingInput] Recording has begun: %s\n", Path);
}

static inline void
Win32_EndRecordingInput(win32_state* State) {
    Assert(State->IsRecordingInput);
    CloseHandle(State->RecordingInputHandle);
    State->IsRecordingInput = false;
    Win32_Log("[Win32_EndRecordingInput] Recording has ended\n");
}

static inline void
Win32_RecordInput(win32_state* State, platform_input* Input) {
    Assert(State->IsRecordingInput);
    DWORD BytesWritten;
    if(!WriteFile(State->RecordingInputHandle,
                  Input,
                  sizeof(platform_input),
                  &BytesWritten, 0)) 
    {
        Win32_Log("[Win32_RecordInput] Cannot write file\n");
        Win32_EndRecordingInput(State);
        return;
    }
    
    if (BytesWritten != sizeof(platform_input)) {
        Win32_Log("[Win32_RecordInput] Did not complete writing\n");
        Win32_EndRecordingInput(State);
        return;
    }
}

static inline void 
Win32_EndPlaybackInput(win32_state* State) {
    Assert(State->IsPlaybackInput);
    CloseHandle(State->PlaybackInputHandle);
    State->IsPlaybackInput = false;
    Win32_Log("[Win32_EndPlaybackInput] Playback has ended\n");
}

static inline void
Win32_BeginPlaybackInput(win32_state* State, const char* Path) {
    Assert(!State->IsPlaybackInput);
    HANDLE RecordFileHandle = CreateFileA(Path,
                                          GENERIC_READ,
                                          FILE_SHARE_READ,
                                          0,
                                          OPEN_EXISTING,
                                          0,
                                          0);
    
    if (RecordFileHandle == INVALID_HANDLE_VALUE) {
        Win32_Log("[Win32_BeginPlaybackInput] Cannot open file: %s\n", Path);
        return;
    }
    State->PlaybackInputHandle = RecordFileHandle;
    State->IsPlaybackInput = true;
    Win32_Log("[Win32_BeginPlaybackInput] Playback has begun: %s\n", Path);
}

// NOTE(Momo): returns true if 'done' reading all input, false otherwise
static inline b8 
Win32_PlaybackInput(win32_state* State, platform_input* Input) {
    DWORD BytesRead;
    BOOL Success = ReadFile(State->PlaybackInputHandle, 
                            Input,
                            sizeof(platform_input),
                            &BytesRead,
                            0);
    if(!Success || BytesRead != sizeof(platform_input)) {
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



static inline void
Win32_GameMemory_Save(win32_game_memory* GameMemory, const char* Path) {
    // We just dump the whole game memory into a file
    HANDLE Win32_Handle = CreateFileA(Path,
                                     GENERIC_WRITE,
                                     FILE_SHARE_WRITE,
                                     0,
                                     CREATE_ALWAYS,
                                     0,
                                     0);
    if (Win32_Handle == INVALID_HANDLE_VALUE) {
        Win32_Log("[Win32_SaveState] Cannot open file: %s\n", Path);
        return;
    }
    Defer { CloseHandle(Win32_Handle); }; 
    
    DWORD BytesWritten;
    if(!WriteFile(Win32_Handle, 
                  GameMemory->Data,
                  (DWORD)GameMemory->DataSize,
                  &BytesWritten,
                  0)) 
    {
        Win32_Log("[Win32_SaveState] Cannot write file: %s\n", Path);
        return;
    }
    
    if (BytesWritten != GameMemory->DataSize) {
        Win32_Log("[Win32_SaveState] Did not complete writing: %s\n", Path);
        return;
    }
    Win32_Log("[Win32_SaveState] State saved: %s\n", Path);
    
}

static inline void
Win32_GameMemory_Load(win32_game_memory* GameMemory, const char* Path) {
    HANDLE Win32_Handle = CreateFileA(Path,
                                     GENERIC_READ,
                                     FILE_SHARE_READ,
                                     0,
                                     OPEN_EXISTING,
                                     0,
                                     0);
    if (Win32_Handle == INVALID_HANDLE_VALUE) {
        Win32_Log("[Win32_LoadState] Cannot open file: %s\n", Path);
        return;
    }
    Defer { CloseHandle(Win32_Handle); }; 
    DWORD BytesRead;
    
    BOOL Success = ReadFile(Win32_Handle, 
                            GameMemory->Data,
                            (DWORD)GameMemory->DataSize,
                            &BytesRead,
                            0);
    
    if (Success && GameMemory->DataSize == BytesRead) {
        Win32_Log("[Win32_LoadState] State loaded from: %s\n", Path);
        return;
    }
    Win32_Log("[Win32_LoadState] Could not read all bytes: %s\n", Path);
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
    u16 Channels;
    
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
               u16 Channels,
               u32 LatencyFrames,
               u32 RefreshRate)
{
    Audio->Channels = Channels;
    Audio->BitsPerSample = BitsPerSample;
    Audio->SamplesPerSecond = SamplesPerSecond;
    Audio->LatencySampleCount = (SamplesPerSecond / RefreshRate) * LatencyFrames;
    
    HRESULT Hr = CoInitializeEx(0, COINIT_SPEED_OVER_MEMORY);
    if (FAILED(Hr)) {
        Win32_Log("[Win32_Audio] Failed CoInitializeEx\n");
        return false;
    }
    
    IMMDeviceEnumerator* DeviceEnumerator;
    Hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), 
                          nullptr,
                          CLSCTX_ALL, 
                          __uuidof(IMMDeviceEnumerator),
                          (LPVOID*)(&DeviceEnumerator));
    
    if (FAILED(Hr)) {
        Win32_Log("[Win32_Audio] Failed to create IMMDeviceEnumerator\n");
        return false;
    }
    Defer { DeviceEnumerator->Release(); };
    
    
    IMMDevice* Device;
    Hr = DeviceEnumerator->GetDefaultAudioEndpoint(eRender, 
                                                   eConsole, 
                                                   &Device);
    if (FAILED(Hr)) {
        Win32_Log("[Win32_Audio] Failed to get audio endpoint\n");
        return false;
    }
    Defer { Device->Release(); };
    
    Hr = Device->Activate(__uuidof(IAudioClient2), 
                          CLSCTX_ALL, 
                          nullptr, 
                          (LPVOID*)&Audio->Client);
    if(FAILED(Hr)) {
        Win32_Log("[Win32_Audio] Failed to create IAudioClient\n");
        return false;
    }
    
    WAVEFORMATEX WaveFormat = {};
    WaveFormat.wFormatTag = WAVE_FORMAT_PCM;
    WaveFormat.wBitsPerSample = BitsPerSample;
    WaveFormat.nChannels = Channels;
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
        Win32_Log("[Win32_Audio] Failed to initialize audio client\n");
        return false;
    }
    
    if (FAILED(Audio->Client->GetService(__uuidof(IAudioRenderClient),
                                         (LPVOID*)(&Audio->RenderClient))))
    {
        Win32_Log("[Win32_Audio] Failed to create IAudioClient\n");
        return false;
    }
    
    UINT32 SoundFrameCount;
    Hr = Audio->Client->GetBufferSize(&SoundFrameCount);
    if (FAILED(Hr))
    {
        Win32_Log("[Win32_Audio] Failed to get buffer size\n");
        return false;
    }
    
    Audio->BufferSize = SoundFrameCount;
    Audio->Buffer = (s16*)Win32_AllocateMemory(Audio->BufferSize);
    if (!Audio->Buffer) {
        Win32_Log("[Win32_Audio] Failed to allocate secondary buffer\n");
        return false;
    }
    
    
    Win32_Log("[Win32_Audio] Loaded!\n");
    
    Audio->Client->Start();
    
    return true;
}

static inline platform_audio
Win32_AudioPrepare(win32_audio* Audio) {
    platform_audio Ret = {};
    
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
    
    Ret.SampleBuffer = Audio->Buffer;
    Ret.SampleCount = SamplesToWrite; 
    Ret.Channels = Audio->Channels;
    
    return Ret;
}

static inline void
Win32_AudioFlush(win32_audio* Audio, 
                platform_audio Output) 
{
    // NOTE(Momo): Kinda assumes 16-bit Sound
    BYTE* SoundBufferData;
    if (SUCCEEDED(Audio->RenderClient->GetBuffer((UINT32)Output.SampleCount, &SoundBufferData))) 
    {
        s16* SrcSample = Output.SampleBuffer;
        s16* DestSample = (s16*)SoundBufferData;
        // Buffer structure for stereo:
        // s16   s16    s16  s16   s16  s16
        // [LEFT RIGHT] LEFT RIGHT LEFT RIGHT....
        for(u32 I = 0; I < Output.SampleCount; ++I ){
            for (u32 J = 0; J < Audio->Channels; ++J) {
                *DestSample++ = *SrcSample++;
            }
            
        }
        
        Audio->RenderClient->ReleaseBuffer((UINT32)Output.SampleCount, 0);
    }
}


static inline v2u
Win32_GetMonitorDimensions() {
    v2u Ret = {};
    Ret.W = S32_ToU32(GetSystemMetrics(SM_CXSCREEN));
    Ret.H = S32_ToU32(GetSystemMetrics(SM_CYSCREEN));
    return Ret;
}

static inline v2u
Win32_GetWindowDimensions(HWND Window) {
    RECT Rect = {};
    GetWindowRect(Window, &Rect);
    return V2u_Create(u16(Rect.right - Rect.left),
                      u16(Rect.bottom - Rect.top));
    
}

static inline v2u
Win32_GetClientDimensions(HWND Window) {
    RECT Rect = {};
    GetClientRect(Window, &Rect);
    return V2u_Create(u32(Rect.right - Rect.left),
                      u32(Rect.bottom - Rect.top));
    
}

static inline void
Win32_ProcessMessages(HWND Window, 
                     win32_state* State,
                     platform_input* Input)
{
    MSG Msg = {};
    while(PeekMessage(&Msg, Window, 0, 0, PM_REMOVE)) {
        switch(Msg.message) {
            case WM_QUIT:
            case WM_CLOSE: {
                State->IsRunning = false;
            } break;
            case WM_CHAR: {
                Input_TryPushCharacterInput(Input, (char)Msg.wParam);
            } break;
            case WM_PAINT: {
                PAINTSTRUCT Paint;
                HDC DeviceContext = BeginPaint(Window, &Paint);
                v2u Dimension = Win32_GetClientDimensions(Window);
                Win32_DisplayBufferInWindow(Window, &State->ScreenBuffer);
                EndPaint(Window, &Paint);
            } break;
            case WM_MOUSEMOVE: {
                // NOTE(Momo): This is the actual conversion from screen space to 
                // design space. I'm not 100% if this should be here but I guess
                // only time will tell.
#if 0
                Input->WindowMousePos.X = (f32)GET_X_LPARAM(Msg.lParam);
                Input->WindowMousePos.Y = (f32)GET_Y_LPARAM(Msg.lParam);
                
                v2u WindowDims = State->Opengl->WindowDimensions;
                aabb2u RenderRegion = State->Opengl->RenderRegion;
                
                Input->RenderMousePos.X = Input->WindowMousePos.X - RenderRegion.Min.X;
                Input->RenderMousePos.Y = Input->WindowMousePos.Y - RenderRegion.Min.Y;
                
                v2f DesignDimsF = V2f_CreateFromV2u(State->Opengl->DesignDimensions);
                v2u RenderDimsU = Aabb2u_Dimensions(RenderRegion);
                v2f RenderDimsF = V2f_CreateFromV2u(RenderDimsU);
                v2f DesignToRenderRatio = V2f_Ratio(DesignDimsF, RenderDimsF);
                
                Input->DesignMousePos.X = Input->RenderMousePos.X * DesignToRenderRatio.W;
                Input->DesignMousePos.Y = Input->RenderMousePos.Y * DesignToRenderRatio.H;
#endif
                
            } break;
            case WM_LBUTTONUP:
            case WM_LBUTTONDOWN: {
                u32 Code = (u32)Msg.wParam;
                b8 IsDown = Msg.message == WM_LBUTTONDOWN;
                Input->ButtonSwitch.Now = IsDown;
            } break;
            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
            case WM_KEYDOWN:
            case WM_KEYUP: {
                u32 KeyCode = (u32)Msg.wParam;
                b8 IsDown = Msg.message == WM_KEYDOWN;
                switch(KeyCode) {
                    case VK_RETURN:{
                        Input->ButtonConfirm.Now = IsDown;
                    } break;
                    case VK_F1:{
                        Input->ButtonConsole.Now = IsDown;
                    } break;
                    case VK_F2:{
                        Input->ButtonInspector.Now = IsDown;
                    } break;
                    case VK_F3:{
                        if (Msg.message == WM_KEYDOWN) {
                            Win32_GameMemory_Save(&State->GameMemory, Win32_SaveStateFile);
                        }
                    } break;
                    case VK_F4:{
                        if (Msg.message == WM_KEYDOWN) {
                            Win32_GameMemory_Load(&State->GameMemory, Win32_SaveStateFile);
                        }
                    } break;
                    case VK_F5:{
                        Input->ButtonPause.Now = IsDown;
                    } break;
                    case VK_F6:{
                        if (Msg.message == WM_KEYDOWN) {
                            if(State->IsRecordingInput) {
                                Win32_EndRecordingInput(State);
                            }
                            else {
                                Win32_GameMemory_Save(&State->GameMemory, Win32_RecordStateFile);
                                Win32_BeginRecordingInput(G_State, Win32_RecordInputFile);
                            }
                        }
                        
                    } break;
                    case VK_F7:{
                        if (Msg.message == WM_KEYDOWN) {
                            if(State->IsPlaybackInput) {
                                Win32_EndPlaybackInput(G_State);
                            }
                            else {
                                Win32_GameMemory_Load(&State->GameMemory, Win32_RecordStateFile);
                                Win32_BeginPlaybackInput(State, 
                                                         Win32_RecordInputFile);
                            }
                        }
                        
                    } break;
                    case VK_F11:{
                        Input->ButtonSpeedDown.Now = IsDown;
                    } break;
                    case VK_F12: {
                        Input->ButtonSpeedUp.Now = IsDown;
                    } break;
                    case VK_BACK:{
                        Input->ButtonBack.Now = IsDown;
                    } break;
                } 
                TranslateMessage(&Msg);
            } break;
            default: 
            {
                //Win32_Log("[Win32_ProcessMessages] %d\n", Msg.message);
                TranslateMessage(&Msg);
                DispatchMessage(&Msg);
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
            G_State->IsRunning = false;
        } break;
        case WM_DESTROY: {
            G_State->IsRunning = false;
        } break;
        case WM_WINDOWPOSCHANGED: {
            v2u WindowWH = Win32_GetWindowDimensions(Window);
            Win32_Log("[Win32_Resize] Window: %d x %d\n", WindowWH.W, WindowWH.H);
                
            v2u ClientWH = Win32_GetClientDimensions(Window);
            Win32_Log("[Win32_Resize] Client: %d x %d\n", ClientWH.W, ClientWH.H);
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
        Win32_Log("[Win32_Window] Failed to create class\n");
        return NULL;
    }
    
    HWND Window = {};
    RECT WindowRect = {};
    v2u MonitorDimensions = Win32_GetMonitorDimensions();
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
                             RECT_Width(WindowRect),
                             RECT_Height(WindowRect),
                             0,
                             0,
                             Instance,
                             0);
    
    if (!Window) {
        Win32_Log("[Win32_Window] Failed to create window\n");
        return NULL;
    }
    Win32_Log("[Win32_Window] Window created successfully\n");
    v2u WindowWH = Win32_GetWindowDimensions(Window);
    v2u ClientWH = Win32_GetClientDimensions(Window);
    Win32_Log("[Win32_Window] Client: %d x %d\n", ClientWH.W, ClientWH.H);
    Win32_Log("[Win32_Window] Window: %d x %d\n", WindowWH.W, WindowWH.H);
    return Window;
    
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
PlatformGetPerformanceCounterDecl(Win32_GetPerformanceCounterU64) {
    return (u64)Win32_GetPerformanceCounter().QuadPart;
}

static inline 
PlatformOpenAssetFileDecl(Win32_OpenAssetFile) {
    platform_file_handle Ret = {}; 
    const char* Path = Game_AssetFileName;
    
    // Check if there are free handlers to go around
    if (G_State->HandleFreeCount == 0) {
        Ret.Error = PlatformFileError_NotEnoughHandlers;
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
        Win32_Log("[Win32_OpenAssetFile] Cannot open file: %s\n", Path);
        Ret.Error = PlatformFileError_CannotOpenFile;
        return Ret;
    } 
    
    u32 FreeSlotIndex = G_State->HandleFreeList[G_State->HandleFreeCount-1];
    G_State->Handles[FreeSlotIndex] = Win32_Handle;
    --G_State->HandleFreeCount;
    Ret.Id = FreeSlotIndex;
    
    return Ret; 
}

static inline 
PlatformLogFileErrorDecl(Win32_LogFileError) {
    switch(Handle->Error) {
        case PlatformFileError_None: {
            Win32_Log("[Win32_File] There is no file error\n");
        } break;
        case PlatformFileError_NotEnoughHandlers: {
            Win32_Log("[Win32_File] There is not enough handlers\n");
        } break;
        case PlatformFileError_CannotOpenFile:{
            Win32_Log("[Win32_File] Cannot open file\n");
        } break;
        case PlatformFileError_Closed:{
            Win32_Log("[Win32_File] File is already closed\n");
        } break;
        case PlatformFileError_ReadFileFailed: {
            Win32_Log("[Win32_File] File read failed\n");
        } break;
        default: {
            Win32_Log("[Win32_File] Undefined error!\n");
        };
    }
}

static inline
PlatformCloseFileDecl(Win32_CloseFile) {
    Assert(Handle->Id < ArrayCount(G_State->Handles));
    HANDLE Win32_Handle = G_State->Handles[Handle->Id];
    if (Win32_Handle != INVALID_HANDLE_VALUE) {
        CloseHandle(Win32_Handle); 
    }
    G_State->HandleFreeList[G_State->HandleFreeCount++] = Handle->Id;
    Assert(G_State->HandleFreeCount <= ArrayCount(G_State->Handles));
}
static inline
PlatformAddTextureDecl(Win32_AddTexture) {
    //TODO(Momo): todo
    return {0};
}

static inline 
PlatformClearTexturesDecl(Win32_ClearTextures) {
    //TODO(Momo): todo
    return;
}

static inline 
PlatformReadFileDecl(Win32_ReadFile) {
    if (Handle->Error) {
        return;
    }
    Assert(Handle->Id < ArrayCount(G_State->Handles));
    
    HANDLE Win32_Handle = G_State->Handles[Handle->Id];
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
        Handle->Error = PlatformFileError_ReadFileFailed; 
    }
}

static inline 
PlatformHideCursorDecl(Win32_HideCursor) {
    ShowCursor(FALSE);
}

static inline
PlatformShowCursorDecl(Win32_ShowCursor) {
    ShowCursor(TRUE);
}

static inline
PlatformGetFileSizeDecl(Win32_GetFileSize) 
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
        Win32_Log("[Win32_GetFileSize] Cannot open file: %s\n", Path);
        return 0;
    } else {
        LARGE_INTEGER FileSize;
        if (!GetFileSizeEx(FileHandle, &FileSize)) {
            Win32_Log("[Win32_GetFileSize] Problems getting file size: %s\n", Path);
            return 0;
        }
        
        return (u32)FileSize.QuadPart;
    }
}

static inline platform_api
Win32_InitPlatformApi() {
    platform_api PlatformApi = {};
    PlatformApi.LogFp = Win32_Log;
    PlatformApi.ReadFileFp = Win32_ReadFile;
    PlatformApi.GetFileSizeFp = Win32_GetFileSize;
    PlatformApi.ClearTexturesFp = Win32_ClearTextures;
    PlatformApi.AddTextureFp = Win32_AddTexture;
    PlatformApi.OpenAssetFileFp = Win32_OpenAssetFile;
    PlatformApi.CloseFileFp = Win32_CloseFile;
    PlatformApi.LogFileErrorFp = Win32_LogFileError;
    PlatformApi.ShowCursorFp = Win32_ShowCursor;
    PlatformApi.HideCursorFp = Win32_HideCursor;
    PlatformApi.GetPerformanceCounterFp = Win32_GetPerformanceCounterU64;
    return PlatformApi;
}

static inline void
Win32_FreeGameMemory(win32_game_memory* GameMemory) {
    Win32_Log("[Win32_GameMemory] Freed\n");
    Win32_FreeMemory(GameMemory->Data);
}

static inline b8
Win32_InitGameMemory(win32_game_memory* GameMemory,
                    u32 PermanentMemorySize,
                    u32 TransientMemorySize,
                    u32 ScratchMemorySize,
                    u32 DebugMemorySize) 
{
    GameMemory->DataSize = PermanentMemorySize + TransientMemorySize + ScratchMemorySize + DebugMemorySize;
    
#if INTERNAL
    SYSTEM_INFO SystemInfo;
    GetSystemInfo(&SystemInfo);
    
    GameMemory->Data = 
        Win32_AllocateMemoryAtAddress(GameMemory->DataSize, 
                                     SystemInfo.lpMinimumApplicationAddress);
#else
    GameMemory->Data = Win32_AllocateMemory(GameMemory->DataSize);
#endif
    if (!GameMemory->Data) {
        Win32_Log("[Win32_GameMemory] Failed to allocate\n");
        return false;
    }
    
    u8* MemoryPtr = (u8*)GameMemory->Data;
    
    GameMemory->Head.PermanentMemorySize = PermanentMemorySize;
    GameMemory->Head.PermanentMemory = GameMemory->Data;
    MemoryPtr += PermanentMemorySize;
    
    GameMemory->Head.TransientMemorySize = TransientMemorySize;
    GameMemory->Head.TransientMemory = MemoryPtr;;
    MemoryPtr += TransientMemorySize;
    
    GameMemory->Head.ScratchMemorySize = ScratchMemorySize;
    GameMemory->Head.ScratchMemory = MemoryPtr;
    MemoryPtr += ScratchMemorySize;
    
    GameMemory->Head.DebugMemorySize = DebugMemorySize;
    GameMemory->Head.DebugMemory = MemoryPtr;
    
    Win32_Log("[Win32_GameMemory] Allocated\n");
    Win32_Log("[Win32_GameMemory] Permanent Memory Size: %d bytes\n", PermanentMemorySize);
    Win32_Log("[Win32_GameMemory] Transient Memory Size: %d bytes\n", TransientMemorySize);
    Win32_Log("[Win32_GameMemory] Scratch Memory Size: %d bytes\n", ScratchMemorySize);
    Win32_Log("[Win32_GameMemory] Debug Memory Size: %d bytes\n", DebugMemorySize);
    
    return true;
}

static inline void
Win32_FreeRenderCommands(mailbox* RenderCommands) {
    Win32_Log("[Win32_RenderCommands] Freed\n"); 
    Win32_FreeMemory(RenderCommands->Memory);
}


static inline b8
Win32_InitRenderCommands(mailbox* RenderCommands,
                        u32 RenderCommandsMemorySize) {
    void* RenderCommandsMemory =
        Win32_AllocateMemory(RenderCommandsMemorySize); 
    if (!RenderCommandsMemory) {
        Win32_Log("[Win32_RenderCommands] Failed to allocate\n"); 
        return false;
    }
    (*RenderCommands) = Mailbox_Create(RenderCommandsMemory,
                                       RenderCommandsMemorySize);
    Win32_Log("[Win32_RenderCommands] Allocated: %d bytes\n", RenderCommandsMemorySize);
    
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
    Defer { Win32_Free(State); };
    G_State = State;
    
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
    platform_input GameInput = {};
    if(!Input_Init(&GameInput, &State->Arena)) {
        Win32_Log("[Win32::Main] Cannot initialize input");
        return 1;
    }
    
    
    // Initialize platform api
    platform_api PlatformApi = Win32_InitPlatformApi();
    
    
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
    Defer { Win32_AudioFree(&Audio); }; 
    
    // Initialize game memory
    if (!Win32_InitGameMemory(&State->GameMemory,
                             Megibytes(1),
                             Megibytes(16),
                             Megibytes(8),
                             Megibytes(1))) 
    {
        Win32_Log("[Win32::Main] Cannot initialize game memory");
        return 1;
    }
    Defer { Win32_FreeGameMemory(&State->GameMemory); };


    // Allocate back buffer
    if (!Win32_InitScreenBuffer(&State->ScreenBuffer, 
                                Game_DesignWidth, 
                                Game_DesignHeight)) 
    {
        Win32_Log("[Win32::Main] Cannot initialize screen buffer");                           
        return 1;
    }
    Defer { Win32_FreeScreenBuffer(&State->ScreenBuffer); };

    // Initialize RenderCommands
    mailbox RenderCommands = {};
    if(!Win32_InitRenderCommands(&RenderCommands, Megibytes(64))) {
        Win32_Log("[Win32::Main] Cannot initialize render commands");
        return 1;
    }
    Defer { Win32_FreeRenderCommands(&RenderCommands); };
    
    
    // Set sleep granularity to 1ms
    b8 SleepIsGranular = timeBeginPeriod(1) == TIMERR_NOERROR;
    
    
    // Game Loop
    LARGE_INTEGER LastCount = Win32_GetPerformanceCounter(); 
    while (State->IsRunning) {
        if (Win32_IsGameCodeOutdated(&GameCode)) {
            Win32_Log("[Win32::Main] Reloading game code!\n");
            Win32_UnloadGameCode(&GameCode);
            Win32_LoadGameCode(&GameCode);
            ZeroBlock(State->GameMemory.Head.TransientMemory, 
                      State->GameMemory.Head.TransientMemorySize);
        }
        
        
        Input_Update(&GameInput);
        Win32_ProcessMessages(Window, 
                              State,
                              &GameInput);
        
        // NOTE(Momo): Recording/Playback input
        if (State->IsRecordingInput) {
            Win32_RecordInput(State, &GameInput);
        }
        if (State->IsPlaybackInput) {
            // NOTE(Momo): This will actually modify GameInput
            if (Win32_PlaybackInput(State, &GameInput)) {
                Win32_EndPlaybackInput(State);
                Win32_GameMemory_Load(&State->GameMemory, Win32_RecordStateFile);
                Win32_BeginPlaybackInput(State, Win32_RecordInputFile);
            }
        }
        
        
        // Compute how much sound to write and where
        // TODO: Functionize this
        platform_audio GameAudioOutput = Win32_AudioPrepare(&Audio);
        
        if (GameCode.GameUpdate) 
        {
            f32 GameDeltaTime = TargetSecsPerFrame;
            
#if 0
            b8 IsGameRunning = GameCode.GameUpdate(&State->GameMemory.Head,
                                                   &PlatformApi,
                                                   &RenderCommands,
                                                   &GameInput,
                                                   &GameAudioOutput,
                                                   GameDeltaTime);
            State->IsRunning = IsGameRunning && State->IsRunning;
#endif

        }
        
        
        Mailbox_Clear(&RenderCommands);
        
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
        
        Win32_DisplayBufferInWindow(Window, &State->ScreenBuffer);

        //Win32_SwapBuffers(Window);
    }
    
    return 0;
}



