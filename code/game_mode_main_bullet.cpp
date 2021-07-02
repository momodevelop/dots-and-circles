

static inline void
Bullet_Spawn(game_mode_main* mode, 
             v2f position, 
             v2f direction, 
             f32 speed, 
             Mood_Type mood) 
{
    Bullet* b = nullptr;
    switch (mood) {
        case MOOD_TYPE_DOT: {
            b = mode->dot_bullets.push();
        } break;
        case MOOD_TYPE_CIRCLE: {
            b = mode->circle_bullets.push();
        } break;
        default: {
            ASSERT(false);
        }
    }
    b->position = position;
	b->speed = speed;
    b->Size = v2f::create(16.f, 16.f);
    
    b->hit_circle = circle2f::create(v2f::create(0.f, 0.f), 4.f);
    
    if (length_sq(direction) > 0.f) {
	    b->direction = normalize(direction);
    }
    b->mood_type = mood;
    
}

static inline void
Main_UpdateBullets(game_mode_main* Mode,
                   f32 DeltaTime) 
{
    auto SlearIfLamb = [&](Bullet* B) {
        B->position += B->direction * B->speed * DeltaTime;
        
        return B->position.x <= -Game_DesignWidth * 0.5f - B->hit_circle.radius || 
            B->position.x >= Game_DesignWidth * 0.5f + B->hit_circle.radius ||
            B->position.y <= -Game_DesignHeight * 0.5f - B->hit_circle.radius ||
            B->position.y >= Game_DesignHeight * 0.5f + B->hit_circle.radius;
    };
    
    Mode->dot_bullets.foreach_slear_if(SlearIfLamb);
    Mode->circle_bullets.foreach_slear_if(SlearIfLamb);
    
}

static inline void
Main_RenderBullets(game_mode_main* Mode) 
{
    // Bullet Rendering.
    // NOTE(Momo): Circles are in front of Dots and are therefore 'nearer'.
    // Thus we have to render Dots first before Circles.
    f32 LayerOffset = 0.f;
    auto ForEachLamb = [&](Bullet* B, Image_ID Image) {
        m44f S = m44f::create_scale(B->Size.x, 
                                    B->Size.y, 
                                    1.f);
        
        m44f T = m44f::create_translation(B->position.x,
                                          B->position.y,
                                          ZLayBullet + LayerOffset);
        
        draw_textured_quad_from_image(Image,
                                      T*S, 
                                      C4F_WHITE);
        LayerOffset += 0.00001f;
    };
    
    
    Mode->dot_bullets.foreach(ForEachLamb, IMAGE_BULLET_DOT);
    Mode->circle_bullets.foreach(ForEachLamb, IMAGE_BULLET_CIRCLE);
    
    
    
}
