


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
            b = List_Push(&mode->dot_bullets);
        } break;
        case MOOD_TYPE_CIRCLE: {
            b = List_Push(&mode->circle_bullets);
        } break;
        default: {
            ASSERT(false);
        }
    }
    b->position = position;
	b->speed = speed;
    b->size = v2f_Create(16.f, 16.f);
    b->hit_circle = circle2f_Create(v2f_Create(0.f, 0.f), 4.f);
    
    if (LengthSq(direction) > 0.f) {
	    b->direction = Normalize(direction);
    }
    b->mood_type = mood;
}

static inline void
update_bullets(Game_Mode_Main* mode,
               f32 dt) 
{
    auto slear_if_lamb = [&](Bullet* B) {
        B->position += B->direction * B->speed * dt;
        
        return B->position.x <= -GAME_DESIGN_WIDTH * 0.5f - B->hit_circle.radius || 
            B->position.x >= GAME_DESIGN_WIDTH * 0.5f + B->hit_circle.radius ||
            B->position.y <= -GAME_DESIGN_HEIGHT * 0.5f - B->hit_circle.radius ||
            B->position.y >= GAME_DESIGN_HEIGHT * 0.5f + B->hit_circle.radius;
    };
    
    List_ForEachSlearIf(&mode->dot_bullets, slear_if_lamb);
    List_ForEachSlearIf(&mode->circle_bullets, slear_if_lamb);
    
}

static inline void
render_bullets(Game_Mode_Main* mode) 
{
    // Bullet Rendering.
    // NOTE(Momo): Circles are in front of Dots and are therefore 'nearer'.
    // Thus we have to render Dots first before Circles.
    f32 layer_offset = 0.f;
    auto for_lamb = [&](Bullet* b, Image_ID image) {
        m44f s = m44f_Scale(b->size.x, 
                                    b->size.y, 
                                    1.f);
        
        m44f t = m44f_Translation(b->position.x,
                                          b->position.y,
                                          ZLayBullet + layer_offset);
        
        DrawTexturedQuadFromImage(image,
                                      t*s, 
                                      C4F_WHITE);
        layer_offset += 0.00001f;
    };
    
    
#if 0
    List_Foreach(&mode->dot_bullets, for_lamb, IMAGE_BULLET_DOT);
    List_Foreach(&mode->circle_bullets, for_lamb, IMAGE_BULLET_CIRCLE);
#endif
    
    
    
}
