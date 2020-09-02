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
        
        game_assets* GameAssets = PushStruct<game_assets>(MainArena);
        GameState->Assets = GameAssets;
        Init(GameAssets, MainArena);
        {
            Log("Reading assets...");
            temp_memory_arena TempAssetMemory = BeginTempArena(MainArena);
            Defer{ EndTempArena(&TempAssetMemory); };
            
            const char* Filepath = "yuu";
            u32 Filesize = Platform->GetFileSize(Filepath);
            Assert(Filesize);
            u8* AssetMemory = (u8*)PushBlock(TempAssetMemory.Arena, Filesize);
            u8* AssetMemoryItr = AssetMemory;
            Platform->ReadFile(AssetMemory, Filesize, Filepath);
            
            Assert(CheckAssetSignature(AssetMemoryItr));
            AssetMemoryItr += ArrayCount(AssetSignature);
            
            u32 AssetCount = Read32<u32>(&AssetMemoryItr, false);
            
            Log("Asset Count = %d", AssetCount);
            
            for (u32 i = 0; i < AssetCount; ++i)
            {
                // NOTE(Momo): Read headers
                asset_type Type = Read32<asset_type>(&AssetMemoryItr, false);
                u32 OffsetToData = Read32<u32>(&AssetMemoryItr, false);
                asset_id AssetId = Read32<asset_id>(&AssetMemoryItr, false);
                
                u8* DataItr = AssetMemory + OffsetToData;
                
                // NOTE(Momo): Read Data
                switch(Type) {
                    case asset_type::Image: {
                        u32 Width = Read32<u32>(&DataItr, false);
                        u32 Height = Read32<u32>(&DataItr, false);
                        u32 Channels = Read32<u32>(&DataItr, false);
                        auto* Image = LoadImage(GameAssets, 
                                                AssetId, 
                                                Width, 
                                                Height, 
                                                Channels, 
                                                DataItr);
                        
                        PushCommandLinkTexture(RenderCommands,
                                               Image, 
                                               (u32)AssetId);
                        
                        Log("Image Loaded: %d %d", Width, Height);
                    } break;
                    case asset_type::Font: {
                        // TODO(Momo): Implement
                    } break;
                    case asset_type::Spritesheet: {
                        //temp_memory_arena SpritesheetArena = BeginTempArena(&TempAssetMemory);
                        u32 Width = Read32<u32>(&DataItr, false);
                        u32 Height = Read32<u32>(&DataItr, false);
                        u32 Channels = Read32<u32>(&DataItr, false);
                        u32 Rows = Read32<u32>(&DataItr, false);
                        u32 Cols = Read32<u32>(&DataItr, false);
                        
                        auto* Spritesheet = LoadSpritesheet(GameAssets,
                                                            AssetId,
                                                            Width,
                                                            Height,
                                                            Channels,
                                                            Rows,
                                                            Cols,
                                                            DataItr);
                        
                        PushCommandLinkTexture(RenderCommands,
                                               Spritesheet, 
                                               (u32)AssetId);
                        
                        //EndTempArena(SpritesheetArena);
                        
                    } break;
                    case asset_type::Animation: {
                        // TODO(Momo): Implement 
                    } break;
                    case asset_type::Sound: {
                        // TODO(Momo): Implement
                    } break;
                }
                
                
                
                
            }
            
            
        }
        
        
        
        // NOTE(Momo): Arena for modes
        SubArena(&GameState->ModeArena, &GameState->MainArena, 
                 GetRemainingCapacity(&GameState->MainArena));
        SetGameMode<game_mode_splash>(GameState);
        GameState->IsInitialized = true;
        
        // NOTE(Momo): Set design resolution for game
        {
            PushCommandSetDesignResolution(RenderCommands, 1600, 900);
        }
        
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