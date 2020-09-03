#include "ryoji_bitmanip.h"
#include "game_mode_splash.h"
#include "game_mode_main.h"
#include "game_mode_menu.h"

// NOTE(Momo):  Exported Functions
extern "C" void
GameUpdate(game_memory* GameMemory,  
           platform_api* Platform, 
           commands* RenderCommands, 
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
        memory_arena* MainArena = &GameState->MainArena;
        Init(MainArena, (u8*)GameMemory->MainMemory + sizeof(game_state), GameMemory->MainMemorySize - sizeof(game_state));
        
        // NOTE(Momo): Assets
        game_assets* GameAssets = PushStruct<game_assets>(MainArena);
        GameState->Assets = GameAssets;
        Init(GameAssets, MainArena, Platform, RenderCommands, "yuu");
        
        
        // NOTE(Momo): Arena for modes
        SubArena(&GameState->ModeArena, &GameState->MainArena, 
                 GetRemainingCapacity(&GameState->MainArena));
        SetGameMode<game_mode_splash>(GameState);
        GameState->IsInitialized = true;
        
        // NOTE(Momo): Set design resolution for game
        PushCommandSetDesignResolution(RenderCommands, 1600, 900);
        
    }
    
    // NOTE(Momo): State machine
    switch(GameState->CurrentMode) {
        case game_mode_splash::TypeId: {
            UpdateMode(GameState->SplashMode, GameState, RenderCommands, Input, DeltaTime);
        } break;
        case game_mode_menu::TypeId: {
            UpdateMode(GameState->MenuMode, GameState, RenderCommands, Input, DeltaTime);
        } break;
        case game_mode_main::TypeId: {
            UpdateMode(GameState->MainMode, GameState, RenderCommands, Input, DeltaTime);
        } break;
    }
}