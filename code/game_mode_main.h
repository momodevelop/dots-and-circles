#ifndef GAME_MODE_MAIN_H
#define GAME_MODE_MAIN_H

#include "ryoji_easing.h"
#include "ryoji_maths.h"
#include "game.h"
#include "game_assets.h"


v3f CameraPosition;
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
struct transform_component {
    v3f Scale;
    f32 Rotation;
    v3f Position;
};

struct collision_component {
    aabb2f Box;
};

struct renderable_component {
    v4f Colors;
    u32 TextureHandle;
    quad2f TextureCoords;
};


struct physics_component {
    v3f Velocity;
    v3f Acceleration;
};

struct sprite_component {
    spritesheet_id SpritesheetHandle;
    
    // NOTE(Momo): Animation Data?
    u8* AnimeFrames;
    u8 AnimeFramesCount;
    u8 AnimeAt;
    f32 AnimeTimer;
    f32 AnimeSpeed;
};

struct player {
    transform_component Transform;
    physics_component Physics;
    sprite_component Sprite;
    collision_component Collision;
    
    Direction CurrentDirection;
    f32 MovementSpeed;
};

struct block {
    transform_component Transform;
    collision_component Collision;
};


struct game_mode_main {
    static constexpr u8 TypeId = 2;
    
    u8 AnimeWalkDown[4];
    u8 AnimeWalkLeft[4];
    u8 AnimeWalkRight[4];
    u8 AnimeWalkUp[4];
    
    u8 AnimeIdleDown[1];
    u8 AnimeIdleUp[1];
    u8 AnimeIdleLeft[1];
    u8 AnimeIdleRight[1];
    
    
    player Player;
    union {
        struct {
            block BottomBlock[10];
            block TopBlock[10];
            block LeftBlock[5];
            block RightBlock[5];
        };
        block Blocks[30];
    };
    
    b8 IsControllingCamera;
};



static inline void
SpriteRenderingSystem(game_mode_main* Mode, 
                      transform_component* Transform,
                      sprite_component* Sprite,
                      commands* RenderCommands,
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
    
    auto CurrentFrame = Sprite->AnimeFrames[Sprite->AnimeAt];
    
    PushCommandDrawTexturedQuad(RenderCommands, 
                                {1.f, 1.f, 1.f, 1.f}, 
                                T*R*S,
                                (u32)Sprite->SpritesheetHandle,
                                Quad2(GetSpritesheetFrame(Assets, Sprite->SpritesheetHandle, CurrentFrame)));
    
}

#if INTERNAL
static inline void
DrawCollisionLinesSystem(transform_component* Transform,
                         collision_component* Collision, 
                         commands* RenderCommands) 
{
    
    auto* Box = &Collision->Box;
    
    rect2f Rect = {}; 
    Rect.Min.X = Transform->Position.X + Box->Origin.X - Box->Radius.X;
    Rect.Min.Y = Transform->Position.Y + Box->Origin.Y - Box->Radius.Y;
    Rect.Max.X = Transform->Position.X + Box->Origin.X + Box->Radius.X;
    Rect.Max.Y = Transform->Position.Y + Box->Origin.Y + Box->Radius.Y;
    
    // Bottom
    PushCommandDrawLineRect(RenderCommands, Rect);
}
#endif

static inline void
PhysicsSystem(transform_component* Transform,
              physics_component* Physics,
              f32 DeltaTime) 

{
    Physics->Velocity += Physics->Acceleration * DeltaTime;
    Transform->Position += Physics->Velocity * DeltaTime;
    Physics->Acceleration = {};
}

static inline void
Update(player* Entity, f32 DeltaTime) 
{
    PhysicsSystem(&Entity->Transform, &Entity->Physics, DeltaTime);
}

static inline void 
Render(game_mode_main* Mode,
       player* Entity, 
       game_assets* Assets,
       commands* RenderCommands, 
       f32 DeltaTime) 
{
#if INTERNAL
    DrawCollisionLinesSystem(&Entity->Transform, &Entity->Collision, RenderCommands);
#endif
    SpriteRenderingSystem(Mode,
                          &Entity->Transform, 
                          &Entity->Sprite, 
                          RenderCommands, 
                          Assets,
                          DeltaTime);
}


static inline void
Render(block* Entity, 
       commands * RenderCommands) 
{
    
#if INTERNAL
    DrawCollisionLinesSystem(&Entity->Transform, &Entity->Collision, RenderCommands);
#endif
}

static inline void
InitMode(game_mode_main* Mode, game_state* GameState) {
    
    // NOTE(Momo): Camera 
    {
        CameraPosition = {};
    }
    
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
        
        Player->Transform.Position = { -800, -800 };
        Player->Transform.Rotation = 0.f;
        Player->Transform.Scale = { 48.f*2, 48.f*2, 0.f };
        
        Player->Physics = {};
        Player->Collision = {};
        Player->Collision.Box.Origin = { 0.f, 0.f };
        Player->Collision.Box.Radius = { 24.f*2, 24.f*2 };
        
        Player->Sprite.SpritesheetHandle = asset_id::Spritesheet_Karu;
        Player->Sprite.AnimeFrames = Mode->AnimeWalkDown;
        Player->Sprite.AnimeFramesCount = ArrayCount(Mode->AnimeWalkDown);
        Player->Sprite.AnimeAt = 0;
        Player->Sprite.AnimeTimer = 0.f;
        Player->Sprite.AnimeSpeed = 10.0f;
        Player->MovementSpeed = 400.f;
        Player->CurrentDirection = DOWN;
    }
    
    // NOTE(Momo): Init blocks
    {
        v2f BlockHalfDimensions = { 48.f, 48.f };
        v3f StartPt = { -420.f, -280.f, 0.f };
        
        // bottom line
        for(u8 i = 0; i < ArrayCount(Mode->BottomBlock); ++i) {
            auto* Block =  &Mode->BottomBlock[i];
            
            v3f Offset = { BlockHalfDimensions.X * 2.f, 0.f, 0.f };
            Block->Transform.Position = StartPt + Offset * i;
            Block->Transform.Rotation = 0.f;
            Block->Transform.Scale = { 48.f, 48.f };
            
            Block->Collision = {};
            Block->Collision.Box.Origin = {};
            Block->Collision.Box.Radius = BlockHalfDimensions;
        }
        
        // top line
        for(u8 i = 0; i < ArrayCount(Mode->TopBlock); ++i) {
            auto* Block =  &Mode->TopBlock[i];
            v3f Offset = { BlockHalfDimensions.X * 2.f, 0.f , 0.f };
            Block->Transform.Position = StartPt + Offset * i;
            
            Block->Transform.Position.Y +=  BlockHalfDimensions.Y * 2.f * (ArrayCount(Mode->LeftBlock) + 1), 0.f;
            
            Block->Transform.Rotation = 0.f;
            Block->Transform.Scale = { 48.f, 48.f };
            
            Block->Collision = {};
            Block->Collision.Box.Origin = {};
            Block->Collision.Box.Radius = BlockHalfDimensions;
        }
        
        // left line
        for(u8 i = 0; i < ArrayCount(Mode->LeftBlock); ++i) {
            auto* Block =  &Mode->LeftBlock[i];
            
            v3f Offset = { 0.f,  BlockHalfDimensions.Y * 2.f, 0.f };
            Block->Transform.Position = StartPt + Offset * i;
            Block->Transform.Rotation = 0.f;
            Block->Transform.Scale = { 48.f, 48.f };
            
            Block->Transform.Position.Y +=  BlockHalfDimensions.Y * 2.f;
            Block->Collision = {};
            Block->Collision.Box.Origin = {};
            Block->Collision.Box.Radius = BlockHalfDimensions;
        }
        
        // right line
        for(u8 i = 0; i < ArrayCount(Mode->RightBlock); ++i) {
            auto* Block =  &Mode->RightBlock[i];
            
            v3f Offset = { 0.f,  BlockHalfDimensions.Y * 2.f, 0.f };
            Block->Transform.Position = StartPt + Offset * i;
            Block->Transform.Rotation = 0.f;
            Block->Transform.Scale = { 48.f, 48.f };
            
            Block->Transform.Position.X +=  BlockHalfDimensions.X * 2.f * (ArrayCount(Mode->TopBlock) - 1), 0.f;
            
            
            Block->Transform.Position.Y +=  BlockHalfDimensions.Y * 2.f;
            Block->Collision = {};
            Block->Collision.Box.Origin = {};
            Block->Collision.Box.Radius = BlockHalfDimensions;
        }
    }
    Log("Sandbox state initialized!");
}


static inline bool
IsColliding(aabb2f Lhs, aabb2f Rhs) {
    for(u8 i = 0; i < 2; ++i) 
    {
        f32 OriginDisplacement = Lhs.Origin[i] - Rhs.Origin[i];
        f32 OriginDistance = Abs(OriginDisplacement);
        f32 HalfRadiusSum = Lhs.Radius[i] + Rhs.Radius[i];
        if (OriginDistance > HalfRadiusSum) {
            return false;
        }
    }
    return true;
}

static inline v2f 
GetPushbackNormal(aabb2f Static, aabb2f Dynamic ) {
    v3f PushbackVector = {}; 
    for(u8 i = 0; i < 2; ++i) 
    {
        f32 OriginDisplacement = Static.Origin[i] - Dynamic.Origin[i];
        f32 OriginDistance = Abs(OriginDisplacement);
        f32 HalfRadiusSum = Static.Radius[i] + Dynamic.Radius[i];
        if (OriginDistance > HalfRadiusSum) {
            
            return {};
        }
        
        f32 PushbackScalar = OriginDistance - HalfRadiusSum;
        if (OriginDistance > 0.f) { 
            f32 NormalizedOriginDisplacement = OriginDisplacement / OriginDistance;
            PushbackVector[i] = NormalizedOriginDisplacement * PushbackScalar;
        } 
        else {
            PushbackVector[i] = PushbackScalar;
        }
    }
    
    if ( Abs(PushbackVector.X) < Abs(PushbackVector.Y)) 
        return { PushbackVector.X, 0.f }; 
    else 
        return { 0.f, PushbackVector.Y };
}


static inline void
UpdateMode(game_mode_main* Mode,
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
    // TODO(Momo): Player controls in a system
    auto* Player = &Mode->Player;
    
    
    if(IsPoked(Input->ButtonConfirm)) {
        Mode->IsControllingCamera = !Mode->IsControllingCamera;
        if ( Mode->IsControllingCamera ) { 
            Log("Now controlling camera");
        }
        else { 
            Log("Now controlling player");
        }
    }
    
    if (!Mode->IsControllingCamera)
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
    
    else {
        // NOTE(Momo): Controlling camera
        f32 CameraSpeed = 500.f;
        if(IsDown(Input->ButtonLeft)) {
            CameraPosition.X -= CameraSpeed * DeltaTime;;
        };
        
        if(IsDown(Input->ButtonRight)) {
            CameraPosition.X += CameraSpeed * DeltaTime;;
        }
        
        if(IsDown(Input->ButtonUp)) {
            CameraPosition.Y += CameraSpeed * DeltaTime;;
        }
        if(IsDown(Input->ButtonDown)) {
            CameraPosition.Y -= CameraSpeed * DeltaTime;;
        }
        
        if(IsDown(Input->ButtonDebug[5])) {
            CameraPosition.Z += CameraSpeed * DeltaTime;;
        }
        if(IsDown(Input->ButtonDebug[6])) {
            CameraPosition.Z -= CameraSpeed * DeltaTime;;
        }
    }
    
    // NOTE(Momo): Entity updates
    Update(Player, DeltaTime);
    
    PushCommandClearColor(RenderCommands, { 0.3f, 0.3f, 0.3f, 1.f });
    PushCommandSetOrthoBasis(RenderCommands, CameraPosition, { 1600.f, 900.f, 200.f });
    
    
#define STATE 0
#if STATE
    // TODO(Momo): Sledgehammered collision
    for (u8 i = 0; i < ArrayCount(Mode->Blocks); ++i )
    {
        auto Lhs = Mode->Blocks[i].Collision.Box;
        auto Rhs = Mode->Player.Collision.Box;
        Lhs.Origin += V2F(Mode->Blocks[i].Transform.Position);
        Rhs.Origin += V2F(Mode->Player.Transform.Position);
        if (IsColliding(Lhs, Rhs)) {
            auto Pushback = GetPushbackNormal(Lhs, Rhs);
            Mode->Player.Transform.Position += V3F(Pushback);
            Log("Pushback %f %f", Pushback.X, Pushback.Y);
        }
    }
    
    
#else
    // TODO(Momo): Sledgehammered line collision
    {
        line2f Line = {};
        Line.Min = { -400.f, 0.f };
        Line.Max = { 0.f, 400.f };
        ray2f Ray1 = Ray2F(Line);
        ray2f Ray2F = {};
        Ray2F.Origin = V2F(Player->Transform.Position) + Player->Collision.Box.Origin;
        Ray2F.Direction = { -Player->Collision.Box.Radius.W, Player->Collision.Box.Radius.H };
        
        auto [t1, t2] = GetIntersectionTime(Ray1, Ray2F);
        if (t2 >= 0.f && t2 <= 1.f && t1 >= 0.f && t1 <= 1.f) {
            v2f IntersectionPt = GetPoint(Ray2F, t2);
            Log("%f, %f", IntersectionPt.X, IntersectionPt.Y);
            v2f cornerPt = GetPoint(Ray2F, 1.f);
            v2f cornerToIntersectionVec = IntersectionPt - cornerPt;
            Mode->Player.Transform.Position += V3F(cornerToIntersectionVec) ;
        }
        PushCommandDrawLine(RenderCommands, Line);
    }
    
    {
        line2f Line = {};
        Line.Max = { 0.f, 400.f };
        Line.Min = { 400.f, 0.f };
        // line pov
        v2f p1;
        v2f v1; 
        f32 t1;
        
        // object's pov
        v2f p2;
        v2f v2;
        f32 t2;
        
        p1 = Line.Min; 
        v1 = Line.Max - Line.Min;
        
        p2 = V2F(Player->Transform.Position) + Player->Collision.Box.Origin;
        v2 = { Player->Collision.Box.Radius.W, Player->Collision.Box.Radius.H };
        
        t2 = (v1.X*p2.Y - v1.X*p1.Y - v1.Y*p2.X + v1.Y*p1.X)/(v1.Y*v2.X - v1.X*v2.Y);
        
        t1 = (p2.X + t2*v2.X - p1.X)/v1.X;
        
        if (t2 >= 0.f && t2 <= 1.f && t1 >= 0.f && t1 <= 1.f) {
            v2f IntersectionPt = p2 + v2*t2;
            //Log("%f", t2);
            Log("%f, %f", IntersectionPt.X, IntersectionPt.Y);
            
            v2f cornerPt = p2 + v2;
            v2f cornerToIntersectionVec = IntersectionPt - cornerPt;
            Mode->Player.Transform.Position += V3F(cornerToIntersectionVec) ;
        }
        PushCommandDrawLine(RenderCommands, Line);
    }
    
    {
        line2f Line = {};
        Line.Max = { 400.f, 0.f };
        Line.Min = { 0.f, -400.f };
        
        // line pov
        v2f p1;
        v2f v1; 
        f32 t1;
        
        // object's pov
        v2f p2;
        v2f v2;
        f32 t2;
        
        p1 = Line.Min; 
        v1 = Line.Max - Line.Min;
        
        p2 = V2F(Player->Transform.Position) + Player->Collision.Box.Origin;
        v2 = { Player->Collision.Box.Radius.W, -Player->Collision.Box.Radius.H };
        
        t2 = (v1.X*p2.Y - v1.X*p1.Y - v1.Y*p2.X + v1.Y*p1.X)/(v1.Y*v2.X - v1.X*v2.Y);
        
        t1 = (p2.X + t2*v2.X - p1.X)/v1.X;
        
        if (t2 >= 0.f && t2 <= 1.f && t1 >= 0.f && t1 <= 1.f) {
            v2f IntersectionPt = p2 + v2*t2;
            Log("%f, %f", IntersectionPt.X, IntersectionPt.Y);
            
            v2f cornerPt = p2 + v2;
            v2f cornerToIntersectionVec = IntersectionPt - cornerPt;
            Mode->Player.Transform.Position += V3F(cornerToIntersectionVec) ;
        }
        PushCommandDrawLine(RenderCommands, Line);
    }
    
    {
        line2f Line = {};
        Line.Max = { -400.f, 0.f };
        Line.Min = { 0.f, -400.f };
        // line pov
        v2f p1;
        v2f v1; 
        f32 t1;
        
        // object's pov
        v2f p2;
        v2f v2;
        f32 t2;
        
        p1 = Line.Min; 
        v1 = Line.Max - Line.Min;
        
        p2 = V2F(Player->Transform.Position) + Player->Collision.Box.Origin;
        v2 = { -Player->Collision.Box.Radius.W, -Player->Collision.Box.Radius.H };
        
        t2 = (v1.X*p2.Y - v1.X*p1.Y - v1.Y*p2.X + v1.Y*p1.X)/(v1.Y*v2.X - v1.X*v2.Y);
        
        t1 = (p2.X + t2*v2.X - p1.X)/v1.X;
        
        if (t2 >= 0.f && t2 <= 1.f && t1 >= 0.f && t1 <= 1.f) {
            v2f IntersectionPt = p2 + v2*t2;
            Log("%f, %f", IntersectionPt.X, IntersectionPt.Y);
            
            v2f cornerPt = p2 + v2;
            v2f cornerToIntersectionVec = IntersectionPt - cornerPt;
            Mode->Player.Transform.Position += V3F(cornerToIntersectionVec) ;
        }
        PushCommandDrawLine(RenderCommands, Line);
    }
#endif
    
    
    Render(Mode, Player, GameState->Assets, RenderCommands, DeltaTime);
#if STATE
    for ( u8 i = 0; i < ArrayCount(Mode->Blocks); ++i ) {
        Render(&Mode->Blocks[i], RenderCommands);
    }
#endif
    
    
    
}

#endif //GAME_MODE_H
