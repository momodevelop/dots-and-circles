
#include "game.h"

// cmd: jump main/menu/atlas_test/etc...
static inline void 
CmdJump(debug_console* Console, void* Context, String Arguments) {
    auto* DebugState = (debug_state*)Context;
    permanent_state* PermState = DebugState->PermanentState;
    
    Defer{ G_Scratch->clear(); };
    
    String Buffer = {};
    String_Split_Result ArgList = split(Arguments, G_Scratch, ' ');
    if ( ArgList.item_count != 2 ) {
        // Expect two arguments
        init(&Buffer, "Expected only 2 arguments");
        DebugConsole_PushInfo(Console, 
                              Buffer, 
                              C4F_RED);
        return;
    }
    
    String StateToChangeTo = ArgList.items[1];
    if (is_equal(StateToChangeTo, "main")) {
        init(&Buffer, "Jumping to Main");
        DebugConsole_PushInfo(Console, 
                              Buffer, 
                              C4F_YELLOW);
        PermState->NextGameMode = GameModeType_Main;
    }
    else if (is_equal(StateToChangeTo, "splash")) {
        init(&Buffer, "Jumping to Splash");
        DebugConsole_PushInfo(Console, 
                              Buffer,  
                              C4F_YELLOW);
        PermState->NextGameMode = GameModeType_Splash;
    }
    else if (is_equal(StateToChangeTo, "sandbox")) {
        init(&Buffer, "Jumping to Sandbox");
        DebugConsole_PushInfo(Console, 
                              Buffer, 
                              C4F_YELLOW);
        PermState->NextGameMode = GameModeType_Sandbox;
    }
    else if (is_equal(StateToChangeTo, "anime")) {
        init(&Buffer, "Jumping to Anime");
        DebugConsole_PushInfo(Console, 
                              Buffer, 
                              C4F_YELLOW);
        PermState->NextGameMode = GameModeType_AnimeTest;
    }
    else {
        init(&Buffer, "Invalid state to jump to");
        DebugConsole_PushInfo(Console, 
                              Buffer, 
                              C4F_RED);
    }
}

extern "C" 
GameUpdateFunc(GameUpdate) 
{
    // NOTE(Momo): Initialize globals
    Arena Scratch = {};
    {
        // Let's say we want to time this block
        G_Platform = PlatformApi;
        G_Log = PlatformApi->LogFp;
        G_Renderer = RenderCommands;
        G_Input = PlatformInput;
        if (!Scratch.init(GameMemory->ScratchMemory, GameMemory->ScratchMemorySize)) {
            G_Log("Cannot initialize Scratch Memory");
            return false;
        }
        G_Scratch = &Scratch;
    }
    
    
    
    auto* PermState = (permanent_state*)GameMemory->PermanentMemory;
    auto* TranState = (transient_state*)GameMemory->TransientMemory;
    auto* DebugState = (debug_state*)GameMemory->DebugMemory; 
    
    //  Initialization of the game
    if(!PermState->IsInitialized) {
        G_Log("[Permanent] Init Begin\n");
        // NOTE(Momo): Arenas
        PermState = ARENA_BOOT_STRUCT(permanent_state,
                                      arena,
                                      GameMemory->PermanentMemory, 
                                      GameMemory->PermanentMemorySize);
        
        PermState->ModeArena = PermState->arena.mark();
        PermState->CurrentGameMode = GameModeType_None;
        PermState->NextGameMode = GameModeType_Main;
        PermState->IsInitialized = true;
        PermState->IsPaused = false;
        
        Renderer_SetDesignResolution(G_Renderer,
                                     Game_DesignWidth, 
                                     Game_DesignHeight);
        PermState->GameSpeed = 1.f;
        G_Log("[Permanent] Init End\n");
        
    }
    
    
    
    if (!TranState->IsInitialized) {
        G_Log("[Transient] Init Begin\n");
        TranState = ARENA_BOOT_STRUCT(transient_state,
                                      arena,
                                      GameMemory->TransientMemory, 
                                      GameMemory->TransientMemorySize);
        b8 Success = Assets_Init(&TranState->Assets,
                                 &TranState->arena);
        
        if(!Success) {
            G_Log("[Transient] Failed to initialize assets\n");
            return false;
        }
        G_Assets = &TranState->Assets;
        
        Success = AudioMixer_Init(&TranState->Mixer, 1.f, 32, &TranState->arena);
        if (!Success) {
            G_Log("[Transient] Failed to initialize audio\n");
            return false;
        }
        
        TranState->IsInitialized = true;
        G_Log("[Transient] Init End\n");
    }
    
    if (!DebugState->IsInitialized) {
        G_Log("[Debug] Init Begin\n");
        DebugState = ARENA_BOOT_STRUCT(debug_state,
                                       arena,
                                       GameMemory->DebugMemory,
                                       GameMemory->DebugMemorySize);
        // Init inspector
        DebugInspector_Init(&DebugState->Inspector, &DebugState->arena);
        
        
        // Init console
        {
            String Buffer = {};
            init(&Buffer, "jump");
            
            if (!DebugConsole_Init(&DebugState->Console,
                                   &DebugState->arena)) {
                return false;
            }
            
            if (!DebugConsole_AddCmd(&DebugState->Console, 
                                     Buffer, 
                                     CmdJump, 
                                     DebugState)) {
                return false;
            }
        }
        
        DebugState->PermanentState = PermState;
        DebugState->TransientState = TranState;
        DebugState->IsInitialized = true;
        G_Log("[Debug] Init End\n");
    }
    
    // NOTE(Momo): Input
    // TODO(Momo): Consider putting everything into a Debug_Update()
    // Or, change seperate variable state into inspector and update seperately
    if (Button_IsPoked(G_Input->ButtonInspector)) {
        DebugState->Inspector.IsActive = !DebugState->Inspector.IsActive;
    }
    //StartProfiling(Test);
    DebugInspector_Begin(&DebugState->Inspector);
    DebugConsole_Update(&DebugState->Console, DeltaTime);
    //EndProfiling(Test);
    
#if 0
    G_Log("%s %s %d: %d\n", 
          G_ProfilerEntries[0].CustomName,
          G_ProfilerEntries[0].FunctionName,
          G_ProfilerEntries[0].LineNumber,
          G_ProfilerEntries[0].HitCount,
          G_ProfilerEntries[0].Cycles);
#endif
    
    
    // NOTE(Momo): Pause
    if (Button_IsPoked(G_Input->ButtonPause)) {
        PermState->IsPaused = !PermState->IsPaused;
    }
    if (PermState->IsPaused) {
        DeltaTime = 0.f;
    }
    
    // NOTE(Momo): Speed up/down
    if (Button_IsPoked(G_Input->ButtonSpeedDown)) {
        PermState->GameSpeed -= 0.1f;
    }
    if (Button_IsPoked(G_Input->ButtonSpeedUp)) {
        PermState->GameSpeed += 0.1f;
    }
    DeltaTime *= PermState->GameSpeed;
    
    
    // NOTE(Momo): Clean state/Switch states
    if (PermState->NextGameMode != GameModeType_None) {
        PermState->ModeArena.revert();
        Arena* ModeArena = PermState->ModeArena;
        
        switch(PermState->NextGameMode) {
            case GameModeType_Splash: {
                PermState->SplashMode = ModeArena->push_struct<game_mode_splash>(); 
                SplashMode_Init(PermState);
            } break;
            case GameModeType_Main: {
                PermState->MainMode = ModeArena->push_struct<game_mode_main>(); 
                if (!MainMode_Init(PermState, TranState, DebugState)){
                    return false;
                }
            } break;
            case GameModeType_Sandbox: {
                PermState->SandboxMode = ModeArena->push_struct<game_mode_sandbox>(); 
                SandboxMode_Init(PermState);
            } break;
            case GameModeType_AnimeTest: {
                PermState->AnimeTestMode = ModeArena->push_struct<game_mode_anime_test>(); 
                AnimeTestMode_Init(PermState);
            } break;
            default: {
                return false;
            }
        }
        
        PermState->CurrentGameMode = PermState->NextGameMode;
        PermState->NextGameMode = GameModeType_None;
    }
    
    String Buffer = {};
    init(&Buffer, "Debug Memory: ");
    DebugInspector_PushU32(&DebugState->Inspector, 
                           Buffer,
                           DebugState->arena.remaining());
    init(&Buffer, "Mode Memory: ");
    DebugInspector_PushU32(&DebugState->Inspector, 
                           Buffer,
                           PermState->ModeArena.arena->remaining());
    init(&Buffer, "TranState Memory: ");
    DebugInspector_PushU32(&DebugState->Inspector, 
                           Buffer,
                           TranState->arena.remaining());
    
    
    // State update
    switch(PermState->CurrentGameMode) {
        case GameModeType_Splash: {
            SplashMode_Update(PermState, 
                              TranState,
                              DeltaTime);
        } break;
        case GameModeType_Main: {
            MainMode_Update(PermState, 
                            TranState,
                            DebugState,
                            DeltaTime);
        } break; 
        case GameModeType_Sandbox: {
            SandboxMode_Update(PermState, 
                               TranState,
                               DeltaTime);
        } break;
        case GameModeType_AnimeTest: {
            AnimeTestMode_Update(PermState, 
                                 TranState,
                                 DeltaTime);
        } break;
        default: {
            return false;
        }
    }
    
    DebugConsole_Render(&DebugState->Console);
    DebugInspector_End(&DebugState->Inspector);
    
    AudioMixer_Update(&TranState->Mixer, Audio);
    
    
    
    return true;
}


