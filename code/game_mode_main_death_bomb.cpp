

static inline void
update_death_bomb(Game_Mode_Main* mode, f32 dt) {
    Death_Bomb* death_bomb = &mode->death_bomb;
    death_bomb->radius += death_bomb->growth_speed * dt;
    
    circle2f death_bomb_circle = circle2f::create(death_bomb->position, death_bomb->radius);
    
    auto bullet_lamb = [&](Bullet* b) {
        circle2f BCircle = translate(b->hit_circle, b->position);
        
        // NOTE(Momo): We can safely assume that the circles are not moving
        if (Bonk2_IsCircleXCircle(death_bomb_circle,
                                  BCircle)) 
        {
            v2f vector_to_bullet = Normalize(b->position - death_bomb->position);
            v2f spawn_pos = death_bomb->position + vector_to_bullet * death_bomb->radius;
            spawn_particles(mode, spawn_pos, 5);
            return true;
        }
        
        return false;
    };
    
    List_ForEachSlearIf(&mode->circle_bullets, bullet_lamb);
    List_ForEachSlearIf(&mode->dot_bullets, bullet_lamb);
    
    auto enemy_lamb = [&](Enemy* e) {
        circle2f ECir = circle2f::create(e->position, 0.1f);
        
        // NOTE(Momo): We can safely assume that the circles are not moving
        if (Bonk2_IsCircleXCircle(death_bomb_circle, ECir)) 
        {
            v2f vector_to_bullet = Normalize(e->position - death_bomb->position);
            v2f spawn_pos = death_bomb->position + vector_to_bullet * death_bomb->radius;
            spawn_particles(mode, spawn_pos, 5);
            return true;
        }
        
        return false;
    };
    List_ForEachSlearIf(&mode->enemies, enemy_lamb);
}

static inline void
render_death_bomb(Game_Mode_Main* Mode)
{
    Death_Bomb* death_bomb = &Mode->death_bomb;
    // Circle?
    Renderer_DrawCircle2f(g_renderer,
                          circle2f::create(death_bomb->position, death_bomb->radius),
                          5.f, 32, C4F_WHITE, ZLayDeathBomb);
}

