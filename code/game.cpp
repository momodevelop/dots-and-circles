// This is purely for convienience.
// There are some things that are easier to debug 
// with console logging than other methods.
// TODO: Perhaps we have REAL in game logging?
#define GLOBAL_LOG 0
#if GLOBAL_LOG
void (*Log)(const char* Format, ...);
#endif

#include "game.h"
#include "game_assets.h"
#include "game_mode_splash.h"
#include "game_mode_main.h"
#include "game_mode_sandbox.h"
#include "game_debug.h"
#include "game_draw.h"

#if INTERNAL 
// cmd: jump main/menu/atlas_test/etc...
static inline void 
CmdJump(debug_console* Console, void* Context, string Arguments) {
    auto* DebugState = (debug_state*)Context;
    permanent_state* PermState = DebugState->PermanentState;
    scratch Scratchpad = BeginScratch(&DebugState->Arena);
    Defer{ EndScratch(&Scratchpad); };
    array<string> ArgList = DelimitSplit(Arguments, Scratchpad, ' ');
    if ( ArgList.Count != 2 ) {
        // Expect two arguments
        PushInfo(Console, 
                 String("Expected only 2 arguments"), 
                 Color_Red);
        return;
    }
    
    string StateToChangeTo = ArgList[1];
    if (StateToChangeTo == String("main")) {
        PushInfo(Console, 
                 String("Jumping to Main"), 
                 Color_Yellow);
        PermState->NextGameMode = GameModeType_Main;
    }
    else if (StateToChangeTo == String("splash")) {
        PushInfo(Console, 
                 String("Jumping to Splash"),  
                 Color_Yellow);
        PermState->NextGameMode = GameModeType_Splash;
    }
    else if (StateToChangeTo == String("sandbox")) {
        PushInfo(Console, 
                 String("Jumping to Sandbox"), 
                 Color_Yellow);
        PermState->NextGameMode = GameModeType_Sandbox;
    }
    else {
        PushInfo(Console, 
                 String("Invalid state to jump to"), 
                 Color_Red);
    }
    
}
#endif




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
        PermState = BootstrapStruct(permanent_state,
                                    MainArena,
                                    GameMemory->PermanentMemory, 
                                    GameMemory->PermanentMemorySize);
        
        PermState->ModeArena = SubArena(&PermState->MainArena, 
                                        Remaining(PermState->MainArena));
        PermState->CurrentGameMode = GameModeType_None;
        PermState->NextGameMode = GameModeType_Splash;
        PermState->IsInitialized = true;
        PermState->IsRunning = true;
        
        PushSetDesignResolution(RenderCommands, 
                                (u32)Global_DesignSpace.W, 
                                (u32)Global_DesignSpace.H);
    }
    
    if (!TranState->IsInitialized) {
        TranState = BootstrapStruct(transient_state,
                                    Arena,
                                    GameMemory->TransientMemory, 
                                    GameMemory->TransientMemorySize);
        
        TranState->Assets = AllocateAssets(&TranState->Arena, 
                                           Platform);
        Assert(TranState->Assets);
        
        
        TranState->IsInitialized = true;
    }
    
    if (!DebugState->IsInitialized) {
        DebugState = BootstrapStruct(debug_state,
                                     Arena,
                                     GameMemory->DebugMemory,
                                     GameMemory->DebugMemorySize);
        
        DebugState->Variables = List<debug_variable>(&DebugState->Arena, 16); 
        
        // Init console
        {
            v2f Dimensions = { Global_DesignSpace.W, 240.f };
            v3f Position = { 
                -Global_DesignSpace.W * 0.5f + Dimensions.W * 0.5f, 
                -Global_DesignSpace.H * 0.5f + Dimensions.H * 0.5f, 
                Global_DesignSpace.D * 0.5f - 1.f
            };
            
            DebugState->Console = 
                DebugConsole(&DebugState->Arena, 
                             5, 
                             110, 
                             16,
                             Position - v3f{0.f, 240.f},
                             Position,
                             0.1f,
                             Dimensions,
                             0.5f,
                             0.025f);
            
            RegisterCommand(&DebugState->Console, 
                            String("jump"), 
                            CmdJump, 
                            DebugState);
        }
        
        DebugState->PermanentState = PermState;
        DebugState->TransientState = TranState;
        DebugState->IsInitialized = true;
    }
    
    
    Update(&DebugState->Console, Input, DeltaTime);
    
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
        switch(PermState->CurrentGameMode) {
            case GameModeType_Splash: {
            } break;
            case GameModeType_Main: {
                UninitMainMode(DebugState);
            } break;
            case GameModeType_Sandbox: {
            } break;
            default: {
            }
        }
        
        
        
        Clear(&PermState->ModeArena);
        arena* ModeArena = &PermState->ModeArena;
        
        switch(PermState->NextGameMode) {
            case GameModeType_Splash: {
                PermState->SplashMode = 
                    PushStruct<game_mode_splash>(ModeArena); 
                InitSplashMode(PermState);
            } break;
            case GameModeType_Main: {
                PermState->MainMode = 
                    PushStruct<game_mode_main>(ModeArena); 
                InitMainMode(PermState, TranState, DebugState);
            } break;
            case GameModeType_Sandbox: {
                PermState->SandboxMode = 
                    PushStruct<game_mode_sandbox>(ModeArena); 
                InitSandboxMode(PermState);
            } break;
            default: {
            }
        }
        
        PermState->CurrentGameMode = PermState->NextGameMode;
        PermState->NextGameMode = GameModeType_None;
    }
    
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
    Render(DebugState, TranState->Assets, RenderCommands);
    //Render(&DebugState->Console, RenderCommands, TranState->Assets);
    
    return PermState->IsRunning;
}

