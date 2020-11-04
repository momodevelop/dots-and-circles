#include "game.h"
#include "game_assets.h"
#include "game_mode_splash.h"
#include "game_mode_menu.h"
#include "game_mode_main.h"
#include "game_mode_atlas_test.h"
#include "game_text.h"
#include "mm_maths.h"

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
#if INTERNAL
        gLog = Platform->Log;
#endif
        // NOTE(Momo): Arenas
        GameState->MainArena = mmarn_CreateArena((u8*)GameMemory->MainMemory + sizeof(game_state), GameMemory->MainMemorySize - sizeof(game_state));
        mmarn_arena* MainArena = &GameState->MainArena;

        // NOTE(Momo): Assets
        game_assets* GameAssets = mmarn_PushStruct<game_assets>(MainArena);
        GameState->Assets = GameAssets;
        Init(GameAssets, MainArena, Platform, RenderCommands, "yuu");
        
        // NOTE(Momo): Arena for modes
        GameState->ModeArena = mmarn_PushArenaAll(MainArena);
        GameState->ModeType = GameModeType_None;
        GameState->NextModeType = GameModeType_Splash;
        GameState->IsInitialized = true;
        
        // NOTE(Momo): Set design resolution for game
        PushCommandSetDesignResolution(RenderCommands, 1600, 900);
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




#if INTERNAL
    // System Debug
    {
        PushCommandSetBasis(RenderCommands, mmm_Orthographic(-1.f, 1.f, -1.f, 1.f, -1.f, 1.f, 
                0.f, 1600.f, 0.f, 900.f, -100.f, 100.f, true));
        {
            mmm_m44f A = mmm_Translation(0.5f, 0.5f, 0.f);
            mmm_m44f S = mmm_Scale(1600.f, 100.f, 1.f);
            mmm_m44f T = mmm_Translation(0.f, 0.f, 1.f);
            PushCommandDrawQuad(RenderCommands, {0.3f, 0.3f, 0.3f, 1.f}, T*S*A);
        }
        // Black background
        char buffer[128];
        Itoa(buffer, (i32)TicksElapsed);
        StrConcat(buffer, "ms");
        DrawText(RenderCommands, GameState->Assets, {}, Font_Default, 32.f, buffer);  
    }
#endif
}
