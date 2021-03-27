// This is purely for convienience.
// There are some things that are easier to debug 
// with console logging than other methods.
// TODO: Perhaps we have REAL in game logging?
#define GLOBAL_LOG 0
#if GLOBAL_LOG
void (*Log)(const char* Format, ...);
#endif

#include "game.h"

// cmd: jump main/menu/atlas_test/etc...
static inline void 
CmdJump(debug_console* Console, void* Context, u8_cstr Arguments) {
    auto* DebugState = (debug_state*)Context;
    permanent_state* PermState = DebugState->PermanentState;
    arena_mark Scratch = Arena_Mark(&DebugState->Arena);
    Defer{ Arena_Revert(&Scratch); };
    u8_cstr_split_res ArgList = U8CStr_SplitByDelimiter(Arguments, Scratch.Arena, ' ');
    if ( ArgList.ItemCount != 2 ) {
        // Expect two arguments
        PushInfo(Console, 
                 U8CStr_FromSiStr("Expected only 2 arguments"), 
                 Color_Red);
        return;
    }
    
    u8_cstr StateToChangeTo = ArgList.Items[1];
    if (U8CStr_Compare(StateToChangeTo, U8CStr_FromSiStr("main"))) {
        PushInfo(Console, 
                 U8CStr_FromSiStr("Jumping to Main"), 
                 Color_Yellow);
        PermState->NextGameMode = GameModeType_Main;
    }
    else if (U8CStr_Compare(StateToChangeTo, U8CStr_FromSiStr("splash"))) {
        PushInfo(Console, 
                 U8CStr_FromSiStr("Jumping to Splash"),  
                 Color_Yellow);
        PermState->NextGameMode = GameModeType_Splash;
    }
    else if (U8CStr_Compare(StateToChangeTo, U8CStr_FromSiStr("sandbox"))) {
        PushInfo(Console, 
                 U8CStr_FromSiStr("Jumping to Sandbox"), 
                 Color_Yellow);
        PermState->NextGameMode = GameModeType_Sandbox;
    }
    else {
        PushInfo(Console, 
                 U8CStr_FromSiStr("Invalid state to jump to"), 
                 Color_Red);
    }
    
}
extern "C" 
GameUpdateFunc(GameUpdate) 
{
#if GLOBAL_LOG
    Log = Platform->Log;
#endif
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
        
        PushSetDesignResolution(RenderCommands, 
                                Game_DesignWidth, 
                                Game_DesignHeight);
    }
    
    if (!TranState->IsInitialized) {
        TranState = Arena_BootupStruct(transient_state,
                                       Arena,
                                       GameMemory->TransientMemory, 
                                       GameMemory->TransientMemorySize);
        
        TranState->Assets = AllocateAssets(&TranState->Arena, 
                                           Platform);
        Assert(TranState->Assets);
        
        
        TranState->IsInitialized = true;
    }
    
    if (!DebugState->IsInitialized) {
        DebugState = Arena_BootupStruct(debug_state,
                                        Arena,
                                        GameMemory->DebugMemory,
                                        GameMemory->DebugMemorySize);
        // Init inspector
        DebugInspector_Init(&DebugState->Inspector);
        
        
        // Init console
        {
            
            DebugConsole_Create(&DebugState->Console,
                                &DebugState->Arena);
            DebugConsole_AddCmd(&DebugState->Console, 
                                U8CStr_FromSiStr("jump"), 
                                CmdJump, 
                                DebugState);
        }
        
        DebugState->PermanentState = PermState;
        DebugState->TransientState = TranState;
        DebugState->IsInitialized = True;
    }
    
    DebugInspector_Begin(&DebugState->Inspector);
    // NOTE(Momo): Input
    // TODO(Momo): Consider putting everything into a Debug_Update()
    // Or, change seperate variable state into inspector and update seperately
    if (IsPoked(Input->ButtonSaveState)) {
        Platform->SaveStateFp();
    }
    else if(IsPoked(Input->ButtonLoadState)) {
        Platform->LoadStateFp();
    }
    if (IsPoked(Input->ButtonPauseState)) {
        PermState->IsPaused = !PermState->IsPaused;
    }
    if (IsPoked(Input->ButtonInspector)) {
        DebugState->Inspector.IsActive = !DebugState->Inspector.IsActive;
    }
    
    
    DebugConsole_Update(&DebugState->Console, Input, DeltaTime);
    
    if (PermState->IsPaused) {
        DeltaTime = 0.f;
    }
    
#if 0 
    static f32 TSine = 0.f;
    // TODO: Shift this part to game code
    i16* SampleOut = Audio->SampleBuffer;
    for(usize I = 0; I < Audio->SampleCount; ++I) {
        const i16 Volume = 3000;
        i16 SampleValue = (i16)(Sin(TSine) * Volume);
        // Based on number of channels!
        *SampleOut++ = SampleValue; // Left Speaker
        *SampleOut++ = SampleValue; // Right Speaker
        TSine += DeltaTime;
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
            default: {
            }
        }
        
        PermState->CurrentGameMode = PermState->NextGameMode;
        PermState->NextGameMode = GameModeType_None;
    }
    
    
    DebugInspector_PushU32(&DebugState->Inspector, 
                           U8CStr_FromSiStr("Debug Memory: "),
                           Arena_Remaining(DebugState->Arena));
    DebugInspector_PushU32(&DebugState->Inspector, 
                           U8CStr_FromSiStr("Mode Memory: "),
                           Arena_Remaining(PermState->ModeArena));
    DebugInspector_PushU32(&DebugState->Inspector, 
                           U8CStr_FromSiStr("TranState Memory: "),
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
        default: {
            Assert(false);
        }
    }
    
    // Render Console
    DebugConsole_Render(&DebugState->Console, RenderCommands, TranState->Assets);
    
    
    DebugInspector_End(&DebugState->Inspector, RenderCommands, TranState->Assets);
    
    return PermState->IsRunning;
}


