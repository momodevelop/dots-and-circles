static inline void
Main_SpawnParticle(game_mode_main* mode, 
                   v2f position,
                   u32 amount) 
{
    for (u32 I = 0; I < amount; ++I) {
        Particle* p = mode->particles.push();
        if (!p) {
            return;
        }
        p->position = position;
        p->direction.x = mode->rng.bilateral();
        p->direction.y = mode->rng.bilateral();
        p->direction = normalize(p->direction);
        p->timer = 0.f;
        p->speed = mode->rng.between(p->min_speed, p->max_speed);
    }
}

static inline void 
Main_UpdateParticles(game_mode_main* mode, f32 dt) {
    Queue<Particle>* q = &mode->particles;
    
    auto pop_lamb = [](Particle* p) {
        return p->timer >= p->duration;
    };
    q->pop_until(pop_lamb);
    
    
    auto for_lamb = [](Particle* p, f32 dt) {
        p->timer += dt;
        p->position += p->direction * p->speed * dt;
    };
    q->foreach(for_lamb, dt);
}


static inline void
Main_RenderParticles(game_mode_main* mode) {
    Queue<Particle>* q = &mode->particles;
    u32 current_count = 0;
    auto ForEachLamb = [&](Particle* p) {
        f32 ease = 1.f - (p->timer/p->duration);
        f32 alpha = p->alpha * ease;
        f32 size = p->size * ease;
        
        f32 Offset = current_count*0.001f;
        
        m44f S = m44f::create_scale(size, size, 1.f);
        m44f T = m44f::create_translation(p->position.x,
                                          p->position.y,
                                          ZLayParticles + Offset);
        draw_textured_quad_from_image(IMAGE_PARTICLE,
                                      T*S,
                                      c4f::create(1.f, 1.f, 1.f, alpha));
        ++current_count;
    };
    q->foreach(ForEachLamb);
}
