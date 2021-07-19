
static inline void
update_wave(Game_Mode_Main* mode,
            f32 dt) 
{
    if (mode->wave.is_done) {
        // TODO: Random wave type
        mode->wave.type = WAVE_PATTERN_TYPE_SPAWN_N_FOR_DURATION;
        // Initialize the wave
        switch (mode->wave.type) {
            case WAVE_PATTERN_TYPE_SPAWN_N_FOR_DURATION: {
                auto* pattern = &mode->wave.pattern_spawn_n_for_duration;
                pattern->enemies_per_spawn = 1;
                pattern->spawn_timer = 0.f;
                pattern->spawn_duration = 3.f;
                pattern->timer = 0.f;
                pattern->duration = 30.f;
            } break;
            default: {
                ASSERT(false);
            }
        }
        mode->wave.is_done = false;
    }
    else {
        // Update the wave.
        switch(mode->wave.type) {
            case WAVE_PATTERN_TYPE_SPAWN_N_FOR_DURATION: {
                auto* pattern = &mode->wave.pattern_spawn_n_for_duration;
                pattern->spawn_timer += dt;
                pattern->timer += dt;
                if (pattern->spawn_timer >= pattern->spawn_duration ) {
                    v2f pos = 
                        v2f::create(RngSeries_Bilateral(&mode->rng) * GAME_DESIGN_WIDTH * 0.5f,
                                    RngSeries_Bilateral(&mode->rng) * GAME_DESIGN_HEIGHT * 0.5f);
                    
                    
                    Enemy_Spawn(mode,
                                pos,
                                ENEMY_SHOOT_TYPE_8_DIR,
                                ENEMY_MOVEMENT_TYPE_STATIC);
                    
                    pattern->spawn_timer = 0.f;
                }
                
                if (pattern->timer >= pattern->duration) {
                    mode->wave.is_done = true;
                }
                
            } break;
            default: {
                ASSERT(false);
            }
        }
        
    }
    
}
