#ifndef GAME_MODE_MENU_H
#define GAME_MODE_MENU_H

#include "game_mode_splash.h"


// NOTE(Momo): Mode /////////////////////////////////////////////
struct game_mode_menu {
};

static inline void 
InitMenuMode(permanent_state* PermState) {
}

static inline void
UpdateMenuMode(permanent_state* PermState, 
       mailbox* RenderCommands, 
       input* Input,
       f32 DeltaTime) 
{
    game_mode_menu* Mode = PermState->MenuMode;
    PushCommandClearColor(RenderCommands, { 0.0f, 0.3f, 0.3f, 0.f });
    PushCommandOrthoCamera(RenderCommands, 
            v3f{}, 
            CenteredRect( 
                v3f{ Global_DesignWidth, Global_DesignHeight, Global_DesignDepth }, 
                v3f{ 0.5f, 0.5f, 0.5f }
            )
    );
#if 0
    game_assets* Assets = PermState->Assets;
    {
        
        v4f Color = {1.f, 1.f, 1.f, 1.f};
        font_id Font = GetFont(Assets, Asset_FontDefault);
        
        
        f32 Size = 36.f;
        f32 OffsetX = 0.f;
        for ( u32 i = 0; i < 5; ++i ){
            quad2f Quad = Quad2(GetFontCharacterUV(Assets, Font, i));
            f32 Aspect = GetFontCharacterAspect(Assets, Font, i);
            
            
            u32 TextureHandle = GetBitmapId(Assets, Font);
            m44f Transform = TranslationMtxOffsetX, 1.f, 0.f) *M44fScale(Size * Aspect, Size, 1.f); 
            
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
