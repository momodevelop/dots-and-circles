#ifndef GAME_MODE_SANDBOX_H
#define GAME_MODE_SANDBOX_H

#include "ryoji_easing.h"
#include "ryoji_maths.h"
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


struct sandbox_sprite_component {
    u32 TextureHandle;
    
    // NOTE(Momo): Animation Data?
    quad2f* SpriteFrames;
    u8* AnimeFrames;
    u8 AnimeFramesCount;
    u8 AnimeAt;
    f32 AnimeTimer;
    f32 AnimeSpeed;
};




static inline void
SpriteRenderingSystem(sandbox_transform_component* Transform,
                      sandbox_sprite_component* Sprite,
                      render_commands* RenderCommands,
                      game_assets* Assets,
                      f32 DeltaTime) 
{
    
    // NOTE(Momo): should be in a seperate system probably
    Sprite->AnimeTimer += Sprite->AnimeSpeed * DeltaTime;
    if (Sprite->AnimeTimer >= 1.f) {
        Sprite->AnimeTimer = 0.f;
        ++Sprite->AnimeAt;
        if (Sprite->AnimeAt >= Sprite->AnimeFramesCount) {
            Sprite->AnimeAt = 0;
        }
        //Log("AnimeAt: %d is %d", Sprite->AnimeAt , Sprite->AnimeFrames[Sprite->AnimeAt]);
        
    }
    
    m44f T = TranslationMatrix(Transform->Position);
    m44f R = RotationZMatrix(Transform->Rotation);
    m44f S = ScaleMatrix(Transform->Scale);

    
    // TODO(Momo): This part should be done by renderer?
    PushCommandTexturedQuad(RenderCommands, 
                            {1.f, 1.f, 1.f, 1.f}, 
                            T*R*S,
                            Sprite->TextureHandle,
                            Sprite->SpriteFrames[Sprite->AnimeFrames[Sprite->AnimeAt]]);
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
    // NOTE(Momo): Spritesheet data? Animation data???
    quad2f SpriteFrames[12];
    
    u8 AnimeWalkDown[4];
    u8 AnimeWalkLeft[4];
    u8 AnimeWalkRight[4];
    u8 AnimeWalkUp[4];
    
    u8 AnimeIdleDown[1];
    u8 AnimeIdleUp[1];
    u8 AnimeIdleLeft[1];
    u8 AnimeIdleRight[1];
    
    
    // TODO(Momo): sprite animation system?
    sandbox_entity Player;
};

static inline void
InitMode(game_mode_sandbox* Mode, game_state* GameState) {
    // TODO(Momo): Shift this to some other object?
    bitmap Bitmap = GetBitmap(GameState->Assets, GameBitmapHandle_Karu);
    f32 CellWidth = 48.f/Bitmap.Width;
    f32 CellHeight = 48.f/Bitmap.Height;
    f32 HalfPixelWidth =  1.f/Bitmap.Width;
    f32 HalfPixelHeight = 1.f/Bitmap.Height;
    
    // NOTE(Momo):  Init sprite frames
    for (u8 r = 0; r < 4; ++r) {
        for (u8 c = 0; c < 3; ++c) {
            rect2f Rect = { 
                c * CellWidth + HalfPixelWidth,
                r * CellHeight + HalfPixelHeight,
                (c+1) * CellWidth - HalfPixelWidth,
                (r+1) * CellHeight - HalfPixelHeight,
            };
            Mode->SpriteFrames[TwoToOne(r,c,3)] = RectToQuad(Rect);
        }
    }
    
    
    Mode->AnimeWalkDown[0] = 1; 
    Mode->AnimeWalkDown[1] = 0;
    Mode->AnimeWalkDown[2] = 1;
    Mode->AnimeWalkDown[3] = 2;
    
    Mode->AnimeWalkLeft[0] = 4; 
    Mode->AnimeWalkLeft[1] = 3;
    Mode->AnimeWalkLeft[2] = 4;
    Mode->AnimeWalkLeft[3] = 5;
    
    Mode->AnimeWalkRight[0] = 7; 
    Mode->AnimeWalkRight[1] = 6;
    Mode->AnimeWalkRight[2] = 7;
    Mode->AnimeWalkRight[3] = 8;
    
    Mode->AnimeWalkUp[0] = 10; 
    Mode->AnimeWalkUp[1] = 9;
    Mode->AnimeWalkUp[2] = 10;
    Mode->AnimeWalkUp[3] = 11;
    
    Mode->AnimeIdleDown[0] = 1;
    Mode->AnimeIdleLeft[0] = 4;
    Mode->AnimeIdleRight[0] = 7;
    Mode->AnimeIdleUp[0] = 10;
    
    
    // NOTE(Momo): Init player
    sandbox_entity* Player = &Mode->Player;
    
    Player->Transform.Position = {};
    Player->Transform.Rotation = 0.f;
    Player->Transform.Scale = { 48.f * 2, 48.f * 2 };
    
    Player->Physics = {};
    
    Player->Sprite.TextureHandle = GameBitmapHandle_Karu;
    Player->Sprite.SpriteFrames = Mode->SpriteFrames;
    Player->Sprite.AnimeFrames = Mode->AnimeWalkDown;
    Player->Sprite.AnimeFramesCount = ArrayCount(Mode->AnimeWalkDown);
    Player->Sprite.AnimeAt = 0;
    Player->Sprite.AnimeTimer = 0.f;
    Player->Sprite.AnimeSpeed = 10.f;
    
    
    Log("Sandbox state initialized!");
}


enum Direction  {
    UP,
    DOWN,
    LEFT,
    RIGHT
} CurrentDirection = DOWN;


static inline void
UpdateMode(game_mode_sandbox* Mode,
           game_state* GameState, 
           render_commands* RenderCommands, 
           game_input* Input,
           f32 DeltaTime) 
{
    
#if INTERNALGameBitmapHandle_Karu
    if (ProcessMetaInput(GameState, Input)) {
        return;
    }
#endif
    auto* Player = &Mode->Player;
    f32 PlayerSpeed = 400.f;
    v3f Direction = {};
    b8 IsMovementButtonDown = false;
    // NOTE(Momo): Player controls
    if(IsDown(Input->ButtonLeft)) {
        Direction.X = -1.f;
        Player->Sprite.AnimeFrames = Mode->AnimeWalkLeft;
        Player->Sprite.AnimeFramesCount = ArrayCount(Mode->AnimeWalkLeft);
        IsMovementButtonDown = true;
        CurrentDirection = LEFT;
    };
    
    if(IsDown(Input->ButtonRight)) {
        Direction.X = 1.f;
        Player->Sprite.AnimeFrames = Mode->AnimeWalkRight;
        Player->Sprite.AnimeFramesCount = ArrayCount(Mode->AnimeWalkRight);
        IsMovementButtonDown = true;
        CurrentDirection = RIGHT;
    }
    
    if(IsDown(Input->ButtonUp)) {
        Direction.Y = 1.f;
        Player->Sprite.AnimeFrames = Mode->AnimeWalkUp;
        Player->Sprite.AnimeFramesCount = ArrayCount(Mode->AnimeWalkUp);
        IsMovementButtonDown = true;
        CurrentDirection = UP;
    }
    if(IsDown(Input->ButtonDown)) {
        Direction.Y = -1.f;
        Player->Sprite.AnimeFrames = Mode->AnimeWalkDown;
        Player->Sprite.AnimeFramesCount = ArrayCount(Mode->AnimeWalkDown);
        IsMovementButtonDown = true;
        CurrentDirection = DOWN;
    }
    
    
    if (IsMovementButtonDown) 
        Player->Physics.Velocity = Normalize(Direction) * PlayerSpeed;
    else {
        Player->Physics.Velocity = {};
        // TODO(Momo): Idle animation?????
        switch(CurrentDirection) {
            case DOWN: {
                Player->Sprite.AnimeAt = 0;
                Player->Sprite.AnimeFrames = Mode->AnimeIdleDown;
                Player->Sprite.AnimeFramesCount = ArrayCount(Mode->AnimeIdleDown);
                
            } break;
            case LEFT: {
                Player->Sprite.AnimeAt = 0;
                Player->Sprite.AnimeFrames = Mode->AnimeIdleLeft;
                Player->Sprite.AnimeFramesCount = ArrayCount(Mode->AnimeIdleLeft);
            } break;
            case RIGHT: {
                Player->Sprite.AnimeAt = 0;
                Player->Sprite.AnimeFrames = Mode->AnimeIdleRight;
                Player->Sprite.AnimeFramesCount = ArrayCount(Mode->AnimeIdleRight);
            } break;
            case UP: {
                Player->Sprite.AnimeAt = 0;
                Player->Sprite.AnimeFrames = Mode->AnimeIdleUp;
                Player->Sprite.AnimeFramesCount = ArrayCount(Mode->AnimeIdleUp);
            } break;
            
        }
    }
    
    
    PushCommandClear(RenderCommands, { 0.65f, 0.9f, 1.0f, 0.f });
    Update(Player, 
           GameState->Assets, 
           RenderCommands, 
           DeltaTime);
}
#endif //GAME_MODE_SANDBOX_H
