#include "game.h"


// NOTE(Momo):  Exported Functions
extern "C" void
GameUpdate(platform_api * Platform, 
           game_memory* GameMemory,  
           render_commands* RenderCommands, 
           game_input* Input, 
           f32 DeltaTime)
{
    game* Game = (game*)GameMemory->Memory;
    
    // NOTE(Momo): Initialization of the game
    if(!GameMemory->IsInitialized) {
        Game->CurrentStateType = game_state_splash::TypeId;
        Game->IsStateInitialized = false;
        
        Game->MainArena = MakeMemoryArena((u8*)GameMemory->Memory + sizeof(game), GameMemory->MemorySize - sizeof(game));
        
        // NOTE(Momo): Init Assets
        Init(&Game->Assets, &Game->MainArena);
        {
            auto Result = Platform->ReadFile("assets/ryoji.bmp");
            Assert(Result.Content);
            LoadTexture(&Game->Assets, GameTextureType_ryoji, Result.Content);
            Platform->FreeFile(Result);
        }
        {
            auto Result = Platform->ReadFile("assets/yuu.bmp");
            Assert(Result.Content);
            LoadTexture(&Game->Assets, GameTextureType_yuu, Result.Content);
            Platform->FreeFile(Result);
        }
        {
            auto Result = Platform->ReadFile("assets/blank.bmp");
            Assert(Result.Content);
            LoadTexture(&Game->Assets, GameTextureType_blank, Result.Content);
            Platform->FreeFile(Result);
        }
        GameMemory->IsInitialized = true;
        gPlatform = Platform;
    }
    
    
    // NOTE(Momo): State machine
    switch(Game->CurrentStateType) {
        case game_state_splash::TypeId: {
            UpdateAndRenderSplashState(Game, Platform, RenderCommands, DeltaTime);
        } break;
        case game_state_main::TypeId: {
            Platform->Log("Main!");
        } break;
        
    }
}