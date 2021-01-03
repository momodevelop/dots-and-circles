#include "game.h"
#include "game_assets.h"
#include "game_input.h"
#include "game_mode_splash.h"
#include "game_mode_menu.h"
#include "game_mode_main.h"
#include "game_mode_sandbox.h"
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
    game_state* GameState = (game_state*)Context;
    
    auto Scratch = BeginScratch(&GameState->DebugArena);
    Defer {  EndScratch(&Scratch); };

    dlink_list<string> ArgList = DelimitSplit(Arguments, Scratch, ' ');
    if ( ArgList.Count != 2 ) {
        // Expect two arguments
        PushDebugInfo(&GameState->DebugConsole, String("Expected only 2 arguments"), ColorRed);
        return;
    }

    string StateToChangeTo = ArgList[1];
    if (StateToChangeTo == String("main")) {
        PushDebugInfo(&GameState->DebugConsole, String("Jumping to Main"), ColorYellow);
        GameState->NextModeType = GameModeType_Main;
    }
    else if (StateToChangeTo == String("splash")) {
        PushDebugInfo(&GameState->DebugConsole, String("Jumping to Splash"), ColorYellow);
        GameState->NextModeType = GameModeType_Splash;
    }
    else if (StateToChangeTo == String("menu")) {
        PushDebugInfo(&GameState->DebugConsole, String("Jumping to Menu"), ColorYellow);
        GameState->NextModeType = GameModeType_Menu;
    }
    else if (StateToChangeTo == String("sandbox")) {
        PushDebugInfo(&GameState->DebugConsole, String("Jumping to Sandbox"), ColorYellow);
        GameState->NextModeType = GameModeType_Sandbox;
    }
    else {
        PushDebugInfo(&GameState->DebugConsole, String("Invalid state to jump to"), ColorRed);
    }
}
#endif

extern "C" 
GAME_UPDATE(GameUpdate) 
{
    game_state* GameState = (game_state*)GameMemory->MainMemory;
       
    // NOTE(Momo): Initialization of the game
    if(!GameState->IsInitialized) {
        // NOTE(Momo): Arenas
        GameState->MainArena = Arena((u8*)GameMemory->MainMemory + sizeof(game_state), 
                                     GameMemory->MainMemorySize - sizeof(game_state));

        // NOTE(Momo): Assets
        GameState->Assets = CreateAssets(
                &GameState->MainArena, 
                Platform, 
                RenderCommands, 
                String("yuu\0")
        );

        // NOTE(Momo): Arena for modes
        GameState->ModeArena = SubArena(&GameState->MainArena, 
                                        Remaining(GameState->MainArena));
        GameState->ModeType = GameModeType_None;
        GameState->NextModeType = GameModeType_Splash;
        GameState->IsInitialized = true;

        // NOTE(Momo): Set design resolution for game
        PushCommandSetDesignResolution(RenderCommands, 
                                       (u32)DesignWidth, 
                                       (u32)DesignHeight);

#if INTERNAL
        GameState->DebugArena = Arena(GameMemory->DebugMemory, 
                                      GameMemory->DebugMemorySize);
        GameState->DebugCommands = List<debug_command>(&GameState->DebugArena, 1);
        // Debug Console Init
        {
            debug_console Console = CreateDebugConsole(&GameState->DebugArena, 5, 110, 32);
            Console.InfoBgColor = { 0.3f, 0.3f, 0.3f, 1.f };
            Console.InfoDefaultColor = ColorWhite;
            Console.InputBgColor = { 0.2f, 0.2f, 0.2f, 1.f };
            Console.InputColor = ColorWhite;
            Console.Dimensions = { DesignWidth, 240.f };
            Console.Position = { 
                -DesignWidth * 0.5f + Console.Dimensions.W * 0.5f, 
                -DesignHeight * 0.5f + Console.Dimensions.H * 0.5f, 
                DesignDepth * 0.5f - 1.f
            };

            GameState->DebugConsole = Console;
        
        }
        Register(&GameState->DebugCommands, String("jump"), CmdJump, GameState);
#endif
    }

#if INTERNAL
    // System Debug
    {
        // F1 to toggle debug console
        if (IsPoked(Input->DebugKeys[GameDebugKey_F1])) {
            GameState->IsDebug = !GameState->IsDebug; 
        }

        if (IsPoked(Input->DebugKeys[GameDebugKey_F2])) {
            GameState->IsShowTicksElapsed = !GameState->IsShowTicksElapsed;
        }

        if (GameState->IsDebug) {
            debug_console* Console = &GameState->DebugConsole;
            if (Update(Console, Input)){ 
                Execute(GameState->DebugCommands, GetCommandString(Console));
            }
        }

    }

#endif


    // Clean state/Switch states
    if (GameState->NextModeType != GameModeType_None) {
        Clear(&GameState->ModeArena);
        arena* ModeArena = &GameState->ModeArena;
        switch(GameState->NextModeType) {
            case GameModeType_Splash: {
                GameState->SplashMode = PushStruct<game_mode_splash>(ModeArena); 
                InitSplashMode(GameState);
            } break;
            case GameModeType_Main: {
                GameState->MainMode = PushStruct<game_mode_main>(ModeArena); 
                InitMainMode(GameState);
            } break;
            case GameModeType_Menu: {
                GameState->MenuMode = PushStruct<game_mode_menu>(ModeArena); 
                InitMenuMode(GameState);
            } break;
            case GameModeType_Sandbox: {
                GameState->SandboxMode = PushStruct<game_mode_sandbox>(ModeArena); 
                InitSandboxMode(GameState);
            } break;
            default: {
            }
        }
        
        GameState->ModeType = GameState->NextModeType;
        GameState->NextModeType = GameModeType_None;
    }

    // State update
    switch(GameState->ModeType) {
        case GameModeType_Splash: {
            UpdateSplashMode(GameState, RenderCommands, Input, DeltaTime);
        } break;
        case GameModeType_Menu: {
            UpdateMenuMode(GameState, RenderCommands, Input, DeltaTime);
        } break;
        case GameModeType_Main: {
            UpdateMainMode(GameState, RenderCommands, Input, DeltaTime);
        } break; 
        case GameModeType_Sandbox: {
            UpdateSandboxMode(GameState, RenderCommands, Input, DeltaTime);
        } break;
        default: {
            Assert(false);
        }
    }

#if INTERNAL
    // Debug Rendering
    if (GameState->IsDebug) {
        Render(&GameState->DebugConsole, RenderCommands, &GameState->Assets);
    }
#endif
}
