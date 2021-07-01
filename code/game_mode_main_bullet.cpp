

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
            ASSERT(false);
        }
    }
    B->Position = Position;
	B->Speed = Speed;
    B->Size = v2f::create(16.f, 16.f);
    
    B->HitCircle = circle2f::create(v2f::create(0.f, 0.f), 4.f);
    
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
        
        return B->Position.x <= -Game_DesignWidth * 0.5f - B->HitCircle.radius || 
            B->Position.x >= Game_DesignWidth * 0.5f + B->HitCircle.radius ||
            B->Position.y <= -Game_DesignHeight * 0.5f - B->HitCircle.radius ||
            B->Position.y >= Game_DesignHeight * 0.5f + B->HitCircle.radius;
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
    auto ForEachLamb = [&](bullet* B, Image_ID Image) {
        m44f S = m44f::create_scale(B->Size.x, 
                            B->Size.y, 
                            1.f);
        
        m44f T = m44f::create_translation(B->Position.x,
                                  B->Position.y,
                                  ZLayBullet + LayerOffset);
        
        Draw_TexturedQuadFromImage(Image,
                                   T*S, 
                                   C4F_WHITE);
        LayerOffset += 0.00001f;
    };
    
    
    Mode->DotBullets.foreach(ForEachLamb, Image_BulletDot);
    Mode->CircleBullets.foreach(ForEachLamb, Image_BulletCircle);
    
    
    
}
