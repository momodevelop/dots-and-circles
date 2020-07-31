#ifndef GAME_MODE_SANDBOX_H
#define GAME_MODE_SANDBOX_H

#include "ryoji_easing.h"
#include "game.h"
#include "game_assets.h"



struct test_entity {
    v3f Scale;
    f32 Rotation;
    v3f Position;
    c4f Colors;
    u32 TextureHandle;
    f32 Timer;
};


static inline void
Update(test_entity* Entity, 
       game_assets* Assets,
       render_commands * RenderCommands, 
       f32 DeltaTime) {
    
    // NOTE(Momo): Update
    
    Entity->Rotation +=  DeltaTime;
    
    // NOTE(Momo): Render
    m44f T = TranslationMatrix(Entity->Position);
    m44f R = RotationZMatrix(Entity->Rotation);
    m44f S = ScaleMatrix(Entity->Scale);
    
    // TODO(Momo): This part should be done by renderer?
    PushCommandTexturedQuad(RenderCommands, 
                            Entity->Colors, 
                            T*R*S,
                            Assets->Textures[Entity->TextureHandle]);
}


struct game_mode_sandbox {
    static constexpr u8 TypeId = 2;
    static constexpr u32 TotalEntities = 2500;
    test_entity Entities[TotalEntities];
};


static inline void
InitMode(game_mode_sandbox* Mode, game_state* GameState) {
    // NOTE(Momo): Create entities
    f32 offsetX = -700.f;
    f32 offsetY = -200.f;
    f32 offsetDeltaX = 5.f;
    f32 offsetDeltaY = 5.f;
    for (u32 i = 0; i < game_mode_sandbox::TotalEntities; ++i)
    {
        Mode->Entities[i].Position = { offsetX, offsetY, 0.f };
        Mode->Entities[i].Rotation = 0.f;
        Mode->Entities[i].Scale = { 5.f, 5.f };
        Mode->Entities[i].Colors = { 1.f, 1.f, 1.f, 0.5f };
#if 1
        if ( i < (game_mode_sandbox::TotalEntities / 3))
            Mode->Entities[i].TextureHandle = 1;
        else if ( i < (game_mode_sandbox::TotalEntities / 3 * 2))
            Mode->Entities[i].TextureHandle = 2;
        else if ( i < game_mode_sandbox::TotalEntities)
            Mode->Entities[i].TextureHandle = 0;
#else
        Mode->Entities[i].TextureHandle = i % 3;
#endif
        offsetX += offsetDeltaX;
        if (offsetX >= 700.f) {
            offsetX = -700.f;
            offsetY += offsetDeltaY;
        }
        
    }
    Log("Sandbox state initialized!");
}

static inline void
UpdateMode(game_mode_sandbox* Mode,
           game_state* GameState, 
           render_commands* RenderCommands, 
           f32 DeltaTime) 
{
    PushCommandClear(RenderCommands, { 0.0f, 0.0f, 0.0f, 0.f });
    
    for (u32 i = 0; i < game_mode_sandbox::TotalEntities; ++i) {
        Update(&Mode->Entities[i], 
               GameState->Assets, 
               RenderCommands, 
               DeltaTime);
    }
}











#endif //GAME_MODE_SANDBOX_H
