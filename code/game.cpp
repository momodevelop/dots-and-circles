#include "game.h"


static inline void
UpdateSplashState(game* Game, 
                  platform_api* Platform, 
                  render_commands* RenderCommands, 
                  f32 DeltaTime) {
    game_state_splash* State = &Game->GameState.Splash; 
    if(!Game->IsStateInitialized) {
        // NOTE(Momo): Create entities
        {
            State->SplashImg[0].Position = { 0.f, 0.f, 0.f };
            State->SplashImg[0].Scale = { 200.f, 200.f };
            State->SplashImg[0].Colors = { 1.f, 1.f, 1.f, 1.f };
            State->SplashImg[0].TextureHandle = GameTextureType_ryoji;
            State->SplashImg[0].CountdownTimer = 0.f;
            State->SplashImg[0].CountdownDuration = 3.f;
            State->SplashImg[0].Timer = 0.f;
            State->SplashImg[0].Duration = 2.f;
            State->SplashImg[0].StartX = -1000.f;
            State->SplashImg[0].EndX = -200.f;
            
            State->SplashImg[1].Position = { 0.f };
            State->SplashImg[1].Scale = { 200.f, 200.f };
            State->SplashImg[1].Colors = { 1.f, 1.f, 1.f, 1.f };
            State->SplashImg[1].TextureHandle = GameTextureType_yuu;
            State->SplashImg[1].CountdownTimer = 0.f;
            State->SplashImg[1].CountdownDuration = 3.f;
            State->SplashImg[1].Timer = 0.f;
            State->SplashImg[1].Duration = 2.f;
            State->SplashImg[1].StartX = 1000.f;
            State->SplashImg[1].EndX = 200.f;
            
            State->SplashBlackout.Position = { 0.f, 900.f, 1.0f };
            State->SplashBlackout.Scale = { 800.f, 450.f };
            State->SplashBlackout.Colors = { 0.f, 0.f, 0.f, 0.f };
            State->SplashBlackout.TextureHandle = GameTextureType_blank;
            State->SplashBlackout.CountdownTimer = 0.f;
            State->SplashBlackout.CountdownDuration = 5.f;
            State->SplashBlackout.Timer = 0.f;
            State->SplashBlackout.Duration = 1.f;
            State->SplashBlackout.StartY = 900.f;
            State->SplashBlackout.EndY = 0.f;
        }
        Game->IsStateInitialized = true;
        Platform->Log("Splash state initialized!");
    }
    
    PushCommandClear(RenderCommands, { 0.0f, 0.3f, 0.3f, 0.f });
    
    for (u32 i = 0; i < 2; ++i) {
        Update(&State->SplashImg[i], 
               &Game->Assets, 
               RenderCommands, 
               DeltaTime);
    }
    Update(&State->SplashBlackout,
           &Game->Assets,
           RenderCommands,
           DeltaTime);
}




// NOTE(Momo):  Exported Functions
extern "C" void
GameUpdate(platform_api* Platform, 
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