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
    // NOTE(Momo): Delta time cap
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
        
        // TODO(Momo): All this should really be in game_assets.h
        // NOTE(Momo): Init Bitmap Assets
        {// ryoji
            auto TempMemory = BeginTemporaryMemory(MainArena);
            const char* Filepath = "assets/ryoji.bmp";
            u32 Filesize = Platform->GetFileSize(Filepath);
            Assert(Filesize);
            void* BitmapMemory = PushBlock(TempMemory.Arena, Filesize);
            Platform->ReadFile(BitmapMemory, Filesize, Filepath);
            LoadBitmap(GameAssets, GameBitmapHandle_Ryoji, BitmapMemory);
            PushCommandLinkTexture(RenderCommands,
                                   GetBitmap(GameAssets, GameBitmapHandle_Ryoji),
                                   GameBitmapHandle_Ryoji);
            EndTemporaryMemory(TempMemory);
        }
        {// yuu
            auto TempMemory = BeginTemporaryMemory(MainArena);
            const char* Filepath = "assets/yuu.bmp";
            u32 Filesize = Platform->GetFileSize(Filepath);
            Assert(Filesize);
            void* BitmapMemory = PushBlock(TempMemory.Arena, Filesize);
            Platform->ReadFile(BitmapMemory, Filesize, Filepath);
            LoadBitmap(GameAssets, GameBitmapHandle_Yuu, BitmapMemory);
            PushCommandLinkTexture(RenderCommands,
                                   GetBitmap(GameAssets, GameBitmapHandle_Yuu), 
                                   GameBitmapHandle_Yuu);
            EndTemporaryMemory(TempMemory);
        }
        {// blank
            auto TempMemory = BeginTemporaryMemory(MainArena);
            const char* Filepath = "assets/blank.bmp";
            u32 Filesize = Platform->GetFileSize(Filepath);
            Assert(Filesize);
            void* BitmapMemory = PushBlock(TempMemory.Arena, Filesize);
            Platform->ReadFile(BitmapMemory, Filesize, Filepath);
            LoadBitmap(GameAssets, GameBitmapHandle_Blank, BitmapMemory);
            PushCommandLinkTexture(RenderCommands, 
                                   GetBitmap(GameAssets, GameBitmapHandle_Blank),
                                   GameBitmapHandle_Blank);
            EndTemporaryMemory(TempMemory);
        }
        {// karu
            auto TempMemory = BeginTemporaryMemory(MainArena);
            const char* Filepath = "assets/karu.bmp";
            u32 Filesize = Platform->GetFileSize(Filepath);
            Assert(Filesize);
            void* BitmapMemory = PushBlock(TempMemory.Arena, Filesize);
            Platform->ReadFile(BitmapMemory, Filesize, Filepath);
            LoadBitmap(GameAssets, GameBitmapHandle_Karu, BitmapMemory);
            PushCommandLinkTexture(RenderCommands, 
                                   GetBitmap(GameAssets, GameBitmapHandle_Karu),
                                   GameBitmapHandle_Karu);
            EndTemporaryMemory(TempMemory);
        }
        
        // NOTE(Momo): Init Spritesheet Assets
        {// karu
            bitmap Bitmap = GetBitmap(GameState->Assets, GameBitmapHandle_Karu);
            f32 CellWidth = 48.f/Bitmap.Width;
            f32 CellHeight = 48.f/Bitmap.Height;
            f32 HalfPixelWidth =  0.25f/Bitmap.Width;
            f32 HalfPixelHeight = 0.25f/Bitmap.Height;
            
            // NOTE(Momo):  Init sprite frames
            rect2f Rects[12];
            for (u8 r = 0; r < 4; ++r) {
                for (u8 c = 0; c < 3; ++c) {
                    Rects[TwoToOne(r,c,3)] = { 
                        c * CellWidth + HalfPixelWidth,
                        r * CellHeight + HalfPixelHeight,
                        (c+1) * CellWidth - HalfPixelWidth,
                        (r+1) * CellHeight - HalfPixelHeight,
                    };
                }
            }
            LoadSpritesheet(GameAssets, GameSpritesheetHandle_Karu, GameBitmapHandle_Karu, Rects, 12);
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
        case game_mode_menu::TypeId: {
            UpdateMode(GameState->MenuMode, GameState, RenderCommands, Input, DeltaTime);
        } break;
        case game_mode_main::TypeId: {
            UpdateMode(GameState->MainMode, GameState, RenderCommands, Input, DeltaTime);
        } break;
    }
}