#ifndef GAME_MODE_ATLAS_TEST_H
#define GAME_MODE_ATLAS_TEST_H

#include "game.h"


// NOTE(Momo): Mode /////////////////////////////////////////////
struct game_mode_atlas_test {
    static constexpr u8 TypeId = 3;
};

static inline void 
Init(game_mode_atlas_test* Mode, game_state* GameState) {
    Log("Atlas Test state initialized!");
}

static inline void
Update(game_mode_atlas_test* Mode,
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
    PushCommandClearColor(RenderCommands, { 0.0f, 0.3f, 0.3f, 0.f });
    PushCommandSetOrthoBasis(RenderCommands, { 0.f, 0.f, 0.f }, { 1600.f, 900.f, 200.f });
    
    // NOTE(Momo): Image Test
    {
        v4f Color = { 1.f, 1.f, 1.f, 1.f };
        m44f Transform = ScaleMatrix(200.f, 200.f, 1.f);
        auto AtlasRect = GameState->Assets->AtlasRects[AtlasRect_Ryoji];
        
        auto TextureHandle = AtlasRect.BitmapId;
        PushCommandDrawTexturedQuad(RenderCommands, 
                                    Color, 
                                    Transform, 
                                    AtlasRect.BitmapId,
                                    GetAtlasUV(GameState->Assets, AtlasRect));
    }
    
    
    
    
}



#endif 
