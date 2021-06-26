
static inline void
Bullet_Spawn(game_mode_main* Mode, 
             assets* Assets, 
             v2f Position, 
             v2f Direction, 
             f32 Speed, 
             mood_type Mood) 
{
    bullet* B = nullptr;
    switch (Mood) {
        case MoodType_Dot: {
            B = Mode->DotBullets.push();
        } break;
        case MoodType_Circle: {
            B = Mode->CircleBullets.push();
        } break;
        default: {
            Assert(false);
        }
    }
    B->Position = Position;
	B->Speed = Speed;
    B->Size = v2f_create(16.f, 16.f);
    
    B->HitCircle = Circle2f_Create(v2f_create(0.f, 0.f), 4.f);
    
    if (length_sq(Direction) > 0.f) {
	    B->Direction = normalize(Direction);
    }
    B->MoodType = Mood;
    
}

static inline void
Main_UpdateBullets(game_mode_main* Mode,
                   f32 DeltaTime) 
{
    auto SlearIfLamb = [&](bullet* B) {
        B->Position += B->Direction * B->Speed * DeltaTime;
        
        return B->Position.x <= -Game_DesignWidth * 0.5f - B->HitCircle.Radius || 
            B->Position.x >= Game_DesignWidth * 0.5f + B->HitCircle.Radius ||
            B->Position.y <= -Game_DesignHeight * 0.5f - B->HitCircle.Radius ||
            B->Position.y >= Game_DesignHeight * 0.5f + B->HitCircle.Radius;
    };
    
    Mode->DotBullets.foreach_slear_if(SlearIfLamb);
    Mode->CircleBullets.foreach_slear_if(SlearIfLamb);
    
}

static inline void
Main_RenderBullets(game_mode_main* Mode) 
{
    // Bullet Rendering.
    // NOTE(Momo): Circles are in front of Dots and are therefore 'nearer'.
    // Thus we have to render Dots first before Circles.
    f32 LayerOffset = 0.f;
    auto ForEachLamb = [&](bullet* B, image_id Image) {
        m44f S = M44f_Scale(B->Size.x, 
                            B->Size.y, 
                            1.f);
        
        m44f T = M44f_Translation(B->Position.x,
                                  B->Position.y,
                                  ZLayBullet + LayerOffset);
        
        Draw_TexturedQuadFromImage(Image,
                                   M44f_Concat(T,S), 
                                   C4f_White);
        LayerOffset += 0.00001f;
    };
    
    
    Mode->DotBullets.foreach(ForEachLamb, Image_BulletDot);
    Mode->CircleBullets.foreach(ForEachLamb, Image_BulletCircle);
    
    
    
}
