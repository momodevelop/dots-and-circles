#include "game.h"

static inline void
UpdateSandboxState(game* Game, 
                   render_command_queue* RenderCommands, 
                   f32 DeltaTime) {
    
    game_state_sandbox* State = &Game->GameState.Sandbox; 
    if(!Game->IsStateInitialized) {
        // NOTE(Momo): Create entities
        
        f32 offsetX = -700.f;
        f32 offsetY = -200.f;
        f32 offsetDeltaX = 5.f;
        f32 offsetDeltaY = 5.f;
        for (u32 i = 0; i < game_state_sandbox::TotalEntities; ++i)
        {
            State->Entities[i].Position = { offsetX, offsetY, 0.f };
            State->Entities[i].Rotation = 0.f;
            State->Entities[i].Scale = { 5.f, 5.f };
            State->Entities[i].Colors = { 1.f, 1.f, 1.f, 0.5f };
#if 0
            if ( i < (game_state_sandbox::TotalEntities / 3))
                State->Entities[i].TextureHandle = 1;
            else if ( i < (game_state_sandbox::TotalEntities / 3 * 2))
                State->Entities[i].TextureHandle = 2;
            else if ( i < game_state_sandbox::TotalEntities)
                State->Entities[i].TextureHandle = 0;
#else
            State->Entities[i].TextureHandle = i % 3;
#endif
            offsetX += offsetDeltaX;
            if (offsetX >= 700.f) {
                offsetX = -700.f;
                offsetY += offsetDeltaY;
            }
            
        }
        Game->IsStateInitialized = true;
        Log("Sandbox state initialized!");
    }
    
    PushCommandClear(RenderCommands, { 0.0f, 0.0f, 0.0f, 0.f });
    
    for (u32 i = 0; i < game_state_sandbox::TotalEntities; ++i) {
        Update(&State->Entities[i], 
               &Game->Assets, 
               RenderCommands, 
               DeltaTime);
    }
}





static inline void
UpdateSplashState(game* Game, 
                  render_command_queue* RenderCommands, 
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
            State->SplashImg[0].CountdownDuration = 5.f;
            State->SplashImg[0].Timer = 0.f;
            State->SplashImg[0].Duration = 2.f;
            State->SplashImg[0].StartX = -1000.f;
            State->SplashImg[0].EndX = -200.f;
            
            State->SplashImg[1].Position = { 0.f };
            State->SplashImg[1].Scale = { 200.f, 200.f };
            State->SplashImg[1].Colors = { 1.f, 1.f, 1.f, 1.f };
            State->SplashImg[1].TextureHandle = GameTextureType_yuu;
            State->SplashImg[1].CountdownTimer = 0.f;
            State->SplashImg[1].CountdownDuration = 5.f;
            State->SplashImg[1].Timer = 0.f;
            State->SplashImg[1].Duration = 2.f;
            State->SplashImg[1].StartX = 1000.f;
            State->SplashImg[1].EndX = 200.f;
            
            State->SplashBlackout.Position = { 0.f, 0.f, 1.0f };
            State->SplashBlackout.Scale = { 800.f, 450.f };
            State->SplashBlackout.Colors = { 0.f, 0.f, 0.f, 0.0f };
            State->SplashBlackout.TextureHandle = GameTextureType_blank;
            State->SplashBlackout.CountdownTimer = 0.f;
            State->SplashBlackout.CountdownDuration = 8.f;
            State->SplashBlackout.Timer = 0.f;
            State->SplashBlackout.Duration = 1.f;
        }
        Game->IsStateInitialized = true;
        Log("Splash state initialized!");
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
           render_command_queue* RenderCommands, 
           game_input* Input, 
           f32 DeltaTime)
{
    game* Game = (game*)GameMemory->Memory;
    
    // NOTE(Momo): Initialization of the game
    if(!GameMemory->IsInitialized) {
        Game->CurrentStateType = game_state_splash::TypeId;
        Game->IsStateInitialized = false;
        
        Init(&Game->MainArena,(u8*)GameMemory->Memory + sizeof(game), GameMemory->MemorySize - sizeof(game));
        
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
#if INTERNAL
        gLog = Platform->Log;
#endif
    }
    
    // NOTE(Momo): State machine
    switch(Game->CurrentStateType) {
        case game_state_splash::TypeId: {
            UpdateSplashState(Game, RenderCommands, DeltaTime);
        } break;
        case game_state_sandbox::TypeId: {
            UpdateSandboxState(Game, RenderCommands, DeltaTime);
        } break;
        case game_state_main::TypeId: {
            Log("Main!");
        } break;
    }
}