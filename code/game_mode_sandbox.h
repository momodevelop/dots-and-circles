#ifndef GAME_MODE_SANDBOX_H
#define GAME_MODE_SANDBOX_H

#include "ryoji_easing.h"
#include "ryoji_maths.h"
#include "game.h"
#include "game_assets.h"

// NOTE(Momo): Should we promote this to ryoji?
// The problem is that there are multiple ways to map a rect to quad
// This is but only one way.



enum Direction  {
    UP,
    DOWN,
    LEFT,
    RIGHT
};

// NOTE(Momo): EC....S?
struct sandbox_transform_component {
    v3f Scale;
    f32 Rotation;
    v3f Position;
};

struct sandbox_collision_component {
    aabb3f Box;
};

struct sandbox_renderable_component {
    c4f Colors;
    u32 TextureHandle;
    quad2f TextureCoords;
};

struct sandbox_sprite_component {
    game_spritesheet_handle SpritesheetHandle;
    
    // NOTE(Momo): Animation Data?
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
    
    
    auto Spritesheet = GetSpritesheet(Assets, Sprite->SpritesheetHandle);
    auto CurrentFrame = Sprite->AnimeFrames[Sprite->AnimeAt];
    
    PushCommandTexturedQuad(RenderCommands, 
                            {1.f, 1.f, 1.f, 1.f}, 
                            T*R*S,
                            Spritesheet.BitmapHandle,
                            UVRect2ToQuad2(Spritesheet.Frames[CurrentFrame]));
    
}

struct sandbox_physics_component {
    v3f Velocity;
    v3f Acceleration;
};



struct sandbox_player {
    sandbox_transform_component Transform;
    sandbox_physics_component Physics;
    sandbox_sprite_component Sprite;
    sandbox_collision_component Collision;
    
    Direction CurrentDirection;
    f32 MovementSpeed;
};

struct sandbox_block {
    sandbox_transform_component Transform;
    sandbox_collision_component Collision;
};

#if INTERNAL
static inline void
DrawCollisionDebugLinesSystem(sandbox_transform_component* Transform,
                              sandbox_collision_component* Collision, render_commands * RenderCommands) 
{
    
    auto* Box = &Collision->Box;
    
    rect2f Rect = {}; 
    Rect.Min.X = Transform->Position.X + Box->Origin.X - Box->HalfDimensions.X;
    Rect.Min.Y = Transform->Position.Y + Box->Origin.Y - Box->HalfDimensions.Y;
    Rect.Max.X = Transform->Position.X + Box->Origin.X + Box->HalfDimensions.X;
    Rect.Max.Y = Transform->Position.Y + Box->Origin.Y + Box->HalfDimensions.Y;
    
    // Bottom
    PushCommandDebugLine(RenderCommands, 
                         { 
                             Rect.Min.X, 
                             Rect.Min.Y,  
                             Rect.Max.X, 
                             Rect.Min.Y,
                         });
    // Left
    PushCommandDebugLine(RenderCommands, 
                         { 
                             Rect.Min.X,
                             Rect.Min.Y,
                             Rect.Min.X,
                             Rect.Max.Y,
                         });
    
    //Top
    PushCommandDebugLine(RenderCommands, 
                         { 
                             Rect.Min.X,
                             Rect.Max.Y,
                             Rect.Max.X,
                             Rect.Max.Y,
                         });
    
    //Right 
    PushCommandDebugLine(RenderCommands, 
                         { 
                             Rect.Max.X,
                             Rect.Min.Y,
                             Rect.Max.X,
                             Rect.Max.Y,
                         });
    
}
#endif

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
Update(sandbox_player* Entity, f32 DeltaTime) 
{
    PhysicsSystem(&Entity->Transform, &Entity->Physics, DeltaTime);
}

static inline void 
Render( sandbox_player* Entity, 
       game_assets* Assets,
       render_commands * RenderCommands, 
       f32 DeltaTime) 
{
#if INTERNAL
    DrawCollisionDebugLinesSystem(&Entity->Transform, &Entity->Collision, RenderCommands);
#endif
    SpriteRenderingSystem(&Entity->Transform, 
                          &Entity->Sprite, 
                          RenderCommands, 
                          Assets,
                          DeltaTime);
}


static inline void
Update(sandbox_block* Entity, 
       render_commands * RenderCommands) 
{
    
#if INTERNAL
    DrawCollisionDebugLinesSystem(&Entity->Transform, &Entity->Collision, RenderCommands);
#endif
}

struct game_mode_sandbox {
    static constexpr u8 TypeId = 2;
    
    u8 AnimeWalkDown[4];
    u8 AnimeWalkLeft[4];
    u8 AnimeWalkRight[4];
    u8 AnimeWalkUp[4];
    
    u8 AnimeIdleDown[1];
    u8 AnimeIdleUp[1];
    u8 AnimeIdleLeft[1];
    u8 AnimeIdleRight[1];
    
    
    sandbox_player Player;
    sandbox_block Block;
};


static inline void
InitMode(game_mode_sandbox* Mode, game_state* GameState) {
    
    // NOTE(Momo): Animations
    {
        Mode->AnimeWalkUp[0] = 1; 
        Mode->AnimeWalkUp[1] = 0;
        Mode->AnimeWalkUp[2] = 1;
        Mode->AnimeWalkUp[3] = 2;
        
        Mode->AnimeWalkRight[0] = 4; 
        Mode->AnimeWalkRight[1] = 3;
        Mode->AnimeWalkRight[2] = 4;
        Mode->AnimeWalkRight[3] = 5;
        
        Mode->AnimeWalkLeft[0] = 7; 
        Mode->AnimeWalkLeft[1] = 6;
        Mode->AnimeWalkLeft[2] = 7;
        Mode->AnimeWalkLeft[3] = 8;
        
        Mode->AnimeWalkDown[0] = 10; 
        Mode->AnimeWalkDown[1] = 9;
        Mode->AnimeWalkDown[2] = 10;
        Mode->AnimeWalkDown[3] = 11;
        
        Mode->AnimeIdleDown[0] = 10;
        Mode->AnimeIdleLeft[0] = 7;
        Mode->AnimeIdleRight[0] = 4;
        Mode->AnimeIdleUp[0] = 1;
    }
    
    
    // NOTE(Momo): Init player
    {
        auto* Player = &Mode->Player;
        
        Player->Transform.Position = {};
        Player->Transform.Rotation = 0.f;
        Player->Transform.Scale = { 48.f, 48.f, 0.f };
        
        Player->Physics = {};
        Player->Collision = {};
        Player->Collision.Box.Origin = { 0.f, 0.f, 0.f };
        Player->Collision.Box.HalfDimensions = { 24.f, 24.f, 0.f };
        
        
        Player->Sprite.SpritesheetHandle = GameSpritesheetHandle_Karu;
        Player->Sprite.AnimeFrames = Mode->AnimeWalkDown;
        Player->Sprite.AnimeFramesCount = ArrayCount(Mode->AnimeWalkDown);
        Player->Sprite.AnimeAt = 0;
        Player->Sprite.AnimeTimer = 0.f;
        Player->Sprite.AnimeSpeed = 10.f;
        Player->MovementSpeed = 400.f;
        Player->CurrentDirection = DOWN;
    }
    
    // NOTE(Momo): Init block
    {
        auto* Block = &Mode->Block;
        
        Block->Transform.Position = { 200.f, 200.f };
        Block->Transform.Rotation = 0.f;
        Block->Transform.Scale = { 48.f, 48.f };
        
        Block->Collision = {};
        Block->Collision.Box.Origin = {};
        Block->Collision.Box.HalfDimensions = { 240.f, 240.f, 0.f };
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
    
    // TODO(Momo): Player controls in a system
    auto* Player = &Mode->Player;
    {
        v3f Direction = {};
        b8 IsMovementButtonDown = false;
        if(IsDown(Input->ButtonLeft)) {
            Direction.X = -1.f;
            Player->Sprite.AnimeFrames = Mode->AnimeWalkLeft;
            Player->Sprite.AnimeFramesCount = ArrayCount(Mode->AnimeWalkLeft);
            Player->CurrentDirection = LEFT;
            IsMovementButtonDown = true;
        };
        
        if(IsDown(Input->ButtonRight)) {
            Direction.X = 1.f;
            Player->Sprite.AnimeFrames = Mode->AnimeWalkRight;
            Player->Sprite.AnimeFramesCount = ArrayCount(Mode->AnimeWalkRight);
            Player->CurrentDirection = RIGHT;
            IsMovementButtonDown = true;
            
        }
        
        if(IsDown(Input->ButtonUp)) {
            Direction.Y = 1.f;
            Player->Sprite.AnimeFrames = Mode->AnimeWalkUp;
            Player->Sprite.AnimeFramesCount = ArrayCount(Mode->AnimeWalkUp);
            Player->CurrentDirection = UP;
            IsMovementButtonDown = true;
            
        }
        if(IsDown(Input->ButtonDown)) {
            Direction.Y = -1.f;
            Player->Sprite.AnimeFrames = Mode->AnimeWalkDown;
            Player->Sprite.AnimeFramesCount = ArrayCount(Mode->AnimeWalkDown);
            Player->CurrentDirection = DOWN;
            IsMovementButtonDown = true;
        }
        
        
        if (IsMovementButtonDown) 
            Player->Physics.Velocity = Normalize(Direction) * Player->MovementSpeed;
        else {
            Player->Physics.Velocity = {};
            // TODO(Momo): Idle animation?????
            switch(Player->CurrentDirection) {
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
    }
    
    // NOTE(Momo): Entity updates
    Update(Player, DeltaTime);
    
    // TODO(Momo): Sledgehammered collision
    {
        auto Lhs = Mode->Block.Collision.Box;
        auto Rhs = Mode->Player.Collision.Box;
        
        Lhs.Origin += Mode->Block.Transform.Position;
        Rhs.Origin += Mode->Player.Transform.Position;
        
        
        b8 IsCollided = true;
        
        v3f PushbackVector = {}; 
        for(u8 i = 0; i < 3; ++i) 
        {
            f32 OriginDisplacement = Lhs.Origin[i] - Rhs.Origin[i];
            f32 OriginDistance = Abs(OriginDisplacement);
            f32 HalfRadiusSum = Lhs.HalfDimensions[i] + Rhs.HalfDimensions[i];
            if (OriginDistance > HalfRadiusSum) {
                IsCollided = false;
                break;
            }
            
            f32 PushbackScalar = OriginDistance - HalfRadiusSum;
            f32 NormalizedOriginDisplacement = OriginDisplacement / OriginDistance;
            PushbackVector[i] = NormalizedOriginDisplacement * PushbackScalar;
        }
        
        if (IsCollided) {
            if ( Abs(PushbackVector.X) < Abs(PushbackVector.Y)) 
                Mode->Player.Transform.Position.X += PushbackVector.X ;
            else 
                Mode->Player.Transform.Position.Y += PushbackVector.Y ;
        }
        
    }
    
    
    // NOTE(Momo): Rendering 
    PushCommandClear(RenderCommands, { 0.5f, 0.5f, 0.5f, 0.f });
    Render(Player, GameState->Assets, RenderCommands, DeltaTime);
    
    Update(&Mode->Block, 
           RenderCommands);
    
    
}
#endif //GAME_MODE_SANDBOX_H
