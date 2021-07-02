/* date = May 22nd 2021 7:09 pm */

#ifndef GAME_MODE_MAIN_COLLISION_H
#define GAME_MODE_MAIN_COLLISION_H


// TODO: We could put this into Bullet update
static inline void
Main_UpdatePlayerBulletCollision(game_mode_main* Mode,
                                 f32 DeltaTime)
{
    Player* player = &Mode->player;
    circle2f PlayerCircle = player->hit_circle;
    v2f PlayerVel = player->position - player->prev_position;
    PlayerCircle.origin += player->position;
    
    auto Lamb = [&](Bullet* B) {
        circle2f BCircle = B->hit_circle;
        v2f BVel = B->direction * B->speed * DeltaTime;
        BCircle.origin += B->position;
        
        if (Bonk2_IsDynaCircleXDynaCircle(PlayerCircle, 
                                          PlayerVel,
                                          BCircle,
                                          BVel)) 
        {
            if (player->mood_type == B->mood_type) {
                v2f VectorToBullet = normalize(B->position - player->position);
                v2f SpawnPos = player->position + VectorToBullet * player->hit_circle.radius;
                Main_SpawnParticle(Mode, SpawnPos, 5);
                Mode->score += 50;
            }
            else {
                player->is_dead = true;
            }
            return true;
        }
        
        return false;
    };
    Mode->dot_bullets.foreach_slear_if(Lamb);
    Mode->circle_bullets.foreach_slear_if(Lamb);
}

#endif //GAME_MODE_MAIN_COLLISION_H
