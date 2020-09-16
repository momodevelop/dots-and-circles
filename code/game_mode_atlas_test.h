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
        atlas_rect Rect = GetAtlasRect(GameState->Assets, Asset_RectRyoji);
        
        v4f Color = { 1.f, 1.f, 1.f, 1.f };
        m44f Transform = ScaleMatrix(200.f, 200.f, 1.f);
        //quad2f Quad = Quad2F(GetImageUV(GameState->Assets, AtlasId, AtlasDefault_Ryoji));
        
        atlas* Atlas = GetAtlas(GameState->Assets, Rect.AtlasAssetId);
        rect2f NormRect = RatioRect(Rect.Rect, {0, 0, Atlas->Width, Atlas->Height});
        quad2f Quad = Quad2F(NormRect);
        
        Log("%d %d %d %d", Rect.Rect.Min.X, Rect.Rect.Min.Y, Rect.Rect.Max.X, Rect.Rect.Max.Y);
        
        Log("%f %f %f %f", NormRect.Min.X, NormRect.Min.Y, NormRect.Max.X, NormRect.Max.Y);
        auto TextureHandle =  Atlas->BitmapId;
        PushCommandDrawTexturedQuad(RenderCommands, 
                                    Color, 
                                    Transform, 
                                    TextureHandle,
                                    Quad);
    }
    
    
    
    
}



#endif 
