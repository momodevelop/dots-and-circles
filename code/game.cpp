#include "game_platform.h"


// TODO(Momo): Complete this function
#if 0
static inline render_info* 
ConstructRenderInfo(/**/usize Size) {
    // TODO(Momo): Change to memory arena
    render_info* Ret = (render_info*)malloc(Size);
    return Ret;
}

static inline void
Push(render_cmd_list* RenderCmdList,  render_cmd_type Type, void* Data) {
}

// TODO(Momo): Complete this function
static inline void 
Clear(render_cmd_list* RenderCmdList) {
    
}

#endif

struct game_state {
    render_info RenderInfo;
    f32 TimeElapsed;
    f32 Rotation;
};


static inline void 
GameInit(game_state * GameState) {
    GameState->TimeElapsed = 0.f;
    for (u32 i = 0 ; i < 1024; ++i ) {
        GameState->RenderInfo.Transforms[i] = Identity();
    }
    GameState->RenderInfo.Count = 1024;
    GameState->Rotation = 0.f;
}

// NOTE(Momo):  Exported Functions
extern "C" 
GAME_UPDATE(GameUpdate) {
    game_state* GameState = (game_state*)GameMemory->PermanentStore;
    if(!GameMemory->IsInitialized) {
        GameInit(GameState);
        GameMemory->IsInitialized = true;
    }
    
    GameState->TimeElapsed += DeltaTime;
    
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
        GameState->RenderInfo.Transforms[i] = Transpose(T*R*S);
        GameState->RenderInfo.Colors[i] = quadColorful;
        
        
        currentXOffset += xOffset;
        if (currentXOffset > 1600) {
            currentXOffset = 0.f;
            currentYOffset += yOffset;
        }
    }
    
    GameState->Rotation += DeltaTime;
    Wrap(GameState->Rotation, -Pi32, Pi32);
    
    //PlatformApi->Log("Updating: %f", GameState->TimeElapsed);
    
    GameMemory->PlatformApi.SetRenderInfo(&GameState->RenderInfo);
    
}