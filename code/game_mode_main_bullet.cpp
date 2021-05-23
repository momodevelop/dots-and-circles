
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
Main_UpdateBullets(game_mode_main* Mode,
                   f32 DeltaTime) 
{
    auto SlearIfLamb = [&](bullet* B) {
        B->Position += B->Direction * B->Speed * DeltaTime;
        
        return B->Position.X <= -Game_DesignWidth * 0.5f - B->HitCircle.Radius || 
            B->Position.X >= Game_DesignWidth * 0.5f + B->HitCircle.Radius ||
            B->Position.Y <= -Game_DesignHeight * 0.5f - B->HitCircle.Radius ||
            B->Position.Y >= Game_DesignHeight * 0.5f + B->HitCircle.Radius;
    };
    
    List_ForEachSlearIf(&Mode->DotBullets, SlearIfLamb);
    List_ForEachSlearIf(&Mode->CircleBullets, SlearIfLamb);
    
}

static inline void
Main_RenderBullets(game_mode_main* Mode,
                   assets* Assets,
                   mailbox* RenderCommands) 
{
    // Bullet Rendering.
    // NOTE(Momo): Circles are in front of Dots and are therefore 'nearer'.
    // Thus we have to render Dots first before Circles.
    f32 LayerOffset = 0.f;
    auto ForEachLamb = [&](bullet* B, image_id Image) {
        m44f S = M44f_Scale(B->Size.X, 
                            B->Size.Y, 
                            1.f);
        
        m44f T = M44f_Translation(B->Position.X,
                                  B->Position.Y,
                                  ZLayBullet + LayerOffset);
        
        Draw_TexturedQuadFromImage(RenderCommands,
                                   Assets,
                                   Image,
                                   M44f_Concat(T,S), 
                                   Color_White);
        LayerOffset += 0.01f;
    };
    
    
    List_ForEach(&Mode->DotBullets, ForEachLamb, Image_BulletDot);
    List_ForEach(&Mode->CircleBullets, ForEachLamb, Image_BulletCircle);
    
    
    
}
