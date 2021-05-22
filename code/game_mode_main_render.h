/* date = May 9th 2021 5:43 pm */

#ifndef GAME_MODE_MAIN_RENDER_H
#define GAME_MODE_MAIN_RENDER_H



static inline void 
RenderPlayer(game_mode_main* Mode,
             assets* Assets,
             mailbox* RenderCommands) 
{
    player* Player = &Mode->Player;
    m44f S = M44f_Scale(Player->Size.X, Player->Size.Y, 1.f);
    
    {
        m44f T = M44f_Translation(Player->Position.X,
                                  Player->Position.Y,
                                  ZLayPlayer);
        c4f Color = C4f_Create(1.f, 1.f, 1.f, 1.f - Player->DotImageAlpha);
        
        Draw_TexturedQuadFromImage(RenderCommands,
                                   Assets,
                                   Image_PlayerCircle,
                                   M44f_Concat(T,S), 
                                   Color);
    }
    
    {
        m44f T = M44f_Translation(Player->Position.X,
                                  Player->Position.Y,
                                  ZLayPlayer + 0.01f);
        c4f Color = C4f_Create(1.f, 1.f, 1.f, Player->DotImageAlpha);
        Draw_TexturedQuadFromImage(RenderCommands,
                                   Assets,
                                   Image_PlayerDot,
                                   M44f_Concat(T,S), 
                                   Color);
    }
}

static inline void
RenderBullets(game_mode_main* Mode,
              assets* Assets,
              mailbox* RenderCommands) 
{
    // Bullet Rendering.
    // NOTE(Momo): Circles are in front of Dots and are therefore 'nearer'.
    // Thus we have to render Dots first before Circles.
    
    // Render Dots
    {
        f32 LayerOffset = 0.f;
        image* Image = Assets->Images + Image_BulletDot;
        texture* Texture = Assets->Textures + Image->TextureId;
        for (u32 I = 0; I < Mode->DotBullets.Count; ++I) {
            bullet* DotBullet = Mode->DotBullets.Data + I;
            m44f S = M44f_Scale(DotBullet->Size.X, 
                                DotBullet->Size.Y, 
                                1.f);
            
            m44f T = M44f_Translation(DotBullet->Position.X,
                                      DotBullet->Position.Y,
                                      ZLayDotBullet + LayerOffset);
            
            Draw_TexturedQuadFromImage(RenderCommands,
                                       Assets,
                                       Image_BulletDot,
                                       M44f_Concat(T,S), 
                                       Color_White);
            LayerOffset += 0.01f;
            
        }
        
    }
    
    // Render Circles
    {
        f32 LayerOffset = 0.f;
        for (u32 I = 0; I < Mode->CircleBullets.Count; ++I) {
            bullet* CircleBullet = Mode->CircleBullets.Data + I;
            m44f S = M44f_Scale(CircleBullet->Size.X, 
                                CircleBullet->Size.Y, 
                                1.f);
            
            m44f T = M44f_Translation(CircleBullet->Position.X,
                                      CircleBullet->Position.Y,
                                      ZLayCircleBullet + LayerOffset);
            
            Draw_TexturedQuadFromImage(RenderCommands,
                                       Assets,
                                       Image_BulletCircle,
                                       M44f_Concat(T,S), 
                                       Color_White);
            LayerOffset += 0.01f;
            
        }
        
    }
    
    
    
}

static inline void
RenderEnemies(game_mode_main* Mode, 
              assets* Assets,
              mailbox* RenderCommands) 
{
    for(u32 I = 0; I < Mode->Enemies.Count; ++I )
    {
        enemy* Enemy = Mode->Enemies + I;
        m44f S = M44f_Scale(Enemy->Size.X, Enemy->Size.Y, 1.f);
        m44f T = M44f_Translation(Enemy->Position.X,
                                  Enemy->Position.Y,
                                  ZLayEnemy);
        
        Draw_TexturedQuadFromImage(RenderCommands,
                                   Assets,
                                   Image_Enemy,
                                   M44f_Concat(T,S), 
                                   Color_White);
    }
}

static inline void 
RenderDebugLines(game_mode_main* Mode, mailbox* RenderCommands){
    circle2f Circle = {};
    Circle.Origin = Mode->Player.Position;
    Circle.Radius = Mode->Player.HitCircle.Radius;
    Renderer_DrawCircle2f(RenderCommands, Circle, 1.f, 8, Color_Green, ZLayDebug);
}

static inline void
RenderParticlesSub(queue<particle>* Q, 
                   assets* Assets,
                   mailbox* RenderCommands,
                   u32 Begin, 
                   u32 End, 
                   u32* CurrentCount) 
{
    for (u32 I = Begin; I <= End; ++I ) {
        
        particle* P = Queue_Get(Q, I);
        if (!P) {
            return;
        }
        f32 Ease = 1.f - (P->Timer/P->Duration);
        f32 Alpha = P->Alpha * Ease;
        f32 Size = P->Size * Ease;
        
        f32 Offset = (*CurrentCount)*0.001f;
        
        m44f S = M44f_Scale(Size, Size, 1.f);
        m44f T = M44f_Translation(P->Position.X,
                                  P->Position.Y,
                                  ZLayParticles + Offset);
        
        Draw_TexturedQuadFromImage(RenderCommands,
                                   Assets,
                                   Image_Particle,
                                   M44f_Concat(T,S),
                                   C4f_Create(1.f, 1.f, 1.f, Alpha));
        ++(*CurrentCount);
        
    }
}

static inline void
RenderParticles(game_mode_main* Mode, 
                assets* Assets, 
                mailbox* RenderCommands)

{
    queue<particle>* Q = &Mode->Particles;
    if (Queue_IsEmpty(Q)) {
        return;
    }
    
    // Then update the living ones
    u32 CurrentCount = 0;
    if (Q->Begin <= Q->End) {
        RenderParticlesSub(Q, Assets, RenderCommands, Q->Begin, Q->End, &CurrentCount);
    }
    else {
        RenderParticlesSub(Q, Assets, RenderCommands, Q->Begin, Q->Cap - 1, &CurrentCount);
        RenderParticlesSub(Q, Assets, RenderCommands, 0, Q->End, &CurrentCount);
    }
    
}


#endif //GAME_MODE_MAIN_RENDER_H
