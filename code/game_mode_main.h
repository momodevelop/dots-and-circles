/* date = July 30th 2020 0:41 pm */

#ifndef GAME_MODE_MAIN_H
#define GAME_MODE_MAIN_H

#include "game.h"
#include "game_mode_splash.h"

// NOTE(Momo): Entities ///////////////////////////////////
struct main_image_entity {
    v3f Scale;
    v3f Position;
    c4f Colors;
    u32 TextureHandle;
};

static inline void
Update(main_image_entity* Entity, 
       game_assets* Assets,
       render_commands * RenderCommands, 
       f32 DeltaTime) {
    
    // NOTE(Momo): Render
    m44f T = TranslationMatrix(Entity->Position);
    m44f S = ScaleMatrix(Entity->Scale);
    
    // TODO(Momo): This part should be done by renderer?
    PushCommandDrawTexturedQuad(RenderCommands, 
                                Entity->Colors, 
                                T*S,
                                Entity->TextureHandle);
}


// NOTE(Momo): Mode /////////////////////////////////////////////
struct game_mode_main {
    static constexpr u8 TypeId = 1;
    main_image_entity Img;
};

static inline void 
InitMode(game_mode_main* Mode, game_state* GameState) {
    // NOTE(Momo): Create entities
    {
        Mode->Img.Position = { 0.f, 0.f, 0.f };
        Mode->Img.Scale = { 200.f, 200.f };
        Mode->Img.Colors = { 1.f, 1.f, 1.f, 1.f };
        Mode->Img.TextureHandle = GameBitmapHandle_Yuu;
    }
    Log("Main state initialized!");
}

static inline void
UpdateMode(game_mode_main* Mode,
           game_state* GameState, 
           render_commands* RenderCommands, 
           game_input* Input,
           f32 DeltaTime) 
{
    
#if INTERNAL
    if (ProcessMetaInput(GameState, Input)) {
        return;
    }
#endif
    
    PushCommandClear(RenderCommands, { 0.0f, 0.3f, 0.3f, 0.f });
    
    Update(&Mode->Img, 
           GameState->Assets, 
           RenderCommands, 
           DeltaTime);
}



#endif //GAME_MODE_MAIN_H
