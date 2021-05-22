/* date = May 22nd 2021 7:11 pm */

#ifndef GAME_MODE_MAIN_BULLET_H
#define GAME_MODE_MAIN_BULLET_H

static inline void
SpawnBullet(game_mode_main* Mode, 
            assets* Assets, 
            v2f Position, 
            v2f Direction, 
            f32 Speed, 
            mood_type Mood) 
{
    bullet* B = nullptr;
    switch (Mood) {
        case MoodType_Dot: {
            B = List_Push(&Mode->DotBullets);
        } break;
        case MoodType_Circle: {
            B = List_Push(&Mode->CircleBullets);
        } break;
        default: {
            Assert(false);
        }
    }
    B->Position = Position;
	B->Speed = Speed;
    B->Size = V2f_Create(16.f, 16.f);
    
    B->HitCircle = Circle2f_Create(V2f_Create(0.f, 0.f), 4.f);
    
    if (V2f_LengthSq(Direction) > 0.f) {
	    B->Direction = V2f_Normalize(Direction);
    }
    B->MoodType = Mood;
    
}

static inline void
UpdateBulletsSub(list<bullet>* L,
                 f32 DeltaTime) 
{
    for(u32 I = 0; I < L->Count;) {
        bullet* B = List_Get(L, I);
        
        f32 SpeedDt = B->Speed * DeltaTime;
        v2f Velocity = V2f_Mul(B->Direction, SpeedDt);
        B->Position = V2f_Add(B->Position, Velocity);
        
        if (B->Position.X <= -Game_DesignWidth * 0.5f - B->HitCircle.Radius || 
            B->Position.X >= Game_DesignWidth * 0.5f + B->HitCircle.Radius ||
            B->Position.Y <= -Game_DesignHeight * 0.5f - B->HitCircle.Radius ||
            B->Position.Y >= Game_DesignHeight * 0.5f + B->HitCircle.Radius) {
            List_Slear(L, I);
            continue;
        }
        ++I;
    }
    
}

static inline void
UpdateBullets(game_mode_main* Mode,
              f32 DeltaTime) 
{
    UpdateBulletsSub(&Mode->DotBullets, DeltaTime);
    UpdateBulletsSub(&Mode->CircleBullets, DeltaTime);
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

#endif //GAME_MODE_MAIN_BULLET_H
