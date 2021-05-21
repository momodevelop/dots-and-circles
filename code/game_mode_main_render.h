/* date = May 9th 2021 5:43 pm */

#ifndef GAME_MODE_MAIN_RENDER_H
#define GAME_MODE_MAIN_RENDER_H



static inline void 
RenderPlayer(game_mode_main* Mode,
             assets* Assets,
             mailbox* RenderCommands) 
{
    player* Player = &Mode->Player;
    MM_M44f S = MM_M44f_Scale(Player->Size.x, Player->Size.y, 1.f);
    
    {
        MM_M44f T = MM_M44f_Translation(Player->Position.x,
                                  Player->Position.y,
                                  ZLayPlayer);
        c4f Color = C4f_Create(1.f, 1.f, 1.f, 1.f - Player->DotImageAlpha);
        
        Draw_TexturedQuadFromImage(RenderCommands,
                                   Assets,
                                   Image_PlayerCircle,
                                   MM_M44f_Concat(T,S), 
                                   Color);
    }
    
    {
        MM_M44f T = MM_M44f_Translation(Player->Position.x,
                                  Player->Position.y,
                                  ZLayPlayer + 0.01f);
        c4f Color = C4f_Create(1.f, 1.f, 1.f, Player->DotImageAlpha);
        Draw_TexturedQuadFromImage(RenderCommands,
                                   Assets,
                                   Image_PlayerDot,
                                   MM_M44f_Concat(T,S), 
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
        for (u32 I = 0; I < Mode->DotBullets.count; ++I) {
            bullet* DotBullet = MM_List_Get(&Mode->DotBullets, I);
            MM_M44f S = MM_M44f_Scale(DotBullet->Size.x, 
                                DotBullet->Size.y, 
                                1.f);
            
            MM_M44f T = MM_M44f_Translation(DotBullet->Position.x,
                                      DotBullet->Position.y,
                                      ZLayDotBullet + LayerOffset);
            
            Draw_TexturedQuadFromImage(RenderCommands,
                                       Assets,
                                       Image_BulletDot,
                                       MM_M44f_Concat(T,S), 
                                       Color_White);
            LayerOffset += 0.01f;
            
        }
        
    }
    
    // Render Circles
    {
        f32 LayerOffset = 0.f;
        for (u32 I = 0; I < Mode->CircleBullets.count; ++I) {
            bullet* CircleBullet = MM_List_Get(&Mode->CircleBullets, I);
            MM_M44f S = MM_M44f_Scale(CircleBullet->Size.x, 
                                CircleBullet->Size.y, 
                                1.f);
            
            MM_M44f T = MM_M44f_Translation(CircleBullet->Position.x,
                                      CircleBullet->Position.y,
                                      ZLayCircleBullet + LayerOffset);
            
            Draw_TexturedQuadFromImage(RenderCommands,
                                       Assets,
                                       Image_BulletCircle,
                                       MM_M44f_Concat(T,S), 
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
    for(u32 I = 0; I < Mode->Enemies.count; ++I )
    {
        enemy* Enemy = Mode->Enemies + I;
        MM_M44f S = MM_M44f_Scale(Enemy->Size.x, Enemy->Size.y, 1.f);
        MM_M44f T = MM_M44f_Translation(Enemy->Position.x,
                                  Enemy->Position.y,
                                  ZLayEnemy);
        
        Draw_TexturedQuadFromImage(RenderCommands,
                                   Assets,
                                   Image_Enemy,
                                   MM_M44f_Concat(T,S), 
                                   Color_White);
    }
}

static inline void 
RenderDebugLines(game_mode_main* Mode, mailbox* RenderCommands){
    MM_Circle2f Circle = {};
    Circle.origin = Mode->Player.Position;
    Circle.radius = Mode->Player.HitCircle.radius;
    Renderer_DrawCircle2f(RenderCommands, Circle, 1.f, 8, Color_Green, ZLayDebug);
}

static inline void
RenderParticlesSub(MM_Queue<particle>* Q, 
                   assets* Assets,
                   mailbox* RenderCommands,
                   u32 Begin, u32 End) 
{
    
    for (u32 I = 0; I <= End; ++I ) {
        particle* P = MM_Queue_Get(Q, I);
        Assert(P);
        
        MM_M44f S = MM_M44f_Scale(1.f, 1.f, 1.f);
        MM_M44f T = MM_M44f_Translation(P->Position.x,
                                  P->Position.y,
                                  ZLayParticles);
        
        Draw_TexturedQuadFromImage(RenderCommands,
                                   Assets,
                                   Image_BulletDot,
                                   MM_M44f_Concat(T,S));
        
    }
}

static inline void
RenderParticles(game_mode_main* Mode, 
                assets* Assets, 
                mailbox* RenderCommands)

{
    MM_Queue<particle>* Q = &Mode->Particles;
    if (MM_Queue_IsEmpty(Q)) {
        return;
    }
    
    // Then update the living ones
    if (Q->begin <= Q->end) {
        RenderParticlesSub(Q, Assets, RenderCommands, Q->begin, Q->end);
    }
    else {
        RenderParticlesSub(Q, Assets, RenderCommands, Q->begin, Q->count - 1);
        RenderParticlesSub(Q, Assets, RenderCommands, 0, Q->end);
    }
    
}


#endif //GAME_MODE_MAIN_RENDER_H
