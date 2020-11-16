#include "game.h"
#include "game_assets.h"
#include "game_input.h"
#include "game_mode_splash.h"
#include "game_mode_menu.h"
#include "game_mode_main.h"
#include "game_mode_atlas_test.h"
#include "game_text.h"
#include "mm_maths.h"
#include "mm_colors.h"

extern "C" void
GameUpdate(game_memory* GameMemory,  
           platform_api* Platform, 
           mmcmd_commands* RenderCommands, 
           game_input* Input, 
           f32 DeltaTime,
           u64 TicksElapsed)
{
    game_state* GameState = (game_state*)GameMemory->MainMemory;
       // NOTE(Momo): Initialization of the game
    if(!GameState->IsInitialized) {
       // NOTE(Momo): Arenas
        GameState->MainArena = mmarn_Arena((u8*)GameMemory->MainMemory + sizeof(game_state), GameMemory->MainMemorySize - sizeof(game_state));

        // NOTE(Momo): Assets
        GameState->Assets = CreateAssets(
                &GameState->MainArena, 
                Platform, 
                RenderCommands, 
                "yuu"
        );
   
        // NOTE(Momo): Arena for modes
        GameState->ModeArena = mmarn_SubArena(&GameState->MainArena, mmarn_Remaining(&GameState->MainArena));
        GameState->ModeType = GameModeType_None;
        GameState->NextModeType = GameModeType_Splash;
        GameState->IsInitialized = true;

        // NOTE(Momo): Set design resolution for game
        PushCommandSetDesignResolution(RenderCommands, 1600, 900);

#if INTERNAL

        GameState->DebugArena = mmarn_Arena(GameMemory->DebugMemory, GameMemory->DebugMemorySize);
        GameState->DebugConsole = CreateDebugConsole(&GameState->DebugArena);

        // Temp, set some simple callbacks to debug callbacks
        Register(&GameState->DebugConsole, 
            mms_ConstString("jump"), 
            [](void* Context) {
                game_state* GameState = (game_state*)Context;
                PushDebugInfo(&GameState->DebugConsole, mms_ConstString("Jumping to game!"), mmc_ColorYellow);
                GameState->NextModeType = GameModeType_Main;
            }, GameState);
    
#endif
    }

    // Remove this?
#if INTERNAL
    gLog = Platform->Log;
#endif


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
            Update(&GameState->DebugConsole, Input);
            Render(&GameState->DebugConsole, RenderCommands, &GameState->Assets);  
        }


    }

    if (GameState->IsShowTicksElapsed) {
        auto Scratch = mmarn_BeginScratch(&GameState->DebugArena);
        Defer { mmarn_EndScratch(&Scratch); };
        mms_string TempBuffer = mms_String(Scratch.Arena, 32);
        mms_Itoa(&TempBuffer, (i32)TicksElapsed);
        mms_Concat(&TempBuffer, mms_ConstString("ms"));
        mms_NullTerm(&TempBuffer);

        DrawText(RenderCommands, 
                &GameState->Assets, 
                { 10.f, 880.f, 700.f }, 
                mmc_ColorWhite,
                Font_Default, 
                32.f, 
                TempBuffer.String);  
    }

    // Clean state/Switch states
    if (GameState->NextModeType != GameModeType_None) {
        mmarn_Clear(&GameState->ModeArena);
        mmarn_arena* ModeArena = &GameState->ModeArena;
        switch(GameState->NextModeType) {
            case GameModeType_Splash: {
                GameState->SplashMode = mmarn_PushStruct<game_mode_splash>(ModeArena); 
                Init(GameState->SplashMode, GameState);
            } break;
            case GameModeType_Main: {
                GameState->MainMode = mmarn_PushStruct<game_mode_main>(ModeArena); 
                Init(GameState->MainMode, GameState);
            } break;
            case GameModeType_Menu: {
                GameState->MenuMode = mmarn_PushStruct<game_mode_menu>(ModeArena); 
                Init(GameState->MenuMode, GameState);
            } break;
            case GameModeType_AtlasTest: {
                GameState->AtlasTestMode = mmarn_PushStruct<game_mode_atlas_test>(ModeArena); 
                Init(GameState->AtlasTestMode, GameState);
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
            Update(GameState->SplashMode, GameState, RenderCommands, Input, DeltaTime);
        } break;
        case GameModeType_Menu: {
            Update(GameState->MenuMode, GameState, RenderCommands, Input, DeltaTime);
        } break;
        case GameModeType_Main: {
            Update(GameState->MainMode, GameState, RenderCommands, Input, DeltaTime);
        } break; 
        case GameModeType_AtlasTest: {
            Update(GameState->AtlasTestMode, GameState, RenderCommands, Input, DeltaTime);
        } break;
        default: {
            Assert(false);
        }
    }




}
