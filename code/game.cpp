#include "game.h"
#include "game_assets.h"
#include "game_mode_splash.h"
#include "game_mode_main.h"
#include "game_mode_sandbox.h"
#include "game_console.h"
#include "game_text.h"
#include "mm_arena.h"
#include "mm_list.h"
#include "mm_maths.h"
#include "mm_colors.h"
#include "mm_link_list.h"

#if INTERNAL 
// cmd: jump main/menu/atlas_test/etc...
static inline void 
CmdJump(void * Context, string Arguments) {
    auto* PermState = (permanent_state*)Context;
    
    auto Scratch = BeginScratch(&PermState->ModeArena);
    Defer {  EndScratch(&Scratch); };

    dlink_list<string> ArgList = DelimitSplit(Arguments, Scratch, ' ');
    if ( ArgList.Count != 2 ) {
        // Expect two arguments
        PushInfo(&PermState->Console, 
                 String("Expected only 2 arguments"), 
                 Color_Red);
        return;
    }

    string StateToChangeTo = ArgList[1];
    if (StateToChangeTo == String("main")) {
        PushInfo(&PermState->Console, 
                 String("Jumping to Main"), 
                 Color_Yellow);
        PermState->NextGameMode = GameModeType_Main;
    }
    else if (StateToChangeTo == String("splash")) {
        PushInfo(&PermState->Console, 
                 String("Jumping to Splash"),  
                 Color_Yellow);
        PermState->NextGameMode = GameModeType_Splash;
    }
    else if (StateToChangeTo == String("sandbox")) {
        PushInfo(&PermState->Console, 
                 String("Jumping to Sandbox"), 
                 Color_Yellow);
        PermState->NextGameMode = GameModeType_Sandbox;
    }
    else {
        PushInfo(&PermState->Console, 
                 String("Invalid state to jump to"), 
                 Color_Red);
    }
}
#endif

extern "C" 
GameUpdateFunc(GameUpdate) 
{
    auto* PermState = (permanent_state*)GameMemory->PermanentMemory;
    auto* TranState = (transient_state*)GameMemory->TransientMemory;
       
    //  Initialization of the game
    if(!PermState->IsInitialized) {
        // NOTE(Momo): Arenas
        PermState = BootstrapStruct(permanent_state,
                                    MainArena,
                                    GameMemory->PermanentMemory, 
                                    GameMemory->PermanentMemorySize);

        // Console Init
        {
            v2f Dimensions = { Global_DesignWidth, 240.f };
            v3f Position = { 
                -Global_DesignWidth * 0.5f + Dimensions.W * 0.5f, 
                -Global_DesignHeight * 0.5f + Dimensions.H * 0.5f, 
                Global_DesignDepth * 0.5f - 1.f
            };
            PermState->Console = 
                GameConsole(&PermState->MainArena, 
                            5, 
                            110, 
                            1,
                            Color_Grey3,
                            Color_White,
                            Color_Grey2,
                            Color_White,
                            Dimensions,
                            Position); 
            
            RegisterCommand(&PermState->Console, 
                            String("jump"), 
                            CmdJump, 
                            PermState);
        }

        // NOTE(Momo): Arena for modes
        PermState->ModeArena = SubArena(&PermState->MainArena, 
                                        Remaining(PermState->MainArena));
        PermState->CurrentGameMode = GameModeType_None;
        PermState->NextGameMode = GameModeType_Splash;
        PermState->IsInitialized = true;

        // NOTE(Momo): Set design resolution for game
        PushSetDesignResolution(RenderCommands, 
                                (u32)Global_DesignWidth, 
                                (u32)Global_DesignHeight);
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

    // Console
    Update(&PermState->Console, Input);

    // Clean state/Switch states
    if (PermState->NextGameMode != GameModeType_None) {
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
                InitMainMode(PermState, TranState);
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
    Render(&PermState->Console, RenderCommands, TranState->Assets);
}

