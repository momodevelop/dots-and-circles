
#include "game.h"

// cmd: jump main/menu/atlas_test/etc...
static inline void 
CmdJump(debug_console* Console, void* Context, u8_cstr Arguments) {
    auto* DebugState = (debug_state*)Context;
    permanent_state* PermState = DebugState->PermanentState;
    arena_mark Scratch = Arena_Mark(&DebugState->Arena);
    Defer{ Arena_Revert(&Scratch); };
    u8_cstr Buffer = {};
    
    u8_cstr_split_res ArgList = U8CStr_SplitByDelimiter(Arguments, Scratch.Arena, ' ');
    if ( ArgList.ItemCount != 2 ) {
        // Expect two arguments
        U8CStr_InitFromSiStr(&Buffer, "Expected only 2 arguments");
        DebugConsole_PushInfo(Console, 
                              Buffer, 
                              C4f_Red);
        return;
    }
    
    u8_cstr StateToChangeTo = ArgList.Items[1];
    if (U8CStr_CmpSiStr(StateToChangeTo, "main")) {
        U8CStr_InitFromSiStr(&Buffer, "Jumping to Main");
        DebugConsole_PushInfo(Console, 
                              Buffer, 
                              C4f_Yellow);
        PermState->NextGameMode = GameModeType_Main;
    }
    else if (U8CStr_CmpSiStr(StateToChangeTo, "splash")) {
        U8CStr_InitFromSiStr(&Buffer, "Jumping to Splash");
        DebugConsole_PushInfo(Console, 
                              Buffer,  
                              C4f_Yellow);
        PermState->NextGameMode = GameModeType_Splash;
    }
    else if (U8CStr_CmpSiStr(StateToChangeTo, "sandbox")) {
        U8CStr_InitFromSiStr(&Buffer, "Jumping to Sandbox");
        DebugConsole_PushInfo(Console, 
                              Buffer, 
                              C4f_Yellow);
        PermState->NextGameMode = GameModeType_Sandbox;
    }
    else if (U8CStr_CmpSiStr(StateToChangeTo, "anime")) {
        U8CStr_InitFromSiStr(&Buffer, "Jumping to Anime");
        DebugConsole_PushInfo(Console, 
                              Buffer, 
                              C4f_Yellow);
        PermState->NextGameMode = GameModeType_AnimeTest;
    }
    else {
        U8CStr_InitFromSiStr(&Buffer, "Invalid state to jump to");
        DebugConsole_PushInfo(Console, 
                              Buffer, 
                              C4f_Red);
    }
    
}
extern "C" 
GameUpdateFunc(GameUpdate) 
{
#if INTERNAL
    G_Platform = Platform;
#endif 
    
    auto* PermState = (permanent_state*)GameMemory->PermanentMemory;
    auto* TranState = (transient_state*)GameMemory->TransientMemory;
    auto* DebugState = (debug_state*)GameMemory->DebugMemory; 
    //  Initialization of the game
    if(!PermState->IsInitialized) {
        // NOTE(Momo): Arenas
        PermState = Arena_BootupStruct(permanent_state,
                                       Arena,
                                       GameMemory->PermanentMemory, 
                                       GameMemory->PermanentMemorySize);
        
        PermState->ModeArena = Arena_Mark(&PermState->Arena);
        PermState->CurrentGameMode = GameModeType_None;
        PermState->NextGameMode = GameModeType_Main;
        PermState->IsInitialized = true;
        PermState->IsPaused = false;
        
        Renderer_SetDesignResolution(RenderCommands, 
                                     Game_DesignWidth, 
                                     Game_DesignHeight);
        PermState->GameSpeed = 1.f;
    }
    
    if (!TranState->IsInitialized) {
        Platform->LogFp("Initializing Transient State\n");
        TranState = Arena_BootupStruct(transient_state,
                                       Arena,
                                       GameMemory->TransientMemory, 
                                       GameMemory->TransientMemorySize);
        
        b8 Success = Assets_Init(&TranState->Assets,
                                 &TranState->Arena,
                                 Platform);
        if(!Success) {
            return false;
        }
        
        Success = AudioMixer_Init(&TranState->Mixer, 1.f, 32, &TranState->Arena);
        if (!Success) {
            return false;
        }
        
        TranState->IsInitialized = true;
    }
    
    if (!DebugState->IsInitialized) {
        DebugState = Arena_BootupStruct(debug_state,
                                        Arena,
                                        GameMemory->DebugMemory,
                                        GameMemory->DebugMemorySize);
        // Init inspector
        DebugInspector_Init(&DebugState->Inspector, &DebugState->Arena);
        
        
        // Init console
        {
            u8_cstr Buffer = {};
            U8CStr_InitFromSiStr(&Buffer, "jump");
            DebugConsole_Init(&DebugState->Console,
                              &DebugState->Arena);
            DebugConsole_AddCmd(&DebugState->Console, 
                                Buffer, 
                                CmdJump, 
                                DebugState);
        }
        
        DebugState->PermanentState = PermState;
        DebugState->TransientState = TranState;
        DebugState->IsInitialized = true;
    }
    
    // NOTE(Momo): Input
    // TODO(Momo): Consider putting everything into a Debug_Update()
    // Or, change seperate variable state into inspector and update seperately
    if (Button_IsPoked(Input->ButtonInspector)) {
        DebugState->Inspector.IsActive = !DebugState->Inspector.IsActive;
    }
    DebugInspector_Begin(&DebugState->Inspector);
    DebugConsole_Update(&DebugState->Console, Input, DeltaTime);
    
    // NOTE(Momo): Pause
    if (Button_IsPoked(Input->ButtonPause)) {
        PermState->IsPaused = !PermState->IsPaused;
    }
    if (PermState->IsPaused) {
        DeltaTime = 0.f;
    }
    
    // NOTE(Momo): Speed up/down
    if (Button_IsPoked(Input->ButtonSpeedDown)) {
        PermState->GameSpeed -= 0.1f;
    }
    if (Button_IsPoked(Input->ButtonSpeedUp)) {
        PermState->GameSpeed += 0.1f;
    }
    DeltaTime *= PermState->GameSpeed;
    
    
    // NOTE(Momo): Clean state/Switch states
    if (PermState->NextGameMode != GameModeType_None) {
        Arena_Revert(&PermState->ModeArena);
        arena* ModeArena = PermState->ModeArena;
        
        switch(PermState->NextGameMode) {
            case GameModeType_Splash: {
                PermState->SplashMode = 
                    Arena_PushStruct(game_mode_splash, ModeArena); 
                InitSplashMode(PermState);
            } break;
            case GameModeType_Main: {
                PermState->MainMode = 
                    Arena_PushStruct(game_mode_main, ModeArena); 
                if (!Main_Init(PermState, TranState, DebugState)){
                    return false;
                }
            } break;
            case GameModeType_Sandbox: {
                PermState->SandboxMode = 
                    Arena_PushStruct(game_mode_sandbox, ModeArena); 
                InitSandboxMode(PermState);
            } break;
            case GameModeType_AnimeTest: {
                PermState->AnimeTestMode = 
                    Arena_PushStruct(game_mode_anime_test, ModeArena); 
                InitAnimeTestMode(PermState);
            } break;
            default: {
                return false;
            }
        }
        
        PermState->CurrentGameMode = PermState->NextGameMode;
        PermState->NextGameMode = GameModeType_None;
    }
    
    u8_cstr Buffer = {};
    U8CStr_InitFromSiStr(&Buffer, "Debug Memory: ");
    DebugInspector_PushU32(&DebugState->Inspector, 
                           Buffer,
                           Arena_Remaining(&DebugState->Arena));
    U8CStr_InitFromSiStr(&Buffer, "Mode Memory: ");
    DebugInspector_PushU32(&DebugState->Inspector, 
                           Buffer,
                           Arena_Remaining(PermState->ModeArena));
    U8CStr_InitFromSiStr(&Buffer, "TranState Memory: ");
    DebugInspector_PushU32(&DebugState->Inspector, 
                           Buffer,
                           Arena_Remaining(&TranState->Arena));
    
    
    // State update
    switch(PermState->CurrentGameMode) {
        case GameModeType_Splash: {
            UpdateSplashMode(PermState, 
                             TranState,
                             RenderCommands, 
                             Input, 
                             DeltaTime);
        } break;
        case GameModeType_Main: {
            Main_Update(PermState, 
                        TranState,
                        DebugState,
                        RenderCommands, 
                        Input, 
                        DeltaTime);
        } break; 
        case GameModeType_Sandbox: {
            UpdateSandboxMode(PermState, 
                              TranState,
                              RenderCommands, 
                              Input, 
                              DeltaTime);
        } break;
        case GameModeType_AnimeTest: {
            UpdateAnimeTestMode(PermState, 
                                TranState,
                                RenderCommands, 
                                Input, 
                                DeltaTime);
        } break;
        default: {
            return false;
        }
    }
    
    DebugConsole_Render(&DebugState->Console, RenderCommands, &TranState->Assets);
    DebugInspector_End(&DebugState->Inspector, RenderCommands, &TranState->Assets);
    
    AudioMixer_Update(&TranState->Mixer, Audio, &TranState->Assets);
    
    
    
    return true;
}


