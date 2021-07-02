

static inline void
Enemy_SetStateSpawn(Enemy* E) {
    E->state = EnemyState_Spawning;
    E->state_spawn.timer = 0.f;
}

static inline void
Enemy_SetStateDying(Enemy* E) {
    E->state = EnemyState_Dying;
    E->state_dying.timer = 0.f;
}

static inline void
Enemy_SetStateActive(Enemy* E) {
    E->state = EnemyState_Active;
    E->state_active.timer = 0.f;
}

static inline void
Enemy_Spawn(Game_Mode_Main* mode,
            v2f position,
            Enemy_Shoot_Type shoot_type, 
            Enemy_Movement_Type movement_type) 
{
    Enemy* enemy = mode->enemies.push();
    enemy->position = position;
    
    // NOTE(Momo): Shoot
    enemy->shoot_type = shoot_type;
    switch(enemy->shoot_type) {
        case ENEMY_SHOOT_TYPE_HOMING: {
            Enemy_Shoot_Homing* shoot = &enemy->shoot_homing;
            shoot->timer = 0.f;
            shoot->duration = 0.1f;
            shoot->mood = (Mood_Type)mode->rng.choice(MOOD_TYPE_COUNT);
        } break;
        case ENEMY_SHOOT_TYPE_8_DIR: {
            Enemy_Shoot_8_Dir* shoot = &enemy->shoot_8_dir;
            shoot->timer = 0.f;
            shoot->duration = 0.1f;
            shoot->mood = (Mood_Type)mode->rng.choice(MOOD_TYPE_COUNT);
        } break;
    };
    
    enemy->movement_type = movement_type;
    
    Enemy_SetStateSpawn(enemy);
}

static inline void
Enemy_DoStateActive(Enemy* enemy, Player* player, Game_Mode_Main* mode, f32 dt) {
    Enemy_State_Active* active = &enemy->state_active;
    // Movement
    switch( enemy->movement_type ) {
        case ENEMY_MOVEMENT_TYPE_STATIC:
        // Do nothing
        break;
        default: {
            ASSERT(false);
        }
    }
    
    // rotation
    enemy->rotation += enemy->rotation_speed * dt;
    enemy->rotation_speed += (enemy->active_state_rotation_speed - enemy->rotation_speed) * 0.9f;
    
    // Fire
    switch (enemy->shoot_type) {
        case ENEMY_SHOOT_TYPE_HOMING: {
            Enemy_Shoot_Homing* shoot = &enemy->shoot_homing;
            shoot->timer += dt;
            if (shoot->timer > shoot->duration) {
                v2f direction = normalize(player->position - enemy->position);
                spawn_bullet(mode, enemy->position, direction, 200.f, shoot->mood);
                shoot->timer = 0.f;
            }
        } break;
        case ENEMY_SHOOT_TYPE_8_DIR: {
            Enemy_Shoot_8_Dir* shoot = &enemy->shoot_8_dir;
            shoot->timer += dt;
            static m22f rotate_mtx = m22f::create_rotation(TAU/8);
            if (shoot->timer > shoot->duration) {
                v2f dir = v2f::create(1.f, 0.f);
                for (u32 I = 0; I < 8; ++I) {
                    dir = rotate_mtx * dir;
                    spawn_bullet(mode, enemy->position, dir, 200.f, shoot->mood);
                }
                shoot->timer = 0.f;
            }
            
        } break;
        default: {
            ASSERT(false);
        }
    }
    
    // Life time
    active->timer += dt;
    if (active->timer > active->duration) {
        Enemy_SetStateDying(enemy);
    }
}


static inline void 
update_enemies(Game_Mode_Main* mode,
               f32 dt) 
{
    Player* player = &mode->player;
    auto slear_lamb = [&](Enemy* enemy) {
        switch(enemy->state) {
            case EnemyState_Spawning: {
                Enemy_State_Spawn* spawn = &enemy->state_spawn;
                spawn->timer += dt;
                
                enemy->rotation += dt * enemy->spawn_state_rotation_speed;
                if (spawn->timer >= spawn->duration) {
                    Enemy_SetStateActive(enemy);
                }
            } break;
            case EnemyState_Active: {
                Enemy_DoStateActive(enemy, player, mode, dt);
            } break;
            case EnemyState_Dying: {
                Enemy_State_Dying* dying = &enemy->state_dying;
                dying->timer += dt;
                enemy->rotation += dt * enemy->die_state_rotation_speed;
                
                return dying->timer >= dying->duration;
            } break;
        }
        return false;
    };
    mode->enemies.foreach_slear_if(slear_lamb);
    
}

static inline void
render_enemies(Game_Mode_Main* mode) 
{
    u32 current_count = 0;
    
    auto for_lamb = [&](Enemy* enemy){
        f32 size = enemy->size;
        f32 offset = 0.00001f * current_count++;
        switch(enemy->state) {
            case EnemyState_Spawning: {
                Enemy_State_Spawn* spawn = &enemy->state_spawn;
                f32 ease = ease_out_quad(spawn->timer/spawn->duration);
                size = enemy->size * ease;
            } break;
            case EnemyState_Dying: {
                Enemy_State_Dying* Dying = &enemy->state_dying;
                f32 ease = 1.f - Dying->timer/Dying->duration;
                size = enemy->size * ease;
            }
        }
        m44f s = m44f::create_scale(size, size, 1.f);
        m44f r = m44f::create_rotation_z(enemy->rotation);
        m44f t = m44f::create_translation(enemy->position.x,
                                          enemy->position.y,
                                          ZLayEnemy + offset);
        
        draw_textured_quad_from_image(IMAGE_ENEMY,
                                      t*r*s, 
                                      C4F_WHITE);
    };
    mode->enemies.foreach(for_lamb);
}
