#ifndef GAME_MODE_SANDBOX_H
#define GAME_MODE_SANDBOX_H

#include "ryoji_easing.h"
#include "game.h"
#include "game_assets.h"

// NOTE(Momo): EC....S?
struct sandbox_transform_component {
    v3f Scale;
    f32 Rotation;
    v3f Position;
};

struct sandbox_renderable_component {
    c4f Colors;
    u32 TextureHandle;
    quad2f TextureCoords;
};

// TODO(Momo): Animation component

struct sandbox_entity {
    sandbox_transform_component Transform;
    sandbox_renderable_component Renderable;
    f32 Timer;
};



static inline void RenderingSystem(sandbox_transform_component* Transform, sandbox_renderable_component* Renderable,
                                   render_commands* RenderCommands,
                                   game_assets* Assets) 
{
    // NOTE(Momo): Render
    m44f T = TranslationMatrix(Transform->Position);
    m44f R = RotationZMatrix(Transform->Rotation);
    m44f S = ScaleMatrix(Transform->Scale);
    
    // TODO(Momo): This part should be done by renderer?
    PushCommandTexturedQuad(RenderCommands, 
                            Renderable->Colors, 
                            T*R*S,
                            Assets->Textures[Renderable->TextureHandle],
                            {
                                0.5f, 0.5f, // top right
                                0.5f, 0.f, // bottom right
                                0.f, 0.f, // bottom left
                                0.f, 0.5f,  // top left
                            });
}

static inline void
Update(sandbox_entity* Entity, 
       game_assets* Assets,
       render_commands * RenderCommands, 
       f32 DeltaTime) {
    
    // NOTE(Momo): Update
    Entity->Transform.Rotation +=  DeltaTime;
    
    RenderingSystem(&Entity->Transform, 
                    &Entity->Renderable, 
                    RenderCommands, 
                    Assets);
    
}


struct game_mode_sandbox {
    static constexpr u8 TypeId = 2;
    static constexpr u32 TotalEntities = 2500;
    sandbox_entity Entities[TotalEntities];
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
        Mode->Entities[i].Transform.Position = { offsetX, offsetY, 0.f };
        Mode->Entities[i].Transform.Rotation = 0.f;
        Mode->Entities[i].Transform.Scale = { 5.f, 5.f };
        
        Mode->Entities[i].Renderable.Colors = { 1.f, 1.f, 1.f, 0.5f };
        Mode->Entities[i].Renderable.TextureCoords = {
            0.0f, 1.0f,
            1.0f, 1.0f,
            0.0f, 0.0f,
            1.0f, 0.0f
        };
        
        if ( i < (game_mode_sandbox::TotalEntities / 3))
            Mode->Entities[i].Renderable.TextureHandle = 1;
        else if ( i < (game_mode_sandbox::TotalEntities / 3 * 2))
            Mode->Entities[i].Renderable.TextureHandle = 2;
        else if ( i < game_mode_sandbox::TotalEntities)
            Mode->Entities[i].Renderable.TextureHandle = 0;
        
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
           game_input* Input,
           f32 DeltaTime) 
{
    
#if INTERNAL
    if (ProcessMetaInput(GameState, Input)) {
        return;
    }
#endif
    PushCommandClear(RenderCommands, { 0.0f, 0.0f, 0.0f, 0.f });
    
    for (u32 i = 0; i < game_mode_sandbox::TotalEntities; ++i) {
        Update(&Mode->Entities[i], 
               GameState->Assets, 
               RenderCommands, 
               DeltaTime);
    }
}

#endif //GAME_MODE_SANDBOX_H
