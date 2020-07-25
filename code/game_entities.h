
#ifndef GAME_ENTITIES_H
#define GAME_ENTITIES_H

#include "ryoji_easing.h"
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
    m44f T = MakeTranslationMatrix(Entity->Position);
    m44f R = MakeRotationZMatrix(Entity->Rotation);
    m44f S = MakeScaleMatrix(Entity->Scale);
    
    // TODO(Momo): This part should be done by renderer?
    PushCommandTexturedQuad(RenderCommands, 
                            Entity->Colors, 
                            T*R*S,
                            Assets->Textures[Entity->TextureHandle]);
}

struct splash_image_entity {
    v3f Scale;
    v3f Position;
    c4f Colors;
    u32 TextureHandle;
    
    f32 CountdownTimer;
    f32 CountdownDuration;
    
    f32 Timer;
    f32 Duration;
    f32 StartX;
    f32 EndX;
};

static inline void
Update(splash_image_entity* Entity, 
       game_assets* Assets,
       render_commands * RenderCommands, 
       f32 DeltaTime) {
    Entity->CountdownTimer += DeltaTime;
    if (Entity->CountdownTimer <= Entity->CountdownDuration) 
        return;
    
    // NOTE(Momo): Update
    f32 ease = EaseOutBounce(Clamp(Entity->Timer/Entity->Duration, 0.f, 1.f));
    
    Entity->Position.X = Entity->StartX + (Entity->EndX - Entity->StartX) * ease; 
    Entity->Timer += DeltaTime;
    
    // NOTE(Momo): Render
    m44f T = MakeTranslationMatrix(Entity->Position);
    m44f S = MakeScaleMatrix(Entity->Scale);
    
    // TODO(Momo): This part should be done by renderer?
    PushCommandTexturedQuad(RenderCommands, 
                            Entity->Colors, 
                            T*S,
                            Assets->Textures[Entity->TextureHandle]);
}

struct splash_blackout_entity {
    v3f Scale;
    v3f Position;
    c4f Colors;
    u32 TextureHandle;
    
    f32 CountdownTimer;
    f32 CountdownDuration;
    
    f32 Timer;
    f32 Duration;
};

static inline void
Update(splash_blackout_entity* Entity, 
       game_assets* Assets, 
       render_commands* RenderCommands,
       f32 DeltaTime) 
{
    Entity->CountdownTimer += DeltaTime;
    if (Entity->CountdownTimer <= Entity->CountdownDuration) 
        return;
    
    Entity->Colors.A = EaseInSine(Clamp(Entity->Timer/Entity->Duration, 0.f, 1.f));
    Entity->Timer += DeltaTime;
    
    // NOTE(Momo): Render
    m44f T = MakeTranslationMatrix(Entity->Position);
    m44f S = MakeScaleMatrix(Entity->Scale);
    
    // TODO(Momo): This part should be done by renderer?
    PushCommandTexturedQuad(RenderCommands, 
                            Entity->Colors, 
                            T*S,
                            Assets->Textures[Entity->TextureHandle]);
    
}





#endif //GAME_ENTITIES_H
