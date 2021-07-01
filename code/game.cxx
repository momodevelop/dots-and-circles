
#include "game.h"

// cmd: jump main/menu/atlas_test/etc...
static inline void 
CmdJump(Debug_Console* Console, void* Context, String Arguments) {
    auto* DebugState = (debug_state*)Context;
    permanent_state* PermState = DebugState->PermanentState;
    
    defer{ G_Scratch->clear(); };
    
    String Buffer = {};
    String_Split_Result ArgList = Arguments.split(G_Scratch, ' ');
    if ( ArgList.item_count != 2 ) {
        // Expect two arguments
        Buffer.init("Expected only 2 arguments");
        Console->push_info(Buffer, 
                           C4F_RED);
        return;
    }
    
    String StateToChangeTo = ArgList.items[1];
    if (StateToChangeTo == "main") {
        Buffer.init("Jumping to Main");
        Console->push_info(Buffer, 
                           C4F_YELLOW);
        PermState->NextGameMode = GameModeType_Main;
    }
    else if (StateToChangeTo == "splash") {
        Buffer.init("Jumping to Splash");
        Console->push_info(Buffer,  
                           C4F_YELLOW);
        PermState->NextGameMode = GameModeType_Splash;
    }
    else if (StateToChangeTo == "sandbox") {
        Buffer.init("Jumping to Sandbox");
        Console->push_info(Buffer, 
                           C4F_YELLOW);
        PermState->NextGameMode = GameModeType_Sandbox;
    }
    else if (StateToChangeTo == "anime") {
        Buffer.init("Jumping to Anime");
        Console->push_info(Buffer, 
                           C4F_YELLOW);
        PermState->NextGameMode = GameModeType_AnimeTest;
    }
    else {
        Buffer.init("Invalid state to jump to");
        Console->push_info(Buffer, 
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
        b8 Success = TranState->assets.init(&TranState->arena);
        
        if(!Success) {
            G_Log("[Transient] Failed to initialize assets\n");
            return false;
        }
        G_Assets = &TranState->assets;
        
        Success =  TranState->Mixer.init(1.f, 32, &TranState->arena);
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
            Buffer.init("jump");
            
            if (!DebugState->Console.init(&DebugState->arena, 16)) {
                return false;
            }
            
            if (!DebugState->Console.add_command(Buffer, 
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
    DebugState->Console.update(DeltaTime);
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
                PermState->AnimeTestMode = ModeArena->push_struct<Game_Mode_Anime_Test>(); 
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
    Buffer.init("Debug Memory: ");
    DebugInspector_PushU32(&DebugState->Inspector, 
                           Buffer,
                           DebugState->arena.remaining());
    Buffer.init("Mode Memory: ");
    DebugInspector_PushU32(&DebugState->Inspector, 
                           Buffer,
                           PermState->ModeArena.arena->remaining());
    Buffer.init("TranState Memory: ");
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
    
    DebugState->Console.render();
    DebugInspector_End(&DebugState->Inspector);
    
    TranState->Mixer.update(Audio);
    
    
    
    return true;
}


