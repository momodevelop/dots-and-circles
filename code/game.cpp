#include "game.h"
#include "game_assets.h"
#include "game_input.h"
#include "game_mode_splash.h"
#include "game_mode_menu.h"
#include "game_mode_main.h"
#include "game_mode_atlas_test.h"
#include "game_text.h"
#include "mm_arena.h"
#include "mm_maths.h"
#include "mm_colors.h"
#include "mm_link_list.h"
// cmd: jump main/menu/atlas_test/etc...
static inline void 
CmdJump(void * Context, string Arguments) {
    game_state* GameState = (game_state*)Context;
    
    auto Scratch = BeginScratch(&GameState->DebugArena);
    Defer {  EndScratch(&Scratch); };

    dlink_list<string> ArgList = DelimitSplit(Arguments, Scratch, ' ');
    if ( ArgList.Length != 2 ) {
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
    else if (StateToChangeTo == String("atlas_test")) {
        PushDebugInfo(&GameState->DebugConsole, String("Jumping to Atlas Test"), ColorYellow);
        GameState->NextModeType = GameModeType_AtlasTest;
    }
    else {
        PushDebugInfo(&GameState->DebugConsole, String("Invalid state to jump to"), ColorRed);
    }
    
    

}


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
        GameState->MainArena = Arena((u8*)GameMemory->MainMemory + sizeof(game_state), GameMemory->MainMemorySize - sizeof(game_state));

        // NOTE(Momo): Assets
        GameState->Assets = CreateAssets(
                &GameState->MainArena, 
                Platform, 
                RenderCommands, 
                String("yuu\0")
        );
   
        // NOTE(Momo): Arena for modes
        GameState->ModeArena = SubArena(&GameState->MainArena, Remaining(GameState->MainArena));
        GameState->ModeType = GameModeType_None;
        GameState->NextModeType = GameModeType_Splash;
        GameState->IsInitialized = true;

        // NOTE(Momo): Set design resolution for game
        PushCommandSetDesignResolution(RenderCommands, 1600, 900);

#if INTERNAL

        GameState->DebugArena = Arena(GameMemory->DebugMemory, GameMemory->DebugMemorySize);
        
        // Debug Console Init
        {
            m44f Basis = Orthographic(-1.f, 1.f, -1.f, 1.f, -1.f, 1.f, 
                        0.f, 1600.f, 0.f, 900.f, -1000.f, 1000.f, true);
            m44f InfoBgTransform = 
                Translation(0.5f, 0.5f, 0.f) *
                Scale(1600.f, 240.f, 1.f) *
                Translation(0.f, 0.f, 500.f);
 

            m44f InputBgTransform =  
                Translation(0.5f, 0.5f, 0.f) * 
                Scale(1600.f, 40.f, 1.f) *
                Translation(0.f, 0.f, 501.f);
 
            v4f InfoBgColor = { 0.3f, 0.3f, 0.3f, 1.f };
            v4f InputBgColor = { 0.2f, 0.2f, 0.2f, 1.f };
            

            GameState->DebugConsole = CreateDebugConsole(
                    &GameState->DebugArena, 
                    5, 110, 32, 
                    Basis,
                    InfoBgTransform,
                    InputBgTransform,
                    InfoBgColor,
                    InputBgColor);
            debug_console* Console = &GameState->DebugConsole;
        
        }


        // Temp, set some simple callbacks to debug callbacks
        Register(&GameState->DebugConsole, String("jump"), CmdJump, GameState);
    
#endif
    }

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
        auto Scratch = BeginScratch(&GameState->DebugArena);
        Defer { EndScratch(&Scratch); };
        string_buffer TempBuffer = StringBuffer(Scratch.Arena, 32);
        Itoa(&TempBuffer, (i32)TicksElapsed);
        Push(&TempBuffer, String("ms"));
        NullTerm(&TempBuffer);

        DrawText(RenderCommands, 
                &GameState->Assets, 
                { 10.f, 880.f, 700.f }, 
                ColorWhite,
                Font_Default, 
                32.f, 
                TempBuffer.Array);  
    }

    // Clean state/Switch states
    if (GameState->NextModeType != GameModeType_None) {
        Clear(&GameState->ModeArena);
        arena* ModeArena = &GameState->ModeArena;
        switch(GameState->NextModeType) {
            case GameModeType_Splash: {
                GameState->SplashMode = PushStruct<game_mode_splash>(ModeArena); 
                Init(GameState->SplashMode, GameState);
            } break;
            case GameModeType_Main: {
                GameState->MainMode = PushStruct<game_mode_main>(ModeArena); 
                Init(GameState->MainMode, GameState);
            } break;
            case GameModeType_Menu: {
                GameState->MenuMode = PushStruct<game_mode_menu>(ModeArena); 
                Init(GameState->MenuMode, GameState);
            } break;
            case GameModeType_AtlasTest: {
                GameState->AtlasTestMode = PushStruct<game_mode_atlas_test>(ModeArena); 
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
