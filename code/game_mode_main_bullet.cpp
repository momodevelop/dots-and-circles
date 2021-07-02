

static inline void
spawn_bullet(Game_Mode_Main* mode, 
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
    b->size = v2f::create(16.f, 16.f);
    b->hit_circle = circle2f::create(v2f::create(0.f, 0.f), 4.f);
    
    if (length_sq(direction) > 0.f) {
	    b->direction = normalize(direction);
    }
    b->mood_type = mood;
    
}

static inline void
update_bullets(Game_Mode_Main* mode,
               f32 dt) 
{
    auto slear_if_lamb = [&](Bullet* B) {
        B->position += B->direction * B->speed * dt;
        
        return B->position.x <= -Game_DesignWidth * 0.5f - B->hit_circle.radius || 
            B->position.x >= Game_DesignWidth * 0.5f + B->hit_circle.radius ||
            B->position.y <= -Game_DesignHeight * 0.5f - B->hit_circle.radius ||
            B->position.y >= Game_DesignHeight * 0.5f + B->hit_circle.radius;
    };
    
    mode->dot_bullets.foreach_slear_if(slear_if_lamb);
    mode->circle_bullets.foreach_slear_if(slear_if_lamb);
    
}

static inline void
render_bullets(Game_Mode_Main* mode) 
{
    // Bullet Rendering.
    // NOTE(Momo): Circles are in front of Dots and are therefore 'nearer'.
    // Thus we have to render Dots first before Circles.
    f32 layer_offset = 0.f;
    auto for_lamb = [&](Bullet* b, Image_ID image) {
        m44f s = m44f::create_scale(b->size.x, 
                                    b->size.y, 
                                    1.f);
        
        m44f t = m44f::create_translation(b->position.x,
                                          b->position.y,
                                          ZLayBullet + layer_offset);
        
        draw_textured_quad_from_image(image,
                                      t*s, 
                                      C4F_WHITE);
        layer_offset += 0.00001f;
    };
    
    
    mode->dot_bullets.foreach(for_lamb, IMAGE_BULLET_DOT);
    mode->circle_bullets.foreach(for_lamb, IMAGE_BULLET_CIRCLE);
    
    
    
}
