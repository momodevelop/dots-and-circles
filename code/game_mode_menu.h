#ifndef GAME_MODE_MENU_H
#define GAME_MODE_MENU_H

#include "game_mode_splash.h"


// NOTE(Momo): Mode /////////////////////////////////////////////
struct game_mode_menu {
};

static inline void 
Init(game_mode_menu* Mode, game_state* GameState) {
    Log("Menu state initialized!");
}

static inline void
Update(game_mode_menu* Mode,
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
    
    game_assets* Assets = GameState->Assets;
    {
        v4f Color = {1.f, 1.f, 1.f, 1.f};
        m44f Transform = ScaleMatrix(36.f, 36.f, 1.f);
        
        font_id Font = GetFont(Assets, Asset_FontDefault);
        u32 TextureHandle = GetBitmapId(Assets, Font);
        
        quad2f Quad = Quad2(GetFontUV(Assets, Font, 0));
        PushCommandDrawTexturedQuad(RenderCommands, 
                                    Color, 
                                    Transform, 
                                    TextureHandle,
                                    Quad);
        
    }
}



#endif //GAME_MODE_MENU_H
