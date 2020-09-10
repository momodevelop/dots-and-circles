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
#if 0
    game_assets* Assets = GameState->Assets;
    {
        
        v4f Color = {1.f, 1.f, 1.f, 1.f};
        font_id Font = GetFont(Assets, Asset_FontDefault);
        
        
        f32 Size = 36.f;
        f32 OffsetX = 0.f;
        for ( u32 i = 0; i < 5; ++i ){
            quad2f Quad = Quad2(GetFontCharacterUV(Assets, Font, i));
            f32 Aspect = GetFontCharacterAspect(Assets, Font, i);
            
            
            u32 TextureHandle = GetBitmapId(Assets, Font);
            m44f Transform = TranslationMatrix(OffsetX, 1.f, 0.f) *ScaleMatrix(Size * Aspect, Size, 1.f); 
            
            OffsetX += Size * Aspect;
            PushCommandDrawTexturedQuad(RenderCommands, 
                                        Color, 
                                        Transform, 
                                        TextureHandle,
                                        Quad);
        }
    }
#endif
}



#endif //GAME_MODE_MENU_H
