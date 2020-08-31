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
    // TODO(Momo): Fix the fixed timestep
    if (DeltaTime > 1/30.f)
        DeltaTime = 1/30.f;
    
    
    game_state* GameState = (game_state*)GameMemory->MainMemory;
    // NOTE(Momo): Initialization of the game
    if(!GameState->IsInitialized) {
        
        // NOTE(Momo): Arenas
        memory_arena* MainArena = &GameState->MainArena;
        Init(MainArena, (u8*)GameMemory->MainMemory + sizeof(game_state), GameMemory->MainMemorySize - sizeof(game_state));
        
        game_assets* GameAssets = PushStruct<game_assets>(MainArena);
        GameState->Assets = GameAssets;
        Init(GameAssets, MainArena, Megabytes(10));
        
        {
            Log("Reading assets...");
            // TODO(Momo): 
            temporary_memory TempAssetMemory = BeginTemporaryMemory(MainArena);
            const char* Filepath = "yuu";
            u32 Filesize = Platform->GetFileSize(Filepath);
            Assert(Filesize);
            u8* AssetMemory = (u8*)PushBlock(TempAssetMemory.Arena, Filesize);
            u8* AssetMemoryItr = AssetMemory;
            
            Platform->ReadFile(AssetMemory, Filesize, Filepath);
            
            // NOTE(Momo): Check signature
            Assert(CheckAssetSignature(AssetMemoryItr));
            AssetMemoryItr += ArrayCount(AssetSignature);
            
            u32 AssetCount = ReadU32(&AssetMemoryItr, false);
            
            Log("Asset Count = %d", AssetCount);
            // NOTE(Momo): Read headers
            for (u32 i = 0; i < AssetCount; ++i)
            {
                asset_type Type = (asset_type)ReadU32(&AssetMemoryItr, false);
                u32 OffsetToData = ReadU32(&AssetMemoryItr, false);
                
                u8* DataItr = AssetMemory;
                DataItr += OffsetToData;
                
                // NOTE(Momo): Read Data
                switch(Type) {
                    case asset_type::Image: {
                        u32 Width = ReadU32(&DataItr, false);
                        u32 Height = ReadU32(&DataItr, false);
                        u32 Channels = ReadU32(&DataItr, false);
                        game_bitmap_handle Handle = (game_bitmap_handle)ReadU32(&DataItr, false);
                        
                        bitmap Bitmap = MakeEmptyBitmap(Width, Height, &GameAssets->Arena);
                        CopyBlock(Bitmap.Pixels, DataItr, Width * Height * Channels);
                        
                        Log("Image Read: %d %d", Width, Height);
                        
                        LoadBitmap(GameAssets, Handle, Bitmap);
                        PushCommandLinkTexture(RenderCommands,
                                               Bitmap, 
                                               Handle);
                    } break;
                    case asset_type::Font: {
                        // TODO(Momo): Implement
                    } break;
                    case asset_type::Spritesheet: {
                        temporary_memory SpritesheetArena = BeginTemporaryMemory(&TempAssetMemory);
                        
                        u32 Width = ReadU32(&DataItr, false);
                        u32 Height = ReadU32(&DataItr, false);
                        u32 Channels = ReadU32(&DataItr, false);
                        game_spritesheet_handle Handle = (game_spritesheet_handle)ReadU32(&DataItr, false);
                        u32 Rows = ReadU32(&DataItr, false);
                        u32 Cols = ReadU32(&DataItr, false);
                        
                        bitmap Bitmap = MakeEmptyBitmap(Width, Height, &GameAssets->Arena);
                        CopyBlock(Bitmap.Pixels, DataItr, Width * Height * Channels);
                        
                        Log("Spritesheet Read: %d %d %d %d %d", Width, Height, Rows, Cols);
                        
                        PushCommandLinkTexture(RenderCommands,
                                               Bitmap, 
                                               Handle);
                        
                        
                        
                        f32 FrameWidth = (f32)Bitmap.Width/Cols;
                        f32 FrameHeight = (f32)Bitmap.Height/Rows;
                        f32 CellWidth = FrameWidth/Width;
                        f32 CellHeight = FrameHeight/Height;
                        f32 HalfPixelWidth =  0.25f/Bitmap.Width;
                        f32 HalfPixelHeight = 0.25f/Bitmap.Height;
                        
                        // NOTE(Momo):  Init sprite frames
                        u32 TotalFrames = Rows * Cols;
                        rect2f Rects[12]; // TODO(Momo): Dynamic pls
                        for (u8 r = 0; r < Rows; ++r) {
                            for (u8 c = 0; c < Cols; ++c) {
                                Rects[TwoToOne(r,c,Cols)] = { 
                                    c * CellWidth + HalfPixelWidth,
                                    r * CellHeight + HalfPixelHeight,
                                    (c+1) * CellWidth - HalfPixelWidth,
                                    (r+1) * CellHeight - HalfPixelHeight,
                                };
                            }
                        }
                        LoadSpritesheet(GameAssets, GameSpritesheetHandle_Karu, Bitmap, Rects, TotalFrames);
                        
                        EndTemporaryMemory(SpritesheetArena);
                        
                    } break;
                    case asset_type::Animation: {
                        // TODO(Momo): Implement 
                    } break;
                    case asset_type::Sound: {
                        // TODO(Momo): Implement
                    } break;
                }
                EndTemporaryMemory(TempAssetMemory);
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