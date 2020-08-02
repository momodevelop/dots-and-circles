#include "game_mode_splash.h"
#include "game_mode_sandbox.h"
#include "game_mode_main.h"

// NOTE(Momo):  Exported Functions
extern "C" void
GameUpdate(game_memory* GameMemory,  
           platform_api* Platform, 
           render_commands* RenderCommands, 
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
        
        game_assets* GameAssets = PushStruct<game_assets>(MainArena);
        GameState->Assets = GameAssets;
        Init(GameAssets, MainArena, Megabytes(10));
        
        // NOTE(Momo): Init Assets
        {
            auto TempMemory = BeginTemporaryMemory(MainArena);
            const char* Filepath = "assets/ryoji.bmp";
            u32 Filesize = Platform->GetFileSize(Filepath);
            Assert(Filesize);
            void* BitmapMemory = PushBlock(TempMemory.Arena, Filesize);
            Platform->ReadFile(BitmapMemory, Filesize, Filepath);
            LoadTexture(GameAssets, GameTextureType_ryoji, BitmapMemory);
            PushCommandLinkTexture(RenderCommands, GameAssets->Textures[GameTextureType_ryoji]);
            EndTemporaryMemory(TempMemory);
        }
        {
            auto TempMemory = BeginTemporaryMemory(MainArena);
            const char* Filepath = "assets/yuu.bmp";
            u32 Filesize = Platform->GetFileSize(Filepath);
            Assert(Filesize);
            void* BitmapMemory = PushBlock(TempMemory.Arena, Filesize);
            Platform->ReadFile(BitmapMemory, Filesize, Filepath);
            LoadTexture(GameAssets, GameTextureType_yuu, BitmapMemory);
            PushCommandLinkTexture(RenderCommands, GameAssets->Textures[GameTextureType_yuu]);
            EndTemporaryMemory(TempMemory);
        }
        {
            auto TempMemory = BeginTemporaryMemory(MainArena);
            const char* Filepath = "assets/blank.bmp";
            u32 Filesize = Platform->GetFileSize(Filepath);
            Assert(Filesize);
            void* BitmapMemory = PushBlock(TempMemory.Arena, Filesize);
            Platform->ReadFile(BitmapMemory, Filesize, Filepath);
            LoadTexture(GameAssets, GameTextureType_blank, BitmapMemory);
            PushCommandLinkTexture(RenderCommands, GameAssets->Textures[GameTextureType_blank]);
            EndTemporaryMemory(TempMemory);
        }
        {
            auto TempMemory = BeginTemporaryMemory(MainArena);
            const char* Filepath = "assets/karu.bmp";
            u32 Filesize = Platform->GetFileSize(Filepath);
            Assert(Filesize);
            void* BitmapMemory = PushBlock(TempMemory.Arena, Filesize);
            Platform->ReadFile(BitmapMemory, Filesize, Filepath);
            LoadTexture(GameAssets, GameTextureType_karu, BitmapMemory);
            PushCommandLinkTexture(RenderCommands, GameAssets->Textures[GameTextureType_karu]);
            EndTemporaryMemory(TempMemory);
        }
        
        
        // NOTE(Momo): Arena for modes
        SubArena(&GameState->ModeArena, &GameState->MainArena, 
                 GetRemainingCapacity(&GameState->MainArena));
        
        SetGameMode<game_mode_splash>(GameState);
        GameState->IsInitialized = true;
    }
    
    // NOTE(Momo): State machine
    switch(GameState->CurrentMode) {
        case game_mode_splash::TypeId: {
            UpdateMode(GameState->SplashMode, GameState, RenderCommands, Input, DeltaTime);
        } break;
        case game_mode_sandbox::TypeId: {
            UpdateMode(GameState->SandboxMode, GameState, RenderCommands, Input, DeltaTime);
        } break;
        case game_mode_main::TypeId: {
            UpdateMode(GameState->MainMode, GameState, RenderCommands, Input, DeltaTime);
        } break;
    }
}