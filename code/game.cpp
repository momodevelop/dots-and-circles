#include "game.h"
#include "game_assets.h"
#include "game_input.h"
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

        // NOTE(Momo): Assets
        game_assets* GameAssets = mmarn_PushStruct<game_assets>(&GameState->MainArena);
        GameState->Assets = GameAssets;
        Init(GameAssets, &GameState->MainArena, Platform, RenderCommands, "yuu");
        
        // NOTE(Momo): Arena for modes
        GameState->ModeArena = mmarn_PushArenaAll(&GameState->MainArena);
        GameState->ModeType = GameModeType_None;
        GameState->NextModeType = GameModeType_Splash;
        GameState->IsInitialized = true;


        // NOTE(Momo): Set design resolution for game
        PushCommandSetDesignResolution(RenderCommands, 1600, 900);
        
#if INTERNAL
        
        GameState->DebugArena = mmarn_CreateArena(GameMemory->DebugMemory, GameMemory->DebugMemorySize);
        GameState->DebugInputBuffer = mms_CreateString(mmarn_PushArray<char>(&GameState->DebugArena, 110), 110);
        for (u32 i = 0; i < ArrayCount(GameState->DebugInfoBuffer); ++i) {
            char* Buffer = mmarn_PushArray<char>(&GameState->DebugArena, 110);
            GameState->DebugInfoBuffer[i] = mms_CreateString(Buffer, 110);
        }
#endif
    }


    // System Debug
    {
        // F1 to toggle debug console
        if (IsPoked(Input->DebugKeys[GameDebugKey_F1])) {
            GameState->IsDebug = !GameState->IsDebug; 
            if( GameState->IsDebug ) {
                // Init the console
                mms_Clear(&GameState->DebugInputBuffer);
            }
        }

        if (IsPoked(Input->DebugKeys[GameDebugKey_F2])) {
            GameState->IsShowTicksElapsed = !GameState->IsShowTicksElapsed;
        }


        if (GameState->IsDebug) {
            PushCommandSetBasis(RenderCommands, 
                    mmm_Orthographic(-1.f, 1.f, -1.f, 1.f, -1.f, 1.f, 
                                      0.f, 1600.f, 0.f, 900.f, -1000.f, 1000.f, true));
            // Background
            {
                mmm_m44f A = mmm_Translation(0.5f, 0.5f, 0.f);
                mmm_m44f S = mmm_Scale(1600.f, 240.f, 1.f);
                mmm_m44f T = mmm_Translation(0.f, 0.f, 500.f);
                PushCommandDrawQuad(RenderCommands, { 0.3f, 0.3f, 0.3f, 1.f }, T*S*A);
                
                S = mmm_Scale(1600.f, 40.f, 1.f);
                T = mmm_Translation(0.f, 0.f, 501.f);
                PushCommandDrawQuad(RenderCommands, { 0.2f, 0.2f, 0.2f, 1.f }, T*S*A);
            }


            if (Input->DebugTextInputBuffer.Length > 0) {
                mms_Concat(&GameState->DebugInputBuffer, &Input->DebugTextInputBuffer);
            }
            
            // Remove character
            if (IsPoked(Input->DebugKeys[GameDebugKey_Backspace])) {
                // TODO: God i need my own string object...
                mms_Pop(&GameState->DebugInputBuffer);
            }

            if (IsPoked(Input->DebugKeys[GameDebugKey_Return])) {
                for(i32 i = ArrayCount(GameState->DebugInfoBuffer) - 2; i >= 0 ; --i) {
                    mms_Copy(&GameState->DebugInfoBuffer[i+1], &GameState->DebugInfoBuffer[i]);
                }

                // Feed the text into info window
                mms_Clear(&GameState->DebugInfoBuffer[0]);
                mms_Copy(&GameState->DebugInfoBuffer[0], &GameState->DebugInputBuffer);
                mms_Clear(&GameState->DebugInputBuffer);
            }
             
            // Draw text
            {
                for (u32 i = 0; i < ArrayCount(GameState->DebugInfoBuffer); ++i ) {
                    DrawText(RenderCommands,
                            GameState->Assets,
                            { 10.f, 50.f + i * 40.f, 502.f },
                            Font_Default, 32.f,
                            GameState->DebugInfoBuffer[i]);
                }

                DrawText(RenderCommands, 
                    GameState->Assets, 
                    { 10.f, 10.f, 600.f }, 
                    Font_Default, 32.f, 
                    GameState->DebugInputBuffer);
                


            }
        }


        if (GameState->IsShowTicksElapsed) {
            char buffer[128];
            Itoa(buffer, (i32)TicksElapsed);
            StrConcat(buffer, "ms");
            DrawText(RenderCommands, GameState->Assets, { 0.f, 900.f, 501.f }, Font_Default, 32.f, buffer);  
        }

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
