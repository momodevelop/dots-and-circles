#include "game.h"
#include "game_assets.h"
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
    
    auto Scratch = BeginScratch(&GameState->MainArena);
    Defer {  EndScratch(&Scratch); };

    dlink_list<string> ArgList = DelimitSplit(Arguments, Scratch, ' ');
    if ( ArgList.Count != 2 ) {
        // Expect two arguments
        PushDebugInfo(&GameState->Console, String("Expected only 2 arguments"), ColorRed);
        return;
    }

    string StateToChangeTo = ArgList[1];
    if (StateToChangeTo == String("main")) {
        PushDebugInfo(&GameState->Console, String("Jumping to Main"), ColorYellow);
        GameState->NextModeType = GameModeType_Main;
    }
    else if (StateToChangeTo == String("splash")) {
        PushDebugInfo(&GameState->Console, String("Jumping to Splash"), ColorYellow);
        GameState->NextModeType = GameModeType_Splash;
    }
    else if (StateToChangeTo == String("menu")) {
        PushDebugInfo(&GameState->Console, String("Jumping to Menu"), ColorYellow);
        GameState->NextModeType = GameModeType_Menu;
    }
    else if (StateToChangeTo == String("sandbox")) {
        PushDebugInfo(&GameState->Console, String("Jumping to Sandbox"), ColorYellow);
        GameState->NextModeType = GameModeType_Sandbox;
    }
    else {
        PushDebugInfo(&GameState->Console, String("Invalid state to jump to"), ColorRed);
    }
}
#endif

extern "C" 
GameUpdateFunc(GameUpdate) 
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
                                       (u32)Global_DesignWidth, 
                                       (u32)Global_DesignHeight);

        GameState->ConsoleCommands = List<game_console_command>(&GameState->MainArena, 1);
        // Console Init
        {
            game_console Console = GameConsole(&GameState->MainArena, 5, 110, 32);
            Console.InfoBgColor = { 0.3f, 0.3f, 0.3f, 1.f };
            Console.InfoDefaultColor = ColorWhite;
            Console.InputBgColor = { 0.2f, 0.2f, 0.2f, 1.f };
            Console.InputColor = ColorWhite;
            Console.Dimensions = { Global_DesignWidth, 240.f };
            Console.Position = { 
                -Global_DesignWidth * 0.5f + Console.Dimensions.W * 0.5f, 
                -Global_DesignHeight * 0.5f + Console.Dimensions.H * 0.5f, 
                Global_DesignDepth * 0.5f - 1.f
            };

            GameState->Console = Console;
        
        }
        Register(&GameState->ConsoleCommands, String("jump"), CmdJump, GameState);
    }



    // Console
    {
        if (IsPoked(Input->ButtonConsole)) {
            GameState->IsConsole = !GameState->IsConsole; 

            // TODO: Seperate button or maybe settings?
            GameState->IsShowTicksElapsed = !GameState->IsShowTicksElapsed;
        }
        if (GameState->IsConsole) {
            game_console* Console = &GameState->Console;
            if (Update(Console, Input)){ 
                Execute(&GameState->ConsoleCommands, GetCommandString(Console));
            }
        }

    }



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

    // Render Console
    if (GameState->IsConsole) {
        Render(&GameState->Console, RenderCommands, &GameState->Assets);
    }
}
