#ifndef GAME_MODE_MAIN_CPP
#define GAME_MODE_MAIN_CPP


#include "game_mode_main_player.cpp"
#include "game_mode_main_bullet.cpp"
#include "game_mode_main_enemy.cpp"
#include "game_mode_main_wave.cpp"
#include "game_mode_main_particle.cpp"
#include "game_mode_main_collision.cpp"
#include "game_mode_main_debug.cpp"
#include "game_mode_main_death_bomb.cpp"

static inline b8 
init_main_mode(Permanent_State* perm_state,
               Transient_State* tran_state,
               Debug_State* debug_state) 
{
    Game_Mode_Main* mode = perm_state->main_mode;
    Arena* mode_arena = perm_state->mode_arena;
    
    g_platform->hide_cursor();
    
    // NOTE(Momo): Init camera
    {
        mode->camera.position = v3f::create(0.f, 0.f, 0.f);
        mode->camera.anchor = v3f::create(0.5f, 0.5f, 0.5f);
        mode->camera.color = C4F_GREY2;
        mode->camera.dimensions = v3f::create(GAME_DESIGN_WIDTH,
                                              GAME_DESIGN_HEIGHT,
                                              Game_DesignDepth);
    }
    
    b8 success = false;
    
    success = List_Alloc(&mode->dot_bullets, mode_arena, DotCap);
    if (!success) {
        return false;
    }
    
    success = List_Alloc(&mode->circle_bullets, mode_arena, CircleCap);
    if (!success) {
        return false;
    }
    
    success = List_Alloc(&mode->enemies, mode_arena, EnemyCap);
    if (!success) {
        return false;
    }
    
    success = Queue_Alloc(&mode->particles, mode_arena, ParticleCap);
    if (!success) {
        return false;
    }
    
    
    success = BigInt_Alloc(&mode->score, mode_arena, 128);
    if (!success) {
        return false;
    }
    mode->wave.is_done = true;
    mode->rng = RngSeries_Create(0); // TODO: Used system clock for seed.
    
    Player* player = &mode->player;
    {
        player->position = {};
        player->prev_position = {};
        player->size = player->max_size;
        player->hit_circle = { v2f{}, 16.f};
        
        // NOTE(Momo): We start as Dot
        player->mood_type = MOOD_TYPE_DOT;
        player->dot_image_alpha = 1.f;
        player->dot_image_alpha_target = 1.f;
        
        player->dot_image_transition_duration = 0.1f;
        player->dot_image_transition_timer = player->dot_image_transition_duration;
        player->is_dead = false;
    }
    mode->wave.is_done = true;
    
#if 0    
    success = AudioMixer_Play(&tran_state->Mixer, SOUND_TEST, false, &mode->BgmHandle);
    if (!success) {
        return false;
        
    }
#endif
    
    
    mode->camera.set();
    
    mode->state = GAME_MODE_MAIN_STATE_SPAWN;
    return true; 
    
}

static inline void
Main_StateNormal_Update(Permanent_State* perm_state, 
                        Transient_State* tran_state,
                        Debug_State* debug_state,
                        f32 DeltaTime) 
{
    Game_Mode_Main* mode = perm_state->main_mode;
    
    update_input(mode);
    update_player(mode, DeltaTime);    
    update_bullets(mode, DeltaTime);
    update_wave(mode, DeltaTime);
    update_enemies(mode, DeltaTime); 
    update_player_bullet_collision(mode, DeltaTime);
    update_particles(mode, DeltaTime);
    
    // NOTE(Momo): if player's dead, do dead stuff
    if(mode->player.is_dead) 
    {
        // NOTE(Momo): Drop the death bomb
        mode->death_bomb.radius = 0.f;
        mode->death_bomb.position = mode->player.position;
        
        mode->state = GAME_MODE_MAIN_STATE_PLAYER_DIED;
        mode->player.position = v2f::create(-1000.f, -1000.f);
    }
    
    render_player(mode);
    render_bullets(mode);
    render_enemies(mode);
    render_particles(mode);
    render_score(mode);
    
}



static inline void
update_main_mode_player_died_state(Permanent_State* perm_state, 
                                   Transient_State* tran_state,
                                   Debug_State* debug_state,
                                   f32 dt) 
{
    // Everything stops
    Game_Mode_Main* mode = perm_state->main_mode;
    
    update_death_bomb(mode, dt);
    update_particles(mode, dt);
    
    //Main_RenderScore(mode, &tran_state->Arena);
    render_player(mode);
    render_bullets(mode);
    render_enemies(mode);
    render_particles(mode);
    render_death_bomb(mode);
    
    
    // NOTE: PlayerDied -> Spawning state
    // NOTE: Change state if enemy and Bullet count is 0
    if (mode->death_bomb.radius >= GAME_DESIGN_WIDTH * 2.f) 
    {
        mode->state = GAME_MODE_MAIN_STATE_PLAYER_DIED;
        mode->spawn_timer = 0.f;
        mode->player.is_dead = false;
    }
}

static inline void
update_main_mode_spawn_state(Permanent_State* perm_state, 
                             Transient_State* tran_state,
                             Debug_State* debug_state,
                             f32 dt) 
{
    Game_Mode_Main* mode = perm_state->main_mode;
    
    f32 ease = ease_out_bounce(CLAMP(mode->spawn_timer/mode->spawn_duration, 0.f, 1.f));
    mode->player.size = mode->player.max_size * ease;
    
    update_input(mode);
    update_player(mode, dt);    
    
    //Main_RenderScore(mode, &tran_state->Arena, Assets);
    render_player(mode);
    
    
    // NOTE(Momo): Spawning -> Normal state
    if (mode->spawn_timer >= mode->spawn_duration) {
        mode->state = GAME_MODE_MAIN_STATE_NORMAL;
        mode->player.size = mode->player.max_size;
    }
    mode->spawn_timer += dt;
    
    
}

static inline void
update_main_mode(Permanent_State* perm_state, 
                 Transient_State* tran_state,
                 Debug_State* debug_state,
                 f32 dt) 
{
    Game_Mode_Main* mode = perm_state->main_mode;
    
    switch(mode->state) {
        case GAME_MODE_MAIN_STATE_SPAWN: {
            update_main_mode_spawn_state(perm_state, tran_state, debug_state, dt);
        } break;
        case GAME_MODE_MAIN_STATE_NORMAL: {
            Main_StateNormal_Update(perm_state, tran_state, debug_state, dt);
        }break;
        case GAME_MODE_MAIN_STATE_PLAYER_DIED: {
            update_main_mode_player_died_state(perm_state, tran_state, debug_state, dt);
        } break;
        case GAME_MODE_MAIN_STATE_CLEANUP: {
        } break;
        
    }
    
    //Main_RenderDebugLines(mode, RenderCommands);
    String buffer = {};
    buffer.init("Dots: ");
    debug_state->inspector.push_u32(buffer,
                                    mode->dot_bullets.count);
    buffer.init("Circles: ");
    debug_state->inspector.push_u32(buffer, 
                                    mode->circle_bullets.count);
    
    buffer.init("Bullets: ");
    debug_state->inspector.push_u32(buffer, 
                                    mode->dot_bullets.count + mode->circle_bullets.count);
    
    buffer.init("enemies: ");
    debug_state->inspector.push_u32(buffer, 
                                    mode->enemies.count);
}

#endif //GAME_MODE_H
