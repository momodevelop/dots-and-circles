#include "game.h"
#include "game_assets.h"
#include "game_mode_splash.h"
#include "game_mode_menu.h"
#include "game_mode_main.h"
#include "game_mode_atlas_test.h"


#if INTERNAL
static  b32
ProcessMetaInput(game_state* GameState, game_input* Input) {
    if (IsPoked(Input->ButtonDebug[1])) {
        SetGameMode(GameState, GameModeType_Splash);
        Log("Jumping to splash state");
        return true;
    }
    else if (IsPoked(Input->ButtonDebug[2])) {
        SetGameMode(GameState, GameModeType_Menu);
        Log("Jumping to menu state");
        return true;
    }
    else if (IsPoked(Input->ButtonDebug[9])) {
        SetGameMode(GameState, GameModeType_AtlasTest);
        Log("Jumping to atlas test state");
        return true;
    }
    
    else if (IsPoked(Input->ButtonDebug[0])) {
        SetGameMode(GameState, GameModeType_Main);
        Log("Jumping to main state");
        return true;
    }
    
    return false;
}
#endif

static void
SetGameMode(game_state* GameState, game_mode_type ModeType) {
    mmarn_Clear(&GameState->ModeArena);
    mmarn_arena* ModeArena = &GameState->ModeArena;
    switch(ModeType) {
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
    }
    
    GameState->ModeType = ModeType;
}




// NOTE(Momo):  Exported Functions
extern "C" void
GameUpdate(game_memory* GameMemory,  
           platform_api* Platform, 
           mmcmd_commands* RenderCommands, 
           game_input* Input, 
           f32 DeltaTime)
{
#if INTERNAL
    gLog = Platform->Log;
#endif
    game_state* GameState = (game_state*)GameMemory->MainMemory;
    // NOTE(Momo): Initialization of the game
    if(!GameState->IsInitialized) {
        // NOTE(Momo): Arenas
        GameState->MainArena = mmarn_CreateArena((u8*)GameMemory->MainMemory + sizeof(game_state), GameMemory->MainMemorySize - sizeof(game_state));
        mmarn_arena* MainArena = &GameState->MainArena;

        // NOTE(Momo): Assets
        game_assets* GameAssets = mmarn_PushStruct<game_assets>(MainArena);
        GameState->Assets = GameAssets;
        Init(GameAssets, MainArena, Platform, RenderCommands, "yuu");
        
        // NOTE(Momo): Arena for modes
        GameState->ModeArena = mmarn_PushArena(MainArena, mmarn_Remaining(GameState->MainArena));
        SetGameMode(GameState, GameModeType_Splash);
        GameState->IsInitialized = true;
        
        // NOTE(Momo): Set design resolution for game
        PushCommandSetDesignResolution(RenderCommands, 1600, 900);
    }
    
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
    }
}