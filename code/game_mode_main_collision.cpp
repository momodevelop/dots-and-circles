/* date = May 22nd 2021 7:09 pm */

#ifndef GAME_MODE_MAIN_COLLISION_H
#define GAME_MODE_MAIN_COLLISION_H


// TODO: We could put this into Bullet update
static inline void
update_player_bullet_collision(Game_Mode_Main* mode,
                               f32 dt)
{
    Player* player = &mode->player;
    circle2f player_circle = player->hit_circle;
    v2f player_vel = player->position - player->prev_position;
    player_circle.origin += player->position;
    
    auto lamb = [&](Bullet* b) {
        circle2f b_circle = b->hit_circle;
        v2f b_vel = b->direction * b->speed * dt;
        b_circle.origin += b->position;
        
        if (Bonk2_IsDynaCircleXDynaCircle(player_circle, 
                                          player_vel,
                                          b_circle,
                                          b_vel)) 
        {
            if (player->mood_type == b->mood_type) {
                v2f vector_to_bullet = normalize(b->position - player->position);
                v2f SpawnPos = player->position + vector_to_bullet * player->hit_circle.radius;
                spawn_particles(mode, SpawnPos, 5);
                mode->score += 50;
            }
            else {
                player->is_dead = true;
            }
            return true;
        }
        
        return false;
    };
    List_ForEachSlearIf(&mode->dot_bullets, lamb);
    List_ForEachSlearIf(&mode->circle_bullets, lamb);
}

#endif //GAME_MODE_MAIN_COLLISION_H
