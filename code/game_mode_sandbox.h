#ifndef GAME_MODE_SANDBOX_H
#define GAME_MODE_SANDBOX_H

#include "ryoji_easing.h"
#include "game.h"
#include "game_assets.h"


// NOTE(Momo): Wtf...
quad2f SpriteAnimationDown[] {
    {// left
        0.00f, 0.01f,
        1.f/3.f, 0.01f,
        0.00f, 0.25f,
        1.f/3.f, 0.25f
    },
    {// right
        2.f/3.f, 0.01f,
        1.00f, 0.01f,
        2.f/3.f, 0.25f,
        1.00f, 0.25f
    },
};

quad2f SpriteAnimationLeft[] {
    {// left
        0.00f, 0.25f,
        0.34f, 0.25f,
        0.00f, 0.50f,
        0.34f, 0.50f
    },
    {// right
        0.67f, 0.25f,
        1.00f, 0.25f,
        0.67f, 0.50f,
        1.00f, 0.50f
    },
};

quad2f SpriteAnimationRight[] {
    {// left
        0.00f, 0.50f,
        0.34f, 0.50f,
        0.00f, 0.75f,
        0.34f, 0.75f
    },
    {// right
        0.67f, 0.50f,
        1.00f, 0.50f,
        0.67f, 0.75f,
        1.00f, 0.75f
    },
};

quad2f SpriteAnimationUp[] {
    {// left
        0.00f, 0.75f,
        0.34f, 0.75f,
        0.00f, 1.00f,
        0.34f, 1.00f
    },
    {// right
        0.67f, 0.75f,
        1.00f, 0.75f,
        0.67f, 1.00f,
        1.00f, 1.00f
    },
};


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


struct sandbox_sprite_component {
    u32 TextureHandle;
    
    // NOTE(Momo): Animation Data?
    quad2f* AnimationArray;
    u8 AnimationArraySize;
    u8 AnimationAt;
    u8 AnimationStopIndex;
    f32 AnimationTimer;
    f32 AnimationSpeed;
};


static inline void
SpriteRenderingSystem(sandbox_transform_component* Transform,
                      sandbox_sprite_component* Sprite,
                      render_commands* RenderCommands,
                      game_assets* Assets,
                      f32 DeltaTime) 
{
    
    // NOTE(Momo): should be in a seperate system probably
    Sprite->AnimationTimer += Sprite->AnimationSpeed * DeltaTime;
    if (Sprite->AnimationTimer >= 1.f) {
        Sprite->AnimationTimer = 0.f;
        ++Sprite->AnimationAt;
        if (Sprite->AnimationAt >= Sprite->AnimationArraySize) {
            Sprite->AnimationAt = 0;
        }
    }
    
    
    m44f T = TranslationMatrix(Transform->Position);
    m44f R = RotationZMatrix(Transform->Rotation);
    m44f S = ScaleMatrix(Transform->Scale);
    
    
    // TODO(Momo): This part should be done by renderer?
    PushCommandTexturedQuad(RenderCommands, 
                            {1.f, 1.f, 1.f, 1.f}, 
                            T*R*S,
                            Assets->Textures[Sprite->TextureHandle],
                            Sprite->AnimationArray[Sprite->AnimationAt]);
}

struct sandbox_physics_component {
    v3f Velocity;
    v3f Acceleration;
};

// TODO(Momo): Animation component
struct sandbox_entity {
    sandbox_transform_component Transform;
    sandbox_physics_component Physics;
    sandbox_sprite_component Sprite;
};

static inline void
PhysicsSystem(sandbox_transform_component* Transform,
              sandbox_physics_component* Physics,
              f32 DeltaTime) 

{
    Physics->Velocity += Physics->Acceleration * DeltaTime;
    Transform->Position += Physics->Velocity * DeltaTime;
    Physics->Acceleration = {};
}

static inline void
Update(sandbox_entity* Entity, 
       game_assets* Assets,
       render_commands * RenderCommands, 
       f32 DeltaTime) {
    
    PhysicsSystem(&Entity->Transform, &Entity->Physics, DeltaTime);
    
    SpriteRenderingSystem(&Entity->Transform, 
                          &Entity->Sprite, 
                          RenderCommands, 
                          Assets,
                          DeltaTime);
}

struct game_mode_sandbox {
    static constexpr u8 TypeId = 2;
    sandbox_entity Player;
};

static inline void
InitMode(game_mode_sandbox* Mode, game_state* GameState) {
    
    sandbox_entity* Player = &Mode->Player;
    
    Player->Transform.Position = {};
    Player->Transform.Rotation = 0.f;
    Player->Transform.Scale = { 48.f * 2, 48.f * 2 };
    
    Player->Physics = {};
    
    Player->Sprite.TextureHandle = GameTextureType_karu;
    Player->Sprite.AnimationArray = SpriteAnimationDown;
    Player->Sprite.AnimationArraySize = ArrayCount(SpriteAnimationDown);
    Player->Sprite.AnimationAt = 0;
    Player->Sprite.AnimationStopIndex = 0;
    Player->Sprite.AnimationTimer = 0.f;
    Player->Sprite.AnimationSpeed = 10.f;
    
    
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
    auto* Player = &Mode->Player;
    f32 PlayerSpeed = 500.f;
    v3f Direction = {};
    
    // NOTE(Momo): Player controls
    if(IsDown(Input->ButtonLeft)) {
        Direction.X = -1.f;
        Player->Sprite.AnimationArray = SpriteAnimationLeft;
    };
    
    if(IsDown(Input->ButtonRight)) {
        Direction.X = 1.f;
        Player->Sprite.AnimationArray = SpriteAnimationRight;
    }
    
    if(IsDown(Input->ButtonUp)) {
        Direction.Y = 1.f;
        Player->Sprite.AnimationArray = SpriteAnimationUp;
    };
    if(IsDown(Input->ButtonDown)) {
        Direction.Y = -1.f;
        Player->Sprite.AnimationArray = SpriteAnimationDown;
    }
    if (Len(Direction) > 0.f) 
        Player->Physics.Acceleration = Normalize(Direction) * PlayerSpeed;
    
    PushCommandClear(RenderCommands, { 0.65f, 0.9f, 1.0f, 0.f });
    Update(Player, 
           GameState->Assets, 
           RenderCommands, 
           DeltaTime);
}
#endif //GAME_MODE_SANDBOX_H
