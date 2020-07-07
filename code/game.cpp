#include "game_platform.h"


struct game_state {
    f32 Rotation;
};


static inline void 
Init(game_state * GameState) {
    
    GameState->Rotation = 0.f;
}

// NOTE(Momo):  Exported Functions
extern "C" 
GAME_UPDATE(GameUpdate) {
    
    game_state* GameState = (game_state*)GameMemory->PermanentStore;
    if(!GameMemory->IsInitialized) {
        Init(GameState);
        for (u32 i = 0 ; i < 1024; ++i ) {
            RenderInfo->Transforms[i] = Identity();
        }
        RenderInfo->Count = 1024;
        GameMemory->IsInitialized = true;
    }
    
    f32 startX = -1600.f/2.f;
    f32 startY = -900.f/2.f;
    f32 xOffset = 200.f;
    f32 yOffset = 200.f;
    f32 currentXOffset = 0.f;
    f32 currentYOffset = 0.f;
    
    m44f quadColorful = {
        1.f, 1.f, 0.f, 0.f,
        0.f, 1.f, 0.f, 0.f,
        0.f, 0.f, 1.f, 0.0f,
        1.f, 1.f, 1.f, 0.0f,
    };
    
    for (u32 i = 0 ; i < 1024; ++i ) {
        auto T = MakeTranslationMatrix(startX + currentXOffset, startY + currentYOffset, 0.f);
        auto R = MakeRotationZMatrix(GameState->Rotation);
        auto S = MakeScaleMatrix(100.f, 100.f, 1.f);
        RenderInfo->Transforms[i] = Transpose(T*R*S);
        RenderInfo->Colors[i] = quadColorful;
        
        
        currentXOffset += xOffset;
        if (currentXOffset > 1600) {
            currentXOffset = 0.f;
            currentYOffset += yOffset;
        }
    }
    
    GameState->Rotation += DeltaTime;
    Wrap(GameState->Rotation, -Pi32, Pi32);
    
}