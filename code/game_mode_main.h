#ifndef GAME_MODE_MAIN_H
#define GAME_MODE_MAIN_H

#include "ryoji_easing.h"
#include "ryoji_maths.h"

enum player_absorb_mode : b32 {
    PlayerAbsorbMode_White,
    PlayerAbsorbMode_Black,
};

struct player {
    // NOTE(Momo): Rendering
    atlas_rect* WhiteImageRect;
    atlas_rect* BlackImageRect;
    f32 WhiteImageAlpha;
    f32 WhiteImageAlphaTarget;
    f32 WhiteImageTransitionTimer;
    f32 WhiteImageTransitionDuration;
    
    v3f Size;
    
    
    // NOTE(Momo): Physics??
    v3f Position;
    v3f Velocity; 
    
    // NOTE(Momo): Gameplay
    player_absorb_mode AbsorbMode;
    f32 MovementSpeed;
    
};

struct game_mode_main {
    player Player;
};

static inline void 
Init(game_mode_main* Mode, game_state* GameState) {
    Log("Main state initialized!");
    
    auto* Assets = GameState->Assets;
    auto* Player = &Mode->Player;
    Player->MovementSpeed = 250.f;
    Player->WhiteImageRect = Assets->AtlasRects + AtlasRect_PlayerWhite;
    Player->BlackImageRect = Assets->AtlasRects + AtlasRect_PlayerBlack;
    Player->Position = {};
    Player->Velocity = {};
    Player->Size = { 32.f, 32.f };
    
    // NOTE(Momo): We start as White
    Player->AbsorbMode = PlayerAbsorbMode_White;
    Player->WhiteImageAlpha = 1.f;
    Player->WhiteImageAlphaTarget = 1.f;
    
    Player->WhiteImageTransitionDuration = 0.05f;
    Player->WhiteImageTransitionTimer = Player->WhiteImageTransitionDuration;
}



static inline void
Update(game_mode_main* Mode,
       game_state* GameState, 
       commands* RenderCommands, 
       game_input* Input,
       f32 DeltaTime) 
{
#if INTERNAL
    if (ProcessMetaInput(GameState, Input)) {
        return;
    }
#endif
    PushCommandClearColor(RenderCommands, { 0.3f, 0.3f, 0.3f, 1.f });
    PushCommandSetOrthoBasis(RenderCommands, {}, { 1600.f, 900.f, 200.f });
    
    auto* Assets = GameState->Assets;
    auto* Player = &Mode->Player;
    
    // NOTE(Momo): Input
    {
        v3f Direction = {};
        b8 IsMovementButtonDown = false;
        if(IsDown(Input->ButtonLeft)) {
            Direction.X = -1.f;
            IsMovementButtonDown = true;
        };
        
        if(IsDown(Input->ButtonRight)) {
            Direction.X = 1.f;
            IsMovementButtonDown = true;
        }
        
        if(IsDown(Input->ButtonUp)) {
            Direction.Y = 1.f;
            IsMovementButtonDown = true;
        }
        if(IsDown(Input->ButtonDown)) {
            Direction.Y = -1.f;
            IsMovementButtonDown = true;
        }
        
        if (IsMovementButtonDown) 
            Player->Velocity = Normalize(Direction) * Player->MovementSpeed;
        else {
            Player->Velocity = {};
        }
        
        
        // NOTE(Momo): Absorb Mode Switch
        if(IsPoked(Input->ButtonSwitch)) {
            Player->AbsorbMode = (Player->AbsorbMode == PlayerAbsorbMode_White) ? PlayerAbsorbMode_Black : PlayerAbsorbMode_White;
            
            
            switch(Player->AbsorbMode) {
                case PlayerAbsorbMode_White: {
                    Log("White nao!");
                    Player->WhiteImageAlphaTarget = 1.f;
                } break;
                case PlayerAbsorbMode_Black: {
                    Log("Black nao!");
                    Player->WhiteImageAlphaTarget = 0.f;
                }break;
                
            }
            Player->WhiteImageTransitionTimer = 0.f;
        }
    }
    
    // NOTE(Momo): Player Update
    {
        
        Player->WhiteImageAlpha = Lerp(1.f - Player->WhiteImageAlphaTarget, 
                                       Player->WhiteImageAlphaTarget, Player->WhiteImageTransitionTimer / Player->WhiteImageTransitionDuration);
        
        Player->WhiteImageTransitionTimer += DeltaTime;
        Player->WhiteImageTransitionTimer = Clamp(Player->WhiteImageTransitionTimer, 0.f, Player->WhiteImageTransitionDuration);
        
        Log("%f =  %f, %f ,%f, %f", Player->WhiteImageAlpha, 1.f - Player->WhiteImageAlphaTarget, Player->WhiteImageAlphaTarget, Player->WhiteImageTransitionTimer, Player->WhiteImageTransitionDuration);
        
        Player->Position += Player->Velocity * DeltaTime;
    }
    
    // NOTE(Momo): Rendering
    {
        m44f S = ScaleMatrix(Player->Size);
        m44f T = TranslationMatrix(Player->Position.X, Player->Position.Y, Player->Position.Z);
        PushCommandDrawTexturedQuad(RenderCommands, 
                                    {1.f, 1.f, 1.f, 1.f }, 
                                    T*S, 
                                    Player->BlackImageRect->BitmapId,
                                    GetAtlasUV(Assets, Player->BlackImageRect));
        
        T = TranslationMatrix(Player->Position.X, Player->Position.Y, Player->Position.Z + 0.1f);
        PushCommandDrawTexturedQuad(RenderCommands, 
                                    {1.f, 1.f, 1.f, Player->WhiteImageAlpha}, 
                                    T*S, 
                                    Player->WhiteImageRect->BitmapId,
                                    GetAtlasUV(Assets, Player->WhiteImageRect));
    }
    
}

#endif //GAME_MODE_H
