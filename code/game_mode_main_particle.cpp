static inline void
spawn_particles(Game_Mode_Main* mode, 
                v2f position,
                u32 amount) 
{
    for (u32 I = 0; I < amount; ++I) {
        Particle* p = Queue_Push(&mode->particles);
        if (!p) {
            return;
        }
        p->position = position;
        p->direction.x = RngSeries_Bilateral(&mode->rng);
        p->direction.y = RngSeries_Bilateral(&mode->rng);
        p->direction = Normalize(p->direction);
        p->timer = 0.f;
        p->speed = RngSeries_Between(&mode->rng, p->min_speed, p->max_speed);
    }
}

static inline void 
update_particles(Game_Mode_Main* mode, f32 dt) {
    Queue<Particle>* q = &mode->particles;
    
    auto pop_lamb = [](Particle* p) {
        return p->timer >= p->duration;
    };
    Queue_PopUntil(q, pop_lamb);
    
    
    auto for_lamb = [](Particle* p, f32 dt) {
        p->timer += dt;
        p->position += p->direction * p->speed * dt;
    };
    Queue_ForEach(q, for_lamb, dt);
}


static inline void
render_particles(Game_Mode_Main* mode) {
    Queue<Particle>* q = &mode->particles;
    u32 current_count = 0;
    auto for_each_lamb = [&](Particle* p) {
        f32 ease = 1.f - (p->timer/p->duration);
        f32 alpha = p->alpha * ease;
        f32 size = p->size * ease;
        
        f32 Offset = current_count*0.001f;
        
        m44f S = m44f_Scale(size, size, 1.f);
        m44f T = m44f_Translation(p->position.x,
                                          p->position.y,
                                          ZLayParticles + Offset);
        DrawTexturedQuadFromImage(IMAGE_PARTICLE,
                                      T*S,
                                      c4f::create(1.f, 1.f, 1.f, alpha));
        ++current_count;
    };
    Queue_ForEach(q, for_each_lamb);
}
