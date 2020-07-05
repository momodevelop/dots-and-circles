#include <stdlib.h>

#include "ryoji_maths.h"

#include "thirdparty/sdl2/include/SDL.h"
#include "thirdparty/glad/glad.c"


#include "game_platform.h"
#include "game_asset_loader.h"
#include "game_render_gl.h"

#include "sdl_platform_timer.h"
#include "sdl_platform_gldebug.h"
#include "sdl_platform_utils.h"

// NOTE(Momo): sdl_game_code
struct sdl_game_code {
    game_update* Update;
};

static inline void
Unload(sdl_game_code* GameCode) {
    GameCode->Update = nullptr;
}

static inline bool
Load(sdl_game_code* GameCode)
{
    Unload(GameCode);
    
    void* GameCodeDLL = SDL_LoadObject("game.dll");
    if (!GameCodeDLL) {
        SDL_Log("Failed to open game.dll");
        return false;
    }
    
    GameCode->Update = (game_update*)SDL_LoadFunction(GameCodeDLL, "GameUpdate");
    
    return true;
}

static bool gIsRunning = true;

static inline
PLATFORM_LOG(PlatformLog) {
    va_list va;
    va_start(va, Format);
    SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO, Format, va);
    va_end(va);
}

#if 0
static
PlatformGetFileSizeRes
PlatformGetFileSize(const char* path) {
    SDL_RWops * file = SDL_RWFromFile(path, "r");
    if (file == nullptr) {
        return { false, 0 };
    }
    Defer {
        SDL_RWclose(file);
    };
    
    Sint64 filesize = SDL_RWsize(file);
    if(filesize < 0) {
        return { false, 0 };
    }
    u64 ret = (u64)filesize; 
    
    return { true, ret };
}

static inline
bool ReadFileStr(char* dest, u64 destSize, const char * path) {
    SDL_RWops* file = SDL_RWFromFile(path, "r");
    if (file == nullptr) {
        return false;
    }
    wDefer{
        SDL_RWclose(file);
    };
    
    // Get file size
    u64 filesize = SDL_RWsize(file); // Does not include EOF
    
    if ((filesize + 1) > destSize) {
        return false;
    }
    
    SDL_RWread(file, dest, sizeof(char), filesize);
    
    // Don't forget null terminating value
    dest[filesize] = 0;
    
    return true;
}
#endif

// NOTE(Momo): Threading
struct platform_work_queue_entry {
    void* Data;
    platform_work_queue_entry_callback* Callback;
};

struct thread_work_dequeue_result {
    platform_work_queue_entry Work;
    bool IsValid;
};

struct platform_work_queue {
    // TODO(Momo): Let user define max entries?
    constexpr static int MaxEntries = 2;
    SDL_sem* Semaphore;
    SDL_mutex* Lock;
    
    u32 volatile EntriesCompleted;
    u32 volatile EntriesTotal;
    
    u32 volatile NextEntryToRead;
    u32 volatile NextEntryToWrite;
    
    platform_work_queue_entry Entries[MaxEntries];
};

static inline void
Init(platform_work_queue* Queue) {
    Queue->Semaphore = SDL_CreateSemaphore(0);
    Queue->Lock = SDL_CreateMutex();
    
    Queue->EntriesCompleted = 0;
    Queue->EntriesTotal = 0;
    Queue->NextEntryToRead = 0;
    Queue->NextEntryToWrite = 0;
}

static inline void
Free(platform_work_queue * Queue) {
    SDL_DestroySemaphore(Queue->Semaphore);
    SDL_DestroyMutex(Queue->Lock);
}

struct thread_info {
    u32 Index;
    platform_work_queue* Queue;
};



static inline 
PLATFORM_ADD_WORK(PlatformAddWork) {
    // Start Critical Section 
    for(;;) {
        SDL_LockMutex(Queue->Lock);
        u32 volatile OldNextEntryToWrite = Queue->NextEntryToWrite;
        u32 volatile NewNextEntryToWrite =  (OldNextEntryToWrite + 1) % ArrayCount(Queue->Entries);
        
        
        if (NewNextEntryToWrite == Queue->NextEntryToRead) {
            SDL_UnlockMutex(Queue->Lock);
            continue;
        }
        
        Queue->NextEntryToWrite = NewNextEntryToWrite;
        
        ++Queue->EntriesTotal;
        SDL_UnlockMutex(Queue->Lock);
        // End Critical Section
        
        SDL_Log("Adding: %s", (char*)Data);
        platform_work_queue_entry* Entry = &Queue->Entries[OldNextEntryToWrite];
        Entry->Data = Data;
        Entry->Callback = Callback;
        
        SDL_SemPost(Queue->Semaphore);
        break;
    }
}


// NOTE(Momo): Returns whether we successfully took up the job
static inline bool
DoNextWork(platform_work_queue* Queue, u32 Index) {
    // Start Critical Section
    SDL_LockMutex(Queue->Lock);
    u32 OriginNextEntryToRead = Queue->NextEntryToRead;
    u32 NewNextEntryToRead = (OriginNextEntryToRead + 1) % ArrayCount(Queue->Entries);
    if (OriginNextEntryToRead == Queue->NextEntryToWrite) {
        // Here, we realize someone already took the job, so we are done.
        SDL_UnlockMutex(Queue->Lock);
        return false;
    }
    Queue->NextEntryToRead = NewNextEntryToRead;
    SDL_UnlockMutex(Queue->Lock);
    // End Critical Section
    
    platform_work_queue_entry Entry = Queue->Entries[OriginNextEntryToRead];
    SDL_Log("Thread %d: Working on: %s", Index,(char*)Entry.Data );
    Entry.Callback(Queue, Entry.Data);
    
    // Start Critical Section 
    SDL_LockMutex(Queue->Lock);
    ++Queue->EntriesCompleted;
    SDL_UnlockMutex(Queue->Lock);
    // End Critical Section
    
    return true;
}




static inline 
PLATFORM_WORK_QUEUE_ENTRY_CALLBACK(WorkCallbackBoliao) {
    SDL_Delay(1000); // Simulate difficult task
    SDL_Log((char*)Data);
}

static inline int 
ThreadProc(void *ptr) {
    thread_info* Info = (thread_info*)ptr;
    
    for(;;) {
        if(!DoNextWork(Info->Queue, Info->Index)) {
            SDL_SemWait(Info->Queue->Semaphore);
        }
    }
}



static inline 
PLATFORM_WAIT_FOR_ALL_WORK_TO_COMPLETE(PlatformWaitForAllWorkToComplete) {
    // NOTE(Momo): I don't think I need to mutex this? It shouldn't matter.
    while (Queue->EntriesCompleted != Queue->EntriesTotal) {
        DoNextWork(Queue, 1337);
    }
    
    Queue->EntriesCompleted = 0;
    Queue->EntriesTotal = 0;
}


// NOTE(Momo): entry point
int main(int argc, char* argv[]) {
    platform_work_queue Queue = {};
    Init(&Queue);
    Defer{ Free(&Queue); };
    
    // TODO(Momo): Test Thread Code
    thread_info ThreadContext[3];
    for (int i = 0; i < ArrayCount(ThreadContext); ++i) {
        thread_info * Context = ThreadContext + i;
        Context->Index = i;
        Context->Queue = &Queue;
        
        auto Thread = SDL_CreateThread(ThreadProc, "ThreadProc", Context);
        SDL_Log("Thread %d created", i);
        SDL_DetachThread(Thread);
    }
    
    
    PlatformAddWork(&Queue, WorkCallbackBoliao, "String 01");
    PlatformAddWork(&Queue, WorkCallbackBoliao, "String 02");
    PlatformAddWork(&Queue, WorkCallbackBoliao, "String 03");
    PlatformAddWork(&Queue, WorkCallbackBoliao, "String 04");
    PlatformAddWork(&Queue, WorkCallbackBoliao, "String 05");
    PlatformAddWork(&Queue, WorkCallbackBoliao, "String 06");
    PlatformAddWork(&Queue, WorkCallbackBoliao, "String 07");
    PlatformAddWork(&Queue, WorkCallbackBoliao, "String 08");
    PlatformAddWork(&Queue, WorkCallbackBoliao, "String 09");
    PlatformAddWork(&Queue, WorkCallbackBoliao, "String 10");
    
    PlatformWaitForAllWorkToComplete(&Queue); 
    
    
    SDL_Log("SDL initializing\n");
    if (SDL_Init(SDL_INIT_VIDEO) < 0 ) {
        SDL_Log("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }
    Defer{
        SDL_Log("SDL shutting down\n");
        SDL_Quit();
    };
    
    // NOTE(Momo): Create Window
    SDL_Log("SDL creating Window\n");
    SDL_Window* window = SDL_CreateWindow("Vigil", 
                                          SDL_WINDOWPOS_UNDEFINED, 
                                          SDL_WINDOWPOS_UNDEFINED, 
                                          1600, 
                                          900, 
                                          SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
    if (window == nullptr) {
        SDL_Log("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }
    Defer{
        SDL_Log("SDL destroying window\n");
        SDL_DestroyWindow(window);
    };
    
    // NOTE(Momo): Set OpenGL attributes
    SDL_GL_LoadLibrary(nullptr);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    
#if SLOW_MODE
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif  
    
    
    // NOTE(Momo): Load game code
    sdl_game_code GameCode;
    Load(&GameCode);
    
    // NOTE(Momo) Initialize OpenGL context  core)
    SDL_Log("SDL creating context\n");
    SDL_GLContext context = SDL_GL_CreateContext(window);
    if (context == nullptr) { 
        SDL_Log("Failed to create OpenGL context! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }
    Defer {
        SDL_Log("SDL deleting context\b");
        SDL_GL_DeleteContext(context);
    };
    
    gladLoadGLLoader(SDL_GL_GetProcAddress);
    
    SDL_Log("OpenGL loaded!\n");
    SDL_Log("[OpenGL] Vendor:   %s\n", glGetString(GL_VENDOR));
    SDL_Log("[OpenGL] Renderer: %s\n", glGetString(GL_RENDERER));
    SDL_Log("[OpenGL] Version:  %s\n", glGetString(GL_VERSION));
    
    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //glDisable(GL_DEPTH_TEST);
    
#ifdef SLOW_MODE
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(SdlGlDebugCallback, nullptr);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, true);
#endif
    
    auto [windowWidth, windowHeight] = SDLGetWindowSize(window);
    glViewport(0,0,windowWidth,windowHeight);
    glClearColor(0.0f, 0.3f, 0.3f, 0.0f);
    
    
    
    loaded_bitmap TestBitmap = DebugMakeBitmapFromBmp("assets/ryoji.bmp");
    Defer{ free(TestBitmap.Pixels); };
    
    GlRendererInit((f32)windowWidth, (f32)windowHeight,  1024, &TestBitmap);
    
    // NOTE(Momo): Game Init
    game_memory GameMemory = {};
    GameMemory.IsInitialized = false;
    GameMemory.PermanentStore = malloc(Megabytes(64));
    
    if ( !GameMemory.PermanentStore ) {
        SDL_Log("Cannot allocate for PermanentStore");
        return 1;
    }
    Defer { free(GameMemory.PermanentStore); };
    
    GameMemory.PermanentStoreSize = Megabytes(64);
    
    // NOTE(Momo): PlatformAPI
    GameMemory.PlatformApi.Log = PlatformLog;
    GameMemory.PlatformApi.GlProcessRenderGroup = GlProcessRenderGroup;
    
    sdl_timer timer;
    Start(&timer);
    
    // NOTE(Momo): Game Loop
    while(gIsRunning) {
        SDL_Event e;
        while(SDL_PollEvent(&e)) {
            switch(e.type) {
                case SDL_QUIT: {
                    gIsRunning = false;
                    SDL_Log("Quit triggered\n");
                } break; 
            }
        }
        u64 timeElapsed = TimeElapsed(&timer);
        f32 deltaTime = timeElapsed / 1000.f;
        
        
        GameCode.Update(&GameMemory, deltaTime); 
        
        // NOTE(Momo): Timer update
        Tick(&timer);
        //SDL_Log("%lld  ms\n", timeElapsed);
        SDL_GL_SwapWindow(window);
        
        
    }
    
    
    return 0;
    
}
