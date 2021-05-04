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
                              Color_Red);
        return;
    }
    
    u8_cstr StateToChangeTo = ArgList.Items[1];
    if (U8CStr_CmpSiStr(StateToChangeTo, "main")) {
        U8CStr_InitFromSiStr(&Buffer, "Jumping to Main");
        DebugConsole_PushInfo(Console, 
                              Buffer, 
                              Color_Yellow);
        PermState->NextGameMode = GameModeType_Main;
    }
    else if (U8CStr_CmpSiStr(StateToChangeTo, "splash")) {
        U8CStr_InitFromSiStr(&Buffer, "Jumping to Splash");
        DebugConsole_PushInfo(Console, 
                              Buffer,  
                              Color_Yellow);
        PermState->NextGameMode = GameModeType_Splash;
    }
    else if (U8CStr_CmpSiStr(StateToChangeTo, "sandbox")) {
        U8CStr_InitFromSiStr(&Buffer, "Jumping to Sandbox");
        DebugConsole_PushInfo(Console, 
                              Buffer, 
                              Color_Yellow);
        PermState->NextGameMode = GameModeType_Sandbox;
    }
    else if (U8CStr_CmpSiStr(StateToChangeTo, "anime")) {
        U8CStr_InitFromSiStr(&Buffer, "Jumping to Anime");
        DebugConsole_PushInfo(Console, 
                              Buffer, 
                              Color_Yellow);
        PermState->NextGameMode = GameModeType_AnimeTest;
    }
    else {
        U8CStr_InitFromSiStr(&Buffer, "Invalid state to jump to");
        DebugConsole_PushInfo(Console, 
                              Buffer, 
                              Color_Red);
    }
    
}
extern "C" 
GameUpdateFunc(GameUpdate) 
{
    G_Platform = Platform;
    auto* PermState = (permanent_state*)GameMemory->PermanentMemory;
    auto* TranState = (transient_state*)GameMemory->TransientMemory;
    auto* DebugState = (debug_state*)GameMemory->DebugMemory; 
    //  Initialization of the game
    if(!PermState->IsInitialized) {
        // NOTE(Momo): Arenas
        PermState = Arena_BootupStruct(permanent_state,
                                       MainArena,
                                       GameMemory->PermanentMemory, 
                                       GameMemory->PermanentMemorySize);
        
        PermState->ModeArena = Arena_SubArena(&PermState->MainArena, 
                                              Arena_Remaining(PermState->MainArena));
        PermState->CurrentGameMode = GameModeType_None;
        PermState->NextGameMode = GameModeType_Splash;
        PermState->IsInitialized = True;
        PermState->IsRunning = True;
        PermState->IsPaused = False;
        
        Renderer_SetDesignResolution(RenderCommands, 
                                     Game_DesignWidth, 
                                     Game_DesignHeight);
    }
    
    if (!TranState->IsInitialized) {
        TranState = Arena_BootupStruct(transient_state,
                                       Arena,
                                       GameMemory->TransientMemory, 
                                       GameMemory->TransientMemorySize);
        
        b32 Success = Assets_Init(&TranState->Assets,
                                  &TranState->Arena,
                                  Platform);
        
        Assert(Success);
        
        
        TranState->IsInitialized = True;
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
        DebugState->IsInitialized = True;
    }
    
    // NOTE(Momo): Input
    // TODO(Momo): Consider putting everything into a Debug_Update()
    // Or, change seperate variable state into inspector and update seperately
    if (Button_IsPoked(Input->ButtonInspector)) {
        DebugState->Inspector.IsActive = !DebugState->Inspector.IsActive;
    }
    DebugInspector_Begin(&DebugState->Inspector);
    
    
    DebugConsole_Update(&DebugState->Console, Input, DeltaTime);
    
    if (PermState->IsPaused) {
        DeltaTime = 0.f;
    }
    
#if 0
    static f32 TSine = 0.f;
    static u32 SoundCounter = 0;
    // TODO: Shift this part to game code
    s16* SampleOut = Audio->SampleBuffer;
    sound* Sound = Assets_GetSound(&TranState->Assets, Sound_Test);
    for(u32 I = 0; I < Audio->SampleCount; ++I) {
        
        const f32 Volume = 0.2f;
        s16 SampleValue = s16(Sound->Data[SoundCounter++] * Volume);
        if (SoundCounter >= Sound->DataCount) {
            SoundCounter = 0;
        }
        // Based on number of channels!
        *SampleOut++ = SampleValue; // Left Speaker
        *SampleOut++ = SampleValue; // Right Speaker
        TSine += DeltaTime;
    }
#else
    s16* SampleOut = Audio->SampleBuffer;
    for(u32 I = 0; I < Audio->SampleCount; ++I) {
        // Based on number of channels!
        *SampleOut++ = 0; // Left Speaker
        *SampleOut++ = 0; // Right Speaker
    }
#endif
    
    // Clean state/Switch states
    if (PermState->NextGameMode != GameModeType_None) {
        Arena_Clear(&PermState->ModeArena);
        arena* ModeArena = &PermState->ModeArena;
        
        switch(PermState->NextGameMode) {
            case GameModeType_Splash: {
                PermState->SplashMode = 
                    Arena_PushStruct(game_mode_splash, ModeArena); 
                InitSplashMode(PermState);
            } break;
            case GameModeType_Main: {
                PermState->MainMode = 
                    Arena_PushStruct(game_mode_main, ModeArena); 
                InitMainMode(PermState, TranState, DebugState);
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
            }
        }
        
        PermState->CurrentGameMode = PermState->NextGameMode;
        PermState->NextGameMode = GameModeType_None;
    }
    
    u8_cstr Buffer = {};
    U8CStr_InitFromSiStr(&Buffer, "Debug Memory: ");
    DebugInspector_PushU32(&DebugState->Inspector, 
                           Buffer,
                           Arena_Remaining(DebugState->Arena));
    U8CStr_InitFromSiStr(&Buffer, "Mode Memory: ");
    DebugInspector_PushU32(&DebugState->Inspector, 
                           Buffer,
                           Arena_Remaining(PermState->ModeArena));
    U8CStr_InitFromSiStr(&Buffer, "TranState Memory: ");
    DebugInspector_PushU32(&DebugState->Inspector, 
                           Buffer,
                           Arena_Remaining(TranState->Arena));
    
    
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
            UpdateMainMode(PermState, 
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
            Assert(false);
        }
    }
    
    DebugConsole_Render(&DebugState->Console, RenderCommands, &TranState->Assets);
    DebugInspector_End(&DebugState->Inspector, RenderCommands, &TranState->Assets);
    
    return PermState->IsRunning;
}


