
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

#define STB_SPRINTF_IMPLEMENTATION
#include "stb_sprintf.h"

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

struct Win32_Screen_Buffer {
    BITMAPINFO info;
    
    u32 width;
    u32 height;
    u32 bytes_per_pixel;
    void* data;
};

struct Win32_State {
    Arena arena;
    b8 is_running;
    
    // TODO: maybe these can be shifted out?
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
    Win32_Game_Memory game_memory;
    Win32_Screen_Buffer screen_buffer;
};


//~ Globals
Win32_State* g_state = {};


//~ NOTE(Momo): Rendering!!
struct Win32_Pixel {
    u8 r, g, b, a;
};


static inline void
win32_flush_screen_buffer(Win32_Screen_Buffer* screen) {
    u32 buffer_size = screen->width * screen->height * screen->bytes_per_pixel;
    zero_block(screen->data, buffer_size);
}

static inline void
win32_set_pixel_on_screen(Win32_Screen_Buffer* screen, s32 x, s32 y, Win32_Pixel pixel) 
{
    // NOTE(Momo): Reject pixels that are not in screen
    // TODO(Momo): Maybe width and height should be same type as x and y?
    if (x < 0 || y < 0 || x >= (s32)screen->width || y >= (s32)screen->height)
        return;
    
    u32 pitch = screen->width * screen->bytes_per_pixel;
    u8* itr = ((u8*)screen->data) + (y * pitch + x * screen->bytes_per_pixel);
    (*itr++) = pixel.b;
    (*itr++) = pixel.g;
    (*itr++) = pixel.r;
    (*itr++) = pixel.a;
}

static inline void
win32_draw_line_on_screen(Win32_Screen_Buffer* screen, v2s p0, v2s p1, Win32_Pixel pixel = {255, 255, 255, 255}) 
{
    b8 steep = false;
    if (ABS(p0.x - p1.x) < ABS(p0.y - p1.y)) {
        SWAP(p0.x, p0.y);
        SWAP(p1.x, p1.y);
        steep = true;
    }
    
    // NOTE(Momo): Force lines to be left-to-right
    if (p0.x > p1.x)  {
        SWAP(p0.x, p1.x);
        SWAP(p0.y, p1.y);
    }
    
    for (s32 x = p0.x; x <= p1.x; ++x) {
        f32 t = (x - p0.x)/(float)(p1.x - p0.x);
        s32 y = s32((p0.y * (1.f - t)) + (p1.y * t));
        if (steep)
            win32_set_pixel_on_screen(screen, y, x, pixel);
        else
            win32_set_pixel_on_screen(screen, x, y, pixel);
    }
}

static inline void 
win32_draw_triangle_outline_on_screen(Win32_Screen_Buffer* screen, v2s p0, v2s p1, v2s p2, Win32_Pixel pixel) 
{
    win32_draw_line_on_screen(screen, p0, p1, pixel);
    win32_draw_line_on_screen(screen, p1, p2, pixel);
    win32_draw_line_on_screen(screen, p2, p0, pixel);
}

static inline void
win32_draw_filled_triangle_on_screen(Win32_Screen_Buffer* screen, v2s p0, v2s p1, v2s p2, Win32_Pixel c0,
                                     Win32_Pixel c1,
                                     Win32_Pixel c2) 

{
    v2s pts[3] = { p0, p1, p2 };
    
    // NOTE(Momo): Figure out the bounding box of the triangle
    // Ie. find the smallest xy and the largest xy.
    static constexpr u32 point_count = 3;
    aabb2i box = { pts[0], pts[0] };
    {
        for (u32 i = 1; i < point_count; ++i) {
            for (u32 j = 0; j < 2; ++j) {
                if(box.min[j] > pts[i][j]) {
                    box.min[j] = pts[i][j];
                }
                if (box.max[j] < pts[i][j]) {
                    box.max[j] = pts[i][j];
                }
            }
        }
    }
    
    v2s ab = pts[1] - pts[0];
    v2s ac = pts[2] - pts[0];
    
    for (s32 y = box.min.y; y <= box.max.y; ++y) {
        for (s32 x = box.min.x; x <= box.max.x; ++x) {
            // NOTE(Momo): Check barycenter
            v2s pa = pts[0] - v2s{ x, y } ;
            v3s x_vector = { pa.x, ab.x, ac.x };
            v3s y_vector = { pa.y, ab.y, ac.y };
            
            v3s orthorgonal = cross(y_vector, x_vector);
            
            // NOTE(Momo): Check if triangle is degenerate
            if (orthorgonal.x == 0) {
                // NOTE(Momo): we are degenerate...then we give up
                return;
            }
            v3f barycentric = {
                1.f - (f32)(orthorgonal.y + orthorgonal.z)/(f32)orthorgonal.x,
                (f32)orthorgonal.y/(f32)orthorgonal.x,
                (f32)orthorgonal.z/(f32)orthorgonal.x
            };
            
            // NOTE(Momo): Check if within bounds
            if (barycentric.x < 0.f || barycentric.y < 0.f || barycentric.z < 0.f)
                continue;
            
            // NOTE(Momo): Draw pixel
            Win32_Pixel color = {};
            
            color.r = (u8)(barycentric.x * c0.r + 
                           barycentric.y * c1.r + 
                           barycentric.z * c2.r);
            color.g = (u8)(barycentric.x * c0.g + 
                           barycentric.y * c1.g + 
                           barycentric.z * c2.g);
            color.b = (u8)(barycentric.x * c0.b + 
                           barycentric.y * c1.b + 
                           barycentric.z * c2.b);
            color.a = (u8)(barycentric.x * c0.a + 
                           barycentric.y * c1.a + 
                           barycentric.z * c2.a);
            
            
            win32_set_pixel_on_screen(screen, x, y, color);
            
        };
        
    }
    
    
}


// NOTE(Momo): Upper/Lower triangle algorithm
//
// Every triangle can segment itself into up to two triangles;
// upper and lower. The main idea is that we start from the lowest vertex and
// and work our way upwards, drawing horizontal lines
// 
// Note that it is entirely possible for a vertical version; where we 
// we sort the vertices from left to right, and divide between left and right triangles
//
static inline void 
win32_draw_filled_triangle_on_screen_upper_lower_algo(Win32_Screen_Buffer* screen, v2s p0, v2s p1, v2s p2, Win32_Pixel pixel = { 255, 255, 255, 255}) 
{
    // NOTE(Momo): Sort verices by y-axis, from smallest to largest
    // such that p0.y < p1.y < p2.y 
    if (p0.y > p2.y) SWAP(p0, p2);
    if (p0.y > p1.y) SWAP(p0, p1);
    if (p1.y > p2.y) SWAP(p1, p2);
    
    // NOTE(Momo): Draw bottom triangle segment 
    // We start from p0 and go up towards p1.
    // As we go up the y, we find the corresponding x on the lines
    // formed by (p1 - p0) and (p2 - p0). Having these two x's
    // will form a line that we will draw on that y-axis.
    // We then move upwards until the segment is completed
    // 
    s32 total_height = p2.y - p0.y;
    for (s32 y = p0.y; y < p1.y; ++y) {
        s32 segment_height = p1.y - p0.y;
        
        // NOTE(Momo): long side is formed by (p2 - p0).
        // NOTE(Momo): short side is formed by (p1 - p0).
        f32 long_side_alpha = (f32)(y - p0.y)/total_height;
        f32 short_side_alpha = (f32)(y - p0.y)/segment_height;
        
        v2s long_side_pt = LERP(p0, p2, long_side_alpha);
        v2s short_side_pt = LERP(p0, p1, short_side_alpha);
        
        // NOTE(Momo): This is to make sure that we go from left to right
        // We will make sure that long_side_pt is the left side
        if (long_side_pt.x > short_side_pt.x) {
            SWAP(long_side_pt, short_side_pt);
        }
        
        for(int x = long_side_pt.x; x <= short_side_pt.x; ++x) {
            win32_set_pixel_on_screen(screen, x, y, pixel);
        }
    }
    
    // NOTE(Momo): Draw top triangle
    // Same as bottom segment, just that the lines involved are:
    // (p2 - p1) and (p2 - p0)
    for (s32 y = p1.y; y <= p2.y; ++y) {
        s32 segment_height = p2.y - p1.y;
        
        // NOTE(Momo): long side is formed by (p2 - p0).
        // NOTE(Momo): short side is formed by (p2 - p1).
        f32 long_side_alpha = (f32)(y - p0.y)/total_height;
        f32 short_side_alpha = (f32)(y - p1.y)/segment_height;
        
        v2s long_side_pt = LERP(p0, p2, long_side_alpha);
        v2s short_side_pt = LERP(p1, p2, short_side_alpha);
        
        // NOTE(Momo): This is to make sure that we go from left to right
        // We will make sure that long_side_pt is the left side
        if (long_side_pt.x > short_side_pt.x) {
            SWAP(long_side_pt, short_side_pt);
        }
        
        for(int x = long_side_pt.x; x <= short_side_pt.x; ++x) {
            win32_set_pixel_on_screen(screen, x, y, pixel);
        }
    }
}

//~ NOTE(Momo): Helper functions and globals
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
static inline void
win32_write_console(const char* Message) {
    WriteConsoleA(g_state->std_out,
                  Message, 
                  cstr_length(Message), 
                  0, 
                  NULL);
}
#endif

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

#if INTERNAL
static inline void*
win32_allocate_memory_at_address(usize memory_size, LPVOID address ) {
    return VirtualAllocEx(GetCurrentProcess(),
                          address, 
                          memory_size,
                          MEM_RESERVE | MEM_COMMIT, 
                          PAGE_READWRITE);
    
}
#endif

static inline void*
win32_allocate_memory(usize memory_size) {
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

//~ NOTE(Momo): Screen buffer related
static inline void
win32_free_screen_buffer(Win32_Screen_Buffer* screen) {
    win32_free_memory(screen->data);
}

static inline b8 
win32_init_screen_buffer(Win32_Screen_Buffer* screen, u32 width, u32 height) {
    
    // TODO(Momo): Should follow window width/height?
    
    screen->width = width;
    screen->height = height;
    screen->bytes_per_pixel = 4;
    
    screen->info.bmiHeader.biSize = sizeof(screen->info.bmiHeader);
    screen->info.bmiHeader.biWidth = width;
    screen->info.bmiHeader.biHeight = height;
    screen->info.bmiHeader.biPlanes = 1;
    screen->info.bmiHeader.biBitCount = 32;
    screen->info.bmiHeader.biCompression = BI_RGB;
    
    screen->data = win32_allocate_memory(screen->width * screen->height * screen->bytes_per_pixel);
    
    if (!screen->data) {
        return false;
    }
    return true;
}

static inline void
win32_push_screen_buffer_to_window(Win32_Screen_Buffer* screen, HDC dc, u32 window_w, u32 window_h) 
{
    // TODO: Aspect ratio correction
    // TODO: use correct values
    StretchDIBits(dc,
                  /*
x, y, window width, window height,
x, y, buffer width, buffer height
*/
                  0, 0, window_w, window_h,
                  0, 0, screen->width, screen->height, 
                  screen->data,
                  &screen->info,
                  DIB_RGB_COLORS, SRCCOPY);
}



//~ NOTE(Momo): state related
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
    state->std_out = GetStdHandle(STD_OUTPUT_HANDLE);
    
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
win32_begin_recording_input(Win32_State* state, const char* path) {
    ASSERT(!state->is_recording_input);
    HANDLE record_file_handle = CreateFileA(path,
                                            GENERIC_WRITE,
                                            FILE_SHARE_WRITE,
                                            0,
                                            CREATE_ALWAYS,
                                            0,
                                            0);
    
    if (record_file_handle == INVALID_HANDLE_VALUE) {
        win32_log("[Win32::BeginRecordingInput] Cannot open file: %s\n", path);
        return;
    }
    state->recording_input_handle = record_file_handle;
    state->is_recording_input = true;
    win32_log("[Win32::BeginRecordingInput] Recording has begun: %s\n", path);
}

static inline void
win32_end_recording_input(Win32_State* state) {
    ASSERT(state->is_recording_input);
    CloseHandle(state->recording_input_handle);
    state->is_recording_input = false;
    win32_log("[Win32::EndRecordingInput] Recording has ended\n");
}

static inline void
win32_record_input(Win32_State* state, Platform_Input* input) {
    ASSERT(state->is_recording_input);
    DWORD BytesWritten;
    if(!WriteFile(state->recording_input_handle,
                  input,
                  sizeof(Platform_Input),
                  &BytesWritten, 0)) 
    {
        win32_log("[Win32::Recordinput] Cannot write file\n");
        win32_end_recording_input(state);
        return;
    }
    
    if (BytesWritten != sizeof(Platform_Input)) {
        win32_log("[Win32::Recordinput] Did not complete writing\n");
        win32_end_recording_input(state);
        return;
    }
}

static inline void 
win32_end_playback_input(Win32_State* state) {
    ASSERT(state->is_playback_input);
    CloseHandle(state->playback_input_handle);
    state->is_playback_input = false;
    win32_log("[Win32::EndPlaybackInput] Playback has ended\n");
}

static inline void
win32_begin_playback_input(Win32_State* state, const char* path) {
    ASSERT(!state->is_playback_input);
    HANDLE record_file_handle = CreateFileA(path,
                                            GENERIC_READ,
                                            FILE_SHARE_READ,
                                            0,
                                            OPEN_EXISTING,
                                            0,
                                            0);
    
    if (record_file_handle == INVALID_HANDLE_VALUE) {
        win32_log("[Win32::BeginPlaybackInput] Cannot open file: %s\n", path);
        return;
    }
    state->playback_input_handle = record_file_handle;
    state->is_playback_input = true;
    win32_log("[Win32::BeginPlaybackInput] Playback has begun: %s\n", path);
}

// NOTE(Momo): returns true if 'done' reading all input, false otherwise
static inline b8 
win32_playback_input(Win32_State* state, Platform_Input* input) {
    DWORD bytes_read;
    BOOL success = ReadFile(state->playback_input_handle, 
                            input,
                            sizeof(Platform_Input),
                            &bytes_read,
                            0);
    if(!success || bytes_read != sizeof(Platform_Input)) {
        return true;
    }
    return false;
}

//~ NOTE(Momo): game code related
struct Win32_Game_Code {
    HMODULE dll;
    Game_Update* game_update;
    FILETIME last_write_time;
    b8 is_valid;
    
    char src_filename[MAX_PATH];
    char temp_filename[MAX_PATH];
    char lock_filename[MAX_PATH];        
};


static inline Win32_Game_Code 
win32_init_game_code(Win32_State* state,
                     const char* src_filename,
                     const char* temp_filename,
                     const char* lock_filename) 
{
    Win32_Game_Code ret = {};
    win32_build_exe_path_filename(state, ret.src_filename, src_filename);
    win32_build_exe_path_filename(state, ret.temp_filename, temp_filename);
    win32_build_exe_path_filename(state, ret.lock_filename, lock_filename);
    
    return ret;
}

static inline void
win32_load_game_code(Win32_Game_Code* code) 
{
    WIN32_FILE_ATTRIBUTE_DATA ignored; 
    if(!GetFileAttributesEx(code->lock_filename, 
                            GetFileExInfoStandard, 
                            &ignored)) 
    {
        code->last_write_time = win32_get_file_last_write_time(code->src_filename);
        BOOL success = FALSE;
        do {
            success = CopyFile(code->src_filename, code->temp_filename, FALSE); 
        } while (!success); 
        code->dll = LoadLibraryA(code->temp_filename);
        if(code->dll) {
            code->game_update = 
                (Game_Update*)GetProcAddress(code->dll, "game_update");
            code->is_valid = (code->game_update != 0);
        }
    }
}

static inline void 
win32_unload_game_code(Win32_Game_Code* code) {
    if (code->dll) {
        FreeLibrary(code->dll);
        code->dll = 0;
    }
    code->is_valid = false;
    code->game_update = 0;
}

static inline b8
win32_is_game_code_outdated(Win32_Game_Code* code) {    
    // Check last modified date
    FILETIME last_write_time = win32_get_file_last_write_time(code->src_filename);
    LARGE_INTEGER current_last_write_time = win32_file_time_to_large_int(last_write_time); 
    LARGE_INTEGER game_code_last_write_time = win32_file_time_to_large_int(code->last_write_time);
    
    return (current_last_write_time.QuadPart > game_code_last_write_time.QuadPart); 
}



static inline void
win32_save_game_memory(Win32_Game_Memory* game_memory, const char* path) {
    // We just dump the whole game memory into a file
    HANDLE win32_handle = CreateFileA(path,
                                      GENERIC_WRITE,
                                      FILE_SHARE_WRITE,
                                      0,
                                      CREATE_ALWAYS,
                                      0,
                                      0);
    if (win32_handle == INVALID_HANDLE_VALUE) {
        win32_log("[Win32::SaveState] Cannot open file: %s\n", path);
        return;
    }
    defer { CloseHandle(win32_handle); }; 
    
    DWORD bytes_written;
    if(!WriteFile(win32_handle, 
                  game_memory->data,
                  (DWORD)game_memory->data_size,
                  &bytes_written,
                  0)) 
    {
        win32_log("[Win32::SaveState] Cannot write file: %s\n", path);
        return;
    }
    
    if (bytes_written != game_memory->data_size) {
        win32_log("[Win32::SaveState] Did not complete writing: %s\n", path);
        return;
    }
    win32_log("[Win32::SaveState] state saved: %s\n", path);
    
}

static inline void
win32_load_game_memory(Win32_Game_Memory* game_memory, const char* path) {
    HANDLE win32_handle = CreateFileA(path,
                                      GENERIC_READ,
                                      FILE_SHARE_READ,
                                      0,
                                      OPEN_EXISTING,
                                      0,
                                      0);
    if (win32_handle == INVALID_HANDLE_VALUE) {
        win32_log("[Win32::LoadState] Cannot open file: %s\n", path);
        return;
    }
    defer { CloseHandle(win32_handle); }; 
    DWORD bytes_read;
    
    BOOL success = ReadFile(win32_handle, 
                            game_memory->data,
                            (DWORD)game_memory->data_size,
                            &bytes_read,
                            0);
    
    if (success && game_memory->data_size == bytes_read) {
        win32_log("[Win32::LoadState] state loaded from: %s\n", path);
        return;
    }
    win32_log("[Win32::LoadState] Could not read all bytes: %s\n", path);
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
    return { u32(rect.right - rect.left), u32(rect.bottom - rect.top) };
    
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
                       Platform_Input* input)
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
                // TODO
            } break;
            case WM_LBUTTONUP:
            case WM_LBUTTONDOWN: {
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
                            win32_save_game_memory(&state->game_memory, WIN32_SAVE_STATE_FILE);
                        }
                    } break;
                    case VK_F4:{
                        if (msg.message == WM_KEYDOWN) {
                            win32_load_game_memory(&state->game_memory, WIN32_SAVE_STATE_FILE);
                        }
                    } break;
                    case VK_F5:{
                        input->button_pause.now = is_down;
                    } break;
                    case VK_F6:{
                        if (msg.message == WM_KEYDOWN) {
                            if(state->is_recording_input) {
                                win32_end_recording_input(state);
                            }
                            else {
                                win32_save_game_memory(&state->game_memory, WIN32_RECORD_STATE_FILE);
                                win32_begin_recording_input(state, WIN32_RECORD_INPUT_FILE);
                            }
                        }
                        
                    } break;
                    case VK_F7:{
                        if (msg.message == WM_KEYDOWN) {
                            if(state->is_playback_input) {
                                win32_end_playback_input(state);
                            }
                            else {
                                win32_load_game_memory(&state->game_memory, WIN32_RECORD_STATE_FILE);
                                win32_begin_playback_input(state, 
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
        case WM_PAINT: {
            PAINTSTRUCT paint;
            HDC dc = BeginPaint(window, &paint);
            v2u dimensions = win32_get_client_dimensions(window);
            win32_push_screen_buffer_to_window(&g_state->screen_buffer, dc, dimensions.w, dimensions.h);
            EndPaint(window, &paint);
        } break;
        case WM_WINDOWPOSCHANGED: {
            // TODO
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
    const char* path = GAME_ASSET_FILENAME;
    
    // Check if there are free handlers to go around
    if (g_state->handle_free_count == 0) {
        ret.error = WIN32_FILE_ERROR_NO_HANDLERS;
        return ret;
    }    
    
    HANDLE win32_handle = CreateFileA(path, 
                                      GENERIC_READ, 
                                      FILE_SHARE_READ,
                                      0,
                                      OPEN_EXISTING,
                                      0,
                                      0);
    
    
    if(win32_handle == INVALID_HANDLE_VALUE) {
        win32_log("[Win32::OpenAssetFile] Cannot open file: %s\n", path);
        ret.error = WIN32_FILE_ERROR_CANNOT_OPEN;
        return ret;
    } 
    
    u32 free_slot_index = g_state->handle_free_list[g_state->handle_free_count-1];
    g_state->handles[free_slot_index] = win32_handle;
    --g_state->handle_free_count;
    ret.id = free_slot_index;
    
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


#if 0
static inline
PLATFORM_ADD_TEXTURE_DECL(win32_add_texture) {
    return Opengl_AddTexture(g_state->opengl, width, height, pixels);
}

static inline 
PLATFORM_CLEAR_TEXTURES_DECL(win32_clear_textures) {
    return Opengl_ClearTextures(g_state->opengl);
}
#endif

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
    //platform_api.clear_textures = win32_clear_textures;
    //platform_api.add_texture = win32_add_texture;
    platform_api.open_asset_file = win32_open_asset_file;
    platform_api.close_file = win32_close_file;
    platform_api.log_file_error = win32_log_file_error;
    platform_api.show_cursor = win32_show_cursor;
    platform_api.hide_cursor = win32_hide_cursor;
    platform_api.get_performance_counter = win32_get_performance_counter_u64;
    
    return platform_api;
}

static inline void
win32_free_game_memory(Win32_Game_Memory* game_memory) {
    win32_log("[Win32::game_memory] Freed\n");
    win32_free_memory(game_memory->data);
}

static inline b8
win32_init_game_memory(Win32_Game_Memory* game_memory,
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
        win32_allocate_memory_at_address(game_memory->data_size, 
                                         SystemInfo.lpMinimumApplicationAddress);
#else
    game_memory->data = win32_allocate_memory(game_memory->data_size);
#endif
    if (!game_memory->data) {
        win32_log("[Win32::game_memory] Failed to allocate\n");
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
    
    win32_log("[Win32::game_memory] Allocated\n");
    win32_log("[Win32::game_memory] Permanent Memory Size: %d bytes\n", permanent_memory_size);
    win32_log("[Win32::game_memory] Transient Memory Size: %d bytes\n", transient_memory_size);
    win32_log("[Win32::game_memory] Scratch Memory Size: %d bytes\n", scratch_memory_size);
    win32_log("[Win32::game_memory] Debug Memory Size: %d bytes\n", debug_memory_size);
    
    return true;
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
    
    
    // TODO: should determine the 'best size' based on design width and height
    HWND window = win32_create_window(instance, 
                                      GAME_DESIGN_WIDTH,
                                      GAME_DESIGN_HEIGHT,
                                      "Dots and Circles");
    if (!window) { 
        win32_log("[Win32::Main] Cannot create window\n");
        return 1; 
    }
    
    u32 refresh_rate = win32_determine_ideal_refresh_rate(window, 60); 
    f32 target_secs_per_frame = 1.f / refresh_rate; 
    win32_log("[Win32::Main] Target Secs Per Frame: %.2f\n", target_secs_per_frame);
    win32_log("[Win32::Main] Monitor Refresh Rate: %d Hz\n", refresh_rate);
    
    // Load the game code DLL
    Win32_Game_Code GameCode = win32_init_game_code(state,
                                                    "compiled_game.dll", 
                                                    "active_game.dll", 
                                                    "lock");
    
    // Initialize game input
    Platform_Input game_input = {};
    if(!game_input.alloc(&state->arena)) {
        win32_log("[Win32::Main] Cannot initialize input\n");
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
        win32_log("[Win32::Main] Cannot initialize audio\n");
        return 1;
    }
    defer { win32_audio_free(&audio); }; 
    
    // Initialize game memory
    if (!win32_init_game_memory(&state->game_memory,
                                MEBIBYTES(1),
                                MEBIBYTES(16),
                                MEBIBYTES(8),
                                MEBIBYTES(1))) 
    {
        win32_log("[Win32::Main] Cannot initialize game memory\n");
        return 1;
    }
    defer { win32_free_game_memory(&state->game_memory); };
    
    // Initialize render_commands
    Mailbox render_commands = {};
    if(!win32_init_render_commands(&render_commands, MEBIBYTES(64))) {
        win32_log("[Win32::Main] Cannot initialize render commands\n");
        return 1;
    }
    defer { win32_free_render_commands(&render_commands); };
    
    // Initialize screen buffer
    if (!win32_init_screen_buffer(&state->screen_buffer, GAME_DESIGN_WIDTH, GAME_DESIGN_HEIGHT)) {
        win32_log("[Win32::Main] Cannot initialize screen buffer\n");
    }
    defer { win32_free_screen_buffer(&state->screen_buffer); };
    
    // Set sleep granularity to 1ms
    b8 is_sleep_granular = timeBeginPeriod(1) == TIMERR_NOERROR;
    
    
    // Game Loop
    LARGE_INTEGER last_count = win32_get_performance_counter(); 
    while (state->is_running) {
        if (win32_is_game_code_outdated(&GameCode)) {
            win32_log("[Win32::Main] Reloading game code!\n");
            win32_unload_game_code(&GameCode);
            win32_load_game_code(&GameCode);
            zero_block(state->game_memory.head.transient_memory, 
                       state->game_memory.head.transient_memory_size);
        }
        
        
        game_input.update();
        win32_process_messages(window, state, &game_input);
        
        // NOTE(Momo): Recording/Playback input
        if (state->is_recording_input) {
            (state, &game_input);
        }
        if (state->is_playback_input) {
            // NOTE(Momo): This will actually modify game_input
            if (win32_playback_input(state, &game_input)) {
                win32_end_playback_input(state);
                win32_load_game_memory(&state->game_memory, WIN32_RECORD_STATE_FILE);
                win32_begin_playback_input(state, WIN32_RECORD_INPUT_FILE);
            }
        }
        
        
        // Compute how much sound to write and where
        Platform_Audio platform_audio_output = win32_audio_prepare(&audio);
        
#if 1
        // NOTE(Momo) Test our software renderer here!
        {
            Win32_Screen_Buffer* screen = &state->screen_buffer;
            win32_flush_screen_buffer(screen);
            
            static f32 rotation = 0.f;
            
            f32 game_dt = target_secs_per_frame;
            
#if 1
            // Basic Triangle
            {
                v4f p0 = v4f::create(0.f, 0.f, 0.f, 1.f); 
                v4f p1 = v4f::create(0.5f, 0.f, 0.f, 1.f);
                v4f p2 = v4f::create(0.f, 0.5f, 0.f, 1.f);
                
                m44f s = m44f::create_scale(400.f, 400.f, 0.f);
                m44f r = m44f::create_rotation_z(rotation);
                m44f t = m44f::create_translation(0.f, 0.f, 0.f);
                m44f transform = t*r*s;
                p0 = transform * p0;
                p1 = transform * p1;
                p2 = transform * p2;
                
                //win32_log("(%f, %f) and (%f, %f)\n", pt1.x, pt1.y, pt2.x, pt2.y);
                
                win32_draw_filled_triangle_on_screen(screen, 
                                                     to_v2s(p0.xy), 
                                                     to_v2s(p1.xy), 
                                                     to_v2s(p2.xy), 
                                                     {255, 0, 0, 0},
                                                     {0, 255, 0, 0},
                                                     {0, 0, 255, 0});
                
                
            }
#else
            // Triangle 1
            {
                v4f p0 = v4f::create(-0.5f, -0.5f, 0.f, 1.f); 
                v4f p1 = v4f::create(0.5f, -0.5f, 0.f, 1.f);
                v4f p2 = v4f::create(0.f, 0.5f, 0.f, 1.f);
                
                m44f s = m44f::create_scale(100.f, 100.f, 0.f);
                m44f r = m44f::create_rotation_z(rotation);
                m44f t = m44f::create_translation(400.f, 400.f, 0.f);
                m44f transform = t*r*s;
                p0 = transform * p0;
                p1 = transform * p1;
                p2 = transform * p2;
                
                
                
                //win32_log("(%f, %f) and (%f, %f)\n", pt1.x, pt1.y, pt2.x, pt2.y);
                win32_draw_filled_triangle_on_screen(screen, 
                                                     to_v2s(p0.xy), 
                                                     to_v2s(p1.xy), 
                                                     to_v2s(p2.xy), 
                                                     {255, 0, 0, 0});
                
                
            }
            
            // Triangle 2
            {
                v4f p0 = v4f::create(-1.5f, -0.5f, 0.f, 1.f); 
                v4f p1 = v4f::create(3.f, -0.25f, 0.f, 1.f);
                v4f p2 = v4f::create(0.f, 1.5f, 0.f, 1.f);
                
                m44f s = m44f::create_scale(100.f, 100.f, 0.f);
                m44f r = m44f::create_rotation_z(rotation);
                m44f t = m44f::create_translation(300.f, 500.f, 0.f);
                m44f transform = t*r*s;
                p0 = transform * p0;
                p1 = transform * p1;
                p2 = transform * p2;
                
                
                
                //win32_log("(%f, %f) and (%f, %f)\n", pt1.x, pt1.y, pt2.x, pt2.y);
                win32_draw_filled_triangle_on_screen(screen, 
                                                     to_v2s(p0.xy), 
                                                     to_v2s(p1.xy), 
                                                     to_v2s(p2.xy), 
                                                     {0, 255, 0, 0});
                
                
            }
            
            // Triangle 3
            {
                v4f p0 = v4f::create(-0.5f, -3.f, 0.f, 1.f); 
                v4f p1 = v4f::create(1.5f, -1.5f, 0.f, 1.f);
                v4f p2 = v4f::create(1.5f, 1.5f, 0.f, 1.f);
                
                m44f s = m44f::create_scale(100.f, 100.f, 0.f);
                m44f r = m44f::create_rotation_z(rotation);
                m44f t = m44f::create_translation(400.f, 300.f, 0.f);
                m44f transform = t*r*s;
                p0 = transform * p0;
                p1 = transform * p1;
                p2 = transform * p2;
                
                
                
                //win32_log("(%f, %f) and (%f, %f)\n", pt1.x, pt1.y, pt2.x, pt2.y);
                win32_draw_filled_triangle_on_screen(screen, 
                                                     to_v2s(p0.xy), 
                                                     to_v2s(p1.xy), 
                                                     to_v2s(p2.xy), 
                                                     {0, 0, 255, 0});
                win32_draw_triangle_outline_on_screen(screen, 
                                                      to_v2s(p0.xy), 
                                                      to_v2s(p1.xy), 
                                                      to_v2s(p2.xy), 
                                                      {0, 0, 255, 0});
                
            }
            rotation +=  game_dt;
#endif
            
            
            
            
            
            
        }
        
#else
        if (GameCode.game_update) 
        {
            f32 game_dt = target_secs_per_frame;
            b8 is_game_running = GameCode.game_update(&state->game_memory.head,
                                                      &platform_api,
                                                      &render_commands,
                                                      &game_input,
                                                      &platform_audio_output,
                                                      game_dt);
            state->is_running = is_game_running && state->is_running;
        }
#endif
        
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
        
#if 1
        win32_log("ms: %f\n", secs_elapsed);
#endif
        last_count = win32_get_performance_counter();
        
        v2u dimensions = win32_get_client_dimensions(window);
        win32_push_screen_buffer_to_window(&state->screen_buffer, GetDC(window), dimensions.w, dimensions.h);
        
        
    }
    
    return 0;
}



