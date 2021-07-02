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
MainMode_Init(permanent_state* PermState,
              transient_state* TranState,
              debug_state* DebugState) 
{
    game_mode_main* Mode = PermState->MainMode;
    Arena* ModeArena = PermState->ModeArena;
    
    G_Platform->HideCursorFp();
    
    // NOTE(Momo): Init camera
    {
        Mode->camera.position = v3f::create(0.f, 0.f, 0.f);
        Mode->camera.anchor = v3f::create(0.5f, 0.5f, 0.5f);
        Mode->camera.color = C4F_GREY2;
        Mode->camera.dimensions = v3f::create(Game_DesignWidth,
                                              Game_DesignHeight,
                                              Game_DesignDepth);
    }
    
    b8 Success = false;
    
    Success = Mode->dot_bullets.alloc(ModeArena, DotCap);
    if (!Success) {
        return false;
    }
    
    Success = Mode->circle_bullets.alloc(ModeArena, CircleCap);
    if (!Success) {
        return false;
    }
    
    Success = Mode->enemies.alloc(ModeArena, EnemyCap);
    if (!Success) {
        return false;
    }
    
    Success = Mode->particles.alloc(ModeArena, ParticleCap);
    if (!Success) {
        return false;
    }
    
    
    Success = Mode->score.alloc(ModeArena, 128);
    if (!Success) {
        return false;
    }
    Mode->wave.is_done = true;
    Mode->rng = Rng_Series::create(0); // TODO: Used system clock for seed.
    
    Player* player = &Mode->player;
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
    Mode->wave.is_done = true;
    
#if 0    
    Success = AudioMixer_Play(&TranState->Mixer, SOUND_TEST, false, &Mode->BgmHandle);
    if (!Success) {
        return false;
        
    }
#endif
    
    
    Mode->camera.set();
    
    Mode->State = Main_StateType_Spawning;
    return true; 
    
}

static inline void
Main_StateNormal_Update(permanent_state* PermState, 
                        transient_state* TranState,
                        debug_state* DebugState,
                        f32 DeltaTime) 
{
    game_mode_main* Mode = PermState->MainMode;
    
    Main_UpdateInput(Mode);
    Main_UpdatePlayer(Mode, DeltaTime);    
    Main_UpdateBullets(Mode, DeltaTime);
    Main_UpdateWaves(Mode, DeltaTime);
    Main_UpdateEnemies(Mode, DeltaTime); 
    Main_UpdatePlayerBulletCollision(Mode, DeltaTime);
    Main_UpdateParticles(Mode, DeltaTime);
    
    // NOTE(Momo): if player's dead, do dead stuff
    if(Mode->player.is_dead) 
    {
        // NOTE(Momo): Drop the death bomb
        Mode->death_bomb.radius = 0.f;
        Mode->death_bomb.position = Mode->player.position;
        
        Mode->State = Main_StateType_PlayerDied;
        Mode->player.position = v2f::create(-1000.f, -1000.f);
    }
    
    Main_RenderPlayer(Mode);
    Main_RenderBullets(Mode);
    Main_RenderEnemies(Mode);
    Main_RenderParticles(Mode);
    Main_RenderScore(Mode);
    
}



static inline void
Main_StatePlayerDied_Update(permanent_state* PermState, 
                            transient_state* TranState,
                            debug_state* DebugState,
                            f32 DeltaTime) 
{
    // Everything stops
    game_mode_main* Mode = PermState->MainMode;
    
    Main_UpdateDeathBomb(Mode, DeltaTime);
    Main_UpdateParticles(Mode, DeltaTime);
    
    //Main_RenderScore(Mode, &TranState->Arena);
    Main_RenderPlayer(Mode);
    Main_RenderBullets(Mode);
    Main_RenderEnemies(Mode);
    Main_RenderParticles(Mode);
    Main_RenderDeathBomb(Mode);
    
    
    // NOTE: PlayerDied -> Spawning state
    // NOTE: Change state if enemy and Bullet count is 0
    if (Mode->death_bomb.radius >= Game_DesignWidth * 2.f) 
    {
        Mode->State = Main_StateType_Spawning;
        Mode->spawn_timer = 0.f;
        Mode->player.is_dead = false;
    }
}

static inline void
Main_StateSpawning_Update(permanent_state* PermState, 
                          transient_state* TranState,
                          debug_state* DebugState,
                          f32 DeltaTime) 
{
    game_mode_main* Mode = PermState->MainMode;
    
    f32 Ease = ease_out_bounce(CLAMP(Mode->spawn_timer/Mode->spawn_duration, 0.f, 1.f));
    Mode->player.size = Mode->player.max_size * Ease;
    
    Main_UpdateInput(Mode);
    Main_UpdatePlayer(Mode, DeltaTime);    
    
    //Main_RenderScore(Mode, &TranState->Arena, Assets);
    Main_RenderPlayer(Mode);
    
    
    // NOTE(Momo): Spawning -> Normal state
    if (Mode->spawn_timer >= Mode->spawn_duration) {
        Mode->State = Main_StateType_Normal;
        Mode->player.size = Mode->player.max_size;
    }
    Mode->spawn_timer += DeltaTime;
    
    
}

static inline void
MainMode_Update(permanent_state* PermState, 
                transient_state* TranState,
                debug_state* DebugState,
                f32 DeltaTime) 
{
    game_mode_main* Mode = PermState->MainMode;
    
    switch(Mode->State) {
        case Main_StateType_Spawning: {
            Main_StateSpawning_Update(PermState, TranState, DebugState, DeltaTime);
        } break;
        case Main_StateType_Normal: {
            Main_StateNormal_Update(PermState, TranState, DebugState, DeltaTime);
        }break;
        case Main_StateType_PlayerDied: {
            Main_StatePlayerDied_Update(PermState, TranState, DebugState, DeltaTime);
        } break;
        case Main_StateType_Cleanup: {
        } break;
        
    }
    
    //Main_RenderDebugLines(Mode, RenderCommands);
    String Buffer = {};
    Buffer.init("Dots: ");
    DebugState->inspector.push_u32(Buffer,
                                   Mode->dot_bullets.count);
    Buffer.init("Circles: ");
    DebugState->inspector.push_u32(Buffer, 
                                   Mode->circle_bullets.count);
    
    Buffer.init("Bullets: ");
    DebugState->inspector.push_u32(Buffer, 
                                   Mode->dot_bullets.count + Mode->circle_bullets.count);
    
    Buffer.init("enemies: ");
    DebugState->inspector.push_u32(Buffer, 
                                   Mode->enemies.count);
}

#endif //GAME_MODE_H
